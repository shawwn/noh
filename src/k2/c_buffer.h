// (C)2005 S2 Games
// c_buffer.h
//
// All multi-byte values are stored in host endian order
// numeric types (and associated cvec's) are stored in little endian order and converted to/from host endian format when read/written
//=============================================================================
#ifndef __C_BUFFER_H__
#define __C_BUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_exception.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int BUFFER_FAULT_INITIALIZE(1);   // Failure in Init()
const int BUFFER_FAULT_COPY(2);         // Failure in copy ctor
const int BUFFER_FAULT_ALLOCATE(4);     // Failed to allocate memory
const int BUFFER_FAULT_OVERRUN(8);      // A write to a static buffer was truncated
const int BUFFER_FAULT_UNDERRUN(16);    // A read went beyond the length of the buffer

const size_t DEFAULT_CBUFFER_SIZE = 64;
//=============================================================================


//=============================================================================
// IBuffer
// Base class
//=============================================================================
#ifdef __GNUC__
class __attribute__((visibility("default"))) IBuffer
#else
class IBuffer
#endif
{
protected:
    char*           m_pBuffer;
    uint            m_uiSize;
    uint            m_uiEnd;
    mutable uint    m_uiRead;
    mutable int     m_iFaults;

    K2_API void     Init(uint uiSize);

public:
    K2_API IBuffer();
    K2_API IBuffer(const IBuffer &buffer);
    K2_API virtual ~IBuffer();

    virtual inline void         Clear(char c);
    virtual inline void         Clear();

    size_t                      GetCapacity() const         { return m_uiSize; }
    virtual uint                GetLength() const           { return m_uiEnd; }
    bool                        IsEmpty() const             { return GetLength() == 0; }
    virtual uint                GetReadPos() const          { return m_uiRead; }
    uint                        GetUnreadLength() const     { return GetLength() - GetReadPos(); }
    int                         GetFaults() const           { return m_iFaults; }
    void                        ClearFaults()               { m_iFaults = 0; }
    void                        SetLength(uint uiLength)    { m_uiEnd = uiLength; }

    K2_API bool                 Resize(uint uiSize);
    K2_API bool                 Reserve(uint uiSize);
    K2_API bool                 Reallocate(uint uiSize);

    virtual inline IBuffer&     operator=(const TCHAR *sz);
    virtual inline IBuffer&     operator=(const IBuffer &B);

    virtual inline char&        operator[](uint uiIndex);
    virtual inline char         operator[](uint uiIndex) const;

    virtual bool                Seek(uint uiPos) = 0;
    virtual void                Rewind() const              { m_uiRead = 0; }
    K2_API virtual uint         FindNext(char c) const;
    K2_API virtual uint         FindNext(wchar_t c) const;

    virtual inline char*        Duplicate(uint uiOffset, uint uiLength);
    virtual char*               Duplicate(uint uiOffset)    { return Duplicate(uiOffset, m_uiEnd - uiOffset); }
    virtual char*               Duplicate()                 { return Duplicate(0, m_uiEnd); }

    virtual inline const char*  Get(uint uiOffset) const;
    virtual const char*         Get() const                 { return Get(0); }

    virtual bool                Append(const void *pBuffer, uint uiSize) = 0;
    virtual bool                Write(const void *pBuffer, uint uiSize) = 0;
    virtual bool                Read(void *pBuffer, uint uiSize) const = 0;

    virtual char*               Lock(uint uiSize)           { return nullptr; }
    //virtual inline bool           Advance(uint uiSize) const  { return false; }
    virtual bool                WriteBytes(byte yValue, uint uiSize)    { return false; }

    byte                        ReadByte() const            { byte y; Read(&y, sizeof(byte)); return y; }
    short                       ReadShort() const           { short n; Read(&n, sizeof(short)); return LittleShort(n); }
    int                         ReadInt() const             { int i; Read(&i, sizeof(int)); return LittleInt(i); }
    LONGLONG                    ReadInt64() const           { LONGLONG ll; Read(&ll, sizeof(LONGLONG)); return LittleInt64(ll); }
    float                       ReadFloat() const           { float f; Read(&f, sizeof(float)); return LittleFloat(f); }
    double                      ReadDouble() const          { double d; Read(&d, sizeof(double)); return LittleDouble(d); }
    K2_API string               ReadString() const;
    K2_API wstring              ReadWString() const;
    K2_API tstring              ReadTString() const;

    void                        WriteByte(byte y)           { Append(&y, sizeof(y)); }
    void                        WriteShort(short n)         { ToLittle(n); Append(&n, sizeof(n)); }
    void                        WriteInt(int i)             { ToLittle(i); Append(&i, sizeof(i)); }
    void                        WriteInt64(LONGLONG ll)     { ToLittle(ll); Append(&ll, sizeof(ll)); }
    void                        WriteFloat(float f)         { ToLittle(f); Append(&f, sizeof(f)); }
    void                        WriteDouble(double d)       { ToLittle(d); Append(&d, sizeof(d)); }
    
    inline int                  CompareBuffer(const IBuffer &cBuffer, uint uiSize) const;
    inline int                  CompareBuffer4(const IBuffer &cBuffer) const;
};

