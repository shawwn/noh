// (C)2005 S2 Games
// c_archive.cpp
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_archive.h"
#include "c_mmapunzip.h"
#include "c_zip.h"
#include "c_filemanager.h"
#include "c_checksumtable.h"
#include "stringutils.h"
//=============================================================================

// public static variables.
bool        CArchive::ExamineChecksums = true;

/*====================
  CArchive::CArchive
  ====================*/
CArchive::CArchive()
{
}

/*====================
  CArchive::CArchive
  ====================*/
CArchive::CArchive(const tstring &sPath, int iMode)
{
    Open(sPath, iMode);
}


/*====================
  CArchive::~CArchive
  ====================*/
CArchive::~CArchive()
{
    Close();

    SAFE_DELETE(m_pZipFile);
    SAFE_DELETE(m_pUnzipFile);
    SAFE_DELETE(m_pChecksums);
}


/*====================
  CArchive::ValidateFileChecksum
  ====================*/
bool    CArchive::ValidateFileChecksum(const tstring &sPath)
{
    if (!m_pUnzipFile)
        return false;

    tstring sFullPath(FileManager.IsCleanPath(sPath) ? sPath : FileManager.SanitizePath(sPath, false));
    if (sFullPath.compare(0, m_sPathToArchive.length(), m_sPathToArchive) != 0)
        return false;

    // read the file.
    char* pBuffer(NULL);
    int iSize(m_pUnzipFile->OpenUnzipFile(sFullPath.substr(m_sPathToArchive.length()), pBuffer));

    // if it's invalid, abort.
    if (iSize <= 0 || pBuffer == NULL)
        goto invalid;

    if (m_pChecksums)
    {
        // compute the file's checksum.
        byte yChecksum[CHECKSUM_SIZE];
        if (!CChecksumTable::ComputeChecksum(yChecksum, (const byte*)pBuffer, iSize))
        {
            Console << _T("Could not compute checksum for file ") << sPath << newl;
            goto invalid;
        }

        if (!m_pChecksums->Compare(sPath, yChecksum))
        {
            goto invalid;
        }
    }

    return true;

invalid:
    SAFE_DELETE_ARRAY(pBuffer);
    return false;
}


/*====================
  CArchive::Open
  ====================*/
