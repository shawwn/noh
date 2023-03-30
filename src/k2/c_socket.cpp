// (C)2005 S2 Games
// c_socket.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_socket.h"
#include "c_hostclient.h"
#include "c_netdriver.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_BOOL	(net_reliablePackets,	true);
CVAR_FLOATR	(net_forcedPacketDrop,	0.0f,	0,	0.0f,	1.0f);
CVAR_BOOL	(net_showStats,			false);
CVAR_INT	(net_problemIconTime,	2000);
CVAR_INT	(net_lagThreshold,		1000);
CVAR_BOOL	(net_debugReliable,		false);
//=============================================================================

/*====================
  CSocket::~CSocket
  ====================*/
CSocket::~CSocket()
{
	if (m_bCloseOnDelete)
		Close();

	NetDriver.FreeSendAddr(m_pSendAddr);
}


/*====================
  CSocket::CSocket
  ====================*/
CSocket::CSocket(const tstring &sName) :
m_sName(sName),
m_unConnectionID(0),
m_bInitialized(false),
m_eType(K2_SOCKET_INVALID),
m_dwSocket(SOCKET_INVALID),

m_wLocalPort(0),

m_uiReliableRecvLastSeq(0),
m_wRecvPort(0),

m_uiReliableSendLastSeq(0),
m_wSendPort(0),

m_pSendAddr(NULL),

m_bIsLocalConnection(false),
m_bIsLANConnection(false),
m_bCloseOnDelete(true),

m_uiBytesSent(0),
m_uiPacketsSent(0),
m_uiBytesDropped(0),
m_uiPacketsDropped(0)
{
}


/*====================
  CSocket::Init
  ====================*/
bool	CSocket::Init(ESocketType eType, word wPort, bool bBlocking, uint uiSendBuffer, uint uiRecvBuffer)
{
	Console.Net << _T("Created CSocket ") << QuoteStr(m_sName) << newl;

	m_dwSocket = SOCKET_INVALID;
	m_eType = eType;
	m_uiReliableRecvLastSeq = 0;
	m_uiReliableSendLastSeq = 0;
	m_UnackPackets.clear();
	m_lOutOfSequencePackets.clear();
	m_unConnectionID = 0;
	m_cBuffer.Clear();

	Console.Net << _T("[") << m_sName << _T("] Initializing...") << newl;
	Console.Net << _T("[") << m_sName << _T("] Attempting to open port: ") << wPort << newl;

	if (!NetDriver.OpenPort(m_dwSocket, wPort, eType, bBlocking, uiSendBuffer, uiRecvBuffer))
		return false;

	m_bInitialized = true;

	Console.Net << _T("[") << m_sName << _T("] Successfully opened port: ") << wPort << newl;
	m_wLocalPort = wPort;

	ClearProfileStats();
	return true;
}

bool	CSocket::Init(const CSocket &sock)
{
	Console.Net << _T("[") << m_sName << _T("] Initializing as an instance of [") << sock.m_sName << _T("]...") << newl;

	m_dwSocket = sock.m_dwSocket;
	m_bInitialized = sock.m_bInitialized;
	m_eType = sock.m_eType;
	m_uiReliableRecvLastSeq = 0;
	m_uiReliableSendLastSeq = 0;
	m_UnackPackets.clear();
	m_lOutOfSequencePackets.clear();
	m_unConnectionID = 0;
	m_cBuffer.Clear();

	m_wLocalPort = sock.GetLocalPort();

	m_bCloseOnDelete = false;

	ClearProfileStats();
	return true;
}


/*====================
  CSocket::Close
  ====================*/
bool	CSocket::Close()
{
	if (!m_bInitialized)
		return true;

	Console.Net << _T("[") << m_sName << _T("] Closing...") << newl;
	
	m_bInitialized = false;
	m_UnackPackets.clear();
	m_lOutOfSequencePackets.clear();
	m_cBuffer.Clear();
	
	return NetDriver.CloseConnection(m_dwSocket);
}


/*====================
  CSocket::RequiresConnection
  ====================*/
bool	CSocket::RequiresConnection()
{
	if (m_eType == K2_SOCKET_TCP || m_eType == K2_SOCKET_TCP2)
		return true;

	return false;
}