/*====================
  IBuffer::Clear
  ====================*/
inline
void    IBuffer::Clear()
{
    m_uiRead = m_uiEnd = 0;
    ClearFaults();
}


/*====================
  IBuffer::Clear
  ====================*/
inline
void    IBuffer::Clear(char c)
{
    MemManager.Set(m_pBuffer, c, m_uiSize);
    m_uiRead = m_uiEnd = 0;
    ClearFaults();
}


/*====================
  operator<<
  ====================*/
inline
IBuffer&    operator<<(IBuffer &buffer, byte y)
{
    buffer.Append((char*)&y, sizeof(byte));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, char c)
{
    buffer.Append((char*)&c, sizeof(char));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, wchar_t c)
{
    buffer.Append((char*)&c, sizeof(wchar_t));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, short n)
{
    buffer.WriteShort(n);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, ushort un)
{
    buffer.WriteShort(un);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, int i)
{
    buffer.WriteInt(i);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, uint ui)
{
    buffer.WriteInt(ui);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, LONGLONG ll)
{
    buffer.WriteInt64(ll);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, ULONGLONG ull)
{
    buffer.WriteInt64(ull);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, float f)
{
    buffer.WriteFloat(f);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, double d)
{
    buffer.WriteDouble(d);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const char *sz)
{
    buffer.Append(sz, uint(strlen(sz)) * sizeof(char));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const wchar_t *sz)
{
    buffer.Append(sz, uint(wcslen(sz)) * sizeof(wchar_t));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const string &s)
{
    buffer.Append(s.c_str(), uint(s.length()) * sizeof(char));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const wstring &s)
{
    buffer.Append(s.c_str(), uint(s.length()) * sizeof(wchar_t));
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const CVec2f &v2)
{
    buffer.WriteFloat(v2.x);
    buffer.WriteFloat(v2.y);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const CVec3f &v3)
{
    buffer.WriteFloat(v3.x);
    buffer.WriteFloat(v3.y);
    buffer.WriteFloat(v3.z);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const CVec2<short> &v2)
{
    buffer.WriteShort(v2.x);
    buffer.WriteShort(v2.y);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &buffer, const CVec3<short> &v3)
{
    buffer.WriteShort(v3.x);
    buffer.WriteShort(v3.y);
    buffer.WriteShort(v3.z);
    return buffer;
}

inline
IBuffer&    operator<<(IBuffer &bufferA, const IBuffer &bufferB)
{
    if (bufferB.GetLength() == 0)
        return bufferA;

    bufferA.Append(bufferB.Get(), bufferB.GetLength());
    return bufferA;
}


/*====================
  operator>>
  ====================*/