bool    CArchive::Open(const tstring &sPath, int iMode, const tstring &sMod)
{
    if (IsOpen())
        Close();

    // check for conflicting mode flags
    if (((iMode & ARCHIVE_READ) && (iMode & ARCHIVE_WRITE)) ||
        ((iMode & ARCHIVE_APPEND) && (iMode & ARCHIVE_TRUNCATE)))
    {
        Console.Warn << _T("Conflicting mode flags opening archive ") << sPath << newl;
        return false;
    }

    m_iMode = iMode;

    tstring sBasePath;

    // store and compare paths in lowercase
    m_sPath = LowerString(FileManager.SanitizePath(sPath));
    m_sPathToArchive = m_sPath.substr(0, m_sPath.find_last_of(_T("/")) + 1);
    m_sBasePath = sBasePath;
    m_sMod = sMod.empty() ? FileManager.GetTopModPath() : sMod;

    if (m_iMode & ARCHIVE_READ)
    {
        tstring sPathDir(FileManager.GetSystemPath(Filename_StripExtension(FileManager.SanitizePath(sPath)) + _T("/"), sMod, false, false, false, &sBasePath));
#if defined(linux) || defined(__APPLE__)
        // need to check ~sPath as well since maps are saved/downloaded to user dir and mod s2z files can reside there
        if (sPathDir.empty() && sPath[0] != _T('~') && sPath[0] != _T('#')  && sPath[0] != _T(':'))
            sPathDir = FileManager.GetSystemPath(Filename_StripExtension(FileManager.SanitizePath(_T("~") + sPath)) + _T("/"));
#endif

        tstring sPathCopy(FileManager.GetSystemPath(FileManager.SanitizePath(sPath), sMod, false, false, false, &sBasePath));

#if defined(linux) || defined(__APPLE__)
        // need to check ~sPath as well since maps are saved/downloaded to user dir and mod s2z files can reside there
        if (sPathCopy.empty() && sPath[0] != _T('~') && sPath[0] != _T('#')  && sPath[0] != _T(':'))
            sPathCopy = FileManager.GetSystemPath(FileManager.SanitizePath(_T("~") + sPath));
#endif

        if (sPathCopy.empty() && !sPathDir.empty()) {
            sPathCopy = TrimRight(sPathDir, _T("/")) + sPath.substr(sPath.find_last_of(_T(".")));
        }

        if (sPathCopy.empty())
            return false;

        m_pUnzipFile = K2_NEW(ctx_FileSystem,  CMMapUnzip)(sPathCopy);

        m_sCompleteDiskPath = sPathCopy;

        // try to read the checksums table.
        tstring sChecksumsPath(_T("checksums"));
        if (ContainsFile(sChecksumsPath))
        {
            CFileHandle cChecksumsFile(sChecksumsPath, FILE_READ | FILE_BINARY, *this);
            if (cChecksumsFile.IsOpen())
            {
                uint uiChecksumsFileSize((uint)cChecksumsFile.GetLength());
                if (uiChecksumsFileSize > 0)
                {
                    bool bLoadedChecksumsFile(false);
                    byte *pChecksumsFileBuf = K2_NEW_ARRAY(ctx_FileSystem, byte, uiChecksumsFileSize);
                    if (cChecksumsFile.Read((char*)pChecksumsFileBuf, uiChecksumsFileSize) == uiChecksumsFileSize)
                    {
                        assert(m_pChecksums == NULL);
                        m_pChecksums = K2_NEW(ctx_FileSystem,  CChecksumTable)();
                        if (m_pChecksums->Load(pChecksumsFileBuf, uiChecksumsFileSize))
                        {
                            bLoadedChecksumsFile = true;
                        }
                    }
                    SAFE_DELETE_ARRAY(pChecksumsFileBuf);

                    if (!bLoadedChecksumsFile)
                    {
                        Console << _T("Failed to load checksums table for archive ") << sPathCopy << newl;
                        SAFE_DELETE(m_pChecksums);
                    }
                }
            }
        }

        if (ExamineChecksums && !m_pChecksums && m_bRequireChecksums)
        {
            // disable this for now.
            //FileManager.CoreFilesModified(sChecksumsPath);
            Console.Err << _T("MODIFIED RESOURCES0.S2Z DETECTED for file ") << sPath << newl;
        }
    }
    else if (m_iMode & ARCHIVE_WRITE)
    {
        // default behavior is to truncate
        if (!(iMode & ARCHIVE_APPEND))
            m_iMode |= ARCHIVE_TRUNCATE;

        if ((m_iMode & ARCHIVE_TRUNCATE) && FileManager.Exists(sPath))
            FileManager.Delete(sPath);

        m_pZipFile = K2_NEW(ctx_FileSystem,  CZip)(sPath, (iMode & ARCHIVE_APPEND) != 0);

        if (m_pZipFile == NULL)
            return false;

        if (!m_pZipFile->IsOpen())
        {
            Console.Warn << _T("Failed to create archive") << newl;
            SAFE_DELETE(m_pZipFile);
            return false;
        }
    }
    return true;
}


/*====================
  CArchive::Close
  ====================*/
bool    CArchive::Close(uint uiMaxTime)
{
    if (m_pZipFile != NULL)
    {
        if (!m_pZipFile->Close(uiMaxTime))
            return false;
        else
            SAFE_DELETE(m_pZipFile);
    }

    SAFE_DELETE(m_pUnzipFile);
    SAFE_DELETE(m_pChecksums);

    // TODO: Maybe not? pFileManager->UnregisterArchive(m_sPath);
    // TODO: close all filehandles that reference me

    return true;
}


/*====================
  CArchive::CancelWrite
  ====================*/
bool    CArchive::CancelWrite()
{
    if (m_pZipFile != NULL)
    {
        if (!m_pZipFile->CancelWrite())
            return false;
        else
            SAFE_DELETE(m_pZipFile);
    }

    SAFE_DELETE(m_pUnzipFile)
    return true;
}


/*====================
  CArchive::IsOpen
  ====================*/
bool    CArchive::IsOpen() const
{
    if (m_iMode & ARCHIVE_READ)
        return m_pUnzipFile != NULL;
    else if (m_iMode & ARCHIVE_WRITE)
        return m_pZipFile != NULL;
    else
        return false;
}


/*====================
  CArchive::ReadFile

  Creates a buffer for the specified file and decompresses it there
  ====================*/
