// (C)2009 S2 Games
// c_voiceserver.h
//
//=============================================================================
#ifndef __C_VOICESERVER__
#define __C_VOICESERVER__

//=============================================================================
// Headers
//=============================================================================
#include "c_socket.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CVoiceServer	&VoiceServer;

class CClientConnection;

enum EVoiceTargetSets
{
	VOICE_TARGET_SET_TEAM,
	VOICE_TARGET_SET_LANE,
	NUM_VOICE_TARGET_SETS
};

struct SVoiceClient
{
	int iClientID;
	CSocket socket;
	CClientConnection *pClient;
	tstring sAddress;
	set<byte> setTargets[NUM_VOICE_TARGET_SETS];
	CPacket packet;
	bool bRecievedData;

	SVoiceClient() :
	socket(_T("VoiceClient")),
	pClient(NULL)
	{
	}
};
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef	map<byte, SVoiceClient*>	VoiceClientMap;
typedef pair<byte, SVoiceClient*>	VoiceClientPair;
typedef VoiceClientMap::iterator	VoiceClientMap_it;

const ushort	DEFAULT_VOICE_PORT		(11435);
//=============================================================================

//=============================================================================
// CVoiceManager
//=============================================================================
class CVoiceServer
{
	SINGLETON_DEF(CVoiceServer)

private:
	CSocket			m_sockVoice;
	VoiceClientMap	m_mapVoiceClients;
	byte			m_yNextVoiceID;

	uint			m_uiBytesReceived;
	uint			m_uiPacketsReceived;

public:
	~CVoiceServer();

	void			Frame();
	void			Init();

	K2_API void		AddVoiceClient(CClientConnection *pClient);
	K2_API void		RemoveVoiceClient(CClientConnection *pClient);

	K2_API void		AddVoiceTarget(int iClientNumToAdd, ivector &vClientsToAddTo, EVoiceTargetSets eTargetSet);
	K2_API void		AddVoiceTargets(ivector &vClientsToAdd, int iClientNumToAddto, EVoiceTargetSets eTargetSet);
	K2_API void		RemoveVoiceTarget(int iClientNumToRemove, ivector &vClientsToRemoveFrom, EVoiceTargetSets eTargetSet);
	K2_API void		RemoveAllVoiceTargets(int iClientNum, EVoiceTargetSets eTargetSet);

	K2_API void		GetProfileStats(uint &uiBytesSent, uint &uiPacketsSent, uint &uiBytesDropped, uint &uiPacketsDropped, uint &uiBytesRecieved, uint &uiPacketsReceived);
};
//=============================================================================

#endif //__C_VOICEMANAGER__