inline
const IBuffer&  operator>>(const IBuffer &buffer, byte &y)
{
    buffer.Read(&y, sizeof(byte));
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, char &c)
{
    buffer.Read(&c, sizeof(char));
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, wchar_t &c)
{
    buffer.Read(&c, sizeof(wchar_t));
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, short &n)
{
    n = buffer.ReadShort();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, ushort &un)
{
    un = ushort(buffer.ReadShort());
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, int &i)
{
    i = buffer.ReadInt();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, uint &ui)
{
    ui = uint(buffer.ReadInt());
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, LONGLONG &ll)
{
    ll = buffer.ReadInt64();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, ULONGLONG &ull)
{
    ull = ULONGLONG(buffer.ReadInt64());
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, float &f)
{
    f = buffer.ReadFloat();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, double &d)
{
    d = buffer.ReadDouble();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, CVec2f &v2)
{
    v2.x = buffer.ReadFloat();
    v2.y = buffer.ReadFloat();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, CVec3f &v3)
{
    v3.x = buffer.ReadFloat();
    v3.y = buffer.ReadFloat();
    v3.z = buffer.ReadFloat();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, CVec2<short> &v2)
{
    v2.x = buffer.ReadShort();
    v2.y = buffer.ReadShort();
    return buffer;
}

inline
const IBuffer&  operator>>(const IBuffer &buffer, CVec3<short> &v3)
{
    v3.x = buffer.ReadShort();
    v3.y = buffer.ReadShort();
    v3.z = buffer.ReadShort();
    return buffer;
}

/*template <class T>
inline
const IBuffer&  operator>>(const IBuffer &buffer, T &x)
{
    buffer.Read(&x, sizeof(T));
    return buffer;
}*/


/*====================
  IBuffer::operator=
  ====================*/
inline
IBuffer&    IBuffer::operator=(const TCHAR *sz)
{
    Write(sz, uint(_tcslen(sz) * sizeof(TCHAR)));
    return *this;
}

inline
IBuffer&    IBuffer::operator=(const IBuffer &B)
{
    Write(B.Get(), B.GetLength());
    return *this;
}


/*====================
  IBuffer::operator[]
  ====================*/
inline
char&   IBuffer::operator[](uint uiIndex)
{
    if (uiIndex >= m_uiEnd)
        EX_ERROR(_T("IBuffer::operator[] - invalid index"));
    return m_pBuffer[uiIndex];
}


/*====================
  IBuffer::operator[]
  ====================*/
inline
char    IBuffer::operator[](uint uiIndex) const
{
    if (uiIndex >= m_uiEnd)
        EX_ERROR(_T("IBuffer::operator[] - invalid index"));
    return m_pBuffer[uiIndex];
}


/*====================
  IBuffer::Duplicate
  ====================*/
inline
char*   IBuffer::Duplicate(uint uiOffset, uint uiLength)
{
    if (uiLength <= 0)
        return nullptr;

    if (uiOffset + uiLength > m_uiEnd)
        uiLength = m_uiEnd - uiOffset;

    char *pRet(K2_NEW_ARRAY(ctx_Buffers, char, uiLength));
    MemManager.Copy(pRet, &m_pBuffer[uiOffset], uiLength);
    return pRet;
}


/*====================
  IBuffer::Get
  ====================*/
inline
const char* IBuffer::Get(uint uiOffset) const
{
    if (uiOffset >= m_uiEnd)
        return nullptr;

    return &m_pBuffer[uiOffset];
}


/*====================
  IBuffer::CompareBuffer
  ====================*/
inline
int     IBuffer::CompareBuffer(const IBuffer &cBuffer, uint uiSize) const
{
    return memcmp(&m_pBuffer[m_uiRead], &cBuffer.m_pBuffer[cBuffer.m_uiRead], uiSize);
}


/*====================
  IBuffer::CompareBuffer4
  ====================*/
inline
int     IBuffer::CompareBuffer4(const IBuffer &cBuffer) const
{
    char *pBuffer0(&m_pBuffer[m_uiRead]);
    char *pBuffer1(&cBuffer.m_pBuffer[cBuffer.m_uiRead]);

    if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
    if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
    if (*pBuffer0 != *pBuffer1) return 1; ++pBuffer0; ++pBuffer1;
    if (*pBuffer0 != *pBuffer1) return 1;

    return 0;
}
//=============================================================================