int     CArchive::ReadFile(const tstring &sPath, char *&pBuffer)
{
    if (!m_pUnzipFile)
        return 0;

    tstring sFullPath(FileManager.IsCleanPath(sPath) ? sPath : FileManager.SanitizePath(sPath, false));
    if (sFullPath.compare(0, m_sPathToArchive.length(), m_sPathToArchive) != 0)
        return 0;

    // read the file.
    int iSize(m_pUnzipFile->OpenUnzipFile(sFullPath.substr(m_sPathToArchive.length()), pBuffer));

    // if it's invalid, abort.
    if (iSize <= 0 || pBuffer == NULL)
        return iSize;

    if (!FileManager.GetCoreFilesModified())
    {
        if (m_bRequireChecksums && m_pChecksums == NULL)
        {
            if (ExamineChecksums && sPath != _T("checksums"))
            {
                // disable this for now.
                //FileManager.CoreFilesModified(_T("checksums"));
                Console.Err << _T("MODIFIED RESOURCES0.S2Z DETECTED for file ") << sPath << newl;
            }
        }

        if (m_pChecksums)
        {
            bool bValidChecksum(true);

            // compute the file's checksum.
            byte yChecksum[CHECKSUM_SIZE];
            if (!CChecksumTable::ComputeChecksum(yChecksum, (const byte*)pBuffer, iSize))
            {
                Console << _T("Could not compute checksum for file ") << sPath << newl;
                bValidChecksum = false;
            }
            else if (ExamineChecksums && !m_pChecksums->Compare(sPath, yChecksum))
            {
                Console << _T("Checksum for file ") << sPath << _T(" does not match!") << newl;
                bValidChecksum = false;
            }

            if (ExamineChecksums && !bValidChecksum)
            {
                // disable this for now.
                //FileManager.CoreFilesModified(sPath);
                Console.Err << _T("MODIFIED RESOURCES0.S2Z DETECTED for file ") << sPath << newl;
            }
        }
    }

    return iSize;
}


/*====================
  CArchive::StopFilePreload
  ====================*/
void    CArchive::StopFilePreload(const tstring &sFilename)
{
    if (m_pUnzipFile == NULL)
        return;
#if 0
    m_pUnzipFile->StopFilePreload(sFilename);
#endif
}


/*====================
  CArchive::ContainsFile
  ====================*/
bool    CArchive::ContainsFile(const tstring &sPath)
{
    tstring sFullPath(FileManager.IsCleanPath(sPath) ? sPath : FileManager.SanitizePath(sPath, false));

    auto sFilePath(Filename_StripExtension(m_sPath) + _T("/") + sFullPath);
    const tstring &sSystemPath(FileManager.GetSystemPath(sFilePath, m_sMod, false, false, true));
    if (!sSystemPath.empty())
        return true;

    if (sFullPath.compare(0, m_sPathToArchive.length(), m_sPathToArchive) != 0)
        return false;

    if (m_pUnzipFile != NULL)
        return m_pUnzipFile->FileExists(sFullPath.substr(m_sPathToArchive.length()));
    if (m_pZipFile != NULL)
        return m_pZipFile->FileExists(sFullPath.substr(m_sPathToArchive.length()));

    return false;
}


/*====================
  CArchive::GetFileList
  ====================*/
void    CArchive::GetFileList(tsvector &vFileList) const
{
    if (m_pUnzipFile == NULL)
        return;

    const tsvector &vArchiveFiles(m_pUnzipFile->GetFileList());
    vFileList.insert(vFileList.end(), vArchiveFiles.begin(), vArchiveFiles.end());

    tsvector vDiskFileList;
    K2System.GetFileList(Filename_StripExtension(m_sPath) + _T("/"), _T("*"), true, vDiskFileList, m_sMod);
    if (!vDiskFileList.empty())
    {
        hash_set<tstring> vSeen;
        std::copy(vFileList.begin(), vFileList.end(), std::inserter(vSeen, vSeen.end()));
        for (const auto& sFile : vDiskFileList) {
            if (vSeen.find(sFile) == vSeen.end()) {
                vFileList.emplace_back(sFile);
                vSeen.insert(sFile);
            }
        }

    }
}


/*====================
  CArchive::GetModifiedFilesList
  ====================*/
