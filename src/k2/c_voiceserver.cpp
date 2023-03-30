// (C)2009 S2 Games
// c_voiceserver.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_voiceserver.h"
#include "c_clientconnection.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CVoiceServer)
CVoiceServer& VoiceServer(*CVoiceServer::GetInstance());

CVAR_INTR(		svr_voicePortStart,		DEFAULT_VOICE_PORT,			CVAR_SAVECONFIG,	1024,	65535);
CVAR_INTR(		svr_voicePortEnd,		DEFAULT_VOICE_PORT + 100,	CVAR_SAVECONFIG,	1024,	65535);
CVAR_BOOL(		svr_voiceDisable,		false);
//=============================================================================

/*====================
  CVoiceServer::~CVoiceServer
  ====================*/
CVoiceServer::~CVoiceServer()
{
	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
		SAFE_DELETE(it->second);
}


/*====================
  CVoiceServer::CVoiceServer
  ====================*/
CVoiceServer::CVoiceServer() :
m_sockVoice(_T("VoiceServer")),
m_yNextVoiceID(1),
m_uiBytesReceived(0),
m_uiPacketsReceived(0)
{
}


/*====================
  CVoiceServer::Init
  ====================*/
void	CVoiceServer::Init()
{
	int iPort(svr_voicePortStart);
	bool bSuccess(false);

	while (iPort <= svr_voicePortEnd && !bSuccess)
		bSuccess = m_sockVoice.Init(K2_SOCKET_UDP, iPort++);

	m_uiBytesReceived = 0;
	m_uiPacketsReceived = 0;
}


/*====================
  CVoiceServer::Frame
  ====================*/
void	CVoiceServer::Frame()
{
	if (!m_sockVoice.IsInitialized())
		return;

	CPacket pkt;

	while (m_sockVoice.ReceivePacket(pkt) > 0)
	{
		m_uiBytesReceived += pkt.GetLength();
		++m_uiPacketsReceived;

		while (!pkt.DoneReading())
		{
			byte yVoiceID(pkt.ReadByte());

			if (yVoiceID == 0)
			{
				// Control command, indicating the user is connected and ready for processing
				byte yActualID(pkt.ReadByte());
								
				VoiceClientMap_it findit = m_mapVoiceClients.find(yActualID);

				if (findit == m_mapVoiceClients.end())
					continue;

				if (findit->second->bRecievedData)
				{
					if (m_sockVoice.GetRecvPort() != findit->second->socket.GetSendPort())
					{
						Console.Warn << _T("Voice Server: Voice user ") << yActualID << _T(" has changed ports from ") << findit->second->socket.GetSendPort() << _T(" to ") << m_sockVoice.GetRecvPort() << _T(".") << newl;
						findit->second->socket.SetSendPort(m_sockVoice.GetRecvPort());
					}

					continue;
				}

				findit->second->socket.SetSendAddr(m_sockVoice.GetRecvAddrName(), m_sockVoice.GetRecvPort());
				findit->second->bRecievedData = true;
				
				continue;
			}

			uint uiSequence(pkt.ReadInt());
			byte yTargetSet(pkt.ReadByte());
			byte yLength(pkt.ReadByte());

			if (pkt.GetUnreadLength() < yLength || pkt.HasFaults())
				break;

			if (yTargetSet >= NUM_VOICE_TARGET_SETS)
			{
				pkt.Advance(yLength);
				continue;
			}

			VoiceClientMap_it findit = m_mapVoiceClients.find(yVoiceID);
	
			if (findit == m_mapVoiceClients.end())
			{
				pkt.Advance(yLength);
				continue;
			}

			if (!findit->second->bRecievedData)
			{
				findit->second->socket.SetSendAddr(m_sockVoice.GetRecvAddrName(), m_sockVoice.GetRecvPort());
				findit->second->bRecievedData = true;
			}
			else if (findit->second->sAddress != m_sockVoice.GetRecvAddrName())
			{
				Console.Warn << _T("Voice Server: Voice packet skipped due to mismatched address") << newl;
				pkt.Advance(yLength);
				continue;
			}
			else if (m_sockVoice.GetRecvPort() != findit->second->socket.GetSendPort())
			{
				Console.Warn << _T("Voice Server: Voice user ") << yVoiceID << _T(" has changed ports from ") << findit->second->socket.GetSendPort() << _T(" to ") << m_sockVoice.GetRecvPort() << _T(".") << newl;
				findit->second->socket.SetSendPort(m_sockVoice.GetRecvPort());
			}

			CBufferDynamic buffer;

			buffer << yVoiceID << uiSequence << yLength;

			buffer.Append(pkt.GetBuffer(), yLength);

			pkt.Advance(yLength);

			for (set<byte>::iterator it(findit->second->setTargets[yTargetSet].begin()); it != findit->second->setTargets[yTargetSet].end(); it++)
			{
				VoiceClientMap_it targetit = m_mapVoiceClients.find(*it);

				if (targetit == m_mapVoiceClients.end())
				{
					Console.Warn << _T("Voice Server: Data target not found!") << newl;
					continue;
				}

				if (targetit->second->packet.GetLength() + buffer.GetLength() < MAX_PACKET_SIZE)
					targetit->second->packet << buffer;
				else
					Console.Warn << _T("Voice Server: Max packet length exceeded") << newl;
			}
		}
	}

	// Consolidate all packets for this frame into one submission, otherwise we will end up with ~10 packets out per recieved packet (ouch)
	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		// Check if we've initialized this user's socket yet (Recieved control command)
		if (!it->second->bRecievedData)
		{
			it->second->packet.Clear();
			continue;
		}

		if (it->second->packet.GetLength() > 0)
		{
			it->second->socket.SendPacket(it->second->packet);
			it->second->packet.Clear();
		}
	}
}

