// (C)2005 S2 Games
// unzip.cpp
//
// IO on .zip files using zlib
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifndef _WIN32
#include <utime.h>
#endif

#include "c_unzip.h"
#include "stringutils.h"
#include "c_cvar.h"

//#include "../core/hash.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES) && \
                      !defined(CASESENSITIVITYDEFAULT_NO)
#define CASESENSITIVITYDEFAULT_NO
#endif

#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifdef  CASESENSITIVITYDEFAULT_NO
#define CASESENSITIVITYDEFAULTVALUE 2
#else
#define CASESENSITIVITYDEFAULTVALUE 1
#endif

#ifndef STRCMPCASENOSENTIVEFUNCTION
#define STRCMPCASENOSENTIVEFUNCTION _tcsncmp
#endif

/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif
//=============================================================================


/*====================
  CUnzip::CUnzip
 ====================*/
CUnzip::CUnzip (const tstring &sPath)
{
    m_pzipFile = Open(sPath);
}


/*====================
  CUnzip::_stringFilenameCompare

  Compare two filename (fileName1,fileName2).
  If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
  If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi or strcasecmp)
  If iCaseSenisivity = 0, case sensitivity is defaut of your operating system (like 1 on Unix, 2 on Windows)
  ====================*/
int     CUnzip::StringFilenameCompare (const tstring &sFilename1, const tstring &sFilename2, int iCaseSensitivity)
{
    if (iCaseSensitivity == 0)
        iCaseSensitivity = CASESENSITIVITYDEFAULTVALUE;

    if (iCaseSensitivity == 1)
        return _tcscmp(sFilename1.c_str(), sFilename2.c_str());

    return STRCMPCASENOSENTIVEFUNCTION(sFilename1.c_str(), sFilename2.c_str(), sFilename1.length());
}


/*====================
  unzlocal_SearchCentralDir

  Locate the Central directory of a zipfile (at the end, just before the global comment)
  ====================*/
uLong   CUnzip::SearchCentralDir(FILE *pFileIn)
{
    unsigned char* buf;
    uLong uSizeFile;
    uLong uBackRead;
    uLong uMaxBack = 0xffff; /* maximum size of global comment */
    uLong uPosFound = 0;

    if (fseek(pFileIn, 0, SEEK_END) != 0)
        return 0;

    uSizeFile = ftell(pFileIn);

    if (uMaxBack > uSizeFile)
        uMaxBack = uSizeFile;

    buf = K2_NEW_ARRAY(ctx_FileSystem, byte, BUFREADCOMMENT+4);
    if (buf == NULL)
        return 0;

    uBackRead = 4;
    while (uBackRead < uMaxBack)
    {
        uLong uReadSize,uReadPos;
        int i;
        if (uBackRead + BUFREADCOMMENT > uMaxBack)
            uBackRead = uMaxBack;
        else
            uBackRead+=BUFREADCOMMENT;
        uReadPos = uSizeFile - uBackRead;

        uReadSize = ((BUFREADCOMMENT + 4) < (uSizeFile - uReadPos)) ?
                     (BUFREADCOMMENT + 4) : (uSizeFile - uReadPos);
        if (fseek(pFileIn,uReadPos,SEEK_SET) != 0)
            break;

        if (fread(buf,(uInt)uReadSize, 1,pFileIn)!=1)
            break;

        for (i = (int)uReadSize - 3; (i--) > 0; )
        {
            if (((*(buf + i)) == 0x50) && ((*(buf + i + 1)) == 0x4b) &&
                ((*(buf + i + 2)) == 0x05) && ((*(buf + i + 3)) == 0x06))
            {
                uPosFound = uReadPos + i;
                break;
            }
        }

        if (uPosFound!=0)
            break;
    }
    K2_DELETE_ARRAY(buf);
    return uPosFound;
}


/*====================
  open

  Open a Zip pFile. path contain the full pathname (by example,
  on a Windows NT computer "c:\\test\\zlib109.zip" or on an Unix computer
  "zlib/zlib109.zip".

  If the zipfile cannot be opened (pFile don't exist or in not valid), the
  return value is NULL.

  Else, the return value is a unzFile Handle, usable with other function
  of this unzip package.
  ====================*/
