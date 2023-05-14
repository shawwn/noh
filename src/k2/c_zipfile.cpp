// (C)2005 S2 Games
// c_zipfile.cpp
//
// parent class for zip / unzip functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifndef _WIN32
#include <utime.h>
#endif

#include "c_zipfile.h"
//=============================================================================


/*====================
  CZipFile::getCurrentFileInfoInternal
 ====================*/
int     CZipFile::GetCurrentFileInfoInternal (zipFileInfo_t *pFileInfo, zipFileInfoInternal_t *pFileInfoInternal,
                                              TCHAR *szFilename, uLong ulFilenameBufferSize, void *pExtraField,
                                              uLong ulExtraFieldBufferSize, char *szComment, uLong ulCommentBufferSize)
{
    zipFileInfo_t fileInfo;
    zipFileInfoInternal_t fileInfoInternal;
    int status = UNZ_OK;
    uLong uMagic;
    long lSeek = 0;

    SZipFile *pfileInfInternal((SZipFile *)nullptr/*m_pzipFile*/);
    if (fseek(pfileInfInternal->pfile, pfileInfInternal->posCentralDir
        + pfileInfInternal->byteBefore, SEEK_SET) != 0)
        status = UNZ_ERRNO;

    /* we check the magic */
    if (status == UNZ_OK) {
        if (GetLong(pfileInfInternal->pfile, &uMagic) != UNZ_OK)
            status = UNZ_ERRNO;
        else if (uMagic!=0x02014b50)
            status = UNZ_BADZIPFILE;
    }

    if (GetShort(pfileInfInternal->pfile, &fileInfo.version) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.versionNeeded) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.flag) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.compressionMethod) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetLong(pfileInfInternal->pfile, &fileInfo.dosDate) != UNZ_OK)
        status = UNZ_ERRNO;

    DosDateToTmuDate(fileInfo.dosDate, &fileInfo.timeInfo);

    if (GetLong(pfileInfInternal->pfile, &fileInfo.crc) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetLong(pfileInfInternal->pfile, &fileInfo.compressedSize) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetLong(pfileInfInternal->pfile, &fileInfo.uncompressedSize) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.sizeFilename) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.sizeFileExtra) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.sizeFileComment) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.diskNumStart) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetShort(pfileInfInternal->pfile, &fileInfo.internalFA) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetLong(pfileInfInternal->pfile, &fileInfo.externalFA) != UNZ_OK)
        status = UNZ_ERRNO;

    if (GetLong(pfileInfInternal->pfile, &fileInfoInternal.offsetCurfile) != UNZ_OK)
        status = UNZ_ERRNO;

    lSeek += fileInfo.sizeFilename;
    if ((status == UNZ_OK) && (szFilename!=nullptr))
    {
        uLong uSizeRead;
        if (fileInfo.sizeFilename < ulFilenameBufferSize)
        {
            *(szFilename + fileInfo.sizeFilename)='\0';
            uSizeRead = fileInfo.sizeFilename;
        }
        else
            uSizeRead = ulFilenameBufferSize;

        if ((fileInfo.sizeFilename > 0) && (ulFilenameBufferSize > 0))
            if (fread(szFilename, (uInt)uSizeRead, 1, pfileInfInternal->pfile)!=1)
                status = UNZ_ERRNO;
        lSeek -= uSizeRead;
    }


    if ((status == UNZ_OK) && (pExtraField != nullptr))
    {
        uLong uSizeRead;
        if (fileInfo.sizeFileExtra < ulExtraFieldBufferSize)
            uSizeRead = fileInfo.sizeFileExtra;
        else
            uSizeRead = ulExtraFieldBufferSize;

        if (lSeek!=0)
        {
            if (fseek(pfileInfInternal->pfile, lSeek, SEEK_CUR) == 0)
                lSeek = 0;
            else
                status = UNZ_ERRNO;
        }

        if ((fileInfo.sizeFileExtra > 0) && (ulExtraFieldBufferSize > 0))
            if (fread(pExtraField, (uInt)uSizeRead, 1, pfileInfInternal->pfile)!=1)
                status = UNZ_ERRNO;
        lSeek += fileInfo.sizeFileExtra - uSizeRead;
    }
    else
        lSeek += fileInfo.sizeFileExtra;


    if ((status == UNZ_OK) && (szComment!=nullptr))
    {
        uLong uSizeRead;
        if (fileInfo.sizeFileComment < ulCommentBufferSize)
        {
            *(szComment + fileInfo.sizeFileComment)='\0';
            uSizeRead = fileInfo.sizeFileComment;
        }
        else
            uSizeRead = ulCommentBufferSize;

        if (lSeek!=0)
        {
            if (fseek(pfileInfInternal->pfile, lSeek, SEEK_CUR) == 0)
                lSeek = 0;
            else
                status = UNZ_ERRNO;
        }

        if ((fileInfo.sizeFileComment > 0) && (ulCommentBufferSize > 0))
            if (fread(szComment, (uInt)uSizeRead, 1, pfileInfInternal->pfile)!=1)
                status = UNZ_ERRNO;
        lSeek+=fileInfo.sizeFileComment - uSizeRead;
    }
    else
        lSeek+=fileInfo.sizeFileComment;

    if ((status == UNZ_OK) && sizeof(pfileInfInternal->curFileInfo) > 0)
        pfileInfInternal->curFileInfo = fileInfo;

    if ((status == UNZ_OK) && sizeof(pfileInfInternal) > 0)
        pfileInfInternal->curFileInfoInternal = fileInfoInternal;
    return status;
}


