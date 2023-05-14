// (C)2006 S2 Games
// c_updater.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifdef linux
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>
#endif

#ifdef __APPLE__
#include <sys/stat.h>
#include <zlib.h>
#endif

#include "c_updater.h"

#include "c_uitrigger.h"
#include "c_cmd.h"
#include "c_filemanager.h"
#include "c_uimanager.h"
#include "c_eventmanager.h"
#include "c_uicmd.h"
#include "c_hostserver.h"
#include "c_world.h"
#include "c_xmldoc.h"
#include "c_mmapunzip.h"
#include "c_xmlprocroot.h"
#include "c_compressedfile.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_checksumtable.h"
#include "c_restorevalue.h"
#include "c_soundmanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
UI_TRIGGER(UpdateStatus);
UI_TRIGGER(UpdateMessage);
UI_TRIGGER(UpdatePercent);
UI_TRIGGER(UpdateRestartNeeded);
UI_TRIGGER(UpdateLocalVersion);
UI_TRIGGER(UpdateCurVersion);
UI_TRIGGER(UpdateError);
UI_TRIGGER(UpdateProgress);
UI_TRIGGER(UpdateSpeed);
UI_TRIGGER(UpdateTime);
UI_TRIGGER(UpdateFilesLeft);
UI_TRIGGER(UpdateChangelog);
UI_TRIGGER(UpdateApplyPercent);

UI_TRIGGER(CompatCalculating);
UI_TRIGGER(CompatDownloading);
UI_TRIGGER(CompatMessage);
UI_TRIGGER(CompatVersion);
UI_TRIGGER(CompatFilesLeft);
UI_TRIGGER(CompatProgress);
UI_TRIGGER(CompatPercent);
UI_TRIGGER(CompatSpeed);
UI_TRIGGER(CompatTime);
UI_TRIGGER(CompatWriting);
UI_TRIGGER(CompatWritePercent);
UI_TRIGGER(CompatError);

CVAR_UINTR(upd_maxActiveDownloads,      4,              CVAR_SAVECONFIG,    1,      50);
CVAR_BOOLF(upd_checkForUpdates,         true,           CVAR_SAVECONFIG);
CVAR_BOOLF(upd_ftpActive,               false,          CVAR_SAVECONFIG);
CVAR_BOOL(upd_saveCompatArchives,       true);
CVAR_BOOL(upd_fakeUpdate,               false);

EXTERN_CVAR_BOOL(host_dynamicResReload);
EXTERN_CVAR_STRING(host_startupCfg);

SINGLETON_INIT(CUpdater)

struct SArchiveEntry
{
    SArchiveEntry()
        : pArchive(NULL)
    {
    }
    SArchiveEntry(CArchive* pArch)
        : pArchive(pArch)
    {
    }

    tsvector        vFileList;
    CChecksumTable  cChecksums;
    CArchive*       pArchive;
};
//=============================================================================

/*====================
  CUpdater::CUpdater
  ====================*/
CUpdater::CUpdater() :
m_pRequest(NULL),
m_uiLastSpeedUpdate(-1),
m_uiCurDownloaded(0),
m_uiTotalResumed(0),
m_uiTotalSize(0),
m_uiNumComplete(0),
m_bCheckingForUpdate(false),
m_sUpdateVersion(_T("")),
m_uiTotalFiles(0),
m_uiTotalSized(0),
m_uiNumActive(0),
m_uiNumSpeedUpdates(0),
m_uiFirstSpeedUpdate(0),
m_bDownloadingCompat(false),
m_eStatus(UPDATE_STATUS_IDLE),
m_bRequireConfirmation(false)
{
}


/*====================
  CUpdater::~CUpdater
  ====================*/
CUpdater::~CUpdater()
{
    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);

    for (UpdVector::iterator it(m_vUpdateFiles.begin()); it != m_vUpdateFiles.end(); it++)
    {
        m_fileHTTP.StopTransfer((!it->bPrimaryFail ? m_sPrimaryServer : m_sSecondaryServer) + it->sOS + _T("/") + it->sArch + _T("/") + it->sVersion + it->sURLFile + _T(".zip"));

        if (it->pFile != NULL)
        {
            it->pFile->Close();
            SAFE_DELETE(it->pFile);
        }
    }
}


/*====================
  CUpdateer::Initialize
  ====================*/
void    CUpdater::Initialize()
{
    m_sMasterServerURL = K2System.GetMasterServerAddress() + "/patcher/patcher.php";
}


/*====================
  CUpdater::GenerateIgnore
  ====================*/
void    CUpdater::GenerateIgnore(const tstring &sFilename)
{
    tsvector vsFileList;
    CFile *pFile;

    K2System.GetHiddenFileList(_T(":"), _T("*"), true, vsFileList, FileManager.GetTopModPath());

    pFile = FileManager.GetFile(sFilename, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_WRITE | FILE_TEXT);

    if (pFile == NULL)
    {
        Console << _T("[Ignore File Generation] Target file could not be opened for writing.") << newl;
        return;
    }

    pFile->WriteString(ConcatinateArgs(vsFileList, _T("\n")));

    pFile->Close();
    SAFE_DELETE(pFile);
}


/*====================
  CUpdater::ReadFileList
  ====================*/
void    CUpdater::ReadFileList(const tstring &sFile, sset &setFiles)
{
    CFile *pFile(FileManager.GetFile(sFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_TEXT));
    if (pFile == NULL)
        return;

    tstring sFilename;
    while (!pFile->IsEOF())
    {
        sFilename = LowerString(pFile->ReadLine());
        StripNewline(sFilename);

        if (!sFilename.empty())
            setFiles.insert(sFilename);
    }

    pFile->Close();
    K2_DELETE(pFile);
}


/*====================
  CUpdater::GenerateUpdate
  ====================*/
void    CUpdater::GenerateUpdate(bool bResourceFilesOnly)
{
    // since wer'e generating the update, don't examine any checksums.  Start
    // examining checksums again after we leave this method.
    CRestoreValue<bool>     cRestoreExamineChecksums(CArchive::ExamineChecksums);
    CArchive::ExamineChecksums = false;

    // Disable directory monitoring
    bool bWasDirectoryMonitoring(K2System.IsDirectoryMonitoring());
    if (bWasDirectoryMonitoring)
        K2System.StopDirectoryMonitoring();

    // Store version as an integer
    tsvector vsVersion(TokenizeString(K2System.GetVersionString(), '.'));

    tstring sVersion;
    if (vsVersion.size() < 4 || CompareNoCase(vsVersion[3], _T("0")) == 0)
        sVersion = ConcatinateArgs(vsVersion.begin(), vsVersion.end() - 1, _T("."));
    else
        sVersion = ConcatinateArgs(vsVersion, _T("."));

    uint uiVersion((AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]));

    // Get current live version from master server
    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
    {
        Console << _T("[Update Generation] Error spawning HTTP request.") << newl;
        return;
    }

    pRequest->SetTargetURL(m_sMasterServerURL);
    pRequest->AddVariable(_T("latest"), TSNULL);
    pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    pRequest->SendPostRequest();
    pRequest->Wait();

    if (!pRequest->WasSuccessful())
    {
        Console << _T("[Update Generation] Error retrieving checksum values.") << newl;
        Host.GetHTTPManager()->ReleaseRequest(pRequest);
        return;
    }

    CPHPData response(pRequest->GetResponse());
    Host.GetHTTPManager()->ReleaseRequest(pRequest);
    if (response.GetVar(_T("error")) != NULL)
    {
        Console << _T("[Update Generation] Error: ") << response.GetVar(_T("error"))->GetString(0) << newl;
        return;
    }

    tstring sPreviousVersion(response.GetString(_T("latest")));

    if (sPreviousVersion == K2System.GetVersionString())
    {
        Console << _T("[Update Generation] ") << _T("Version ") << sPreviousVersion << _T(" is already live! NO CHANGES ALLOWED!") << newl;
        return;
    }
    else
        Console << _T("[Update Generation] ") << _T("Previous version: ") << (sPreviousVersion.empty() ? _T("None") : sPreviousVersion) << _T(" New version: ") << K2System.GetVersionString() << newl;

    tsvector vsPreviousVersion(TokenizeString(sPreviousVersion, '.'));

    if (vsPreviousVersion.size() > 1 && (vsPreviousVersion.size() < 4 || CompareNoCase(vsPreviousVersion[3], _T("0")) == 0))
        sPreviousVersion = ConcatinateArgs(vsPreviousVersion.begin(), vsPreviousVersion.end() - 1, _T("."));
    else
        sPreviousVersion = ConcatinateArgs(vsPreviousVersion, _T("."));

    // Grab patch server's manifest
    SFileManifest cPreviousManifest;
    if (!sPreviousVersion.empty() && m_fileHTTP.Open(_T("http://ps1.hon.s2games.com/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sPreviousVersion + _T("/manifest.xml.zip"), FILE_BLOCK))
    {
        uint uiSize;
        const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));

        CMMapUnzip cManifestZip(pBuffer, uiSize);

        char *pManifestBuffer;
        uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));

        if (uiManifestSize > 0)
            XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &cPreviousManifest);

        K2_DELETE_ARRAY(pManifestBuffer);
    }
    
    // Initialize new manifest
    SFileManifest cNewManifest;
    cNewManifest.sVersion = K2System.GetVersionString();
    cNewManifest.sOS = K2System.GetBuildOSCodeString();
    cNewManifest.sArch = K2System.GetBuildArchString();

    FileManager.DeleteTree(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sVersion);

    // Get list of all files
    tsvector vsFileList;
    K2System.GetFileList(_T(":"), _T("*"), true, vsFileList, FileManager.GetTopModPath());

    // Get set of files to ignore
    sset setIgnore;
    ReadFileList(_T(":/patch_ignore.txt"), setIgnore);
    
#ifdef _WIN32
    ReadFileList(_T(":/patch_ignore_windows.txt"), setIgnore);
#elif defined(__APPLE__)
    ReadFileList(_T(":/patch_ignore_macos.txt"), setIgnore);
#elif defined(linux)
    ReadFileList(_T(":/patch_ignore_linux.txt"), setIgnore);
#endif

    ReadFileList(_T(":/patch_ignore_") + K2System.GetBuildOSString() + _T(".txt"), setIgnore);

    sset setResIgnore;
    ReadFileList(_T(":/resource_ignore.txt"), setResIgnore);
    
#ifdef _WIN32
    ReadFileList(_T(":/resource_ignore_windows.txt"), setResIgnore);
#elif defined(__APPLE__)
    ReadFileList(_T(":/resource_ignore_macos.txt"), setResIgnore);
#elif defined(linux)
    ReadFileList(_T(":/resource_ignore_linux.txt"), setResIgnore);