bool    CArchive::GetModifiedFilesList(tsvector &vFileList)
{
    // if we're not open for reading, abort.
    assert(m_pUnzipFile != NULL);
    if (m_pUnzipFile == NULL)
        return false;

    // if we don't have a checksum table, then we have no way of determining
    // whether files have been modified, so abort.
    assert(m_pChecksums != NULL);
    if (m_pChecksums == NULL)
        return false;

    // iterate over each file and determine whether it's been modified.
    tsvector vFiles;
    GetFileList(vFiles);
    for (tsvector_cit it(vFiles.begin()), itEnd(vFiles.end()); it != itEnd; ++it)
    {
        const tstring &sFile(*it);
        char* pFile(NULL);

        // skip the checksums file.
        if (sFile == _T("checksums"))
            continue;

        tstring sUsePath(sFile);
        if (sUsePath.empty())
            continue;

        // verify that the path begins with /
        if (sUsePath[0] != _T('/'))
            sUsePath.insert(sUsePath.begin(), 1, _T('/'));

        // validate the file's checksum.
        if (!ValidateFileChecksum(sUsePath))
            vFileList.push_back(sUsePath);

        SAFE_DELETE_ARRAY(pFile);
    }

    return true;
}


/*====================
  CArchive::WriteFile
  ====================*/
