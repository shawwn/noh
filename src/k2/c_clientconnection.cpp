// (C)2005 S2 Games
// c_clientconnection.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_clientconnection.h"
#include "c_buffer.h"
#include "c_hostserver.h"
#include "c_world.h"
#include "c_clientsnapshot.h"
#include "c_zip.h"
#include "c_voiceserver.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_BOOL(svr_broadcast);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTR	(svr_maxbps,						MAX_BPS,						CVAR_SAVECONFIG,	MIN_BPS,	MAX_BPS);
CVAR_INTF	(svr_clientWarnTimeout,			SecToMs(2u),					CVAR_SAVECONFIG);
CVAR_INTF	(svr_clientConnectedTimeout,		SecToMs(30u),					CVAR_SAVECONFIG);
CVAR_INTF	(svr_clientConnectingTimeout,	SecToMs(30u),					CVAR_SAVECONFIG);
CVAR_UINTF	(svr_firstSnapshotRetryInterval,	5000,							CVAR_SAVECONFIG);
CVAR_UINTF	(svr_minSnapshotCompressSize,	256,							CVAR_SAVECONFIG);
CVAR_BOOLF	(svr_snapshotCompress,			false,							CVAR_SAVECONFIG);
CVAR_UINTF	(svr_minStateStringCompressSize,	256,							CVAR_SAVECONFIG);
CVAR_BOOLF	(svr_stateStringCompress,		true,							CVAR_SAVECONFIG);
CVAR_UINT	(svr_bandwidthWarnTime,			5000);
CVAR_UINTF	(svr_reliableUnresponsiveTime,	1500,							CVAR_SAVECONFIG);
//CVAR_BOOLF(		svr_requireAuthentication,		false,							CVAR_SAVECONFIG);
CVAR_UINTF	(svr_authTimeout,				SecToMs(10u),					CVAR_SAVECONFIG);
CVAR_INTF	(svr_kickBanCount,				2,								CVAR_SAVECONFIG);

CVAR_UINTF	(svr_maxIncomingPacketsPerSecond,	300,						CVAR_SAVECONFIG);
CVAR_UINTF	(svr_maxIncomingBytesPerSecond,		10240,						CVAR_SAVECONFIG);

