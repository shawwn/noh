// (C)2009 S2 Games
// c_voiceuser.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_soundmanager.h"
#include "c_sample.h"
#include "c_voicemanager.h"
#include "c_voiceuser.h"
//=============================================================================

/*====================
  CVoiceUser::~CVoiceUser
  ====================*/
CVoiceUser::~CVoiceUser()
{
	speex_bits_destroy(&m_Bits);
	speex_decoder_destroy(m_DecoderState);

	if (m_hHandle != INVALID_RESOURCE)
		K2SoundManager.StopHandle(m_hHandle);

	SAFE_DELETE(m_pSample);

	VoiceJitterDeq_it it(m_dequeJitter.begin());

	while (it != m_dequeJitter.end())
	{
		if ((*it) != NULL)
		{
			K2_DELETE_ARRAY((*it)->pData);
			K2_DELETE(*it);
		}

		it++;
	}
}


/*====================
  CVoiceUser::CVoiceUser
  ====================*/
CVoiceUser::CVoiceUser(uint uiClientNum, bool bMuted) :
m_DecoderState(NULL),
m_uiFrameSize(0),
m_uiBytesPerFrame(0),
m_uiSampleLength(0),
m_yFramesAhead(0),
m_yLastWritePos(0),
m_yLastReadPos(0),
m_uiLastWrittenFrame(0),
m_uiLastInternalFrame(0),
m_pSample(NULL),
m_hHandle(INVALID_RESOURCE),
m_uiClientNum(uiClientNum),
m_uiLastTalkTime(0),
m_bMuted(bMuted),
m_bRecievedFirstFrame(false)
{
	int iOn(1);

	speex_bits_init(&m_Bits);
	m_DecoderState = speex_decoder_init(&speex_nb_mode);

	speex_decoder_ctl(m_DecoderState, SPEEX_SET_ENH, &iOn);
	speex_decoder_ctl(m_DecoderState, SPEEX_GET_FRAME_SIZE, &m_uiFrameSize);

	FMOD::Sound *pSound(K2SoundManager.CreateSound(VOICE_SAMPLE_RATE, 1, SOUND_SAMPLE_RATE, VOICE_SAMPLE_SIZE, SND_LOOP | SND_2D));

	if (pSound == NULL)
		return;

	m_pSample = K2_NEW(ctx_Voice,  CSample)(pSound);

	m_uiBytesPerFrame = INT_CEIL((SOUND_SAMPLE_BYTES) * float(m_uiFrameSize));
	m_uiSampleLength = K2SoundManager.GetSampleLength(m_pSample);
}


/*====================
  CVoiceUser::Stop
  ====================*/
void	CVoiceUser::Stop()
{
	SAFE_DELETE(m_pSample);
	m_hHandle = INVALID_RESOURCE;
}


/*====================
  CVoiceUser::Restart
  ====================*/
void	CVoiceUser::Restart()
{
	FMOD::Sound *pSound(K2SoundManager.CreateSound(VOICE_SAMPLE_RATE, 1, SOUND_SAMPLE_RATE, VOICE_SAMPLE_SIZE, SND_LOOP | SND_2D));

	if (pSound == NULL)
		return;

	m_pSample = K2_NEW(ctx_Voice,  CSample)(pSound);

	m_uiBytesPerFrame = INT_CEIL((SOUND_SAMPLE_BYTES) * float(m_uiFrameSize));
	m_uiSampleLength = K2SoundManager.GetSampleLength(m_pSample);
}


/*====================
  CVoiceUser::DecodeFrame
  ====================*/
