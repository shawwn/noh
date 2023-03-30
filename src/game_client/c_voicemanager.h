// (C)2007 S2 Games
// c_voicemanager.h
//
//=============================================================================
#ifndef __C_VOICEMANAGER__
#define __C_VOICEMANAGER__

//=============================================================================
// Headers
//=============================================================================
#include "speex/speex.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CVoice;

extern CCvari cg_voiceSampleSize;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int VOICE_SAMPLE_RATE(11025);

typedef map<int, CVoice*>			ClientVoiceMap;
typedef ClientVoiceMap::iterator	ClientVoiceMap_it;
//=============================================================================

//=============================================================================
// CVoiceManager
//=============================================================================
class CVoiceManager
{
private:
	bool				m_bValid;

	ICvar*				m_pSoundVoiceChatVolume;

	CVoice*				m_pLocalVoice;
	ClientVoiceMap		m_mapPeerVoices;

	void*		 		m_pEncoder;
	byte*				m_pEncodeBuffer;
	uint				m_uiEncodeBufferLen;

	SpeexBits			m_bitsClientDecode;
	SpeexBits			m_bitsClientEncode;

	uint				m_uiVoiceDataRemaining;

public:
	~CVoiceManager();
	CVoiceManager();

	void			StartRecording();
	void			StopRecording();

	void			SendData(uint uiBytes, const byte *pBuffer);
	void			ReadData(int iClientIndex, uint uiLength, char *pData);

	void			Frame();

	CVoice*			AddClient(int iClientIndex);
	void			RemoveClient(int iClientIndex);
	CVoice*			GetClientVoice(int iClientIndex);

	void			StartTalking(int iClientIndex);
};
//=============================================================================

#endif //__C_VOICEMANAGER__