//=============================================================================
// CBufferDynamic
// Buffer that will allocate more memmory as needed
//=============================================================================
#ifdef __GNUC__
class __attribute__((visibility("default"))) CBufferDynamic : public IBuffer
#else
class CBufferDynamic : public IBuffer
#endif
{
private:
public:
    CBufferDynamic()            { Init(DEFAULT_CBUFFER_SIZE); }
    CBufferDynamic(uint uiSize) { Init(uiSize); }

    inline bool     Append(const void *pBuffer, uint uiSize);
    K2_API bool     WriteBytes(byte yValue, uint uiSize);
    K2_API bool     Write(const void *pBuffer, uint uiSize);
    K2_API bool     Read(void *pBuffer, uint uiSize) const;
    K2_API bool     Seek(uint uiPos);
    K2_API bool     Overwrite(const void *pBuffer, uint uiSize);
    K2_API char*    Lock(uint uiSize);
    inline bool     Advance(uint uiSize) const;

    // Non-virtual version of standard functions
    void            RewindBuffer() const    { m_uiRead = 0; }
    const char*     GetBuffer() const       { return m_pBuffer; }
    uint            GetBufferLength() const { return m_uiEnd; }

    inline int      CompareBuffer(const CBufferDynamic &cBuffer, uint uiSize) const;
    __forceinline uint      FindFirstDiff(const CBufferDynamic &cBuffer) const;
    __forceinline uint      FindFirstDiff2(const CBufferDynamic &cBuffer) const;
};


/*====================
  CBufferDynamic::Append
  ====================*/
inline
bool    CBufferDynamic::Append(const void *pBuffer, uint uiSize)
{
    bool    bRet(true);

    if (uiSize == 0)
        return true;

    uint uiCopyLen(uiSize);
    if (uiSize > (m_uiSize - m_uiEnd))
    {
        uint uiNewSize(MAX(m_uiSize, 1u));
        while (uiNewSize < (m_uiEnd + uiSize))
            uiNewSize <<= 1;
        if (!Resize(uiNewSize))
        {
            m_iFaults |= BUFFER_FAULT_OVERRUN;
            //Console.Err << _T("CBufferDynamic::Append - Overrun") << newl;
            bRet = false;
            uiCopyLen = m_uiSize - m_uiEnd;
        }
    }

    MemManager.Copy(&m_pBuffer[m_uiEnd], pBuffer, uiCopyLen);
    m_uiEnd += uiCopyLen;
    return bRet;
}


/*====================
  CBufferDynamic::Advance
  ====================*/
inline
bool    CBufferDynamic::Advance(uint uiSize) const
{
    bool bRet(true);

    if (m_uiRead + uiSize > m_uiEnd)
    {
        m_iFaults |= BUFFER_FAULT_OVERRUN;
        //Console.Err << _T("CBufferDynamic::Advance - Overrun") << newl;
        bRet = false;
    }
    else
    {
        m_uiRead += uiSize;
    }

    return bRet;
}


/*====================
  CBufferDynamic::CompareBuffer
  ====================*/
inline
int     CBufferDynamic::CompareBuffer(const CBufferDynamic &cBuffer, uint uiSize) const
{
    return memcmp(&m_pBuffer[m_uiRead], &cBuffer.m_pBuffer[cBuffer.m_uiRead], uiSize);
}


/*====================
  CBufferDynamic::FindFirstDiff

  Returns the length of the buffer if no difference is found,
  otherwise it returns the byte index of the first difference
  ====================*/
