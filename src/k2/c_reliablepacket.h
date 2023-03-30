// (C)2005 S2 Games
// c_reliablepacket.h
//
//=============================================================================
#ifndef __C_RELIABLEPACKET_H__
#define __C_RELIABLEPACKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
//=============================================================================

//=============================================================================
// CReliablePacket
//=============================================================================
class CReliablePacket
{
private:
    uint    m_uiTimeStamp;
    uint    m_uiOriginalTimeStamp;
    CPacket m_pktSaved;

    CReliablePacket();

public:
    ~CReliablePacket();
    CReliablePacket(const CPacket &pkt);
    CReliablePacket(const CReliablePacket &pkt);

    CReliablePacket&    operator=(const CReliablePacket &pkt);

    void            SetTimeStamp(uint uiTime)       { m_uiTimeStamp = uiTime; }

    uint            GetTimeStamp() const            { return m_uiTimeStamp; }
    uint            GetOriginalTimeStamp() const    { return m_uiOriginalTimeStamp; }
    uint            GetSequenceID() const           { return m_pktSaved.GetHeader()->m_uiSequence; }
    const CPacket&  GetPacket() const               { return m_pktSaved; }
};
//=============================================================================

#endif //__C_RELIABLEPACKET_H__
