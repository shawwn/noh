// (C)2007 S2 Games
// c_voice.h
//
//=============================================================================
#ifndef __C_VOICE_H__
#define __C_VOICE_H__

//=============================================================================
// Declarations
//=============================================================================
struct SpeexMode;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EVoiceBandwidth
{
    VOICE_BANDWIDTH_NARROW,
    VOICE_BANDWIDTH_WIDE,
    VOICE_BANDWIDTH_ULTRAWIDE,

    NUM_VOICE_BANDWIDTH_MODES
};
//=============================================================================

//=============================================================================
// CVoice
//=============================================================================
class CVoice
{
private:
    void*           m_pDecoder;
    bool            m_bIsMute;
    bool            m_bIsTalking;
    uint            m_uiOffset;
    EVoiceBandwidth m_eBandwidthMode;
    CSample*        m_pSample;
    SoundHandle     m_hSound;

    CVoice();

public:
    ~CVoice();
    CVoice(CSample *pSample);

    void                SetMute(bool bMute)             { m_bIsMute = bMute; }
    bool                IsMute() const                  { return m_bIsMute; }

    void                SetTalking(bool bTalking)       { m_bIsTalking = bTalking; }
    bool                IsTalking() const               { return m_bIsTalking; }

    SoundHandle         GetSoundHandle() const          { return m_hSound; }
    void                SetSoundHandle(SoundHandle hSound)  { m_hSound = hSound; }

    void                AddDecoder();
    void*               GetDecoder() const              { return m_pDecoder; }
    CSample*            GetSample() const               { return m_pSample; }

    void                SetOffset(uint uiOffset)        { m_uiOffset = uiOffset; }
    void                AdvanceOffset(uint uiCount);
    uint                GetOffset() const               { return m_uiOffset; }

    const SpeexMode*    GetBandwidthMode();

    void                SendData();

    void                StopTalking();

    bool                IsValid()                       { return (m_pSample != NULL); }
};
//=============================================================================

#endif //__C_VOICE_H__