/*====================
  CVoiceServer::AddVoiceTarget
  ====================*/
void	CVoiceServer::AddVoiceTarget(int iClientNumToAdd, ivector &vClientsToAddTo, EVoiceTargetSets eTargetSet)
{
	// Find the target's voice ID first
	byte yVoiceID(0);

	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		if (it->second->iClientID != iClientNumToAdd)
			continue;

		yVoiceID = it->first;
		break;
	}

	if (yVoiceID == 0)
		return;

	// Now add that voice ID to all "vClientsToAddTo" targets
	for (ivector_it it(vClientsToAddTo.begin()); it != vClientsToAddTo.end(); it++)
	{
		if (*it == -1 || *it == iClientNumToAdd)
			continue;

		for (VoiceClientMap_it targetit(m_mapVoiceClients.begin()); targetit != m_mapVoiceClients.end(); targetit++)
		{
			if (targetit->second->iClientID != *it)
				continue;

			targetit->second->setTargets[eTargetSet].insert(yVoiceID);
			break;
		}
	}
}

/*====================
  CVoiceServer::AddVoiceTargets
  ====================*/
void	CVoiceServer::AddVoiceTargets(ivector &vClientsToAdd, int iClientNumToAddTo, EVoiceTargetSets eTargetSet)
{
	// Find the target's voice ID first
	VoiceClientMap_it it(m_mapVoiceClients.begin());

	while (it != m_mapVoiceClients.end())
	{
		if (it->second->iClientID != iClientNumToAddTo)
		{
			it++;
			continue;
		}

		break;
	}

	if (it == m_mapVoiceClients.end())
		return;

	// Now add the voice IDs in vClientsToAdd to the target
	for (ivector_it clientit(vClientsToAdd.begin()); clientit != vClientsToAdd.end(); clientit++)
	{
		if (*clientit == -1 || *clientit == iClientNumToAddTo)
			continue;

		for (VoiceClientMap_it targetit(m_mapVoiceClients.begin()); targetit != m_mapVoiceClients.end(); targetit++)
		{
			if (targetit->second->iClientID != *clientit)
				continue;

			it->second->setTargets[eTargetSet].insert(targetit->first);
			break;
		}
	}
}

/*====================
  CVoiceServer::RemoveVoiceTarget
  ====================*/
void	CVoiceServer::RemoveVoiceTarget(int iClientNumToRemove, ivector &vClientsToRemoveFrom, EVoiceTargetSets eTargetSet)
{
	// Find the target's voice ID first
	byte yVoiceID(0);

	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		if (it->second->iClientID != iClientNumToRemove)
			continue;

		yVoiceID = it->first;
		break;
	}

	if (yVoiceID == 0)
		return;

	// Now remove that voice ID from all "vClientsToAddTo" targets
	for (ivector_it it(vClientsToRemoveFrom.begin()); it != vClientsToRemoveFrom.end(); it++)
	{
		if (*it == -1 || *it == iClientNumToRemove)
			continue;

		for (VoiceClientMap_it targetit(m_mapVoiceClients.begin()); targetit != m_mapVoiceClients.end(); targetit++)
		{
			if (targetit->second->iClientID != *it)
				continue;

			targetit->second->setTargets[eTargetSet].erase(yVoiceID);
			break;
		}
	}
}

