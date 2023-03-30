// (C)2005 S2 Games
// c_reliablepacket.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_reliablepacket.h"
//=============================================================================

/*====================
  CReliablePacket::~CReliablePacket
  ====================*/
CReliablePacket::~CReliablePacket()
{
}


/*====================
  CReliablePacket::CReliablePacket
  ====================*/
CReliablePacket::CReliablePacket(const CPacket &pkt) :
m_uiTimeStamp(Host.GetSystemTime()),
m_uiOriginalTimeStamp(Host.GetSystemTime()),
m_pktSaved(pkt)
{
}

CReliablePacket::CReliablePacket(const CReliablePacket &pkt) :
m_uiTimeStamp(pkt.m_uiTimeStamp),
m_uiOriginalTimeStamp(pkt.m_uiOriginalTimeStamp),
m_pktSaved(pkt.m_pktSaved)
{
}


/*====================
  CReliablePacket::operator=
  ====================*/
CReliablePacket&    CReliablePacket::operator=(const CReliablePacket &pkt)
{
    m_uiTimeStamp = pkt.m_uiTimeStamp;
    m_pktSaved = pkt.m_pktSaved;
    return *this;
}
