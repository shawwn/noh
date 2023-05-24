// (C)2005 S2 Games
// c_soundmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#ifndef K2_NOSOUND
#include "fmod.hpp"
#include "fmod_errors.h"
#include "speex/speex_resampler.h"
#endif // K2_NOSOUND

#include "c_soundmanager.h"
#include "c_sample.h"
#include "c_uicmd.h"
#include "i_resourcelibrary.h"
#include "i_widget.h"
#include "i_listwidget.h"
#include "c_voicemanager.h"
#include "c_resourcemanager.h"
#include "c_filestream.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CSoundManager&  K2SoundManager(*CSoundManager::GetInstance());

#ifndef K2_NOSOUND
CVAR_BOOLF  (sound_disable,             false,      CVAR_SAVECONFIG);
CVAR_BOOLF  (sound_disableRecording,    false,      CVAR_SAVECONFIG);
CVAR_BOOLF  (sound_mute,                false,      CVAR_SAVECONFIG);
CVAR_BOOLF  (sound_muteMusic,           false,      CVAR_SAVECONFIG);
CVAR_STRINGF(sound_output,              "",         CVAR_SAVECONFIG);
CVAR_INTF   (sound_driver,              0,          CVAR_SAVECONFIG);
CVAR_INTF   (sound_recording_driver,    0,          CVAR_SAVECONFIG);
CVAR_BOOLF  (sound_prologic,            false,      CVAR_SAVECONFIG);
CVAR_INTR   (sound_numChannels,         128,        CVAR_VALUERANGE | CVAR_SAVECONFIG, 0, MAX_CHANNELS);
CVAR_INTF   (sound_mixrate,             44100,      CVAR_SAVECONFIG);
CVAR_FLOATF (sound_masterVolume,        1.0f,       CVAR_SAVECONFIG);
CVAR_BOOLF  (sound_debug,               false,      CONEL_DEV | CVAR_DONTSAVE);
CVAR_FLOATF (sound_musicVolume,         0.2f,       CVAR_SAVECONFIG);
CVAR_FLOATF (sound_sfxVolume,           0.6f,       CVAR_SAVECONFIG);
CVAR_FLOATF (sound_interfaceVolume,     0.7f,       CVAR_SAVECONFIG);
CVAR_BOOL   (sound_noCullLooping,       true);
CVAR_BOOL   (sound_noCull,              true);
CVAR_BOOL   (sound_geometryOcclusion,   false);
CVAR_FLOATF (sound_dopplerFactor,       0.0f,       CVAR_DONTSAVE);
CVAR_FLOATF (sound_distanceFactor,      39.36f,     CVAR_DONTSAVE);
CVAR_FLOATF (sound_rolloffScale,        1.0f,       CVAR_DONTSAVE);
CVAR_FLOATF (sound_defaultFalloff,      250.0f,     CVAR_DONTSAVE);
CVAR_FLOATF (sound_defaultMaxFalloff,   393600.0f,  CVAR_DONTSAVE);
CVAR_BOOLF  (sound_customRolloff,       false,      CVAR_DONTSAVE);
CVAR_FLOATF (sound_customRolloffFalloffMult,    10,     CVAR_DONTSAVE);
CVAR_INTR   (sound_maxVariations,       16,         CVAR_VALUERANGE | CVAR_SAVECONFIG, 1, 16);
CVAR_INTR   (sound_resampler,           2,          CVAR_VALUERANGE | CVAR_SAVECONFIG, 0, 3);
CVAR_INTF   (sound_channelsPlaying,     0,          CVAR_READONLY | CVAR_DONTSAVE);
CVAR_INTF   (sound_activeSounds,        0,          CVAR_READONLY | CVAR_DONTSAVE);
CVAR_BOOLF  (sound_useCompressedSamples,true,       CVAR_SAVECONFIG);
CVAR_FLOAT  (sound_stereoSpread,        140.0f);
CVAR_FLOAT  (sound_falloffHalfLife,     0.1f);
CVAR_STRING (sound_compressedFormat,    "ogg,mp3");

CVAR_BOOLF  (sound_downsample,          true,       CVAR_DONTSAVE);
CVAR_INTR   (sound_downsampleQuality,   3,          CVAR_VALUERANGE | CVAR_DONTSAVE, 0, 10);
CVAR_INTF   (sound_bufferSize,          -1,         CVAR_SAVECONFIG);
#endif // K2_NOSOUND

CVAR_BOOLF( sound_voiceMicMuted,                false,      CVAR_SAVECONFIG);
CVAR_FLOATF(sound_voiceChatVolume,              1.0f,       CVAR_SAVECONFIG);
EXTERN_CVAR_BOOL(fs_disablemods);

//const float sound_dopplerFactor(1.0f);
//const float sound_distanceFactor(39.36f);
//const float sound_rolloffScale(1.0f);

const unsigned int sound_musicFadeTime(2000);

#if TKTK // This seems removed as of 2023
#if defined(linux)
SpeexResamplerState *g_VoiceResampler(nullptr); // For OSS since we need to resample :(
#endif
#endif

SINGLETON_INIT(CSoundManager)
//=============================================================================

#ifdef K2_NOSOUND
/*====================
  CSoundManager::CSoundManager
  ====================*/
CSoundManager::CSoundManager()
{   
}

#else //K2_NOSOUND
void CSoundManager::CSoundFadeVolume::Start(FMOD::Channel *pChannel, uint uiNow, bool bFadeIn)
{
    float fVolume;

    m_uiStartTime = uiNow;

    assert(m_uiFadeTime >= 1);
    if (m_uiFadeTime < 1)
        m_uiFadeTime = 1;

    pChannel->getVolume(&fVolume);
    m_rfVolume.Set(fVolume, m_rfVolume);
}

bool CSoundManager::CSoundFadeVolume::Update(FMOD::Channel *pChannel, uint uiNow)
{
    if (m_uiStartTime == INVALID_TIME)
        m_uiStartTime = uiNow;

    if (uiNow > m_uiStartTime + m_uiFadeTime)
    {
        pChannel->setVolume(m_rfVolume.Lerp(1.0f));
        return false; // fade is finished
    }

    if (uiNow >= m_uiStartTime)
        pChannel->setVolume(m_rfVolume.Lerp(float(uiNow - m_uiStartTime) / (m_uiFadeTime)));

    return true;
}

void CSoundManager::CSoundFadeSpeed::Start(FMOD::Channel *pChannel, uint uiNow, bool bFadeIn)
{
    float fFrequency;

    m_uiStartTime = uiNow;

    assert(m_uiFadeTime >= 1);
    if (m_uiFadeTime < 1)
        m_uiFadeTime = 1;
    
    pChannel->getFrequency(&fFrequency);
    m_rfFrequency.Set(fFrequency, m_rfFrequency);
}

bool CSoundManager::CSoundFadeSpeed::Update(FMOD::Channel *pChannel, uint uiNow)
{
    if (m_uiStartTime == INVALID_TIME)
        m_uiStartTime = uiNow;

    if (uiNow > m_uiStartTime + m_uiFadeTime)
    {
        pChannel->setFrequency(m_rfFrequency.Lerp(1.0f));
        return false; // fade is finished
    }

    if (uiNow >= m_uiStartTime)
        pChannel->setFrequency(m_rfFrequency.Lerp(float(uiNow - m_uiStartTime) / (m_uiFadeTime)));

    return true;
}

void CSoundManager::CSoundFadeVolumeSpeed::Start(FMOD::Channel *pChannel, uint uiNow, bool bFadeIn)
{
    float fVolume, fFrequency;

    assert(m_uiVolumeFadeTime >= 1);
    if (m_uiVolumeFadeTime < 1)
        m_uiVolumeFadeTime = 1;

    assert(m_uiFrequencyFadeTime >= 1);
    if (m_uiFrequencyFadeTime < 1)
        m_uiFrequencyFadeTime = 1;

    m_uiVolumeStartTime = m_uiFrequencyStartTime = uiNow;
    if (!bFadeIn)
    {
        if (m_uiVolumeFadeTime < m_uiFrequencyFadeTime)
            m_uiVolumeStartTime += m_uiFrequencyFadeTime - m_uiVolumeFadeTime;
        else if (m_uiVolumeFadeTime > m_uiFrequencyFadeTime)
            m_uiFrequencyStartTime += m_uiVolumeFadeTime - m_uiFrequencyFadeTime;
    }
    
    pChannel->getVolume(&fVolume);
    m_rfVolume.Set(fVolume, m_rfVolume);

    pChannel->getFrequency(&fFrequency);
    m_rfFrequency.Set(fFrequency, m_rfFrequency);
}

bool CSoundManager::CSoundFadeVolumeSpeed::Update(FMOD::Channel *pChannel, uint uiNow)
{
    if (m_uiVolumeStartTime == INVALID_TIME)
        m_uiVolumeStartTime = uiNow;
    if (m_uiFrequencyStartTime == INVALID_TIME)
        m_uiFrequencyStartTime = uiNow;

    if (uiNow > m_uiVolumeStartTime + m_uiVolumeFadeTime && uiNow > m_uiFrequencyStartTime + m_uiFrequencyFadeTime)
    {
        pChannel->setVolume(m_rfVolume.Lerp(1.0f));
        pChannel->setFrequency(m_rfFrequency.Lerp(1.0f));
        return false; // fade is finished
    }

    if (uiNow >= m_uiVolumeStartTime)
        pChannel->setVolume(m_rfVolume.Lerp(MIN(1.0f, float(uiNow - m_uiVolumeStartTime) / (m_uiVolumeFadeTime))));

    if (uiNow >= m_uiFrequencyStartTime)
        pChannel->setFrequency(m_rfFrequency.Lerp(MIN(1.0f, float(uiNow - m_uiFrequencyStartTime) / (m_uiFrequencyFadeTime))));

    return true;
}


/*====================
  Sound_FMODAlloc
  ====================*/
void* F_CALLBACK    Sound_FMODAlloc(uint uiSize, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
    void *pVoid(K2_NEW_ARRAY(ctx_Sound, byte, uiSize));
    return pVoid;
}


/*====================
  Sound_FMODRealloc
  ====================*/
void* F_CALLBACK    Sound_FMODRealloc(void *ptr, uint uiSize, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
    void *pVoid(MemManager.Reallocate(ptr, uiSize));
    return pVoid;
}


/*====================
  Sound_FMODFree
  ====================*/
void F_CALLBACK Sound_FMODFree(void *ptr, FMOD_MEMORY_TYPE type, const char *sourcestr)
{
    K2_DELETE_ARRAY(static_cast<byte*>(ptr));
}


/*====================
  SInfoHandle
  ====================*/
struct SInfoHandle
{
    void* pHandle = nullptr;
    bool bIsStream = false;
};


/*====================
  Sound_CallbackFileOpen
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileOpen(const char* name, unsigned int *filesize, void **handle, void *userdata)
{
    tstring sPath(UTF8ToTString(name));

    tstring sZipFile;
    bool bStream(false);

    size_t uiPos(sPath.find(_T('<')));
    if (uiPos != tstring::npos)
    {
        bStream = true;
        sZipFile = sPath.substr(uiPos + 1);
        sPath = sPath.substr(0, uiPos);
    }

    if (!bStream)
    {
        CFileHandle *pFile = K2_NEW(ctx_Sound,  CFileHandle)(sPath, FILE_READ | FILE_BINARY | FILE_TEST);

        if (pFile == nullptr || !pFile->IsOpen())
        {
            SAFE_DELETE(pFile);

            for (auto&& sCompressedFormat : TokenizeString(sound_compressedFormat, _T(',')))
            {
                if (CompareNoCase(Filename_GetExtension(sPath), sCompressedFormat) != 0)
                {
                    // Try compressed if the current file didn't exist
                    sPath = Filename_StripExtension(sPath) + _T(".") + sCompressedFormat;

                    pFile = K2_NEW(ctx_Sound,  CFileHandle)(sPath, FILE_READ | FILE_BINARY | FILE_TEST);
                    if (pFile && pFile->IsOpen())
                        break;
                    SAFE_DELETE(pFile);
                }
            }
            if (pFile == nullptr)
                return FMOD_ERR_FILE_NOTFOUND;
        }

        *filesize = (int)pFile->GetLength();
        SInfoHandle* pInfo = new SInfoHandle();
        pInfo->pHandle = (void*)pFile;
        pInfo->bIsStream = false;
        *handle = pInfo;
    }
    else
    {
        CFileStream *pFile = K2_NEW(global,  CFileStream)();
        bool bResult(false);
        if (sZipFile.empty())
            bResult = pFile->Open(sPath, FILE_READ | FILE_BINARY | FILE_TEST | FILE_NOBUFFER);
        else
            bResult = pFile->OpenCompressed(sZipFile, sPath, FILE_READ | FILE_BINARY | FILE_TEST | FILE_NOBUFFER);

        if (!bResult)
        {
            SAFE_DELETE(pFile);

            for (auto&& sCompressedFormat : TokenizeString(sound_compressedFormat, _T(',')))
            {
                if (CompareNoCase(Filename_GetExtension(sPath), sCompressedFormat) != 0)
                {
                    // Try compressed if the current file didn't exist
                    sPath = Filename_StripExtension(sPath) + _T(".") + sCompressedFormat;

                    pFile = K2_NEW(global,  CFileStream)();
                    if (sZipFile.empty())
                        bResult = pFile->Open(sPath, FILE_READ | FILE_BINARY | FILE_TEST | FILE_NOBUFFER);
                    else
                        bResult = pFile->OpenCompressed(sZipFile, sPath, FILE_READ | FILE_BINARY | FILE_TEST | FILE_NOBUFFER);

                    if (bResult)
                        break;
                    SAFE_DELETE(pFile);
                }
            }
            if (pFile == nullptr)
                return FMOD_ERR_FILE_NOTFOUND;
        }

        *filesize = (unsigned int)pFile->GetLength();
        
        SInfoHandle* pInfo = new SInfoHandle();
        pInfo->pHandle = (void*)pFile;
        pInfo->bIsStream = true;
        *handle = pInfo;
    }

    return FMOD_OK;
}


/*====================
  Sound_CallbackFileClose
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileClose(void *handle, void *userdata)
{
    SInfoHandle* pInfo = (SInfoHandle*)handle;
    if (pInfo)
    {
        if (void* pHandle = pInfo->pHandle; pHandle)
        {
            pInfo->pHandle = nullptr;
            if (!pInfo->bIsStream)
            {
                CFileHandle *pFile((CFileHandle*)(pHandle));
                SAFE_DELETE(pFile);
            }
            else
            {
                CFileStream *pFile((CFileStream*)(pHandle));
                SAFE_DELETE(pFile);
            }
        }
        SAFE_DELETE(pInfo);
    }
    return FMOD_OK;
}


/*====================
  Sound_CallbackFileRead
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileRead(void *handle, void *buffer, unsigned int sizebytes, unsigned int *bytesread, void *userdata)
{
    *bytesread = 0;

    SInfoHandle* pInfo = (SInfoHandle*)handle;
    if (pInfo)
    {
        if (void* pHandle = pInfo->pHandle; pHandle)
        {
            if (!pInfo->bIsStream)
            {
                CFileHandle *pFile((CFileHandle*)(pHandle));

                *bytesread = pFile->Read((char*)buffer, sizebytes);
            }
            else
            {
                CFileStream *pFile((CFileStream*)(pHandle));

                *bytesread = pFile->Read((char*)buffer, sizebytes);
            }
        }
    }

    if (*bytesread < sizebytes)
        return FMOD_ERR_FILE_EOF;

    return FMOD_OK;
}


/*====================
  Sound_CallbackFileReadAsync
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileReadAsync(FMOD_ASYNCREADINFO *pInfo, void *userdata)
{
    // TKTK: Read asynchronously? See https://www.fmod.com/docs/2.00/api/white-papers-asynchronous-io.html
    FMOD_RESULT result = Sound_CallbackFileRead(pInfo->handle, pInfo->buffer, pInfo->sizebytes, &pInfo->bytesread, pInfo->userdata);
    pInfo->done(pInfo, result);
    return result;
}


/*====================
  Sound_CallbackFileCancelAsync
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileCancelAsync(FMOD_ASYNCREADINFO *pInfo, void *userdata)
{
    return Sound_CallbackFileClose(pInfo->handle, pInfo->userdata);
}


/*====================
  Sound_CallbackFileSeek
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackFileSeek(void *handle, unsigned int pos, void *userdata)
{
    SInfoHandle* pInfo = (SInfoHandle*)handle;
    if (pInfo)
    {
        if (void* pHandle = pInfo->pHandle; pHandle)
        {
            if (!pInfo->bIsStream)
            {
                CFileHandle *pFile((CFileHandle*)(pHandle));

                if (pFile->Seek(pos, SEEK_ORIGIN_START))
                    return FMOD_OK;
            }
            else
            {
                CFileStream *pFile((CFileStream*)(pHandle));

                if (pFile->Seek(pos, SEEK_ORIGIN_START))
                    return FMOD_OK;
            }
        }
    }

    return FMOD_ERR_FILE_COULDNOTSEEK;
}


/*====================
  Sound_CallbackStopActiveSound
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackStopActiveSound(FMOD_CHANNELCONTROL *pControl, FMOD_CHANNELCONTROL_TYPE controltype, FMOD_CHANNELCONTROL_CALLBACK_TYPE type, void* commanddata1, void* commanddata2)
{
    if (type == FMOD_CHANNELCONTROL_CALLBACK_END)
    {
        if (controltype == FMOD_CHANNELCONTROL_CHANNEL) {
            K2SoundManager.StopActiveSound((FMOD::Channel *)pControl);
        } else {
            FMOD::ChannelGroup *pGroup = (FMOD::ChannelGroup *)pControl;
            int iNumChannels = 0;
            pGroup->getNumChannels(&iNumChannels);
            for (int i = 0; i < iNumChannels; i++) {
                FMOD::Channel *pChannel = nullptr;
                pGroup->getChannel(0, &pChannel);
                if (pChannel) {
                    K2SoundManager.StopActiveSound(pChannel);
                }
            }
        }
    }

    return FMOD_OK;
}


/*====================
  Sound_CallbackStopActiveStream
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackStopActiveStream(FMOD_CHANNELCONTROL *pControl, FMOD_CHANNELCONTROL_TYPE controltype, FMOD_CHANNELCONTROL_CALLBACK_TYPE type, void* commanddata1, void* commanddata2)
{
    if (type == FMOD_CHANNELCONTROL_CALLBACK_END)
    {
        if (controltype == FMOD_CHANNELCONTROL_CHANNEL) {
            FMOD::Sound *pStream = nullptr;
            ((FMOD::Channel *)pControl)->getCurrentSound(&pStream);
            K2SoundManager.StopActiveSound((FMOD::Channel *)pControl);
            K2SoundManager.ReleaseSoundNextTick(pStream);
        } else {
            FMOD::ChannelGroup *pGroup = (FMOD::ChannelGroup *)pControl;
            int iNumChannels = 0;
            pGroup->getNumChannels(&iNumChannels);
            for (int i = 0; i < iNumChannels; i++) {
                FMOD::Channel *pChannel = nullptr;
                pGroup->getChannel(0, &pChannel);
                if (pChannel) {
                    FMOD::Sound *pStream = nullptr;
                    pChannel->getCurrentSound(&pStream);
                    K2SoundManager.StopActiveSound(pChannel);
                    K2SoundManager.ReleaseSoundNextTick(pStream);
                }
            }
        }
    }

    return FMOD_OK;
}


/*====================
  Sound_CallbackStopActiveMusicSample
  ====================*/
