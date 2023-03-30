// (C)2010 S2 Games
// c_filestream.h
//
// CFileStream
// Provides streaming access to a file on disk, or a compressed file within
// a zip file
//=============================================================================
#ifndef __C_FILESTREAM__
#define __C_FILESTREAM__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
typedef struct z_stream_s   z_stream;
//=============================================================================


//=============================================================================
// CFileStream
//=============================================================================
class CFileStream : public CFile
{
private:
    FILE*               m_File;

    // File state
    byte*               m_pBuffer;
    mutable uint        m_uiReadSize;

    // Compressed state
    mutable z_stream*   m_pZlib;
    uint                m_uiCompressedOffset;
    uint                m_uiCompressedSize;

    bool        EnsureData(uint uiSize) const;

    uint        ReadCharacter();


    FILE*       LocateCompressedFile(const tstring& sZipFilePath, const tstring& sFilePath,
                                    uint& uiCompressedOffset, uint& uiCompressedSize, uint& uiRawSize);
    bool        AcquireCompressedFile(FILE* fp, uint uiCompressedOffset, uint uiCompressedSize, uint uiRawSize, int iMode);

public:
    ~CFileStream()  { Close(); }
    CFileStream();

    bool        IsBigEndian() const                     { return (m_iMode & FILE_BIG_ENDIAN) == FILE_BIG_ENDIAN; }

    bool        Open(const tstring &sPath, int iMode);
    bool        OpenCompressed(const tstring &sZipFile, const tstring &sPath, int iMode);

    void        Close();
    bool        IsOpen() const;

    tstring     ReadLine();
    uint        Read(char *pBuffer, uint uiBufferSize) const;
    size_t      Write(const void *pBuffer, size_t iBufferSize);

    const char* GetBuffer(uint &uiSize);

    bool        WriteByte(char c);
    bool        WriteInt16(short n, bool bUseBigEndian);
    bool        WriteInt32(int i, bool bUseBigEndian);
    bool        WriteInt64(LONGLONG ll, bool bUseBigEndian);
    bool        WriteString(const string &sText);
    bool        WriteString(const tstring &sText);

    uint        Tell() const;
    bool        Seek(uint uiOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START);

    void        Flush();
};
#endif
//=============================================================================