/*====================
  CSocket::AllowBroadcast
  ====================*/
void	CSocket::AllowBroadcast(bool bValue)
{
	NetDriver.AllowBroadcast(m_dwSocket, bValue);
}


/*====================
  CSocket::GetBroadcastAddress
  ====================*/
tstring	CSocket::GetBroadcastAddress()
{
	return NetDriver.GetBroadcastAddress(m_dwSocket);
}


/*====================
  CSocket::SetSendAddr
  ====================*/
bool	CSocket::SetSendAddr(const tstring &sAddr, word wPort)
{
	PROFILE("CSocket::SetSendAddr");

	try
	{
		if (!m_bInitialized)
			EX_WARN(_T("Socket has not been initialized yet"));

		tstring sRequestAddr(sAddr);
		word	wRequestPort(wPort ? wPort : m_wSendPort);
		
		Console.Net << _T("[") << m_sName << _T("] Setting send address to: ") << sAddr
			<< ((wPort == 0) ? _T("") : (_T(":") + XtoA(wPort))) << newl;
		
		if (!NetDriver.SetSendAddr(sRequestAddr, wRequestPort, m_pSendAddr, m_bIsLocalConnection, m_bIsLANConnection))
			EX_WARN(_T("Failed to set address: ") + sAddr);

		m_sSendAddrName = sRequestAddr;
		m_wSendPort = wRequestPort;
		Console.Net << _T("[") << m_sName << _T("] Address resolved to: ") << m_sSendAddrName << _T(":") << m_wSendPort << newl;

		if (m_bIsLocalConnection)
			Console.Net << _T("[") << m_sName << _T("] This is a local connection.") << newl;

		if (m_bIsLANConnection)
			Console.Net << _T("[") << m_sName << _T("] This is a LAN connection.") << newl;

		if (RequiresConnection())
		{
			if (!NetDriver.Connect(m_dwSocket, m_pSendAddr))
				EX_ERROR(_T("Failed to connect"));
		}

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSocket::SetSendAddr() [") + m_sName + _T("] - "), NO_THROW);
		return false;
	}
}


/*====================
  CSocket::ProcessReliablePacket
  ====================*/
bool	CSocket::ProcessReliablePacket(CPacket &pkt)
{
	try
	{
		if (m_eType != K2_SOCKET_GAME)
			EX_ERROR(_T("Only GAME sockets can use reliable packets"));

		uint uiSequence(pkt.GetSequence());

		if (net_debugReliable)
			Console.Net << _T("[") << m_sName << _T("] Received reliable packet #") << uiSequence
						<< _T(", expecting #") << m_uiReliableRecvLastSeq + 1 << newl;

		bool bProcess(true);
	    if (uiSequence > m_uiReliableRecvLastSeq + 1)
		{
			ReliablePktList::iterator it(m_lOutOfSequencePackets.begin());
			for (; it != m_lOutOfSequencePackets.end(); ++it)
			{
				if (it->GetSequenceID() == uiSequence)
					break;
			}

			if (it == m_lOutOfSequencePackets.end())
			{
				if (net_debugReliable)
					Console.Net << _T("[") << m_sName << _T("] Pre-ACKing and saving #") << uiSequence << newl;
				bProcess = false;
				m_lOutOfSequencePackets.push_back(pkt);
			}
			else
			{
				if (net_debugReliable)
					Console.Net << _T("[") << m_sName << _T("] ACKing duplicate out of sequence packet #") << uiSequence << newl;
				bProcess = false;
			}
		}

		if (uiSequence < m_uiReliableRecvLastSeq + 1)
		{
			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] Already received this one, ACKing it again") << newl;
			bProcess = false;
		}

		if (uiSequence == m_uiReliableRecvLastSeq + 1)
			++m_uiReliableRecvLastSeq;

		// Send an ACK for this reliable packet
		CPacket ackpacket;
		ackpacket.WriteInt(uiSequence);
		ackpacket.SetHeader(m_unConnectionID, PACKET_NORMAL | PACKET_ACK);
		
		size_t zSent(NetDriver.SendPacket(m_dwSocket, m_pSendAddr, ackpacket, true));
		if (zSent > 0)
		{
			m_uiBytesSent += uint(zSent);
			++m_uiPacketsSent;

			++m_uiAcksSent;
		}
		else
		{
			m_uiBytesDropped += ackpacket.GetLength();
			++m_uiPacketsDropped;
		}

		return bProcess;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSocket::ProcessReliablePacket() [") + m_sName + _T("] - "), NO_THROW);
		return false;
	}
}