FMOD_RESULT F_CALLBACK Sound_CallbackStopActiveMusicSample(FMOD_CHANNELCONTROL *pControl, FMOD_CHANNELCONTROL_TYPE controltype, FMOD_CHANNELCONTROL_CALLBACK_TYPE type, void* commanddata1, void* commanddata2)
{
    if (type == FMOD_CHANNELCONTROL_CALLBACK_END)
    {
        if (controltype == FMOD_CHANNELCONTROL_CHANNEL) {
            K2SoundManager.MusicStopped((FMOD::Channel *)pControl);
        } else {
            FMOD::ChannelGroup *pGroup = (FMOD::ChannelGroup *)pControl;
            int iNumChannels = 0;
            pGroup->getNumChannels(&iNumChannels);
            for (int i = 0; i < iNumChannels; i++) {
                FMOD::Channel *pChannel = nullptr;
                pGroup->getChannel(0, &pChannel);
                if (pChannel) {
                    K2SoundManager.MusicStopped(pChannel);
                }
            }
        }
    }

    return FMOD_OK;
}

union floid
{
    float f;
    void *v;
};

/*====================
  Sound_Callback3dRolloff
  ====================*/
float F_CALLBACK Sound_Callback3dRolloff(FMOD_CHANNELCONTROL *Channel, float fFmodDistance)
{
    FMOD::Channel *pChannel((FMOD::Channel*)Channel);

    FMOD_MODE mode(0);
    pChannel->getMode(&mode);
    if (!(mode & FMOD_3D_LINEARROLLOFF))
        return 0.0f;

    float fMinDistance, fMaxDistance;
    pChannel->get3DMinMaxDistance(&fMinDistance, &fMaxDistance);

    CVec3f v3Pos;
    pChannel->get3DAttributes((FMOD_VECTOR *)&v3Pos, nullptr);

    // Read previous rolloff from userdata
    void *pUserData;
    pChannel->getUserData(&pUserData);

    bool bMute;
    pChannel->getMute(&bMute);

    float fRolloff;
    floid u;

    if (bMute)
    {
        fRolloff = 0.0f;
    }
    else
    {
        u.v = pUserData;

        float fDistanceSq(DistanceSq(v3Pos.xy(), K2SoundManager.GetCenter().xy()));

        if (fDistanceSq > SQR(fMaxDistance))
            fRolloff = 0.0f;
        else if (fDistanceSq < SQR(fMinDistance))
            fRolloff = 1.0f;
        else
            fRolloff = 1.0f - ((sqrt(fDistanceSq) - fMinDistance) / (fMaxDistance - fMinDistance));

        fRolloff = DECAY(u.f, fRolloff, sound_falloffHalfLife, MsToSec(Host.GetFrameLength()));

        if (fRolloff < 0.001f)
            fRolloff = 0.0f;
    }

    // Save current rolloff into userdata
    u.f = fRolloff;
    pChannel->setUserData(u.v);

    return fRolloff;
}


/*====================
  CSoundManager::~CSoundManager
  ====================*/
CSoundManager::~CSoundManager()
{
    Stop();
}


/*====================
  CSoundManager::CSoundManager
  ====================*/
CSoundManager::CSoundManager() :
m_pFMODSystem(nullptr),

m_pResamplerMono(nullptr),
m_pResamplerStereo(nullptr),

m_bInitialized(false),
m_uiFailureCount(0),

m_iNumDrivers(0),
m_iNumRecordingDrivers(0),
m_iRecordingDriver(0),

m_iLoopFrame(0),
m_uiHandleCounter(0),
m_iNumLoopingSounds(0),

m_pMusicChannel(nullptr),
m_bPlaylistActive(false),
m_pRecordTarget(nullptr),

m_pWorldGeometry(nullptr),

m_fFalloffBias(0.0f),
m_v3Center(V3_ZERO),

m_fSFXVolumeMult(1.0f),
m_fInterfaceVolumeMult(1.0f),
m_fVoiceVolumeMult(1.0f),
m_fMusicVolumeMult(1.0f),

m_fSFXVolumeDampen(1.0f),
m_fInterfaceVolumeDampen(1.0f),
m_fVoiceVolumeDampen(1.0f),
m_fMusicVolumeDampen(1.0f),
m_uiStopMusicDelayFrames(0),
m_bStopMusic_FadeOut(false),
m_bStopMusic_ClearPlaylist(false)
{
    for (int i(0); i < NUM_ASSIGNED_CHANNELS; ++i)
        m_ahSoundHandle[i] = INVALID_INDEX;
}


/*====================
  CSoundManager::Start
  ====================*/
void    CSoundManager::Start()
{
    if (sound_disable)
        return;

    FMOD_RESULT result;
    tstring     sError;

    try
    {
        // Reset failure count
        m_uiFailureCount = 0;
        
#ifdef K2_TRACK_MEM
        // Tell FMOD to use our custom memory management
        result = FMOD::Memory_Initialize(nullptr, 0, Sound_FMODAlloc, Sound_FMODRealloc, Sound_FMODFree);
#else
        result = FMOD::Memory_Initialize(nullptr, 0, nullptr, nullptr, nullptr);
#endif
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("FMOD::Memory_Initialize() failed: ") + sError);

        // Create and initialize the main FMOD object
        result = FMOD::System_Create(&m_pFMODSystem);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK || m_pFMODSystem == nullptr)
            EX_ERROR(_T("FMOD::System_Create() failed: ") + sError);

        // Set sound output
