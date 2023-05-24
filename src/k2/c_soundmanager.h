// (C)2005 S2 Games
// c_soundmanager.h
//
//=============================================================================
#ifndef __C_SOUNDMANAGER_H__
#define __C_SOUNDMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
class CSample;

extern K2_API class CSoundManager &K2SoundManager;

namespace FMOD
{
    class System;
    class Sound;
    class Channel;
    class ChannelGroup;
    class Geometry;
}

#ifndef _FMOD_HPP
typedef struct
{
    float x;        /* X co-ordinate in 3D space. */
    float y;        /* Y co-ordinate in 3D space. */
    float z;        /* Z co-ordinate in 3D space. */
} FMOD_VECTOR;
#endif

struct SpeexResamplerState_;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint SOUNDMANAGER_FAILURE_THRESHOLD(10);

enum ESoundChannels
{
    CHANNEL_AUTO = -1,
    CHANNEL_MUSIC,
    CHANNEL_NOTIFICATION,
    CHANNEL_NOTIFICATION_2,
    CHANNEL_NOTIFICATION_3,
    CHANNEL_GOAL,
    CHANNEL_GUI,
    CHANNEL_CMDR_BUILDING_SELECT,
    CHANNEL_CMDR_UNIT_SELECT,
    CHANNEL_VOICE_CHAT,
    CHANNEL_WEAPON,                 // our own weapon only...everyone else uses CHANNEL_AUTO
    CHANNEL_FIRST_AUTO              // = 10
};

const int   MAX_SOUND_SOURCES   (8192);
const int   MAX_LOOPING_SOUNDS  (256);
const int   MAX_CHANNELS        (256);
const int   NUM_ASSIGNED_CHANNELS(32);

#define FMOD_VECTOR_CAST(v) reinterpret_cast<const FMOD_VECTOR*>(v)

struct FMOD_CREATESOUNDEXINFO;

const int   SOUND_SAMPLE_RATE(16);
const int   SOUND_SAMPLE_BYTES(SOUND_SAMPLE_RATE / 8);
//=============================================================================

//=============================================================================
// CSoundManager
//=============================================================================
#ifndef K2_NOSOUND
#include "c_range.h"
class CSoundManager
{
    SINGLETON_DEF(CSoundManager)

private:
    FMOD::System*   m_pFMODSystem;

    SpeexResamplerState_* m_pResamplerMono;
    SpeexResamplerState_* m_pResamplerStereo;

    bool            m_bInitialized;
    uint            m_uiFailureCount;

    int             m_iNumDrivers;
    int             m_iNumRecordingDrivers;
    int             m_iRecordingDriver;

    int             m_iLoopFrame;
    uint            m_uiHandleCounter;
    int             m_iNumLoopingSounds;

    float           m_fFalloffBias;
    CVec3f          m_v3Center;
    CVec3f          m_v3Jitter; // Position jitter to force rolloff updates

    struct SPlayMusic
    {
        tstring         sFilename;
        ResHandle       hSample;
        FMOD::Sound*    pSound;
        bool            bLoop;
        bool            bFadeIn;

        SPlayMusic()
            : hSample(INVALID_RESOURCE)
            , pSound(nullptr)
            , bLoop(false)
            , bFadeIn(false)
        { }

        SPlayMusic(const tstring& sFilename, bool bLoop, bool bFadeIn)
            : hSample(INVALID_RESOURCE)
            , sFilename(sFilename)
            , pSound(nullptr)
            , bLoop(bLoop)
            , bFadeIn(bFadeIn)
        { }

        bool    IsValid() const { return !sFilename.empty(); }
    };
    
    // classes for sounds fading in/out
    class CSoundFade
    {
    public:
        CSoundFade() {}
        virtual ~CSoundFade() {}

        virtual void    Start(FMOD::Channel* pChannel, uint uiNow, bool bFadeIn) = 0;
        virtual bool    Update(FMOD::Channel* pChannel, uint uiNow) = 0;
    };

    class CSoundFadeVolume : public CSoundFade
    {
    private:
        uint            m_uiStartTime;
        uint            m_uiFadeTime;
        CRangef         m_rfVolume;
    public:
        CSoundFadeVolume(uint uiFadeTime, float fFinalVolume) : m_uiFadeTime(uiFadeTime), m_rfVolume(fFinalVolume, fFinalVolume) {};

        virtual void    Start(FMOD::Channel* pChannel, uint uiNow, bool bFadeIn);
        virtual bool    Update(FMOD::Channel* pChannel, uint uiNow);
    };

