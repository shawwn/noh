// (C)2005 S2 Games
// c_filedisk.h
//
// CFileDisk
// Provides access to a file that exists in the normal directory structure of
// a drive
//=============================================================================
#ifndef __C_FILEDISK__
#define __C_FILEDISK__

//=============================================================================
// Headers
//=============================================================================
#include "c_file.h"

#include <fstream>

using std::fstream;
using std::ios_base;
//=============================================================================

//=============================================================================
// CFileDisk
//=============================================================================
class CFileDisk : public CFile
{
private:
    mutable fstream m_File;

public:
    ~CFileDisk()    { Close(); }
    CFileDisk();

    bool        Open(const tstring &sPath, int iMode);
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
    bool        WriteString(const wstring &sText);

    uint        Tell() const;
    bool        Seek(uint uiOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START);

    void        Flush();
};
#endif
//=============================================================================
