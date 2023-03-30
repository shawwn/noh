// (C)2007 S2 Games
// c_voicemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_voicemanager.h"
#include "c_voice.h"

#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INTR(	cg_voiceQuality,	8,		CVAR_SAVECONFIG,	1,	10);
CVAR_INTF(	cg_voiceSampleSize,	8192,	CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CVoiceManager::~CVoiceManager
  ====================*/
CVoiceManager::~CVoiceManager()
{
	if (!m_bValid)
		return;

	for (ClientVoiceMap_it it(m_mapPeerVoices.begin()); it != m_mapPeerVoices.end(); ++it)
		SAFE_DELETE(it->second);
	m_mapPeerVoices.clear();

	StopRecording();

	speex_bits_destroy(&m_bitsClientDecode);
	speex_bits_destroy(&m_bitsClientEncode);

	if (m_pEncoder != NULL)
		speex_encoder_destroy(m_pEncoder);

	if (m_pLocalVoice != NULL && m_pLocalVoice->IsValid())
	{
		K2SoundManager.StopRecording();
		SAFE_DELETE(m_pLocalVoice);
	}

	SAFE_DELETE(m_pEncodeBuffer);
}


/*====================
  CVoiceManager::CVoiceManager
  ====================*/
CVoiceManager::CVoiceManager() :
m_bValid(false),

m_pSoundVoiceChatVolume(ICvar::Find(_T("sound_voiceChatVolume"))),

m_pLocalVoice(NULL),
m_pEncoder(NULL),
m_pEncodeBuffer(NULL),
m_uiEncodeBufferLen(0),
m_uiVoiceDataRemaining(0)
{
#ifndef K2_NOSOUND
	try
	{
		/*	NOTE FROM KYLE: Disabled until we can rework how samples are managed

		// Initialize SpeexBits
		speex_bits_init(&m_bitsClientDecode);
		speex_bits_init(&m_bitsClientEncode);

		m_pLocalVoice = new CVoice(K2SoundManager.StartRecording(VOICE_SAMPLE_RATE, cg_voiceSampleSize)); // FIXME
		if (m_pLocalVoice == NULL)
			EX_ERROR(_T("Failed to allocate local CVoice"));

		if (!m_pLocalVoice->IsValid())
		{
			SAFE_DELETE(m_pLocalVoice);
			EX_ERROR(_T("Failed to start voice recording"));
		}

		m_pEncodeBuffer = new byte[cg_voiceSampleSize];
		if (m_pEncodeBuffer == NULL)
			EX_ERROR(_T("Failed to allocate encode buffer"));

		m_pEncoder = speex_encoder_init(m_pLocalVoice->GetBandwidthMode());
		if (m_pEncoder == NULL)
			EX_ERROR(_T("Failed to initialize encoder"));

		int iValue(VOICE_SAMPLE_RATE);
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_SAMPLING_RATE, &iValue);

		iValue = cg_voiceQuality;
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_QUALITY, &iValue);

		iValue = 1;
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_VAD, &iValue);
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_DTX, &iValue);
		speex_encoder_ctl(m_pEncoder, SPEEX_SET_VBR, &iValue);

		m_bValid = true;*/
	}
	catch (CException &ex)
	{
		ex.Process(_T("CVoiceManager::CVoiceManager() - "), NO_THROW);
	}
#endif
}


/*====================
  CVoiceManager::StartRecording
  ====================*/
void	CVoiceManager::StartRecording()
{
	if (!m_bValid)
		return;

	if (m_pLocalVoice->IsTalking())
		return;

	m_pLocalVoice->SetTalking(true);
	m_pLocalVoice->SetOffset(K2SoundManager.GetRecordingPos());

	m_uiVoiceDataRemaining = 0;

	CBufferDynamic buffer;
	buffer << GAME_CMD_VOICE_STARTTALKING;
	GameClient.SendGameData(buffer, true);
}


/*====================
  CVoiceManager::StopRecording
  ====================*/
void	CVoiceManager::StopRecording()
{
	if (!m_bValid)
		return;

	if (!m_pLocalVoice->IsTalking())
		return;

	uint uiSampleLength(K2SoundManager.GetSampleLength(m_pLocalVoice->GetSample()));
	m_uiVoiceDataRemaining = (uiSampleLength - m_pLocalVoice->GetOffset()) + K2SoundManager.GetRecordingPos();
	m_uiVoiceDataRemaining %= uiSampleLength;

	m_pLocalVoice->SetTalking(false);
}


/*====================
  CVoiceManager::SendData
  ====================*/
void	CVoiceManager::SendData(uint uiBytes, const byte *pBuffer)
{
	if (uiBytes > cg_voiceSampleSize - m_uiEncodeBufferLen)
		uiBytes = cg_voiceSampleSize - m_uiEncodeBufferLen;

	MemManager.Copy(&m_pEncodeBuffer[m_uiEncodeBufferLen], pBuffer, uiBytes);
	m_uiEncodeBufferLen += uiBytes;

	uint uiFrameSize(0);
	speex_encoder_ctl(m_pEncoder, SPEEX_GET_FRAME_SIZE, &uiFrameSize);

	char *pTempBuffer(K2_NEW_ARRAY(global, char, uiFrameSize));

	while (m_uiEncodeBufferLen > uiFrameSize)
	{
		uint uiBytesToSend(0);
		speex_bits_reset(&m_bitsClientEncode);
		if (speex_encode_int(m_pEncoder, (short *)m_pEncodeBuffer, &m_bitsClientEncode))
		{
			speex_encoder_ctl(m_pEncoder, SPEEX_GET_FRAME_SIZE, &uiFrameSize);
			uiBytesToSend = speex_bits_write(&m_bitsClientEncode, pTempBuffer, uiFrameSize);
		}

		if (uiBytesToSend > 0)
		{
			CBufferDynamic buffer;
			buffer << GAME_CMD_VOICE_DATA << uiBytesToSend;
			buffer.Append(pTempBuffer, uiBytesToSend);
			GameClient.SendGameData(buffer, false);
		}

		MemManager.Copy(m_pEncodeBuffer, &m_pEncodeBuffer[uiFrameSize], m_uiEncodeBufferLen - uiFrameSize);

		m_uiEncodeBufferLen -= uiFrameSize;
		uiBytesToSend = 0;
	}

	SAFE_DELETE(pTempBuffer);
}


/*====================
  CVoiceManager::ReadData
  ====================*/
void	CVoiceManager::ReadData(int iClientIndex, uint uiLength, char *pData)
{
	byte *pDecodedData(NULL);

	try
	{
		if (!m_bValid)
			return;

		ClientVoiceMap_it itPeer(m_mapPeerVoices.find(iClientIndex));
		if (itPeer == m_mapPeerVoices.end() ||
			itPeer->second->IsMute() ||
			!itPeer->second->IsTalking())
			return;

		uint uiSampleLength(K2SoundManager.GetSampleLength(itPeer->second->GetSample()));
		pDecodedData = K2_NEW_ARRAY(global, byte, uiSampleLength);
		MemManager.Set(pDecodedData, 0, uiSampleLength * sizeof(byte));

		speex_bits_read_from(&m_bitsClientDecode, pData, uiLength);
		speex_decode_int(itPeer->second->GetDecoder(), &m_bitsClientDecode, (short*)pDecodedData);

		uint uFrameSize;
		speex_decoder_ctl(itPeer->second->GetDecoder(), SPEEX_GET_FRAME_SIZE, &uFrameSize);
		if (uFrameSize == -1)
			EX_ERROR(_T("Invalid voice data recieved from server."));

		//Add the sound data to the buffer, and then keep track of
		//our last written position. What we're actually doing is writing
		//from our current write position right up to the current playing
		//position of the sample, to eliminate repeating sounds if we don't
		//recieve data fast enough.
		uint uiChanPos(K2SoundManager.GetChannelPosition(itPeer->second->GetSoundHandle()));
		if (uiChanPos == -1)
			EX_ERROR(_T("Failure in K2SoundManager::GetChannelPosition()"));

		if (itPeer->second->GetOffset() <= uiChanPos)
		{
			K2SoundManager.ModifySampleAtPos(itPeer->second->GetSample(), itPeer->second->GetOffset(), MAX(uiChanPos - itPeer->second->GetOffset(), uFrameSize), pDecodedData);
		}
		else
		{
			uint uSampleLength = MAX((K2SoundManager.GetSampleLength(itPeer->second->GetSample()) - itPeer->second->GetOffset()) + uiChanPos, uFrameSize);
			K2SoundManager.ModifySampleAtPos(itPeer->second->GetSample(), itPeer->second->GetOffset(), uSampleLength, pDecodedData);
		}

		itPeer->second->AdvanceOffset(uFrameSize);
	}
	catch (CException &ex)
	{
		SAFE_DELETE(pDecodedData);
		ex.Process(_T("CVoiceManager::ReadData() - "), NO_THROW);
	}
}


/*====================
  CVoiceManager::Frame
  ====================*/
void	CVoiceManager::Frame()
{
	if (!m_bValid || m_pLocalVoice == NULL || m_pLocalVoice->GetSample() == NULL)
		return;

	if (m_pSoundVoiceChatVolume != NULL && m_pSoundVoiceChatVolume->IsModified())
	{
		m_pSoundVoiceChatVolume->SetModified(false);
		for (ClientVoiceMap_it it(m_mapPeerVoices.begin()); it != m_mapPeerVoices.end(); ++it)
		{
			if (it->second->GetSoundHandle() != INVALID_INDEX)
				K2SoundManager.SetVolume(it->second->GetSoundHandle(), m_pSoundVoiceChatVolume->GetFloat());
		}
	}

	byte *sVoiceData(NULL);
	uint uWrittenLength(0);

	uint uiSampleLength(K2SoundManager.GetSampleLength(m_pLocalVoice->GetSample()));
	uint uMaxLength((uiSampleLength - m_pLocalVoice->GetOffset()) + K2SoundManager.GetRecordingPos());

	if (uiSampleLength == 0 || uiSampleLength == -1)
		return;

	if (uMaxLength == 0)
		return;

	uMaxLength %= uiSampleLength;

	if (uMaxLength > 0)
	{
		sVoiceData = K2SoundManager.GetSampleData(m_pLocalVoice->GetSample(), &uWrittenLength, uMaxLength, m_pLocalVoice->GetOffset());
		m_pLocalVoice->AdvanceOffset(uWrittenLength);
	}

	if (uWrittenLength > 0 && (m_pLocalVoice->IsTalking() || m_uiVoiceDataRemaining > 0))
	{
		if (m_uiVoiceDataRemaining > uWrittenLength)
		{
			SendData(uWrittenLength, sVoiceData);
			m_uiVoiceDataRemaining = m_uiVoiceDataRemaining - uWrittenLength;
		}
		else if (m_uiVoiceDataRemaining > 0)
		{
			CBufferDynamic buffer;
			SendData(m_uiVoiceDataRemaining, sVoiceData);

			buffer << GAME_CMD_VOICE_STOPTALKING;
			GameClient.SendGameData(buffer, true);

			m_uiVoiceDataRemaining = 0;
		}
		else
			SendData(uWrittenLength, sVoiceData);
	}

	SAFE_DELETE(sVoiceData);
}


/*====================
  CVoiceManager::AddClient
  ====================*/
CVoice*	CVoiceManager::AddClient(int iClientIndex)
{
	if (!m_bValid)
		return NULL;

	RemoveClient(iClientIndex);

	CSample *pSample(K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CSample)(K2SoundManager.CreateSound(VOICE_SAMPLE_RATE, 1, 16, cg_voiceSampleSize, SND_LOOP | SND_2D)));
	if (pSample == NULL)
		return NULL;

	//NOTE FROM KYLE: Voice chat should NOT be registered as a sample...
	//					Either that, or we cannot delete the samples that we create.
	//g_ResourceManager.Register(pSample, RES_SAMPLE);
	
	K2SoundManager.ResetSampleAtPos(pSample, 0, K2SoundManager.GetSampleLength(pSample));
	
	float fVolume(1.0f);
	if (m_pSoundVoiceChatVolume != NULL)
		fVolume = m_pSoundVoiceChatVolume->GetFloat();

	SoundHandle hSound(K2SoundManager.PlayVoiceSound(pSample->GetHandle(), fVolume));

	if (hSound == INVALID_INDEX)
	{
		SAFE_DELETE(pSample);
		return NULL;
	}

	CVoice *pNewVoice(K2_NEW(global,   CVoice)(pSample));
	if (pNewVoice == NULL)
	{
		K2SoundManager.StopHandle(hSound);
		SAFE_DELETE(pSample);
		return NULL;
	}
	m_mapPeerVoices[iClientIndex] = pNewVoice;

	pNewVoice->SetSoundHandle(hSound);
	pNewVoice->AddDecoder();
	
	int iSampleRate(VOICE_SAMPLE_RATE);
	speex_decoder_ctl(pNewVoice->GetDecoder(), SPEEX_SET_SAMPLING_RATE, &iSampleRate);
	
	return pNewVoice;
}


/*====================
  CVoiceManager::RemoveClient
  ====================*/
void	CVoiceManager::RemoveClient(int iClientIndex)
{
	ClientVoiceMap_it itFind(m_mapPeerVoices.find(iClientIndex));
	if (itFind == m_mapPeerVoices.end())
		return;

	STL_ERASE(m_mapPeerVoices, itFind);
}


/*====================
  CVoiceManager::GetClientVoice
  ====================*/
CVoice*	CVoiceManager::GetClientVoice(int iClientIndex)
{
	ClientVoiceMap_it itFind(m_mapPeerVoices.find(iClientIndex));
	if (itFind == m_mapPeerVoices.end())
		return NULL;

	return itFind->second;
}


/*====================
  CVoiceManager::StartTalking
  ====================*/
void	CVoiceManager::StartTalking(int iClientIndex)
{
	CVoice *pVoice(GetClientVoice(iClientIndex));
	if (pVoice == NULL)
	{
		pVoice = AddClient(iClientIndex);
		if (pVoice == NULL)
			return;
	}

	pVoice->SetOffset(0);
	pVoice->SetTalking(true);
	K2SoundManager.SetPlayPosition(pVoice->GetSoundHandle(), 0);
}
