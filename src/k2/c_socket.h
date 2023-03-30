// (C)2005 S2 Games
// c_socket.h
//
//=============================================================================
#ifndef __C_SOCKET_H__
#define __C_SOCKET_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_protocol.h"

#include "c_packet.h"
#include "c_reliablepacket.h"
#include "c_netdriver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int MAX_ADDRESS_LENGTH(64);
const int HEADER_FLAG_LOC(4);
const int MAX_PACKETS_RESENT_PER_FRAME(1);
const int PACKET_TIMEOUT(500);

typedef list<CReliablePacket>   ReliablePktList;
//=============================================================================

//=============================================================================
// CSocket
//=============================================================================
class CSocket
{
private:
    tstring         m_sName;
    ushort          m_unConnectionID;
    bool            m_bInitialized;
    ESocketType     m_eType;
    dword           m_dwSocket;                 // used as a socket identifier for the driver layer

    word            m_wLocalPort;

    uint            m_uiReliableRecvLastSeq;
    wstring         m_sRecvAddrName;            // address related to this packaet
    word            m_wRecvPort;

    ReliablePktList m_UnackPackets;
    ReliablePktList m_lOutOfSequencePackets;

    uint            m_uiReliableSendLastSeq;
    tstring         m_sSendAddrName;            // address to send packet to
    word            m_wSendPort;

    void*           m_pSendAddr;                // Holds a structure allocated and maintained by CNetDriver

    bool            m_bIsLocalConnection;
    bool            m_bIsLANConnection;
    bool            m_bCloseOnDelete;

    uint            m_uiBytesSent;
    uint            m_uiPacketsSent;
    uint            m_uiBytesDropped;
    uint            m_uiPacketsDropped;

    uint            m_uiUnreliablePacketsSent;
    uint            m_uiReliablePacketsSent;
    uint            m_uiAcksSent;

    CBufferDynamic  m_cBuffer;

    bool            RequiresConnection();

public:
    K2_API ~CSocket();
    K2_API CSocket(const tstring &sName);

    tstring     GetName() const                             { return m_sName; }
    void        SetConnectionID(ushort unConnectionID)      { m_unConnectionID = unConnectionID; }

    wstring     GetRecvAddrName() const                     { return m_sRecvAddrName; }
    word        GetRecvPort() const                         { return m_wRecvPort; }

    tstring     GetSendAddrName() const                     { return m_sSendAddrName; }
    word        GetSendPort() const                         { return m_wSendPort; }

    tstring     GetLocalAddr() const;
    word        GetLocalPort() const                        { return m_wLocalPort; }

    void        ResetSeq()                                  { m_uiReliableSendLastSeq = m_uiReliableRecvLastSeq = 0; }

    bool        Init(ESocketType eType, word wPort = 0, bool bBlocking = false, uint uiSendBuffer = -1, uint uiRecvBuffer = -1);
    bool        Init(const CSocket &sock);

    bool        ProcessReliablePacket(CPacket &pkt);
    bool        ProcessPacketAck(CPacket &pkt);
    bool        PreProcessPacket(CPacket &pkt);
    int         CheckOutOfSequencePacket(CPacket &pkt);
    int         ReceivePacket(CPacket &pkt);
    void        WriteClientID(CPacket &pkt);
    bool        SendPacket(CPacket &pkt);

    bool        SendReliablePacket(CPacket &pkt, bool bQueue);

    bool        Close();

    bool        OpenListenPort(int iMaxConnectionsWaiting);
    CSocket*    AcceptConnection(const tstring &sSocketName);
    bool        SetSendAddr(const tstring &addr, word wPort = 0);
    bool        SetSendPort(word wPort)                     { return SetSendAddr(m_sSendAddrName, wPort); }

    void        SetSocketOptions(int iOption, const string &sOptVal)    { if (m_bInitialized) NetDriver.SetSocketOptions(m_dwSocket, m_eType, iOption, sOptVal); }

    void        AllowBroadcast(bool bValue);
    tstring     GetBroadcastAddress();

    void        CheckPacketTimeouts();
    void        ClearReliablePackets();

    bool        IsInitialized() const                       { return m_bInitialized; }
    bool        IsLocalConnection() const                   { return m_bIsLocalConnection; }
    bool        IsLANConnection() const                     { return m_bIsLANConnection; }

    bool        IsConnected(int iSecondsToWait = 0);
    bool        DataWaiting(uint uiWaitTime = 0);
    bool        HasError(uint uiWaitTime = 0);

    void        SetSendBuffer(uint uiSendBuffer)            { if (m_bInitialized) NetDriver.SetSendBuffer(m_dwSocket, uiSendBuffer); }
    void        SetRecvBuffer(uint uiRecvBuffer)            { if (m_bInitialized) NetDriver.SetRecvBuffer(m_dwSocket, uiRecvBuffer); }

    uint        GetBytesSent() const                        { return m_uiBytesSent; }
    uint        GetPacketsSent() const                      { return m_uiPacketsSent; }
    uint        GetBytesDropped() const                     { return m_uiBytesDropped; }
    uint        GetPacketsDropped() const                   { return m_uiPacketsDropped; }

    void        ClearProfileStats()
    {
        m_uiBytesSent = 0;
        m_uiPacketsSent = 0;
        m_uiBytesDropped = 0;
        m_uiPacketsDropped = 0;

        m_uiUnreliablePacketsSent = 0;
        m_uiReliablePacketsSent = 0;
        m_uiAcksSent = 0;
    }

    uint        GetOldestReliable() const;
    void        SetBlockIncoming(bool b);
    void        SetBlockOutgoing(bool b);
};
//=============================================================================

#endif //__C_SOCKET_H__