#if defined(linux)
#if TKTK // This seems removed as of 2023
        if (_tcsicmp(sound_output.c_str(), _T("oss")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_OSS);
        else if (_tcsicmp(sound_output.c_str(), _T("ossv4")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_OSSV4);
        else if (_tcsicmp(sound_output.c_str(), _T("esd")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_ESD);
#endif
        if (_tcsicmp(sound_output.c_str(), _T("pulseaudio")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_PULSEAUDIO);
        else if ((_tcsicmp(sound_output.c_str(), _T("alsa")) == 0))
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
#elif defined(_WIN32)
#if TKTK // This seems removed as of 2023
        if (_tcsicmp(sound_output.c_str(), _T("dsound")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_DSOUND);
        else if (_tcsicmp(sound_output.c_str(), _T("winmm")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_WINMM);
        else if (_tcsicmp(sound_output.c_str(), _T("openal")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_OPENAL);
#endif
        if (_tcsicmp(sound_output.c_str(), _T("asio")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
        else if (_tcsicmp(sound_output.c_str(), _T("wasapi")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_WASAPI);
#elif defined(__APPLE__)
#if TKTK // Seems to be removed as of 2023
        if (_tcsicmp(sound_output.c_str(), _T("soundmanager")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_OSS);
#endif
        if (_tcsicmp(sound_output.c_str(), _T("coreaudio")) == 0)
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
#endif
        else
            result = m_pFMODSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("Couldn't set output: ") + sError);

        FMOD_OUTPUTTYPE output;
        m_pFMODSystem->getOutput(&output);

        Console << _T("Using ");
        switch (output)
        {
#if TKTK // These seem to be removed as of 2023
        case FMOD_OUTPUTTYPE_DSOUND: 
            Console << _T("DirectSound");
            break;
        case FMOD_OUTPUTTYPE_WINMM:
            Console << _T("Windows Multimedia");
            break;
        case FMOD_OUTPUTTYPE_OPENAL:
            Console << _T("OpenAL");
            break;
        case FMOD_OUTPUTTYPE_SOUNDMANAGER:
            Console << _T("SoundManager");
            break;
#endif
        case FMOD_OUTPUTTYPE_WASAPI:
            Console << _T("Windows Audio Session API");
            break;
        case FMOD_OUTPUTTYPE_ASIO:
            Console << _T("Low latency ASIO driver");
            break;
#if defined(linux)
        case FMOD_OUTPUTTYPE_ALSA:
            Console << _T("Advanced Linux Sound Architecture");
            break;
        case FMOD_OUTPUTTYPE_PULSEAUDIO:
            Console << _T("PulseAudio");
            break;
#endif
        case FMOD_OUTPUTTYPE_COREAUDIO:
            Console << _T("CoreAudio");
            break;
        default:
            Console << _T("Unknown (") << output << _T(")") << newl;
        }
        Console << _T(" for sound output.") << newl;

        char szDriverName[1024];
        tstring sDriverName;

        // Enumerate drivers
        result = m_pFMODSystem->getNumDrivers(&m_iNumDrivers);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("Couldn't get driver count from FMOD: ") + sError);

        Console << _T("FMOD found ") << m_iNumDrivers << _T(" sound drivers: ") << newl;
        for (int i(0); i < m_iNumDrivers; ++i)
        {
            m_pFMODSystem->getDriverInfo(i, szDriverName, 1024, nullptr, nullptr, nullptr, nullptr);
            StrToTString(sDriverName, szDriverName);
            Console << XtoA(i, 0, 2) << _T(": ") << sDriverName << newl;
            ICvar::CreateString(_T("sound_driver") + XtoA(i, FMT_PADZERO, 2), sDriverName);
        }

        if (sound_driver >= m_iNumDrivers) sound_driver = 0;
        result = m_pFMODSystem->setDriver(sound_driver);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("Couldn't set driver: ") + sError);
        
        FMOD_SPEAKERMODE speakermode = FMOD_SPEAKERMODE_DEFAULT;
        m_pFMODSystem->getDriverInfo(sound_driver, szDriverName, 1024, nullptr, nullptr, &speakermode, nullptr);
        StrToTString(sDriverName, szDriverName);
        Console << _T("Using driver ") << XtoA(sound_driver) << ": " << sDriverName << newl;
        
        FMOD_SOUND_FORMAT format(FMOD_SOUND_FORMAT_PCM16);
        if (sDriverName == _T("SigmaTel"))   // Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it.
            format = FMOD_SOUND_FORMAT_PCMFLOAT;
        
        //         FMOD_RESULT F_API setSoftwareFormat      (int samplerate, FMOD_SOUND_FORMAT format, int numoutputchannels, int maxinputchannels, FMOD_DSP_RESAMPLER resamplemethod);

//        result = m_pFMODSystem->setSoftwareFormat(sound_mixrate, format, 0, 2, resampler);
        
//        FMOD_RESULT F_API setSoftwareFormat       (int samplerate, FMOD_SPEAKERMODE speakermode, int numrawspeakers);
        result = m_pFMODSystem->setSoftwareFormat(sound_mixrate, speakermode, 2);
        if (result != FMOD_OK)
        {
            StrToTString(sError, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::setSoftwareFormat failed: ") + sError);
        }

        result = m_pFMODSystem->setSoftwareChannels(sound_numChannels);
        if (result != FMOD_OK)
        {
            StrToTString(sError, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::setSoftwareChannels failed: ") + sError);
        }
        
        result = m_pFMODSystem->init(768, FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL, nullptr);
        if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
        {
            // selected speaker mode not supported, fall back to stereo
#if TKTK // Seems removed as of 2023
            m_pFMODSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
#endif
            result = m_pFMODSystem->init(768, FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL, nullptr);
        }
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("FMOD system init failed: ") + sError);

        // Enumerate recording drivers
        int iNumConnectedDrivers = 0; // TKTK 2023: Do we need to make any changes to support this?
        result = m_pFMODSystem->getRecordNumDrivers(&m_iNumRecordingDrivers, &iNumConnectedDrivers);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_ERROR(_T("Couldn't get recording driver count from FMOD: ") + sError);

        if (m_iNumRecordingDrivers == 0)
        {
            Console << _T("No recording drivers found.") << newl;
        }
        else
        {
            Console << _T("FMOD found ") << m_iNumRecordingDrivers << _T(" sound recording drivers: ") << newl;
            for (int i(0); i < m_iNumRecordingDrivers; ++i)
            {
                m_pFMODSystem->getRecordDriverInfo(i, szDriverName, 1024, nullptr, nullptr, nullptr, nullptr, nullptr);
                StrToTString(sDriverName, szDriverName);
                Console << XtoA(i, 0, 2) << _T(": ") << sDriverName << newl;
                ICvar::CreateString(_T("sound_recording_driver") + XtoA(i, FMT_PADZERO, 2), sDriverName);
            }

            if (sound_recording_driver >= m_iNumRecordingDrivers) sound_recording_driver = 0;
            m_iRecordingDriver = sound_recording_driver;
            m_pFMODSystem->getRecordDriverInfo(sound_recording_driver, szDriverName, 1024, nullptr, nullptr, nullptr, nullptr, nullptr);
            StrToTString(sDriverName, szDriverName);
            Console << _T("Using recording driver ") << XtoA(sound_recording_driver) << ": " << sDriverName << newl;

            m_sRecordDriverName = sDriverName;
        }

#if TKTK // seems to be removed as of 2023
        // Set speakermode based on what's set in the control panel
        FMOD_SPEAKERMODE speakermode;
        FMOD_CAPS caps;
        result = m_pFMODSystem->getDriverCaps(sound_driver, &caps, nullptr, nullptr, &speakermode);
        if (sound_prologic)
            speakermode = FMOD_SPEAKERMODE_PROLOGIC;
        if (result == FMOD_OK || speakermode == FMOD_SPEAKERMODE_PROLOGIC)
            result = m_pFMODSystem->setSpeakerMode(speakermode);
        StrToTString(sError, FMOD_ErrorString(result));
        if (result != FMOD_OK)
            EX_WARN(_T("Couldn't set speaker mode: ") + sError);

        m_pFMODSystem->getSpeakerMode(&speakermode);
#endif
        
        Console << _T("Speaker mode set to ");
        switch (speakermode)
        {
        case FMOD_SPEAKERMODE_DEFAULT:
            Console << _T("Default") << newl;
            break;
        case FMOD_SPEAKERMODE_RAW:
            Console << _T("Raw") << newl;
            break;
        case FMOD_SPEAKERMODE_MONO:
            Console << _T("Mono") << newl;
            break;
        case FMOD_SPEAKERMODE_STEREO:
            Console << _T("Stereo") << newl;
            break;
        case FMOD_SPEAKERMODE_QUAD:
            Console << _T("Quadraphonic") << newl;
            break;
        case FMOD_SPEAKERMODE_SURROUND:
            Console << _T("5 Speaker Surround") << newl;
            break;
        case FMOD_SPEAKERMODE_5POINT1:
            Console << _T("5.1 Surround") << newl;
            break;
            case FMOD_SPEAKERMODE_7POINT1:
                Console << _T("7.1 Surround") << newl;
                break;
            case FMOD_SPEAKERMODE_7POINT1POINT4:
                Console << _T("7.1.4 Surround") << newl;
                break;
#if TKTK // Seems removed as of 2023
        case FMOD_SPEAKERMODE_PROLOGIC:
            Console << _T("Prologic") << newl;
            break;
#endif
        case FMOD_SPEAKERMODE_MAX:
        case FMOD_SPEAKERMODE_FORCEINT:
            K2_UNREACHABLE();
            break;
        }

#if TKTK // This seems removed as of 2023
#ifdef _WIN32
        if (caps & FMOD_CAPS_HARDWARE_EMULATED)
        {
            // Acceleration slider set to off - increase buffer size to avoid stuttering
            m_pFMODSystem->setDSPBufferSize(1024, 10);
        }
#endif
#endif

        FMOD_DSP_RESAMPLER resampler(FMOD_DSP_RESAMPLER_LINEAR);
        switch (sound_resampler)
        {
        case 0:
            resampler = FMOD_DSP_RESAMPLER_NOINTERP;
            break;
        case 1:
            resampler = FMOD_DSP_RESAMPLER_LINEAR;
            break;
        case 2:
#ifdef linux
            // getting tons of crashes with the cubic resampler under linux (crashes in FMOD_Resampler_Cubic)
            resampler = FMOD_DSP_RESAMPLER_LINEAR;
#else
            resampler = FMOD_DSP_RESAMPLER_CUBIC;
#endif
            break;
        case 3:
            resampler = FMOD_DSP_RESAMPLER_SPLINE;
            break;
        }

        result = m_pFMODSystem->setFileSystem(Sound_CallbackFileOpen, Sound_CallbackFileClose, Sound_CallbackFileRead, Sound_CallbackFileSeek,
#if TKTK // TODO: read async?
                                              Sound_CallbackFileReadAsync, Sound_CallbackFileCancelAsync,
#else
                                              nullptr, nullptr,
#endif
                                              -1);
        if (result != FMOD_OK)
        {
            StrToTString(sError, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::setFileSystem failed: ") + sError);
        }
        
        if (sound_bufferSize > 0)
        {
            int iBufferLength, iNumBuffers;
            if (sound_bufferSize >= 4096)
            {
                iBufferLength = 1024;
                iNumBuffers = 4;
                while (iNumBuffers * iBufferLength < sound_bufferSize)
                    iNumBuffers *= 2;
            }
            else
            {
                iBufferLength = 128;
                iNumBuffers = 4;
                while (iNumBuffers * iBufferLength < sound_bufferSize)
                    iBufferLength *= 2;
            }
            sound_bufferSize = iBufferLength * iNumBuffers;
            result = m_pFMODSystem->setDSPBufferSize(iBufferLength, iNumBuffers);
            if (result != FMOD_OK)
                Console << _T("Error setting sound buffer size ") << ParenStr(UTF8ToTString(FMOD_ErrorString(result))) << _T(". Using default buffer size.") << newl;
        }

#if 0
        FMOD_ADVANCEDSETTINGS settings = {
            sizeof(FMOD_ADVANCEDSETTINGS),
            sound_useCompressedSamples ? sound_numChannels - 1 : 0, // number of mpeg compressed channels. Each one takes 29,424 bytes memory
            0, 0, 0, nullptr };
        result = m_pFMODSystem->setAdvancedSettings(&settings);
        if (result != FMOD_OK)
        {
            StrToTString(sError, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::setAdvancedSettings failed: ") + sError);
        }
#endif

        result = m_pFMODSystem->set3DSettings(sound_dopplerFactor, sound_distanceFactor, sound_rolloffScale);
        if (result != FMOD_OK)
        {
            StrToTString(sError, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::set3DSettings failed: ") + sError);
        }

        // Output info
        uint uiVersion(0);
        m_pFMODSystem->getVersion(&uiVersion);
        Console << _T("Loaded FMOD version: ")
                << XtoA(uiVersion >> 16, FMT_NOPREFIX, 0, 16) << _T(".")
                << XtoA((uiVersion >> 8) & 0xff, FMT_PADZERO | FMT_NOPREFIX, 2, 16) << _T(".")
                << XtoA(uiVersion & 0xff, FMT_PADZERO | FMT_NOPREFIX, 2, 16) << newl;

        // Create channel groups for interface, SFX, and voice sounds and add them to the master group
        FMOD::ChannelGroup* pMasterChannelGroup;
        m_pFMODSystem->getMasterChannelGroup(&pMasterChannelGroup);
        m_pFMODSystem->createChannelGroup("interface", &m_pInterfaceChannelGroup);
        pMasterChannelGroup->addGroup(m_pInterfaceChannelGroup);
        m_pFMODSystem->createChannelGroup("sfx", &m_pSFXChannelGroup);
        pMasterChannelGroup->addGroup(m_pSFXChannelGroup);
        m_pFMODSystem->createChannelGroup("voice", &m_pVoiceChannelGroup);
        pMasterChannelGroup->addGroup(m_pVoiceChannelGroup);
        m_pFMODSystem->createChannelGroup("music", &m_pMusicChannelGroup);
        pMasterChannelGroup->addGroup(m_pMusicChannelGroup);

        SetListenerPosition(V_ZERO, V_ZERO, CVec3f(0.0f, 1.0f, 0.0f), CVec3f(0.0f, 0.0f, 1.0f), true);

        m_pFMODSystem->set3DRolloffCallback(Sound_Callback3dRolloff);

        m_pResamplerMono = speex_resampler_init(1, 44100, sound_mixrate, sound_downsampleQuality, nullptr);
        m_pResamplerStereo = speex_resampler_init(2, 44100, sound_mixrate, sound_downsampleQuality, nullptr);
        if (!m_pResamplerMono || !m_pResamplerStereo)
        {
            Console.Warn << _T("Sound downsampling disabled.") << newl;
            sound_downsample = false; // disable downsampling
        }

#if TKTK // This seems removed as of 2023
#if defined(linux)
        if (output == FMOD_OUTPUTTYPE_OSS)
        {
            // Set up the resampler used to convert from output/input rate -> voice sample rate
            int iRate, iChannels;
            m_pFMODSystem->getSoftwareFormat(&iRate, nullptr, &iChannels, nullptr, nullptr, nullptr);
            g_VoiceResampler = speex_resampler_init(1, iRate, VOICE_SAMPLE_RATE, 3, nullptr);
            speex_resampler_set_input_stride(g_VoiceResampler, iChannels);
        }
#endif
#endif

        m_bInitialized = true;

        Frame();
        return;
    }
    catch (CException &ex)
    {
        ex.Process(_T("SoundManager::Start() - "), NO_THROW);
        if (m_pFMODSystem != nullptr)
            m_pFMODSystem->release();
    }
}

/*====================
  CSoundManager::Restart
  ====================*/
void    CSoundManager::Restart()
{
    void *pWorldGeometry(nullptr);
    int size(0);

    // save current world geometry
    if (!sound_disable && m_pWorldGeometry && m_pWorldGeometry->save(nullptr, &size) == FMOD_OK)
    {
        pWorldGeometry = K2_NEW_ARRAY(ctx_Sound, byte, size);
        m_pWorldGeometry->save(pWorldGeometry, &size);
    }

    Stop();

    if (sound_disable)
        return;

    Start();

    // reload world geometry
    if (pWorldGeometry)
    {
        m_pFMODSystem->loadGeometry(pWorldGeometry, size, &m_pWorldGeometry);
        K2_DELETE_ARRAY(static_cast<byte*>(pWorldGeometry));
    }

    g_ResourceManager.GetLib(RES_SAMPLE)->ReloadAll();

    Frame();
    
    VoiceManager.Restart();
}


/*====================
  CSoundManager::Frame
  ====================*/
void    CSoundManager::Frame()
{
    if (!m_bInitialized)
        return;

    PROFILE("CSoundManager::Frame");

    try
    {
        ReleasePendingSounds();

        // Stop music (delayed)
        if (m_uiStopMusicDelayFrames > 0)
        {
            --m_uiStopMusicDelayFrames;
            if (m_uiStopMusicDelayFrames == 0)
                StopMusic(m_bStopMusic_FadeOut, m_bStopMusic_ClearPlaylist);
        }

        // Set master volume
        FMOD::ChannelGroup *pChannelGroup;
        if (m_pFMODSystem->getMasterChannelGroup(&pChannelGroup) == FMOD_OK)
        {
            pChannelGroup->setPitch(host_timeScale);
            pChannelGroup->setVolume(sound_mute ? 0.0f : sound_masterVolume);
        }
        
        UpdateDampen();

        // Set SFX/Interface/Voice volumes
        m_pSFXChannelGroup->setVolume(sound_sfxVolume * m_fSFXVolumeMult * m_fSFXVolumeDampen);
        m_pInterfaceChannelGroup->setVolume(sound_interfaceVolume * m_fInterfaceVolumeMult * m_fInterfaceVolumeDampen);
        m_pVoiceChannelGroup->setVolume(sound_voiceChatVolume * m_fVoiceVolumeMult * m_fVoiceVolumeDampen);

        if (m_pWorldGeometry)
            m_pWorldGeometry->setActive(sound_geometryOcclusion);

        // Process the music stream
        UpdateMusic();

        // Update sounds that are fading in/out
        UpdateFades();

        if (sound_dopplerFactor.IsModified() || sound_rolloffScale.IsModified() || sound_distanceFactor.IsModified())
        {
            m_pFMODSystem->set3DSettings(sound_dopplerFactor, sound_distanceFactor, sound_rolloffScale);
            sound_dopplerFactor.SetModified(false);
            sound_distanceFactor.SetModified(false);
            sound_rolloffScale.SetModified(false);
        }

        m_v3Jitter = CVec3f(M_Randnum(0.0f, 0.01f));

        // Update FMOD
        FMOD_RESULT result(m_pFMODSystem->update());
        if (result != FMOD_OK)
        {
            tstring sErr;
            StrToTString(sErr, FMOD_ErrorString(result));
            EX_ERROR(_T("FMOD::System::update() failed: ") + sErr);
        }

        int iChannelsPlaying(0);
        m_pFMODSystem->getChannelsPlaying(&iChannelsPlaying);

        sound_channelsPlaying = iChannelsPlaying;
        sound_activeSounds = INT_SIZE(m_mapActiveSounds.size());
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundManager::Frame() - "), NO_THROW);
        ++m_uiFailureCount;
        if (m_uiFailureCount >= SOUNDMANAGER_FAILURE_THRESHOLD)
            Stop();
    }
}


/*====================
  CSoundManager::Stop
  ====================*/
void    CSoundManager::Stop()
{
    Console << _T("Shutting down CSoundManager...") << newl;

    if (!m_bInitialized)
        return;
    
    // Stop recording
    VoiceManager.Stop();

    // Stop all music streaming
    StopStreamingImmediately();

    // stop all sounds
    FMOD::SoundGroup *pSounds;
    m_pFMODSystem->getMasterSoundGroup(&pSounds);
    for (map<SoundHandle, FMOD::Channel *>::iterator it(m_mapActiveSounds.begin()); it != m_mapActiveSounds.end(); it++)
    {
        it->second->setCallback(nullptr);
    }
    pSounds->stop();
    m_mapActiveSounds.clear();

    // and unload the fmod samples so that we aren't leaking memory
    g_ResourceManager.GetLib(RES_SAMPLE)->FreeAll();

    // free any streams that were in use since they weren't freed via callback
    int numsounds;
    pSounds->getNumSounds(&numsounds);
    while (numsounds)
    {
        FMOD::Sound *pSound;
        pSounds->getSound(0, &pSound);
        pSound->release();
        pSounds->getNumSounds(&numsounds);
    }

    m_pMusicChannel = nullptr;

    // and free channel groups
    m_pSFXChannelGroup->release();
    m_pInterfaceChannelGroup->release();
    m_pVoiceChannelGroup->release();
    m_pMusicChannelGroup->release();
    
    ReleasePendingSounds();

    // and custom rolloff curves
    for (map<int, FMOD_VECTOR*>::iterator it(m_mapRolloffCurves.begin()); it != m_mapRolloffCurves.end(); it++)
        K2_DELETE_ARRAY(it->second);
    m_mapRolloffCurves.clear();

    // and geometry
    if (m_pWorldGeometry)
    {
        m_pWorldGeometry->release();
        m_pWorldGeometry = nullptr;
    }

    if (m_pResamplerMono)
    {
        speex_resampler_destroy(m_pResamplerMono);
        m_pResamplerMono = nullptr;
    }

    if (m_pResamplerStereo)
    {
        speex_resampler_destroy(m_pResamplerStereo);
        m_pResamplerStereo = nullptr;
    }

#if TKTK // This seems removed as of 2023
#if defined(linux)
    if (g_VoiceResampler)
    {
        speex_resampler_destroy(g_VoiceResampler);
        g_VoiceResampler = nullptr;
    }
#endif
#endif

    if (m_pFMODSystem != nullptr)
    {
        m_pFMODSystem->release();
        m_pFMODSystem = nullptr;
    }

    m_bInitialized = false;
    //sound_disable = true;
}


/*====================
  CSoundManager::StopStreamingImmediately
  ====================*/
void    CSoundManager::StopStreamingImmediately()
{
    Console << _T("CSoundManager stopping any active music streaming...") << newl;

    if (m_pMusicChannel != nullptr)
    {
        m_pMusicChannel->stop();
        assert(m_pMusicChannel == nullptr);
        m_pMusicChannel = nullptr;
    }

    ReleasePendingSounds();
}


/*====================
  CSoundManager::RefreshDrivers
  ====================*/
void    CSoundManager::RefreshDrivers()
{
    FMOD::System *pSystem;
    FMOD_RESULT result;

    if (FMOD::System_Create(&pSystem) != FMOD_OK)
        return;

    // Set sound output
#ifdef linux
    if (_tcsicmp(sound_output.c_str(), _T("pulseaudio")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_PULSEAUDIO);
    else if ((_tcsicmp(sound_output.c_str(), _T("alsa")) == 0))
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
#elif defined(_WIN32)
#if TKTK // Seems removed as of 2023
    if (_tcsicmp(sound_output.c_str(), _T("dsound")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_DSOUND);
    else if (_tcsicmp(sound_output.c_str(), _T("winmm")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_WINMM);
    else if (_tcsicmp(sound_output.c_str(), _T("openal")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_OPENAL);
#endif
    if (_tcsicmp(sound_output.c_str(), _T("asio")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
    else if (_tcsicmp(sound_output.c_str(), _T("wasapi")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_WASAPI);
#elif defined(__APPLE__)
#if TKTK // Seems removed as of 2023
    if (_tcsicmp(sound_output.c_str(), _T("soundmanager")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_OSS);
#endif
    if (_tcsicmp(sound_output.c_str(), _T("coreaudio")) == 0)
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_ALSA);
#endif
    else
        result = pSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
    if (result != FMOD_OK)
    {
        pSystem->release();
        m_iNumDrivers = m_iNumRecordingDrivers = 0;
        return;
    }
    
    char szDriverName[1024];
    tstring sDriverName;

    if (pSystem->getNumDrivers(&m_iNumDrivers) != FMOD_OK)
    {
        m_iNumDrivers = 0;
    }
    else
    {
        for (int i(0); i < m_iNumDrivers; ++i)
        {
            if (pSystem->getDriverInfo(i, szDriverName, 1024, nullptr, nullptr, nullptr, nullptr) != FMOD_OK)
                continue;
            StrToTString(sDriverName, szDriverName);
            ICvar::CreateString(_T("sound_driver") + XtoA(i, FMT_PADZERO, 2), sDriverName);
        }
    }

    int iNumConnected = 0; // TKTK 2023: Do we need to make any changes to support this new parameter?
    if (pSystem->getRecordNumDrivers(&m_iNumRecordingDrivers, &iNumConnected) != FMOD_OK)
    {
        m_iNumRecordingDrivers = 0;
    }
    else
    {
        for (int i(0); i < m_iNumRecordingDrivers; ++i)
        {
            if (pSystem->getRecordDriverInfo(i, szDriverName, 1024, nullptr, nullptr, nullptr, nullptr, nullptr) != FMOD_OK)
                continue;
            StrToTString(sDriverName, szDriverName);
            ICvar::CreateString(_T("sound_recording_driver") + XtoA(i, FMT_PADZERO, 2), sDriverName);
        }
    }

    pSystem->release();
}


/*====================
  CSoundManager::GetDriver
  ====================*/
bool    CSoundManager::GetDriver(int iDriver, tstring &sDriverReturn)
{
    if (iDriver >= m_iNumDrivers)
        return false;

    sDriverReturn = ICvar::GetString(_T("sound_driver") + XtoA(iDriver, FMT_PADZERO, 2));
    return true;
}


/*====================
  CSoundManager::GetRecordDriver
  ====================*/
bool    CSoundManager::GetRecordDriver(int iDriver, tstring &sDriverReturn)
{
    if (iDriver >= m_iNumRecordingDrivers)
        return false;

    sDriverReturn = ICvar::GetString(_T("sound_recording_driver") + XtoA(iDriver, FMT_PADZERO, 2));
    return true;
}


/*====================
  CSoundManager::StartRecording
  ====================*/
CSample *CSoundManager::StartRecording(int iFrequency, uint uiBufferSize)
{
    bool bRecording;
    FMOD_RESULT result;
    tstring sError;
    FMOD::Sound *pNewSound;
    CSample *pNewSample;
    
    if (sound_disableRecording)
        return nullptr;

    if (!m_bInitialized)
    {
        Console.Warn << _T("CSoundManager::StartRecording() - CSoundManager has not been initialized") << newl;
        return nullptr;
    }

    if (m_iNumRecordingDrivers == 0)
    {
        Console.Warn << _T("CSoundManager::StartRecording() - No recording device present") << newl;
        return nullptr;
    }

    result = m_pFMODSystem->isRecording(m_iRecordingDriver, &bRecording);
    
    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::StartRecording() - FMOD::System::isRecording failed: ") << sError << newl;
        return nullptr;
    }

    if (bRecording)
        return m_pRecordTarget;

#if TKTK // This seems removed as of 2023
#ifdef linux
    FMOD_OUTPUTTYPE output;
    if (m_pFMODSystem->getOutput(&output),output == FMOD_OUTPUTTYPE_OSS)
    {
        // OSS shares input and output settings so we need to use them and then do some resampling and stuff later...
        int iRate, iChannels;
        m_pFMODSystem->getSoftwareFormat(&iRate, nullptr, &iChannels, nullptr, nullptr, nullptr);
        uiBufferSize = static_cast<uint>(static_cast<unsigned long long>(uiBufferSize) * iRate * iChannels / iFrequency);
        pNewSound = K2SoundManager.CreateSound(iRate, iChannels, SOUND_SAMPLE_RATE, uiBufferSize, SND_LOOP | SND_2D);
    }
    else
#endif
#endif
    pNewSound = K2SoundManager.CreateSound(iFrequency, 1, SOUND_SAMPLE_RATE, uiBufferSize, SND_LOOP | SND_2D);
    pNewSample = K2_NEW(ctx_Sound,  CSample)(pNewSound);

    result = m_pFMODSystem->recordStart(m_iRecordingDriver, pNewSound, true);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::StartRecording() - FMOD::System::recordStart failed: ") << sError << newl;
        Console.Err << XtoA(m_iRecordingDriver) << _T(",") << XtoA(pNewSound) << newl;
        return nullptr;
    }

    m_pRecordTarget = pNewSample;

    return pNewSample;
}


/*====================
  CSoundManager::StopRecording
  ====================*/
void    CSoundManager::StopRecording()
{
    bool bRecording;
    FMOD_RESULT result;
    tstring sError;
    
    if (sound_disableRecording)
        return;

    if (!m_bInitialized)
    {
        Console.Warn << _T("CSoundManager::StopRecording() - CSoundManager has not been initialized") << newl;
        return;
    }

    if (m_iNumRecordingDrivers == 0)
    {
        Console.Warn << _T("CSoundManager::StopRecording() - No recording device present") << newl;
        return;
    }

    result = m_pFMODSystem->isRecording(m_iRecordingDriver, &bRecording);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::StopRecording() - FMOD::System::isRecording failed: ") << sError << newl;
        return;
    }

    if (!bRecording)
        return;

    result = m_pFMODSystem->recordStop(m_iRecordingDriver);
    m_pRecordTarget = nullptr;
    
    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::StopRecording() - FMOD::System::recordStart failed: ") << sError << newl;
    }
}


/*====================
  CSoundManager::GetRecordingPos
  ====================*/
uint    CSoundManager::GetRecordingPos()
{
    uint uPos;
    tstring sError;
    FMOD_RESULT result;

    result = (m_pFMODSystem->getRecordPosition(m_iRecordingDriver, &uPos));

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetRecordingPos() - FMOD::System::getRecordPosition failed: ") << sError << newl;
        return 0;
    }

#if TKTK // This seems removed as of 2023
#ifdef linux
    FMOD_OUTPUTTYPE output;
    if (m_pFMODSystem->getOutput(&output),output == FMOD_OUTPUTTYPE_OSS)
    {
        // OSS shares input and output settings so we need to use them and then do some resampling and stuff later...
        int iRate;
        m_pFMODSystem->getSoftwareFormat(&iRate, nullptr, nullptr, nullptr, nullptr, nullptr);
        uPos = static_cast<uint>(static_cast<unsigned long long>(uPos) * VOICE_SAMPLE_RATE / iRate);
    }
#endif
#endif
    uPos *= 2;

    return uPos;
}


/*====================
  CSoundManager::GetSampleLength
  ====================*/
uint    CSoundManager::GetSampleLength(CSample *pSample)
{
    uint uLength;
    tstring sError;
    FMOD_RESULT result;

    result = pSample->GetSampleData()->getLength(&uLength, FMOD_TIMEUNIT_PCMBYTES);

#if TKTK // This seems removed as of 2023
#ifdef linux
    FMOD_OUTPUTTYPE output;
    if (pSample == m_pRecordTarget && (m_pFMODSystem->getOutput(&output),output == FMOD_OUTPUTTYPE_OSS))
    {
        // OSS shares input and output settings so we need to use them and then do some resampling and stuff later...
        int iRate, iChannels;
        m_pFMODSystem->getSoftwareFormat(&iRate, nullptr, &iChannels, nullptr, nullptr, nullptr);
        uLength = static_cast<uint>(static_cast<unsigned long long>(uLength) * VOICE_SAMPLE_RATE / (iRate * iChannels));
    }
#endif
#endif
    
    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetSampleLength() - FMOD::Sound::getLength failed: ") << sError << newl;

        return 0;
    }

    return uLength;
}


/*====================
  CSoundManager::GetSampleData

  Gets raw data from a sample, putting it in a buffer.
  ====================*/
byte    *CSoundManager::GetSampleData(CSample *pSample, uint uLength, uint uOffset)
{
    byte *sReadData;
    tstring sError;
    FMOD_RESULT result;
    unsigned int len1, len2;
    void *ptr1, *ptr2;

    sReadData = K2_NEW_ARRAY(ctx_Sound, byte, uLength);

    result = pSample->GetSampleData()->lock(uOffset, uLength, &ptr1, &ptr2, &len1, &len2);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetSampleData() - FMOD::Sound::lock failed: ") << sError << newl;

        K2_DELETE_ARRAY(sReadData);
        return nullptr;
    }

    MemManager.Copy(sReadData, ptr1, len1);

    if (len2 > 0)
        MemManager.Copy(&sReadData[len1], ptr2, len2);

    result = pSample->GetSampleData()->unlock(ptr1, ptr2, len1, len2);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetSampleData() - FMOD::Sound::unlock failed: ") << sError << newl;

        K2_DELETE_ARRAY(sReadData);
        return nullptr;
    }

    return sReadData;
}

/*====================
  CSoundManager::GetSampleData

  Gets raw data from a sample, putting it in a buffer.
  ====================*/
bool    CSoundManager::GetSampleData(CSample *pSample, byte *pTarget, uint uLength, uint uOffset)
{
    tstring sError;
    FMOD_RESULT result;
    unsigned int len1, len2;
    void *ptr1, *ptr2;

#if TKTK // This seems removed as of 2023
#ifdef linux
    FMOD_OUTPUTTYPE output;
    int iRate, iChannels;
    uint uOriginalLength;
    if (pSample == m_pRecordTarget && (m_pFMODSystem->getOutput(&output),output == FMOD_OUTPUTTYPE_OSS))
    {
        // OSS shares input and output settings so we need to use them and then do some resampling and stuff later...
        uOriginalLength = uLength;
        m_pFMODSystem->getSoftwareFormat(&iRate, nullptr, &iChannels, nullptr, nullptr, nullptr);
        uOffset = static_cast<uint>(static_cast<unsigned long long>(uOffset) * iRate * iChannels / VOICE_SAMPLE_RATE);
        uLength = static_cast<uint>(static_cast<unsigned long long>(uLength) * iRate * iChannels / VOICE_SAMPLE_RATE);
    }
#endif
#endif
    
    result = pSample->GetSampleData()->lock(uOffset, uLength, &ptr1, &ptr2, &len1, &len2);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetSampleData() - FMOD::Sound::lock failed: ") << sError << newl;
        return false;
    }

#if TKTK // This seems removed as of 2023
#ifdef linux
    if (pSample == m_pRecordTarget && output == FMOD_OUTPUTTYPE_OSS)
    {
        // extract left channel & downsample
        spx_uint32_t in_len1(len1 / (iChannels * SOUND_SAMPLE_RATE / 8)), out_len1(in_len1 * iRate / VOICE_SAMPLE_RATE);
        speex_resampler_process_int(g_VoiceResampler, 0, reinterpret_cast<spx_int16_t*>(ptr1), &in_len1, reinterpret_cast<spx_int16_t*>(pTarget), &out_len1);
        
        if (len2 > 0)
        {
            spx_uint32_t in_len2(len2 / (iChannels * SOUND_SAMPLE_RATE / 8)), out_len2(in_len2 * iRate / VOICE_SAMPLE_RATE);
            speex_resampler_process_int(g_VoiceResampler, 0, reinterpret_cast<spx_int16_t*>(ptr2), &in_len2, reinterpret_cast<spx_int16_t*>(&pTarget[out_len1 * SOUND_SAMPLE_RATE / 8]), &out_len2);
        }
    }
    else
#endif
#endif
    {
        MemManager.Copy(pTarget, ptr1, len1);

        if (len2 > 0)
            MemManager.Copy(&pTarget[len1], ptr2, len2);
    }

    result = pSample->GetSampleData()->unlock(ptr1, ptr2, len1, len2);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::GetSampleData() - FMOD::Sound::unlock failed: ") << sError << newl;
        return false;
    }

    return true;
}


/*====================
  CSoundManager::ResetSampleAtPos

  Resets the specified portion of the sample to no sound.
  ====================*/
uint    CSoundManager::ResetSampleAtPos(CSample *pSample, uint uPos, uint uLength)
{
    byte *pSampleReset;
    uint uBytesWritten;

    pSampleReset = K2_NEW_ARRAY(ctx_Sound, byte, uLength);

    MemManager.Set(pSampleReset, 0, sizeof(byte) * uLength);
    uBytesWritten = K2SoundManager.ModifySampleAtPos(pSample, uPos, uLength, pSampleReset);

    K2_DELETE_ARRAY(pSampleReset);

    return uBytesWritten;
}

/*====================
  CSoundManager::ModifySampleAtPos

  Return amount of data written to sample.
  ====================*/
uint    CSoundManager::ModifySampleAtPos(CSample *pSample, uint uPos, uint uLength, byte *pData)
{
    if (pData == nullptr || uLength == uint(-1))
    {
        Console.Err << _T("CSoundManager::ModifySampleAtPos() - Invalid sound data passed to function") << newl;
        return 0;
    }

    FMOD::Sound *pSound(pSample->GetSampleData());
    if (pSound == nullptr)
    {
        Console.Err << _T("CSoundManager::ModifySampleAtPos() - nullptr sound data passed to function") << newl;
        return 0;
    }

    unsigned int len1, len2;
    void *ptr1, *ptr2;
    FMOD_RESULT result(pSound->lock(uPos, uLength, &ptr1, &ptr2, &len1, &len2));
    if (result != FMOD_OK)
    {
        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::ModifySampleAtPos() - FMOD::Sound::lock failed: ") << sError << newl;
        return 0;
    }

    MemManager.Copy(ptr1, pData, len1);

    if (len2 != 0)
        MemManager.Copy(ptr2, &pData[len1], len2);

    result = pSound->unlock(ptr1, ptr2, len1, len2);

    if (result != FMOD_OK)
    {
        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::ModifySampleAtPos() - FMOD::Sound::unlock failed: ") << sError << newl;
        return 0;
    }

    return len1 + len2;
}


/*====================
  CSoundManager::CreateSound
  ====================*/
FMOD::Sound*    CSoundManager::CreateSound(int iFrequency, int iNumChannels, int iNumBits, uint uiBufferSize, int iSoundFlags)
{
    if (!m_bInitialized)
    {
        Console.Warn << _T("CSoundManager::CreateSample() - CSoundManager has not been initialized") << newl;
        return nullptr;
    }

    FMOD_CREATESOUNDEXINFO  exInfo;
    MemManager.Set(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exInfo.defaultfrequency = iFrequency;
    exInfo.numchannels = iNumChannels;

    switch (iNumBits)
    {
        case 8:
            exInfo.format = FMOD_SOUND_FORMAT_PCM8;
            break;
        case 16:
            exInfo.format = FMOD_SOUND_FORMAT_PCM16;
            break;
        case 24:
            exInfo.format = FMOD_SOUND_FORMAT_PCM24;
            break;
        case 32:
            exInfo.format = FMOD_SOUND_FORMAT_PCM32;
            break;
        default:
            Console.Err << _T("CSoundManager::CreateSample() - WARNING: Invalid number of bits specified (") << XtoA(iNumBits) << _T("), defaulting to 16") << newl;
            exInfo.format = FMOD_SOUND_FORMAT_PCM16;
            iNumBits = 16;
            break;
    }

    exInfo.length = uiBufferSize;

    FMOD::Sound *pNewSound;

    pNewSound = nullptr;

    uint uiFlags(FMOD_OPENUSER | FMOD_LOWMEM);
    if (iSoundFlags & SND_2D)
        uiFlags |= FMOD_2D;
    else
        uiFlags |= FMOD_3D;

    FMOD_RESULT result(m_pFMODSystem->createSound(nullptr, uiFlags, &exInfo, &pNewSound));
    if (result != FMOD_OK)
    {
        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::CreateSample() - FMOD::System::createSound failed: ") << sError << newl;
        return nullptr;
    }

    return pNewSound;
}

/*====================
  CSoundManager::CreateExtendedSound
  ====================*/
FMOD::Sound*    CSoundManager::CreateExtendedSound(FMOD_CREATESOUNDEXINFO &exInfo, int iSoundFlags)
{
    if (!m_bInitialized)
    {
        Console.Warn << _T("CSoundManager::CreateSample() - CSoundManager has not been initialized") << newl;
        return nullptr;
    }

    FMOD::Sound *pNewSound;

    pNewSound = nullptr;

    uint uiFlags(FMOD_OPENUSER | FMOD_LOWMEM);
    if (iSoundFlags & SND_2D)
        uiFlags |= FMOD_2D;
    else
        uiFlags |= FMOD_3D;

    FMOD_RESULT result(m_pFMODSystem->createSound(nullptr, uiFlags, &exInfo, &pNewSound));
    if (result != FMOD_OK)
    {
        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::CreateSample() - FMOD::System::createSound failed: ") << sError << newl;
        return nullptr;
    }

    return pNewSound;
}

/*====================
  CSoundManager::LoadSample
  ====================*/
FMOD::Sound*    CSoundManager::LoadSample(const tstring &sInPath, int iSoundFlags)
{
    tstring sPath(sInPath);

    if (!m_bInitialized)
    {
        Console.Warn << _T("CSoundManager::LoadSample() - CSoundManager has not been initialized") << newl;
        return nullptr;
    }

    uint uiFlags(FMOD_LOWMEM);
    if (iSoundFlags & SND_2D)
        uiFlags |= FMOD_2D;
    else
        uiFlags |= FMOD_3D;
    
    if (iSoundFlags & SND_COMPRESSED)
        uiFlags |= FMOD_CREATECOMPRESSEDSAMPLE;
    else
        uiFlags |= FMOD_CREATESAMPLE;

#if 0
    if (sound_useCompressedSamples)
        uiFlags |= FMOD_CREATECOMPRESSEDSAMPLE;
#endif

#if 0
    if (iSoundFlags & SND_STREAM)
        return (FMOD::Sound*)(-1);
#endif

    FMOD::Sound *pNewSound(nullptr);

    FMOD_RESULT result;

    if (iSoundFlags & SND_STREAM)
    {
        tstring sFinalPath(FileManager.FindFilePath(sPath, FILE_READ | FILE_BINARY));
        if (sFinalPath.empty())
        {
            for (auto&& sCompressedFormat : TokenizeString(sound_compressedFormat, _T(',')))
            {
                if (CompareNoCase(Filename_GetExtension(sPath), sCompressedFormat) != 0)
                {
                    // Try compressed if the current file didn't exist
                    sPath = Filename_StripExtension(sPath) + _T(".") + sCompressedFormat;
                    sFinalPath = FileManager.FindFilePath(sPath, FILE_READ | FILE_BINARY);
                    if (!sFinalPath.empty())
                        break;
                }
            }
        }

        if (sFinalPath.empty())
            result = FMOD_ERR_FILE_NOTFOUND;
        else
        {
            if (sFinalPath.find(_T('<')) == tstring::npos)
                sFinalPath += _T("<");

            // Don't seek to the end of the file when it's first opened (because that would result in
            // decompressing the entire file)
            uiFlags |= FMOD_IGNORETAGS;

            result = m_pFMODSystem->createStream(sFinalPath.c_str(), uiFlags, nullptr, &pNewSound);
        }
    }
    else
    {
        result = m_pFMODSystem->createSound(sPath.c_str(), uiFlags, nullptr, &pNewSound);
    }

    if (result != FMOD_OK)
    {
        if (~iSoundFlags & SND_QUIETFAIL)
        {
            tstring sError;
            StrToTString(sError, FMOD_ErrorString(result));
            Console.Err << _T("CSoundManager::LoadSample() - FMOD::System::createSound failed: ") << sError << newl;
        }
        return nullptr;
    }
    
    FMOD_TAG Tag;
    if (pNewSound->getTag("XK2L", 0, &Tag) == FMOD_OK)
    {
        uint *pLoopPoints;
        pLoopPoints = static_cast<uint*>(Tag.data);
        pNewSound->setLoopPoints(pLoopPoints[0], FMOD_TIMEUNIT_PCM, pLoopPoints[1], FMOD_TIMEUNIT_PCM);
    }

    if (sound_mixrate < 44100 && sound_downsample && !(iSoundFlags & SND_COMPRESSED))
    {
        FMOD_SOUND_TYPE type;
        FMOD_SOUND_FORMAT format;
        int iChannels, iBits;
        unsigned int length, len1, len2;
        float fRate;
        void *ptr1, *ptr2;
        SpeexResamplerState* pResampler(nullptr);
        FMOD::Sound *pDownsampledSound(nullptr);

        pNewSound->getFormat(&type, &format, &iChannels, &iBits);
        pNewSound->getDefaults(&fRate, nullptr);
        pNewSound->getLength(&length, FMOD_TIMEUNIT_PCMBYTES);
        pNewSound->lock(0, length, &ptr1, &ptr2, &len1, &len2);

        if (iChannels == 2)
            pResampler = m_pResamplerStereo;
        else if (iChannels == 1)
            pResampler = m_pResamplerMono;
        
        speex_resampler_set_rate(pResampler, static_cast<unsigned int>(fRate), sound_mixrate);
        speex_resampler_skip_zeros(pResampler);
        speex_resampler_reset_mem(pResampler);

        if (format == FMOD_SOUND_FORMAT_PCM16)
        {
            uint uiSamples(length / (iChannels * iBits / 8));
            uint uiNewSamples(static_cast<int>(ceil(uiSamples * (sound_mixrate / fRate))));
            ushort *pData = K2_NEW_ARRAY(ctx_Sound, ushort, uiNewSamples*iChannels);
            if (speex_resampler_process_interleaved_int(pResampler, reinterpret_cast<spx_int16_t*>(ptr1), &uiSamples, reinterpret_cast<spx_int16_t*>(pData), &uiNewSamples) == RESAMPLER_ERR_SUCCESS)
            {
                FMOD_CREATESOUNDEXINFO exinfo;
                MemManager.Set(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
                exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
                exinfo.length = uiNewSamples * iChannels * iBits / 8;
                exinfo.numchannels = iChannels;
                exinfo.defaultfrequency = sound_mixrate;
                exinfo.format = FMOD_SOUND_FORMAT_PCM16;
                
#if BYTE_ORDER == BIG_ENDIAN
                // have to pass the raw sound to fmod in little endian format
                for (uint ui(0); ui < uiNewSamples * iChannels; ++ui)
                    pData[ui] = LittleShort(pData[ui]);
#endif

                if (m_pFMODSystem->createSound(reinterpret_cast<char*>(pData), uiFlags | FMOD_OPENMEMORY | FMOD_OPENRAW, &exinfo, &pDownsampledSound) != FMOD_OK)
                {
                    Console.Warn << _T("CSoundManager::LoadSample() - Unable to create downsampled sample.") << newl;
                }
            }

            K2_DELETE_ARRAY(pData);
        }
        else if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
        {
            uint uiSamples(length / (iChannels * iBits / 8));
            uint uiNewSamples(static_cast<int>(ceil(uiSamples * (sound_mixrate / fRate))));
            float *pData = K2_NEW_ARRAY(ctx_Sound, float, uiNewSamples*iChannels);
            if (speex_resampler_process_interleaved_float(pResampler, reinterpret_cast<float*>(ptr1), &uiSamples, pData, &uiNewSamples) == RESAMPLER_ERR_SUCCESS)
            {
                FMOD_CREATESOUNDEXINFO exinfo;
                MemManager.Set(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
                exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
                exinfo.length = uiNewSamples * iChannels * iBits / 8;
                exinfo.numchannels = iChannels;
                exinfo.defaultfrequency = sound_mixrate;
                exinfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
                
#if BYTE_ORDER == BIG_ENDIAN
                // have to pass the raw sound to fmod in little endian format
                for (uint ui(0); ui < uiNewSamples * iChannels; ++ui)
                    pData[ui] = LittleFloat(pData[ui]);
#endif

                if (m_pFMODSystem->createSound(reinterpret_cast<char*>(pData), uiFlags | FMOD_OPENMEMORY | FMOD_OPENRAW, &exinfo, &pDownsampledSound) != FMOD_OK)
                {
                    Console.Warn << _T("CSoundManager::LoadSample() - Unable to create downsampled sample.") << newl;
                }
            }

            K2_DELETE_ARRAY(pData);
        }
        else
            Console.Warn << _T("CSoundManager::LoadSample() - Downsampling unsupported.") << newl;

        pNewSound->unlock(ptr1, ptr2, len1, len2);
        if (pDownsampledSound)
        {
            pNewSound->release();
            pNewSound = pDownsampledSound;
        }
    }

    return pNewSound;
}


/*====================
  CSoundManager::ResetWorldGeometry

  Create a new geometry object to add the world polygons into
  ====================*/
void    CSoundManager::ResetWorldGeometry(int iMaxPolys, int iMaxVertices, float fMaxWorldSize)
{
    if (!m_bInitialized)
        return;

    if (m_pWorldGeometry)
        m_pWorldGeometry->release();

    m_pFMODSystem->setGeometrySettings(fMaxWorldSize);
    m_pFMODSystem->createGeometry(iMaxPolys, iMaxVertices, &m_pWorldGeometry);
    FMOD_VECTOR zero = { 0.0, 0.0, 0.0 };
    m_pWorldGeometry->setPosition(&zero);
    m_pWorldGeometry->setActive(true);
}


/*====================
  CSoundManager::AddWorldGeometry

  Inserts a triangel of the world's geometry into FMOD's geometry processing engine
  ====================*/
void    CSoundManager::AddWorldGeometry(CVec3f &a, CVec3f &b, CVec3f &c)
{
    if (!m_bInitialized)
        return;

    if (!m_pWorldGeometry)
        return;

    FMOD_VECTOR vertices[3] = { { a.x, a.y, a.z },
                                { b.x, b.y, b.z },
                                { c.x, c.y, c.z } };

    m_pWorldGeometry->addPolygon(1.0, 1.0, false, 3, vertices, nullptr);
}


/*====================
  CSoundManager::SetListenerPosition

  Set the listener position
  ====================*/
void    CSoundManager::SetListenerPosition(const CVec3f &v3Pos, const CVec3f &v3Velocity, const CVec3f &v3Forward, const CVec3f &v3Up, bool bWarp)
{
    if (!m_bInitialized)
        return;

    /*if (!v3Pos.IsValid())
    {
        Console.Warn << _T("CSoundManager::SetListenerPosition() - Invalid position") << newl;
        //return;
    }
    if (!v3Forward.IsValid())
    {
        Console.Warn << _T("CSoundManager::SetListenerPosition() - Invalid forward vector") << newl;
        //return;
    }
    if (!v3Up.IsValid())
    {
        Console.Warn << _T("CSoundManager::SetListenerPosition() - Invalid up vector") << newl;
        //return;
    }*/
    CVec3f v3Pos2 = v3Pos + m_v3Jitter;
    m_pFMODSystem->set3DListenerAttributes(0,
        FMOD_VECTOR_CAST(&v3Pos2), nullptr/*FMOD_VECTOR_CAST(&v3Velocity)*/, // Velocity set to nullptr to disable doppler
        FMOD_VECTOR_CAST(&v3Forward), FMOD_VECTOR_CAST(&v3Up));

    m_v3Center = v3Pos;

#if 0
    m_Listener.v3Pos = v3Pos;
    m_Listener.v3Velocity = v3Velocity;
#endif
}


/*====================
  CSoundManager::StopHandle

  Stops the given sound handle
  ====================*/
void    CSoundManager::StopHandle(SoundHandle hHandle)
{
    if (!m_bInitialized)
        return;

    PROFILE("CSoundManager::StopHandle");

    map<SoundHandle, FMOD::Channel *>::iterator itFind(m_mapActiveSounds.find(hHandle));

    if (itFind == m_mapActiveSounds.end())
        return;

    FMOD::Channel* pChannel = itFind->second;

    for (int i(0); i < NUM_ASSIGNED_CHANNELS; ++i)
    {
        if (m_ahSoundHandle[i] == hHandle)
            m_ahSoundHandle[i] = INVALID_INDEX;
    }

    map<SoundHandle, CSoundFade*>::iterator itFind2(m_mapFadeOutSounds.find(hHandle));
    if (itFind2 == m_mapFadeOutSounds.end())
    {
        if (m_mapFadingOutSounds.find(hHandle) == m_mapFadingOutSounds.end())
        {
            if (m_mapActiveSounds.contains(hHandle))
            {
                pChannel->stop();
            }
        }
    }
    else
    {
        m_mapFadingOutSounds[hHandle] = itFind2->second;
        m_mapFadingOutSounds[hHandle]->Start(pChannel, Host.GetTime(), false);
        m_mapFadeOutSounds.erase(itFind2);

        map<SoundHandle, CSoundFade*>::iterator itFind3(m_mapFadingInSounds.find(hHandle));
        if (itFind3 != m_mapFadingInSounds.end())
        {
            K2_DELETE(itFind3->second);
            STL_ERASE(m_mapFadingInSounds, itFind3);
        }
    }
}


/*====================
  CSoundManager::StopChannel
  ====================*/
void    CSoundManager::StopChannel(int iChannel)
{
    if (iChannel >= NUM_ASSIGNED_CHANNELS || iChannel < 0)
        return;

    StopHandle(m_ahSoundHandle[iChannel]);

    m_ahSoundHandle[iChannel] = INVALID_INDEX;
}


/*====================
  CSoundManager::UpdateHandle
  ====================*/
bool    CSoundManager::UpdateHandle(SoundHandle hHandle, const CVec3f &v3Pos, const CVec3f &v3Vel, bool b3D)
{
    if (!m_bInitialized)
        return false;

    map<SoundHandle, FMOD::Channel *>::iterator itFind(m_mapActiveSounds.find(hHandle));

    if (itFind == m_mapActiveSounds.end())
        return false;
    
    if (b3D) // Velocity set to nullptr to disable doppler
    {
        /*if (!v3Pos.IsValid())
            Console.Warn << _T("CSoundManager::UpdateHandle() - Invalid position") << newl;
        //else*/
            itFind->second->set3DAttributes(FMOD_VECTOR_CAST(&v3Pos), nullptr/*FMOD_VECTOR_CAST(&v3Vel)*/);
    }

    return true;
}


/*====================
  CSoundManager::IsHandleActive
  ====================*/
bool    CSoundManager::IsHandleActive(SoundHandle hHandle)
{
    if (!m_bInitialized)
        return false;

    return m_mapActiveSounds.find(hHandle) != m_mapActiveSounds.end();
}


/*====================
  CSoundManager::GetChannelPosition
  ====================*/
uint    CSoundManager::GetChannelPosition(SoundHandle hHandle)
{
    try
    {
        if (!m_bInitialized)
            EX_ERROR(_T("Sound manager not initialized"));

        map<SoundHandle, FMOD::Channel*>::iterator itFind(m_mapActiveSounds.find(hHandle));
        if (itFind == m_mapActiveSounds.end())
            EX_ERROR(_T("Sound handle not found"));

        uint uPos;
        FMOD_RESULT result(itFind->second->getPosition(&uPos, FMOD_TIMEUNIT_PCMBYTES));
        if (result != FMOD_OK)
            EX_ERROR(_TS("FMOD::Channel::getPosition failed: ") + StringToTString(FMOD_ErrorString(result)));

        return uPos;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CSoundManager::GetChannelPosition() - "), NO_THROW);
        return -1;
    }
}


/*====================
  CSoundManager::FreeSample
  ====================*/
void    CSoundManager::FreeSample(FMOD::Sound* pSample)
{
    if (!m_bInitialized)
        return;

    if (pSample != nullptr)
        pSample->release();
}


/*====================
  CSoundManager::ReleaseSoundNextTick
  ====================*/
void     CSoundManager::ReleaseSoundNextTick(FMOD::Sound* pSample)
{
    if (pSample != nullptr)
    {
        m_setReleaseSounds.insert(pSample);
    }
}


/*====================
  CSoundManager::SetPlayPosition
  ====================*/
void    CSoundManager::SetPlayPosition(SoundHandle hHandle, uint uiPos)
{
    if (!m_bInitialized)
        return;

    tstring sError;
    FMOD_RESULT result;

    map<SoundHandle, FMOD::Channel *>::iterator itFind(m_mapActiveSounds.find(hHandle));

    if (itFind == m_mapActiveSounds.end())
        return;

    result = itFind->second->setPosition(uiPos, FMOD_TIMEUNIT_PCMBYTES);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::SetChannelPosition() - FMOD::Channel::setPosition failed: ") << sError << newl;
    }
}


/*====================
  CSoundManager::SetVolume
  ====================*/
void    CSoundManager::SetVolume(SoundHandle hHandle, float fVolume)
{
    if (!m_bInitialized)
        return;

    tstring sError;
    FMOD_RESULT result;

    map<SoundHandle, FMOD::Channel *>::iterator itFind(m_mapActiveSounds.find(hHandle));

    if (itFind == m_mapActiveSounds.end())
        return;

    result = itFind->second->setVolume(fVolume);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::SetVolume() - FMOD::Channel::setVolume failed: ") << sError << newl;
    }
}


/*====================
  CSoundManager::SetMute
  ====================*/
void    CSoundManager::SetMute(SoundHandle hHandle, bool bMute)
{
    if (!m_bInitialized)
        return;

    tstring sError;
    FMOD_RESULT result;

    map<SoundHandle, FMOD::Channel *>::iterator itFind(m_mapActiveSounds.find(hHandle));

    if (itFind == m_mapActiveSounds.end())
        return;

    result = itFind->second->setMute(bMute);

    if (result != FMOD_OK)
    {
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::SetMute() - FMOD::Channel::setMute failed: ") << sError << newl;
    }
}


/*====================
  CSoundManager::PlaySound

  a handle to the sound is returned.  in most cases this is not needed, but may be occasionally useful
  if tracking of a specific sound effect is needed
  ====================*/
SoundHandle CSoundManager::PlaySound
(
    FMOD::ChannelGroup *pGroup,
    CSample *pSample,
    const CVec3f *pv3Pos,
    const CVec3f *pv3Vel,
    float fVolume,
    float fFalloff,
    int iChannel,
    int iPriority,
    int iFlags,
    int iFadeIn,
    int iFadeOutStartTime,
    int iFadeOut,
    int iSpeedUpTime,
    float fSpeed1,
    float fSpeed2,
    int iSlowDownTime,
    float fSpreadAngle,
    float fFalloffEnd,
    float fDampen
)
{
    if (!m_bInitialized || pSample == nullptr)
        return INVALID_INDEX;

    PROFILE("CSoundManager::PlaySound");

    if (iChannel != CHANNEL_AUTO && iChannel <= NUM_ASSIGNED_CHANNELS)
    {
        if (m_ahSoundHandle[iChannel] != INVALID_INDEX && IsHandleActive(m_ahSoundHandle[iChannel]))
        {
            StopHandle(m_ahSoundHandle[iChannel]);
            m_ahSoundHandle[iChannel] = INVALID_INDEX;
        }
    }

    bool bStream((pSample->GetSoundFlags() & SND_STREAM) == SND_STREAM);

#if 0
    if (!sound_noCull && pv3Pos)
    {
        // Cull out the sound if it's out of range
        float fDistSq = DistanceSq(*pv3Pos, m_Listener.v3Pos);
        if (fDistSq > fFalloff * fFalloff)
            return INVALID_INDEX;
    }
#endif

    FMOD::Channel *pChannel;
    FMOD_RESULT result;
    if (bStream)
    {
        PROFILE("Stream playSound");

        FMOD::Sound *pSound;
        result = m_pFMODSystem->createStream(TStringToString(pSample->GetPath()).c_str(), FMOD_LOWMEM, nullptr, &pSound);
        if (result != FMOD_OK)
            return INVALID_INDEX;
        result = m_pFMODSystem->playSound(pSound, nullptr, true, &pChannel);
    }
    else
    {
        PROFILE("Normal playSound");

        FMOD::Sound *pSampleData(pSample->GetSampleData());

        if (!pSampleData)
            return INVALID_INDEX;

        result = m_pFMODSystem->playSound(pSampleData, nullptr, true, &pChannel);
    }
    if (result != FMOD_OK)
    {
        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::PlaySound() - FMOD::System::playSound failed: ") << sError << newl;
        return INVALID_INDEX;
    }

    pChannel->setPriority(iPriority);

    // if we're a 3D sound and no falloff mode is specified, then default to linear falloff.
    if (!(iFlags & SND_2D) && !(iFlags & SND_LINEARFALLOFF) && !(iFlags & SND_SQUAREDFALLOFF))
        iFlags |= SND_LINEARFALLOFF;

    FMOD_MODE mode((iFlags & SND_2D) ? FMOD_2D : FMOD_3D);

    if (iFlags & SND_LINEARFALLOFF)
        mode |= FMOD_3D_LINEARROLLOFF;
    else if (iFlags & SND_SQUAREDFALLOFF)
        mode |= FMOD_3D_LINEARROLLOFF;
    else if (!(iFlags & SND_2D) && sound_customRolloff)
        mode |= FMOD_3D_CUSTOMROLLOFF;
    else
    {
        // if we're a 3D sound with no falloff mode, complain.
        if (!(iFlags & SND_2D))
        {
            Console.Warn << "Invalid falloff mode for sound!" << newl;
            assert(false);
        }
    }

    if (iFlags & SND_LOOP)
        mode |= FMOD_LOOP_NORMAL;
    else
        mode |= FMOD_LOOP_OFF;
    
    pChannel->setMode(mode);
    
    if (bStream)
        pChannel->setCallback(Sound_CallbackStopActiveStream);
    else
        pChannel->setCallback(Sound_CallbackStopActiveSound);

    if (pv3Pos)
    {   // Velocity set to nullptr to disable doppler
        /*if (!pv3Pos->IsValid())
            Console.Warn << _T("CSoundManager::PlaySound() - Invalid position") << newl;
        //else*/
            pChannel->set3DAttributes(FMOD_VECTOR_CAST(pv3Pos), nullptr/*FMOD_VECTOR_CAST(pv3Vel)*/);
    }

    result = pChannel->setChannelGroup(pGroup);

    SoundHandle hHandle(m_uiHandleCounter);
    ++m_uiHandleCounter;

    m_mapActiveSounds[hHandle] = pChannel;

    if (*pv3Pos)
    {
        floid u;

        float fDistanceSq(DistanceSq(pv3Pos->xy(), GetCenter().xy()));

        float fRolloff;
        if (fDistanceSq > SQR(fFalloffEnd))
            fRolloff = 0.0f;
        else if (fDistanceSq < SQR(fFalloff))
            fRolloff = 1.0f;
        else
            fRolloff = 1.0f - ((sqrt(fDistanceSq) - fFalloff) / (fFalloffEnd - fFalloff));

        u.f = fRolloff;
        pChannel->setUserData(u.v);
    }

    // fading in/out stuff
    if (iFlags & SND_LOOP)
    {
        float fFrequency;
        pChannel->getFrequency(&fFrequency);

        if (iSlowDownTime > 0 && iFadeOut > 0)
            m_mapFadeOutSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolumeSpeed)(iFadeOut, 0, iSlowDownTime, fFrequency * fSpeed1);
        else if (iSlowDownTime > 0 && iFadeOut <= 0)
            m_mapFadeOutSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeSpeed)(iSlowDownTime, fFrequency * fSpeed1);
        else if (iFadeOut > 0)
            m_mapFadeOutSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolume)(iFadeOut, 0);

        if (iSpeedUpTime > 0 && iFadeIn > 0)
        {
            pChannel->setVolume(0.0f);
            pChannel->setFrequency(fFrequency * fSpeed1);
            m_mapFadingInSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolumeSpeed)(iFadeIn, fVolume, iSpeedUpTime, fFrequency * fSpeed2);
            m_mapFadingInSounds[hHandle]->Start(pChannel, Host.GetTime(), true);
        }
        else if (iSpeedUpTime > 0 && iFadeIn <= 0)
        {
            pChannel->setVolume(fVolume);
            pChannel->setFrequency(fFrequency * fSpeed1);
            m_mapFadingInSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeSpeed)(iSpeedUpTime, fFrequency * fSpeed2);
            m_mapFadingInSounds[hHandle]->Start(pChannel, Host.GetTime(), true);
        }
        else if (iFadeIn > 0)
        {
            pChannel->setVolume(0.0f);
            m_mapFadingInSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolume)(iFadeIn, fVolume);
            m_mapFadingInSounds[hHandle]->Start(pChannel, Host.GetTime(), true);
        }
        else
        {
            pChannel->setVolume(fVolume);
        }

        if (iSpeedUpTime == 0 && iSlowDownTime >= 0 && fSpeed2 != 1.0f)
        {
            pChannel->setFrequency(fFrequency * fSpeed2);
        }
    }
    else
    {
        int iRealFadeOutStartTime(iFadeIn < iFadeOutStartTime ? iFadeIn : iFadeOutStartTime);
        if (iFadeOut > 0)
        {
            m_mapFadingOutSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolume)(iFadeOut, 0);
            m_mapFadingOutSounds[hHandle]->Start(pChannel, Host.GetTime() + iRealFadeOutStartTime, false);
        }

        if (iFadeIn > 0)
        {
            pChannel->setVolume(0.0f);
            m_mapFadingInSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundFadeVolume)(iFadeIn, fVolume);
            m_mapFadingInSounds[hHandle]->Start(pChannel, Host.GetTime(), true);
        }
        else
        {
            pChannel->setVolume(fVolume);
        }
    }

    if (iFlags & (SND_LINEARFALLOFF | SND_SQUAREDFALLOFF))
    {
        pChannel->set3DMinMaxDistance(fFalloff + m_fFalloffBias, fFalloffEnd + m_fFalloffBias);
    }
    else
    {
        if (fFalloff < 0)
            fFalloff = sound_defaultFalloff;
        
        if (!sound_customRolloff)
        {
            pChannel->set3DMinMaxDistance(fFalloff, sound_defaultMaxFalloff);
        }
        else
        {
            int iFalloff(fFalloff * sound_customRolloffFalloffMult);
            map<int, FMOD_VECTOR *>::iterator itFind(m_mapRolloffCurves.find(iFalloff));

            if (itFind == m_mapRolloffCurves.end())
            {
                // PRETTY!!!!!! -JM

                // custom rolloff curve that Arney's wanted - fmod's log falloff shifted and with a different scaling factor
                // 1.0 |********* y = 1, y <= d0
                //     |         *
                //     |          *         d1     1
                //     |           *    y = -- * ------ + y0
                //     |             *      z    x - x0
                //     |               *
                //     |                 *
                //     |                    *
                //     |                       *
                //     |                           *
                //     |                                *  y = 0, y >= d1
                // 0.0-|--------------------------------------*************
                //    0.0       d0 = a * d1                   d1
                // z is a scaling factor that determines how fast the curve falls off
                // x0 = x01 * d1
                // x01 = 0.5 * (1 + a) - sqrt(0.25 * a**2 - a / z + 0.25 + 1 / z - 0.5 * a)
                // y0 = - 1 / (z * (1 - x01))
                #define NUM_CURVE_POINTS 20 // number of points used to define the curved part
                const float a(0.20f);//a(0.20f);
                const float z(3.0f);
                // recalculate x01/y0 if you change a or z
                const float x01(-0.053197264742180783f);//x01(-0.24283907089541168f);
                const float y0(-0.316496580927726090f);//y0(-0.26820313356674658f);

                m_mapRolloffCurves[iFalloff] = K2_NEW_ARRAY(ctx_Sound, FMOD_VECTOR, NUM_CURVE_POINTS+2);
                
                float fDist(iFalloff * a);
                float fDelta((iFalloff - fDist) / float(NUM_CURVE_POINTS));
                float x0(iFalloff * x01);
                float fNumerator(iFalloff / z);

                m_mapRolloffCurves[iFalloff][0].x = 0.0f;
                m_mapRolloffCurves[iFalloff][0].y = 1.0f;
                m_mapRolloffCurves[iFalloff][0].z = 0.0f;

                m_mapRolloffCurves[iFalloff][1].x = fDist;
                m_mapRolloffCurves[iFalloff][1].y = 1.0f;
                m_mapRolloffCurves[iFalloff][1].z = 0.0f;

                //Console << m_mapRolloffCurves[iFalloff][0].y << " "  << m_mapRolloffCurves[iFalloff][1].y;

                for (int i = 2; i < NUM_CURVE_POINTS + 1; i++)
                {
                    fDist += fDelta;
                    m_mapRolloffCurves[iFalloff][i].x = fDist; // distance
                    m_mapRolloffCurves[iFalloff][i].y = fNumerator / (fDist - x0) + y0; // volume
                    m_mapRolloffCurves[iFalloff][i].z = 0.0f; // unused
                    //Console << " " << m_mapRolloffCurves[iFalloff][i].y;
                }

                m_mapRolloffCurves[iFalloff][NUM_CURVE_POINTS+1].x = float(iFalloff);
                m_mapRolloffCurves[iFalloff][NUM_CURVE_POINTS+1].y = 0.0f;
                m_mapRolloffCurves[iFalloff][NUM_CURVE_POINTS+1].z = 0.0f;
                //Console << " " << m_mapRolloffCurves[iFalloff][NUM_CURVE_POINTS+1].y << newl;
            }
            pChannel->set3DCustomRolloff(m_mapRolloffCurves[iFalloff], NUM_CURVE_POINTS+2);
        }
    }

    if (fSpreadAngle > 0.0f)
        pChannel->set3DSpread(fSpreadAngle);

    if (fDampen < 1.0f)
    {
        m_mapDampenSounds[hHandle] = K2_NEW(ctx_Sound,  CSoundDampen)(fDampen);
    }

    pChannel->setPaused(false);

    if (iChannel != CHANNEL_AUTO && iChannel <= NUM_ASSIGNED_CHANNELS)
    {
        m_ahSoundHandle[iChannel] = hHandle;
    }

    return hHandle;
}

