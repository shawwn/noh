// (C)2010 S2 Games
// c_filebuffer.h
//=============================================================================
#ifndef __C_FILEBUFFER_H__
#define __C_FILEBUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// CFileBuffer
//=============================================================================
class CFileBuffer : public CFile
{
private:
    CBufferDynamic  m_bufferWrite;
    time_t          m_tModTime;

public:
    ~CFileBuffer();
    CFileBuffer();

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

#endif //__C_FILEBUFFER_H__