/*====================
  CSocket::ProcessPacketAck
  ====================*/
bool	CSocket::ProcessPacketAck(CPacket &pkt)
{
	try
	{
		uint uiSequence(pkt.ReadInt());

		if (net_debugReliable)
			Console.Net << _T("[") << m_sName << _T("] Received ACK #") << uiSequence << newl;

		ReliablePktList::iterator itPacket(m_UnackPackets.begin());
		if (itPacket == m_UnackPackets.end())
		{
			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] - ") << _T("No packets currently waiting for an ACK") << newl;

			return true;
		}

		while (itPacket != m_UnackPackets.end() && itPacket->GetSequenceID() != uiSequence)
		{
			++itPacket;
		}

		if (itPacket == m_UnackPackets.end())
		{
			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] - ") << _T("ACK does not match any currently waiting packets") << newl;

			return true;
		}

		// Free ACK'd packet
		m_UnackPackets.erase(itPacket);
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSocket::ProcessPacketAck() [") + m_sName + _T("] - "), NO_THROW);
		return false;
	}
}


/*====================
  CSocket::PreProcessPacket

  Filter out bad packets and ACKs
  ====================*/
bool	CSocket::PreProcessPacket(CPacket &pkt)
{
	PROFILE("CSocket::PreProcessPacket");

	// If this is a reliable packet, make sure it is in sequence and then send an ACK
	if (pkt.GetHeader()->m_yFlags & PACKET_RELIABLE)
		return ProcessReliablePacket(pkt);

	// Process ACKS and discard them
	if (pkt.GetHeader()->m_yFlags & PACKET_ACK)
	{
		ProcessPacketAck(pkt);
		return true;
	}

	return true;
}


/*====================
  CSocket::CheckOutOfSequencePacket
  ====================*/
int		CSocket::CheckOutOfSequencePacket(CPacket &pkt)
{
	PROFILE("CSocket::CheckOutOfSequencePacket");

	if (!m_bInitialized)
		return 0;

	for (ReliablePktList::iterator it(m_lOutOfSequencePackets.begin()); it != m_lOutOfSequencePackets.end(); ++it)
	{
		if (it->GetSequenceID() == m_uiReliableRecvLastSeq + 1)
		{
			pkt = it->GetPacket();
			it = m_lOutOfSequencePackets.erase(it);
			return pkt.GetLength();
		}
	}

	pkt.Clear();
	return 0;
}


/*====================
  CSocket::ReceivePacket
  ====================*/
int		CSocket::ReceivePacket(CPacket &pkt)
{
	PROFILE("CSocket::ReceivePacket");

	if (!m_bInitialized)
		return 0;

	int iBytes(CheckOutOfSequencePacket(pkt));
	if (iBytes > 0)
		return iBytes;

	pkt.Clear();
	return NetDriver.ReceivePacket(m_eType, m_dwSocket, pkt, m_sRecvAddrName, m_wRecvPort, m_cBuffer);
}


/*====================
  CSocket::IsConnected
  ====================*/
bool	CSocket::IsConnected(int iMSecToWait)
{
	if (!m_bInitialized)
		return false;

	return NetDriver.Connected(m_dwSocket, iMSecToWait);
}


/*====================
  CSocket::HasError
  ====================*/
bool	CSocket::HasError(uint uiWaitTime)
{
	if (!m_bInitialized)
		return false;

	return NetDriver.HasError(m_dwSocket, uiWaitTime);
}


/*====================
  CSocket::DataWaiting
  ====================*/
bool	CSocket::DataWaiting(uint uiWaitTime)
{
	return NetDriver.DataWaiting(m_dwSocket, m_cBuffer, uiWaitTime);
}


/*====================
  CSocket::OpenListenPort
  ====================*/