__forceinline
uint    CBufferDynamic::FindFirstDiff(const CBufferDynamic &cBuffer) const
{
    const char *pByteBuffer1(m_pBuffer);
    const char *pByteBuffer2(cBuffer.m_pBuffer);
    uint uiSize(CEIL_MULTIPLE<4>(m_uiEnd));

    if ((intptr_t(pByteBuffer1) & 0x3) != 0 || (intptr_t(pByteBuffer2) & 0x3) != 0 || uiSize > m_uiSize)
        return FindFirstDiff2(cBuffer); // Not aligned

    const uint *pBuffer1((const uint *)pByteBuffer1);
    const uint *pBuffer2((const uint *)pByteBuffer2);

    while (uiSize >= 4)
    {
        if (*pBuffer1 != *pBuffer2)
        {
            uint uiStartSize(CEIL_MULTIPLE<4>(m_uiEnd));

            const char *pSubBuffer1((const char *)pBuffer1);
            const char *pSubBuffer2((const char *)pBuffer2);
            uint uiSubSize(uiSize == 4 ? 4 - (uiStartSize - m_uiEnd) : 4);

            while (uiSubSize > 0)
            {
                if (*pSubBuffer1 != *pSubBuffer2)
                {
                    return uiStartSize - uiSize + uint(pSubBuffer1 - (const char *)pBuffer1);
                }
                else
                {
                    --uiSubSize;
                    ++pSubBuffer1;
                    ++pSubBuffer2;
                }
            }

            return m_uiEnd;
        }
        else
        {
            uiSize -= 4;
            ++pBuffer1;
            ++pBuffer2;
        }
    }

    return m_uiEnd;
}


/*====================
  CBufferDynamic::FindFirstDiff2

  Returns the length of the buffer if no difference is found,
  otherwise it returns the byte index of the first difference
  ====================*/
__forceinline
uint    CBufferDynamic::FindFirstDiff2(const CBufferDynamic &cBuffer) const
{
    const char *pBuffer1(m_pBuffer);
    const char *pBuffer2(cBuffer.m_pBuffer);
    uint uiSize(m_uiEnd);

    while (uiSize > 0)
    {
        if (*pBuffer1 != *pBuffer2)
        {
            return m_uiEnd - uiSize;
        }
        else
        {
            --uiSize;
            ++pBuffer1;
            ++pBuffer2;
        }
    }

    return m_uiEnd;
}
//=============================================================================


//=============================================================================
// CBufferStatic
// Buffer of a fixed size that only reallocates when requested
//=============================================================================
#ifdef __GNUC__
class __attribute__((visibility("default"))) CBufferStatic : public IBuffer
#else
class CBufferStatic : public IBuffer
#endif
{
private:
public:
    CBufferStatic()             { Init(DEFAULT_CBUFFER_SIZE); }
    CBufferStatic(uint uiSize)  { Init(uiSize); }

    K2_API bool     Append(const void *pBuffer, uint uiSize);
    K2_API bool     Write(const void *pBuffer, uint uiSize);
    K2_API bool     Read(void *pBuffer, uint uiSize) const;
    K2_API bool     Seek(uint uiPos);
    K2_API char*    Lock(uint uiSize);
    K2_API bool     Advance(uint uiSize) const;
};
//=============================================================================


//=============================================================================
// CBufferCircular
// Buffer of a fixed size that loops
//=============================================================================
class CBufferCircular : public IBuffer
{
private:
    uint    m_uiLength;

    inline void AlignBuffer();

public:
    ~CBufferCircular()                                  {}
    CBufferCircular()                                   { m_uiLength = 0; Init(DEFAULT_CBUFFER_SIZE); }
    CBufferCircular(const CBufferCircular &buffer);
    CBufferCircular(uint uiSize)                        { m_uiLength = 0; Init(uiSize); }

    void        Clear(char c)                           { IBuffer::Clear(c); m_uiLength = 0; }
    void        Clear()                                 { Clear(0); }

    uint        GetLength() const                       { return m_uiLength; }

    const char* Get(uint uiOffset)                      { AlignBuffer(); return IBuffer::Get(uiOffset); }
    const char* Get()                                   { return Get(0); }

    char*       Duplicate(uint uiOffset, uint uiLength) { AlignBuffer(); return IBuffer::Duplicate(uiOffset, uiLength); }
    char*       Duplicate(uint uiOffset)                { return Duplicate(uiOffset, m_uiLength); }
    char*       Duplicate()                             { return Duplicate(0, m_uiLength); }

    inline char operator[](uint uiIndex) const;