    class CSoundFadeSpeed : public CSoundFade
    {
    private:
        uint            m_uiStartTime;
        uint            m_uiFadeTime;
        CRangef         m_rfFrequency;
    public:
        CSoundFadeSpeed(uint uiFadeTime, float fFinalFrequency) : m_uiFadeTime(uiFadeTime), m_rfFrequency(fFinalFrequency, fFinalFrequency) {};

        virtual void    Start(FMOD::Channel* pChannel, uint uiNow, bool bFadeIn);
        virtual bool    Update(FMOD::Channel* pChannel, uint uiNow);
    };

    class CSoundFadeVolumeSpeed : public CSoundFade
    {
    private:
        uint            m_uiVolumeStartTime;
        uint            m_uiVolumeFadeTime;
        CRangef         m_rfVolume;
        uint            m_uiFrequencyStartTime;
        uint            m_uiFrequencyFadeTime;
        CRangef         m_rfFrequency;
    public:
        CSoundFadeVolumeSpeed(uint uiVolumeFadeTime, float fFinalVolume, uint uiFrequencyFadeTime, float fFinalFrequency) : m_uiVolumeFadeTime(uiVolumeFadeTime), m_rfVolume(fFinalVolume, fFinalVolume), m_uiFrequencyFadeTime(uiFrequencyFadeTime), m_rfFrequency(fFinalFrequency, fFinalFrequency) {};

        virtual void    Start(FMOD::Channel* pChannel, uint uiNow, bool bFadeIn);
        virtual bool    Update(FMOD::Channel* pChannel, uint uiNow);
    };

    class CSoundDampen
    {
    private:
        float           m_fDampen;

    public:
        ~CSoundDampen() {}
        CSoundDampen(float fDampen) : m_fDampen(fDampen) {}

        float   GetDampen(uint uiNow)   { return m_fDampen; }
    };

    map<SoundHandle, CSoundFade*>   m_mapFadingInSounds;    // sounds being faded in
    map<SoundHandle, CSoundFade*>   m_mapFadeOutSounds;     // sounds to fade out when stopped
    map<SoundHandle, CSoundFade*>   m_mapFadingOutSounds;   // sounds being faded out
    map<SoundHandle, CSoundDampen*> m_mapDampenSounds;      // dampens other sounds

    map<int, FMOD_VECTOR*>          m_mapRolloffCurves;     // custom rolloff curves

    map<SoundHandle, FMOD::Channel *>   m_mapActiveSounds; // TODO: Make pool
    SoundHandle                         m_ahSoundHandle[NUM_ASSIGNED_CHANNELS]; // tracking sounds that give channel to PlaySound
    FMOD::Channel   *m_pMusicChannel;
    tsvector        m_vMusicPlaylist;
    bool            m_bPlaylistActive;
    int             m_iNextPlaylistIndex;
    CSample         *m_pRecordTarget;

    FMOD::ChannelGroup  *m_pInterfaceChannelGroup;
    FMOD::ChannelGroup  *m_pSFXChannelGroup;
    FMOD::ChannelGroup  *m_pVoiceChannelGroup;
    FMOD::ChannelGroup  *m_pMusicChannelGroup;

    FMOD::Geometry      *m_pWorldGeometry;

    SoundHandle         m_hWorldAmbientSound;

    float           m_fSFXVolumeMult;
    float           m_fInterfaceVolumeMult;
    float           m_fVoiceVolumeMult;
    float           m_fMusicVolumeMult;

    float           m_fSFXVolumeDampen;
    float           m_fInterfaceVolumeDampen;
    float           m_fVoiceVolumeDampen;
    float           m_fMusicVolumeDampen;

    tstring         m_sRecordDriverName;

    SPlayMusic      m_cCurrentMusic;
    SPlayMusic      m_cNextMusic;

    typedef hash_set<FMOD::Sound*>  ReleaseSounds;
    ReleaseSounds   m_setReleaseSounds;

    uint            m_uiStopMusicDelayFrames;
    bool            m_bStopMusic_FadeOut;
    bool            m_bStopMusic_ClearPlaylist;

    void    ReleasePendingSounds();

    void    UpdateMusic();

    void    UpdateFades();
    void    UpdateDampen();