bool	CSocket::OpenListenPort(int iMaxConnectionsWaiting)
{
	if (!m_bInitialized)
		return false;

	if (RequiresConnection())
		return NetDriver.StartListening(m_dwSocket, iMaxConnectionsWaiting);
	else
		return false;
}


/*====================
  CSocket::AcceptConnection
  ====================*/
CSocket*	CSocket::AcceptConnection(const tstring &sSocketName)
{
	CSocket *sockNewConn(K2_NEW(ctx_Net,  CSocket)(sSocketName));

	// If it doesn't require a connection, it will return
	// an uninitialized socket, which indicates an error.
	if (RequiresConnection())
	{
		dword dwTempSocket = m_dwSocket;
		m_dwSocket = NetDriver.AcceptConnection(m_dwSocket);

		if (m_dwSocket != SOCKET_INVALID)
			sockNewConn->Init(*this);
		else
			SAFE_DELETE(sockNewConn);

		m_dwSocket = dwTempSocket;
	}

	return sockNewConn;
}


/*====================
  CSocket::SendPacket
  ====================*/
bool	CSocket::SendPacket(CPacket &pkt)
{
	PROFILE("CSocket::SendPacket");

	try
	{
		if (!m_bInitialized)
			EX_ERROR(_T("Socket not initialized"));

		size_t zHeaderLength(0);
		if (m_eType == K2_SOCKET_GAME)
		{
			zHeaderLength = 3;
			pkt.SetHeader(m_unConnectionID, PACKET_NORMAL);
		}

		size_t zSent(NetDriver.SendPacket(m_dwSocket, m_pSendAddr, pkt, m_eType == K2_SOCKET_GAME));
		if (zSent < pkt.GetLength() + zHeaderLength)
		{
			Console.Net << _T("[") + m_sName + _T("] ") +
				_T("Truncated packet ") + XtoA(uint(zSent)) + _T(" / ") + XtoA(uint(pkt.GetLength() + zHeaderLength)) << newl;

			m_uiBytesDropped += pkt.GetLength() + zHeaderLength;
			++m_uiPacketsDropped;
			
			return false;
		}
		else
		{
			m_uiBytesSent += uint(zSent);
			++m_uiPacketsSent;

			++m_uiUnreliablePacketsSent;
		}

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSocket::SendPacket() - [") + m_sName + _T("] "), NO_THROW);
		return false;
	}
}


/*====================
  CSocket::SendReliablePacket
  ====================*/
bool	CSocket::SendReliablePacket(CPacket &pktSend, bool bQueue)
{
	try
	{
		if (!m_bInitialized)
			EX_ERROR(_T("Socket has not been initialized"));

		if (m_eType != K2_SOCKET_GAME)
			EX_ERROR(_T("Only GAME sockets can use reliable packets"));

		size_t zLength(pktSend.GetLength() + HEADER_SIZE);

		if (!net_reliablePackets)
		{
			pktSend.SetHeader(m_unConnectionID, PACKET_NORMAL);
			size_t zSent(NetDriver.SendPacket(m_dwSocket, m_pSendAddr, pktSend, true));
			pktSend.Clear();

			if (zSent == zLength)
			{
				m_uiBytesSent += uint(zSent);
				++m_uiPacketsSent;

				++m_uiUnreliablePacketsSent;
				return true;
			}
			else
			{
				m_uiBytesDropped += uint(zLength);
				++m_uiPacketsDropped;

				++m_uiUnreliablePacketsSent;
				return false;
			}
		}

		if (bQueue)
		{
			uint uiNewSeq(m_uiReliableSendLastSeq + 1);

			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] Queuing reliable packet #") << uiNewSeq << newl;

			pktSend.SetHeader(m_unConnectionID, PACKET_NORMAL | PACKET_RELIABLE, uiNewSeq);

			// Add this packet to the end of the list of sent reliable packets waiting for ACKs
			CReliablePacket	pktSave(pktSend);
			m_UnackPackets.push_back(pktSave);
			m_uiReliableSendLastSeq = uiNewSeq;

			pktSend.Clear();
		}
		else
		{
			uint uiNewSeq(m_uiReliableSendLastSeq + 1);

			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] Sending reliable packet #") << uiNewSeq << newl;

			pktSend.SetHeader(m_unConnectionID, PACKET_NORMAL | PACKET_RELIABLE, uiNewSeq);

			// Add this packet to the end of the list of sent reliable packets waiting for ACKs
			CReliablePacket	pktSave(pktSend);
			m_UnackPackets.push_back(pktSave);
			m_uiReliableSendLastSeq = uiNewSeq;

			if (net_forcedPacketDrop != 0.0f && net_forcedPacketDrop > M_Randnum(0.0f, 1.0f))
				EX_DEBUG(_T("Packet was artificially dropped by net_forcePacketDrop"));

			size_t zSent(NetDriver.SendPacket(m_dwSocket, m_pSendAddr, pktSend, true));
			if  (zSent < zLength)
			{
				Console.Net << _T("[") + m_sName + _T("] ") + 
					_T("Truncated reliable packet ") + XtoA(uint(zSent)) + _T(" / ") + XtoA(uint(pktSend.GetLength())) << newl;
				pktSend.Clear();

				m_uiBytesDropped += uint(zLength);
				++m_uiPacketsDropped;

				return false;
			}
			else
			{
				m_uiBytesSent += uint(zSent);
				++m_uiPacketsSent;

				++m_uiReliablePacketsSent;
			}

			pktSend.Clear();
		}
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CSocket::SendReliablePacket() [") + m_sName + _T("] - "), NO_THROW);
		pktSend.Clear();
		return false;
	}
}