    bool        Append(const void *pBuffer, uint uiSize);
    bool        Write(const void *pBuffer, uint uiSize);
    bool        Read(void *pBuffer, uint uiSize) const;
    bool        Seek(uint uiPos);
    uint        FindNext(char c) const;
    uint        FindNext(wchar_t c) const;
    void        Rewind();
};

/*====================
  CBufferCircular::AlignBuffer
  ====================*/
inline
void    CBufferCircular::AlignBuffer()
{
    if (m_uiLength < m_uiSize)
        return;

    char *pTmp(K2_NEW_ARRAY(ctx_Buffers, char, m_uiEnd));
    MemManager.Copy(pTmp, m_pBuffer, m_uiEnd);

    MemManager.Move(m_pBuffer, &m_pBuffer[m_uiEnd], m_uiSize - m_uiEnd);
    MemManager.Copy(&m_pBuffer[m_uiSize - m_uiEnd], pTmp, m_uiEnd);
    m_uiEnd = m_uiSize;
}


/*====================
  CBufferCircular::operator[]
  ====================*/
inline
char    CBufferCircular::operator[](uint uiIndex) const
{
    if (uiIndex >= m_uiLength)
        EX_ERROR(_T("CBufferCircular::operator[] - Invalid index"));

    if (m_uiLength == m_uiSize)
        uiIndex = abs(int((m_uiEnd + uiIndex) % m_uiSize));
    return m_pBuffer[uiIndex];
}


//=============================================================================
// CBufferFixed
// Buffer of a fixed size that cannot reallocate
//=============================================================================
template <uint BUFFER_SIZE>
class CBufferFixed : public IBuffer
{
private:
    char m_pFixedBuffer[BUFFER_SIZE];

public:
    CBufferFixed()              { Init(0); m_pBuffer = m_pFixedBuffer; m_uiSize = BUFFER_SIZE; }
    CBufferFixed(const CBufferFixed &buffer);
    ~CBufferFixed()             { m_pBuffer = nullptr; }

    bool    Append(const void *pBuffer, uint uiSize);
    bool    Write(const void *pBuffer, uint uiSize);
    bool    Read(void *pBuffer, uint uiSize) const;
    bool    Seek(uint uiPos);
    bool    AppendNulls(uint uiNewMaxIndex);
    char*   Lock(uint uiSize);
};
//=============================================================================


/*====================
  CBufferFixed::CBufferFixed
  ====================*/
template <uint BUFFER_SIZE>
inline
CBufferFixed<BUFFER_SIZE>::CBufferFixed(const CBufferFixed<BUFFER_SIZE> &buffer)
{
    Init(0); m_pBuffer = m_pFixedBuffer; m_uiSize = BUFFER_SIZE;
    MemManager.Copy(m_pBuffer, buffer.m_pBuffer, MIN(buffer.m_uiSize, m_uiSize));
    m_uiEnd = buffer.m_uiEnd;
}


/*====================
  CBufferFixed::Write

  Copy the input to the start of the buffer, overwriting
  ====================*/
template <uint BUFFER_SIZE>
inline
bool    CBufferFixed<BUFFER_SIZE>::Write(const void *pBuffer, uint uiSize)
{
    bool ret(true);

    if (uiSize == 0)
    {
        m_uiEnd = 0;
        m_uiRead = 0;
        return true;
    }

    if (pBuffer == nullptr)
        return false;

    uint uiCopyLen(uiSize);
    if (uiSize > m_uiSize)
    {
        uiCopyLen = m_uiSize;
        m_iFaults |= BUFFER_FAULT_OVERRUN;
        ret = false;
    }

    MemManager.Copy(m_pBuffer, pBuffer, uiCopyLen);
    m_uiEnd = uiCopyLen;
    m_uiRead = 0;
    return ret;
}


/*====================
  CBufferFixed::Append

  Copy the input to the end of the buffer 
  ====================*/
