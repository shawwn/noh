// (C)2009 S2 Games
// c_voicemanager.h
//
//=============================================================================
#ifndef __C_VOICEMANAGER__
#define __C_VOICEMANAGER__

//=============================================================================
// Headers
//=============================================================================
#include "c_socket.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CVoiceManager	&VoiceManager;

class CVoiceUser;
struct SpeexBits;
struct SpeexPreprocessState_;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef	map<byte, CVoiceUser*>	VoiceUserMap;
typedef	VoiceUserMap::iterator	VoiceUserMap_it;
typedef pair<byte, CVoiceUser*>	VoiceUserPair;

const uint	VOICE_PACKET_DELAY_FRAMES(10);
const uint	VOICE_SAMPLE_SIZE(81920);
const uint	VOICE_SAMPLE_RATE(8000);
const uint	VOICE_OUTPUT_BUFFER_SIZE(1024);
const uint	VOICE_INPUT_BUFFER_SIZE(81920);

const float	VOICE_MS_PER_FRAME(160000 / float(VOICE_SAMPLE_RATE));
const float	VOICE_FRAMES_PER_SEC(1000 / VOICE_MS_PER_FRAME);
const float	VOICE_PACKET_DELAY_MS(VOICE_PACKET_DELAY_FRAMES * VOICE_MS_PER_FRAME);
//=============================================================================

//=============================================================================
// CVoiceManager
//=============================================================================
class CVoiceManager
{
	SINGLETON_DEF(CVoiceManager)

private:
	SpeexBits*		m_Bits;
	void*			m_EncoderState;
	SpeexPreprocessState_*	m_Preprocess;

	uint			m_uiFrameSize;
	uint			m_uiBytesPerFrame;

	uint			m_uiLastReadPos;
	uint			m_uiRecordingLength;

	short			m_pInputBuffer[VOICE_INPUT_BUFFER_SIZE];
	char			m_pOutputBuffer[VOICE_OUTPUT_BUFFER_SIZE];

	uint			m_uiFrameNumber;

	CSocket			m_sockVoice;
	CPacket			m_packSend;

	bool			m_bConnected;
	bool			m_bRecording;

	VoiceUserMap	m_mapVoiceUsers;

	uint			m_uiLastSend;

	byte			m_yVoiceID;

	bool			m_bTestingVoiceLevel;
	bool			m_bTalkPushed;
	bool			m_bLaneTalkPushed;

	bool			m_bUserTalking;
	bool			m_bRecievedData;

	iset			m_setTalking;
	iset			m_setMuted;

	uint			m_uiControlAccumulator;

	uint			m_uiLastTalkTime;

	int				m_iClientNum;
	
#ifndef _WIN32
	float			m_fGain;
#endif

public:
	~CVoiceManager();

	void			Frame();
	void			Init();
	
	void			Stop();
	void			Restart();

	void			EncodeFrame();
	void			ProcessFrames(uint uiNumFrames);

	void			NetFrame();

	void			DecodeFrame();

	void			AddClient(uint uiClientID, byte yVoiceID);
	void			RemoveClient(byte yVoiceID);

	void			Disconnect();
	void			Connect(byte yVoiceID, const tstring &sAddress, ushort unPort, int iClientNum);

	CVoiceUser*		GetVoiceUser(byte yVoiceID);

	void			StartLevelTest();
	void			StopLevelTest();

	void			SetTalkPushed(bool bValue)				{ m_bTalkPushed = bValue; }
	void			SetLaneTalkPushed(bool bValue)			{ m_bLaneTalkPushed = bValue; }

	void			UpdateMicSettings();

	bool			IsTalking(int iClientNum)				{ return (m_setTalking.find(iClientNum) != m_setTalking.end()); }

	void			MuteClient(int iClientNum);
	void			UnmuteClient(int iClientNum);
	bool			IsClientMuted(int iClientNum)			{ return (m_setMuted.find(iClientNum) != m_setMuted.end()); }
};
//=============================================================================

#endif //__C_VOICEMANAGER__
