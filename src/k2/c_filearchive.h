// (C)2005 S2 Games
// c_filearchive.h
//
// CFileArchive
// Provides access to a file that exists within an archive
//=============================================================================
#ifndef __C_FILEARCHIVE__
#define __C_FILEARCHIVE__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"
#include "c_archive.h"
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// CFileArchive
//=============================================================================
class CFileArchive : public CFile
{
private:
    CBufferDynamic  m_bufferWrite;
    CArchive*       m_pArchive;
    time_t          m_tModTime;

public:
    ~CFileArchive();
    CFileArchive(CArchive *pArchive);

    bool        Open(const tstring &sPath, int iMode);
    void        Close();
    bool        IsOpen() const;

    size_t      ReadLine(char *pBuffer, int iBufferSize);
    tstring     ReadLine();

    uint        Read(char *pBuffer, uint uiBufferSize) const;
    size_t      Write(const void *pBuffer, size_t iBufferSize);

    const char* GetBuffer(uint &uiSize);

    bool        WriteByte(char c);
    bool        WriteInt16(short c, bool bUseBigEndian);
    bool        WriteInt32(int c, bool bUseBigEndian);
    bool        WriteInt64(LONGLONG c, bool bUseBigEndian);
    bool        WriteString(const string &sText);
    bool        WriteString(const wstring &sText);

    virtual void        SetModificationTime(time_t tModTime)        { m_tModTime = tModTime; }
};
//=============================================================================

#endif //__C_FILEARCHIVE__