SoundHandle CSoundManager::PlaySound
(
    FMOD::ChannelGroup *pGroup,
    ResHandle hSample,
    const CVec3f *pv3Pos,
    const CVec3f *pv3Vel,
    float fVolume,
    float fFalloff,
    int iChannel,
    int iPriority,
    int iFlags,
    int iFadeIn,
    int iFadeOutStartTime,
    int iFadeOut,
    int iSpeedUpTime,
    float fSpeed1,
    float fSpeed2,
    int iSlowDownTime,
    float fSpreadAngle,
    float fFalloffEnd,
    float fDampen
)
{
    CSample *pSample(g_ResourceManager.GetSample(hSample));
    if (!pSample)
        return INVALID_INDEX;

    return PlaySound(pGroup, pSample, pv3Pos, pv3Vel, fVolume, fFalloff, iChannel, iPriority, iFlags, iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime, fSpreadAngle, fFalloffEnd, fDampen);
}


/*==========================
  CSoundManager::Play2DSFXSound
  ==========================*/
SoundHandle CSoundManager::Play2DSFXSound(ResHandle hSample, float fVolume, int iChannel, int iPriority, bool bLoop, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime)
{
    return PlaySound(m_pSFXChannelGroup, hSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D | (bLoop ? SND_LOOP : 0), iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime, sound_stereoSpread, 0.0f);
}