#endif

    ReadFileList(_T(":/resource_ignore_") + K2System.GetBuildOSString() + _T(".txt"), setResIgnore);

    sset setUnignore;
    ReadFileList(_T(":/patch_unignore.txt"), setUnignore);

    sset setFixupNewlines;
    ReadFileList(_T(":/fixup_newlines.txt"), setFixupNewlines);

    uint uiTotalSize(0);
    sset setArchiveList;
    tsvector_it it(vsFileList.begin());

    Console << _T("[Update Generation] Building resources0.s2z") << newl;
    map<tstring, SArchiveEntry*> mapArchives;

    // Generate resource archive, build changed file list, and build new file manifest
    while (it != vsFileList.end())
    {
        bool bUnignored(false);
        bool bIgnored(false);
        bool bResIgnored(false);
        bool bFixupNewlines(false);

        for (sset::iterator itUnignore(setUnignore.begin()); itUnignore != setUnignore.end(); itUnignore++)
        {
            if (EqualsWildcardNoCase(*itUnignore, *it))
            {
                bUnignored = true;
                break;
            }
        }

        if (!bUnignored)
        {
            for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); itIgnore++)
            {
                if (EqualsWildcardNoCase(*itIgnore, *it))
                {
                    bIgnored = true;
                    break;
                }
            }
        }

        if (bIgnored)
        {
            it = vsFileList.erase(it);
            continue;
        }

        for (sset::iterator itIgnore(setResIgnore.begin()); itIgnore != setResIgnore.end(); itIgnore++)
        {
            if (EqualsWildcardNoCase(*itIgnore, *it))
            {
                bResIgnored = true;
                break;
            }
        }

        for (sset::iterator itFixupNewlines(setFixupNewlines.begin()); itFixupNewlines != setFixupNewlines.end(); itFixupNewlines++)
        {
            if (EqualsWildcardNoCase(*itFixupNewlines, *it))
            {
                bFixupNewlines = true;
                break;
            }
        }

        if (CompareNoCase(Filename_GetExtension(*it), _T("s2z")) != 0)
        {
            CFile *pFile(FileManager.GetFile(*it, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY));
            if (pFile == NULL || !pFile->IsOpen())
            {
                K2_DELETE(pFile);
                continue;
            }

            tstring sModPath(it->substr(0, it->length() - 1));
            size_t zPos(sModPath.find(_T("/"), 2));

            if (!bResIgnored && zPos != tstring::npos)
            {
                sModPath = sModPath.substr(0, zPos);
                map<tstring, SArchiveEntry*>::iterator findit(mapArchives.find(sModPath + _T("/resources0.s2z")));

                if (findit == mapArchives.end())
                {
                    // Unregister archives so we can access them without a problem
                    FileManager.UnregisterArchive(_T("/resources0.s2z"));
                    K2SoundManager.StopStreamingImmediately();

                    mapArchives.insert(pair<tstring, SArchiveEntry*>(sModPath + _T("/resources0.s2z"), K2_NEW(ctx_FileSystem,  SArchiveEntry)(K2_NEW(ctx_FileSystem,  CArchive)(sModPath + _T("/resources0.s2z"), ARCHIVE_WRITE | ARCHIVE_TRUNCATE | ARCHIVE_MAX_COMPRESS))));

                    findit = mapArchives.find(sModPath + _T("/resources0.s2z"));

                    if (findit == mapArchives.end())
                    {
                        Console.Err << _T("Could not open ") << sModPath << _T("/resources0.s2z for writing") << newl;
                        pFile->Close();
                        SAFE_DELETE(pFile);
                        continue;
                    }

                    if (!findit->second->pArchive->IsOpen())
                        findit->second->pArchive->Open(sModPath + _T("/resources0.s2z"), ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS);

                    if (!findit->second->pArchive->IsOpen())
                    {
                        Console.Err << _T("Could not open ") << sModPath << _T("/resources0.s2z for writing") << newl;
                        pFile->Close();
                        SAFE_DELETE(pFile);
                        SAFE_DELETE(findit->second->pArchive);
                        SAFE_DELETE(findit->second);
                        mapArchives.erase(findit);
                        continue;
                    }
                }

                const tstring &sFilePath(it->substr(zPos + 1, it->length() - (zPos + 1)));
                tstring sChecksumFilePath(_T("/") + LowerString(sFilePath));

                CArchive &cArchive(*findit->second->pArchive);
                tsvector &vArchiveFileList(findit->second->vFileList);
                CChecksumTable &cChecksums(findit->second->cChecksums);
                CFileHandle hDestFile(sFilePath, FILE_WRITE | FILE_BINARY | FILE_COMPRESS, cArchive);
                if (!hDestFile.IsOpen())
                {
                    Console.Err << _T("Couldn't open file ") << sFilePath << _T(" in archive ") << sModPath << _T("/resources0.s2z") <<  newl;
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    it = vsFileList.erase(it);
                    continue;
                }

                uint uiSize(0);
                const char *pFileBuf(pFile->GetBuffer(uiSize));
                uint uiFileSize(pFile->GetBufferSize());
                const char *pUseFileBuf(pFileBuf);
                uint uiUseFileSize(uiFileSize);

                // convert '\r\n' to '\n' on Windows.
                char *pFileConvertedBuf(NULL);
                uint uiFileConvertedSize(0);
                if (bFixupNewlines)
                {
                    // convert UTF-16 to UTF-8
                    bool bLittleEndian(true);
                    if (IsUTF16(bLittleEndian, pFileBuf, uiFileSize))
                    {
                        if (uiFileSize < 4)
                        {
                            Console.Err << _T("Couldn't convert file ") << sFilePath << _T(" from UTF16 to UTF8:  Empty file!") << newl;
                            pFile->Close();
                            SAFE_DELETE(pFile);
                            it = vsFileList.erase(it);
                            continue;
                        }

                        short *pUTFBuf(((short *)pFileBuf) + 1);
                        uint uiUTFSize((uiFileSize-2) / 2);
                        int iConvertedSize(UTF16to8((short *)pUTFBuf, uiUTFSize, NULL));
                        bool bFailed(true);
                        if (iConvertedSize > 0)
                        {
                            pFileConvertedBuf = K2_NEW_ARRAY(ctx_FileSystem, char, iConvertedSize);
                            uiFileConvertedSize = iConvertedSize;
                            if (UTF16to8((short*)pUTFBuf, uiUTFSize, pFileConvertedBuf) == iConvertedSize)
                            {
                                bFailed = false;
                                uiFileConvertedSize = ConvertLineEndings(pFileConvertedBuf, pFileConvertedBuf, uiFileConvertedSize);
                                pUseFileBuf = pFileConvertedBuf;
                                uiUseFileSize = uiFileConvertedSize;
                                Console << _T("Converted ") << sFilePath << _T(" from UTF16 to UTF8") << newl;
                            }
                        }

                        if (bFailed)
                            Console.Err << _T("Could not convert file ") << sFilePath << _T(" from UTF16 to UTF8!") << newl;
                    }
                    else
                    {
                        // skip UTF-8 BOM
                        const char *pUTFBuf(pFileBuf);
                        uint uiUTFSize(uiFileSize);
                        if (IsUTF8(pUTFBuf, uiUTFSize))
                        {
                            if (uiUTFSize <= 3)
                            {
                                Console.Err << _T("Couldn't skip UTF8 BOM for file ") << sFilePath << _T(":  Empty file!") << newl;
                                pFile->Close();
                                SAFE_DELETE(pFile);
                                it = vsFileList.erase(it);
                                continue;
                            }

                            pUTFBuf += 3;
                            uiUTFSize -= 3;
                            Console << _T("Skipped BOM for UTF-8 file ") << sFilePath << newl;
                        }

                        pFileConvertedBuf = K2_NEW_ARRAY(ctx_FileSystem, char, uiUTFSize);
                        uiFileConvertedSize = ConvertLineEndings(pFileConvertedBuf, pUTFBuf, uiUTFSize);
                        pUseFileBuf = pFileConvertedBuf;
                        uiUseFileSize = uiFileConvertedSize;
                    }
                }

                if (hDestFile.Write(pUseFileBuf, uiUseFileSize) != uiUseFileSize)
                {
                    Console.Err << _T("Couldn't write file ") << sFilePath << _T(" in archive ") << sModPath << _T("/resources0.s2z") <<  newl;
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    it = vsFileList.erase(it);
                    continue;
                }

                byte yChecksum[CHECKSUM_SIZE];
                if (!CChecksumTable::ComputeChecksum(yChecksum, (const byte*)pUseFileBuf, uiUseFileSize))
                {
                    Console.Err << _T("Failed to compute checksum for file ") << it->substr(zPos + 1, it->length() - (zPos + 1)) << _T(" in archive ") << sModPath << _T("/resources0.s2z") <<  newl;
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    it = vsFileList.erase(it);
                    continue;
                }
                cChecksums.Add((uint)vArchiveFileList.size(), sChecksumFilePath, yChecksum);
                vArchiveFileList.push_back(sChecksumFilePath);

                if (setArchiveList.find(sModPath + _T("/resources0.s2z")) == setArchiveList.end())
                    setArchiveList.insert(sModPath + _T("/resources0.s2z"));

                SAFE_DELETE_ARRAY(pFileConvertedBuf);

                it = vsFileList.erase(it);
            }
            else
            {
                uint uiSize;
                const char *pBuffer(pFile->GetBuffer(uiSize));

                uint uiChecksum(FileManager.GetCRC32(pBuffer, uiSize));

                tstring sName(it->substr(2));

                ManifestEntryMap::iterator itPreviousFile(cPreviousManifest.mapManifestFiles.find(sName));

                if (itPreviousFile != cPreviousManifest.mapManifestFiles.end() &&
                    uiChecksum == itPreviousFile->second.uiChecksum &&
                    uiSize == itPreviousFile->second.uiSize)
                {
                    cNewManifest.mapManifestFiles[sName] = itPreviousFile->second;

                    pFile->Close();
                    SAFE_DELETE(pFile);

                    it = vsFileList.erase(it);
                    continue;
                }

                SManifestEntry cNewFile;
                cNewFile.uiChecksum = uiChecksum;
                cNewFile.uiSize = uiSize;
                cNewFile.uiVersion = uiVersion;
                cNewFile.uiZipSize = 0;

                cNewManifest.mapManifestFiles[sName] = cNewFile;

                uiTotalSize += uiSize;
                ++it;
            }

            pFile->Close();
            K2_DELETE(pFile);
        }
        else if (setArchiveList.find(*it) == setArchiveList.end() && !EqualsWildcardNoCase(_T(":/*/resources*.s2z"), *it))
        {
            setArchiveList.insert(*it);
            it = vsFileList.erase(it);
        }
        else
            it = vsFileList.erase(it);
    }

    for (map<tstring, SArchiveEntry*>::iterator closeit(mapArchives.begin()); closeit != mapArchives.end(); closeit++)
    {
        SArchiveEntry* pEntry(closeit->second);
        byte *pBuf(NULL);
        uint uiBufSize(0);

        CFileHandle hChecksumsFile(_T("/checksums"), FILE_WRITE | FILE_BINARY | FILE_COMPRESS, *pEntry->pArchive);
        if (!hChecksumsFile.IsOpen())
        {
            Console.Err << _T("Could not write checksums table for archive: ") << pEntry->pArchive->GetPath() << _T(" Could not open table for writing.") << newl;
            goto next;
        }

        std::sort(pEntry->vFileList.begin(), pEntry->vFileList.end());
        if (!pEntry->cChecksums.Serialize(pBuf, uiBufSize, pEntry->vFileList))
        {
            Console.Err << _T("Could not write checksums table for archive ") << pEntry->pArchive->GetPath() << _T(": Could not serialize table.") << newl;
            goto next;
        }

        if (hChecksumsFile.Write(pBuf, uiBufSize) != uiBufSize)
        {
            Console.Err << _T("Could not write checksums table for archive ") << pEntry->pArchive->GetPath() << _T(": Could not write table.") << newl;
            SAFE_DELETE_ARRAY(pBuf);
            goto next;
        }
        SAFE_DELETE_ARRAY(pBuf);

next:
        hChecksumsFile.Close();
        pEntry->pArchive->Close();
        SAFE_DELETE(pEntry->pArchive);
        SAFE_DELETE(pEntry);
    }

    mapArchives.clear();

    if (bResourceFilesOnly)
        return;

    sset_it itArchive(setArchiveList.begin());

    while (itArchive != setArchiveList.end())
    {
        CArchive cArchive;
        cArchive.Open(*itArchive);

        if (!cArchive.IsOpen())
            continue;

        vsFileList.push_back(*itArchive);

        tsvector vArchiveFiles;

        cArchive.GetFileList(vArchiveFiles);

        for (tsvector_it itFile(vArchiveFiles.begin()); itFile != vArchiveFiles.end(); ++itFile)
        {
            if ((*itFile).empty() || (*itFile)[itFile->length() - 1] == _T('/'))
                continue;

            char *pBuf(NULL);
            size_t zLen;

            zLen = cArchive.ReadFile(LowerString(Filename_GetPath(*itArchive) + *itFile), pBuf);

            if (pBuf == NULL)
                continue;

            uint uiChecksum(FileManager.GetCRC32(pBuf, zLen));

            SAFE_DELETE_ARRAY(pBuf);

            tstring sName(itArchive->substr(2) + _T("/") + *itFile);

            ManifestEntryMap::iterator itPreviousFile(cPreviousManifest.mapManifestFiles.find(sName));

            if (itPreviousFile != cPreviousManifest.mapManifestFiles.end() &&
                uiChecksum == itPreviousFile->second.uiChecksum &&
                uint(zLen) == itPreviousFile->second.uiSize)
            {
                cNewManifest.mapManifestFiles[sName] = itPreviousFile->second;
                continue;
            }

            SManifestEntry cNewFile;
            cNewFile.uiChecksum = uiChecksum;
            cNewFile.uiSize = uint(zLen);
            cNewFile.uiVersion = uiVersion;
            cNewFile.uiZipSize = 0;

            cNewManifest.mapManifestFiles[sName] = cNewFile;

            if (cArchive.IsOpen())
            {
                uiTotalSize += INT_SIZE(zLen);
            }
        }

        cArchive.Close();
        itArchive++;
    }

    class CGenerateUpdateFunctions : public CLoadJob<tsvector>::IFunctions
    {
    protected:
        const tstring &m_sVersion;

        SFileManifest &m_cPreviousManifest;
        SFileManifest &m_cNewManifest;

    public:
        CGenerateUpdateFunctions(const tstring &sVersion, SFileManifest &cPreviousManifest, SFileManifest &cNewManifest) : 
        m_sVersion(sVersion),
        m_cPreviousManifest(cPreviousManifest),
        m_cNewManifest(cNewManifest)
        {
        }
        virtual ~CGenerateUpdateFunctions() {}

        float   Frame(tsvector_it &it, float f) const       
        {
            SetTitle(_T("Generating Update (") + XtoA(f * 100, FMT_NOPREFIX, 0, 2) + _T("%)"));
            SetProgress(f);

            cURL_Frame();

            return 0.0f;
        }

        float   PostFrame(tsvector_it &it, float f) const   
        {
            // If we're not working with an archive
            if (CompareNoCase(Filename_GetExtension(*it), _T("s2z")) != 0)
            {
                const tstring &sName(it->substr(2));

                ManifestEntryMap::iterator itNewFile(m_cNewManifest.mapManifestFiles.find(sName));

                if (itNewFile == m_cNewManifest.mapManifestFiles.end())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                    ++it;
                    return 0.0f;
                }

                ManifestEntryMap::iterator itPreviousFile(m_cPreviousManifest.mapManifestFiles.find(sName));

                if (itPreviousFile != m_cPreviousManifest.mapManifestFiles.end() &&
                    itNewFile->second.uiSize == itPreviousFile->second.uiSize &&
                    itNewFile->second.uiChecksum == itPreviousFile->second.uiChecksum &&
                    itNewFile->second.uiVersion == itPreviousFile->second.uiVersion)
                {
                    ++it;
                    return 0.0f;
                }

                Console << _T("[Update Generation] Copying file ") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;

                CFile *pFile(FileManager.GetFile(*it, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY));

                if (pFile == NULL || !pFile->IsOpen())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                    K2System.Sleep(5000);
                    return 0.0f;
                }


                CFile *pOutFile(FileManager.GetFile(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + m_sVersion + it->substr(1), FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_WRITE | FILE_BINARY));
                if (pOutFile == NULL || !pOutFile->IsOpen())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not copy file. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                    K2System.Sleep(5000);
                    return 0.0f;
                }

                uint uiSize;
                const char *pBuffer(pFile->GetBuffer(uiSize));

                pOutFile->Write(pBuffer, uiSize);

                pFile->Close();
                pOutFile->Close();
                SAFE_DELETE(pOutFile);

                ++it;
                SAFE_DELETE(pFile);
            }
            else
            {
                // We are working with a .s2z, copy each file in the archive
                CArchive cArchive;
                cArchive.Open(*it);

                if (!cArchive.IsOpen())
                {
                    Console.Err << _T("[Update Generation] Failure on archive ") << it->substr(2) << _T(", could not open archive for reading.") << newl;
                    return 0.0f;
                }

                tsvector vArchiveFiles;
                cArchive.GetFileList(vArchiveFiles);
                tsvector_it itFile(vArchiveFiles.begin());

                while (itFile != vArchiveFiles.end())
                {
                    const tstring &sName(it->substr(2) + _T("/") + *itFile);

                    ManifestEntryMap::iterator itNewFile(m_cNewManifest.mapManifestFiles.find(sName));

                    if (itNewFile == m_cNewManifest.mapManifestFiles.end())
                    {
                        Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                        ++itFile;
                        continue;
                    }

                    ManifestEntryMap::iterator itPreviousFile(m_cPreviousManifest.mapManifestFiles.find(sName));

                    if (itPreviousFile != m_cPreviousManifest.mapManifestFiles.end() &&
                        itNewFile->second.uiSize == itPreviousFile->second.uiSize &&
                        itNewFile->second.uiChecksum == itPreviousFile->second.uiChecksum &&
                        itNewFile->second.uiVersion == itPreviousFile->second.uiVersion)
                    {
                        ++itFile;
                        continue;
                    }

                    Console << _T("[Update Generation] Copying file ") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;

                    CFile *pOutFile(FileManager.GetFile(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + m_sVersion + it->substr(1) + _T("/") + *itFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_WRITE | FILE_BINARY));
                    if (pOutFile == NULL || !pOutFile->IsOpen())
                    {
                        Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not copy file. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                        K2System.Sleep(5000);
                        return 0.0f;
                    }

                    char *pBuffer(NULL);
                    uint uiSize(cArchive.ReadFile(LowerString(Filename_GetPath(_T(":/") + it->substr(2))) + _T("/") + *itFile, pBuffer));
                    if (pBuffer == NULL)
                    {
                        Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                        continue;
                    }

                    pOutFile->Write(pBuffer, uiSize);

                    SAFE_DELETE_ARRAY(pBuffer);

                    pOutFile->Close();
                    SAFE_DELETE(pOutFile);
                    
                    ++itFile;
                }
                
                cArchive.Close();
                ++it;
            }

            return 0;
        }
    };
    CGenerateUpdateFunctions fnGenerateUpdate(sVersion, cPreviousManifest, cNewManifest);
    CLoadJob<tsvector>  job(vsFileList, &fnGenerateUpdate, LOADING_DISPLAY_NONE);
    job.Execute(uiTotalSize);

    //
    // Generate manifest
    //

    CXMLDoc xmlManifest(XML_ENCODE_UTF8);

    xmlManifest.NewNode("manifest");

        xmlManifest.AddProperty("version", K2System.GetVersionString());
        xmlManifest.AddProperty("os", K2System.GetBuildOSCodeString());
        xmlManifest.AddProperty("arch", K2System.GetBuildArchString());

        for (ManifestEntryMap::iterator it(cNewManifest.mapManifestFiles.begin()); it != cNewManifest.mapManifestFiles.end(); ++it)
        {
            xmlManifest.NewNode("file");
                xmlManifest.AddProperty("path", it->first);
                xmlManifest.AddProperty("size", XtoA(it->second.uiSize));
                xmlManifest.AddProperty("checksum", CtoA(it->second.uiChecksum));
                xmlManifest.AddProperty("version", VtoA(it->second.uiVersion));
                xmlManifest.AddProperty("zipsize", XtoA(it->second.uiZipSize));
            xmlManifest.EndNode();
        }

    xmlManifest.EndNode();

    xmlManifest.WriteFile(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sVersion + _T("/manifest.xml"), false);

    //
    // Generate local manifest
    //

    CXMLDoc xmlManifest2(XML_ENCODE_UTF8);

    xmlManifest2.NewNode("manifest");

        xmlManifest2.AddProperty("version", K2System.GetVersionString());
        xmlManifest2.AddProperty("os", K2System.GetBuildOSCodeString());
        xmlManifest2.AddProperty("arch", K2System.GetBuildArchString());

        for (ManifestEntryMap::iterator it(cNewManifest.mapManifestFiles.begin()); it != cNewManifest.mapManifestFiles.end(); ++it)
        {
            xmlManifest2.NewNode("file");
                xmlManifest2.AddProperty("path", it->first);
                xmlManifest2.AddProperty("size", XtoA(it->second.uiSize));
                xmlManifest2.AddProperty("checksum", CtoA(it->second.uiChecksum));
                xmlManifest2.AddProperty("version", VtoA(it->second.uiVersion));
                //xmlManifest2.AddProperty("zipsize", XtoA(it->second.uiZipSize));
            xmlManifest2.EndNode();
        }

    xmlManifest2.EndNode();

    xmlManifest2.WriteFile(_T(":/manifest.xml"), false);

    Console << _T("[Update Generation] Update generation completed.") << newl;

    // Re-enable directory monitoring if it was active before
    if (bWasDirectoryMonitoring)
        K2System.StartDirectoryMonitoring();
}


/*====================
  CUpdater::UploadUpdate
  ====================*/