    SoundHandle PlaySound(FMOD::ChannelGroup *pGroup, ResHandle hSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume, float fFalloff, int iChannel, int iPriority, int iFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fSpreadAngle = 0.0f, float fFalloffEnd = 0.0f, float fDampen = 1.0f);
    SoundHandle PlaySound(FMOD::ChannelGroup *pGroup, CSample *pSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume, float fFalloff, int iChannel, int iPriority, int iFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fSpreadAngle = 0.0f, float fFalloffEnd = 0.0f, float fDampen = 1.0f);

    void            StartNextMusic();

public:
    ~CSoundManager();

    K2_API void     Start();
    K2_API void     Restart();
    K2_API void     Frame();
    K2_API void     Stop();

    K2_API void     StopStreamingImmediately();

    K2_API void     RefreshDrivers();
    K2_API bool     GetDriver(int iDriver, tstring &sDriverReturn);
    K2_API bool     GetRecordDriver(int iDriver, tstring &sDriverReturn);

    FMOD::Sound*    LoadSample(const tstring &sPath, int iSoundFlags);
    K2_API byte*    GetSampleData(CSample *pSample, uint uLength, uint uOffset);
    K2_API bool     GetSampleData(CSample *pSample, byte *pTarget, uint uLength, uint uOffset);
    K2_API uint     GetSampleLength(CSample *pSample);
    K2_API void     FreeSample(FMOD::Sound* pSample);
    K2_API void     ReleaseSoundNextTick(FMOD::Sound* pSample);
    K2_API FMOD::Sound* CreateSound(int iFrequency, int iNumChannels, int iNumBits, uint uiBufferSize, int iSoundFlags = 0);
    K2_API FMOD::Sound* CreateExtendedSound(FMOD_CREATESOUNDEXINFO &exInfo, int iSoundFlags = 0);

    void            ResetWorldGeometry(int iMaxPolys, int iMaxVertices, float fMaxWorldSize);
    void            AddWorldGeometry(CVec3f &a, CVec3f &b, CVec3f &c);

    K2_API CSample* StartRecording(int iFrequency, uint uiBufferSize);
    K2_API void     StopRecording();
    K2_API uint     GetRecordingPos();

    K2_API CSample* GetRecordTarget()                   { return m_pRecordTarget; }

    K2_API uint     ModifySampleAtPos(CSample *pSample, uint uPos, uint uLength, byte *pData);
    K2_API uint     ResetSampleAtPos(CSample *pSample, uint uPos, uint uLength);