/*====================
  GetByte
 ====================*/
int     CZipFile::GetByte (FILE *pFileIn, int *iPi)
{
    unsigned char c;
    size_t status = fread(&c, 1, 1, pFileIn);

    if (status == 1)
    {
        *iPi = (int)c;
        return UNZ_OK;
    }
    else
    {
        if (ferror(pFileIn))
            return UNZ_ERRNO;
        else
            return UNZ_EOF;
    }
}


/*====================
  getShort
  ====================*/
int CZipFile::GetShort (FILE *pFileIn, uLong *pX)
{
    uLong x;
    int i, status;

    status = GetByte(pFileIn, &i);
    x = (uLong)i;

    if (status == UNZ_OK)
        status = GetByte(pFileIn, &i);
    x += ((uLong)i) << 8;

    if (status == UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return status;
}

/*====================
  getLong
  ====================*/
int CZipFile::GetLong (FILE *pFileIn, uLong *pX)
{
    uLong x;
    int i, status;

    status = GetByte(pFileIn, &i);
    x = (uLong)i;

    if (status == UNZ_OK)
        status = GetByte(pFileIn, &i);
    x += ((uLong)i) << 8;

    if (status == UNZ_OK)
        status = GetByte(pFileIn, &i);
    x += ((uLong)i) << 16;

    if (status == UNZ_OK)
        status = GetByte(pFileIn, &i);
    x += ((uLong)i) << 24;

    if (status == UNZ_OK)
        *pX = x;
    else
        *pX = 0;
    return status;
}


/*====================
  CZipFile::getCurrentFileInfo

  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
  ====================*/
int     CZipFile::GetCurrentFileInfo (zipFileInfo_t *pFileInfo, TCHAR *szFilename, uLong ulFilenameBufferSize,
                                      void *pExtraField, uLong ulExtraFieldBufferSize, char *szComment,
                                      uLong ulCommentBufferSize)
{
        return GetCurrentFileInfoInternal(pFileInfo, nullptr, szFilename, ulFilenameBufferSize, pExtraField,
            ulExtraFieldBufferSize, szComment, ulCommentBufferSize);
}


/*====================
  CZipFile::DosDateToTmuDate
  ====================*/
void    CZipFile::DosDateToTmuDate(uLong ulDosDate, SZipTime* ptm)
{
    uLong uDate;
    uDate = (uLong)(ulDosDate >> 16);
    ptm->tm_mday = (uInt)(uDate & 0x1f);
    ptm->tm_mon =  (uInt)((((uDate) & 0x1E0)/0x20) - 1);
    ptm->tm_year = (uInt)(((uDate & 0x0FE00)/0x0200) + 1980);

    ptm->tm_hour = (uInt) ((ulDosDate &0xF800) / 0x800);
    ptm->tm_min =  (uInt) ((ulDosDate & 0x7E0) / 0x20);
    ptm->tm_sec =  (uInt) (2 * (ulDosDate & 0x1f));
}