void    CUpdater::UploadUpdate(const tstring &sAddress, const tstring &sUser, const tstring &sPass)
{
    tsvector vsFileList;
    SFileManifest cPreviousManifest;
    SFileManifest cNewManifest;

    bool bWasDirectoryMonitoring(K2System.IsDirectoryMonitoring());
    if (bWasDirectoryMonitoring)
        K2System.StopDirectoryMonitoring();

    tsvector vsVersion(TokenizeString(K2System.GetVersionString(), '.'));

    tstring sVersion;
    if (vsVersion.size() < 4 || CompareNoCase(vsVersion[3], _T("0")) == 0)
        sVersion = ConcatinateArgs(vsVersion.begin(), vsVersion.end() - 1, _T("."));
    else
        sVersion = ConcatinateArgs(vsVersion, _T("."));

    uint uiVersion((AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]));

    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
    {
        Console << _T("[Update Generation] Error spawning HTTP request.") << newl;
        Host.GetHTTPManager()->ReleaseRequest(pRequest);
        return;
    }

    pRequest->SetTargetURL(m_sMasterServerURL);
    pRequest->AddVariable(_T("latest"), TSNULL);
    pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    pRequest->SendPostRequest();
    pRequest->Wait();

    if (!pRequest->WasSuccessful())
    {
        Console << _T("[Update Generation] Error retrieving checksum values.") << newl;
        Host.GetHTTPManager()->ReleaseRequest(pRequest);
        return;
    }

    CPHPData response(pRequest->GetResponse());
    Host.GetHTTPManager()->ReleaseRequest(pRequest);
    if (response.GetVar(_T("error")) != NULL)
    {
        Console << _T("[Update Generation] Error: ") << response.GetVar(_T("error"))->GetString(0) << newl;
        return;
    }

    tstring sPreviousVersion(response.GetString(_T("latest")));

    if (sPreviousVersion == K2System.GetVersionString())
    {
        Console << _T("[Update Generation] ") << _T("Version ") << sPreviousVersion << _T(" is already live! NO CHANGES ALLOWED!") << newl;
        return;
    }
    else
        Console << _T("[Update Generation] ") << _T("Previous version: ") << (sPreviousVersion.empty() ? _T("None") : sPreviousVersion) << _T(" New version: ") << K2System.GetVersionString() << newl;

    tsvector vsPreviousVersion(TokenizeString(sPreviousVersion, '.'));

    if (vsPreviousVersion.size() > 1 && (vsPreviousVersion.size() < 4 || CompareNoCase(vsPreviousVersion[3], _T("0")) == 0))
        sPreviousVersion = ConcatinateArgs(vsPreviousVersion.begin(), vsPreviousVersion.end() - 1, _T("."));
    else
        sPreviousVersion = ConcatinateArgs(vsPreviousVersion, _T("."));

    // Grab patch server's manifest
    if (!sPreviousVersion.empty() && m_fileHTTP.Open(_T("http://patch1.hon.s2games.com/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sPreviousVersion + _T("/manifest.xml.zip"), FILE_BLOCK))
    {
        uint uiSize;
        const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));

        CMMapUnzip cManifestZip(pBuffer, uiSize);

        char *pManifestBuffer;
        uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));

        if (uiManifestSize > 0)
        {
            XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &cPreviousManifest);

            K2_DELETE_ARRAY(pManifestBuffer);
        }
    }

    // Grab new manifest
    XMLManager.Process(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sVersion + _T("/manifest.xml"), _T("manifest"), &cNewManifest);

    // Build new filelist
    for (ManifestEntryMap::iterator it(cNewManifest.mapManifestFiles.begin()); it != cNewManifest.mapManifestFiles.end(); ++it)
    {
        if (it->second.uiVersion != uiVersion)
            continue;

        vsFileList.push_back(it->first);
    }

    class CGenerateUpdateFunctions : public CLoadJob<tsvector>::IFunctions
    {
    protected:
        const tstring &m_sVersion;
        SFileManifest &m_cPreviousManifest;
        SFileManifest &m_cNewManifest;
        const tstring &m_sAddress;
        const tstring &m_sUser;
        const tstring &m_sPass;
        
    public:
        CGenerateUpdateFunctions(const tstring &sVersion, SFileManifest &cPreviousManifest, SFileManifest &cNewManifest, const tstring &sAddress, const tstring &sUser, const tstring &sPass) : 
        m_sVersion(sVersion),
        m_cPreviousManifest(cPreviousManifest),
        m_cNewManifest(cNewManifest),
        m_sAddress(sAddress),
        m_sUser(sUser),
        m_sPass(sPass)
        {
        }
        virtual ~CGenerateUpdateFunctions() {}

        float   Frame(tsvector_it &it, float f) const       
        {
            SetTitle(_T("Generating Update (") + XtoA(f * 100, FMT_NOPREFIX, 0, 2) + _T("%)"));
            SetProgress(f);

            cURL_Frame();

            return 0.0f;
        }

        float   PostFrame(tsvector_it &it, float f) const   
        {
            uint uiSize(0);
            CFileHTTP fileHTTP;

            const tstring &sName(*it);

            ManifestEntryMap::iterator itNewFile(m_cNewManifest.mapManifestFiles.find(sName));
            if (itNewFile == m_cNewManifest.mapManifestFiles.end())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                ++it;
                return 0.0f;
            }

            tstring sFilename(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + m_sVersion + _T("/") + *it);

            if (!FileManager.Exists(sFilename))
            {
                ManifestEntryMap::iterator itOldFile(m_cPreviousManifest.mapManifestFiles.find(sName));
                if (itOldFile != m_cPreviousManifest.mapManifestFiles.end())
                {
                    Console << _T("[Update Generation] Restoring changed file ") << sName << newl;
                    itNewFile->second = itOldFile->second;
                }
                else
                {
                    Console << _T("[Update Generation] Ignoring new file ") << sName << newl;
                    m_cNewManifest.mapManifestFiles.erase(itNewFile);
                }

                ++it;
                return 0.0f;
            }

            Console << _T("[Update Generation] Uploading file ") << sName << _T(", size: ") << itNewFile->second.uiSize << newl;

            CFile *pFile(FileManager.GetFile(sFilename, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY));

            if (pFile == NULL || !pFile->IsOpen())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading.") << newl;
                K2System.Sleep(5000);
                return 0.0f;
            }

            CArchive archive;
            CFile *pArchiveFile(NULL);
            tstring sTempFile(_T(":/temp.zip"));
            const char *pBuffer(pFile->GetBuffer(uiSize));
            if (pBuffer == NULL)
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading.") << newl;
                K2System.Sleep(5000);
                return 0.0f;
            }

            uint uiChecksum(FileManager.GetCRC32(pBuffer, uiSize));

            if (uiChecksum != itNewFile->second.uiChecksum)
            {
                itNewFile->second.uiChecksum = uiChecksum;
                Console << _T("[Update Generation] Updated checksum on file") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
            }

            if (archive.Open(sTempFile, ARCHIVE_WRITE | ARCHIVE_TRUNCATE | ARCHIVE_MAX_COMPRESS) && archive.WriteFile(LowerString(Filename_StripPath(*it)), pBuffer, uiSize))
            {
                archive.Close();
                pArchiveFile = FileManager.GetFile(sTempFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);
            }
            else
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not write to archive.") << newl;

                archive.Close();
                pFile->Close();
                SAFE_DELETE(pFile);

                FileManager.Delete(sTempFile);
            }
            
            if (pArchiveFile != NULL)
            {
                if (!pArchiveFile->IsOpen())
                {
                    pArchiveFile->Close();
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    SAFE_DELETE(pArchiveFile);

                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open compressed file.") << newl;

                    FileManager.Delete(sTempFile);

                    return 0.0f;
                }

                uint uiUploadFlags(FILE_HTTP_UPLOAD | FILE_BLOCK);
                if (upd_ftpActive)
                    uiUploadFlags |= FILE_FTP_ACTIVE;

                fileHTTP.SetFileTarget(pArchiveFile);
                fileHTTP.Open(_T("ftp://") + m_sUser + _T(":") + m_sPass + _T("@") + m_sAddress + _T("/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + m_sVersion + _T("/") + *it + _T(".zip"), uiUploadFlags);

                itNewFile->second.uiZipSize = pArchiveFile->GetBufferSize();

                pArchiveFile->Close();
                SAFE_DELETE(pArchiveFile);

                FileManager.Delete(sTempFile);

                if (fileHTTP.ErrorEncountered())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not upload compressed file.") << newl;
                    K2System.Sleep(5000);
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    return 0.0f;
                }

                pFile->Close();
                SAFE_DELETE(pFile);

                if (fileHTTP.ErrorEncountered())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not upload uncompressed file.") << newl;
                    return 0.0f;
                }
                
                //Console << _T("[Update Generation] Upload on file ") << sName << _T(" complete.") << newl;

                ++it;
            }
            else
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", compressed file could not be created.") << newl;
                archive.Close();
                pFile->Close();
                SAFE_DELETE(pFile);
                return 0.0f;
            }

            SAFE_DELETE(pFile);
            return uiSize;
        }
    };
    CGenerateUpdateFunctions fnGenerateUpdate(sVersion, cPreviousManifest, cNewManifest, sAddress, sUser, sPass);
    CLoadJob<tsvector>  job(vsFileList, &fnGenerateUpdate, LOADING_DISPLAY_NONE);
    job.Execute(0.0f);

    //
    // Upload manifest
    //

    while (true)
    {
        CXMLDoc xmlManifest(XML_ENCODE_UTF8);

        xmlManifest.NewNode("manifest");

            xmlManifest.AddProperty("version", K2System.GetVersionString());
            xmlManifest.AddProperty("os", K2System.GetBuildOSCodeString());
            xmlManifest.AddProperty("arch", K2System.GetBuildArchString());

            for (ManifestEntryMap::iterator it(cNewManifest.mapManifestFiles.begin()); it != cNewManifest.mapManifestFiles.end(); ++it)
            {
                xmlManifest.NewNode("file");
                    xmlManifest.AddProperty("path", it->first);
                    xmlManifest.AddProperty("size", XtoA(it->second.uiSize));
                    xmlManifest.AddProperty("checksum", CtoA(it->second.uiChecksum));
                    xmlManifest.AddProperty("version", VtoA(it->second.uiVersion));
                    xmlManifest.AddProperty("zipsize", XtoA(it->second.uiZipSize));
                xmlManifest.EndNode();
            }

        xmlManifest.EndNode();

        CArchive archive;
        CFile *pArchiveFile(NULL);

        if (archive.Open(_T(":/manifest.xml.zip"), ARCHIVE_WRITE | ARCHIVE_MAX_COMPRESS) && archive.WriteFile(_T("manifest.xml"), xmlManifest.GetBuffer()->Get(), xmlManifest.GetBuffer()->GetLength()))
        {
            archive.Close();
            pArchiveFile = FileManager.GetFile(_T(":/manifest.xml.zip"), FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);
        }
        else
        {
            archive.Close();
            FileManager.Delete(_T(":/manifest.xml.zip"));
        }

        if (pArchiveFile != NULL)
        {
            CFileHTTP fileHTTP;

            if (!pArchiveFile->IsOpen())
            {
                pArchiveFile->Close();
                SAFE_DELETE(pArchiveFile);

                Console.Err << _T("[Update Generation] Failure on file manifest, could not open compressed file.") << newl;

                FileManager.Delete(_T(":/manifest.xml.zip"));
                continue;
            }

            uint uiUploadFlags(FILE_HTTP_UPLOAD | FILE_BLOCK);
            if (upd_ftpActive)
                uiUploadFlags |= FILE_FTP_ACTIVE;

            fileHTTP.SetFileTarget(pArchiveFile);
            fileHTTP.Open(_T("ftp://") + sUser + _T(":") + sPass + _T("@") + sAddress + _T("/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sVersion + _T("/manifest.xml.zip"), uiUploadFlags);

            pArchiveFile->Close();
            SAFE_DELETE(pArchiveFile);

            FileManager.Delete(_T(":/manifest.xml.zip"));

            if (fileHTTP.ErrorEncountered())
            {
                Console.Err << _T("[Update Generation] Failure on file manifest, could not upload compressed file.") << newl;
                K2System.Sleep(1000);
                continue;
            }

            Console << _T("[Update Generation] Update on file manifest complete.") << newl;
        }
        else
        {
            archive.Close();
            FileManager.Delete(_T(":/manifest.xml.zip"));
            Console.Err << _T("[Update Generation] Failure on file manifest, could not compress file.") << newl;
            continue;
        }

        break;
    }

    Console << _T("[Update Generation] Upload completed.") << newl;

    if (bWasDirectoryMonitoring)
        K2System.StartDirectoryMonitoring();
}


/*====================
  CUpdater::UploadPrevious
  ====================*/
void    CUpdater::UploadPrevious(const tstring &sAddress, const tstring &sUser, const tstring &sPass)
{
    tsvector vsFileList;
    SFileManifest cCurrentManifest;
    ZipMap mapResFiles;

    bool bWasDirectoryMonitoring(K2System.IsDirectoryMonitoring());
    if (bWasDirectoryMonitoring)
        K2System.StopDirectoryMonitoring();

    tsvector vsVersion(TokenizeString(K2System.GetVersionString(), '.'));

    tstring sVersion;
    if (vsVersion.size() < 4 || CompareNoCase(vsVersion[3], _T("0")) == 0)
        sVersion = ConcatinateArgs(vsVersion.begin(), vsVersion.end() - 1, _T("."));
    else
        sVersion = ConcatinateArgs(vsVersion, _T("."));

    uint uiVersion((AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]));

    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
    {
        Console << _T("[Update Generation] Error spawing HTTP request.") << newl;
        return;
    }

    pRequest->SetTargetURL(m_sMasterServerURL);
    pRequest->AddVariable(_T("latest"), _T(""));
    pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    pRequest->SendPostRequest();
    pRequest->Wait();

    if (!pRequest->WasSuccessful())
    {
        Host.GetHTTPManager()->ReleaseRequest(pRequest);
        Console << _T("[Update Generation] Error retrieving checksum values.") << newl;
        return;
    }

    CPHPData response(pRequest->GetResponse());
    Host.GetHTTPManager()->ReleaseRequest(pRequest);

    if (response.GetVar(_T("error")) != NULL)
    {
        Console << _T("[Update Generation] Error: ") << response.GetVar(_T("error"))->GetString(0) << newl;
        return;
    }

    tstring sCurrentVersion(response.GetString(_T("latest")));

    if (sCurrentVersion != K2System.GetVersionString())
    {
        Console << _T("[Update Generation] ") << _T("Version ") << sCurrentVersion << _T(" does not match current version!") << newl;
        return;
    }
    else
        Console << _T("[Update Generation] ") << _T("Current version: ") << (sCurrentVersion.empty() ? _T("None") : sCurrentVersion) << newl;

    sCurrentVersion = K2_Version(sCurrentVersion);

    // Grab server's manifest
    if (!sCurrentVersion.empty() && m_fileHTTP.Open(_T("http://patch1.hon.s2games.com/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sCurrentVersion + _T("/manifest.xml.zip"), FILE_BLOCK))
    {
        uint uiSize;
        const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));

        CMMapUnzip cManifestZip(pBuffer, uiSize);

        char *pManifestBuffer;
        uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));

        if (uiManifestSize > 0)
        {
            XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &cCurrentManifest);

            K2_DELETE_ARRAY(pManifestBuffer);
        }
    }

    // Build filelist
    for (ManifestEntryMap::iterator it(cCurrentManifest.mapManifestFiles.begin()); it != cCurrentManifest.mapManifestFiles.end(); ++it)
    {
        if (it->second.uiVersion == uiVersion)
            continue;

        vsFileList.push_back(it->first);
    }

    for (tsvector_it it(vsFileList.begin()); it != vsFileList.end(); )
    {
        size_t zPos(it->find(_T(".s2z")));
        if (zPos != tstring::npos)
        {
            // We're working with an archive file
            ZipMap::iterator itFindRes;
            int iFlags(ARCHIVE_READ);

            itFindRes = mapResFiles.find(it->substr(0, zPos + 4));

            if (itFindRes == mapResFiles.end())
            {
                mapResFiles.insert(pair<tstring, CArchive>(it->substr(0, zPos + 4), CArchive()));
                itFindRes = mapResFiles.find(it->substr(0, zPos + 4));

                if (itFindRes == mapResFiles.end())
                {
                    ++it;
                    continue;
                }

                itFindRes->second.Open(_T(":/") + it->substr(0, zPos + 4), iFlags);

                if (!itFindRes->second.IsOpen())
                    itFindRes->second.Open(_T(":/") + it->substr(0, zPos + 4), iFlags & ~ARCHIVE_APPEND);
            }

            if (itFindRes->second.IsOpen())
            {
                const tstring &sName(*it);

                ManifestEntryMap::iterator itNewFile(cCurrentManifest.mapManifestFiles.find(sName));

                if (itNewFile == cCurrentManifest.mapManifestFiles.end())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                    continue;
                }

                tstring sFileVersion(VtoA2(itNewFile->second.uiVersion));

                Console << _T("[Update Generation] Copying file ") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;

                CFile *pOutFile(FileManager.GetFile(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sFileVersion + _T("/") + *it, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_WRITE | FILE_BINARY));
                if (pOutFile == NULL || !pOutFile->IsOpen())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not copy file. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                    K2System.Sleep(5000);
                    continue;
                }

                char *pBuffer(NULL);
                uint uiSize(itFindRes->second.ReadFile(itFindRes->second.GetPathToArchive() + _T("/") + it->substr(zPos + 5), pBuffer));
                if (pBuffer == NULL)
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                    ++it;
                    continue;
                }

                pOutFile->Write(pBuffer, uiSize);

                SAFE_DELETE_ARRAY(pBuffer);

                pOutFile->Close();
                SAFE_DELETE(pOutFile);
            }

            ++it;
        }
        else
        {
            const tstring &sName(*it);

            ManifestEntryMap::iterator itNewFile(cCurrentManifest.mapManifestFiles.find(sName));

            if (itNewFile == cCurrentManifest.mapManifestFiles.end())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                ++it;
                continue;
            }

            tstring sFileVersion(VtoA2(itNewFile->second.uiVersion));

            Console << _T("[Update Generation] Copying file ") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;

            CFile *pFile(FileManager.GetFile(_T(":/") + *it, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY));

            if (pFile == NULL || !pFile->IsOpen())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                K2System.Sleep(5000);
                continue;
            }

            CFile *pOutFile(FileManager.GetFile(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sFileVersion + _T("/") + *it, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_WRITE | FILE_BINARY));
            if (pOutFile == NULL || !pOutFile->IsOpen())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not copy file. Checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
                K2System.Sleep(5000);
                continue;
            }

            uint uiSize;
            const char *pBuffer(pFile->GetBuffer(uiSize));

            pOutFile->Write(pBuffer, uiSize);

            pFile->Close();
            pOutFile->Close();
            SAFE_DELETE(pOutFile);

            ++it;
            SAFE_DELETE(pFile);
        }
    }

    for (ZipMap::iterator itMap(mapResFiles.begin()); itMap != mapResFiles.end(); ++itMap)
        itMap->second.Close();

    mapResFiles.clear();

    class CGenerateUpdateFunctions : public CLoadJob<tsvector>::IFunctions
    {
    protected:
        const tstring &m_sVersion;
        SFileManifest &m_cCurrentManifest;
        const tstring &m_sAddress;
        const tstring &m_sUser;
        const tstring &m_sPass;
        
    public:
        CGenerateUpdateFunctions(const tstring &sVersion, SFileManifest &cCurrentManifest, const tstring &sAddress, const tstring &sUser, const tstring &sPass) : 
        m_sVersion(sVersion),
        m_cCurrentManifest(cCurrentManifest),
        m_sAddress(sAddress),
        m_sUser(sUser),
        m_sPass(sPass)
        {
        }
        virtual ~CGenerateUpdateFunctions() {}

        float   Frame(tsvector_it &it, float f) const       
        {
            SetTitle(_T("Generating Update (") + XtoA(f * 100, FMT_NOPREFIX, 0, 2) + _T("%)"));
            SetProgress(f);

            cURL_Frame();

            return 0.0f;
        }

        float   PostFrame(tsvector_it &it, float f) const   
        {
            uint uiSize(0);
            CFileHTTP fileHTTP;

            const tstring &sName(*it);

            ManifestEntryMap::iterator itNewFile(m_cCurrentManifest.mapManifestFiles.find(sName));
            if (itNewFile == m_cCurrentManifest.mapManifestFiles.end())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not find manifest entry for file.") << newl;
                ++it;
                return 0.0f;
            }

            tstring sFileVersion(VtoA2(itNewFile->second.uiVersion));

            tstring sFilename(_T(":/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sFileVersion + _T("/") + *it);

            if (!FileManager.Exists(sFilename))
            {
                ManifestEntryMap::iterator itOldFile(m_cCurrentManifest.mapManifestFiles.find(sName));
                if (itOldFile != m_cCurrentManifest.mapManifestFiles.end())
                {
                    Console << _T("[Update Generation] Restoring changed file ") << sName << newl;
                    itNewFile->second = itOldFile->second;
                }
                else
                {
                    Console << _T("[Update Generation] Ignoring new file ") << sName << newl;
                    m_cCurrentManifest.mapManifestFiles.erase(itNewFile);
                }

                ++it;
                return 0.0f;
            }

            Console << _T("[Update Generation] Uploading file ") << sName << _T(", size: ") << itNewFile->second.uiSize << newl;

            CFile *pFile(FileManager.GetFile(sFilename, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY));

            if (pFile == NULL || !pFile->IsOpen())
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading.") << newl;
                K2System.Sleep(5000);
                return 0.0f;
            }

            CArchive archive;
            CFile *pArchiveFile(NULL);
            tstring sTempFile(_T(":/temp.zip"));
            const char *pBuffer(pFile->GetBuffer(uiSize));
            if (pBuffer == NULL)
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open file for reading.") << newl;
                K2System.Sleep(5000);
                return 0.0f;
            }

            uint uiChecksum(FileManager.GetCRC32(pBuffer, uiSize));

            if (uiChecksum != itNewFile->second.uiChecksum)
            {
                itNewFile->second.uiChecksum = uiChecksum;
                Console << _T("[Update Generation] Updated checksum on file") << sName << _T(", checksum: ") << CtoA(itNewFile->second.uiChecksum) << newl;
            }

            if (archive.Open(sTempFile, ARCHIVE_WRITE | ARCHIVE_TRUNCATE | ARCHIVE_MAX_COMPRESS) && archive.WriteFile(LowerString(Filename_StripPath(*it)), pBuffer, uiSize))
            {
                archive.Close();
                pArchiveFile = FileManager.GetFile(sTempFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);
            }
            else
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not write to archive.") << newl;

                archive.Close();
                pFile->Close();
                SAFE_DELETE(pFile);

                FileManager.Delete(sTempFile);
            }
            
            if (pArchiveFile != NULL)
            {
                if (!pArchiveFile->IsOpen())
                {
                    pArchiveFile->Close();
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    SAFE_DELETE(pArchiveFile);

                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not open compressed file.") << newl;

                    FileManager.Delete(sTempFile);

                    return 0.0f;
                }

                uint uiUploadFlags(FILE_HTTP_UPLOAD | FILE_BLOCK);
                if (upd_ftpActive)
                    uiUploadFlags |= FILE_FTP_ACTIVE;

                fileHTTP.SetFileTarget(pArchiveFile);
                fileHTTP.Open(_T("ftp://") + m_sUser + _T(":") + m_sPass + _T("@") + m_sAddress + _T("/") + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sFileVersion + _T("/") + *it + _T(".zip"), uiUploadFlags);

                itNewFile->second.uiZipSize = pArchiveFile->GetBufferSize();

                pArchiveFile->Close();
                SAFE_DELETE(pArchiveFile);

                FileManager.Delete(sTempFile);

                if (fileHTTP.ErrorEncountered())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not upload compressed file.") << newl;
                    K2System.Sleep(5000);
                    pFile->Close();
                    SAFE_DELETE(pFile);
                    return 0.0f;
                }

                pFile->Close();
                SAFE_DELETE(pFile);

                if (fileHTTP.ErrorEncountered())
                {
                    Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", could not upload uncompressed file.") << newl;
                    return 0.0f;
                }
                
                //Console << _T("[Update Generation] Upload on file ") << sName << _T(" complete.") << newl;

                ++it;
            }
            else
            {
                Console.Err << _T("[Update Generation] Failure on file ") << sName << _T(", compressed file could not be created.") << newl;
                archive.Close();
                pFile->Close();
                SAFE_DELETE(pFile);
                return 0.0f;
            }

            SAFE_DELETE(pFile);
            return uiSize;
        }
    };
    CGenerateUpdateFunctions fnGenerateUpdate(sVersion, cCurrentManifest, sAddress, sUser, sPass);
    CLoadJob<tsvector>  job2(vsFileList, &fnGenerateUpdate, LOADING_DISPLAY_NONE);
    job2.Execute(0.0f);

    Console << _T("[Update Generation] Upload completed.") << newl;

    if (bWasDirectoryMonitoring)
        K2System.StartDirectoryMonitoring();
}


