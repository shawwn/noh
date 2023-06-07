// (C)2007 S2 Games
// c_voice.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_voice.h"
#include "c_voicemanager.h"

#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"

#include "speex/speex.h"
//=============================================================================

/*====================
  CVoice::~CVoice
  ====================*/
CVoice::~CVoice()
{
    if (m_pDecoder != NULL)
        speex_decoder_destroy(m_pDecoder);
    
    if (m_hSound != -1)
        K2SoundManager.StopHandle(m_hSound);

    SAFE_DELETE(m_pSample);
}


/*====================
  CVoice::CVoice
  ====================*/
CVoice::CVoice(CSample *pSample) :
m_pDecoder(NULL),
m_bIsMute(false),
m_bIsTalking(false),
m_uiOffset(0),
m_eBandwidthMode(VOICE_BANDWIDTH_WIDE),
m_pSample(pSample),
m_hSound(-1)
{
}


/*====================
  CVoice::AddDecoder
  ====================*/
void    CVoice::AddDecoder()
{
    m_pDecoder = speex_decoder_init(GetBandwidthMode());
}


/*====================
  CVoice::AdvanceOffset
  ====================*/
void    CVoice::AdvanceOffset(uint uiCount)
{
    m_uiOffset += uiCount;
    m_uiOffset %= K2SoundManager.GetSampleLength(m_pSample);
}


/*====================
  CVoice::GetBandwidthMode
  ====================*/
const SpeexMode*    CVoice::GetBandwidthMode()
{
    switch (m_eBandwidthMode)
    {
    case VOICE_BANDWIDTH_NARROW:
        return &speex_nb_mode;
        break;

    default:
    case VOICE_BANDWIDTH_WIDE:
        return &speex_wb_mode;
        break;

    case VOICE_BANDWIDTH_ULTRAWIDE:
        return &speex_uwb_mode;
        break;
    }
}


/*====================
  CVoice::SendData
  ====================*/
void    CVoice::SendData()
{
}


/*====================
  CVoice::StopTalking
  ====================*/
void    CVoice::StopTalking()
{
    SetTalking(false);
    K2SoundManager.ResetSampleAtPos(m_pSample, 0, K2SoundManager.GetSampleLength(m_pSample));
}
