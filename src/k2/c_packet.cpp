// (C)2005 S2 Games
// c_packet.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_packet.h"
#include "c_buffer.h"
//=============================================================================

/*====================
  CPacket::CPacket
  ====================*/
CPacket::CPacket() :
m_bOverflowed(false)
{
}

CPacket::CPacket(const CPacket &pkt) :
m_PacketHeader(pkt.m_PacketHeader),
m_Buffer(pkt.m_Buffer),
m_bOverflowed(pkt.m_bOverflowed)
{
}

CPacket::CPacket(const char *pBuffer, uint uiLength) :
m_bOverflowed(false)
{
    Write(pBuffer, uiLength);
}


/*====================
  CPacket::Clear
  ====================*/
void    CPacket::Clear()
{
    m_Buffer.Clear();
    m_bOverflowed = false;
}


/*====================
  CPacket::WriteInt64
  ====================*/
bool    CPacket::WriteInt64(LONGLONG ll)
{
    assert(sizeof(LONGLONG) == 8);

    try
    {
        m_Buffer << LittleInt64(ll);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing longlong ") + XtoA(ll));
        return true;    
    }   
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteInt64() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
    

    return true;
}


/*====================
  CPacket::WriteInt
  ====================*/
bool    CPacket::WriteInt(int i)
{
    assert(sizeof(int) == 4);

    try
    {
        m_Buffer << i;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing int ") + XtoA(i));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteInt() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}


/*====================
  CPacket::WriteFloat
  ====================*/
bool    CPacket::WriteFloat(float f)
{
    assert(sizeof(float) == 4);

    try
    {
        m_Buffer << f;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing float ") + XtoA(f));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteFloat() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteV3f(CVec3f in)
{
    if( WriteFloat( in.x ) )
        if( WriteFloat( in.y ) )
            if( WriteFloat( in.z ) )
                return true;
    return false;
}

/*====================
  CPacket::WriteShort
  ====================*/
bool    CPacket::WriteShort(short n)
{
    assert(sizeof(short) == 2);

    try
    {
        m_Buffer << n;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing short ") + XtoA(n));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteShort() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}


/*====================
  CPacket::WriteByte
  ====================*/
bool    CPacket::WriteByte(byte y)
{
    assert(sizeof(byte) == 1);

    try
    {
        m_Buffer << y;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing byte ") + XtoA(int(y)));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteByte() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}


/*====================
  CPacket::WriteString
  ====================*/
bool    CPacket::WriteString(const char *sz)
{
    try
    {
        m_Buffer << sz << byte(0);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + XtoA(sz) + _T("\", length: ") + XtoA(INT_SIZE(strlen(sz))));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteString(const wchar_t *sz)
{
    try
    {
        m_Buffer << WStringToUTF8(sz) << byte(0);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + XtoA(sz) + _T("\", length: ") + XtoA(INT_SIZE(wcslen(sz))));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteString(const tstring &s)
{
    try
    {
        m_Buffer << TStringToUTF8(s) << byte(0);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + s + _T("\", length: ") + XtoA(INT_SIZE(s.length())));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteString(const tstring &s, byte bTerminatingChar)
{
    try
    {
        m_Buffer << TStringToUTF8(s) << byte(bTerminatingChar);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + s + _T("\", length: ") + XtoA(INT_SIZE(s.length())));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

/*====================
  CPacket::WriteUnterminatedString
  ====================*/
bool    CPacket::WriteUnterminatedString(const tstring &s)
{
    try
    {
        m_Buffer << TStringToUTF8(s);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + s + _T("\", length: ") + XtoA(INT_SIZE(s.length())));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteUnterminatedString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteUnterminatedString(const char *sz)
{
    try
    {
        m_Buffer << sz;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + XtoA(sz) + _T("\", length: ") + XtoA(INT_SIZE(strlen(sz))));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteUnterminatedString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

bool    CPacket::WriteUnterminatedString(const wchar_t *sz)
{
    try
    {
        m_Buffer << WStringToUTF8(sz);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing string: \"") + XtoA(sz) + _T("\", length: ") + XtoA(INT_SIZE(wcslen(sz))));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::WriteUnterminatedString() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}

/*====================
  CPacket::Write
  ====================*/
bool    CPacket::Write(const char *data, uint uiSize)
{
    try
    {
        m_Buffer.Append(data, uiSize);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_OVERRUN)
            EX_ERROR(_T("Exceeded MAX_PACKET_SIZE while writing data: \"") + XtoA(data) + _T("\", length: ") + XtoA(uiSize));
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::Write() - "), NO_THROW);
        m_bOverflowed = true;
        return false;
    }
}


/*====================
  CPacket::WriteArray
  ====================*/
bool    CPacket::WriteArray(char *array, ushort unSize)
{
    // Write the length at the beginning
    WriteShort(unSize);
    Write(array, unSize);

    return (!m_bOverflowed);
}


/*====================
  CPacket::operator<<
  ====================*/
CPacket&    CPacket::operator<<(const IBuffer &buffer)
{
    if (!Write(buffer.Get(), buffer.GetLength()))
        EX_ERROR(_T("CPacket::operator<<() - Packet overflowed"));
    return *this;
}


/*====================
  CPacket::ReadInt
  ====================*/
int     CPacket::ReadInt(int iFailed)
{
    try
    {
        int i;
        m_Buffer >> i;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
            EX_ERROR(_T("Attempted to read past end of packet"));
        return i;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::ReadInt() - "), NO_THROW);
        return iFailed;
    }
}


/*====================
  CPacket::ReadInt64
  ====================*/
LONGLONG    CPacket::ReadInt64(LONGLONG llFailed)
{
    LONGLONG ll;
    m_Buffer >> ll;
    if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
        return llFailed;

    return LittleInt64(ll);
}


/*====================
  CPacket::ReadShort
  ====================*/
short   CPacket::ReadShort(short nFailed)
{
    try
    {
        short n;
        m_Buffer >> n;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
            EX_ERROR(_T("Attempted to read past end of packet"));
        return n;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::ReadShort() - "), NO_THROW);
        return nFailed;
    }
}


/*====================
  CPacket::ReadByte
  ====================*/
byte    CPacket::ReadByte(byte yFailed)
{
    try
    {
        byte y;
        m_Buffer >> y;
        if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
            EX_ERROR(_T("Attempted to read past end of packet"));
        return y;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::ReadByte() - "), NO_THROW);
        return yFailed;
    }
}


/*====================
  CPacket::ReadString
  ====================*/
void    CPacket::ReadString(char *sz, uint uiSize)
{
    try
    {
        uint uiEnd(m_Buffer.FindNext('\x00'));
        if (uiEnd == INVALID_INDEX)
            EX_ERROR(_T("Attempted to read unterminated string"));

        uint uiRead(uiEnd - m_Buffer.GetReadPos() + 1);
        if (uiRead > uiSize)
            uiRead = uiSize;
        m_Buffer.Read(sz, uiRead);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
            EX_ERROR(_T("Attempted to read past end of packet"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::ReadString() - "), NO_THROW);
    }
}


/*====================
  CPacket::ReadString
  ====================*/
string  CPacket::ReadString(const string &sFailed)
{
    uint uiEnd(m_Buffer.FindNext('\x00'));
    if (uiEnd == INVALID_INDEX)
    {
        Console.Net << L"Attempted to read unterminated string" << newl;
        return sFailed;
    }

    uint uiPos(m_Buffer.GetReadPos());
    m_Buffer.Seek(uiEnd + 1);
    return UTF8ToString(m_Buffer.Get(uiPos));
}


/*====================
  CPacket::ReadWString
  ====================*/
wstring CPacket::ReadWString(const wstring &sFailed)
{
    uint uiEnd(m_Buffer.FindNext('\x00'));
    if (uiEnd == INVALID_INDEX)
    {
        Console.Net << L"Attempted to read unterminated string" << newl;
        return sFailed;
    }

    uint uiPos(m_Buffer.GetReadPos());
    m_Buffer.Seek(uiEnd + 1);
    return UTF8ToWString(m_Buffer.Get(uiPos));
}


/*====================
  CPacket::ReadTString
  ====================*/
tstring CPacket::ReadTString(const tstring &sFailed)
{
    uint uiEnd(m_Buffer.FindNext('\x00'));
    if (uiEnd == INVALID_INDEX)
    {
        Console.Net << L"Attempted to read unterminated string" << newl;
        return sFailed;
    }

    uint uiPos(m_Buffer.GetReadPos());
    m_Buffer.Seek(uiEnd + 1);
    return UTF8ToTString(m_Buffer.Get(uiPos));
}


/*====================
  CPacket::Read
  ====================*/
size_t  CPacket::Read(char *out, uint uiSize)
{
    try
    {
        m_Buffer.Read(out, uiSize);
        if (m_Buffer.GetFaults() & BUFFER_FAULT_UNDERRUN)
            EX_ERROR(_T("Attempted to read past end of packet"));
        return uiSize;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPacket::Read() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CPacket::ReadArray
  ====================*/
size_t  CPacket::ReadArray(char *str, uint uiMaxSize)
{
    uint uiSize(ReadShort());

    return Read(str, MIN(uiMaxSize, uiSize));
}


/*====================
  CPacket::ReadFloat
  ====================*/
float   CPacket::ReadFloat(float fFailed)
{
    int iValue(ReadInt(static_cast<int>(fFailed)));
    return FLOAT_CAST(iValue);
}


/*====================
  CPacket::ReadV3f
  ====================*/
CVec3f  CPacket::ReadV3f(const CVec3f &v3Failed)
{
    CVec3f out;
    int lvalue = ReadInt(static_cast<int>(v3Failed.x));
    out.x = FLOAT_CAST(lvalue);
    lvalue = ReadInt(static_cast<int>(v3Failed.y));
    out.y = FLOAT_CAST(lvalue);
    lvalue = ReadInt(static_cast<int>(v3Failed.z));
    out.z = FLOAT_CAST(lvalue);

    return out;
}


/*====================
  CPacket::ReadV2f
  ====================*/
CVec2f  CPacket::ReadV2f(const CVec2f &v2Failed)
{
    CVec2f out;
    int lvalue = ReadInt(static_cast<int>(v2Failed.x));
    out.x = FLOAT_CAST(lvalue);
    lvalue = ReadInt(static_cast<int>(v2Failed.y));
    out.y = FLOAT_CAST(lvalue);

    return out;
}


/*====================
  CPacket::operator=
  ====================*/
CPacket  &CPacket::operator=(const CPacket &from)
{
    m_PacketHeader = from.m_PacketHeader;
    m_Buffer = from.m_Buffer;
    m_bOverflowed = from.m_bOverflowed;
    return *this;
}
