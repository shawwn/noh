// (C)2009 S2 Games
// c_voiceuser.h
//
//=============================================================================
#ifndef __C_VOICEUSER__
#define __C_VOICEUSER__

//=============================================================================
// Headers
//=============================================================================
#include <speex/speex.h>
#include <speex/speex_preprocess.h>
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct JitterData
{
    char *pData;
    uint uiLength;
    uint uiFrame;

    JitterData() :
    pData(nullptr),
    uiLength(0),
    uiFrame(0)
    {
    }
};

typedef deque<JitterData*>          VoiceJitterDeq;
typedef VoiceJitterDeq::iterator    VoiceJitterDeq_it;

const uint  VOICE_TALK_DURATION(500);
const uint  VOICE_FRAMES_WRITE_AHEAD(3);
const uint  VOICE_MAX_EXTRAPOLATE_FRAMES(10);
//=============================================================================

//=============================================================================
// CVoiceUser
//=============================================================================
class CVoiceUser
{
private:
    SpeexBits       m_Bits;
    void*           m_DecoderState;

    uint            m_uiFrameSize;
    uint            m_uiBytesPerFrame;

    uint            m_uiSampleLength;

    byte            m_yLastWritePos;
    byte            m_yLastReadPos;
    byte            m_yFramesAhead;
    uint            m_uiLastWrittenFrame;
    uint            m_uiLastInternalFrame;

    CSample*        m_pSample;
    SoundHandle     m_hHandle;

    VoiceJitterDeq  m_dequeJitter;

    bool            m_bRecievedFirstFrame;

    short           m_pInputBuffer[VOICE_INPUT_BUFFER_SIZE];

    uint            m_uiClientNum;

    uint            m_uiLastTalkTime;

    bool            m_bMuted;

    CVoiceUser() {}

public:
    CVoiceUser(uint uiClientNum, bool bMuted);
    ~CVoiceUser();

    void            DecodeFrame();
    void            AddFrame(uint uiSequence, byte *pData, uint uiLength);

    bool            IsTalking();
    uint            GetClientNum()          { return m_uiClientNum; }

    bool            IsMuted()               { return m_bMuted; }
    void            SetMuted(bool bValue)   { m_bMuted = bValue; }
    
    void            Stop();
    void            Restart();
};
//=============================================================================

#endif //__C_VOICEUSER__