SoundHandle CSoundManager::Play2DSFXSound(CSample *pSample, float fVolume, int iChannel, int iPriority, bool bLoop, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime)
{
    return PlaySound(m_pSFXChannelGroup, pSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D | (bLoop ? SND_LOOP : 0), iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime, sound_stereoSpread, 0.0f);
}


/*==========================
  CSoundManager::PlaySFXSound
  ==========================*/
SoundHandle CSoundManager::PlaySFXSound(ResHandle hSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume, float fFalloff, int iChannel, int iPriority, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fFalloffEnd)
{
    return PlaySound(m_pSFXChannelGroup, hSample, pv3Pos, pv3Vel, fVolume, fFalloff, iChannel, iPriority, iSoundFlags, iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime, sound_stereoSpread, fFalloffEnd);
}

SoundHandle CSoundManager::PlaySFXSound(CSample *pSample, const CVec3f *pv3Pos, const CVec3f *pv3Vel, float fVolume, float fFalloff, int iChannel, int iPriority, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fFalloffEnd)
{
    return PlaySound(m_pSFXChannelGroup, pSample, pv3Pos, pv3Vel, fVolume, fFalloff, iChannel, iPriority, iSoundFlags, iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime, sound_stereoSpread, fFalloffEnd);
}