CVAR_STRINGF(svr_masterServerAuthScript,		"/server_requester.php",		CVAR_SAVECONFIG);
CVAR_UINTF	(svr_clientRefreshUpgradesThrottle,	5000,						CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CClientConnection::~CClientConnection
  ====================*/
CClientConnection::~CClientConnection()
{
	if (m_eConnectionState != CLIENT_CONNECTION_STATE_DISCONNECTED)
	{
		CPacket pkt;
		pkt << NETCMD_KICK << _T("Unknown");
		SendPacket(pkt);
	}

	SAFE_DELETE_SNAPSHOT(m_hFirstSnapshot);
	SAFE_DELETE_SNAPSHOT(m_hRetryFirstSnapshot);
	SAFE_DELETE_SNAPSHOT(m_hLastAckedSnapshot);
}


/*====================
  CClientConnection::CClientConnection
  ====================*/
CClientConnection::CClientConnection(CHostServer* pHostServer, CHTTPManager *pHTTPManager, const tstring &sAddress, ushort unPort, CSocket &sockGame) :
m_pHostServer(pHostServer),
m_pHTTPManager(pHTTPManager),

m_iClientNum(-1),
m_iAccountID(-1),
m_iClanID(-1),
m_unConnectionID(0),
m_uiFlags(0),
m_fLoadingProgress(0.0f),

m_sockGame(_T("CLIENT")),
m_sAddress(sAddress),
m_sPublicAddress(sAddress),
m_unPort(unPort),

m_eConnectionState(CLIENT_CONNECTION_STATE_CONNECTING),
m_uiLastReceiveTime(INVALID_TIME),
m_uiLastAckedServerFrame(0),
m_uiLastReceivedClientTime(m_pHostServer->GetTime()),
m_bFirstAckReceived(false),
m_bWorldLoaded(false),
m_unPing(0),

m_bSnapshotIsCompressed(false),
m_hFirstSnapshot(INVALID_POOL_HANDLE),
m_hRetryFirstSnapshot(INVALID_POOL_HANDLE),
m_bFirstSnapshotSent(false),
m_uiFirstSnapshotSendTime(0),
m_hLastAckedSnapshot(INVALID_POOL_HANDLE),
m_uiSnapshotBufferIndex(0),
m_uiSnapshotFragmentFrame(0),
m_ySnapshotFragmentIndex(0),
m_uiPrevButtonStates(0),

m_yStateSequence(0),

m_unStateStringFragmentIndex(0),
m_uiStateStringBufferIndex(0),
m_bStateStringIsCompressed(false),

m_unStateBlockFragmentIndex(0),
m_uiStateBlockBufferIndex(0),
m_bStateBlockIsCompressed(false),

m_bAllStateDataSent(false),

m_zSentLength(0),
m_uiMaxPacketSize(1300),
m_uiMaxBPS(20000),
m_uiNetFPS(20),

m_bSnapshotSent(false),

m_uiLastBandwidthWarn(INVALID_TIME),
m_bBehind(false),

m_uiLastLoadingProgressUpdate(INVALID_TIME),
m_uiLongestLoadingProgressInterval(0),

m_uiLastAuthSuccess(INVALID_TIME),
m_uiAuthRequestTime(INVALID_TIME),

#ifndef K2_CLIENT
m_pAuthenticateRequest(NULL),
m_pValidateMatchKeyRequest(NULL),
m_pRefreshUpgradesRequest(NULL),
m_pRecentMatchStatsRequest(NULL),
m_bForceTermination(false),
#endif

m_uiStream(0),
m_uiLastAckedStream(0),

m_bRefreshUpgrades(false),
m_uiLastRefreshUpgradesTime(INVALID_TIME)
{
	assert(m_pHostServer != NULL);

	m_sockGame.Init(sockGame);
	m_sockGame.SetSendAddr(sAddress, unPort);

	ClearFlags();
	//if (m_sockGame.IsLocalConnection())
	//	SetFlags(CLIENT_CONNECTION_LOCAL);
	if (m_sockGame.IsLANConnection())
		SetFlags(CLIENT_CONNECTION_LAN);

	// Check each state block
	const StateBlockVector &vStateBlocks(m_pHostServer->GetStateBlockVector());
	m_vStateBlockModCounts.resize(vStateBlocks.size(), -1);
	for (StateBlockVector_cit itBlock(vStateBlocks.begin()); itBlock != vStateBlocks.end(); ++itBlock)
		m_vStateBlockModCounts[itBlock - vStateBlocks.begin()] = itBlock->GetModifiedCount();

	if (sAddress.empty())
	{
		SetFlags(CLIENT_CONNECTION_PSEUDO);

		m_eConnectionState = CLIENT_CONNECTION_STATE_IN_GAME;

		m_iAccountID = 0;
		m_unConnectionID = -9000;
		m_iClientNum = m_pHostServer->GenerateClientID(m_unConnectionID);

		m_sName = _T("Pseudo") + XtoA(m_iClientNum);
	}
}


/*====================
  CClientConnection::ProcessNetSettings
  ====================*/
bool	CClientConnection::ProcessNetSettings(CPacket &pkt)
{
	Console.Server << _T("Received net settings from client #") << m_iClientNum << newl;

	uint uiMaxPacketSize(pkt.ReadInt());
	m_uiMaxPacketSize = CLAMP(uiMaxPacketSize, MIN_MTU_SIZE, MAX_MTU_SIZE);
	Console.Server << _T("Client #") << m_iClientNum << _T(" set maxPacketSize to ") << m_uiMaxPacketSize << newl;

	uint uiMaxBPS(pkt.ReadInt());
	m_uiMaxBPS = CLAMP(uiMaxBPS, MIN_BPS, svr_maxbps.GetUnsignedInteger());
	Console.Server << _T("Client #") << m_iClientNum << _T(" set max bytes per second to ") << m_uiMaxBPS << newl;

	uint uiNetFPS(pkt.ReadInt());
	m_uiNetFPS = CLAMP(uiNetFPS, MIN_NET_FPS, MAX_NET_FPS);
	Console.Server << _T("Client #") << m_iClientNum << _T(" set net fps to ") << m_uiNetFPS << newl;

	return true;
}


/*====================
  CClientConnection::SendPacket
  ====================*/
bool	CClientConnection::SendPacket(CPacket &pkt)
{
	if (pkt.GetLength() > MAX_MTU_SIZE)
		return false;

	m_zSentLength += pkt.GetLength();
	return m_sockGame.SendPacket(pkt);
}


/*====================
  CClientConnection::SendReliablePacket
  ====================*/
bool	CClientConnection::SendReliablePacket(CPacket &pkt)
{
	if (pkt.GetLength() > MAX_MTU_SIZE)
	{
		// FIXME: Allow the packet to be fragmented?
		Console.Warn << _T("Reliable packet overflow for client: #") << m_iClientNum << _T(", packet size: ") << pkt.GetLength() << newl;
		Console.Warn << _T("DATA: 0x");
			
		for (uint i(0); i < pkt.GetLength(); ++i)
			Console.Warn << XtoA(pkt.ReadByte(), FMT_PADZERO | FMT_NOPREFIX, 2, 16);

		Console.Warn << newl;

		m_pktReliable.Clear();
		m_pktSend.Clear();
		Disconnect(_T("disconnect_overflow"));
		return false;
	}

	m_zSentLength += pkt.GetLength();

	bool bQueue(false);
	if (Host.GetSystemTime() - m_uiLastReceiveTime > svr_reliableUnresponsiveTime)
		bQueue = true;

	return m_sockGame.SendReliablePacket(pkt, bQueue);
}


/*====================
  CClientConnection::SendPackets
  ====================*/
void	CClientConnection::SendPackets(uint uiFPS)
{	
	GAME_PROFILE(_T("CClientConnection::SendPackets"));

	if (GetState() == CLIENT_CONNECTION_STATE_DISCONNECTED)
		return;

	if (HasFlags(CLIENT_CONNECTION_PSEUDO))
	{
		m_uiLastReceiveTime = Host.GetSystemTime();

		m_pktReliable.Clear();
		m_pktSend.Clear();

		m_bSnapshotSent = false;
		return;
	}

	if (m_uiLastReceiveTime == INVALID_TIME)
		m_uiLastReceiveTime = Host.GetSystemTime();

	if (Host.GetSystemTime() - m_uiLastReceiveTime <= svr_reliableUnresponsiveTime)
		m_sockGame.CheckPacketTimeouts();

	// Send a reliable packet, if data is waiting
	if (!m_pktReliable.IsEmpty())
	{	  		
		if (m_pktReliable.GetLength() > MAX_MTU_SIZE)
		{
			// FIXME: Allow the packet to be fragmented?
			Console.Warn << _T("Reliable packet overflow for client: #") << m_iClientNum << _T(", packet size: ") << m_pktReliable.GetLength() << newl;
			Console.Warn << _T("DATA: 0x");
			
			for (uint i(0); i < m_pktReliable.GetLength(); i++)
				Console.Warn << XtoA(m_pktReliable.ReadByte(), FMT_PADZERO | FMT_NOPREFIX, 2, 16);

			Console.Warn << newl;

			m_pktReliable.Clear();
			m_pktSend.Clear();
			Disconnect(_T("disconnect_overflow"));
			return;
		}
		else
		{
			SendReliablePacket(m_pktReliable);
			m_pktReliable.Clear();
		}
	}

	if (!m_pktSend.IsEmpty())
	{
		if (m_pktSend.GetLength() > MAX_MTU_SIZE)
		{
			// Drop the packet
			Console.Warn << _T("Skipping large packet ") << ParenStr(GetByteString(m_pktSend.GetLength())) << newl;
			m_pktSend.Clear();
		}
		else
		{
			SendPacket(m_pktSend);
			m_pktSend.Clear();
		}
	}

	m_bSnapshotSent = false;

	m_deqSendSize.push_back(m_zSentLength);
	while (m_deqSendSize.size() > uiFPS)
		m_deqSendSize.pop_front();

	m_zSentLength = 0;
}


/*====================
  CClientConnection::SendGameData
  ====================*/
void	CClientConnection::SendGameData(const IBuffer &buffer, bool bReliable)
{
	if (GetState() == CLIENT_CONNECTION_STATE_DISCONNECTED)
		return;

	if (bReliable)
	{
		m_pktReliable << NETCMD_SERVER_GAME_DATA << buffer;
	}
	else
	{
		if (m_bSnapshotSent)
		{
			Console.Err << _T("Trying to send game data after snapshot") << newl;
			return;
		}

		if (m_pktSend.GetLength() + 1 + buffer.GetLength() <= MAX_MTU_SIZE)
			m_pktSend << NETCMD_SERVER_GAME_DATA << buffer;
	}
}


/*====================
  CClientConnection::CheckTimeout
  ====================*/
bool	CClientConnection::CheckTimeout()
{
	if (HasFlags(CLIENT_CONNECTION_LOCAL))
		return false;
	if (m_uiLastReceiveTime == INVALID_TIME)
		return false;

	uint uiTimeoutTime;
	if (m_eConnectionState == CLIENT_CONNECTION_STATE_IN_GAME)
		uiTimeoutTime = svr_clientConnectedTimeout;
	else
		uiTimeoutTime = svr_clientConnectingTimeout;

	uint uiWarnTimeoutTime = INVALID_TIME;
	if (m_eConnectionState == CLIENT_CONNECTION_STATE_IN_GAME)
		uiWarnTimeoutTime = svr_clientWarnTimeout;

	uint uiTime(Host.GetSystemTime());
	if (uiTime - m_uiLastReceiveTime > uiTimeoutTime)
	{
		Console.Server << _T("Client #") << GetClientNum()
			<< _T(" timed out during state: ") << ((m_eConnectionState == CLIENT_CONNECTION_STATE_IN_GAME) ? _T("in game") : _T("loading"))
			<< _T(" @ ") << Host.GetSystemTime() << newl;
		return true;
	}

	return false;
}


/*====================
  CClientConnection::IsTimingOut
  ====================*/
bool	CClientConnection::IsTimingOut()
{
	if (HasFlags(CLIENT_CONNECTION_LOCAL))
		return false;
	if (m_uiLastReceiveTime == INVALID_TIME)
		return false;

	if (m_eConnectionState != CLIENT_CONNECTION_STATE_IN_GAME)
		return false;

	uint uiWarnTimeoutTime(svr_clientWarnTimeout);
	uint uiTime(Host.GetSystemTime());
	if (uiTime - m_uiLastReceiveTime > uiWarnTimeoutTime)
		return true;

	return false;
}


/*====================
  CClientConnection::SendLoadWorldRequest
  ====================*/
void	CClientConnection::SendLoadWorldRequest()
{
	if (!m_pHostServer->GetWorld()->IsLoaded())
		return;

	m_pktReliable << NETCMD_LOAD_WORLD << m_pHostServer->GetWorld()->GetName();
	m_eConnectionState = CLIENT_CONNECTION_STATE_STANDBY;
	m_uiLastReceiveTime = INVALID_TIME;
}


/*====================
  CClientConnection::NewGameStarted
  ====================*/
void	CClientConnection::NewGameStarted()
{
	m_uiLastReceiveTime = INVALID_TIME;
	m_uiLastReceivedClientTime = m_pHostServer->GetTime();
}


/*====================
  CClientConnection::SendAllStateData
  ====================*/
void	CClientConnection::SendAllStateData()
{
	PROFILE("CClientConnection::SendAllStateData");

	Console.Server << _T("Sending state data to client #") << m_iClientNum << newl;

	m_pktReliable << NETCMD_START_STATE_DATA;
	m_yStateSequence = 0;

	// Queue up all state strings for sending
	const vector<CStateString> &vStateStrings(m_pHostServer->GetStateStringVector());
	for (vector<CStateString>::const_iterator cit(vStateStrings.begin()); cit != vStateStrings.end(); ++cit)
	{
		ushort unIndex(ushort(cit - vStateStrings.begin()));

		if (!cit->IsEmpty())
			AddStringToUpdateQueue(unIndex, m_pHostServer->GetStateString(unIndex));
	}

	// Queue up all state blocks for sending
	const StateBlockVector &vStateBlocks(m_pHostServer->GetStateBlockVector());
	for (StateBlockVector_cit cit(vStateBlocks.begin()); cit != vStateBlocks.end(); ++cit)
	{
		ushort unIndex(ushort(cit - vStateBlocks.begin()));

		if (!cit->IsEmpty())
			AddBlockToUpdateQueue(unIndex, m_pHostServer->GetStateBlock(unIndex));
	}
}


/*====================
  CClientConnection::AddStringToUpdateQueue
  ====================*/
void	CClientConnection::AddStringToUpdateQueue(ushort unID, const CStateString &ss)
{
	PROFILE("CClientConnection::AddStringToUpdateQueue");

	if (!IsConnected())
		return;

	if (unID == STATE_STRING_CVAR_SETTINGS && HasFlags(CLIENT_CONNECTION_LOCAL))
		return;

	if (ss.IsEmpty())
		return;

	m_deqStateStringUpdates.push_back(ss);
	m_deqStateStringUpdatesID.push_back(unID);
	++m_yStateSequence;
	Console.Server << _T("State string #") << unID <<_T(" added to update queue for client #") << GetClientNum() << newl;
}


/*====================
  CClientConnection::AddBlockToUpdateQueue
  ====================*/
void	CClientConnection::AddBlockToUpdateQueue(ushort unID, const CStateBlock &block)
{
	PROFILE("CClientConnection::AddBlockToUpdateQueue");

	if (!IsConnected() || m_iAccountID == -1)
		return;

	if (block.IsEmpty())
		return;

	m_deqStateBlockUpdates.push_back(block);
	m_deqStateBlockUpdatesID.push_back(unID);
	
	if (m_vStateBlockModCounts.size() <= unID)
		m_vStateBlockModCounts.resize(unID + 1);
	m_vStateBlockModCounts[unID] = block.GetModifiedCount();
	
	++m_yStateSequence;
	Console.Server << _T("State block #") << unID <<_T(" added to update queue for client #") << GetClientNum() << _T(" [") << m_vStateBlockModCounts[unID] << _T("]") << newl;
}


/*====================
  CClientConnection::SendStateStrings

  Send over all state strings that have been modified to the client reliably
  if the data won't fit in this packet, it will be queued for the next frame
  ====================*/
void	CClientConnection::SendStateStrings()
{
	PROFILE("CClientConnection::SendStateStrings");

	try
	{
		// Check to see if the packet is already full
		if (m_pktReliable.GetLength() >= m_uiMaxPacketSize)
			return;

		// Make sure there is enough room to actually send something worth while
		uint uiBytesRemaining(m_uiMaxPacketSize - m_pktReliable.GetLength());
		if (uiBytesRemaining < 10)
			return;

		// If a string got fragmented, send the continuation
		if (!m_bufferStateString.IsEmpty())
		{
			uint uiBytesToWrite(m_bufferStateString.GetLength() - m_uiStateStringBufferIndex);
			if (uiBytesToWrite + (m_bStateStringIsCompressed ? 9 : 5) > uiBytesRemaining)
			{
				uiBytesToWrite = MIN(uiBytesToWrite, uiBytesRemaining - 5);
				m_pktReliable << NETCMD_STATE_STRING_FRAGMENT << m_unStateStringFragmentIndex << ushort(uiBytesToWrite);
				m_pktReliable.Write(m_bufferStateString.Get(m_uiStateStringBufferIndex), uiBytesToWrite);
				m_uiStateStringBufferIndex += uiBytesToWrite;
				return;
			}
			else
			{
				if (m_bStateStringIsCompressed)
					m_pktReliable << NETCMD_COMPRESSED_STATE_STRING_TERMINATION << m_unStateStringFragmentIndex << ushort(uiBytesToWrite) << m_uiUncompressedStateStringLength ;
				else
					m_pktReliable << NETCMD_STATE_STRING_TERMINATION << m_unStateStringFragmentIndex << ushort(uiBytesToWrite);

				m_pktReliable.Write(m_bufferStateString.Get(m_uiStateStringBufferIndex), uiBytesToWrite);
				m_unStateStringFragmentIndex = 0;
				m_bufferStateString.Clear();
				m_bStateStringIsCompressed = false;
				m_uiStateStringBufferIndex = 0;
			}

			uiBytesRemaining = m_uiMaxPacketSize - m_pktReliable.GetLength();
		}

		// Step through the deque of strings waiting to send
		while (!m_deqStateStringUpdates.empty() && uiBytesRemaining >= 10)
		{
			try
			{
				// Retrieve the next string
				ushort unIndex(m_deqStateStringUpdatesID.front());
				const CStateString &ss(m_deqStateStringUpdates.front());

				if (ss.IsEmpty())
				{
					m_deqStateStringUpdates.pop_front();
					m_deqStateStringUpdatesID.pop_front();
					continue;
				}

				Console.Server << _T("Sending state string update: ") << unIndex << newl;

				// Compress string data
				m_bufferStateString.Clear();
				ss.AppendToBuffer(m_bufferStateString);
				m_uiUncompressedStateStringLength = m_bufferStateString.GetLength();
				if (m_uiUncompressedStateStringLength >= svr_minStateStringCompressSize && svr_stateStringCompress)
				{
					PROFILE("Statestring compression");

					byte* pCompressed(NULL);
					uint uiResultLength(CZip::Compress((const byte*)m_bufferStateString.Get(), m_uiUncompressedStateStringLength, pCompressed));
					m_bufferStateString.Write(pCompressed, uiResultLength);
					SAFE_DELETE_ARRAY(pCompressed);
					m_bStateStringIsCompressed = true;
				}

				// If it won't fit in the packet, send a fragment
				if (m_bufferStateString.GetLength() + (m_bStateStringIsCompressed ? 9 : 5) > uiBytesRemaining)
				{
					ushort unBytesToWrite(MIN(m_bufferStateString.GetLength(), uiBytesRemaining - 5));
					m_pktReliable << NETCMD_STATE_STRING_FRAGMENT << unIndex << unBytesToWrite;
					m_pktReliable.Write(m_bufferStateString.Get(), unBytesToWrite);

					m_uiStateStringBufferIndex = unBytesToWrite;
					m_unStateStringFragmentIndex = unIndex;

					m_deqStateStringUpdates.pop_front();
					m_deqStateStringUpdatesID.pop_front();

					break;
				}
					
				// Send the complete string
				if (m_bStateStringIsCompressed)
					m_pktReliable << NETCMD_COMPRESSED_STATE_STRING << unIndex << ushort(m_bufferStateString.GetLength()) << m_uiUncompressedStateStringLength << m_bufferStateString;
				else
					m_pktReliable << NETCMD_STATE_STRING << unIndex << ushort(m_bufferStateString.GetLength()) << m_bufferStateString;

				m_bufferStateString.Clear();
				m_uiStateStringBufferIndex = 0;
				m_bStateStringIsCompressed = false;

				m_deqStateStringUpdates.pop_front();
				m_deqStateStringUpdatesID.pop_front();

				uiBytesRemaining = m_uiMaxPacketSize - m_pktReliable.GetLength();
			}
			catch (CException &ex)
			{
				ex.Process(_T("CClientConnection::SendStateStrings() - "), NO_THROW);
				m_deqStateStringUpdates.pop_front();
				m_deqStateStringUpdatesID.pop_front();
				continue;
			}
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientConnection::SendStateStrings() - "), NO_THROW);
	}
}


/*====================
  CClientConnection::SendStateBlocks

  Send over all state blocks that have been modified to the client reliably
  if the data won't fit in this packet, it will be queued for the next frame
  ====================*/
void	CClientConnection::SendStateBlocks()
{
	PROFILE("CClientConnection::SendStateBlocks");

	try
	{
		// Check to see if the packet is already full
		if (m_pktReliable.GetLength() >= m_uiMaxPacketSize)
			return;

		// Make sure there is enough room to actually send something worth while
		uint uiBytesRemaining(m_uiMaxPacketSize - m_pktReliable.GetLength());
		if (uiBytesRemaining < 10)
			return;

		// If a block got fragmented, send the continuation
		if (!m_bufferStateBlock.IsEmpty())
		{
			uint uiBytesToWrite(m_bufferStateBlock.GetLength() - m_uiStateBlockBufferIndex);
			if (uiBytesToWrite + (m_bStateBlockIsCompressed ? 9 : 5) > uiBytesRemaining)
			{
				uiBytesToWrite = MIN(uiBytesToWrite, uiBytesRemaining - 5);
				m_pktReliable << NETCMD_STATE_BLOCK_FRAGMENT << m_unStateBlockFragmentIndex << ushort(uiBytesToWrite);
				m_pktReliable.Write(m_bufferStateBlock.Get(m_uiStateBlockBufferIndex), uiBytesToWrite);
				m_uiStateBlockBufferIndex += uiBytesToWrite;
				return;
			}
			else
			{
				if (m_bStateBlockIsCompressed)
					m_pktReliable << NETCMD_COMPRESSED_STATE_BLOCK_TERMINATION << m_unStateBlockFragmentIndex << ushort(uiBytesToWrite) << m_uiUncompressedStateBlockLength;
				else
					m_pktReliable << NETCMD_STATE_BLOCK_TERMINATION << m_unStateBlockFragmentIndex << ushort(uiBytesToWrite);

				m_pktReliable.Write(m_bufferStateBlock.Get(m_uiStateBlockBufferIndex), uiBytesToWrite);
				m_unStateBlockFragmentIndex = 0;
				m_bufferStateBlock.Clear();
				m_bStateBlockIsCompressed = false;
				m_uiStateBlockBufferIndex = 0;
			}

			uiBytesRemaining = m_uiMaxPacketSize - m_pktReliable.GetLength();
		}

		// Step through the deque of blocks waiting to send
		while (!m_deqStateBlockUpdates.empty() && uiBytesRemaining >= 10)
		{
			try
			{
				// Retrieve the next block
				ushort unIndex(m_deqStateBlockUpdatesID.front());
				const CStateBlock &block(m_deqStateBlockUpdates.front());

				if (block.IsEmpty())
				{
					m_deqStateBlockUpdates.pop_front();
					m_deqStateBlockUpdatesID.pop_front();
					continue;
				}

				Console.Server << _T("Sending state block update: ") << unIndex << newl;

				// Compress block data
				m_bufferStateBlock.Clear();
				block.AppendToBuffer(m_bufferStateBlock);
				m_uiUncompressedStateBlockLength = m_bufferStateBlock.GetLength();
				if (m_uiUncompressedStateBlockLength >= svr_minStateStringCompressSize && svr_stateStringCompress)
				{
					PROFILE("Stateblock compression");

					byte* pCompressed(NULL);
					uint uiResultLength(CZip::Compress((const byte*)m_bufferStateBlock.Get(), m_uiUncompressedStateBlockLength, pCompressed));
					m_bufferStateBlock.Write(pCompressed, uiResultLength);
					SAFE_DELETE_ARRAY(pCompressed);
					m_bStateBlockIsCompressed = true;
				}

				// If it won't fit in the packet, send a fragment
				if (m_bufferStateBlock.GetLength() + (m_bStateBlockIsCompressed ? 9 : 5) > uiBytesRemaining)
				{
					ushort unBytesToWrite(MIN(m_bufferStateBlock.GetLength(), uiBytesRemaining - 5));
					m_pktReliable << NETCMD_STATE_BLOCK_FRAGMENT << unIndex << unBytesToWrite;
					m_pktReliable.Write(m_bufferStateBlock.Get(), unBytesToWrite);

					m_uiStateBlockBufferIndex = unBytesToWrite;
					m_unStateBlockFragmentIndex = unIndex;

					m_deqStateBlockUpdates.pop_front();
					m_deqStateBlockUpdatesID.pop_front();

					break;
				}
					
				// Send the complete block
				if (m_bStateBlockIsCompressed)
					m_pktReliable << NETCMD_COMPRESSED_STATE_BLOCK << unIndex << ushort(m_bufferStateBlock.GetLength()) << m_uiUncompressedStateBlockLength << m_bufferStateBlock;
				else
					m_pktReliable << NETCMD_STATE_BLOCK << unIndex << ushort(m_bufferStateBlock.GetLength()) << m_bufferStateBlock;

				m_bufferStateBlock.Clear();
				m_uiStateBlockBufferIndex = 0;
				m_bStateBlockIsCompressed = false;

				m_deqStateBlockUpdates.pop_front();
				m_deqStateBlockUpdatesID.pop_front();

				uiBytesRemaining = m_uiMaxPacketSize - m_pktReliable.GetLength();
			}
			catch (CException &ex)
			{
				ex.Process(_T("CClientConnection::SendStateBlocks() - "), NO_THROW);
				m_deqStateBlockUpdates.pop_front();
				m_deqStateBlockUpdatesID.pop_front();
				continue;
			}
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientConnection::SendStateBlocks() - "), NO_THROW);
	}
}


/*====================
  CClientConnection::WriteClientPackets
  ====================*/
void	CClientConnection::WriteClientPackets(uint uiFPS)
{
	GAME_PROFILE(_T("CClientConnection::WriteClientPackets"));

	switch (m_eConnectionState)
	{
	case CLIENT_CONNECTION_STATE_DISCONNECTED:
		break;

	case CLIENT_CONNECTION_STATE_CONNECTING:
		SendStateStrings();
		SendStateBlocks();

		if (m_iAccountID != -1 &&
			!m_bAllStateDataSent &&
			m_deqStateStringUpdates.empty() && m_unStateStringFragmentIndex == 0 &&
			m_deqStateBlockUpdates.empty() && m_unStateBlockFragmentIndex == 0)
		{
			m_pktReliable << NETCMD_END_STATE_DATA;
			Console.Server << _T("Finished sending state data to client #") << m_iClientNum << newl;
			m_bAllStateDataSent = true;
		}
		break;

	case CLIENT_CONNECTION_STATE_READY:
		SendStateStrings();
		SendStateBlocks();

		if (m_pHostServer->GetWorld()->IsLoaded())
		{
			if (!m_bWorldLoaded)
			{
				SendLoadWorldRequest();
			}
			else
			{
				WriteSnapshotFragments();
				WriteSnapshot(m_pHostServer->GetCurrentSnapshot());
			}
		}
		break;

	case CLIENT_CONNECTION_STATE_STANDBY:
		break;

	case CLIENT_CONNECTION_STATE_IN_GAME:
		SendStateStrings();
		SendStateBlocks();

		WriteSnapshotFragments();
		WriteSnapshot(m_pHostServer->GetCurrentSnapshot());
		break;

	default:
		Disconnect(_T("disconnect_server_error"));
		return;
	}

	SendPackets(uiFPS);
}


/*====================
  CClientConnection::WriteSnapshotFragments

  If there is a fragmented snapshot with parts left to send, send as much as possible
  ====================*/
void	CClientConnection::WriteSnapshotFragments()
{
	PROFILE("CClientConnection::WriteSnapshotFragments");

	// Don't exceeded client's bandwidth
	if (!BandwidthOK())
	{
		if (m_uiLastBandwidthWarn == INVALID_TIME || Host.GetSystemTime() > m_uiLastBandwidthWarn + svr_bandwidthWarnTime)
		{
			Console.Warn << _T("Bandwidth exceeded to client #") << m_iClientNum << newl;
			m_uiLastBandwidthWarn = Host.GetSystemTime();
		}
		return;
	}

	if (m_cBufferSnapshot.IsEmpty())
		return;

	// Make sure the amount of space left in this packet is worth while
	if (m_pktSend.GetRemainingSpace() < 10)
		return;

	uint uiBytesToWrite(m_cBufferSnapshot.GetLength() - m_uiSnapshotBufferIndex);
	if (uiBytesToWrite + SNAPSHOT_TERMINATION_HEADER_SIZE > m_uiMaxPacketSize - m_pktSend.GetLength())
	{
		// Send another fragment
		uiBytesToWrite = MIN(uiBytesToWrite, m_uiMaxPacketSize - m_pktSend.GetLength() - SNAPSHOT_FRAGMENT_HEADER_SIZE);

		m_pktSend << NETCMD_SNAPSHOT_FRAGMENT << m_uiSnapshotFragmentFrame << m_ySnapshotFragmentIndex;
		m_pktSend.Write(m_cBufferSnapshot.Get(m_uiSnapshotBufferIndex), uiBytesToWrite);
		m_uiSnapshotBufferIndex += uiBytesToWrite;

		assert(m_uiSnapshotBufferIndex <= m_cBufferSnapshot.GetLength());

		++m_ySnapshotFragmentIndex;
		if (m_ySnapshotFragmentIndex == 255)
		{
			Disconnect(_T("disconnect_snapshot_fragment"));
			return;
		}
	}
	else
	{
		// Send the termination and clear the buffer so we start fresh next frame
		if (m_bSnapshotIsCompressed)
			m_pktSend << NETCMD_COMPRESSED_SNAPSHOT_TERMINATION << m_uiSnapshotFragmentFrame << m_ySnapshotFragmentIndex << m_uiUncompressedSnapshotLength << ushort(uiBytesToWrite);
		else
			m_pktSend << NETCMD_SNAPSHOT_TERMINATION << m_uiSnapshotFragmentFrame << m_ySnapshotFragmentIndex << ushort(uiBytesToWrite);

		if (uiBytesToWrite > 0)
			m_pktSend.Write(m_cBufferSnapshot.Get(m_uiSnapshotBufferIndex), uiBytesToWrite);

		m_cBufferSnapshot.Clear();
		m_uiSnapshotBufferIndex = 0;
		m_uiSnapshotFragmentFrame = 0;
		m_ySnapshotFragmentIndex = 0;
		m_bSnapshotIsCompressed = false;
		m_uiUncompressedSnapshotLength = 0;
	}
}


CVAR_BOOL(_snapshot_error, false);

/*====================
  CClientConnection::WriteSnapshot
  ====================*/
void	CClientConnection::WriteSnapshot(PoolHandle hSnapshot)
{
	GAME_PROFILE(_T("CClientConnection::WriteSnapshot"));

	const CSnapshot &cSnapshot(*CSnapshot::GetByHandle(hSnapshot));

	// Don't exceeded client's bandwidth
	if (!BandwidthOK())
	{
		if (m_uiLastBandwidthWarn == INVALID_TIME || Host.GetSystemTime() > m_uiLastBandwidthWarn + svr_bandwidthWarnTime)
		{
			Console.Warn << _T("Bandwidth exceeded to client #") << m_iClientNum << newl;
			m_uiLastBandwidthWarn = Host.GetSystemTime();
		}
		return;
	}

	// Don't start a new frame if fragments are still being sent
	if (!m_cBufferSnapshot.IsEmpty())
		return;

	// Make sure the amount of space left in this packet is worth while
	if (m_pktSend.GetRemainingSpace() < 10)
		return;

#if 0
	// Make sure the client is still synched up
	if (cSnapshot.GetFrameNumber() < m_uiLastAckedServerFrame)
	{
		Console.Warn << _T("Negative frame difference") << newl;
		return;
	}
#endif

	if (!m_bFirstAckReceived &&
		m_hLastAckedSnapshot == INVALID_POOL_HANDLE &&
		m_bFirstSnapshotSent &&
		Host.GetTime() - m_uiFirstSnapshotSendTime < svr_firstSnapshotRetryInterval)
	{
		//Console.Dev << "Waiting for first client ack..." << newl;
		return;
	}

	if (m_bFirstAckReceived && m_hLastAckedSnapshot != INVALID_POOL_HANDLE)
	{
		CSnapshot *pLastAckedSnapshot(CSnapshot::GetByHandle(m_hLastAckedSnapshot));

		if (_snapshot_error)
			pLastAckedSnapshot->SetFrameNumber(0);

		// Write snapshot diff
		cSnapshot.WriteDiff(m_cBufferSnapshot, *pLastAckedSnapshot, m_yStateSequence, m_uiStream, m_uiLastAckedStream);
	}
	else
	{
		cSnapshot.WriteBuffer(m_cBufferSnapshot, m_yStateSequence, m_uiStream);

		if (m_hFirstSnapshot == INVALID_POOL_HANDLE)
		{
			m_hFirstSnapshot = hSnapshot;
			CSnapshot::AddRefToHandle(hSnapshot);
		}
		else
		{
			SAFE_DELETE_SNAPSHOT(m_hRetryFirstSnapshot);
			m_hRetryFirstSnapshot = hSnapshot;
			CSnapshot::AddRefToHandle(hSnapshot);
		}

		m_uiFirstSnapshotSendTime = Host.GetTime();
		m_bFirstSnapshotSent = true;
	}

	// Compress snapshot data
	m_uiUncompressedSnapshotLength = m_cBufferSnapshot.GetLength();
	if (m_uiUncompressedSnapshotLength >= svr_minSnapshotCompressSize && /*svr_snapshotCompress*/false) // Way too slow...
	{
		PROFILE("Snapshot compression");

		byte* pCompressed(NULL);
		uint uiResultLength(CZip::Compress((const byte*)m_cBufferSnapshot.Get(), m_uiUncompressedSnapshotLength, pCompressed));
		m_cBufferSnapshot.Write(pCompressed, uiResultLength);
		SAFE_DELETE_ARRAY(pCompressed);
		m_bSnapshotIsCompressed = true;
	}
	//Console << _T("Snapshot [") << currentSnapshot.GetFrameNumber() << _T("] ") << m_uiUncompressedSnapshotLength << newl;

	// Write the snapshot to the send packet, breaking it up if necessary
	if (m_cBufferSnapshot.GetLength() + SNAPSHOT_HEADER_SIZE > m_uiMaxPacketSize - m_pktSend.GetLength())
	{
		m_uiSnapshotFragmentFrame = cSnapshot.GetFrameNumber();
		uint uiBytesToWrite(MIN(m_cBufferSnapshot.GetLength(), m_uiMaxPacketSize - m_pktSend.GetLength() - SNAPSHOT_FRAGMENT_HEADER_SIZE));

		m_pktSend << NETCMD_SNAPSHOT_FRAGMENT << m_uiSnapshotFragmentFrame << m_ySnapshotFragmentIndex;
		m_pktSend.Write(m_cBufferSnapshot.Get(), uiBytesToWrite);

		m_cBufferSnapshot.Seek(uiBytesToWrite);
		m_uiSnapshotBufferIndex = uiBytesToWrite;
		m_ySnapshotFragmentIndex = 1;
	}
	else
	{
		if (m_bSnapshotIsCompressed)
			m_pktSend << NETCMD_COMPRESSED_SNAPSHOT << m_cBufferSnapshot.GetLength() << m_uiUncompressedSnapshotLength << m_cBufferSnapshot;
		else
			m_pktSend << NETCMD_SNAPSHOT << m_cBufferSnapshot.GetLength() << m_cBufferSnapshot;

		m_cBufferSnapshot.Clear();
		m_uiSnapshotBufferIndex = 0;
		m_uiSnapshotFragmentFrame = 0;
		m_ySnapshotFragmentIndex = 0;
		m_bSnapshotIsCompressed = false;
		m_uiUncompressedSnapshotLength = 0;
	}

	m_bSnapshotSent = true;

	m_deqSentSnapshots.push_back(SSentSnapshots(cSnapshot.GetFrameNumber(), m_uiStream));
}


/*====================
  CClientConnection::CheckPort
  ====================*/
void	CClientConnection::CheckPort(ushort unPort)
{
	if (unPort != m_unPort)
	{
		Console.Net << _T("Client #") << m_iClientNum << _T(" port change from ") << m_unPort << _T(" to ") << unPort << newl;
		m_unPort = unPort;

		m_sockGame.SetSendPort(m_unPort);
	}
}


/*====================
  CClientConnection::BandwidthOK
  ====================*/
bool	CClientConnection::BandwidthOK()
{
	size_t zTotal(0);
	for (zdeque_it it(m_deqSendSize.begin()); it != m_deqSendSize.end(); ++it)
		zTotal += *it;

	if (zTotal > m_uiMaxBPS)
		return false;

	return true;
}


#ifndef K2_CLIENT
/*====================
  CClientConnection::ConverseWithMasterServer
  ====================*/
void	CClientConnection::ConverseWithMasterServer()
{
	if (HasFlags(CLIENT_CONNECTION_PSEUDO))
		return;

	Authenticate();
	ValidateMatchKey();

	if (m_iAccountID != -1 && m_sMatchKey.empty() && !HasFlags(CLIENT_CONNECTION_IN_GAME))
	{
		if (!HasFlags(CLIENT_CONNECTION_GAME_HOST) &&
			m_pHostServer->GetServerAccess() == ACCESS_INVITEONLY &&
			!(m_pHostServer->ValidateInvitation(m_iAccountID) || 
			m_pHostServer->ValidateInvitation(m_sName)))
		{
			Disconnect(_T("disconnect_not_invited"));
			return;
		}

		if (int(m_pHostServer->GetClientKickCount(m_iAccountID)) >= svr_kickBanCount)
		{
			Disconnect(_T("disconnect_banned"));
			return;
		}

		if (HasFlags(CLIENT_CONNECTION_LOCAL) && !m_sockGame.IsLocalConnection())
		{
			Disconnect(_T("disconnect_invalid_connection"));
			return;
		}

		m_pHostServer->GetGameLib().AddClient(this);
	}

	if (m_bRefreshUpgrades &&
		(m_uiLastRefreshUpgradesTime == INVALID_TIME || Host.GetSystemTime() > m_uiLastRefreshUpgradesTime + svr_clientRefreshUpgradesThrottle))
	{
		m_bRefreshUpgrades = false;
		m_uiLastRefreshUpgradesTime = Host.GetSystemTime();

		m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
		m_pRefreshUpgradesRequest = m_pHTTPManager->SpawnRequest();
		if (m_pRefreshUpgradesRequest != NULL)
		{
			m_pRefreshUpgradesRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
			m_pRefreshUpgradesRequest->AddVariable(L"f", L"get_upgrades");
			m_pRefreshUpgradesRequest->AddVariable(L"cookie", m_sCookie);
			m_pRefreshUpgradesRequest->SendPostRequest();
		}
	}

	if (m_pRefreshUpgradesRequest != NULL && !m_pRefreshUpgradesRequest->IsActive())
	{
		if (m_pRefreshUpgradesRequest->WasSuccessful())
		{
			CPHPData phpResponse(m_pRefreshUpgradesRequest->GetResponse());

			const CPHPData *pMyUpgrades(phpResponse.GetVar(_T("my_upgrades")));
			if (pMyUpgrades != NULL)
			{
				uint uiNum(0);
				const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

				while (pUpgrade != NULL)
				{
					m_setAvailableUpgrades.insert(pUpgrade->GetString());
					pUpgrade = pMyUpgrades->GetVar(uiNum++);
				}
			}

			const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
			if (pSelectedUpgrades != NULL)
			{
				uint uiNum(0);
				const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

				while (pUpgrade != NULL)
				{
					tstring sType(Upgrade_GetType(pUpgrade->GetString()));

					m_mapSelectedUpgrades[sType] = pUpgrade->GetString();
					pUpgrade = pSelectedUpgrades->GetVar(uiNum++);
				}
			}

			m_pHostServer->GetGameLib().UpdateUpgrades(m_iClientNum);
		}

		m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
		m_pRefreshUpgradesRequest = NULL;
	}

	if (m_pRecentMatchStatsRequest != NULL && !m_pRecentMatchStatsRequest->IsActive())
	{
		if (m_pRecentMatchStatsRequest->WasSuccessful())
		{
			CPHPData phpResponse(m_pRecentMatchStatsRequest->GetResponse());

			const CPHPData *pInfos(phpResponse.GetVar(_T("infos")));

			// Grab infos
			if (pInfos != NULL && pInfos->GetVar(_T("error")) == NULL)
				pInfos = pInfos->GetVar(0);

			m_pHostServer->ProcessAuxData(GetClientNum(), pInfos);
		}

		m_pHTTPManager->ReleaseRequest(m_pRecentMatchStatsRequest);
		m_pRecentMatchStatsRequest = NULL;
	}
}
#endif


#ifndef K2_CLIENT
/*====================
  CClientConnection::SendAuthRequest
  ====================*/
void	CClientConnection::SendAuthRequest()
{
	if (HasFlags(CLIENT_CONNECTION_AUTH_REQUESTED))
		return;

	m_pHTTPManager->ReleaseRequest(m_pAuthenticateRequest);
	m_pAuthenticateRequest = m_pHTTPManager->SpawnRequest();
	if (m_pAuthenticateRequest == NULL)
		return;

	SetFlags(CLIENT_CONNECTION_AUTH_REQUESTED);
	m_uiAuthRequestTime = Host.GetTime();

	m_pAuthenticateRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
	m_pAuthenticateRequest->AddVariable(L"f", L"c_conn");
	m_pAuthenticateRequest->AddVariable(L"session", m_pHostServer->GetSessionCookie());
	m_pAuthenticateRequest->AddVariable(L"cookie", GetCookie());
	m_pAuthenticateRequest->AddVariable(L"ip", GetPublicAddress());
	m_pAuthenticateRequest->AddVariable(L"cas", m_pHostServer->GetGameLib().GetGameInfoString(L"IsCasual"));

	if (m_pHostServer->IsArrangedMatch())
		m_pAuthenticateRequest->AddVariable(L"new", 2);
	else if (m_pHostServer->IsTournMatch())
		m_pAuthenticateRequest->AddVariable(L"new", 3);
	else if (m_pHostServer->IsLeagueMatch())
		m_pAuthenticateRequest->AddVariable(L"new", 4);
	else
		m_pAuthenticateRequest->AddVariable(L"new", 1);	

	m_pAuthenticateRequest->SendPostRequest();
}
#endif

#ifndef K2_CLIENT
/*====================
  CClientConnection::CheckAuthResult
  ====================*/
void	CClientConnection::CheckAuthResult()
{
	// Ignore if no request has been made
	if (!HasFlags(CLIENT_CONNECTION_AUTH_REQUESTED))
		return;

	// Wait for a result
	if (m_pAuthenticateRequest->IsActive() && Host.GetTime() < m_uiAuthRequestTime + svr_authTimeout)
		return;

	RemoveFlags(CLIENT_CONNECTION_AUTH_REQUESTED);

	if (!m_pAuthenticateRequest->WasSuccessful() && m_iAccountID > -1)
	{
		Console.Warn << _T("Re-auth request for ") << SingleQuoteStr(m_sName) << _T(" failed") << newl;
		m_uiLastAuthSuccess = Host.GetTime();
		m_pHTTPManager->ReleaseRequest(m_pAuthenticateRequest);
		m_pAuthenticateRequest = NULL;
		return;
	}

	// Read response
	const wstring &sResponse(m_pAuthenticateRequest->GetResponse());
	Console << _T("Auth response for client #") << m_iClientNum << _T(": ") << sResponse << newl;
	CPHPData phpResponse(sResponse);

	m_pHTTPManager->ReleaseRequest(m_pAuthenticateRequest);
	m_pAuthenticateRequest = NULL;

	// Check for a valid account id
	int iAccountID(phpResponse.GetInteger(_T("account_id"), -1));

	if (iAccountID == -1 || (m_iAccountID > -1 && m_iAccountID != iAccountID))
	{
		Disconnect(_T("disconnect_auth_failed"));
		return;
	}

	m_uiLastAuthSuccess = Host.GetTime();

	RemoveFlags(CLIENT_CONNECTION_STAFF | CLIENT_CONNECTION_PREMIUM | CLIENT_CONNECTION_TRIAL);

	int iLevel(0);
	int iRank(0);
	int iMatches(0);
	int iDisconnects(0);
	tstring sTag(phpResponse.GetString(_T("tag")));
	m_sGameCookie = phpResponse.GetString(_T("game_cookie"), TSNULL);

	if (m_sGameCookie == TSNULL)
	{
		Disconnect(_T("disconnect_unique_login_failed"));
		return;
	}

	// Grab infos
	const CPHPData *pInfos(phpResponse.GetVar(_T("infos")));
	if (pInfos != NULL && pInfos->GetVar(_T("error")) == NULL)
	{
		if (sTag.empty())
			sTag = pInfos->GetString(_T("tag"));

		pInfos = pInfos->GetVar(0);

		if (pInfos != NULL)
		{
			iLevel = pInfos->GetInteger(_T("level"));
			iRank = pInfos->GetInteger(_T("acc_pub_skill"));
			iMatches = pInfos->GetInteger(_T("acc_games_played"), 0) + pInfos->GetInteger(_T("rnk_games_played"), 0) + pInfos->GetInteger(_T("cs_games_played"), 0);
			iDisconnects = pInfos->GetInteger(_T("acc_discos"), 0) + pInfos->GetInteger(_T("rnk_discos"), 0) + pInfos->GetInteger(_T("cs_discos"), 0);
			m_iClanID = pInfos->GetInteger(_T("clan_id"));

			if (sTag.empty())
				sTag = pInfos->GetString(_T("tag"));
		}
	}

	uint uiAccountType(phpResponse.GetInteger(_T("account_type")));
	if (uiAccountType == 5)
		SetFlags(CLIENT_CONNECTION_STAFF);
	else if (uiAccountType == 4)
		SetFlags(CLIENT_CONNECTION_PREMIUM);
	else if (uiAccountType == 1)
		SetFlags(CLIENT_CONNECTION_TRIAL);

	// MikeG Trial Account Check Server
	if (uiAccountType == 1)
	{
		int iTrialStatus(phpResponse.GetInteger(L"trial", 0));
		int iTrialGamesPlayed(pInfos->GetInteger(L"acc_trial_games_played", 0));

		if ( (iTrialStatus == 2 || iTrialGamesPlayed >= MAX_TRIAL_GAMES) && !m_pHostServer->GetGameLib().IsPlayerReconnecting(iAccountID) )
		{
			Disconnect(_T("disconnect_trial_expired"));
			return;
		}
	}

	m_pHostServer->ProcessAuthData(iAccountID, &phpResponse);

	tstring sNickname;

	if (sTag.empty())
		sNickname = phpResponse.GetString(_T("nickname"));
	else
		sNickname = _T("[") + sTag + _T("]") + phpResponse.GetString(_T("nickname"));

	const CPHPData *pMyUpgrades(phpResponse.GetVar(_T("my_upgrades")));
	if (pMyUpgrades != NULL)
	{
		uint uiNum(0);
		const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

		while (pUpgrade != NULL)
		{
			m_setAvailableUpgrades.insert(pUpgrade->GetString());
			pUpgrade = pMyUpgrades->GetVar(uiNum++);
		}
	}

	const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
	if (pSelectedUpgrades != NULL)
	{
		uint uiNum(0);
		const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

		while (pUpgrade != NULL)
		{
			tstring sType(Upgrade_GetType(pUpgrade->GetString()));

			m_mapSelectedUpgrades[sType] = pUpgrade->GetString();
			pUpgrade = pSelectedUpgrades->GetVar(uiNum++);
		}
	}
		
	// Check to see if there are two clients connected with the same account, if so disconnect the oldest client and allow the newest client to connect
	if (m_pHostServer->GetGameLib().IsDuplicateAccountInGame(iAccountID))
	{
		m_pHostServer->GetGameLib().RemoveDuplicateAccountsInGame(iAccountID);
	}	
	
	// bypass any attempt to kick a client if they are reconnecting, this fixes players unable to rejoin due to changed PSR or leaver status
	if (!m_pHostServer->GetGameLib().IsPlayerReconnecting(iAccountID))
	{		
		// check to see if any min or max PSR restriction exists only if they don't already have an invitation
		// setup PSR variation tolerances up to 2% from the set min/max PSR
		if(!(m_pHostServer->ValidateInvitation(m_iAccountID) || m_pHostServer->ValidateInvitation(LowerString(sNickname))))
		{
			if (!m_pHostServer->IsValidPSR(iRank) && !m_pHostServer->IsArrangedMatch())
			{	
				Disconnect(_T("disconnect_invalid_psr")); 
				return;
			}
		}

		if (m_pHostServer->GetNoLeaver() && m_pHostServer->IsLeaver(iMatches != 0 ? float(iDisconnects) / iMatches : 0.0f, iMatches))
		{
			Disconnect(_T("disconnect_leaver"));
			return;
		}
	}

	m_pHTTPManager->ReleaseRequest(m_pRecentMatchStatsRequest);
	m_pRecentMatchStatsRequest = m_pHTTPManager->SpawnRequest();
	if (m_pRecentMatchStatsRequest != NULL)
	{
		m_pRecentMatchStatsRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
		m_pRecentMatchStatsRequest->AddVariable(L"f", L"get_quickstats");
		m_pRecentMatchStatsRequest->AddVariable(L"session", m_pHostServer->GetSessionCookie());
		m_pRecentMatchStatsRequest->AddVariable(L"account_id", iAccountID);
		m_pRecentMatchStatsRequest->SendPostRequest();
	}

	// Notify client that they have been accepted
	AuthSuccess(iAccountID, sNickname);
}
#endif


#ifndef K2_CLIENT
/*====================
  CClientConnection::Authenticate
  ====================*/
void	CClientConnection::Authenticate()
{
	if (!IsConnected())
		return;

	// Check if this connection requires authorization
	if (m_pHostServer->GetPractice() || !svr_requireAuthentication ||
		(HasFlags(CLIENT_CONNECTION_LOCAL) && (GetCookie().empty() || m_pHostServer->GetSessionCookie().empty())) ||
		Host.IsReplay())
	{
		// Assign a random fake ID if they don't have one yet
		if (m_iAccountID == -1)
			AuthSuccess(-2 - m_unConnectionID, m_sName);
		return;
	}

	// An empty cookie is an automatic failure
	if (GetCookie().empty())
	{
		Disconnect(_T("disconnect_no_cookie"));
		return;
	}

	// If the client has authed alredy, just recheck their cookie periodically
	if (m_uiLastAuthSuccess == INVALID_TIME)
		SendAuthRequest();

	CheckAuthResult();
}
#endif


/*====================
  CClientConnection::AuthSuccess
  ====================*/
void	CClientConnection::AuthSuccess(int iAccountID, const tstring &sName)
{
	if (HasFlags(CLIENT_CONNECTION_GAME_HOST))
		m_pktReliable << NETCMD_GAME_HOST << byte(1);

	m_iAccountID = iAccountID;
	m_sName = sName;
	m_iClientNum = m_pHostServer->GenerateClientID(m_unConnectionID);

	if ((m_pHostServer->IsArrangedMatch() || m_pHostServer->IsTournMatch()) && !m_pHostServer->IsOnRoster(iAccountID))
	{
		Disconnect(_T("disconnect_not_on_roster"));
		return;
	}

	byte yFlags(0);
	if (HasFlags(CLIENT_CONNECTION_LOCAL))
		yFlags |= BIT(0);

	tstring sVersion(FileManager.GetCompatVersion());
	if (sVersion.empty())
		sVersion = K2System.GetVersionString();

	m_pktReliable << NETCMD_AUTH_OKAY
		<< m_unConnectionID
		<< m_iClientNum
		<< yFlags
		<< sVersion;

	if (m_pHostServer->GetWorld()->IsLoaded())
		m_pktReliable << NETCMD_START_LOADING;

	SendAllStateData();
}


#ifndef K2_CLIENT
/*====================
  CClientConnection::ValidateMatchKey
  ====================*/
void	CClientConnection::ValidateMatchKey()
{
	if (!IsConnected() || GetMatchKey().empty())
		return;

	if (!HasFlags(CLIENT_CONNECTION_KEY_VALIDATE_REQUESTED))
	{
		m_pHTTPManager->ReleaseRequest(m_pValidateMatchKeyRequest);
		m_pValidateMatchKeyRequest = m_pHTTPManager->SpawnRequest();
		if (m_pValidateMatchKeyRequest != NULL)
		{
			m_pValidateMatchKeyRequest->SetTargetURL(m_pHostServer->GetMasterServerURL());
			m_pValidateMatchKeyRequest->AddVariable(L"f", L"accept_key");
			m_pValidateMatchKeyRequest->AddVariable(L"session", m_pHostServer->GetSessionCookie());
			m_pValidateMatchKeyRequest->AddVariable(L"acc_key", GetMatchKey());
			m_pValidateMatchKeyRequest->SendPostRequest();
			SetFlags(CLIENT_CONNECTION_KEY_VALIDATE_REQUESTED);
		}
	}

	if (m_pValidateMatchKeyRequest == NULL || m_pValidateMatchKeyRequest->IsActive())
		return;

	if (m_pValidateMatchKeyRequest->WasSuccessful())
	{
		CPHPData phpResponse(m_pValidateMatchKeyRequest->GetResponse());
		const CPHPData *pServerID(phpResponse.GetVar(_T("server_id")));
		if (pServerID == NULL)
		{
			Console << _T("Could not validate match key") << newl;
		}
		else
		{
			Console << _T("Client #") << m_iClientNum << _T(" promoted to host") << newl;
			SetFlags(CLIENT_CONNECTION_GAME_HOST);

			m_pktReliable << NETCMD_GAME_HOST << byte(1);
		}
	}

	m_pHTTPManager->ReleaseRequest(m_pValidateMatchKeyRequest);
	m_pValidateMatchKeyRequest = NULL;
	ClearMatchKey();
}
#endif


#ifndef K2_CLIENT
/*====================
  CClientConnection::RefreshUpgrades
  ====================*/
void	CClientConnection::RefreshUpgrades()
{
	if (!IsConnected())
		return;

	m_bRefreshUpgrades = true;
}
#endif


/*====================
  CClientConnection::Disconnect
  ====================*/
void	CClientConnection::Disconnect(const tstring &sReason)
{
	m_sockGame.ClearReliablePackets();
	m_pktReliable.Clear();
	m_pktSend.Clear();

	// Clear pending state string fragments
	m_deqStateStringUpdatesID.clear();
	m_deqStateStringUpdates.clear();
	m_unStateStringFragmentIndex = 0;
	m_uiUncompressedStateStringLength = 0;
	m_bufferStateString.Clear();
	m_uiStateStringBufferIndex = 0;
	
	m_deqStateBlockUpdatesID.clear();
	m_deqStateBlockUpdates.clear();
	m_unStateBlockFragmentIndex = 0;
	m_uiUncompressedStateBlockLength = 0;
	m_bufferStateBlock.Clear();
	m_uiStateBlockBufferIndex = 0;
	m_vStateBlockModCounts.clear();

	m_bAllStateDataSent = false;
	m_bFirstSnapshotSent = false;
	m_bFirstAckReceived = false;
	m_bWorldLoaded = false;

#ifndef K2_CLIENT
	m_pHTTPManager->ReleaseRequest(m_pAuthenticateRequest);
	m_pAuthenticateRequest = NULL;
	m_pHTTPManager->ReleaseRequest(m_pValidateMatchKeyRequest);
	m_pValidateMatchKeyRequest = NULL;
#endif

	if (m_eConnectionState == CLIENT_CONNECTION_STATE_DISCONNECTED)
		return;

	Console << _T("Client #") << m_iClientNum << _T(" disconnected: ") << sReason << newl;

	CPacket pkt;
	pkt << NETCMD_KICK << sReason;
	SendPacket(pkt);

	m_eConnectionState = CLIENT_CONNECTION_STATE_DISCONNECTED;

	VoiceServer.RemoveVoiceClient(this);

	m_bRefreshUpgrades = false;
	m_uiLastRefreshUpgradesTime = INVALID_TIME;
}


/*====================
  CClientConnection::ResynchStateData
  ====================*/
void	CClientConnection::ResynchStateData()
{
	m_eConnectionState = CLIENT_CONNECTION_STATE_CONNECTING;
	m_bAllStateDataSent = false;
	m_uiLastReceiveTime = INVALID_TIME;

	m_pktReliable << NETCMD_START_LOADING;
}


/*====================
  CClientConnection::ReadRemoteCommandPacket
  ====================*/
bool	CClientConnection::ReadRemoteCommandPacket(CPacket &pkt)
{
	wstring sCommand(pkt.ReadWString());

	if (!HasFlags(CLIENT_CONNECTION_ADMIN) || sCommand.empty())
		return !pkt.HasFaults();

	try
	{
		Console << L"Remote<" << GetName() << L">: " << sCommand << newl;

		Console.StartWatch();

		Console.Execute(sCommand);

		const tstring &sWatch(Console.GetWatchBuffer());
		if (!sWatch.empty() && m_pktReliable.GetLength() + sWatch.length() < MAX_MTU_SIZE)
			m_pktReliable << NETCMD_CONSOLE_MESSAGE << sWatch;
		else
			m_pktReliable << NETCMD_CONSOLE_MESSAGE << _TS("Overflow\n");

		Console.EndWatch();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientConnection::ReadRemoteCommandPacket() - "), NO_THROW);
	}

	return !pkt.HasFaults();
}


/*====================
  CClientConnection::ReadClientSnapshot
  ====================*/
bool	CClientConnection::ReadClientSnapshot(CPacket &pkt)
{
	PROFILE_EX("CClientConnection::ReadClientSnapshot", PROFILE_CLIENTSNAPSHOT);

	try
	{
		if (!m_pHostServer->GetWorld()->IsLoaded())
			EX_WARN(_T("Received a client snapshot before world was loaded"));

		// Construct a snapshot from the packet
		CClientSnapshot snapshot(pkt);

		if (snapshot.GetServerFrame() < m_uiLastAckedServerFrame && m_uiLastAckedServerFrame != -1)
			return true;

		// Set the button states from the last saved snapshot
		snapshot.SetPrevButtonStates(m_uiPrevButtonStates);
		m_uiPrevButtonStates = snapshot.GetButtons();
		snapshot.SetFrameLength(MAX(int(snapshot.GetTimeStamp() - m_uiLastReceivedClientTime), 0));
		
		if (!HasFlags(CLIENT_CONNECTION_PSEUDO))
		{
			// Pass the snapshot to the game code
			Console.SetDefaultStream(Console.ServerGame);
			m_pHostServer->GetGameLib().ProcessClientSnapshot(m_iClientNum, snapshot);
			Console.SetDefaultStream(Console.Server);
		}

		// Synchronize client/server frame acks
		m_uiLastAckedServerFrame = snapshot.GetServerFrame();
		m_uiLastReceivedClientTime = snapshot.GetTimeStamp();

		if (!m_bFirstAckReceived)
		{
			if (m_hFirstSnapshot == INVALID_POOL_HANDLE)
				return true; // haven't tried to send first snapshot yet

			if (m_uiLastAckedServerFrame == CSnapshot::GetByHandle(m_hFirstSnapshot)->GetFrameNumber())
			{
				m_hLastAckedSnapshot = m_hFirstSnapshot;
				m_hFirstSnapshot = INVALID_POOL_HANDLE;
				SAFE_DELETE_SNAPSHOT(m_hRetryFirstSnapshot);
			}
			else if (m_hRetryFirstSnapshot != INVALID_POOL_HANDLE && m_uiLastAckedServerFrame == CSnapshot::GetByHandle(m_hRetryFirstSnapshot)->GetFrameNumber())
			{
				m_hLastAckedSnapshot = m_hRetryFirstSnapshot;
				m_hRetryFirstSnapshot = INVALID_POOL_HANDLE;
				SAFE_DELETE_SNAPSHOT(m_hFirstSnapshot);
			}
			else
			{
				// If we acked a newer frame than the first snapshot, throw our saved one away
				if (m_uiLastAckedServerFrame > CSnapshot::GetByHandle(m_hFirstSnapshot)->GetFrameNumber())
				{
					SAFE_DELETE_SNAPSHOT(m_hFirstSnapshot);
					m_hFirstSnapshot = m_hRetryFirstSnapshot;
					m_hRetryFirstSnapshot = INVALID_POOL_HANDLE;
				}

				//Console.Warn << _T("First Ack didn't match first snapshot") << newl;
				return true;
			}

			m_bFirstAckReceived = true;
		}
		else if (m_hLastAckedSnapshot == INVALID_POOL_HANDLE || CSnapshot::GetByHandle(m_hLastAckedSnapshot)->GetFrameNumber() != m_uiLastAckedServerFrame)
		{
			const vector<PoolHandle> &vSnapshots(m_pHostServer->GetSnapshotBuffer());
			vector<PoolHandle>::const_iterator it(vSnapshots.begin());
			for (; it != vSnapshots.end(); ++it)
			{
				if (*it == INVALID_POOL_HANDLE)
					continue;

				const CSnapshot &cSnapshot(*CSnapshot::GetByHandle(*it));

				if (cSnapshot.IsValid() && cSnapshot.GetFrameNumber() == m_uiLastAckedServerFrame)
				{
					if (m_hLastAckedSnapshot != INVALID_POOL_HANDLE)
						m_unPing = CLAMP(m_pHostServer->GetRealTime() - cSnapshot.GetTimeStamp(), 0u, 999u);

					SAFE_DELETE_SNAPSHOT(m_hLastAckedSnapshot);

					m_hLastAckedSnapshot = *it;
					CSnapshot::AddRefToHandle(*it);
					break;
				}
			}

			if (it == vSnapshots.end() && !m_bBehind)
			{
				Console.Dev << _T("Client #") << m_iClientNum << _T(" has fallen behind") << newl;
				m_bBehind = true;
			}
			else if (it != vSnapshots.end() && m_bBehind)
			{
				Console.Dev << _T("Client #") << m_iClientNum << _T(" has caught up") << newl;
				m_bBehind = false;
			}
		}

		while (!m_deqSentSnapshots.empty() && m_deqSentSnapshots.front().uiFrame < m_uiLastAckedServerFrame)
			m_deqSentSnapshots.pop_front();

		if (!m_deqSentSnapshots.empty() && m_deqSentSnapshots.front().uiFrame == m_uiLastAckedServerFrame)
			m_uiLastAckedStream = m_deqSentSnapshots.front().uiStream;
		else
			Console.Dev << _T("Client #") << m_iClientNum << _T(" acked unsent frame") << newl;

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientConnection::ReadClientSnapshot() - "));
		pkt.Clear();
		return false;
	}
}


/*====================
  CClientConnection::ProcessOutOfSequencePackets
  ====================*/
void	CClientConnection::ProcessOutOfSequencePackets()
{
	CPacket pkt;
	while (m_sockGame.CheckOutOfSequencePacket(pkt) > 0)
		ProcessPacket(pkt);
}


/*====================
  CClientConnection::ProcessPacket
  ====================*/
void	CClientConnection::ProcessPacket(CPacket &pkt)
{
	PROFILE("CClientConnection::ProcessPacket");

	try
	{
		// Check rates
		while (!m_deqIncomingPackets.empty() && Host.GetTime() - m_deqIncomingPackets.front().uiTimeStamp > 1000)
			m_deqIncomingPackets.pop_front();

		PacketRecordDeque_it itEnd(m_deqIncomingPackets.end());
		uint uiBytes(0);
		for (PacketRecordDeque_it it(m_deqIncomingPackets.begin()); it != itEnd; ++it)
			uiBytes += it->uiSize;

		if (m_deqIncomingPackets.size() > svr_maxIncomingPacketsPerSecond || uiBytes > svr_maxIncomingBytesPerSecond)
		{
			//Disconnect(_T("disconnect_flooding"));
			//Console.Warn << _T("Client #") << m_iClientNum << _T(" is flooding") << newl;
			//return;
		}

		m_deqIncomingPackets.push_back(SPacketRecord(Host.GetTime(), pkt.GetLength()));

		if (!m_sockGame.PreProcessPacket(pkt))
			return;

		// Note the time that this packet was received
		m_uiLastReceiveTime = Host.GetSystemTime();

		while (!pkt.DoneReading())
		{
			byte yCmd(pkt.ReadByte());
			switch (yCmd)
			{
			case NETCMD_CLIENT_KEEP_ALIVE:
				m_pktReliable << NETCMD_SERVER_KEEP_ALIVE;
				break;

			case NETCMD_CLIENT_REMOTE_COMMAND:
				if (!ReadRemoteCommandPacket(pkt))
					EX_ERROR(_T("Failed to process NETCMD_CLIENT_MESSAGE"));
				break;

			case NETCMD_CLIENT_SNAPSHOT:
				if (!ReadClientSnapshot(pkt))
					EX_ERROR(_T("Failed to process client snapshot"));
				break;

			case NETCMD_CLIENT_DISCONNECT:
				Disconnect(_T("disconnected"));
				return;

			case NETCMD_CLIENT_READY:
				m_eConnectionState = CLIENT_CONNECTION_STATE_READY;
				break;

			case NETCMD_CLIENT_NET_SETTINGS:
				if (!ProcessNetSettings(pkt))
					EX_ERROR(_T("Failed to process NETCMD_CLIENT_NET_SETTINGS"));
				break;

			case NETCMD_CLIENT_IN_GAME:
				Console << _T("NETCMD_CLIENT_IN_GAME @ ") << Host.GetSystemTime() << newl;
				m_eConnectionState = CLIENT_CONNECTION_STATE_IN_GAME;
#ifndef K2_CLIENT
				VoiceServer.AddVoiceClient(this);
#endif
				m_pHostServer->GetGameLib().ClientStateChange(m_iClientNum, m_eConnectionState);

				break;

			case NETCMD_FINISHED_LOADING_WORLD:
				Console << _T("NETCMD_FINISHED_LOADING_WORLD @ ") << Host.GetSystemTime() << newl;
				m_bWorldLoaded = true;
				m_fLoadingProgress = 1.0f;
				m_eConnectionState = CLIENT_CONNECTION_STATE_READY;
				break;

			case NETCMD_CLIENT_GAME_DATA:
				if (!m_pHostServer->GetGameLib().ProcessGameData(m_iClientNum, pkt))
					EX_ERROR(_T("Failed to process NETCMD_CLIENT_GAME_DATA"));
				break;

			case NETCMD_SET_PRIVATE:
				{
					byte yValue(pkt.ReadByte());

					
					if (HasFlags(CLIENT_CONNECTION_GAME_HOST) && !pkt.HasFaults())
						m_pHostServer->SetServerAccess(EServerAccess(yValue));
				}
				break;

			case NETCMD_LOADING_PROGRESS:
				{
					float fProgress(pkt.ReadFloat());
					if (pkt.HasFaults())
						EX_ERROR(_T("Bad NETCMD_LOADING_PROGRESS message"));

					m_fLoadingProgress = MAX(m_fLoadingProgress, fProgress);
					
					if (m_uiLastLoadingProgressUpdate != INVALID_TIME && Host.GetSystemTime() - m_uiLastLoadingProgressUpdate > m_uiLongestLoadingProgressInterval)
					{
						m_uiLongestLoadingProgressInterval = Host.GetSystemTime() - m_uiLastLoadingProgressUpdate;
						Console.Server << _T("Client #") << m_iClientNum << _T(" longest load interval: ") << m_uiLongestLoadingProgressInterval << newl;
					}
					m_uiLastLoadingProgressUpdate = Host.GetSystemTime();
				}
				break;

			case NETCMD_CLIENT_HEARTBEAT:
				break;

			case NETCMD_CLIENT_INVITE:
				{
					wstring sName(pkt.ReadWString());
					if ((m_pHostServer->GetServerAccess() == ACCESS_PUBLIC || HasFlags(CLIENT_CONNECTION_GAME_HOST)) && !pkt.HasFaults())
					{
						m_pHostServer->InviteUser(sName);
						m_pktReliable << NETCMD_SERVER_INVITE << sName;
					}
				}
				break;
#ifndef K2_CLIENT
			case NETCMD_CLIENT_COOKIE:
				{
					wstring sCookie(pkt.ReadWString());
					SetCookie(sCookie);
				}
				break;
#endif
			default:
				EX_ERROR(_T("Unknown command received: ") + XtoA(yCmd, FMT_PADZERO, 4, 16));
				break;
			}
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientConnection::ProcessPacket() - "), NO_THROW);
	}
}