/*====================
  CUpdater::UpdateDB
  ====================*/
void    CUpdater::UpdateDB(const tstring &sUsername, const tstring &sPassword)
{
    tsvector vsVersion(TokenizeString(K2System.GetVersionString(), '.'));

    tstring sVersion;
    if (vsVersion.size() < 4 || CompareNoCase(vsVersion[3], _T("0")) == 0)
        sVersion = ConcatinateArgs(vsVersion.begin(), vsVersion.end() - 1, _T("."));
    else
        sVersion = ConcatinateArgs(vsVersion, _T("."));

    //uint uiVersion((AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]));

    CHTTPRequest *pRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pRequest == NULL)
    {
        return;
    }

    pRequest->SetTargetURL(_T("masterserver.hon.s2games.com/patcher/auto_patcher.php"));
    
    pRequest->AddVariable(_T("username"), sUsername);
    pRequest->AddVariable(_T("password"), sPassword);

    pRequest->AddVariable(_T("alpha[0]"), vsVersion[0]);
    pRequest->AddVariable(_T("beta[0]"), vsVersion[1]);
    pRequest->AddVariable(_T("omega[0]"), vsVersion[2]);
    pRequest->AddVariable(_T("hotfix[0]"), vsVersion[3]);

    pRequest->AddVariable(_T("name[0]"), _T("version"));
    pRequest->AddVariable(_T("checksum[0]"), _T("0"));

    pRequest->AddVariable(_T("os[0]"), K2System.GetBuildOSCodeString());
    pRequest->AddVariable(_T("arch[0]"), K2System.GetBuildArchString());

    pRequest->AddVariable(_T("remove[0]"), _T("0"));
    
    int iMaxRetrys(10);
    for (int i(0); i < iMaxRetrys; ++i)
    {
        pRequest->SendPostRequest();
        pRequest->Wait();
        
        if (pRequest->WasSuccessful())
            break;

        Console << _T("[Update Generation] Database update failed, retrying ") << i << _T("/") << iMaxRetrys << newl;
    }

    if (pRequest->WasSuccessful())
        Console << _T("[Update Generation] Database updated. Patch ") << sVersion << _T(" is live!") << newl;
    else
        Console << _T("[Update Generation] Database update failed.") << newl;

    Host.GetHTTPManager()->ReleaseRequest(pRequest);
}


/*====================
  CUpdater::SilentUpdate
  ====================*/
void    CUpdater::SilentUpdate()
{
    PROFILE("CUpdater::SilentUpdate");

    if (!upd_checkForUpdates || m_bCheckingForUpdate)
        return;

    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
    m_pRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pRequest == NULL)
        return;

    m_pRequest->SetTargetURL(m_sMasterServerURL);
    m_pRequest->AddVariable(_T("latest"), TSNULL);
    m_pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    m_pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    m_pRequest->SendPostRequest();

    m_eStatus = UPDATE_STATUS_SILENT;
}


/*====================
  CUpdater::CheckForUpdates
  ====================*/
void    CUpdater::CheckForUpdates(bool bRequireConfirmation)
{
    if (m_bDownloadingCompat)
        return;

    if (!upd_checkForUpdates
#ifdef K2_STEAM
        || K2System.IsSteamInitialized()
#endif
        )
    {
        if (upd_fakeUpdate)
        {
            m_eStatus = UPDATE_STATUS_NEEDS_UPDATE;

            m_sUpdateVersion.clear();
            m_sPrimaryServer.clear();
            m_sSecondaryServer.clear();

            UpdateStatus.Trigger(XtoA(m_eStatus));
        }
        else
        {
            m_eStatus = UPDATE_STATUS_NO_UPDATE;

            // No files to download, we're at the newest version.
            UpdateStatus.Trigger(XtoA(m_eStatus));
            UpdateMessage.Trigger(_T("0")); // No updates found.
            UpdateCurVersion.Trigger(K2System.GetVersionString());
            UpdatePercent.Trigger(_T("1"));
            UpdateError.Trigger(_T("0"));

            m_bCheckingForUpdate = false;
        }

        return;
    }

    if (m_bCheckingForUpdate)
        return;

    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
    m_pRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pRequest == NULL)
        return;

    m_bCheckingForUpdate = true;
    m_bRequireConfirmation = bRequireConfirmation;

    m_eStatus = UPDATE_STATUS_RETRIEVING_FILES;

    UpdateStatus.Trigger(XtoA(m_eStatus));
    UpdateMessage.Trigger(_T("1")); // Searching for updates...
    UpdatePercent.Trigger(_T("0"));

    m_pRequest->SetTargetURL(m_sMasterServerURL);
    m_pRequest->AddVariable(_T("version"), _T("0.0.0.0"));
    m_pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    m_pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    m_pRequest->SendPostRequest();

    UpdateLocalVersion.Trigger(K2System.GetVersionString());

    m_vUpdateFiles.clear();
    m_vDeleteFiles.clear();
    m_uiTotalSize = 0;
    m_uiCurDownloaded = 0;
    m_uiLastSpeedUpdate = -1;
    m_uiNumActive = 0;
    m_uiNumSpeedUpdates = 1;
    m_uiTotalResumed = 0;
    m_uiTotalFiles = 0;
    m_uiTotalSized = 0;
    m_uiFirstSpeedUpdate = 0;
    m_uiNumComplete = 0;
}


/*====================
  CUpdater::CancelUpdate
  ====================*/
void    CUpdater::CancelUpdate()
{
    UpdateMessage.Trigger(_T("2")); // Update cancelled
    UpdatePercent.Trigger(_T("0"));
    UpdateError.Trigger(_T("0"));

    for (UpdVector::iterator it(m_vUpdateFiles.begin()); it != m_vUpdateFiles.end(); it++)
    {
        m_fileHTTP.StopTransfer((!it->bPrimaryFail ? m_sPrimaryServer : m_sSecondaryServer) + it->sOS + _T("/") + it->sArch + _T("/") + it->sVersion + _T("/") + it->sURLFile + _T(".zip"));

        if (it->pFile != NULL)
            it->pFile->Close();

        SAFE_DELETE(it->pFile);

        if (!(it->bComplete))
        {
            // Delete files that weren't fully downloaded, if they exist
            if (FileManager.Exists(_T(":/Update/") + it->sFile))
                FileManager.Delete(_T(":/Update/") + it->sFile);

            if (FileManager.Exists(_T(":/Update/") + it->sFile + _T(".zip")))
                FileManager.Delete(_T(":/Update/") + it->sFile + _T(".zip"));
        }
    }

    m_vUpdateFiles.clear();
    m_vDeleteFiles.clear();
    m_uiTotalFiles = 0;

    m_bCheckingForUpdate = false;
    m_eStatus = UPDATE_STATUS_IDLE;

    UpdateStatus.Trigger(XtoA(m_eStatus));
}


/*====================
  CUpdater::CancelCompatDownload
  ====================*/
void    CUpdater::CancelCompatDownload()
{
    CompatMessage.Trigger(_T("0"));
    CompatPercent.Trigger(_T("0"));
    CompatError.Trigger(_T("0"));
    CompatDownloading.Trigger(_T("0"));

    for (UpdVector::iterator it(m_vUpdateFiles.begin()); it != m_vUpdateFiles.end(); ++it)
    {
        m_fileHTTP.StopTransfer((!it->bPrimaryFail ? m_sPrimaryServer : m_sSecondaryServer) + it->sOS + _T("/") + it->sArch + _T("/") + it->sVersion + _T("/") + it->sURLFile + _T(".zip"));

        if (it->pFile != NULL)
            it->pFile->Close();

        SAFE_DELETE(it->pFile);

        if (!(it->bComplete))
        {
            // Delete files that weren't fully downloaded, if they exist
            if (FileManager.Exists(_T(":/Update/") + it->sFile))
                FileManager.Delete(_T(":/Update/") + it->sFile);

            if (FileManager.Exists(_T(":/Update/") + it->sFile + _T(".zip")))
                FileManager.Delete(_T(":/Update/") + it->sFile + _T(".zip"));
        }
    }

    m_vUpdateFiles.clear();
    m_vDeleteFiles.clear();
    m_uiTotalFiles = 0;

    m_bDownloadingCompat = false;
    m_eStatus = UPDATE_STATUS_IDLE;
}


/*====================
  CUpdater::GetUpdate
  ====================*/
void    CUpdater::GetUpdate(const tstring &sFile, const tstring &sVersion, const tstring &sOS, const tstring &sArch, uint uiSize, uint uiFullSize, uint uiChecksum, const tstring &sOldVersion)
{
    if (sFile.empty())
        return;

    SUpdateFile sUpdate;

    sUpdate.sFile = sFile;
    sUpdate.sURLFile = StringToTString(URLEncode(sFile, true, true));
    sUpdate.pFile = NULL;
    sUpdate.uiDownloaded = 0;
    sUpdate.bDownloading = false;
    sUpdate.bComplete = false;
    sUpdate.iRetries = 0;
    sUpdate.bGotSize = false;
    sUpdate.uiSize = uiSize;
    sUpdate.sVersion = sVersion;
    sUpdate.sOS = sOS;
    sUpdate.sArch = sArch;
    sUpdate.uiFullSize = uiFullSize;
    sUpdate.uiChecksum = uiChecksum;
    sUpdate.bVerified = false;
    sUpdate.bExtracted = false;
    sUpdate.bPrimaryFail = false;
    sUpdate.bSecondaryFail = false;
    sUpdate.sOldVersion = sOldVersion;

    ++m_uiTotalFiles;

    m_vUpdateFiles.push_back(sUpdate);
}


/*====================
  CUpdater::ResponseOK
  ====================*/
void    CUpdater::ResponseOK(CPHPData &response)
{
    // No files to download, we're at the newest version.
    UpdateMessage.Trigger(_T("0")); // No updates found.
    UpdateCurVersion.Trigger(K2System.GetVersionString());
    UpdatePercent.Trigger(_T("1"));
    UpdateError.Trigger(_T("0"));

    m_bCheckingForUpdate = false;

    m_eStatus = UPDATE_STATUS_NO_UPDATE;

    UpdateStatus.Trigger(XtoA(m_eStatus));
}


/*====================
  CUpdater::ResponseVersionError
  ====================*/
void    CUpdater::ResponseVersionError()
{
    UpdateMessage.Trigger(_T("5")); // Error checking for updates.
    UpdatePercent.Trigger(_T("1"));
    UpdateError.Trigger(_T("1"));

    m_bCheckingForUpdate = false;

    m_eStatus = UPDATE_STATUS_IDLE;

    UpdateStatus.Trigger(XtoA(m_eStatus));
}


/*====================
  CUpdater::UpdateConfirm
  ====================*/
void    CUpdater::UpdateConfirm(bool bUpdate)
{
    if (m_eStatus != UPDATE_STATUS_NEEDS_UPDATE)
        return;

    if (bUpdate)
    {
        StartUpdate();
    }
    else
    {
        m_eStatus = UPDATE_STATUS_SKIPPED_UPDATE;

        UpdateStatus.Trigger(XtoA(m_eStatus));
    }
}


/*====================
  CUpdater::StartUpdate
  ====================*/
void    CUpdater::StartUpdate()
{
    if (m_sUpdateVersion.empty())
    {
        m_eStatus = UPDATE_STATUS_NO_UPDATE;
        return;
    }

    m_eStatus = UPDATE_STATUS_DOWNLOADING;

    m_bCheckingForUpdate = true;

    // Delete patch verification file
    if (FileManager.Exists(_T(":/Update/verify")))
        FileManager.Delete(_T(":/Update/verify"));

    UpdateCurVersion.Trigger(m_sUpdateVersion);
    UpdateError.Trigger(_T("0"));
    UpdatePercent.Trigger(_T("2"));
    UpdateStatus.Trigger(XtoA(m_eStatus));

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();

    m_cNewManifest = SFileManifest();
    m_cOldManifest = SFileManifest();

    tstring sOldVersion(K2_Version(K2System.GetVersionString()));

    int i(0);

    i = 0;
    for (; i < 2; ++i)
    {
        if (!sOldVersion.empty())
        {
            bool bSuccess(false);
            for (int iRetry(0); iRetry < UPDATER_MAX_RETRIES; ++iRetry)
            {
                if(m_fileHTTP.Open((i == 0 ? m_sPrimaryServer : m_sSecondaryServer) + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sOldVersion + _T("/manifest.xml.zip"), FILE_BLOCK) &&
                    !m_fileHTTP.ErrorEncountered())
                {
                    uint uiSize;
                    const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));
        
                    CMMapUnzip cManifestZip(pBuffer, uiSize);
        
                    char *pManifestBuffer;
                    uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));
        
                    if (uiManifestSize > 0)
                    {
                        bool bParsed(XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &m_cOldManifest));
        
                        K2_DELETE_ARRAY(pManifestBuffer);
        
                        if (bParsed)
                        {
                            bSuccess = true;
                            break; // Success!
                        }
                        else
                        {
                            Console.Net << _T("[Updater] Failure parsing manifest: ") << sOldVersion << newl;
                            iRetry = UPDATER_MAX_RETRIES;
                            continue;
                        }
                    }
                    else
                    {
                        Console.Net << _T("[Updater] Empty manifest: ") << sOldVersion << newl;
                        iRetry = UPDATER_MAX_RETRIES;
                        continue;
                    }
                }
            }
            if (bSuccess)
                break;
        }
    }
    if (i == 2)
    {
        Console.Net << _T("[Updater] Failed to retrieve manifest: ") << sOldVersion << newl;
        ResponseVersionError();
        return;
    }

    tstring sNewVersion(K2_Version(m_sUpdateVersion));

    i = 0;
    for (; i < 2; ++i)
    {
        if (!sNewVersion.empty())
        {
            bool bSuccess(false);
            for (int iRetry(0); iRetry < UPDATER_MAX_RETRIES; ++iRetry)
            {
                if(m_fileHTTP.Open((i == 0 ? m_sPrimaryServer : m_sSecondaryServer) + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sNewVersion + _T("/manifest.xml.zip"), FILE_BLOCK) &&
                    !m_fileHTTP.ErrorEncountered())
                {
                    uint uiSize;
                    const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));
        
                    CMMapUnzip cManifestZip(pBuffer, uiSize);
        
                    char *pManifestBuffer;
                    uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));
        
                    if (uiManifestSize > 0)
                    {
                        bool bParsed(XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &m_cNewManifest));
                        
                        K2_DELETE_ARRAY(pManifestBuffer);
        
                        if (bParsed)
                        {
                            bSuccess = true;
                            break; // Success!
                        }
                        else
                        {
                            Console.Net << _T("[Updater] Failure parsing manifest: ") << sNewVersion << newl;
                            iRetry = UPDATER_MAX_RETRIES;
                            continue;
                        }
                    }
                    else
                    {
                        Console.Net << _T("[Updater] Empty manifest: ") << sNewVersion << newl;
                        iRetry = UPDATER_MAX_RETRIES;
                        continue;
                    }
                }
            }
            if (bSuccess)
                break;
        }
    }
    if (i == 2)
    {
        Console.Net << _T("[Updater] Failed to retrieve manifest: ") << sNewVersion << newl;
        ResponseVersionError();
        return;
    }

    //
    // Generate manifest
    //

    CXMLDoc xmlManifest(XML_ENCODE_UTF8);

    xmlManifest.NewNode("manifest");

        xmlManifest.AddProperty("version", m_cNewManifest.sVersion);
        xmlManifest.AddProperty("os", m_cNewManifest.sOS);
        xmlManifest.AddProperty("arch", m_cNewManifest.sArch);

        for (ManifestEntryMap::iterator it(m_cNewManifest.mapManifestFiles.begin()); it != m_cNewManifest.mapManifestFiles.end(); ++it)
        {
            xmlManifest.NewNode("file");
                xmlManifest.AddProperty("path", it->first);
                xmlManifest.AddProperty("size", XtoA(it->second.uiSize));
                xmlManifest.AddProperty("checksum", CtoA(it->second.uiChecksum));
                xmlManifest.AddProperty("version", VtoA(it->second.uiVersion));
                //xmlManifest.AddProperty("zipsize", XtoA(it->second.uiZipSize));
            xmlManifest.EndNode();
        }

    xmlManifest.EndNode();

    xmlManifest.WriteFile(_T(":/Update/manifest.xml"), false);

    for (ManifestEntryMap::iterator it(m_cNewManifest.mapManifestFiles.begin()); it != m_cNewManifest.mapManifestFiles.end(); ++it)
    {
        ManifestEntryMap::iterator itOld(m_cOldManifest.mapManifestFiles.find(it->first));

        if (itOld != m_cOldManifest.mapManifestFiles.end() &&
            it->second.uiSize == itOld->second.uiSize &&
            it->second.uiChecksum == itOld->second.uiChecksum &&
            it->second.uiVersion == itOld->second.uiVersion)
            continue;

        tstring sOldVersion(itOld != m_cOldManifest.mapManifestFiles.end() ? VtoA2(itOld->second.uiVersion) : TSNULL);

        GetUpdate(it->first, VtoA2(it->second.uiVersion), m_cNewManifest.sOS, m_cNewManifest.sArch, it->second.uiZipSize, it->second.uiSize, it->second.uiChecksum, sOldVersion);
    }

    for (ManifestEntryMap::iterator it(m_cOldManifest.mapManifestFiles.begin()); it != m_cOldManifest.mapManifestFiles.end(); ++it)
    {
        ManifestEntryMap::iterator itNew(m_cNewManifest.mapManifestFiles.find(it->first));

        if (itNew != m_cNewManifest.mapManifestFiles.end())
            continue;

        SDeleteFile cDelete;
        cDelete.sFile = it->first;
        cDelete.sOldVersion = VtoA2(it->second.uiVersion);

        m_vDeleteFiles.push_back(cDelete);
    }

    if (!m_vUpdateFiles.empty() || !m_vDeleteFiles.empty())
    {
        m_eStatus = UPDATE_STATUS_DOWNLOADING;

        for (UpdVector::iterator it(m_vUpdateFiles.begin()); it != m_vUpdateFiles.end(); ++it)
        {
            ++m_uiTotalSized;
            m_uiTotalSize += it->uiSize;
        }

        CleanupUpdateFiles();
    
        // Get the update
        UpdateStatus.Trigger(XtoA(m_eStatus));
        UpdateMessage.Trigger(_T("4")); // Preparing to download update...
    }
    else
    {
        // No files to download, we're at the newest version.
        UpdateMessage.Trigger(_T("0")); // No updates found.
        UpdateCurVersion.Trigger(K2System.GetVersionString());
        UpdatePercent.Trigger(_T("1"));
        UpdateError.Trigger(_T("0"));

        m_bCheckingForUpdate = false;

        m_eStatus = UPDATE_STATUS_NO_UPDATE;

        UpdateStatus.Trigger(XtoA(m_eStatus));
    }
}