/*==========================
  CSoundManager::PlayVoiceSound
  ==========================*/
SoundHandle CSoundManager::PlayVoiceSound(ResHandle hSample, float fVolume, int iChannel, int iPriority)
{
    return PlaySound(m_pVoiceChannelGroup, hSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D | SND_LOOP, 0, 0, 0, 0, 1, 1, 0);
}

SoundHandle CSoundManager::PlayVoiceSound(CSample *pSample, float fVolume, int iChannel, int iPriority)
{
    return PlaySound(m_pVoiceChannelGroup, pSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D | SND_LOOP, 0, 0, 0, 0, 1, 1, 0);
}


/*==========================
  CSoundManager::Play2DSound
  ==========================*/
SoundHandle CSoundManager::Play2DSound(ResHandle hSample, float fVolume, int iChannel, int iPriority, float fDampen)
{
    return PlaySound(m_pInterfaceChannelGroup, hSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D, 0, 0, 0, 0, 1, 1, 0, 0.0f, 0.0f, fDampen);
}

SoundHandle CSoundManager::Play2DSound(CSample *pSample, float fVolume, int iChannel, int iPriority, float fDampen)
{
    return PlaySound(m_pInterfaceChannelGroup, pSample, nullptr, nullptr, fVolume, 0, iChannel, iPriority, SND_2D, 0, 0, 0, 0, 1, 1, 0, 0.0f, 0.0f, fDampen);
}