SZipFile*   CUnzip::Open(const tstring &sPath)
{
    SZipFile zipFileInternalInfo;
    uLong centralPos, uL;
    FILE *pFileIn;
    tstring lowercaseName;
    //int hashlen;
    //uint hash[MAX_HASH_SIZE];
    uLong number_disk;          /* number of the current dist, used for
                                   spaning ZIP, unsupported, always 0*/
    uLong number_disk_with_CD;  /* number the the disk with central dir, used
                                   for spaning ZIP, unsupported, always 0*/
    uLong number_entry_CD;      /* total number of entries in
                                   the central dir
                                   (same than number_entry on nospan) */
    int numFiles, status = UNZ_OK;

    if (!_TFOPEN_S(pFileIn, TStringToNative(sPath).c_str(), _TNative("rb")))
    {
        Console << _T("can't open ") << sPath << newl;
        return NULL;
    }

    centralPos = SearchCentralDir(pFileIn);
    if (centralPos == 0)
        status = UNZ_ERRNO;

    if (fseek(pFileIn, centralPos, SEEK_SET) != 0)
        status = UNZ_ERRNO;

    /* the signature, already checked */
    if (GetLong(pFileIn, &uL) != UNZ_OK)
        status = UNZ_ERRNO;

    /* number of this disk */
    if (GetShort(pFileIn, &number_disk) != UNZ_OK)
        status = UNZ_ERRNO;

    /* number of the disk with the start of the central directory */
    if (GetShort(pFileIn, &number_disk_with_CD) != UNZ_OK)
        status = UNZ_ERRNO;

    /* total number of entries in the central dir on this disk */
    if (GetShort(pFileIn, &zipFileInternalInfo.globalInfo.numberEntry) != UNZ_OK)
        status = UNZ_ERRNO;

    /* total number of entries in the central dir */
    if (GetShort(pFileIn,&number_entry_CD) != UNZ_OK)
        status = UNZ_ERRNO;

    if ((number_entry_CD != zipFileInternalInfo.globalInfo.numberEntry) ||
        (number_disk_with_CD != 0) ||
        (number_disk != 0))
        status = UNZ_BADZIPFILE;

    /* size of the central directory */
    if (GetLong(pFileIn, &zipFileInternalInfo.sizeCentralDir) != UNZ_OK)
        status = UNZ_ERRNO;

    /* offset of start of central directory with respect to the
          starting disk number */
    if (GetLong(pFileIn, &zipFileInternalInfo.offsetCentralDir) != UNZ_OK)
        status = UNZ_ERRNO;

    /* zipfile comment length */
    if (GetLong(pFileIn, &zipFileInternalInfo.globalInfo.sizeComment) != UNZ_OK)
        status = UNZ_ERRNO;

    if ((centralPos < zipFileInternalInfo.offsetCentralDir + zipFileInternalInfo.sizeCentralDir) &&
        (status == UNZ_OK))
        status = UNZ_BADZIPFILE;

    if (status != UNZ_OK) {
        fclose(pFileIn);
        return NULL;
    }

    zipFileInternalInfo.pfile = pFileIn;
    zipFileInternalInfo.byteBefore = centralPos - (zipFileInternalInfo.offsetCentralDir
        + zipFileInternalInfo.sizeCentralDir);
    zipFileInternalInfo.centralPos = centralPos;
    zipFileInternalInfo.pfileInZipRead = NULL;

    m_pzipFile = K2_NEW(ctx_FileSystem,  SZipFile);
    *m_pzipFile = zipFileInternalInfo;
    status = GoToFirstFile();

    m_pzipFile->pFilenameMap = K2_NEW(ctx_FileSystem,  FilenameMap);

    numFiles = 0;

#if 0 // HASH
#ifdef _DEBUG
    Console << _T("creating new filenameHash") << newl;
#endif

    MemManager.Set(hash, 0, MAX_HASH_SIZE);

    Core.Hash_StartHash();

    while (status == UNZ_OK)
    {
        TCHAR szCurrentFilename[UNZ_MAXFILENAMEINZIP + 1];

        GetCurrentFileInfoInternal(&m_pzipFile->curFileInfo,
            &m_pzipFile->curFileInfoInternal, szCurrentFilename,
            sizeof(szCurrentFilename)-1, NULL, 0, NULL, 0);

        Core.Hash_AddData((char *)&m_pzipFile->curFileInfo,
            sizeof(m_pzipFile->curFileInfo));
        Core.Hash_AddData((char *)&m_pzipFile->curFileInfoInternal,
            sizeof(m_pzipFile->curFileInfoInternal));

        lowercaseName = LowerString(szCurrentFilename);

        (*m_pzipFile->pFilenameMap)[lowercaseName.c_str()] = m_pzipFile->posCentralDir;
        ++numFiles;
        status = GoToNextFile();
    }

    hashlen = Core.Hash_EndHash((unsigned char *)hash);

    Console << _T("s2z hash \"") << sPath << _T("\" is: ") << newl;

    for (int i(0); i < hashlen; ++i)
        Console << hash[i];
    Console << newl;
#else // NO HASH
    while (status == UNZ_OK)
    {
        TCHAR szCurrentFilename[UNZ_MAXFILENAMEINZIP + 1];

        GetCurrentFileInfoInternal(&m_pzipFile->curFileInfo,
            &m_pzipFile->curFileInfoInternal, szCurrentFilename,
            sizeof(szCurrentFilename)-1, NULL, 0, NULL, 0);

        lowercaseName = LowerString(tstring(szCurrentFilename));

        (*m_pzipFile->pFilenameMap)[lowercaseName.c_str()] = m_pzipFile->posCentralDir;
        ++numFiles;
        status = GoToNextFile();
    }
#endif

    //Console << _T("Done reading zip pFile contents - ") << numFiles << _T(" files") << newl;
    if (status != UNZ_END_OF_LIST_OF_FILE)
        Console << _T("ZIP status error: status was ") << status << newl;
    return (SZipFile *)m_pzipFile;
}