/*====================
  CUpdater::ResponseVersion
  ====================*/
void    CUpdater::ResponseVersion(CPHPData &response)
{
    tstring sResponseVersion(response.GetString(_T("version")));

    if (sResponseVersion == K2System.GetVersionString())
    {
        // Current version
        UpdateMessage.Trigger(_T("0")); // No updates found.
        UpdateCurVersion.Trigger(K2System.GetVersionString());
        UpdatePercent.Trigger(_T("1"));
        UpdateError.Trigger(_T("0"));

        m_bCheckingForUpdate = false;

        m_eStatus = UPDATE_STATUS_NO_UPDATE;

        UpdateStatus.Trigger(XtoA(m_eStatus));

        return;
    }

    bool bValid(false);
    tstring sPrimaryServer;
    tstring sSecondaryServer;

    // Grab version info
    const CPHPData *pVersion(response.GetVar(0));
    if (pVersion != NULL)
    {
        bValid = true;
        sPrimaryServer = pVersion->GetString(_T("url"));
        sSecondaryServer = pVersion->GetString(_T("url2"));
    }

    if (!bValid || sPrimaryServer.empty() || sSecondaryServer.empty())
    {
        Console.Net << _T("[Updater] Error in version response") << newl;
        ResponseVersionError();
        return;
    }

    m_sUpdateVersion = sResponseVersion;
    m_sPrimaryServer = sPrimaryServer;
    m_sSecondaryServer = sSecondaryServer;

    if (m_bRequireConfirmation)
    {
        m_eStatus = UPDATE_STATUS_NEEDS_UPDATE;

        UpdateStatus.Trigger(XtoA(m_eStatus));
        UpdateCurVersion.Trigger(m_sUpdateVersion);

        m_bCheckingForUpdate = false;
    }
    else
    {
        StartUpdate();
    }
}


/*====================
  CUpdater::ResponseError
  ====================*/
void    CUpdater::ResponseError(CPHPData &response)
{
    // Error checking for updates.
    UpdateMessage.Trigger(_T("5")); // Error checking for updates.
    UpdatePercent.Trigger(_T("1"));
    UpdateError.Trigger(_T("1"));

    m_bCheckingForUpdate = false;

    m_eStatus = UPDATE_STATUS_IDLE;

    UpdateStatus.Trigger(XtoA(m_eStatus));
}


/*====================
  CUpdater::FrameAwaitingResponse

  We are currently trying to get data from the update script
  ====================*/
void    CUpdater::FrameAwaitingResponse()
{
    if (m_pRequest == NULL || m_pRequest->IsActive())
        return;

    // If we got a response, check for a file being passed.
    // If no file was passed, we either have the newest version
    // or there was an error checking - Either way, no download needed.
    if (m_eStatus == UPDATE_STATUS_RETRIEVING_FILES)
    {
        if (m_pRequest->WasSuccessful())
        {
            CPHPData response(m_pRequest->GetResponse());
            Console << _T("====================") << newl;
            response.Print();
            Console << _T("====================") << newl;

            if (response.GetString(_T("response")) == _T("OK"))
                ResponseOK(response);
            else if (response.GetString(_T("version")) != _T(""))
                ResponseVersion(response);
            else
                ResponseOK(response); //ResponseError(response);
        }
        else
        {
            CPHPData response(_T("N;"));
            ResponseOK(response); //ResponseError(response);
        }
    }
    else if (m_eStatus == UPDATE_STATUS_SILENT)
    {
        m_eStatus = UPDATE_STATUS_IDLE;

        if (m_pRequest->WasSuccessful())
        {
            CPHPData response(m_pRequest->GetResponse());
            tstring sLatestVersion(response.GetString(_T("latest")));
            if (!sLatestVersion.empty() && sLatestVersion != K2System.GetVersionString())
                Host.UpdateAvailable(sLatestVersion);
        }
    }

    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
    m_pRequest = NULL;
}


/*====================
  CUpdater::ErrorDownloading
  ====================*/
void    CUpdater::ErrorDownloading()
{
    CancelUpdate();

    UpdateError.Trigger(_T("1"));
    UpdateMessage.Trigger(_T("7")); // Error downloading update.
    UpdatePercent.Trigger(_T("1"));
}


/*====================
  CUpdater::FrameDownloading
  ====================*/
void    CUpdater::FrameDownloading()
{
    uint uiNumProcessed(0);
    UpdVector::iterator it(m_vUpdateFiles.begin());

    //uint uiMaxToSize(MIN(upd_maxActiveDownloads.GetUnsignedInteger(), m_uiTotalFiles - m_uiTotalSized));

    while (it != m_vUpdateFiles.end() && uiNumProcessed < upd_maxActiveDownloads)
    {
        if (it->bComplete)
        {
            ++it;
            continue;
        }

        if (!it->bGotSize)
        {
            if (m_bCheckingForUpdate && FileManager.Exists(_T(":/Update/") + it->sFile))
            {
                uiNumProcessed++;
                
                CFile *pFile(FileManager.GetFile(_T(":/Update/") + it->sFile, FILE_READ | FILE_BINARY));

                if (pFile != NULL && pFile->IsOpen())
                {
                    uint uiSize;
                    const char *pBuffer(pFile->GetBuffer(uiSize));

                    if (pBuffer != NULL)
                    {
                        if (it->uiFullSize == uiSize && it->uiChecksum == FileManager.GetCRC32(pBuffer, uiSize))
                        {
                            Console.Net << _T("[Updater] Download already complete and extracted ") << it->sFile << newl;
                            m_uiTotalResumed += it->uiSize;

                            it->bComplete = true;
                            it->bVerified = true;
                            it->bGotSize = true;
                            it->bExtracted = true;
                            
                            pFile->Close();
                            SAFE_DELETE(pFile);

                            ++m_uiNumComplete;

                            ++it;
                            continue;
                        }
                        else
                            Console.Net << _T("[Updater] Download of ") << it->sFile << _T(" not completed, restarting download.") << newl;
                    }
                    else
                        Console.Net << _T("[Updater] Download of ") << it->sFile << _T(" not completed, restarting download.") << newl;

                    pFile->Close();
                    SAFE_DELETE(pFile);
                }
            }

            if (!it->bGotSize)
            {
                it->bGotSize = true;

                tstring sNewFilename;
                size_t zFindPos;

                zFindPos = it->sFile.find(_T(".s2z"));

                if (zFindPos != tstring::npos)
                    sNewFilename = it->sFile.substr(0, zFindPos) + _T(".s2z") + it->sFile.substr(zFindPos + 4);
                else
                    sNewFilename = it->sFile;

                if (FileManager.Exists(_T(":/Update/") + sNewFilename + _T(".zip")))
                {
                    uiNumProcessed++;

                    CArchive cArchive(_T(":/Update/") + sNewFilename + _T(".zip"), ARCHIVE_READ);

                    if (cArchive.IsOpen() && cArchive.ContainsFile(LowerString(_T(":/Update/") + sNewFilename)))
                    {
                        char *pBuf(NULL);
                        size_t zLen;

                        zLen = cArchive.ReadFile(LowerString(_T(":/Update/") + sNewFilename), pBuf);

                        if (pBuf == NULL)
                            continue;

                        if (it->uiFullSize == zLen && it->uiChecksum == FileManager.GetCRC32(pBuf, zLen))
                        {
                            Console.Net << _T("[Updater] Download already complete on ") << it->sFile << newl;
                            m_uiTotalResumed += it->uiSize;

                            it->bComplete = true;
                            it->bVerified = true;
                            
                            SAFE_DELETE_ARRAY(pBuf);
                            cArchive.Close();

                            ++m_uiNumComplete;
                            continue;
                        }
                        else
                            Console.Net << _T("[Updater] Download of ") << it->sFile << _T(" not completed, restarting download.") << newl;

                        SAFE_DELETE_ARRAY(pBuf);
                        cArchive.Close();
                    }

                    continue;
                }
            }
        }

        if (it->bDownloading || m_uiNumActive < upd_maxActiveDownloads)
        {
            uiNumProcessed++;

            if (m_uiLastSpeedUpdate == -1)
                m_uiLastSpeedUpdate = K2System.Milliseconds();

            if (!it->bDownloading)
            {
                Console.Net << _T("[Updater] Starting download on ") << it->sFile << newl;

                tstring sNewFilename(it->sFile);

                it->pFile = (CFileDisk *)FileManager.GetFile(_T(":/Update/") + sNewFilename + _T(".zip"), FILE_WRITE | FILE_BINARY);

                // If that didn't work... We've run into an issue.
                if (it->pFile == NULL || !it->pFile->IsOpen())
                {
                    Console.Net << _T("[Updater] Error opening ") << it->sFile << _T(".zip for writing!") << newl;

                    if (it->iRetries < UPDATER_MAX_RETRIES)
                    {
                        it->iRetries++;
                        Console.Net << _T("[Updater] Retrying request, attempt ") << it->iRetries << newl;

                        it->bDownloading = false;
                        m_uiCurDownloaded -= it->uiDownloaded;
                        it->uiDownloaded = 0;

                        SAFE_DELETE(it->pFile);

                        if (FileManager.Exists(_T(":/Update/") + sNewFilename + _T(".zip")))
                            FileManager.Delete(_T(":/Update/") + sNewFilename + _T(".zip"));

                        continue;
                    }
                    else
                    {
                        Console.Net << _T("[Updater] Retries exhausted, update failed.") << newl;

                        ErrorDownloading();
                        return;
                    }
                }

                m_fileHTTP.SetFileTarget(it->pFile);

                it->bDownloading = m_fileHTTP.Open((!it->bPrimaryFail ? m_sPrimaryServer : m_sSecondaryServer) + it->sOS + _T("/") + it->sArch + _T("/") + it->sVersion + _T("/") + it->sURLFile + _T(".zip"), FILE_HTTP_WRITETOFILE);

                m_uiNumActive++;
                ++it;
                continue;

            }

            // Grab the update using cURL. Calling this function
            // constantly will do nothing but update the status,
            // it will NOT cause it to restart the download each time.
            it->bDownloading = m_fileHTTP.Open((!it->bPrimaryFail ? m_sPrimaryServer : m_sSecondaryServer) + it->sOS + _T("/") + it->sArch + _T("/") + it->sVersion + _T("/") + it->sURLFile + _T(".zip"), FILE_HTTP_WRITETOFILE);

            // If we finished the download on this update...
            if (m_fileHTTP.IsOpen())
            {
                bool bError(false);

                if (!m_fileHTTP.ErrorEncountered())
                {
                    Console.Net << _T("[Updater] Finished download on ") << it->sFile << newl;

                    if (it->pFile != NULL)
                        it->pFile->Close();

                    SAFE_DELETE(it->pFile);
                    
                    //
                    // Verify Download
                    //

                    tstring sNewFilename;
                    size_t zFindPos;

                    zFindPos = it->sFile.find(_T(".s2z"));

                    if (zFindPos != tstring::npos)
                        sNewFilename = it->sFile.substr(0, zFindPos) + _T(".s2z") + it->sFile.substr(zFindPos + 4);
                    else
                        sNewFilename = it->sFile;

                    if (FileManager.Exists(_T(":/Update/") + sNewFilename + _T(".zip")))
                    {
                        CArchive cArchive(_T(":/Update/") + sNewFilename + _T(".zip"), ARCHIVE_READ);

                        if (cArchive.IsOpen() && cArchive.ContainsFile(LowerString(_T(":/Update/") + sNewFilename)))
                        {
                            char *pBuf(NULL);
                            size_t zLen;

                            zLen = cArchive.ReadFile(LowerString(_T(":/Update/") + sNewFilename), pBuf);

                            if (pBuf == NULL)
                                continue;

                            if (it->uiFullSize == zLen && it->uiChecksum == FileManager.GetCRC32(pBuf, zLen))
                            {
                                it->bVerified = true;
                            }
                            else
                            {
                                bError = true;
                                it->iRetries = UPDATER_MAX_RETRIES;
                            }

                            SAFE_DELETE_ARRAY(pBuf);
                        }
                        else
                        {
                            bError = true;
                            it->iRetries = UPDATER_MAX_RETRIES;
                        }
                    }
                    else
                    {
                        bError = true;
                        it->iRetries = UPDATER_MAX_RETRIES;
                    }
                }
                else
                    bError = true;

                if (!bError)
                {
                    --m_uiNumActive;
                    m_uiCurDownloaded += (it->uiSize - it->uiDownloaded);
                }

                if (bError)
                {
                    Console.Net << _T("[Updater] Error downloading ") << it->sFile << newl;

                    if (it->iRetries >= UPDATER_MAX_RETRIES)
                    {
                        if (!it->bPrimaryFail)
                            it->bPrimaryFail = true;
                        else if (!it->bSecondaryFail)
                            it->bSecondaryFail = true;

                        it->iRetries = 0;
                    }

                    if (!it->bPrimaryFail || !it->bSecondaryFail)
                    {
                        it->iRetries++;
                        Console.Net << _T("[Updater] Retrying request, attempt ") << it->iRetries << newl;

                        it->bDownloading = false;
                        m_uiCurDownloaded -= it->uiDownloaded;
                        it->uiDownloaded = 0;
                        
                        if (it->pFile != NULL)
                            it->pFile->Close();

                        SAFE_DELETE(it->pFile);

                        if (FileManager.Exists(_T(":/Update/") + it->sFile + _T(".zip")))
                            FileManager.Delete(_T(":/Update/") + it->sFile + _T(".zip"));

                        --m_uiNumActive;
                        continue;
                    }
                    else
                    {
                        Console.Net << _T("[Updater] Retries exhausted, update failed.") << newl;

                        ErrorDownloading();
                        return;
                    }
                }

                it->bComplete = true;
                m_uiNumComplete++;

                continue;
            }
            else
            {
                // Update progress
                SHTTPProgress progress = m_fileHTTP.GetProgress();

                m_uiCurDownloaded += (uint(progress.dDownloaded) - it->uiDownloaded);
                it->uiDownloaded = uint(progress.dDownloaded);
            }
        }

        ++it;
    }

    if (m_bCheckingForUpdate)
    {
        tsvector vsParams;

        UpdateMessage.Trigger(_T("8")); // Downloading update...
        vsParams.push_back(XtoA(float(MIN(m_uiCurDownloaded + m_uiTotalResumed, m_uiTotalSize)) / 1048576, FMT_NOPREFIX, 0, 2));
        vsParams.push_back(XtoA(float(m_uiTotalSize) / 1048576, FMT_NOPREFIX, 0, 2));

        UpdateProgress.Trigger(vsParams);

        // Calculate download speed
        if (m_uiLastSpeedUpdate + 1000 < K2System.Milliseconds())
        {
            if (m_uiTotalSized == m_uiTotalFiles && m_uiFirstSpeedUpdate == 0)
                m_uiFirstSpeedUpdate = m_uiCurDownloaded;

            float fDownloadSpeed = float(m_uiCurDownloaded - m_uiFirstSpeedUpdate) / float(m_uiNumSpeedUpdates);
            uint uiHoursRemaining;
            uint uiMinutesRemaining;
            uint uiSecondsRemaining;

            m_uiLastSpeedUpdate += 1000;

            if (fDownloadSpeed > 0 && m_uiTotalSized == m_uiTotalFiles)
            {
                uiHoursRemaining = (float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) / 3600;
                uiMinutesRemaining = ((float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) - (uiHoursRemaining * 3600)) / 60;
                uiSecondsRemaining = (float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) - (uiMinutesRemaining * 60) - (uiHoursRemaining * 3600);

                UpdateTime.Trigger(XtoA(uiHoursRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutesRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiSecondsRemaining, FMT_PADZERO, 2));
            }
            else
                UpdateTime.Trigger(_T("??:??:??"));


            if (m_uiTotalSized == m_uiTotalFiles)
            {
                UpdateSpeed.Trigger(XtoA(fDownloadSpeed / 1024, FMT_NOPREFIX, 0, 2));
                m_uiNumSpeedUpdates++;
            }
            else
                UpdateSpeed.Trigger(_T("???"));
        }

        // Calculate percent finished...
        if (m_uiTotalSized < m_uiTotalFiles)
        {
            UpdatePercent.Trigger(_T("2"));
            UpdateApplyPercent.Trigger(_T("2"));
        }
        else if (m_uiTotalSize > 0)
        {
            UpdatePercent.Trigger(XtoA(float(INT_ROUND((float(m_uiCurDownloaded + m_uiTotalResumed) / float(m_uiTotalSize)) * 100)) / 100.0f));
            UpdateApplyPercent.Trigger(_T("2"));
        }
        else
        {
            UpdatePercent.Trigger(_T("2"));
            UpdateApplyPercent.Trigger(_T("2"));
        }

        vsParams.clear();

        if (m_uiTotalSized < m_uiTotalFiles)
            vsParams.push_back(XtoA(m_uiTotalFiles - m_uiTotalSized));
        else
            vsParams.push_back(XtoA(INT_SIZE(m_uiTotalFiles - m_uiNumComplete)));

        vsParams.push_back(XtoA(m_uiTotalFiles));

        UpdateFilesLeft.Trigger(vsParams);

        // If we're done downloading
        if (m_uiNumComplete >= m_uiTotalFiles)
        {
            ApplyUpdate();
        }
    }
    else if (m_bDownloadingCompat)
    {
        tsvector vsParams;

        CompatMessage.Trigger(_T("8")); // Downloading update...
        vsParams.push_back(XtoA(float(MIN(m_uiCurDownloaded + m_uiTotalResumed, m_uiTotalSize)) / 1048576, FMT_NOPREFIX, 0, 2));
        vsParams.push_back(XtoA(float(m_uiTotalSize) / 1048576, FMT_NOPREFIX, 0, 2));

        CompatProgress.Trigger(vsParams);

        // Calculate download speed
        if (m_uiLastSpeedUpdate + 1000 < K2System.Milliseconds())
        {
            if (m_uiTotalSized == m_uiTotalFiles && m_uiFirstSpeedUpdate == 0)
                m_uiFirstSpeedUpdate = m_uiCurDownloaded;

            float fDownloadSpeed = float(m_uiCurDownloaded - m_uiFirstSpeedUpdate) / float(m_uiNumSpeedUpdates);
            uint uiHoursRemaining;
            uint uiMinutesRemaining;
            uint uiSecondsRemaining;

            m_uiLastSpeedUpdate += 1000;

            if (fDownloadSpeed > 0 && m_uiTotalSized == m_uiTotalFiles)
            {
                uiHoursRemaining = (float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) / 3600;
                uiMinutesRemaining = ((float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) - (uiHoursRemaining * 3600)) / 60;
                uiSecondsRemaining = (float(m_uiTotalSize - (m_uiCurDownloaded + m_uiTotalResumed)) / fDownloadSpeed) - (uiMinutesRemaining * 60) - (uiHoursRemaining * 3600);

                CompatTime.Trigger(XtoA(uiHoursRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutesRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiSecondsRemaining, FMT_PADZERO, 2));
            }
            else
                CompatTime.Trigger(_T("??:??:??"));


            if (m_uiTotalSized == m_uiTotalFiles)
            {
                CompatSpeed.Trigger(XtoA(fDownloadSpeed / 1024, FMT_NOPREFIX, 0, 2));
                m_uiNumSpeedUpdates++;
            }
            else
                CompatSpeed.Trigger(_T("???"));
        }

        // Calculate percent finished...
        if (m_uiTotalSized < m_uiTotalFiles)
        {
            CompatPercent.Trigger(_T("2"));
            //CompatApplyPercent.Trigger(_T("2"));
        }
        else if (m_uiTotalSize > 0)
        {
            CompatPercent.Trigger(XtoA(float(INT_ROUND((float(m_uiCurDownloaded + m_uiTotalResumed) / float(m_uiTotalSize)) * 100)) / 100.0f));
            //CompatApplyPercent.Trigger(_T("2"));
        }
        else
        {
            CompatPercent.Trigger(_T("2"));
            //CompatApplyPercent.Trigger(_T("2"));
        }

        vsParams.clear();

        if (m_uiTotalSized < m_uiTotalFiles)
            vsParams.push_back(XtoA(m_uiTotalFiles - m_uiTotalSized));
        else
            vsParams.push_back(XtoA(INT_SIZE(m_uiTotalFiles - m_uiNumComplete)));

        vsParams.push_back(XtoA(m_uiTotalFiles));

        CompatFilesLeft.Trigger(vsParams);

        // If we're done downloading
        if (m_uiNumComplete >= m_uiTotalFiles)
        {
            CompatCalculating.Trigger(_T("0"));
            CompatDownloading.Trigger(_T("0"));
            CompatMessage.Trigger(_T("0"));

            UpdateCompatArchives();
        }
    }
}


