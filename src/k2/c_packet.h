// (C)2005 S2 Games
// c_packet.h
//
//=============================================================================
#ifndef __C_PACKET_H__
#define __C_PACKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_protocol.h"
#include "c_buffer.h"
//=============================================================================

//=============================================================================
// Declaratations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#pragma pack(push, 1)
struct SPacketHeader
{
    ushort  m_unConnectionID;
    byte    m_yFlags;
    uint    m_uiSequence;

    SPacketHeader() :
    m_unConnectionID(0),
    m_yFlags(0),
    m_uiSequence(UINT_MAX)
    {}

    void    Set(ushort unConnectionID, byte yFlags, uint uiSequence)
    {
        m_unConnectionID = unConnectionID;
        m_yFlags = yFlags;
        m_uiSequence = uiSequence;
    }
};
#pragma pack(pop)

const size_t HEADER_SIZE(sizeof(SPacketHeader));
const size_t MAX_PACKET_SIZE(8192 - HEADER_SIZE);
//=============================================================================

//=============================================================================
// CPacket
//=============================================================================
class CPacket
{
private:
    SPacketHeader                   m_PacketHeader;
    CBufferFixed<MAX_PACKET_SIZE>   m_Buffer;
    bool                            m_bOverflowed;

public:
    K2_API  ~CPacket()  {}
    K2_API  CPacket();
    CPacket(const CPacket &pkt);
    CPacket(const char *pBuffer, uint uiLength);

    CPacket &operator=(const CPacket &c);

    K2_API  void    Clear();

    uint    GetSequence() const                 { return m_PacketHeader.m_uiSequence; }
    byte    GetFlags() const                    { return m_PacketHeader.m_yFlags; }
    ushort  GetConnectionID() const             { return m_PacketHeader.m_unConnectionID; }

    bool    HasFlags(byte yFlags) const         { return (m_PacketHeader.m_yFlags & yFlags) == yFlags; }

    bool    IsEmpty() const                     { return m_Buffer.GetLength() == 0; }
    uint    GetLength() const                   { return m_Buffer.GetLength(); }
    uint    GetUnreadLength() const             { return m_Buffer.GetUnreadLength(); }
    size_t  GetRemainingSpace() const           { return m_Buffer.GetCapacity() - m_Buffer.GetLength(); }
    bool    DoneReading() const                 { return (m_Buffer.GetUnreadLength() == 0 || GetReadPos() > GetLength()); }
    void    Seek(uint uiOffset)                 { m_Buffer.Seek(uiOffset); }
    void    Advance(uint uiBytes)               { m_Buffer.Seek(m_Buffer.GetReadPos() + uiBytes); }
    bool    HasFaults() const                   { return (m_Buffer.GetFaults() != 0); }
    uint    GetReadPos() const                  { return m_Buffer.GetReadPos(); }

    const char* GetBuffer() const               { return m_Buffer.Get(m_Buffer.GetReadPos()); }
    const char* GetBuffer(uint uiOffset) const  { return m_Buffer.Get(uiOffset); }

    byte    operator[](int i) const             { return m_Buffer[i]; }

    K2_API  bool    Write(const char *data, uint uiSize);
    K2_API  bool    WriteInt64(LONGLONG ll);
    K2_API  bool    WriteInt(int i);
    K2_API  bool    WriteShort(short n);
    K2_API  bool    WriteByte(byte y);
    K2_API  bool    WriteString(const char *sz);
    K2_API  bool    WriteString(const wchar_t *sz);
    K2_API  bool    WriteString(const tstring &s);
    K2_API  bool    WriteString(const tstring &s, byte bTerminatingChar);
    K2_API  bool    WriteUnterminatedString(const char *sz);
    K2_API  bool    WriteUnterminatedString(const wchar_t *sz);
    K2_API  bool    WriteUnterminatedString(const tstring &s);
    K2_API  bool    WriteArray(char *array, ushort unSize);
    K2_API  bool    WriteFloat(float f);
    K2_API  bool    WriteV3f(CVec3f in);

#define PKT_WRITE(t, x) if (!Write##t(x)) { K2System.DebugBreak(); EX_ERROR(_T("CPacket overflowed")); } return *this;
    CPacket&    operator<<(LONGLONG ll)             { PKT_WRITE(Int64, ll) }
    CPacket&    operator<<(int i)                   { PKT_WRITE(Int, i) }
    CPacket&    operator<<(uint ui)                 { PKT_WRITE(Int, ui) }
    CPacket&    operator<<(short n)                 { PKT_WRITE(Short, n) }
    CPacket&    operator<<(ushort un)               { PKT_WRITE(Short, un) }
    CPacket&    operator<<(byte y)                  { PKT_WRITE(Byte, y) }
    CPacket&    operator<<(const char *sz)          { PKT_WRITE(String, sz) }
    CPacket&    operator<<(const tstring &s)        { PKT_WRITE(String, s) }
    CPacket&    operator<<(float f)                 { PKT_WRITE(Float, f) }
    CPacket&    operator<<(CVec3f f)                { PKT_WRITE(V3f, f) }
#undef PK_WRITE
    K2_API  CPacket&    operator<<(const IBuffer &buffer);

    K2_API size_t   Read(char *out, uint uiSize);
    K2_API int      ReadInt(int iFailed = 0);
    K2_API LONGLONG ReadInt64(LONGLONG llFailed = 0);
    K2_API short    ReadShort(short nFailed = 0);
    K2_API byte     ReadByte(byte yFailed = 0);
    K2_API void     ReadString(char *buf, uint uiSize);
    K2_API string   ReadString(const string &sFailed = SNULL);
    K2_API wstring  ReadWString(const wstring &sFailed = WSNULL);
    K2_API tstring  ReadTString(const tstring &sFailed = TSNULL);
    K2_API size_t   ReadArray(char *str, uint uiMaxSize);
    K2_API float    ReadFloat(float fFailed = 0.0f);
    K2_API CVec3f   ReadV3f(const CVec3f &vFailed = V_ZERO);
    K2_API CVec2f   ReadV2f(const CVec2f &vFailed = V2_ZERO);

    K2_API CPacket& operator>>(int &i)      { i = ReadInt(); return *this; }
    K2_API CPacket& operator>>(uint &ui)    { ui = ReadInt(); return *this; }
    K2_API CPacket& operator>>(short &n)    { n = ReadShort(); return *this; }
    K2_API CPacket& operator>>(ushort &un)  { un = ReadShort(); return *this; }
    K2_API CPacket& operator>>(byte &y)     { y = ReadByte(); return *this; }
    K2_API CPacket& operator>>(string &s)   { s = ReadString(); return *this; }
    K2_API CPacket& operator>>(wstring &s)  { s = ReadWString(); return *this; }
    K2_API CPacket& operator>>(float &f)    { f = ReadFloat(); return *this; }
    K2_API CPacket& operator>>(CVec3f &v3)  { v3 = ReadV3f(); return *this; }
    K2_API CPacket& operator>>(CVec2f &v2)  { v2 = ReadV2f(); return *this; }

    void                    SetHeader(ushort unConnectionID, byte yFlags, uint uiSequence = 0)  { m_PacketHeader.Set(unConnectionID, yFlags, uiSequence); }
    const SPacketHeader*    GetHeader() const                                                   { return &m_PacketHeader; }
};
//=============================================================================

#endif  //__C_PACKET_H__