void	CVoiceUser::DecodeFrame()
{
	if (m_pSample == NULL)
		return;

	// If m_hHandle == INVALID_RESOURCE, we haven't started playing the sound yet or we were muted
	if (m_hHandle == INVALID_RESOURCE)
	{
		// If we're still muted, we're done for now
		if (m_bMuted)
			return;

		// Check if we've recieved any frames yet
		if (!m_bRecievedFirstFrame)
			return;

		// If we have, set play position to first frame
		m_hHandle = K2SoundManager.PlayVoiceSound(m_pSample);
		
		if (m_hHandle == INVALID_RESOURCE)
			return;

		m_yLastWritePos = 0;
		m_yLastReadPos = 0;
		m_uiLastWrittenFrame = 0;
		m_uiLastInternalFrame = 0;
		m_yFramesAhead = 0;

		K2SoundManager.ResetSampleAtPos(m_pSample, 0, m_uiSampleLength);
		K2SoundManager.SetPlayPosition(m_hHandle, 0);
	}

	if (m_bMuted)
	{
		K2SoundManager.StopHandle(m_hHandle);
		m_hHandle = INVALID_RESOURCE;
		return;
	}

	uint uiReadPos = K2SoundManager.GetChannelPosition(m_hHandle);
	byte yCurFrame = uiReadPos / m_uiBytesPerFrame;

	byte yDiff;
	uint uiFramesWritten(0);
	
	if (yCurFrame < m_yLastReadPos)
		yDiff = (256 - m_yLastReadPos) + yCurFrame;
	else
		yDiff = yCurFrame - m_yLastReadPos;

	m_yLastReadPos = yCurFrame;

	if (m_yFramesAhead >= yDiff)
		m_yFramesAhead -= yDiff;
	else
	{
		uiFramesWritten = (yDiff - m_yFramesAhead);
		m_yFramesAhead = 0;
	}
		
	JitterData *pJitter(NULL);

	byte yFramesToWrite;

	yFramesToWrite = INT_CEIL(Host.GetFrameLength() / VOICE_MS_PER_FRAME) + VOICE_FRAMES_WRITE_AHEAD;

	MemManager.Set(m_pInputBuffer, 0, sizeof(m_pInputBuffer));

	if (m_dequeJitter.size() > 0)
		pJitter = m_dequeJitter.front();

	while (pJitter != NULL || m_yFramesAhead < yFramesToWrite)
	{
		if (m_dequeJitter.size() > 0)
			m_dequeJitter.pop_front();

		if (pJitter != NULL)
		{
			speex_bits_read_from(&m_Bits, pJitter->pData, pJitter->uiLength);
			speex_decode_int(m_DecoderState, &m_Bits, &(m_pInputBuffer[uiFramesWritten * m_uiFrameSize]));

			m_uiLastInternalFrame = m_uiLastWrittenFrame = pJitter->uiFrame;
			m_uiLastTalkTime = Host.GetTime();

			K2_DELETE_ARRAY(pJitter->pData);
			K2_DELETE(pJitter);

			pJitter = NULL;
		}
		else
		{
			if (m_uiLastWrittenFrame - m_uiLastInternalFrame <= VOICE_MAX_EXTRAPOLATE_FRAMES)
				speex_decode_int(m_DecoderState, NULL, &(m_pInputBuffer[uiFramesWritten * m_uiFrameSize]));

			m_uiLastInternalFrame++;
		}

		m_yFramesAhead++;
		uiFramesWritten++;

		if (m_dequeJitter.size() > 0)
			pJitter = m_dequeJitter.front();
	}

	if (uiFramesWritten > 256)
		uiFramesWritten = 256;

	if (uiFramesWritten > 0)
		K2SoundManager.ModifySampleAtPos(m_pSample, m_yLastWritePos * m_uiBytesPerFrame, uiFramesWritten * m_uiBytesPerFrame, (byte *)m_pInputBuffer);

	m_yLastWritePos += uiFramesWritten;
}


/*====================
  CVoiceUser::AddFrame
  ====================*/
void	CVoiceUser::AddFrame(uint uiSequence, byte *pData, uint uiLength)
{
	if (uiLength == 0)
		return;

	if (m_bMuted)
	{
		m_uiLastTalkTime = Host.GetTime();
		return;
	}

	if (uiSequence < m_uiLastWrittenFrame)
	{
		Console.Warn << _T("Voice: Dropped delayed voice frame ") << uiSequence << newl;
		return;
	}

	if (m_dequeJitter.empty())
		m_uiLastInternalFrame = m_uiLastWrittenFrame = uiSequence - 1;

	uint uiOffset(uiSequence - m_uiLastWrittenFrame);

	if (uiOffset > 0xffff)
	{
		Console.Warn << _T("Voice: large offset ") << uiOffset << newl;
		return;
	}

	if (uiOffset == 0)
	{
		Console.Warn << _T("Voice: zero offset ") << uiOffset << newl;
		return;
	}

	if (!m_bRecievedFirstFrame)
		m_bRecievedFirstFrame = true;

	JitterData *pJitter = K2_NEW(ctx_Voice,  JitterData)();

	pJitter->uiFrame = uiSequence;
	pJitter->uiLength = uiLength;
	pJitter->pData = K2_NEW_ARRAY(ctx_Voice, char, uiLength);
	MemManager.Copy(pJitter->pData, pData, uiLength);

	if (m_dequeJitter.size() < uiOffset)
		m_dequeJitter.resize(uiOffset, NULL);

	JitterData *pDelete = m_dequeJitter[uiOffset - 1];
	m_dequeJitter[uiOffset - 1] = pJitter;

	if (pDelete != NULL)
	{
		K2_DELETE_ARRAY(pDelete->pData);
		K2_DELETE(pDelete);
	}

	m_uiLastTalkTime = Host.GetTime();
}


/*====================
  CVoiceUser::IsTalking
  ====================*/
bool	CVoiceUser::IsTalking()
{
	return (Host.GetTime() - m_uiLastTalkTime < VOICE_TALK_DURATION);
}