    K2_API SoundHandle  Play2DSound(ResHandle hSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10, float fDampen = 1.0f);
    K2_API SoundHandle  Play2DSound(CSample *pSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10, float fDampen = 1.0f);
    K2_API SoundHandle  PlayVoiceSound(ResHandle hSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10);
    K2_API SoundHandle  PlayVoiceSound(CSample *pSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10);
    K2_API SoundHandle  PlaySFXSound(ResHandle hSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 128, int iSoundFlags = 0, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0, float fFalloffEnd = 0.0f);
    K2_API SoundHandle  PlaySFXSound(CSample *pSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 128, int iSoundFlags = 0, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0, float fFalloffEnd = 0.0f);
    K2_API SoundHandle  Play2DSFXSound(ResHandle hSample, float fVolume = 1, int iChannel = CHANNEL_AUTO, int iPriority = 128, bool bLoop = false, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
    K2_API SoundHandle  Play2DSFXSound(CSample *pSample, float fVolume = 1, int iChannel = CHANNEL_AUTO, int iPriority = 128, bool bLoop = false, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
    K2_API SoundHandle  PlayWorldSFXSound(ResHandle hSample, const CVec3f *pv3Pos, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 150, bool bLoop = false);
    K2_API SoundHandle  PlayWorldSFXSound(CSample *pSample, const CVec3f *pv3Pos, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 150, bool bLoop = false);
    K2_API void     SetListenerPosition(const CVec3f &v3Pos, const CVec3f &v3Velocity, const CVec3f &v3Forward, const CVec3f &v3Up, bool bWarp);
    K2_API void     SetPlayPosition(SoundHandle hHandle, uint uiPos);
    K2_API void     SetVolume(SoundHandle hHandle, float fVolume);
    K2_API void     SetMute(SoundHandle hHandle, bool bMute);
    K2_API void     StopChannel(int iChannel);
    K2_API void     StopHandle(SoundHandle hHandle);
    K2_API bool     UpdateHandle(SoundHandle hHandle, const CVec3f &v3Pos, const CVec3f &v3Vel, bool b3DSound = true);
    K2_API void     PlayMusic(const tstring &sStreamFile, bool bLoop, bool bCrossFade = false, bool bFromPlaylist = false);
    K2_API void     PlayPlaylist(const tsvector &vPlaylist, bool bShuffle);
    K2_API void     StopMusic(bool bFadeOut = true, bool bClearPlaylist = true, uint uiDelayFrames = 0);
    K2_API void     ClearPlaylist();
    K2_API void     MuteSFX(bool bMute);

    K2_API void     SetInterfaceVolume(float fVolume);
    K2_API void     SetSFXVolume(float fVolume);
    K2_API void     SetVoiceVolume(float fVolume);
    K2_API void     SetMusicVolume(float fVolume);

    K2_API uint         GetChannelPosition(SoundHandle hHandle);
    K2_API uint         GetTime(SoundHandle hPlaying);
    K2_API uint         GetDuration(SoundHandle hPlaying);

    K2_API const tstring&   GetCurrentMusicName();
    K2_API uint     GetCurrentMusicTime();
    K2_API uint     GetCurrentMusicDuration();
    K2_API void     SetCurrentMusicTime(int iMillisec);

    K2_API const tsvector&  GetCurrentPlaylist();
    K2_API int      GetCurrentPlaylistIdx();

    void            StopActiveSound(FMOD::Channel *pChannel);
    void            MusicStopped(FMOD::Channel *pChannel);

    void            SoundTest(uint uiMs, uint uiFreq);
    bool            GetPlaylistActive() { return m_bPlaylistActive; }

    void            SetFalloffBias(float fFalloffBias)  { m_fFalloffBias = fFalloffBias; }
    float           GetFalloffBias() const              { return m_fFalloffBias; }

    void            SetCenter(const CVec3f &v3Center)   { m_v3Center = v3Center; }
    const CVec3f&   GetCenter() const                   { return m_v3Center; }

    K2_API bool     IsHandleActive(SoundHandle hHandle);
    bool            IsChannelAvailable(int iChannel)    { return iChannel >= 0 && iChannel < NUM_ASSIGNED_CHANNELS && (m_ahSoundHandle[iChannel] == INVALID_INDEX || !IsHandleActive(m_ahSoundHandle[iChannel])); }

    K2_API tstring  GetRecordDriverName()               { return m_sRecordDriverName; }
};

#else // K2_NOSOUND

class CSoundManager
{
    SINGLETON_DEF(CSoundManager)

private:
public:
    ~CSoundManager()    {}

    void            Start() {}
    void            Frame() {}
    void            Stop()  {}

    FMOD::Sound*    LoadSample(const char *pData, uint zLength, int iSoundFlags)    { return nullptr; }
    void            FreeSample(FMOD::Sound *pSampleData)            {}

    K2_API SoundHandle  Play2DSound(ResHandle hSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10) {}
    K2_API SoundHandle  PlayVoiceSound(ResHandle hSample, float fVolume = 1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 10) {}
    K2_API SoundHandle  PlaySFXSound(ResHandle hSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 128, bool bLoop = false, int iFadeIn = 0, int iFadeOut = 0, int iPlaybackStartUpTime = 0, float fInitialPlaybackSpeed = 1, int PlaybackStopTime = 0, float fFinalPlaybackSpeed = 1) {}
    K2_API SoundHandle  Play2DSFXSound(ResHandle hSample, float fVolume = 1, int iChannel = CHANNEL_AUTO, int iPriority = 128, bool bLoop = false, int iFadeIn = 0, int iFadeOut = 0, int iPlaybackStartUpTime = 0, float fInitialPlaybackSpeed = 1, int PlaybackStopTime = 0, float fFinalPlaybackSpeed = 1) {}
    K2_API SoundHandle  PlayWorldSFXSound(ResHandle hSample, const CVec3f *pv3Pos, float fVolume = 1, float fFalloff = -1.0f, int iChannel = CHANNEL_AUTO, int iPriority = 150, bool bLoop = false) {}
    K2_API void         SetListenerPosition(const CVec3f &v3Pos, const CVec3f &v3Velocity, const CVec3f &v3Forward, const CVec3f &v3Up, bool bWarp) {}
    K2_API void         StopHandle(SoundHandle hHandle) {}
    K2_API void         PlayMusic(const tstring &sStreamFile, bool bLoop)   {}
    K2_API void         StopMusic() {}
};
#endif // K2_NOSOUND
//=============================================================================


#endif // __C_SOUNDMANAGER_H__