/*====================
  CUpdater::CancelApplyUpdate
  ====================*/
void    CUpdater::CancelApplyUpdate(ZipMap &mapZipFiles)
{
    for (ZipMap::iterator itMap(mapZipFiles.begin()); itMap != mapZipFiles.end(); ++itMap)
        itMap->second.CancelWrite();

    mapZipFiles.clear();

    Console.Net << _T("[Updater] Update failed") << newl;

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();
    
    m_bCheckingForUpdate = false;
    m_eStatus = UPDATE_STATUS_IDLE;

    Host.UpdateComplete();

    K2System.RestartOnExit(true);
    K2System.Exit(0);
}


/*====================
  CUpdater::AutoUpdateCompatArchives
  ====================*/
void    CUpdater::AutoUpdateCompatArchives(float fLoadingScale, float fLoadingOffset)
{
    FileManager.DisableCompatArchive();

    sset setIgnore;

    CFile *pFile(NULL);

    pFile = FileManager.GetFile(_T(":/compat_ignore.txt"), FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_TEXT);

    if (pFile != NULL)
    {
        tstring sFilename;

        while (!pFile->IsEOF())
        {
            sFilename = pFile->ReadLine();

            StripNewline(sFilename);
            sFilename = LowerString(sFilename);

            if (!sFilename.empty() && setIgnore.find(sFilename) == setIgnore.end())
                setIgnore.insert(sFilename);
        }
    }

    ZipMap mapResFiles;
    ZipMap mapCompatFiles;
    int iFinished(0);

    UpdVector::iterator itFile(m_vUpdateFiles.begin());
    while (itFile != m_vUpdateFiles.end())
    {
        tstring sLower(LowerString(_T(":/") + itFile->sFile));

        bool bIgnored(false);
        for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); ++itIgnore)
        {
            if (EqualsWildcardNoCase(*itIgnore, sLower))
            {
                bIgnored = true;
                break;
            }
        }

        if (bIgnored)
        {
            ++iFinished;

            UpdatePercent.Trigger(_T("2"));

            UpdateApplyPercent.Trigger(XtoA(float(iFinished) / m_vUpdateFiles.size() * fLoadingScale + fLoadingOffset));

            K2System.HandleOSMessages();

            Vid.BeginFrame();
            Vid.SetColor(BLACK);
            Vid.Clear();
            UIManager.Frame(Host.GetFrameLength());
            Vid.EndFrame();

            ++itFile;
            continue;
        }

        tstring sBasePath;
        
#ifdef linux
        if (itFile->sFile == _T("hon_update-x86") || itFile->sFile == _T("hon_update-x86_64"))
#elif defined (__APPLE__)
        if (itFile->sFile == _T("HoN_Update"))
#else
        if (itFile->sFile == _T("hon_update.exe"))
#endif
            sBasePath = _T(":/");
        else
            sBasePath = _T(":/Update/");

        tstring sNewFilename(itFile->sFile);

        // Store the old files for compatibility
        if (!itFile->sOldVersion.empty())
        {
            CFile *pFileTarget(NULL);
            CCompressedFile cFile;
            int iSize(0);

            tstring sArchive(_T(":/compat/compat.s2z"));
            int iFlags(ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

            ZipMap::iterator itFindCompat(mapCompatFiles.find(sArchive));

            if (itFindCompat == mapCompatFiles.end())
            {
                mapCompatFiles.insert(pair<tstring, CArchive>(sArchive, CArchive()));
                itFindCompat = mapCompatFiles.find(sArchive);

                if (itFindCompat == mapCompatFiles.end())
                    continue;

                itFindCompat->second.Open(sArchive, iFlags);

                if (!itFindCompat->second.IsOpen())
                    itFindCompat->second.Open(sArchive, iFlags & ~ARCHIVE_APPEND);
            }

            size_t zPos(itFile->sFile.find(_T(".s2z")));
            if (zPos != tstring::npos)
            {
                // We're working with an archive file
                ZipMap::iterator itFindRes;
                int iFlags(ARCHIVE_READ);

                itFindRes = mapResFiles.find(itFile->sFile.substr(0, zPos + 4));

                if (itFindRes == mapResFiles.end())
                {
                    mapResFiles.insert(pair<tstring, CArchive>(itFile->sFile.substr(0, zPos + 4), CArchive()));
                    itFindRes = mapResFiles.find(itFile->sFile.substr(0, zPos + 4));

                    if (itFindRes == mapResFiles.end())
                        continue;

                    itFindRes->second.Open(_T(":/") + itFile->sFile.substr(0, zPos + 4), iFlags);

                    if (!itFindRes->second.IsOpen())
                        itFindRes->second.Open(_T(":/") + itFile->sFile.substr(0, zPos + 4), iFlags & ~ARCHIVE_APPEND);
                }

                if (itFindRes->second.IsOpen())
                {
                    tstring sFilename(itFile->sFile);

                    if (itFindRes->second.ContainsFile(itFindRes->second.GetPathToArchive() + _T("/") + itFile->sFile.substr(zPos + 5)))
                    {
                        iSize = itFindRes->second.GetCompressedFile(itFindRes->second.GetPathToArchive() + _T("/") + itFile->sFile.substr(zPos + 5), cFile);

                        if (cFile.GetData() != NULL)
                        {
                            if (itFindCompat->second.IsOpen())
                                itFindCompat->second.WriteCompressedFile(itFile->sOldVersion + _T("/") + itFile->sFile, cFile);
                            else
                                Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Could not write to target file.") << newl;
                        }
                        else
                            Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Could not decompress file.") << newl;
                    }
                    else
                        Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Archive does not contain proper file.") << newl;
                }
                else
                    Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Target archive could not be opened for reading.") << newl;
            }
            else
            {
                pFileTarget = FileManager.GetFile(_T(":/") + itFile->sFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);

                CFileHandle hDestFile(itFile->sOldVersion + _T("/") + itFile->sFile, FILE_WRITE | FILE_BINARY | FILE_COMPRESS, itFindCompat->second);
                if (pFileTarget != NULL && hDestFile.IsOpen())
                {
                    uint uiSize(0);
                    if (hDestFile.Write(pFileTarget->GetBuffer(uiSize), pFileTarget->GetBufferSize()) != pFileTarget->GetBufferSize())
                    {
                        Console.Err << _T("Couldn't write file ") << itFile->sFile << _T(" in archive ") << itFindCompat->first << newl;
                        if (pFileTarget != NULL)
                        {
                            pFileTarget->Close();
                            SAFE_DELETE(pFileTarget);
                        }
                    }
                }
                else
                {
                    Console.Err << _T("Couldn't open file ") << itFile->sFile << _T(" in archive ") << itFindCompat->first << newl;
                    if (pFileTarget != NULL)
                    {
                        pFileTarget->Close();
                        SAFE_DELETE(pFileTarget);
                    }
                }

                SAFE_DELETE(pFileTarget);
            }
        }

        ++iFinished;

        UpdatePercent.Trigger(_T("2"));

        UpdateApplyPercent.Trigger(XtoA(float(iFinished) / m_vUpdateFiles.size() * fLoadingScale + fLoadingOffset));

        K2System.HandleOSMessages();

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();

        ++itFile;
    }

    DelVector::iterator itDelFile(m_vDeleteFiles.begin());
    while (itDelFile != m_vDeleteFiles.end())
    {
        tstring sLower(LowerString(_T(":/") + itDelFile->sFile));

        bool bIgnored(false);
        for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); ++itIgnore)
        {
            if (EqualsWildcardNoCase(*itIgnore, sLower))
            {
                bIgnored = true;
                break;
            }
        }

        if (bIgnored)
        {
            ++itDelFile;
            continue;
        }

        // Store the old files for compatibility
        if (!itDelFile->sOldVersion.empty())
        {
            CFile *pFileTarget(NULL);
            CCompressedFile cFile;
            int iSize(0);

            tstring sArchive(_T(":/compat/compat.s2z"));
            int iFlags(ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

            ZipMap::iterator itFindCompat(mapCompatFiles.find(sArchive));

            if (itFindCompat == mapCompatFiles.end())
            {
                mapCompatFiles.insert(pair<tstring, CArchive>(sArchive, CArchive()));
                itFindCompat = mapCompatFiles.find(sArchive);

                if (itFindCompat == mapCompatFiles.end())
                    continue;

                itFindCompat->second.Open(sArchive, iFlags);

                if (!itFindCompat->second.IsOpen())
                    itFindCompat->second.Open(sArchive, iFlags & ~ARCHIVE_APPEND);
            }

            size_t zPos(itDelFile->sFile.find(_T(".s2z")));
            if (zPos != tstring::npos)
            {
                // We're working with an archive file
                ZipMap::iterator itFindRes;
                int iFlags(ARCHIVE_READ);

                itFindRes = mapResFiles.find(itDelFile->sFile.substr(0, zPos + 4));

                if (itFindRes == mapResFiles.end())
                {
                    mapResFiles.insert(pair<tstring, CArchive>(itDelFile->sFile.substr(0, zPos + 4), CArchive()));
                    itFindRes = mapResFiles.find(itDelFile->sFile.substr(0, zPos + 4));

                    if (itFindRes == mapResFiles.end())
                        continue;

                    itFindRes->second.Open(_T(":/") + itDelFile->sFile.substr(0, zPos + 4), iFlags);

                    if (!itFindRes->second.IsOpen())
                        itFindRes->second.Open(_T(":/") + itDelFile->sFile.substr(0, zPos + 4), iFlags & ~ARCHIVE_APPEND);
                }

                if (itFindRes->second.IsOpen())
                {
                    tstring sFilename(itDelFile->sFile);

                    if (itFindRes->second.ContainsFile(itFindRes->second.GetPathToArchive() + _T("/") + itDelFile->sFile.substr(zPos + 5)))
                    {
                        iSize = itFindRes->second.GetCompressedFile(itFindRes->second.GetPathToArchive() + _T("/") + itDelFile->sFile.substr(zPos + 5), cFile);

                        if (cFile.GetData() != NULL)
                        {
                            if (itFindCompat->second.IsOpen())
                                itFindCompat->second.WriteCompressedFile(itDelFile->sOldVersion + _T("/") + itDelFile->sFile, cFile);
                            else
                                Console.Net << _T("[Updater] Error unzipping ") + itDelFile->sFile + _T(": Could not write to target file.") << newl;
                        }
                        else
                            Console.Net << _T("[Updater] Error unzipping ") + itDelFile->sFile + _T(": Could not decompress file.") << newl;
                    }
                    else
                        Console.Net << _T("[Updater] Error unzipping ") + itDelFile->sFile + _T(": Archive does not contain proper file.") << newl;
                }
                else
                    Console.Net << _T("[Updater] Error unzipping ") + itDelFile->sFile + _T(": Target archive could not be opened for reading.") << newl;
            }
            else
            {
                pFileTarget = FileManager.GetFile(_T(":/") + itDelFile->sFile, FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_BINARY);

                CFileHandle hDestFile(itDelFile->sOldVersion + _T("/") + itDelFile->sFile, FILE_WRITE | FILE_BINARY | FILE_COMPRESS, itFindCompat->second);
                if (pFileTarget != NULL && hDestFile.IsOpen())
                {
                    uint uiSize(0);
                    if (hDestFile.Write(pFileTarget->GetBuffer(uiSize), pFileTarget->GetBufferSize()) != pFileTarget->GetBufferSize())
                    {
                        Console.Err << _T("Couldn't write file ") << itDelFile->sFile << _T(" in archive ") << itFindCompat->first << newl;
                        if (pFileTarget != NULL)
                        {
                            pFileTarget->Close();
                            SAFE_DELETE(pFileTarget);
                        }
                    }
                }
                else
                {
                    Console.Err << _T("Couldn't open file ") << itDelFile->sFile << _T(" in archive ") << itFindCompat->first << newl;
                    if (pFileTarget != NULL)
                    {
                        pFileTarget->Close();
                        SAFE_DELETE(pFileTarget);
                    }
                }

                SAFE_DELETE(pFileTarget);
            }
        }

        ++itDelFile;
    }

    // Store manifest
    tstring sOldVersion(K2_Version(K2System.GetVersionString()));

    tstring sArchive(_T(":/compat/compat.s2z"));
    int iFlags(ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

    ZipMap::iterator itFindCompat(mapCompatFiles.find(sArchive));

    if (itFindCompat == mapCompatFiles.end())
    {
        mapCompatFiles.insert(pair<tstring, CArchive>(sArchive, CArchive()));
        itFindCompat = mapCompatFiles.find(sArchive);

        itFindCompat->second.Open(sArchive, iFlags);

        if (!itFindCompat->second.IsOpen())
            itFindCompat->second.Open(sArchive, iFlags & ~ARCHIVE_APPEND);
    }

    CXMLDoc xmlManifest(XML_ENCODE_UTF8);

    xmlManifest.NewNode("manifest");

        xmlManifest.AddProperty("version", m_cOldManifest.sVersion);
        xmlManifest.AddProperty("os", m_cOldManifest.sOS);
        xmlManifest.AddProperty("arch", m_cOldManifest.sArch);

        for (ManifestEntryMap::iterator it(m_cOldManifest.mapManifestFiles.begin()); it != m_cOldManifest.mapManifestFiles.end(); ++it)
        {
            xmlManifest.NewNode("file");
                xmlManifest.AddProperty("path", it->first);
                xmlManifest.AddProperty("size", XtoA(it->second.uiSize));
                xmlManifest.AddProperty("checksum", CtoA(it->second.uiChecksum));
                xmlManifest.AddProperty("version", VtoA(it->second.uiVersion));
                //xmlManifest.AddProperty("zipsize", XtoA(it->second.uiZipSize));
            xmlManifest.EndNode();
        }

    xmlManifest.EndNode();

    if (!itFindCompat->second.WriteFile(sOldVersion + _T("/") + _T("manifest.xml"), xmlManifest.GetBuffer()->Get(), xmlManifest.GetBuffer()->GetLength()))
        Console.Err << _T("Couldn't write file ") << _T("manifest.xml") << _T(" in archive ") << itFindCompat->first << newl;

    CFileHandle hVersion(_T("version"), FILE_WRITE | FILE_TEXT | FILE_ASCII, itFindCompat->second);
    hVersion.WriteString(TStringToString(m_sUpdateVersion));
    hVersion.Close();

    for (ZipMap::iterator itMap(mapResFiles.begin()); itMap != mapResFiles.end(); ++itMap)
        itMap->second.Close();

    mapResFiles.clear();

    int iCount(0);
    for (ZipMap::iterator itMap(mapCompatFiles.begin()); itMap != mapCompatFiles.end(); )
    {
        if (itMap->second.GetWriteProgress() == 0.0f)
            Console.Net << _T("[Updater] Writing archive ") << itMap->first << newl;

        float fProgress((float(iCount) + itMap->second.GetWriteProgress()) / mapCompatFiles.size());

        UpdateApplyPercent.Trigger(XtoA(fProgress * fLoadingScale + fLoadingOffset + fLoadingScale));

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();

        if (itMap->second.Close(200))
        {
            ++itMap;
            ++iCount;
        }
    }
}