/*====================
  CUnzip::~CUnzip
  ==================== */
CUnzip::~CUnzip()
{
    Close();

    if (m_pzipFile)
    {
        K2_DELETE(m_pzipFile);
        m_pzipFile = NULL;
    }
}


/*====================
  CUnzip::close

  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
  these files MUST be closed with _closeCurrentFile before call _close.

  return UNZ_OK if there is no problem.
  ==================== */
int     CUnzip::Close()
{
    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    if (m_pzipFile->pfileInZipRead != NULL)
        CloseCurrentFile(m_pzipFile);

    K2_DELETE(m_pzipFile->pFilenameMap);
    m_pzipFile->pFilenameMap = NULL;

    fclose(m_pzipFile->pfile);
    m_pzipFile->pfile = NULL;
    K2_DELETE(m_pzipFile);
    m_pzipFile = NULL;
    return UNZ_OK;
}


/*====================
  ZIP_SystemDir
  ====================
void CUnzip::zipSystemDir (void *archive, char *_directory, char *wildcard, bool recurse,
                                  void(*dirCallback)(const char *dir, void *userdata),
                                  void(*fileCallback)(const char *filename, void *userdata),
                                  void *userdata) {
    GPatternSpec* pattern;
    SZipFile *pzipfileInfInternal;
    SZipFile backup;
    int status, numFiles;
    char *slash, *fname;
    char filename[384] = {0};
    char directory[256] = {0};
    char enumdir[256] = {0};

    if (archive == NULL)
        return;
    pzipfileInfInternal = (SZipFile *)archive;

#ifdef _DEBUG
    if (!_directory || strlen(_directory) == 0)
        BPrintf(directory, 1024, "");
    else
        BPrintf(directory, 1024, "%s%s", (_directory[0] == '/') ? &_directory[1] : _directory, (_directory[strlen(_directory)-1] == '/') ? "" : "/");

    BPrintf(filename, 1024, "%s%s", directory, wildcard);
#endif

    pattern = g_pattern_spec_new(filename);

    Console.Dev << "Doing a System_Dir in a ZIP pFile matching " << filename << newl;

    status = _goToFirstFile();

    numFiles = 0;
    while (status == UNZ_OK) {
        getCurrentFileInfo((unzFile)pzipfileInfInternal, NULL, filename, 1023, NULL, 0, NULL, 0);
        filename[1023] = 0;

        if (strcmp(filename, directory) != 0 && strstr(filename, "CVS/") == 0) {
            if (g_pattern_match_string(pattern, filename)) {
                slash = strchr(&filename[strlen(directory)], '/');
                if (!slash || (slash == filename + strlen(filename) - 1) || recurse)
                {
                    if (filename[strlen(filename)-1] != '/')
                    {
                        if (fileCallback)
                        {
                            slash = strrchr(filename, '/');
                            if (!slash)
                            {
                                fname = filename;
#ifdef _DEBUG
                                BPrintf(enumdir, 1, "");
#endif
                            }
                            else
                            {
                                fname = slash + 1;
#ifdef _DEBUG
                                BPrintf(enumdir, slash - filename + 1, "%s", filename);
#endif
                            }
                            sys_edir = enumdir;
                            MemManager.Copy(&backup, pzipfileInfInternal, sizeof(SZipFile));
                            fileCallback(fname, userdata);
                            MemManager.Copy(pzipfileInfInternal, &backup, sizeof(SZipFile));
                        }
                    }
                    else
                    {
                        if (dirCallback)
                            dirCallback(filename, userdata);
                    }
                }
            }
        }
        ++numFiles;
        status = _goToNextFile();
    }

    Console.Dev << "Done reading zip pFile contents - " << numFiles << " files" << newl;
    if (status != UNZ_END_OF_LIST_OF_FILE)
        Console.Dev << "ZIP error: err was " << status << newl;
    sys_edir = "";
    g_pattern_spec_free(pattern);
}
*/


