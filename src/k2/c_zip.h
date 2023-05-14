// (C)2005 S2 Games
// c_zip.h
//
// support for zipping .zip files - based on the zlib minizip package
//=============================================================================
#ifndef __C_ZIP_H__
#define __C_ZIP_H__

//=============================================================================
// Headers
//=============================================================================
#include <zlib.h>
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define UNZ_OK                  (0)
#define UNZ_END_OF_LIST_OF_FILE (-100)
#define UNZ_ERRNO               (Z_ERRNO)
#define UNZ_EOF                 (0)
#define UNZ_PARAMERROR          (-102)
#define UNZ_BADZIPFILE          (-103)
#define UNZ_INTERNALERROR       (-104)
#define UNZ_CRCERROR            (-105)

#define ZIP_OK                  (0)
#define ZIP_ERRNO               (Z_ERRNO)
#define ZIP_PARAMERROR          (-102)
#define ZIP_INTERNALERROR       (-104)

const int       ZIP_DEFAULT_COMPRESSION_LEVEL(6);

class CZipCentralFileHeader;
class CZipCentralDirectory;
class CCompressedFile;
//=============================================================================

//=============================================================================
// CZip
//=============================================================================
class CZip
{
private:
    tstring                         m_sPathName;
    fstream                         m_file;
    CZipCentralDirectory*           m_pCentralDirectory;
    vector<CZipCentralFileHeader*>  m_vCentralHeaders;
    
    bool                            m_bWriting;
    tstring                         m_sTempPath;
    fstream                         m_fileTemp;
    vector<CZipCentralFileHeader*>::iterator m_itWrite;
    uint                            m_uiWritePosition;
    float                           m_fWriteProgress;

    void    ReadCentralDirectory();
    bool    WriteToDisk(uint uiMaxTime = INVALID_TIME);

    CZip();

public:
    ~CZip();
    CZip(const tstring &sFilename, bool bAppend);

    bool    IsOpen() const      { return m_file.is_open() && m_pCentralDirectory != nullptr; }

    bool    Open(const tstring &sPathname, bool bAppend);
    int     AddFile(const tstring &sFileName, const char *pSourceBuffer, size_t zLength, int iLevel = ZIP_DEFAULT_COMPRESSION_LEVEL, time_t t = 0, const string &sComment = "");
    int     AddCompressedFile(const tstring &sFileName, const CCompressedFile &cFile, time_t t = 0, const string &sComment = "");
    int     RemoveFile(const tstring &sFileName);
    bool    Close(uint uiMaxTime = INVALID_TIME);
    bool    CancelWrite();

    bool    FileExists(const tstring &sFilename);

    K2_API static uint  Compress(const byte *pSource, uint uiSourceLength, byte *&pDest, int iLevel = ZIP_DEFAULT_COMPRESSION_LEVEL);
    K2_API static bool  Decompress(const byte *pSource, uint uiSourceLength, byte *pDest, uint uiDestLength);

    float   GetWriteProgress() const        { return m_fWriteProgress; }
};
//=============================================================================

#endif //__C_ZIP_H__