/*==========================
  CSoundManager::PlayWorldSFXSound
  ==========================*/
SoundHandle CSoundManager::PlayWorldSFXSound(ResHandle hSample, const CVec3f *pv3Pos, float fVolume, float fFalloff, int iChannel, int iPriority, bool bLoop)
{
    // TODO: start falloff is set to 0.0 for now, configurable later.
    float fStartFalloff(0.0f);
    return PlaySound(m_pSFXChannelGroup, hSample, pv3Pos, nullptr, fVolume, fStartFalloff, iChannel, iPriority, bLoop ? SND_LOOP : 0, 0, 0, 0, 0, 1, 1, 0, 180.0f, fFalloff);
}

SoundHandle CSoundManager::PlayWorldSFXSound(CSample *pSample, const CVec3f *pv3Pos, float fVolume, float fFalloff, int iChannel, int iPriority, bool bLoop)
{
    // TODO: start falloff is set to 0.0 for now, configurable later.
    float fStartFalloff(0.0f);
    return PlaySound(m_pSFXChannelGroup, pSample, pv3Pos, nullptr, fVolume, fStartFalloff, iChannel, iPriority, bLoop ? SND_LOOP : 0, 0, 0, 0, 0, 1, 1, 0, 180.0f, fFalloff);
}


/*==========================
  CSoundManager::StartNextMusic
  ==========================*/
void    CSoundManager::StartNextMusic()
{
    PROFILE("CSoundManager::StartNextMusic");

    if (!m_bInitialized)
        return;

    if (sound_musicVolume == 0.0f)
        return;

    // If the current music is still playing / fading out, then abort
    if (m_cCurrentMusic.IsValid())
        return;

    // Sanity check: the system thinks no music is playing, so if the music channel is valid, abort
    assert(m_pMusicChannel == nullptr);
    if (m_pMusicChannel != nullptr)
        return;

    // If the next music track hasn't been specified yet, then abort
    if (!m_cNextMusic.IsValid())
        return;

    // Attempt to start the next track, once
    tstring sFilename(m_cNextMusic.sFilename);
    bool bFadeIn(m_cNextMusic.bFadeIn);
    bool bLoop(m_cNextMusic.bLoop);
    assert(m_cNextMusic.hSample == INVALID_RESOURCE);
    m_cNextMusic = SPlayMusic();

    // Specify sound flags
    int iSoundFlags(SND_2D | SND_COMPRESSED);
    if (bLoop)
        iSoundFlags |= SND_LOOP;
    iSoundFlags |= SND_STREAM;
    
    // Attempt to load the music
    FMOD::Sound* pSampleData(LoadSample(sFilename, iSoundFlags));

    // Attempt to play the music
    FMOD_RESULT result(m_pFMODSystem->playSound(pSampleData, nullptr, true, &m_pMusicChannel));

    // If the music could not be played for some reason, then abort
    if (result != FMOD_OK)
    {
        assert(!m_cCurrentMusic.IsValid());
        m_pMusicChannel = nullptr;
        pSampleData->release();

        tstring sError;
        StrToTString(sError, FMOD_ErrorString(result));
        Console.Err << _T("CSoundManager::StartNextMusic() - FMOD::System::playSound failed: ") << sError << newl;
        return;
    }

    // We've successfully begun playing the music track
    m_cCurrentMusic = SPlayMusic(sFilename, bLoop, bFadeIn);
    m_cCurrentMusic.pSound = pSampleData;
    FMOD_MODE uiMode(FMOD_2D | FMOD_LOOP_OFF);
    if (iSoundFlags & SND_LOOP)
        uiMode |= FMOD_LOOP_NORMAL;
    m_pMusicChannel->setMode(uiMode);
    m_pMusicChannel->setPriority(0);

    // Allocate a sound handle for the music track and activate it
    SoundHandle hHandle(m_uiHandleCounter);
    ++m_uiHandleCounter;
    m_mapActiveSounds[hHandle] = m_pMusicChannel;

    // Fade in the music track, if desired
    if (bFadeIn)
    {
        m_pMusicChannel->setVolume(0.0f);
        m_mapFadingInSounds[hHandle] = K2_NEW(global,  CSoundFadeVolume)(sound_musicFadeTime, 1.0);
        m_mapFadingInSounds[hHandle]->Start(m_pMusicChannel, INVALID_TIME, true);
    }

    // Put the track into the music group
    m_pMusicChannel->setChannelGroup(m_pMusicChannelGroup);

    // When the music track is completely stopped (completely faded out), this callback will be called.
    // We will likely want to unregister the music track when it's stopped, to save on memory
    m_pMusicChannel->setCallback(Sound_CallbackStopActiveMusicSample);

    // Begin playing the music track
    m_pMusicChannel->setPaused(false);
}


/*==========================
  CSoundManager::PlayMusic

  No stream
  ==========================*/
void    CSoundManager::PlayMusic(const tstring &sFilename, bool bLoop, bool bCrossFade, bool bFromPlaylist)
{
    PROFILE("CSoundManager::PlayMusic");

    if (!m_bInitialized)
        return;

    if (sound_musicVolume == 0.0f)
        return;

    // If no music was playing, then don't fade in the music
    bool bFadeIn(bCrossFade);
    if (m_pMusicChannel == nullptr)
        bFadeIn = false;

    // Indicate that a new track should start after the current track is stopped
    m_cNextMusic = SPlayMusic(sFilename, bLoop, bFadeIn);

    // Stop current music.  If the new track wasn't initiated by the playlist, then cancel the playlist
    StopMusic(bCrossFade, !bFromPlaylist);
}


/*==========================
  CSoundManager::PlayPlaylist
  ==========================*/
void    CSoundManager::PlayPlaylist(const tsvector &vPlaylist, bool bShuffle)
{
    if (vPlaylist.empty())
        return;

    m_vMusicPlaylist = vPlaylist;
    if (bShuffle)
        K2::random_shuffle(m_vMusicPlaylist.begin(), m_vMusicPlaylist.end());

    m_bPlaylistActive = true;
    m_iNextPlaylistIndex = 1 % int(vPlaylist.size());
    PlayMusic(m_vMusicPlaylist[0], false, true, true);
}


/*====================
  CSoundManager::StopMusic
  ====================*/
void    CSoundManager::StopMusic(bool bFadeOut, bool bClearPlaylist, uint uiDelayFrames)
{
    // Delay the StopMusic call if necessary.
    m_bStopMusic_FadeOut = bFadeOut;
    m_bStopMusic_ClearPlaylist = bClearPlaylist;
    m_uiStopMusicDelayFrames = uiDelayFrames;
    if (uiDelayFrames > 0)
        return;

    if (m_pMusicChannel != nullptr)
    {
        // get the previously assigned handle
        SoundHandle hHandle(INVALID_INDEX);
        for (map<SoundHandle, FMOD::Channel *>::iterator it(m_mapActiveSounds.begin()); it != m_mapActiveSounds.end(); ++it)
        {
            if (it->second == m_pMusicChannel)
            {
                hHandle = it->first;
                break;
            }
        }
        
        if (hHandle != INVALID_INDEX)
        {
            // Fade out the music, unless it's already fading out
            if (m_mapFadingOutSounds.find(hHandle) == m_mapFadingOutSounds.end())
            {
                // fade out music
                uint uiFadeOutTime(1);
                if (bFadeOut)
                    uiFadeOutTime = sound_musicFadeTime;
                m_mapFadingOutSounds[hHandle] = K2_NEW(global,  CSoundFadeVolume)(uiFadeOutTime, 0.0);
                m_mapFadingOutSounds[hHandle]->Start(m_pMusicChannel, INVALID_TIME, false); 
            }
        }
    }

    if (bClearPlaylist)
        ClearPlaylist();
}



/*====================
  CSoundManager::ClearPlaylist
  ====================*/
void    CSoundManager::ClearPlaylist()
{
    m_bPlaylistActive = false;
    m_vMusicPlaylist.clear();
}


/*====================
  CSoundManager::ReleasePendingSounds
  ====================*/
void    CSoundManager::ReleasePendingSounds()
{
    while (!m_setReleaseSounds.empty())
    {
        FMOD::Sound* pSound(*m_setReleaseSounds.begin());
        pSound->release();
        m_setReleaseSounds.erase(m_setReleaseSounds.begin());
    }
}


/*====================
  CSoundManager::UpdateMusic

  Check status of music stream
  ====================*/
void    CSoundManager::UpdateMusic()
{
    if (!m_bInitialized)
        return;

    m_pMusicChannelGroup->setVolume(sound_muteMusic ? 0.0f : sound_musicVolume * m_fMusicVolumeMult);

    if (m_pMusicChannel)
    {
        // If the current music track just reached the end...
        bool bPlaying(false);
        m_pMusicChannel->isPlaying(&bPlaying);

        if (!bPlaying)
        {
            // Loop the music track, if desired.
            if (m_cCurrentMusic.IsValid() && m_cCurrentMusic.bLoop && !m_cNextMusic.IsValid())
                m_cNextMusic = SPlayMusic(m_cCurrentMusic.sFilename, true, false);
            m_pMusicChannel->stop();
            assert(m_pMusicChannel == nullptr);
            m_pMusicChannel = nullptr;
        }
    }

    // If the music channel is null, attempt to start the next specified track
    if (m_pMusicChannel == nullptr)
        StartNextMusic();

    // If the music channel is still null, it means that no next track has been specified or it failed
    // to start.
    if (m_pMusicChannel == nullptr)
    {
        // So since no music is playing at this point, then queue up the next track in the playlist.
        if (m_bPlaylistActive)
        {
            PlayMusic(m_vMusicPlaylist[m_iNextPlaylistIndex], false, false, true);
            m_iNextPlaylistIndex = (m_iNextPlaylistIndex + 1) % int(m_vMusicPlaylist.size());
        }
    }
}


/*====================
  CSoundManager::MuteSFX

  Mute/unmute SFX sounds
  ====================*/
void    CSoundManager::MuteSFX(bool bMute)
{
    if (!m_bInitialized)
        return;

    m_pSFXChannelGroup->setMute(bMute);
}

/*====================
  CSoundManager::SetInterfaceVolume

  Set channel group volume
  ====================*/
void    CSoundManager::SetInterfaceVolume(float fVolume)
{
    if (!m_bInitialized)
        return;

    m_fInterfaceVolumeMult = fVolume;
}

/*====================
  CSoundManager::SetSFXVolume

  Set channel group volume
  ====================*/
void    CSoundManager::SetSFXVolume(float fVolume)
{
    if (!m_bInitialized)
        return;

    m_fSFXVolumeMult = fVolume;
}

/*====================
  CSoundManager::SetVoiceVolume

  Set channel group volume
  ====================*/
void    CSoundManager::SetVoiceVolume(float fVolume)
{
    if (!m_bInitialized)
        return;

    m_fVoiceVolumeMult = fVolume;
}

/*====================
  CSoundManager::SetMusicVolume

  Set channel group volume
  ====================*/
void    CSoundManager::SetMusicVolume(float fVolume)
{
    if (!m_bInitialized)
        return;

    m_fMusicVolumeMult = fVolume;
}

/*====================
  CSoundManager::StopActiveSound
  ====================*/
void    CSoundManager::StopActiveSound(FMOD::Channel *pChannel)
{
    if (!m_bInitialized)
        return;

    for (map<SoundHandle, FMOD::Channel *>::iterator it(m_mapActiveSounds.begin()), itEnd(m_mapActiveSounds.end()); it != itEnd; ++it)
    {
        if (it->second == pChannel)
        {
            m_mapActiveSounds.erase(it);
            return;
        }
    }
}


/*====================
  CSoundManager::MusicStopped
  ====================*/
void    CSoundManager::MusicStopped(FMOD::Channel *pChannel)
{
    assert(m_bInitialized);
    assert(!m_bInitialized || pChannel == m_pMusicChannel);

    if (m_bInitialized)
    {
        // Deactivate the music sound
        StopActiveSound(pChannel);
    }

    bool bAbort(false);
    // Sanity check: we are initialized
    if (!m_bInitialized)
    {
        Console.Err << _T("CSoundManager::MusicStopped() failed - not initialized.  Attempting to release...") << newl;
        bAbort = true;
    }
    // Sanity check: the music track being stopped is the music track being played
    else if (pChannel != m_pMusicChannel)
    {
        Console.Err << _T("CSoundManager::MusicStopped() failed - wrong music channel.  Attempting to release...") << newl;
        bAbort = true;
    }
    if (bAbort)
    {
        Console.FlushLogs();

        // Abort + cleanup
        if (pChannel == m_pMusicChannel)
            m_pMusicChannel = nullptr;

        FMOD::Sound *pSound(nullptr);
        pChannel->getCurrentSound(&pSound);
        if (pSound != nullptr)
            pSound->release();
        return;
    }

    // Deactivate the music track
    SPlayMusic sCurMusicMetadata(m_cCurrentMusic);
    m_cCurrentMusic = SPlayMusic();
    m_pMusicChannel = nullptr;

    // Sanity check: we stored the music track's metadata
    assert(sCurMusicMetadata.IsValid());
    if (sCurMusicMetadata.IsValid())
    {
        // Now that we're finished with the music track, unregister it to save memory
        if (sCurMusicMetadata.pSound != nullptr)
        {
            m_setReleaseSounds.insert(sCurMusicMetadata.pSound);
            sCurMusicMetadata.pSound = nullptr;
        }
    }
}


/*====================
  CSoundManager::UpdateFades

  Do the updates for sounds fading in/out
  ====================*/
void    CSoundManager::UpdateFades()
{
    uint uiNow(Host.GetTime());

    // Sounds that are fading in
    for (map<SoundHandle, CSoundFade*>::iterator it = m_mapFadingInSounds.begin(); it != m_mapFadingInSounds.end();)
    {
        map<SoundHandle, FMOD::Channel*>::iterator itFind(m_mapActiveSounds.find(it->first));
        if (itFind == m_mapActiveSounds.end() || !it->second->Update(itFind->second, uiNow))
        {
            K2_DELETE(it->second);
            STL_ERASE(m_mapFadingInSounds, it);
            continue;
        }
        it++;
    }

    // Sounds that are fading out
    for (map<SoundHandle, CSoundFade*>::iterator it = m_mapFadingOutSounds.begin(); it != m_mapFadingOutSounds.end();)
    {
        map<SoundHandle, FMOD::Channel*>::iterator itFind(m_mapActiveSounds.find(it->first));
        if (itFind == m_mapActiveSounds.end())
        {
            K2_DELETE(it->second);
            STL_ERASE(m_mapFadingOutSounds, it);
            continue;
        }
        if (!it->second->Update(itFind->second, uiNow))
        {
            itFind->second->stop();
            K2_DELETE(it->second);
            STL_ERASE(m_mapFadingOutSounds, it);
            continue;
        }
        it++;
    }
}


/*====================
  CSoundManager::UpdateDampen
  ====================*/
void    CSoundManager::UpdateDampen()
{
    uint uiNow(Host.GetTime());

    m_fSFXVolumeDampen = 1.0f;
    m_fInterfaceVolumeDampen = 1.0f;
    m_fVoiceVolumeDampen = 1.0f;
    m_fMusicVolumeDampen = 1.0f;

    float fMinDampen(1.0f);

    // Sounds that are fading out
    for (map<SoundHandle, CSoundDampen*>::iterator it(m_mapDampenSounds.begin()); it != m_mapDampenSounds.end();)
    {
        map<SoundHandle, FMOD::Channel*>::iterator itFind(m_mapActiveSounds.find(it->first));
        if (itFind == m_mapActiveSounds.end())
        {
            K2_DELETE(it->second);
            STL_ERASE(m_mapDampenSounds, it);
            continue;
        }
        else
        {
            fMinDampen = MIN(fMinDampen, it->second->GetDampen(uiNow));
            ++it;
        }
    }

    m_fSFXVolumeDampen = fMinDampen;
    m_fVoiceVolumeDampen = fMinDampen;
    m_fMusicVolumeDampen = fMinDampen;
}