/*====================
  ZIP_FileExists
  ====================*/
bool    CUnzip::FileExists (const tstring &sPath)
{
    tstring lowerFilename;

    if (m_pzipFile == NULL)
        return false;

    lowerFilename = LowerString(sPath);

    if (m_pzipFile->pFilenameMap->find(lowerFilename) != m_pzipFile->pFilenameMap->end())
        return true;

    return false;
}


/*int CUnzip::_locateFile (string szFilename, int iCaseSensitivity) {
    SZipFile *pzipfileInfInternal;
    int status;

    uLong fileNum;
    uLong posCentralDir;

    if (pFile == NULL)
        return UNZ_PARAMERROR;

    if (szFilename.length() >= UNZ_MAXFILENAMEINZIP)
        return UNZ_PARAMERROR;

    pzipfileInfInternal = (SZipFile *)pFile;
    if (!pzipfileInfInternal->fileOk)
        return UNZ_END_OF_LIST_OF_FILE;

    fileNum = pzipfileInfInternal->fileNum;
    posCentralDir = pzipfileInfInternal->posCentralDir;

    status = _goToFirstFile();

    while (status == UNZ_OK) {
        char szCurrentFilename[UNZ_MAXFILENAMEINZIP + 1];
        getCurrentFileInfo((unzFile)pzipfileInfInternal, NULL, szCurrentFilename, sizeof(szCurrentFilename ) - 1,
            NULL, 0, NULL, 0);
        if (_stringFilenameCompare(szCurrentFilename, szFilename, iCaseSensitivity) == 0)
            return UNZ_OK;
        status = _goToNextFile();
    }
    pzipfileInfInternal->fileNum = fileNum;
    pzipfileInfInternal->posCentralDir = posCentralDir;
    return status;
}
*/


/*====================
  checkCurrentFileCoherencyHeader

  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
  directory about this pFile

  store in *piSizeVar the size of extra info in local header
  (filename and size of extra field data)
  ====================*/
int     CUnzip::CheckCurrentFileCoherencyHeader(uint *piSizeVar, uLong *pOffsetLocalExtraField,
                                                uint *piSizeLocalExtraField)
{
    uLong uMagic, uData, uFlags;
    uLong size_filename;
    uLong size_extra_field;
    int status = UNZ_OK;

    *piSizeVar = 0;
    *pOffsetLocalExtraField = 0;
    *piSizeLocalExtraField = 0;

    if (fseek(m_pzipFile->pfile, m_pzipFile->curFileInfoInternal.offsetCurfile +
                                m_pzipFile->byteBefore ,SEEK_SET) != 0)
        return UNZ_ERRNO;


    if (status == UNZ_OK) {
        if (GetLong(m_pzipFile->pfile, &uMagic) != UNZ_OK)
            status = UNZ_ERRNO;
        else if (uMagic!=0x04034b50)
            status = UNZ_BADZIPFILE;
    }

    if (GetShort(m_pzipFile->pfile,&uData) != UNZ_OK)
        status = UNZ_ERRNO;
/*
    else if ((status = =UNZ_OK) && (uData!=m_pzipFile->curFileInfo.wVersion))
        status = UNZ_BADZIPFILE;
*/
    if (GetShort(m_pzipFile->pfile,&uFlags) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(m_pzipFile->pfile,&uData) != UNZ_OK)
        status = UNZ_ERRNO;
    else if ((status == UNZ_OK) && (uData!=m_pzipFile->curFileInfo.compressionMethod))
        status = UNZ_BADZIPFILE;

    if ((status == UNZ_OK) && (m_pzipFile->curFileInfo.compressionMethod!=0) &&
                         (m_pzipFile->curFileInfo.compressionMethod!=Z_DEFLATED))
        status = UNZ_BADZIPFILE;

    if (GetLong(m_pzipFile->pfile,&uData) != UNZ_OK) /* date / time */
        status = UNZ_ERRNO;

    if (GetLong(m_pzipFile->pfile,&uData) != UNZ_OK) /* crc */
        status = UNZ_ERRNO;
    else if ((status == UNZ_OK) && (uData!=m_pzipFile->curFileInfo.crc) &&
                              ((uFlags & 8) == 0))
        status = UNZ_BADZIPFILE;

    if (GetLong(m_pzipFile->pfile,&uData) != UNZ_OK) /* size compr */
        status = UNZ_ERRNO;
    else if ((status == UNZ_OK) && (uData!=m_pzipFile->curFileInfo.compressedSize) &&
                              ((uFlags & 8) == 0))
        status = UNZ_BADZIPFILE;

    if (GetLong(m_pzipFile->pfile,&uData) != UNZ_OK) /* size uncompr */
        status = UNZ_ERRNO;
    else if ((status == UNZ_OK) && (uData!=m_pzipFile->curFileInfo.uncompressedSize) &&
                              ((uFlags & 8) == 0))
        status = UNZ_BADZIPFILE;


    if (GetShort(m_pzipFile->pfile,&size_filename) != UNZ_OK)
        status = UNZ_ERRNO;
    else if ((status == UNZ_OK) && (size_filename != m_pzipFile->curFileInfo.sizeFilename))
        status = UNZ_BADZIPFILE;

    *piSizeVar += (uInt)size_filename;

    if (GetShort(m_pzipFile->pfile,&size_extra_field) != UNZ_OK)
        status = UNZ_ERRNO;
    *pOffsetLocalExtraField = m_pzipFile->curFileInfoInternal.offsetCurfile +
                                    SIZEZIPLOCALHEADER + size_filename;
    *piSizeLocalExtraField = (uInt)size_extra_field;

    *piSizeVar += (uInt)size_extra_field;

    return status;
}