/*====================
  CSocket::CheckPacketTimeouts
  ====================*/
void	CSocket::CheckPacketTimeouts()
{
	if (!m_bInitialized)
		return;

	ReliablePktList::iterator itPacket(m_UnackPackets.begin());
	if (itPacket == m_UnackPackets.end())
		return;

	uint time(Host.GetSystemTime());
	int numPacketsSent(0);

	while (itPacket != m_UnackPackets.end() && numPacketsSent < MAX_PACKETS_RESENT_PER_FRAME)
	{
		if (time - itPacket->GetTimeStamp() > PACKET_TIMEOUT)
		{
			if (net_debugReliable)
				Console.Net << _T("[") << m_sName << _T("] Resending reliable packet #") << itPacket->GetSequenceID() << newl;

			size_t zSent(NetDriver.SendPacket(m_dwSocket, m_pSendAddr, itPacket->GetPacket(), true));
			if (zSent == 0)
			{
				m_uiBytesDropped += itPacket->GetPacket().GetLength();
				++m_uiPacketsDropped;
			}
			else
			{
				m_uiBytesSent += uint(zSent);
				++m_uiPacketsSent;

				++m_uiReliablePacketsSent;
			}

			itPacket->SetTimeStamp(time);
			++numPacketsSent;
		}
		++itPacket;
	}
}


/*====================
  CSocket::ClearReliablePackets
  ====================*/
void    CSocket::ClearReliablePackets()
{
	m_UnackPackets.clear();
}


/*====================
  CSocket::GetLocalAddr
  ====================*/
tstring	CSocket::GetLocalAddr() const
{
	string sAddr;
	for (int i = 0; i < 4; i++)
	{
		sAddr += XtoS(NetDriver.GetLocalIPByte(i));
		
		if (i < 3)
			sAddr += ".";
	}

	return StringToTString(sAddr);
}


/*====================
  CSocket::GetOldestReliable
  ====================*/
uint	CSocket::GetOldestReliable() const
{
	uint uiSystemTime(Host.GetSystemTime());
	uint uiOldest(0);

	for (ReliablePktList::const_iterator it(m_UnackPackets.begin()); it != m_UnackPackets.end(); ++it)
	{
		uint uiPacketAge(uiSystemTime - it->GetOriginalTimeStamp());

		if (uiPacketAge > uiOldest)
			uiOldest = uiPacketAge;
	}

	return uiOldest;
}


/*====================
  CSocket::SetBlockIncoming
  ====================*/
void	CSocket::SetBlockIncoming(bool b)
{
	if (m_bInitialized)
		NetDriver.SetBlockIncoming(b);
}


/*====================
  CSocket::SetBlockOutgoing
  ====================*/
void	CSocket::SetBlockOutgoing(bool b)
{
	if (m_bInitialized)
		NetDriver.SetBlockOutgoing(b);
}