template <uint BUFFER_SIZE>
inline
bool    CBufferFixed<BUFFER_SIZE>::Append(const void *pBuffer, uint uiSize)
{
    bool ret(true);

    uint uiCopyLen(uiSize);
    if (uiSize > (m_uiSize - m_uiEnd))
    {
        uiCopyLen = m_uiSize - m_uiEnd;
        m_iFaults |= BUFFER_FAULT_OVERRUN;
        ret = false;
    }

    MemManager.Copy(&m_pBuffer[m_uiEnd], pBuffer, uiCopyLen);
    m_uiEnd += uiCopyLen;
    return ret;
}


/*====================
  CBufferFixed::Seek
  ====================*/
template <uint BUFFER_SIZE>
inline
bool CBufferFixed<BUFFER_SIZE>::AppendNulls(uint uiNewMaxIndex)
{
    if (uiNewMaxIndex < m_uiSize)
    {
        // Allocate buffer space for [0 through NewMaxIndex]
        uint uiCopyLen(uiNewMaxIndex - m_uiEnd + 1); 

        // Zero the buffer section
        MemManager.Set(&m_pBuffer[m_uiEnd], 0, uiCopyLen);
        m_uiEnd += uiCopyLen;
        return true;
    }
    else
    {
        return false;
    }
}


/*====================
  CBufferFixed::Seek
  ====================*/
template <uint BUFFER_SIZE>
inline
bool    CBufferFixed<BUFFER_SIZE>::Seek(uint uiPos)
{
    if (uiPos <= m_uiEnd)
    {
        m_uiRead = uiPos;
        return true;
    }

    return false;
}


/*====================
  CBufferFixed::Read
  ====================*/
template <uint BUFFER_SIZE>
inline
bool    CBufferFixed<BUFFER_SIZE>::Read(void *pBuffer, uint uiSize) const
{
    if (m_uiRead + uiSize > m_uiSize)
    {
        m_iFaults |= BUFFER_FAULT_UNDERRUN;
        return false;
    }

    MemManager.Copy(pBuffer, &m_pBuffer[m_uiRead], uiSize);
    m_uiRead += uiSize;
    return true;
}


/*====================
  CBufferFixed::Lock

  return a writable region of the buffer from the current write position
  ====================*/
template <uint BUFFER_SIZE>
inline
char*   CBufferFixed<BUFFER_SIZE>::Lock(uint uiSize)
{
    if (uiSize > (m_uiSize - m_uiEnd))
    {
        uiSize = m_uiSize - m_uiEnd;
        m_iFaults |= BUFFER_FAULT_OVERRUN;
    }

    if (uiSize == 0)
        return nullptr;

    char *pBuffer(&m_pBuffer[m_uiEnd]);
    m_uiEnd += uiSize;

    return pBuffer;
}


//=============================================================================
// CBufferBit
// Per bit reading/writing built on top of a dynamic buffer
//=============================================================================
class CBufferBit : public CBufferDynamic
{
private:
    uint    m_uiWrite;
    uint    m_uiWriteBit;
    
    mutable uint    m_uiReadBit;

public:
    CBufferBit() :
    CBufferDynamic(DEFAULT_CBUFFER_SIZE),
    m_uiWrite(0),
    m_uiWriteBit(0),
    m_uiReadBit(0)
    {
    }

    CBufferBit(uint uiSize) :
    CBufferDynamic(uiSize),
    m_uiWrite(0),
    m_uiWriteBit(0),
    m_uiReadBit(0)
    {
    }

    virtual void    Clear()
    {
        CBufferDynamic::Clear();

        m_uiWrite = 0;
        m_uiWriteBit = 0;
        m_uiReadBit = 0;
    }

    virtual void    Rewind() const
    {
        CBufferDynamic::Rewind();

        m_uiReadBit = 0;
    }

    K2_API void     WriteBits(uint uiValue, uint uiNumBits);
    K2_API uint     ReadBits(uint uiNumBits) const;

    K2_API void     WriteBit(uint uiValue);
    K2_API uint     ReadBit() const;

    uint    GetUnreadBits() const
    {
        if (m_uiRead >= m_uiEnd)
            return 0;
        else
            return (m_uiEnd - m_uiRead - 1) * 8 + 8 - m_uiReadBit;
    }
};
//=============================================================================

#endif //__C_BUFFER_H__