/*====================
  CUnzip::OpenCurrentFile

  Open for reading data the current pFile in the zipfile.
  If there is no error and the pFile is opened, the return value is UNZ_OK.
  ====================*/
int     CUnzip::OpenCurrentFile()
{
    int status = UNZ_OK;
    int Store;
    uInt iSizeVar;
    SZipFileInfo *pfileInZipReadInfo;
    uLong offset_local_extrafield;  /* offset of the local extra field */
    uInt  size_local_extrafield;    /* size of the local extra field */

    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    if (!m_pzipFile->fileOk)
        return UNZ_PARAMERROR;

    if (m_pzipFile->pfileInZipRead != NULL)
        CloseCurrentFile(m_pzipFile);

    if (CheckCurrentFileCoherencyHeader(&iSizeVar, &offset_local_extrafield,
        &size_local_extrafield) != UNZ_OK)
        return UNZ_BADZIPFILE;

    pfileInZipReadInfo = K2_NEW(ctx_FileSystem,  SZipFileInfo);
    if (pfileInZipReadInfo == NULL)
        return UNZ_INTERNALERROR;

    pfileInZipReadInfo->preadBuffer = K2_NEW_ARRAY(ctx_FileSystem, char, UNZ_BUFSIZE);
    pfileInZipReadInfo->offsetLocalExtrafield = offset_local_extrafield;
    pfileInZipReadInfo->sizeLocalExtrafield = size_local_extrafield;
    pfileInZipReadInfo->posLocalExtrafield = 0;

    if (pfileInZipReadInfo->preadBuffer == NULL)
    {
        K2_DELETE(pfileInZipReadInfo);
        return UNZ_INTERNALERROR;
    }

    pfileInZipReadInfo->streamInitialised = 0;

    if ((m_pzipFile->curFileInfo.compressionMethod != 0) &&
        (m_pzipFile->curFileInfo.compressionMethod != Z_DEFLATED))
        status = UNZ_BADZIPFILE;
    Store = m_pzipFile->curFileInfo.compressionMethod == 0;

    pfileInZipReadInfo->crc32Wait = m_pzipFile->curFileInfo.crc;
    pfileInZipReadInfo->crc32 = 0;
    pfileInZipReadInfo->compressionMethod = m_pzipFile->curFileInfo.compressionMethod;
    pfileInZipReadInfo->pfile = m_pzipFile->pfile;
    pfileInZipReadInfo->byteBefore = m_pzipFile->byteBefore;

    pfileInZipReadInfo->stream.total_out = 0;

    if (!Store)
    {
        pfileInZipReadInfo->stream.zalloc = (alloc_func)0;
        pfileInZipReadInfo->stream.zfree = (free_func)0;
        pfileInZipReadInfo->stream.opaque = (voidpf)0;

        status = inflateInit2(&pfileInZipReadInfo->stream, -MAX_WBITS);
        if (status == Z_OK)
            pfileInZipReadInfo->streamInitialised = 1;
            /* windowBits is passed < 0 to tell that there is no zlib header.
            * Note that in this case inflate *requires* an extra "dummy" byte
            * after the compressed stream in order to complete decompression and
            * return Z_STREAM_END.
            * In unzip, i don't wait absolutely Z_STREAM_END because I known the
            * size of both compressed and uncompressed data
            */
    }
    pfileInZipReadInfo->restReadCompressed =
            m_pzipFile->curFileInfo.compressedSize;
    pfileInZipReadInfo->restReadUncompressed =
            m_pzipFile->curFileInfo.uncompressedSize;


    pfileInZipReadInfo->posInZipfile =
            m_pzipFile->curFileInfoInternal.offsetCurfile + SIZEZIPLOCALHEADER +
              iSizeVar;

    pfileInZipReadInfo->stream.avail_in = (uInt)0;


    m_pzipFile->pfileInZipRead = pfileInZipReadInfo;
    return UNZ_OK;
}