/*====================
  CVoiceServer::RemoveAllVoiceTargets
  ====================*/
void	CVoiceServer::RemoveAllVoiceTargets(int iClientNum, EVoiceTargetSets eTargetSet)
{
	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		if (it->second->iClientID != iClientNum)
			continue;

		it->second->setTargets[eTargetSet].clear();
		break;
	}
}

/*====================
  CVoiceServer::AddVoiceClient
  ====================*/
void	CVoiceServer::AddVoiceClient(CClientConnection *pClient)
{
	if (svr_voiceDisable)
		return;

	// Check for voice ID conflicts
	byte yStart(m_yNextVoiceID - 1);

	while (m_mapVoiceClients.find(m_yNextVoiceID) != m_mapVoiceClients.end() && m_yNextVoiceID != yStart)
		m_yNextVoiceID++;

	if (m_yNextVoiceID == yStart)
		return;

	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		if (it->second->iClientID != pClient->GetClientNum())
			continue;

		// Already exists
		return;
	}

	Console << _T("Voice: Adding client num ") << pClient->GetClientNum() << _T(", ID: ") << m_yNextVoiceID << newl;

	// Create the new voice client
	SVoiceClient *pVoice = K2_NEW(ctx_Voice,  SVoiceClient)();

	pVoice->pClient = pClient;
	pVoice->iClientID = pClient->GetClientNum();
	pVoice->sAddress = pClient->GetAddress();
	pVoice->bRecievedData = false;

	pVoice->socket.Init(m_sockVoice);

	// Update other voice clients on the new user, and update the new user on them
	CPacket pktSendNew;
	CPacket pktSendUpdate;
	pktSendUpdate << NETCMD_UPDATE_VOICE_CLIENT << m_yNextVoiceID << ushort(m_sockVoice.GetLocalPort()) << byte(m_mapVoiceClients.size());

	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		if (it->second->pClient == NULL)
			continue;

		pktSendNew << NETCMD_NEW_VOICE_CLIENT << pVoice->iClientID << m_yNextVoiceID;
		it->second->pClient->SendReliablePacket(pktSendNew);
		pktSendNew.Clear();

		pktSendUpdate << it->second->iClientID << it->first;
	}

	pClient->SendReliablePacket(pktSendUpdate);

	m_mapVoiceClients[m_yNextVoiceID] = pVoice;

	m_yNextVoiceID++;

	if (m_yNextVoiceID == 0)
		m_yNextVoiceID++;
}

/*====================
  CVoiceServer::RemoveVoiceClient
  ====================*/
void	CVoiceServer::RemoveVoiceClient(CClientConnection *pClient)
{
	VoiceClientMap_it it(m_mapVoiceClients.begin());

	while (it != m_mapVoiceClients.end() && it->second->iClientID != pClient->GetClientNum())
		it++;

	if (it == m_mapVoiceClients.end())
		return;

	byte yVoiceID(it->first);

	Console << _T("Voice: Removing client num ") << pClient->GetClientNum() << _T(", ID: ") << yVoiceID << newl;

	CPacket pktSend;

	SAFE_DELETE(it->second);
	STL_ERASE(m_mapVoiceClients, it);

	for (it = m_mapVoiceClients.begin(); it != m_mapVoiceClients.end(); it++)
	{
		for (uint i(0); i < NUM_VOICE_TARGET_SETS; i++)
			it->second->setTargets[i].erase(yVoiceID);

		pktSend << NETCMD_REMOVE_VOICE_CLIENT << yVoiceID;
		it->second->pClient->SendReliablePacket(pktSend);
		pktSend.Clear();
	}
}


/*====================
  CVoiceServer::GetProfileStats
  ====================*/
void	CVoiceServer::GetProfileStats(uint &uiBytesSent, uint &uiPacketsSent, uint &uiBytesDropped, uint &uiPacketsDropped, uint &uiBytesReceived, uint &uiPacketsReceived)
{
	for (VoiceClientMap_it it(m_mapVoiceClients.begin()); it != m_mapVoiceClients.end(); it++)
	{
		uiBytesSent += it->second->socket.GetBytesSent();
		uiPacketsSent += it->second->socket.GetPacketsSent();
		uiBytesDropped += it->second->socket.GetBytesDropped();
		uiPacketsDropped += it->second->socket.GetPacketsDropped();

		it->second->socket.ClearProfileStats();
	}

	uiBytesReceived += m_uiBytesReceived;
	uiPacketsReceived += m_uiPacketsReceived;

	m_uiBytesReceived = 0;
	m_uiPacketsReceived = 0;
}