void    CSoundManager::SoundTest(uint uiMs, uint uiFreq)
{
    size_t zLength((uiMs * uiFreq) / 1000);
    char *pData(K2_NEW_ARRAY(ctx_Sound, char, zLength));
    for (uint i(0); i < zLength; ++i)
        pData[i] = sin(float(i)) * 127;
    FMOD::Sound *pSound;
    FMOD_CREATESOUNDEXINFO exinfo;
    MemManager.Set(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
    exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    exinfo.length = uint(zLength);
    exinfo.numchannels = 1;
    exinfo.defaultfrequency = uiFreq;
    exinfo.format = FMOD_SOUND_FORMAT_PCM8;
    m_pFMODSystem->createSound(pData, FMOD_OPENMEMORY | FMOD_OPENRAW | FMOD_OPENUSER | FMOD_2D, &exinfo, &pSound);

    CSample *pSample = K2_NEW(ctx_Sound,  CSample)(pSound);
    Play2DSound(pSample->GetHandle());
    K2_DELETE_ARRAY(pData);
}

CMD(SoundTest)
{
    uint uiMs(3000);
    uint uiFreq(1440);
    if (vArgList.size() > 0)
        uiMs = AtoI(vArgList[0]);
    if (vArgList.size() > 1)
        uiFreq = AtoI(vArgList[1]);
    K2SoundManager.SoundTest(uiMs, uiFreq);
    return true;
}


/*====================
  CSoundManager::GetCurrentMusicName
  ====================*/
const tstring&  CSoundManager::GetCurrentMusicName()
{
    return m_cCurrentMusic.sFilename;
}


/*====================
  CSoundManager::GetCurrentMusicTime
  ====================*/
uint    CSoundManager::GetCurrentMusicTime()
{
    if (m_pMusicChannel == nullptr)
        return INVALID_TIME;

    uint uiPos(INVALID_TIME);
    m_pMusicChannel->getPosition(&uiPos, FMOD_TIMEUNIT_MS);
    return uiPos;
}


/*====================
  CSoundManager::GetCurrentMusicDuration
  ====================*/
uint    CSoundManager::GetCurrentMusicDuration()
{
    if (m_pMusicChannel == nullptr)
        return INVALID_TIME;

    FMOD::Sound *pSound(nullptr);
    m_pMusicChannel->getCurrentSound(&pSound);
    assert(pSound != nullptr);
    if (pSound == nullptr)
        return INVALID_TIME;

    uint uiLength(INVALID_TIME);
    pSound->getLength(&uiLength, FMOD_TIMEUNIT_MS);
    return uiLength;
}


/*====================
  CSoundManager::SetCurrentMusicTime
  ====================*/
void    CSoundManager::SetCurrentMusicTime(int iMillisec)
{
    if (m_pMusicChannel == nullptr)
        return;

    uint uiDuration(GetCurrentMusicDuration());
    assert(uiDuration != INVALID_TIME);
    if (uiDuration == INVALID_TIME)
        return;

    uint uiMillisec;
    if (iMillisec < 0)
    {
        // If the specified value is negative, then it's an offset from the end
        uiMillisec = (uint)(uiDuration + iMillisec);
    }
    else
    {
        uiMillisec = (uint)iMillisec;
    }

    m_pMusicChannel->setPosition(uiMillisec, FMOD_TIMEUNIT_MS);
}


/*====================
  CSoundManager::GetCurrentPlaylist
  ====================*/
const tsvector& CSoundManager::GetCurrentPlaylist()
{
    if (!m_bPlaylistActive)
        return VSNULL;

    return m_vMusicPlaylist;
}


/*====================
  CSoundManager::GetCurrentPlaylistIdx
  ====================*/
int CSoundManager::GetCurrentPlaylistIdx()
{
    if (!m_bPlaylistActive)
        return INVALID_INDEX;

    if (m_iNextPlaylistIndex == 0)
        return ((int)m_vMusicPlaylist.size() - 1);
    else
        return (m_iNextPlaylistIndex - 1);
}


/*--------------------
  PlayMusic <sound> [loop]
  --------------------*/
UI_VOID_CMD(PlayMusic, 1)
{
    bool bLoop(false);

    if (K2SoundManager.GetPlaylistActive())
        return; // don't override map playlist

    if (vArgList.size() > 1)
        bLoop = AtoB(vArgList[1]->Evaluate());

    K2SoundManager.PlayMusic(vArgList[0]->Evaluate(), bLoop, true);
}


/*--------------------
  PlayMusic <sound> [loop]
  --------------------*/
CMD(PlayMusic)
{
    if (!vArgList.empty())
        K2SoundManager.PlayMusic(vArgList[0], vArgList.size() > 1 ? AtoB(vArgList[1]) : false, true);
    return true;
}


/*--------------------
  PlaySound <sound> [volume] [channel]
  --------------------*/
UI_VOID_CMD(PlaySound, 1)
{
    ResHandle hSound(g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0]->Evaluate(), SND_2D), RES_SAMPLE));

    float fVolume(1.0f);
    if (vArgList.size() > 1)
        fVolume = AtoF(vArgList[1]->Evaluate());

    int iChannel(CHANNEL_AUTO);
    if (vArgList.size() > 2)
        iChannel = AtoI(vArgList[2]->Evaluate());

    K2SoundManager.Play2DSound(hSound, fVolume, iChannel);
}


/*--------------------
  PlaySoundDampen <sound> <dampen> [volume] [channel]
  --------------------*/
UI_VOID_CMD(PlaySoundDampen, 2)
{
    ResHandle hSound(g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0]->Evaluate(), SND_2D), RES_SAMPLE));

    float fDampen(AtoF(vArgList[1]->Evaluate()));

    float fVolume(1.0f);
    if (vArgList.size() > 2)
        fVolume = AtoF(vArgList[2]->Evaluate());

    int iChannel(CHANNEL_AUTO);
    if (vArgList.size() > 3)
        iChannel = AtoI(vArgList[3]->Evaluate());

    K2SoundManager.Play2DSound(hSound, fVolume, iChannel, 10, fDampen);
}


/*--------------------
  StopSound <channel>
  --------------------*/
UI_VOID_CMD(StopSound, 1)
{
    K2SoundManager.StopChannel(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  PlaySound <sound> [volume] [channel]
  --------------------*/
CMD(PlaySound)
{
    if (vArgList.size() < 1)
        return false;

    ResHandle hSound(g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], SND_2D), RES_SAMPLE));

    float fVolume(1.0f);
    if (vArgList.size() > 1)
        fVolume = AtoF(vArgList[1]);

    int iChannel(CHANNEL_AUTO);
    if (vArgList.size() > 2)
        iChannel = AtoI(vArgList[2]);

    K2SoundManager.Play2DSound(hSound, fVolume, iChannel);

    return true;
}


/*--------------------
  StopSound <channel>
  --------------------*/
CMD(StopSound)
{
    if (vArgList.size() < 1)
        return false;
    K2SoundManager.StopChannel(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  precachePlaySound
  --------------------*/
CMD_PRECACHE(PlaySound)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], SND_2D), RES_SAMPLE);
    return true;
}


/*--------------------
  precachePlaySoundLinear
  --------------------*/
CMD_PRECACHE(PlaySoundLinear)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], SND_2D), RES_SAMPLE);
    return true;
}


/*--------------------
  StopStreamingImmediately
  --------------------*/
CMD(StopStreamingImmediately)
{
    K2SoundManager.StopStreamingImmediately();
    return true;
}


/*--------------------
  StopMusic
  --------------------*/
UI_VOID_CMD(StopMusic, 0)
{
    bool bFadeOut(true);
    if (vArgList.size() >= 1)
        bFadeOut = AtoB(vArgList[0]->Evaluate());
    K2SoundManager.StopMusic(bFadeOut);
}


/*--------------------
  StopMusic
  --------------------*/
CMD(StopMusic)
{
    bool bFadeOut(true);
    if (vArgList.size() >= 1)
        bFadeOut = AtoB(vArgList[0]);
    K2SoundManager.StopMusic(bFadeOut);
    return true;
}


/*--------------------
  MusicInfo
  --------------------*/
CMD(MusicInfo)
{
    uint uiMusicDuration(K2SoundManager.GetCurrentMusicDuration());
    if (uiMusicDuration == INVALID_TIME)
    {
        Console << _T("No music playing") << newl;
        return true;
    }

    uint uiMusicTime(K2SoundManager.GetCurrentMusicTime());
    assert(uiMusicTime != INVALID_TIME);
    if (uiMusicTime == INVALID_TIME)
    {
        Console << _T("Invalid music position") << newl;
        return true;
    }


    Console << _T("Current track '^y") << K2SoundManager.GetCurrentMusicName() << _T("^*'") << newl;
    Console << _T("     (^c");
    {
        uint uiMinutes(uiMusicTime / 60000);
        uint uiSeconds((uiMusicTime / 1000) % 60);
        Console << XtoA(uiMinutes, FMT_PADZERO, 2)  << _T(":") << XtoA(uiSeconds, FMT_PADZERO, 2);
    }
    Console << _T("^* / ^c");
    {
        uint uiMinutes(uiMusicDuration / 60000);
        uint uiSeconds((uiMusicDuration / 1000) % 60);
        Console << XtoA(uiMinutes, FMT_PADZERO, 2)  << _T(":") << XtoA(uiSeconds, FMT_PADZERO, 2);
    }
    Console << _T("^*)") << newl;

    return true;
}


/*--------------------
  MusicSeek
  --------------------*/
CMD(MusicSeek)
{
    uint uiMusicDuration(K2SoundManager.GetCurrentMusicDuration());
    if (uiMusicDuration == INVALID_TIME)
    {
        Console << _T("No music playing") << newl;
        return true;
    }

    if (vArgList.size() >= 1)
    {
        tstring sArg(vArgList[0]);
        bool bFromEnd(false);
        if (sArg[0] == _T('-'))
        {
            bFromEnd = true;
            sArg = sArg.substr(1);
        }
        if (!sArg.empty())
        {
            uint uiMin(0);
            uint uiSec(0);

            size_t uiPos(sArg.find(_T(':')));
            if (uiPos != tstring::npos)
            {
                uiMin = AtoI(sArg);
                sArg = sArg.substr(uiPos + 1);
                uiSec = AtoI(sArg);
            }
            else
            {
                uiSec = AtoI(sArg);
            }
            while (uiSec >= 60)
            {
                ++uiMin;
                uiSec -= 60;
            }
            if (bFromEnd)
                Console << _T("Seeking to end minus ");
            else
                Console << _T("Seeking to ");
            Console << _T("(^c") << XtoA(uiMin, FMT_PADZERO, 2) << _T(":") << XtoA(uiSec, FMT_PADZERO, 2) << _T("^*)") << newl;

            int iMillisec = 60000 * uiMin + 1000 * uiSec;
            if (bFromEnd)
                iMillisec = -iMillisec;
            K2SoundManager.SetCurrentMusicTime(iMillisec);
            cmdMusicInfo();
            return true;
        }
    }

    Console << _T("Specify a time value in 'mm:ss' format, or in seconds.") << newl;
    Console << _T("If the time value begins with '-' then it represents a time offset from the end of the track.") << newl;
    Console << _T("Examples:  '03:55' or '-5' or '92'") << newl;
    cmdMusicInfo();
    return true;
}


/*--------------------
  PlaylistStart <sound> <sound> <sound> ...
  --------------------*/
CMD(PlaylistStart)
{
    if (vArgList.empty())
    {
        Console << _T("Specify a list of music tracks.  Example: ") << newl;
        Console << _T("  PlaylistStart music/track_01.mp3 music/track_02.mp3 music/track_03.mp3") << newl;
        return true;
    }
    bool bShuffle(true);
    K2SoundManager.PlayPlaylist(vArgList, bShuffle);
    return true;
}


/*--------------------
  PlaylistClear
  --------------------*/
CMD(PlaylistClear)
{
    K2SoundManager.ClearPlaylist();
    return true;
}


/*--------------------
  PlaylistNext
  --------------------*/
CMD(PlaylistNext)
{
    bool bFadeOut(true);
    if (vArgList.size() >= 1)
        bFadeOut = AtoB(vArgList[0]);
    bool bClearPlaylist(false);
    K2SoundManager.StopMusic(bFadeOut, bClearPlaylist);
    return true;
}


/*--------------------
  PlaylistInfo
  --------------------*/
CMD(PlaylistInfo)
{
    const tsvector& vPlaylist(K2SoundManager.GetCurrentPlaylist());
    if (vPlaylist.empty())
    {
        Console << _T("No active playlist") << newl;
        return true;
    }
    int iPlaylistIdx(K2SoundManager.GetCurrentPlaylistIdx());

    Console << _T("Active playlist (") << INT_SIZE(vPlaylist.size()) << _T(" tracks): ") << newl;
    for (size_t i(0); i < vPlaylist.size(); ++i)
    {
        Console << _T("^y");
        if ((int)i == iPlaylistIdx)
            Console << _T("->");
        else
            Console << _T("  ");

        Console << _T("'") << vPlaylist[i] << _T("'") << newl;
    }
    cmdMusicInfo();
    return true;
}


/*--------------------
  RefreshSoundDrivers
  --------------------*/
UI_VOID_CMD(RefreshSoundDrivers, 0)
{
    K2SoundManager.RefreshDrivers();
}


/*--------------------
  AddSoundPlaybackDrivers
  --------------------*/
UI_VOID_CMD(AddSoundPlaybackDrivers, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    tsvector vDriverList;
    
    tstring sDriver;
    for (int i(0); K2SoundManager.GetDriver(i, sDriver); ++i)
    {
        vDriverList.push_back(sDriver);
    }

    for (int i(0); i < int(vDriverList.size()); ++i)
    {
        mapParams[_T("label")] = vDriverList[i];

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(i), mapParams);
    }
}


/*--------------------
  AddSoundRecordingDrivers
  --------------------*/
UI_VOID_CMD(AddSoundRecordingDrivers, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    tsvector vDriverList;
    
    tstring sDriver;
    for (int i(0); K2SoundManager.GetRecordDriver(i, sDriver); ++i)
    {
        vDriverList.push_back(sDriver);
    }

    for (int i(0); i < int(vDriverList.size()); ++i)
    {
        mapParams[_T("label")] = vDriverList[i];

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(i), mapParams);
    }
}


/*--------------------
  AddSoundOutputs
  --------------------*/
UI_VOID_CMD(AddSoundOutputs, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    tstring sWidth, sHeight;
    if (pList)
    {
        sWidth = pList->GetListItemWidth();
        sHeight = pList->GetListItemHeight();
    }
    else
    {
        sWidth = _T("0");
        sHeight = _T("0");
    }

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    svector vDriverList;

    mapParams[_T("label")] = _T("Autodetect");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T(""), mapParams);
#ifdef linux
    mapParams[_T("label")] = _T("ALSA");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("alsa"), mapParams);
#if TKTK // This seems removed as of 2023
    mapParams[_T("label")] = _T("ESound");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("esd"), mapParams);
    mapParams[_T("label")] = _T("OSS");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("oss"), mapParams);
    mapParams[_T("label")] = _T("OSSv4");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("ossv4"), mapParams);
#endif
    mapParams[_T("label")] = _T("PulseAudio");
    pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), _T("pulseaudio"), mapParams);
    
#endif
#ifdef _WIN32
    // fill in if needed
#endif
}


/*--------------------
  precachePlaySoundLooping
  --------------------*/
CMD_PRECACHE(PlaySoundLooping)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], SND_LOOP), RES_SAMPLE);
    return true;
}


/*--------------------
  precachePlaySoundLoopingLinear
  --------------------*/
CMD_PRECACHE(PlaySoundLoopingLinear)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], SND_LOOP), RES_SAMPLE);
    return true;
}


/*--------------------
  precachePlaySoundStationary
  --------------------*/
CMD_PRECACHE(PlaySoundStationary)
{
    if (vArgList.size() < 1)
        return false;

    g_ResourceManager.Register(K2_NEW(ctx_Sound,  CSample)(vArgList[0], 0), RES_SAMPLE);
    return true;
}


#endif // K2_NOSOUND