/*====================
  unzReadCurrentFile

  Read bytes from the current pFile.
  buf contain buffer where data must be copied
  iLen the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of pFile was reached
  return <0 with error code if there is an error
    (UNZ_ERRNO for IO error, or zLib error for uncompress error)
  ====================*/
int     CUnzip::ReadCurrentFile(SZipFile *pFile, voidp pBuf, uint iLen)
{
    SZipFileInfo *pfileInZipReadInfo;
    int status = UNZ_OK;
    uInt iRead = 0;

    if (pFile == NULL)
        return UNZ_PARAMERROR;

    pfileInZipReadInfo = pFile->pfileInZipRead;

    if (pfileInZipReadInfo == NULL)
        return UNZ_PARAMERROR;
    if ((pfileInZipReadInfo->preadBuffer == NULL))
        return UNZ_END_OF_LIST_OF_FILE;
    if (iLen == 0)
        return 0;

    pfileInZipReadInfo->stream.next_out = (Bytef*)pBuf;

    pfileInZipReadInfo->stream.avail_out = (uInt)iLen;

    if (iLen > pfileInZipReadInfo->restReadUncompressed)
        pfileInZipReadInfo->stream.avail_out =
          (uInt)pfileInZipReadInfo->restReadUncompressed;

    while (pfileInZipReadInfo->stream.avail_out > 0)
    {
        if ((pfileInZipReadInfo->stream.avail_in == 0) &&
            (pfileInZipReadInfo->restReadCompressed > 0))
        {
            uInt uReadThis = UNZ_BUFSIZE;
            if (pfileInZipReadInfo->restReadCompressed < uReadThis)
                uReadThis = (uInt)pfileInZipReadInfo->restReadCompressed;
            if (uReadThis == 0)
                return UNZ_EOF;
            if (fseek(pfileInZipReadInfo->pfile,
                      pfileInZipReadInfo->posInZipfile +
                         pfileInZipReadInfo->byteBefore, SEEK_SET) != 0)
                return UNZ_ERRNO;
            if (fread(pfileInZipReadInfo->preadBuffer, uReadThis, 1,
                         pfileInZipReadInfo->pfile) != 1)
                return UNZ_ERRNO;
            pfileInZipReadInfo->posInZipfile += uReadThis;

            pfileInZipReadInfo->restReadCompressed -= uReadThis;

            pfileInZipReadInfo->stream.next_in =
                (Bytef*)pfileInZipReadInfo->preadBuffer;
            pfileInZipReadInfo->stream.avail_in = (uInt)uReadThis;
        }

        if (pfileInZipReadInfo->compressionMethod == 0)
        {
            uInt uDoCopy,i;
            if (pfileInZipReadInfo->stream.avail_out <
                            pfileInZipReadInfo->stream.avail_in)
                uDoCopy = pfileInZipReadInfo->stream.avail_out;
            else
                uDoCopy = pfileInZipReadInfo->stream.avail_in;

            for (i = 0; i < uDoCopy; ++i)
                *(pfileInZipReadInfo->stream.next_out + i) = *(pfileInZipReadInfo->stream.next_in + i);

            pfileInZipReadInfo->crc32 = crc32(pfileInZipReadInfo->crc32,
                                pfileInZipReadInfo->stream.next_out,
                                uDoCopy);
            pfileInZipReadInfo->restReadUncompressed -= uDoCopy;
            pfileInZipReadInfo->stream.avail_in -= uDoCopy;
            pfileInZipReadInfo->stream.avail_out -= uDoCopy;
            pfileInZipReadInfo->stream.next_out += uDoCopy;
            pfileInZipReadInfo->stream.next_in += uDoCopy;
            pfileInZipReadInfo->stream.total_out += uDoCopy;
            iRead += uDoCopy;
        }
        else
        {
            uLong uTotalOutBefore,uTotalOutAfter;
            const Bytef *bufBefore;
            uLong uOutThis;
            int flush = Z_SYNC_FLUSH;

            uTotalOutBefore = pfileInZipReadInfo->stream.total_out;
            bufBefore = pfileInZipReadInfo->stream.next_out;

            /*
            if ((pfileInZipReadInfo->restReadUncompressed ==
                     pfileInZipReadInfo->stream.avail_out) &&
                (pfileInZipReadInfo->restReadCompressed == 0))
                flush = Z_FINISH;
            */
            status = inflate(&pfileInZipReadInfo->stream, flush);

            uTotalOutAfter = pfileInZipReadInfo->stream.total_out;
            uOutThis = uTotalOutAfter - uTotalOutBefore;

            pfileInZipReadInfo->crc32 = crc32(pfileInZipReadInfo->crc32, bufBefore, (uInt)(uOutThis));

            pfileInZipReadInfo->restReadUncompressed -= uOutThis;

            iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

            if (status == Z_STREAM_END)
                return (iRead == 0) ? UNZ_EOF : iRead;
            if (status != Z_OK)
                break;
        }
    }
    if (status == Z_OK)
        return iRead;

    return status;
}