/*====================
  CUpdater::UpdateCompatArchives
  ====================*/
void    CUpdater::UpdateCompatArchives()
{
    FileManager.DisableCompatArchive();
    CompatWriting.Trigger(_T("1"));

    CArchive compat(_T(":/compat/compat.s2z"), ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);
    if (!compat.IsOpen())
    {
        compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_WRITE | ARCHIVE_TRUNCATE | ARCHIVE_MAX_COMPRESS);

        if (!compat.IsOpen())
        {
            m_bDownloadingCompat = false;
            m_eStatus = UPDATE_STATUS_IDLE;
            return;
        }
    }

    CArchive archive;
    int iFinished(0);

    CFileHandle hVersion(_T("version"), FILE_WRITE | FILE_TEXT | FILE_ASCII, compat);
    hVersion.WriteString(TStringToString(K2System.GetVersionString()));
    hVersion.Close();

    UpdVector::iterator itFile(m_vUpdateFiles.begin());
    while (itFile != m_vUpdateFiles.end())
    {
        bool bSuccess(false);

        tstring sBasePath(_T(":/Update/"));
        
        tstring sNewFilename(itFile->sFile);

        Console.Net << _T("[Updater] Extacting ") << sNewFilename << newl;

        if (FileManager.Exists(_T(":/Update/") + sNewFilename + _T(".zip")))
        {
            CFile *pFileTarget(NULL);
            CCompressedFile cFile;
            int iSize(0);
            
            archive.Open(_T(":/Update/") + sNewFilename + _T(".zip"));

            if (archive.IsOpen() && archive.ContainsFile(archive.GetPathToArchive() + _T("/") + Filename_StripPath(sNewFilename)))
            {
                iSize = archive.GetCompressedFile(archive.GetPathToArchive() + _T("/") + Filename_StripPath(sNewFilename), cFile);

                if (cFile.GetData() != NULL)
                    bSuccess = compat.WriteCompressedFile(itFile->sVersion + _T("/") + itFile->sFile, cFile);
                else
                    Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Could not decompress file.") << newl;
            }
            else
                Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Archive does not contain proper file.") << newl;

            archive.Close();
            SAFE_DELETE(pFileTarget);

            if (bSuccess)
                FileManager.Delete(_T(":/Update/") + sNewFilename + _T(".zip"));
        }
        else
            Console.Net << _T("[Updater] Error unzipping ") + itFile->sFile + _T(": Archive not downloaded properly.") << newl;

        if (!bSuccess && itFile->iRetries < UPDATER_MAX_RETRIES)
        {
            itFile->iRetries++;
            Console.Net << _T("[Updater] Unzipping failed. Retrying request, attempt ") << itFile->iRetries << newl;

            continue;
        }
        else if (!bSuccess)
        {
            Console.Net << _T("[Updater] Unzipping failed. Retries exhausted, update failed.") << newl;

            //CancelApplyUpdate(mapZipFiles);
            assert(0);
            return;
        }

        ++iFinished;

        CompatWritePercent.Trigger(XtoA(float(iFinished) / m_vUpdateFiles.size() * 0.5f));

        K2System.HandleOSMessages();

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();

        ++itFile;
    }

    // Store manifest
    tstring sNewVersion(m_sCompatVersion);
    tsvector vsNewVersion(TokenizeString(sNewVersion, '.'));

    if (vsNewVersion.size() > 1 && (vsNewVersion.size() < 4 || CompareNoCase(vsNewVersion[3], _T("0")) == 0))
        sNewVersion = ConcatinateArgs(vsNewVersion.begin(), vsNewVersion.end() - 1, _T("."));
    else
        sNewVersion = ConcatinateArgs(vsNewVersion, _T("."));

    CXMLDoc xmlManifest(XML_ENCODE_UTF8);

    xmlManifest.NewNode("manifest");

        xmlManifest.AddProperty("version", m_cNewManifest.sVersion);
        xmlManifest.AddProperty("os", m_cNewManifest.sOS);
        xmlManifest.AddProperty("arch", m_cNewManifest.sArch);

        for (ManifestEntryMap::iterator it(m_cNewManifest.mapManifestFiles.begin()); it != m_cNewManifest.mapManifestFiles.end(); ++it)
        {
            xmlManifest.NewNode("file");
                xmlManifest.AddProperty("path", it->first);
                xmlManifest.AddProperty("size", XtoA(it->second.uiSize));
                xmlManifest.AddProperty("checksum", CtoA(it->second.uiChecksum));
                xmlManifest.AddProperty("version", VtoA(it->second.uiVersion));
                //xmlManifest.AddProperty("zipsize", XtoA(it->second.uiZipSize));
            xmlManifest.EndNode();
        }

    xmlManifest.EndNode();

    if (!compat.WriteFile(sNewVersion + _T("/") + _T("manifest.xml"), xmlManifest.GetBuffer()->Get(), xmlManifest.GetBuffer()->GetLength()))
        Console.Err << _T("Couldn't write file ") << _T("manifest.xml") << _T(" in archive ") << compat.GetPath() << newl;

    Console.Net << _T("[Updater] Writing archive ") << compat.GetPath() << newl;

    while (!compat.Close(200))
    {
        float fProgress(compat.GetWriteProgress());

        CompatWritePercent.Trigger(XtoA(fProgress * 0.5f + 0.5f));

        K2System.HandleOSMessages();

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();
    }

    m_bDownloadingCompat = false;
    m_eStatus = UPDATE_STATUS_IDLE;

    CompatWriting.Trigger(_T("0"));
    FileManager.EnableCompatArchive();

    CompatMessage.Trigger(_T("2"));
}


/*====================
  CUpdater::ApplyUpdate
  ====================*/
void    CUpdater::ApplyUpdate()
{
    UpdateMessage.Trigger(_T("10")); // Extracting update, please wait...

    FileManager.DisableCompatArchive();

    Vid.EndFrame();

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();

    UIManager.ClearOverlayInterfaces();

    UIManager.LoadInterface(_T("/ui/updater.interface"));
    UIManager.SetActiveInterface(_T("updater"));

    UpdateApplyPercent.Trigger(XtoA(0.0f));

    cmdWriteConfigScript(host_startupCfg);
    Host.SetNoConfig(true);

    // Unregister/stop anything that may cause permission conflicts while writing out the files
    FileManager.UnregisterArchive(_T("/resources0.s2z"));
    K2SoundManager.StopStreamingImmediately();
    Vid.CloseTextureArchive();
    Host.StopClient(uint(-1));
    if (Host.HasServer() && Host.GetServer() != NULL && Host.GetServer()->GetWorld() != NULL)
        Host.GetServer()->GetWorld()->Free();

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();

    ZipMap mapZipFiles;
    CArchive archive;
    int iFinished(0);

    float fLoadingScale(0.5f);
    float fLoadingOffset(0.0f);

    if (upd_saveCompatArchives)
    {
        AutoUpdateCompatArchives(0.1f, 0.0f);

        // Rescale loading bar to account for compat archive writing
        fLoadingScale = 0.4f;
        fLoadingOffset = 0.2f;
    }

#if 0
    Host.UpdateComplete();

    K2System.RestartOnExit(true);
    K2System.Exit(0);
#endif

    UpdVector::iterator itUnzip(m_vUpdateFiles.begin());
    while (itUnzip != m_vUpdateFiles.end())
    {
        bool bSuccess(false);

        tstring sBasePath;
        
#ifdef linux
        if (itUnzip->sFile == _T("hon_update-x86") || itUnzip->sFile == _T("hon_update-x86_64"))
#elif defined (__APPLE__)
        if (itUnzip->sFile == _T("HoN_Update"))
#else
        if (itUnzip->sFile == _T("hon_update.exe"))
#endif
            sBasePath = _T(":/");
        else
            sBasePath = _T(":/Update/");

        tstring sNewFilename(itUnzip->sFile);

        if (itUnzip->bExtracted)
        {
            Console.Net << _T("[Updater] ") << sNewFilename << _T(" is already extracted") << newl;
            
            bSuccess = true;
        }
        else
        {
            Console.Net << _T("[Updater] Extacting ") << sNewFilename << newl;

            if (FileManager.Exists(_T(":/Update/") + sNewFilename + _T(".zip")))
            {
                CFile *pFileTarget(NULL);
                CCompressedFile cFile;
                char *pBuf;
                int iSize(0);
                
                size_t zPos(itUnzip->sFile.find(_T(".s2z")));
                if (zPos != tstring::npos)
                {
                    // We're working with an archive file
                    ZipMap::iterator findit;
                    int iFlags(ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

                    findit = mapZipFiles.find(itUnzip->sFile.substr(0, zPos + 4));

                    if (findit == mapZipFiles.end())
                    {
                        mapZipFiles.insert(pair<tstring, CArchive>(itUnzip->sFile.substr(0, zPos + 4), CArchive()));
                        findit = mapZipFiles.find(itUnzip->sFile.substr(0, zPos + 4));

                        if (findit == mapZipFiles.end())
                            continue;

                        findit->second.Open(_T(":/") + itUnzip->sFile.substr(0, zPos + 4), iFlags);

                        if (!findit->second.IsOpen())
                            findit->second.Open(_T(":/") + itUnzip->sFile.substr(0, zPos + 4), iFlags & ~ARCHIVE_APPEND);
                    }

                    if (findit->second.IsOpen())
                    {
                        archive.Open(_T(":/Update/") + sNewFilename + _T(".zip"));

                        tstring sFilename(LowerString(_T(":/Update/") + Filename_GetPath(sNewFilename) + Filename_GetPath(itUnzip->sFile.substr(0, zPos)) + itUnzip->sFile.substr(zPos + 5)));

                        if (archive.IsOpen() && archive.ContainsFile(archive.GetPathToArchive() + _T("/") + Filename_StripPath(sFilename)))
                        {
                            iSize = archive.GetCompressedFile(archive.GetPathToArchive() + _T("/") + Filename_StripPath(sFilename), cFile);

                            if (cFile.GetData() != NULL)
                            {
                                if (findit->second.IsOpen())
                                    bSuccess = findit->second.WriteCompressedFile(itUnzip->sFile.substr(zPos + 5), cFile);
                                else
                                    Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Could not write to target file.") << newl;
                            }
                            else
                                Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Could not decompress file.") << newl;
                        }
                        else
                            Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Archive does not contain proper file.") << newl;
                    }
                    else
                        Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Target archive could not be opened for writing.") << newl;

                    archive.Close();
                    SAFE_DELETE(pFileTarget);

                    if (bSuccess)
                        FileManager.Delete(_T(":/Update/") + sNewFilename + _T(".zip"));
                }
                else
                {
                    pFileTarget = FileManager.GetFile(sBasePath + itUnzip->sFile, FILE_WRITE | FILE_BINARY);

                    archive.Open(_T(":/Update/") + itUnzip->sFile + _T(".zip"));

                    if (archive.ContainsFile(LowerString(_T(":/Update/") + itUnzip->sFile)))
                    {
                        iSize = archive.ReadFile(LowerString(_T(":/Update/") + itUnzip->sFile), pBuf);

                        if (pBuf != NULL)
                        {
                            if (pFileTarget != NULL)
                            {
                                pFileTarget->Write(pBuf, iSize);
                                pFileTarget->Close();

                                bSuccess = true;
                            }
                            else
                                Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Could not write to target file.") << newl;
                        }
                        else
                            Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Could not decompress file.") << newl;

                        SAFE_DELETE_ARRAY(pBuf);
                    }
                    else
                        Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Archive does not contain proper file.") << newl;

                    archive.Close();
                    SAFE_DELETE(pFileTarget);

                    if (bSuccess)
                        FileManager.Delete(_T(":/Update/") + itUnzip->sFile + _T(".zip"));
                }
            }
            else
                Console.Net << _T("[Updater] Error unzipping ") + itUnzip->sFile + _T(": Archive not downloaded properly.") << newl;

            if (!bSuccess && itUnzip->iRetries < UPDATER_MAX_RETRIES)
            {
                itUnzip->iRetries++;
                Console.Net << _T("[Updater] Unzipping failed. Retrying request, attempt ") << itUnzip->iRetries << newl;

                continue;
            }
            else if (!bSuccess)
            {
                Console.Net << _T("[Updater] Unzipping failed. Retries exhausted, update failed.") << newl;

                CancelApplyUpdate(mapZipFiles);
                return;
            }
        }

        ++iFinished;

        UpdatePercent.Trigger(_T("2"));

        UpdateApplyPercent.Trigger(XtoA(float(iFinished) / m_vUpdateFiles.size() * fLoadingScale + fLoadingOffset));

        K2System.HandleOSMessages();

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();

        ++itUnzip;
    }

    DelVector::iterator itDel(m_vDeleteFiles.begin());
    while (itDel != m_vDeleteFiles.end())
    {
        tstring sBasePath(_T(":/"));

        tstring sNewFilename(itDel->sFile);

        Console.Net << _T("[Updater] Deleting ") << sNewFilename << newl;

        size_t zPos(itDel->sFile.find(_T(".s2z")));
        if (zPos != tstring::npos)
        {
            // We're working with an archive file
            ZipMap::iterator findit;
            int iFlags(ARCHIVE_WRITE | ARCHIVE_APPEND | ARCHIVE_MAX_COMPRESS);

            findit = mapZipFiles.find(itDel->sFile.substr(0, zPos + 4));

            if (findit == mapZipFiles.end())
            {
                mapZipFiles.insert(pair<tstring, CArchive>(itDel->sFile.substr(0, zPos + 4), CArchive()));
                findit = mapZipFiles.find(itDel->sFile.substr(0, zPos + 4));

                if (findit == mapZipFiles.end())
                    continue;

                findit->second.Open(_T(":/") + itDel->sFile.substr(0, zPos + 4), iFlags);

                if (!findit->second.IsOpen())
                    findit->second.Open(_T(":/") + itDel->sFile.substr(0, zPos + 4), iFlags & ~ARCHIVE_APPEND);
            }

            if (findit->second.IsOpen())
            {
                if (!findit->second.DeleteCompressedFile(itDel->sFile.substr(zPos + 5)))
                    Console.Net << _T("[Updater] Error deleting ") + itDel->sFile << newl;
            }
            else
                Console.Net << _T("[Updater] Error deleting ") + itDel->sFile + _T(": Target archive could not be opened for writing.") << newl;
        }
        else
        {
            if (!FileManager.Delete(sBasePath + itDel->sFile))
                Console.Net << _T("[Updater] Error deleting ") + itDel->sFile << newl;
        }

        ++itDel;
    }

    UpdateMessage.Trigger(_T("11")); // Writing archives to disk...
    UpdatePercent.Trigger(_T("2"));

    int iCount(0);
    for (ZipMap::iterator itMap(mapZipFiles.begin()); itMap != mapZipFiles.end(); )
    {
        if (itMap->second.GetWriteProgress() == 0.0f)
            Console.Net << _T("[Updater] Writing archive ") << itMap->first << newl;

        float fProgress((float(iCount) + itMap->second.GetWriteProgress()) / mapZipFiles.size());

        UpdateApplyPercent.Trigger(XtoA(fProgress * fLoadingScale + fLoadingOffset + fLoadingScale));

        Vid.BeginFrame();
        Vid.SetColor(BLACK);
        Vid.Clear();
        UIManager.Frame(Host.GetFrameLength());
        Vid.EndFrame();

        if (itMap->second.Close(200))
        {
            ++itMap;
            ++iCount;
        }
    }

    CFile *pVerify(FileManager.GetFile(_T(":/Update/verify"), FILE_WRITE | FILE_BINARY | FILE_TRUNCATE));

    if (pVerify != NULL)
        pVerify->Close();
    
#ifdef linux
    {
        struct stat buf;
        if (stat("hon_update-x86", &buf) == 0)
            chmod("hon_update-x86", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
        if (stat("hon_update-x86_64", &buf) == 0)
            chmod("hon_update-x86_64", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
    }
#endif
#ifdef __APPLE__
    {
        struct stat buf;
        if (stat("HoN_Update", &buf) == 0)
            chmod("HoN_Update", (buf.st_mode & 07777) | S_IXUSR | S_IXGRP | S_IXOTH);
    }
#endif

    Console.Net << _T("[Updater] Update complete.") << newl;
    UpdateMessage.Trigger(_T("12")); // Update complete.
    UpdatePercent.Trigger(_T("1"));
    UpdateApplyPercent.Trigger(_T("1"));
    UpdateError.Trigger(_T("0"));

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();
    
    m_bCheckingForUpdate = false;
    m_eStatus = UPDATE_STATUS_IDLE;

    Host.UpdateComplete();

    K2System.RestartOnExit(true);
    K2System.Exit(0);
}


/*====================
  CUpdater::DownloadCompat
  ====================*/
void    CUpdater::DownloadCompat(const tstring &sVersion)
{
    if (m_bDownloadingCompat || m_bCheckingForUpdate)
        return;

    m_bDownloadingCompat = true;

    m_sCompatVersion = K2_Version3(sVersion) + _T(".0");

    CompatCalculating.Trigger(_T("0"));
    CompatDownloading.Trigger(_T("0"));
    CompatMessage.Trigger(_T("1")); // Searching for updates...
    CompatPercent.Trigger(_T("0"));
    CompatWriting.Trigger(_T("0"));

    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
    m_pRequest = Host.GetHTTPManager()->SpawnRequest();
    if (m_pRequest == NULL)
        return;

    m_eStatus = UPDATE_STATUS_RETRIEVING_COMPAT_FILES;

    m_pRequest->SetTargetURL(m_sMasterServerURL);
    m_pRequest->AddVariable(_T("version"), _T("0.0.0.0"));
    m_pRequest->AddVariable(_T("os"), K2System.GetBuildOSCodeString());
    m_pRequest->AddVariable(_T("arch"), K2System.GetBuildArchString());
    m_pRequest->SendPostRequest();

    m_vUpdateFiles.clear();
    m_vDeleteFiles.clear();
    m_uiTotalSize = 0;
    m_uiCurDownloaded = 0;
    m_uiLastSpeedUpdate = -1;
    m_uiNumActive = 0;
    m_uiNumSpeedUpdates = 1;
    m_uiTotalResumed = 0;
    m_uiTotalFiles = 0;
    m_uiTotalSized = 0;
    m_uiFirstSpeedUpdate = 0;
    m_uiNumComplete = 0;
}


/*====================
  CUpdater::CompatResponseError
  ====================*/
void    CUpdater::CompatResponseError()
{
    // Error checking for updates.
    //UpdateMessage.Trigger(_T("5")); // Error checking for updates.
    //UpdatePercent.Trigger(_T("1"));
    //UpdateError.Trigger(_T("1"));
    CompatWriting.Trigger(_T("0"));

    m_bDownloadingCompat = false;

    m_eStatus = UPDATE_STATUS_IDLE;
}


/*====================
  CUpdater::CompatResponse
  ====================*/
void    CUpdater::CompatResponse(CPHPData &response)
{
    tstring sResponseVersion(response.GetString(_T("version")));

    if (sResponseVersion != K2System.GetVersionString())
    {
        // Not Current version
        CompatMessage.Trigger(_T("5")); // No updates found.
        CompatPercent.Trigger(_T("1"));
        CompatDownloading.Trigger(_T("0"));
        CompatCalculating.Trigger(_T("0"));
        //CompatError.Trigger(_T("1"));

        m_bDownloadingCompat = false;

        m_eStatus = UPDATE_STATUS_IDLE;

        return;
    }

    // Delete patch verification file
    if (FileManager.Exists(_T(":/Update/verify")))
        FileManager.Delete(_T(":/Update/verify"));

    sset setIgnore;

    CFile *pFile(NULL);

    pFile = FileManager.GetFile(_T(":/compat_ignore.txt"), FILE_NOUSERDIR | FILE_NOARCHIVES | FILE_NOWORLDARCHIVE | FILE_TOPMODONLY | FILE_READ | FILE_TEXT);

    if (pFile != NULL)
    {
        tstring sFilename;

        while (!pFile->IsEOF())
        {
            sFilename = pFile->ReadLine();

            StripNewline(sFilename);
            sFilename = LowerString(sFilename);

            if (!sFilename.empty() && setIgnore.find(sFilename) == setIgnore.end())
                setIgnore.insert(sFilename);
        }
    }

    tstring sNewVersion(K2_Version(m_sCompatVersion));

    CompatVersion.Trigger(sNewVersion);
    CompatCalculating.Trigger(_T("1"));
    CompatDownloading.Trigger(_T("0"));
    //CompatError.Trigger(_T("0"));
    CompatPercent.Trigger(_T("0"));
    CompatWriting.Trigger(_T("0"));

    Vid.BeginFrame();
    Vid.SetColor(BLACK);
    Vid.Clear();
    UIManager.Frame(Host.GetFrameLength());
    Vid.EndFrame();

    bool bValid(false);
    tstring sUrl;
    tstring sUrl2;

    // Grab version info
    const CPHPData *pVersion(response.GetVar(0));
    if (pVersion != NULL)
    {
        bValid = true;
        m_sPrimaryServer = pVersion->GetString(_T("url"));
        m_sSecondaryServer = pVersion->GetString(_T("url2"));
    }

    if (!bValid || m_sPrimaryServer.empty() || m_sSecondaryServer.empty())
    {
        Console.Net << _T("[Updater] Error in version response") << newl;
        ResponseVersionError();
        return;
    }

    m_cNewManifest = SFileManifest();
    m_cOldManifest = SFileManifest();

    tstring sOldVersion(K2_Version(K2System.GetVersionString()));

    int i(0);

    i = 0;
    for (; i < 2; ++i)
    {
        if (!sOldVersion.empty() &&
            m_fileHTTP.Open((i == 0 ? m_sPrimaryServer : m_sSecondaryServer) + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sOldVersion + _T("/manifest.xml.zip"), FILE_BLOCK) &&
            !m_fileHTTP.ErrorEncountered())
        {
            uint uiSize;
            const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));

            CMMapUnzip cManifestZip(pBuffer, uiSize);

            char *pManifestBuffer;
            uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));

            if (uiManifestSize > 0)
            {
                bool bParsed(XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &m_cOldManifest));

                K2_DELETE_ARRAY(pManifestBuffer);

                if (bParsed)
                {
                    break; // Success!
                }
                else
                {
                    Console.Net << _T("[Updater] Failure parsing manifest: ") << sOldVersion << newl;
                    continue;
                }
            }
            else
            {
                Console.Net << _T("[Updater] Empty manifest: ") << sOldVersion << newl;
                continue;
            }
        }
    }
    if (i == 2)
    {
        Console.Net << _T("[Updater] Failed to retrieve manifest: ") << sOldVersion << newl;
        CompatResponseError();
        return;
    }

    i = 0;
    for (; i < 2; ++i)
    {
        if (!sNewVersion.empty() &&
            m_fileHTTP.Open((i == 0 ? m_sPrimaryServer : m_sSecondaryServer) + K2System.GetBuildOSCodeString() + _T("/") + K2System.GetBuildArchString() + _T("/") + sNewVersion + _T("/manifest.xml.zip"), FILE_BLOCK) &&
            !m_fileHTTP.ErrorEncountered())
        {
            uint uiSize;
            const char *pBuffer(m_fileHTTP.GetBuffer(uiSize));

            CMMapUnzip cManifestZip(pBuffer, uiSize);

            char *pManifestBuffer;
            uint uiManifestSize(cManifestZip.OpenUnzipFile(_T("manifest.xml"), pManifestBuffer));

            if (uiManifestSize > 0)
            {
                bool bParsed(XMLManager.ReadBuffer(pManifestBuffer, uiManifestSize, _T("manifest"), &m_cNewManifest));
                
                K2_DELETE_ARRAY(pManifestBuffer);

                if (bParsed)
                {
                    break; // Success!
                }
                else
                {
                    Console.Net << _T("[Updater] Failure parsing manifest: ") << sNewVersion << newl;
                    continue;
                }
            }
            else
            {
                Console.Net << _T("[Updater] Empty manifest: ") << sNewVersion << newl;
                continue;
            }
        }
    }
    if (i == 2)
    {
        Console.Net << _T("[Updater] Failed to retrieve manifest: ") << sNewVersion << newl;
        CompatResponseError();
        return;
    }

    FileManager.DisableCompatArchive();

    CArchive compat(_T(":/compat/compat.s2z"), ARCHIVE_READ);

    if (compat.IsOpen())
    {
        CFileHandle hVersion(_T("version"), FILE_READ | FILE_TEXT | FILE_ASCII, compat);
        if (!hVersion.IsOpen())
        {
            compat.Close();
            compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_WRITE | ARCHIVE_TRUNCATE);

            compat.Close();
            compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_READ);

        }
        else
        {
            tstring sArchiveVersion;

            byte y(hVersion.ReadByte());
            while (y && y != _T('\n') && y != _T('\r'))
            {
                sArchiveVersion += TCHAR(y);
                y = hVersion.ReadByte();
            }

            if (sArchiveVersion != K2System.GetVersionString())
            {
                compat.Close();
                compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_WRITE | ARCHIVE_TRUNCATE);

                compat.Close();
                compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_READ);
            }
        }
    }
    else
    {
        compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_WRITE | ARCHIVE_TRUNCATE);

        compat.Close();
        compat.Open(_T(":/compat/compat.s2z"), ARCHIVE_READ);
    }

    if (!compat.IsOpen())
    {
        CompatResponseError();
        return;
    }

    for (ManifestEntryMap::iterator it(m_cNewManifest.mapManifestFiles.begin()); it != m_cNewManifest.mapManifestFiles.end(); ++it)
    {
        ManifestEntryMap::iterator itOld(m_cOldManifest.mapManifestFiles.find(it->first));

        if (itOld != m_cOldManifest.mapManifestFiles.end() &&
            it->second.uiSize == itOld->second.uiSize &&
            it->second.uiChecksum == itOld->second.uiChecksum &&
            it->second.uiVersion == itOld->second.uiVersion)
            continue;

        tstring sLower(LowerString(_T(":/") + it->first));

        bool bIgnored(false);
        for (sset::iterator itIgnore(setIgnore.begin()); itIgnore != setIgnore.end(); ++itIgnore)
        {
            if (EqualsWildcardNoCase(*itIgnore, sLower))
            {
                bIgnored = true;
                break;
            }
        }

        if (bIgnored)
            continue;

        tstring sVersion(VtoA2(it->second.uiVersion));

        if (compat.ContainsFile(compat.GetPathToArchive() + _T("/") + sVersion + _T("/") + it->first))
            continue;

        GetUpdate(it->first, VtoA2(it->second.uiVersion), m_cNewManifest.sOS, m_cNewManifest.sArch, it->second.uiZipSize, it->second.uiSize, it->second.uiChecksum, sOldVersion);
    }

    compat.Close();

    if (!m_vUpdateFiles.empty())
    {
        m_eStatus = UPDATE_STATUS_DOWNLOADING_COMPAT;

        for (UpdVector::iterator it(m_vUpdateFiles.begin()); it != m_vUpdateFiles.end(); ++it)
        {
            ++m_uiTotalSized;
            m_uiTotalSize += it->uiSize;
        }

        CleanupUpdateFiles();
    
        // Get the update
        CompatMessage.Trigger(_T("4")); // Preparing to download update...
        CompatDownloading.Trigger(_T("1"));
        CompatCalculating.Trigger(_T("0"));
    }
    else
    {
        // No files to download, we're at the newest version.
        CompatMessage.Trigger(_T("0")); // No updates found.
        CompatDownloading.Trigger(_T("0"));
        CompatCalculating.Trigger(_T("0"));
        CompatPercent.Trigger(_T("2"));
        //CompatError.Trigger(_T("0"));

        m_bDownloadingCompat = false;

        m_eStatus = UPDATE_STATUS_IDLE;
    }
}


