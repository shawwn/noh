// (C)2005 S2 Games
// c_filehandle.h
//
//=============================================================================
#ifndef __C_FILEHANDLE_H__
#define __C_FILEHANDLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "c_file.h"
#include "xtoa.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CArchive;
class IBuffer;
//=============================================================================

//=============================================================================
// CFileHandle
// Provides a generic interface for accessing files of any type
//=============================================================================
class K2_API CFileHandle
{
    friend class CFile;
    friend class CFileHTTP;
    friend class CFileDisk;
    friend class CFileArchive;

private:
    CFile       *m_pFile;

public:
    ~CFileHandle();
    CFileHandle();
    CFileHandle(const tstring &sPath, int iMode, const tstring &sMod = TSNULL);
    CFileHandle(const tstring &sPath, int iMode, CArchive& archive);

    bool            Open(const tstring &sFilePath, int iMode, const tstring &sMod = TSNULL);
    bool            Open(const tstring &sFilePath, int iMode, CArchive& hArchive);
    void            Close();
    bool            IsOpen() const;
    bool            IsEOF() const;

    CFile*          GetFile()                                       { return m_pFile; }

    tstring         ReadLine();
    bool            WriteLine(const string &sText)                  { return WriteString(sText + LINEBREAK); }
    bool            WriteLine(const wstring &sText)                 { return WriteString(sText + WLINEBREAK); }

    int             Read(char *pBuffer, int iSize) const;
    int             Read(IBuffer &cBuffer, int iSize) const;
    size_t          Write(const void *pBuffer, size_t size);

    const char*     GetBuffer(uint &uiSize);

    const tstring&  GetPath() const;

    byte            ReadByte();
    short           ReadInt16(bool bUseBigEndian = false);
    int             ReadInt32(bool bUseBigEndian = false);
    LONGLONG        ReadInt64(bool bUseBigEndian = false);
    float           ReadFloat(bool bUseBigEndian = false);
    wstring         ReadWString();

    size_t          Tell();
    bool            Seek(int iOffset, ESeekOrigin eOrigin = SEEK_ORIGIN_START);
    size_t          GetLength();

    // TODO: Make all writing functions differentiate between text and binary mode
    bool            WriteByte(char c);
    bool            WriteInt16(short i, bool bUseBigEndian = false);
    bool            WriteInt32(int i, bool bUseBigEndian = false);
    bool            WriteInt64(LONGLONG ll, bool bUseBigEndian = false);
    bool            WriteByte(byte c)                                       { return WriteByte(char(c)); }
    bool            WriteInt16(word w, bool bUseBigEndian = false)          { return WriteInt16(short(w), bUseBigEndian); }
    bool            WriteInt32(uint ui, bool bUseBigEndian = false)         { return WriteInt32(int(ui), bUseBigEndian); }
    bool            WriteInt64(ULONGLONG ull, bool bUseBigEndian = false)   { return WriteInt64(LONGLONG(ull), bUseBigEndian); }
    bool            WriteString(const string &s);
    bool            WriteString(const wstring &s);

    CFileHandle&    operator<<(const char *sz)                      { WriteString(sz); return *this; }
    CFileHandle&    operator<<(const wchar_t *sz)                   { WriteString(sz); return *this; }
    CFileHandle&    operator<<(const string &s)                     { WriteString(s); return *this; }
    CFileHandle&    operator<<(const wstring &s)                    { WriteString(s); return *this; }
    CFileHandle&    operator<<(char c)                              { WriteString(string(1, c)); return *this; }
    CFileHandle&    operator<<(wchar_t c)                           { WriteString(wstring(1, c)); return *this; }
    CFileHandle&    operator<<(int i)                               { *this << XtoA(i); return *this; }
    CFileHandle&    operator<<(long l)                              { *this << XtoA(l); return *this; }
    CFileHandle&    operator<<(uint ui)                             { *this << XtoA(ui); return *this; }
    CFileHandle&    operator<<(unsigned long ul)                    { *this << XtoA(ul); return *this; }
    CFileHandle&    operator<<(LONGLONG ll)                         { *this << XtoA(ll); return *this; }
    CFileHandle&    operator<<(float f)                             { *this << XtoA(f); return *this; }
    CFileHandle&    operator<<(double d)                            { *this << XtoA(d); return *this; }

    void            Flush();

    void            SetModificationTime(time_t tModTime);
};
//=============================================================================

#endif //__C_FILEHANDLE_H__