/*====================
  _eof

  return 1 if the end of pFile was reached, 0 elsewhere
  FIXME UH
  ====================
int CUnzip::_eof () {
    SZipFileInfo* pfileInZipReadInfo;

    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    pfileInZipReadInfo = m_pzipFile->pfileInZipRead;

    if (pfileInZipReadInfo == NULL)
        return UNZ_PARAMERROR;

    if (pfileInZipReadInfo->restReadUncompressed == 0)
        return 1;
    else
        return 0;
}


====================
  unzGetLocalExtrafield

  Read extra field from the current pFile (opened by unzOpenCurrentFile)
  This is the local - header version of the extra field (sometimes, there is
  more info in the local - header version than in the central - header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, iLen is the size of the buffer, the extra header is copied in
  buf.

  the return value is the number of bytes copied in buf, or (if <0)
  the error code
  ====================*/
int     CUnzip::GetLocalExtraField (voidp pBuf, uint iLen)
{
    SZipFileInfo *pfileInZipReadInfo;
    uInt read_now;
    uLong size_to_read;

    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    pfileInZipReadInfo = m_pzipFile->pfileInZipRead;

    if (pfileInZipReadInfo == NULL)
        return UNZ_PARAMERROR;

    size_to_read = (pfileInZipReadInfo->sizeLocalExtrafield -
                pfileInZipReadInfo->posLocalExtrafield);

    if (pBuf == NULL)
        return (int)size_to_read;

    if (iLen > size_to_read)
        read_now = (uInt)size_to_read;
    else
        read_now = (uInt)iLen;

    if (read_now == 0)
        return 0;

    if (fseek(pfileInZipReadInfo->pfile,
              pfileInZipReadInfo->offsetLocalExtrafield +
              pfileInZipReadInfo->posLocalExtrafield, SEEK_SET) != 0)
        return UNZ_ERRNO;

    if (fread(pBuf, (uInt)size_to_read, 1, pfileInZipReadInfo->pfile)!=1)
        return UNZ_ERRNO;

    return (int)read_now;
}


/*====================
  CUnzip::CloseCurrentFile

  Close the pFile in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the pFile was read but the CRC is not good
  ====================*/
int     CUnzip::CloseCurrentFile(SZipFile *pFile)
{
    int status = UNZ_OK;
    SZipFileInfo *pfileInZipReadInfo;

    if (pFile == NULL)
        return UNZ_PARAMERROR;

    pfileInZipReadInfo = pFile->pfileInZipRead;

    if (pfileInZipReadInfo == NULL)
        return UNZ_PARAMERROR;

    if (pfileInZipReadInfo->restReadUncompressed == 0)
    {
        if (pfileInZipReadInfo->crc32 != pfileInZipReadInfo->crc32Wait)
            status = UNZ_CRCERROR;
    }

    K2_DELETE_ARRAY(pfileInZipReadInfo->preadBuffer);
    pfileInZipReadInfo->preadBuffer = NULL;
    if (pfileInZipReadInfo->streamInitialised)
        inflateEnd(&pfileInZipReadInfo->stream);

    pfileInZipReadInfo->streamInitialised = 0;
    K2_DELETE(pfileInZipReadInfo);

    pFile->pfileInZipRead = NULL;

    return status;
}


/*====================
  CUnzip::_goToFirstFile
  ====================*/