/*====================
  CUpdater::CleanupUpdateFiles
  ====================*/
void    CUpdater::CleanupUpdateFiles()
{
    vector<tstring> vExistingFiles;

    FileManager.GetFileList(_T(":/Update"), _T("*"), true, vExistingFiles, true);

    for (vector<tstring>::iterator it(vExistingFiles.begin()), itEnd(vExistingFiles.end()); it != itEnd; ++it)
    {
        if (LowerString(*it) == _T(":/update/manifest.xml"))
            continue;

        if (it->length() < 4)
            continue;

        tstring sSearchFile;

        if (LowerString(Filename_GetExtension(*it)) == _T("zip"))
            sSearchFile = it->substr(9, it->length() - 13);
        else
            sSearchFile = it->substr(9);

        bool bFound(false);
        for (UpdVector::iterator itUpd(m_vUpdateFiles.begin()), itUpdEnd(m_vUpdateFiles.end()); itUpd != itUpdEnd; ++itUpd)
        {
            if (itUpd->sFile == sSearchFile)
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            Console.Net << _T("[Updater] Deleting unneeded file: ") << sSearchFile << newl;

            FileManager.Delete(*it);
        }
    }
}


/*====================
  CUpdater::FrameAwaitingCompatResponse

  We are currently trying to get data from the update script
  ====================*/
void    CUpdater::FrameAwaitingCompatResponse()
{
    if (m_pRequest == NULL || m_pRequest->IsActive())
        return;

    if (!m_pRequest->WasSuccessful())
    {
        Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
        m_pRequest = NULL;
        CompatResponseError();
        return;
    }

    CPHPData response(m_pRequest->GetResponse());
    Host.GetHTTPManager()->ReleaseRequest(m_pRequest);
    m_pRequest = NULL;

    if (response.GetString(_T("version")) != _T(""))
        CompatResponse(response);
    else
        CompatResponseError();
}


/*====================
  CUpdater::Frame
  ====================*/
void    CUpdater::Frame()
{
    if (m_eStatus == UPDATE_STATUS_RETRIEVING_FILES || m_eStatus == UPDATE_STATUS_SILENT)
    {
        // If we are currently trying to get data from the update script
        FrameAwaitingResponse();
    }
    else if (m_eStatus == UPDATE_STATUS_RETRIEVING_COMPAT_FILES)
    {
        FrameAwaitingCompatResponse();
    }
    else if (m_eStatus == UPDATE_STATUS_DOWNLOADING || m_eStatus == UPDATE_STATUS_DOWNLOADING_COMPAT)
    {
        // If we aren't waiting for a file URL from the script,
        // we're ready to download.
        FrameDownloading();
    }
}

/*====================
  CUpdater::ForceUpdate
  ====================*/
void    CUpdater::ForceUpdate(const tstring &sVersion, const tstring &sPrimaryServer, const tstring &sSecondaryServer, bool bRequireConfirmation)
{
    if (sVersion.empty() || sPrimaryServer.empty() || sSecondaryServer.empty())
        return;

    m_sUpdateVersion = sVersion;
    m_sPrimaryServer = sPrimaryServer;
    m_sSecondaryServer = sSecondaryServer;
    m_bRequireConfirmation = bRequireConfirmation;

    if (m_bRequireConfirmation)
    {
        m_eStatus = UPDATE_STATUS_NEEDS_UPDATE;

        UpdateStatus.Trigger(XtoA(m_eStatus));
        UpdateCurVersion.Trigger(m_sUpdateVersion);

        m_bCheckingForUpdate = false;
    }
    else
    {
        StartUpdate();
    }
}


// <manifest>
namespace XMLManifest
{
    DECLARE_XML_PROCESSOR(manifest);
    BEGIN_XML_REGISTRATION(manifest)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(manifest, SFileManifest)
        try
        {
            pObject->sVersion = node.GetProperty(_T("version"));
            pObject->sOS = node.GetProperty(_T("os"));
            pObject->sArch = node.GetProperty(_T("arch"));
        }
        catch (CException &ex)
        {
            ex.Process(_T("<manifest> - "));
            return false;
        }
    END_XML_PROCESSOR(pObject)

    // <file>
    DECLARE_XML_PROCESSOR(file)
    BEGIN_XML_REGISTRATION(file)
        REGISTER_XML_PROCESSOR(manifest)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(file, SFileManifest)
        try
        {
            static tstring SIZE(_T("size"));
            static tstring CHECKSUM(_T("checksum"));
            static tstring VERSION(_T("version"));
            static tstring ZIPSIZE(_T("zipsize"));
            static tstring PATH(_T("path"));

            SManifestEntry cFile;
            cFile.uiSize = node.GetPropertyInt(SIZE);
            cFile.uiChecksum = HexAtoI(node.GetProperty(CHECKSUM));

            const tstring &sVersion(node.GetProperty(VERSION));
            tsvector vsVersion(TokenizeString(sVersion, _T('.')));

            if (vsVersion.size() == 4)
                cFile.uiVersion = (AtoI(vsVersion[0]) << 24) + (AtoI(vsVersion[1]) << 16) + (AtoI(vsVersion[2]) << 8) + AtoI(vsVersion[3]);

            cFile.uiZipSize = node.GetPropertyInt(ZIPSIZE);

            const tstring &sPath(node.GetProperty(PATH));

            pObject->mapManifestFiles[sPath] = cFile;
        }
        catch (CException &ex)
        {
            ex.Process(_T("<file> - "));
            return false;
        }
    END_XML_PROCESSOR(NULL)
}


/*--------------------
  CheckForUpdates
  --------------------*/
UI_VOID_CMD(CheckForUpdates, 0)
{
    K2Updater.CheckForUpdates(false);
}

CMD(CheckForUpdates)
{
    K2Updater.CheckForUpdates(false);
    return true;
}


/*--------------------
  CancelUpdate
  --------------------*/
UI_VOID_CMD(CancelUpdate, 0)
{
    K2Updater.CancelUpdate();
}


/*--------------------
  GenerateUpdate
  --------------------*/
CMD(GenerateUpdate)
{
    bool bResourceFilesOnly(false);
    if (vArgList.size() >= 1)
    {
        bResourceFilesOnly = AtoB(vArgList[0]);
        Console << _T("Generating resource archives only.") << newl;
    }

    K2Updater.GenerateUpdate(bResourceFilesOnly);
    return true;
}


/*--------------------
  UploadUpdate
  --------------------*/
CMD(UploadUpdate)
{
    if (vArgList.size() < 3)
        return false;

    K2Updater.UploadUpdate(vArgList[0], vArgList[1], vArgList[2]);
    return true;
}


/*--------------------
  UploadPrevious
  --------------------*/
CMD(UploadPrevious)
{
    if (vArgList.size() < 3)
        return false;

    K2Updater.UploadPrevious(vArgList[0], vArgList[1], vArgList[2]);
    return true;
}


/*--------------------
  UpdateDB
  --------------------*/
CMD(UpdateDB)
{
    if (vArgList.size() < 2)
        return false;

    K2Updater.UpdateDB(vArgList[0], vArgList[1]);
    return true;
}


/*--------------------
  GenerateIgnore
  --------------------*/
CMD(GenerateIgnore)
{
    if (vArgList.size() < 1)
        return false;

    K2Updater.GenerateIgnore(ConcatinateArgs(vArgList));

    return true;
}


/*--------------------
  DownloadCompat
  --------------------*/
CMD(DownloadCompat)
{
    if (vArgList.size() < 1)
        return false;

    K2Updater.DownloadCompat(vArgList[0]);
    return true;
}


/*--------------------
  DownloadCompat
  --------------------*/
UI_VOID_CMD(DownloadCompat, 1)
{
    K2Updater.DownloadCompat(vArgList[0]->Evaluate());
}


/*--------------------
  CancelCompatDownload
  --------------------*/
UI_VOID_CMD(CancelCompat, 0)
{
    K2Updater.CancelCompatDownload();
}


/*--------------------
  UpdateConfirm
  --------------------*/
UI_VOID_CMD(UpdateConfirm, 1)
{
    K2Updater.UpdateConfirm(AtoB(vArgList[0]->Evaluate()));
}


/*--------------------
  ForceUpdate
  --------------------*/
CMD(ForceUpdate)
{
    if (vArgList.size() < 3)
        return false;

    K2Updater.ForceUpdate(vArgList[0], vArgList[1], vArgList[2], vArgList.size() > 3 ? AtoB(vArgList[3]) : false);
    return true;
}