bool    CArchive::WriteFile(const tstring &sPath, const char *pBuffer, size_t size, int iMode, time_t t)
{
    if (m_pZipFile == NULL)
        return false;

    try
    {
        tstring sPathInArchive(LowerString(sPath));
        tstring sPathToArchive(LowerString(Filename_GetPath(m_sPath)));
        if (sPathInArchive.substr(0, sPathToArchive.length()) == sPathToArchive)
            sPathInArchive = sPathInArchive.substr(sPathToArchive.length());
            
        int iCompressLevel(ZIP_DEFAULT_COMPRESSION_LEVEL);

        // Use archives mode
        if (iMode == -1)
        {
            if (m_iMode & ARCHIVE_NO_COMPRESS)
                iCompressLevel = Z_NO_COMPRESSION;
            else if (m_iMode & ARCHIVE_MAX_COMPRESS)
                iCompressLevel = Z_BEST_COMPRESSION;
            else if (m_iMode & ARCHIVE_SPEED_COMPRESS)
                iCompressLevel = Z_BEST_SPEED;
        }
#if 0
        else
        {
            if (iMode & FILE_NOCOMPRESS)
                iCompressLevel = 0;
        }
#endif

        if (m_pZipFile->AddFile(sPathInArchive, pBuffer, size, iCompressLevel, t) != ZIP_OK)
            EX_ERROR(_T("Failed to write to file in archive"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CARchive::WriteFile() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CArchive::GetCompressedFile
  ====================*/
int     CArchive::GetCompressedFile(const tstring &sPath, CCompressedFile &cFile)
{
    if (!m_pUnzipFile)
        return 0;

    tstring sFullPath(FileManager.IsCleanPath(sPath) ? sPath : FileManager.SanitizePath(sPath, false));
    if (sFullPath.compare(0, m_sPathToArchive.length(), m_sPathToArchive) != 0)
        return 0;

    return m_pUnzipFile->GetCompressedFile(sFullPath.substr(m_sPathToArchive.length()), cFile);
}


/*====================
  CArchive::WriteCompressedFile
  ====================*/
bool    CArchive::WriteCompressedFile(const tstring &sPath, const CCompressedFile &cFile, time_t t)
{
    if (m_pZipFile == NULL)
        return false;

    try
    {
        tstring sPathInArchive(LowerString(sPath));
        tstring sPathToArchive(LowerString(Filename_GetPath(m_sPath)));
        if (sPathInArchive.substr(0, sPathToArchive.length()) == sPathToArchive)
            sPathInArchive = sPathInArchive.substr(sPathToArchive.length());
            
        if (m_pZipFile->AddCompressedFile(sPathInArchive, cFile, t) != ZIP_OK)
            EX_ERROR(_T("Failed to write to file in archive"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CARchive::WriteFile() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CArchive::DeleteFile
  ====================*/
bool    CArchive::DeleteCompressedFile(const tstring &sPath)
{
    if (m_pZipFile == NULL)
        return false;

    try
    {
        tstring sPathInArchive(LowerString(sPath));
        tstring sPathToArchive(LowerString(Filename_GetPath(m_sPath)));
        if (sPathInArchive.substr(0, sPathToArchive.length()) == sPathToArchive)
            sPathInArchive = sPathInArchive.substr(sPathToArchive.length());
            
        if (m_pZipFile->RemoveFile(sPathInArchive) != ZIP_OK)
            EX_ERROR(_T("Failed to delete file in archive"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CARchive::DeleteFile() - "), NO_THROW);
        return false;
    }

    return true;
}


/*====================
  CArchive::GetWriteProgress
  ====================*/
float   CArchive::GetWriteProgress() const
{
    if (m_pZipFile)
        return m_pZipFile->GetWriteProgress();
    else
        return 0.0f;
}


/*====================
  CArchive::GetFileList
  ====================*/
const tsvector& CArchive::GetFileList() const
{
    static tsvector vEmpty;
    if (m_pUnzipFile == NULL)
        return vEmpty;

    return m_pUnzipFile->GetFileList();
}


/*====================
  CArchive::ComputeChecksums
  ====================*/
bool    CArchive::ComputeChecksums(CChecksumTable &cChecksums, const tsvector &vFileList)
{
    cChecksums.Clear();

    // if we are not open for reading, abort.
    if (m_pUnzipFile == NULL)
        return false;

    // loop through each file in the archive and compute its checksum.
    uint i(0);
    for (tsvector::const_iterator it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it, ++i)
    {
        const tstring &sFile(*it);
        char* pFile(NULL);

        // read the file.
        int iFileSize(ReadFile(sFile, pFile));
        if (iFileSize < 0 || pFile == NULL)
        {
            Console << _T("Failed to compute checksum for file '") << sFile << _T("': could not read file!") << newl;
            SAFE_DELETE_ARRAY(pFile);
            cChecksums.Clear();
            return false;
        }

        // compute its checksum.
        byte yChecksum[CHECKSUM_SIZE];
        if (!CChecksumTable::ComputeChecksum(yChecksum, (const byte*)pFile, iFileSize))
        {
            Console << _T("Failed to compute checksum for file '") << sFile << _T("': checksum computation failed!") << newl;
            SAFE_DELETE_ARRAY(pFile);
            cChecksums.Clear();
            return false;
        }

        // add its checksum to the checksum table.
        cChecksums.Add(i, sFile, yChecksum);

        SAFE_DELETE_ARRAY(pFile);
    }

    return true;
}


/*====================
  CArchive::HashChecksums
  ====================*/
bool    CArchive::HashChecksums(byte* pOutChecksum)
{
    if (!(m_iMode & ARCHIVE_READ))
        return false;

    if (m_pChecksums == NULL)
        return false;

    if (!m_pChecksums->HashChecksums(pOutChecksum))
        return false;

    return true;
}


#if !defined(K2_CLIENT) && !defined(K2_SERVER) && !defined(K2_GARENA)
/*--------------------
  cmdWriteChecksums
  --------------------*/
CMD(WriteChecksums)
{
    tstring sArchive(_T("resources0.s2z"));

    if (vArgList.size() >= 1)
        sArchive = vArgList[0];

    sArchive = _T("/") + sArchive;

    CArchive cArchive(sArchive, ARCHIVE_READ);
    if (!cArchive.IsOpen())
    {
        Console << _T("Could not open archive '") << sArchive << _T("'") << newl;
        return true;
    }

    // get a list of files in the archive.
    tsvector vFileList(cArchive.GetFileList());

    // prepend a '/' to the front of each path.
    for (tsvector::iterator it(vFileList.begin()), itEnd(vFileList.end()); it != itEnd; ++it)
    {
        tstring& sFilePath(*it);
        sFilePath.insert(sFilePath.begin(), 1, _T('/'));
    }

    // sort the file list so that ordering does not matter.
    std::sort(vFileList.begin(), vFileList.end());

    CChecksumTable cChecksums;
    if (!cArchive.ComputeChecksums(cChecksums, vFileList))
        return true;

    cArchive.Close();

    byte* yChecksumsFile(NULL);
    uint uiChecksumsFileSize(0);
    if (!cChecksums.Serialize(yChecksumsFile, uiChecksumsFileSize, vFileList))
    {
        Console << _T("Failed to serialize checksums table.") << newl;
        return true;
    }

    CFileHandle cFile(sArchive + _T(".checksums"), FILE_WRITE | FILE_BINARY);
    if (!cFile.IsOpen())
    {
        Console << _T("Could not open file '") << sArchive + _T(".checksums") << _T("' for writing") << newl;
        SAFE_DELETE_ARRAY(yChecksumsFile);
        return true;
    }
    cFile.Write(yChecksumsFile, uiChecksumsFileSize);
    cFile.Close();

    SAFE_DELETE_ARRAY(yChecksumsFile);

    return true;
}


/*--------------------
  cmdVerifyChecksums
  --------------------*/
CMD(VerifyChecksums)
{
    tstring sArchive(_T("resources0.s2z"));
    tstring sMod(_T("game"));

    if (vArgList.size() >= 1)
        sArchive = vArgList[0];

    if (vArgList.size() >= 2)
        sMod = vArgList[1];

    sArchive = _T("/") + sArchive;

    CArchive* pArchive(pFileManager->GetArchive(sArchive,sMod));
    if (pArchive == NULL)
    {
        Console << _T("Invalid archive << ") << sArchive << newl;
        return true;
    }

    byte pChecksum[CHECKSUM_SIZE];
    if (!pArchive->HashChecksums(pChecksum))
    {
        Console << _T("Failed to hash checksums for archive ") << sArchive << _T(" (no checksum table?)") << newl;
        return true;
    }

    tstring sChecksum;
    CChecksumTable::ChecksumToString(sChecksum, pChecksum);

    Console << _T("Checksum for archive ") << sArchive << _T(": ") << newl;
    Console << _T("     ") << sChecksum << newl;

    return true;
}


/*--------------------
  cmdWriteArchiveList
  --------------------*/
CMD(WriteArchiveList)
{
    tstring sArchive(_T("resources0.s2z"));
    tstring sMod(_T("game"));

    if (vArgList.size() >= 1)
        sArchive = vArgList[0];

    if (vArgList.size() >= 2)
        sMod = vArgList[1];

    sArchive = _T("/") + sArchive;

    CArchive* pArchive(pFileManager->GetArchive(sArchive,sMod));
    if (pArchive == NULL)
    {
        Console << _T("Invalid archive ") << sArchive << newl;
        return true;
    }

    tsvector vFileList(pArchive->GetFileList());
    for (tsvector::iterator it(vFileList.begin()); it != vFileList.end();)
    {
        tstring& sFile(*it);
        if (sFile == _T("checksums"))
        {
            it = vFileList.erase(it);
            continue;
        }

        sFile = _T("/") + sFile;
        ++it;
    }
    std::sort(vFileList.begin(), vFileList.end());

    CChecksumTable cChecksums;
    if (!pArchive->ComputeChecksums(cChecksums, vFileList))
    {
        Console << _T("Could not compute checksums for archive ") << pArchive->GetPath() << newl;
        return true;
    }

    tstring sPath(_T(":/") + sMod + pArchive->GetPath() + _T(".txt"));
    CFileHandle cOutFile(sPath, FILE_WRITE | FILE_TEXT);
    if (!cOutFile.IsOpen())
    {
        Console << _T("Could not open ") << sPath << _T(" for writing.") << newl;
        return true;
    }

    for (size_t i = 0; i < vFileList.size(); ++i)
    {
        tstring sFile(vFileList[i]);

        tstring sLine(sFile);

        byte pChecksum[CHECKSUM_SIZE];
        if (cChecksums.GetChecksum(pChecksum, sFile))
        {
            tstring sChecksum;
            CChecksumTable::ChecksumToString(sChecksum, pChecksum);
            sLine.append(_T(" ") + sChecksum);
        }
        else
        {
            sLine.append(_T(" NONE"));
        }

        cOutFile.WriteLine(TStringToWString(sLine));
    }

    byte pHashChecksums[CHECKSUM_SIZE];
    if (cChecksums.HashChecksums(pHashChecksums))
    {
        tstring sHashChecksums;
        CChecksumTable::ChecksumToString(sHashChecksums, pHashChecksums);
        tstring sLine(_T("Final checksum: ") + sHashChecksums);
        cOutFile.WriteLine(TStringToWString(sLine));
    }
    else
    {
        tstring sLine(_T("Could not compute final checksum."));
        cOutFile.WriteLine(TStringToWString(sLine));
    }

    cOutFile.Close();
    return true;
}
#endif