int CUnzip::GoToFirstFile()
{
    int status = UNZ_OK;

    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    m_pzipFile->posCentralDir = m_pzipFile->offsetCentralDir;
    m_pzipFile->fileNum = 0;
    status = GetCurrentFileInfoInternal(&m_pzipFile->curFileInfo, &m_pzipFile->curFileInfoInternal,
        NULL, 0, NULL, 0, NULL, 0);
    m_pzipFile->fileOk = (status == UNZ_OK);
    return status;
}


/*====================
  CUnzip::_goToNextFile
  ====================*/
int CUnzip::GoToNextFile()
{
    int status;

    if (m_pzipFile == NULL)
        return UNZ_PARAMERROR;

    if (!m_pzipFile->fileOk)
        return UNZ_END_OF_LIST_OF_FILE;

    if (m_pzipFile->fileNum + 1 == m_pzipFile->globalInfo.numberEntry)
        return UNZ_END_OF_LIST_OF_FILE;

    m_pzipFile->posCentralDir += SIZECENTRALDIRITEM + m_pzipFile->curFileInfo.sizeFilename +
            m_pzipFile->curFileInfo.sizeFileExtra + m_pzipFile->curFileInfo.sizeFileComment;
    ++m_pzipFile->fileNum;
    status = GetCurrentFileInfoInternal(&m_pzipFile->curFileInfo,
        &m_pzipFile->curFileInfoInternal, NULL,0,NULL,0,NULL,0);
    m_pzipFile->fileOk = (status == UNZ_OK);
    return status;
}


/*====================
  CUnzip::openUnzipFile
  ====================*/
uint    CUnzip::OpenUnzipFile(const tstring &sFilename, char *&pBuffer)
{
    tstring lowerFilename;

    // must have a valid archive
    if (m_pzipFile == NULL)
        return 0;

    // copy archive structure
    SZipFile backup;
    MemManager.Copy(&backup, m_pzipFile, sizeof(SZipFile));

    lowerFilename = LowerString(sFilename);

    // look up pFile
    FilenameMap::iterator file_entry = m_pzipFile->pFilenameMap->find(lowerFilename.c_str());

    // didn't found a valid entry
    if (file_entry == m_pzipFile->pFilenameMap->end())
        return 0;

    // retrieve info about the pFile
    m_pzipFile->posCentralDir = file_entry->second;
    int status = GetCurrentFileInfoInternal(&m_pzipFile->curFileInfo, &m_pzipFile->curFileInfoInternal,
        NULL, 0, NULL, 0, NULL, 0);
    if (status != UNZ_OK)
    {
        MemManager.Copy(m_pzipFile, &backup, sizeof(SZipFile));
        return 0;
    }

    // read (and uncompress) the contents into a buffer
    OpenCurrentFile();
    int size = m_pzipFile->curFileInfo.uncompressedSize;
    pBuffer = K2_NEW_ARRAY(ctx_FileSystem, char, size);
    ReadCurrentFile(m_pzipFile, pBuffer, size);
    CloseCurrentFile(m_pzipFile);

    // restore archive info
    MemManager.Copy(m_pzipFile, &backup, sizeof(SZipFile));

    return size;
}


/*====================
  UNUSED CUnzip::getGlobalInfo

  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
  ====================
int CZipFile::getGlobalInfo () {
    SZipFile *pzipfileInfInternal;
    if (m_hArchive == NULL)
        return UNZ_PARAMERROR;
    pzipfileInfInternal = (SZipFile*)m_hArchive;
    m_info = pzipfileInfInternal->gi;
    return UNZ_OK;
}


UNUSED CUnzip::_getGlobalComment
int CUnzip::_getGlobalComment (char *szComment, uLong uSizeBuf) {
    SZipFile *pzipfileInfInternal;
    uLong uReadThis;
    if (m_hArchive == NULL)
        return UNZ_PARAMERROR;
    pzipfileInfInternal = (SZipFile *)pFile;

    uReadThis = uSizeBuf;
    if (uReadThis > pzipfileInfInternal->globalInfo.size_comment)
        uReadThis = pzipfileInfInternal->globalInfo.size_comment;

    if (fseek(pzipfileInfInternal->pfile, pzipfileInfInternal->centralPos + 22, SEEK_SET) != 0)
        return UNZ_ERRNO;

    if (uReadThis > 0) {
      *szComment = '\0';
      if (fread(szComment, (uInt)uReadThis, 1, pzipfileInfInternal->pfile) != 1)
        return UNZ_ERRNO;
    }

    if ((szComment != NULL) && (uSizeBuf > pzipfileInfInternal->globalInfo.size_comment))
        *(szComment + pzipfileInfInternal->globalInfo.size_comment)='\0';
    return (int)uReadThis;
}
/**/
