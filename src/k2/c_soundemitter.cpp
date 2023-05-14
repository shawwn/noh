// (C)2007 S2 Games
// c_soundemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_soundemitter.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_soundmanager.h"
#include "c_sample.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CSoundEmitterDef::~CSoundEmitterDef
  ====================*/
CSoundEmitterDef::~CSoundEmitterDef()
{
}


/*====================
  CSoundEmitterDef::CSoundEmitterDef
  ====================*/
CSoundEmitterDef::CSoundEmitterDef
(
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    int iFadeIn,
    int iFadeOutStartTime,
    int iFadeOut,
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
    const CRangef &rfFalloff,
    const CRangef &rfVolume,
    const CTemporalPropertyRangef &trfPitch,
    uint uiSoundFlags,
    ResHandle hSample,
    const CRangef &rfSpeed1,
    const CRangef &rfSpeed2,
    int iSpeedUpTime,
    int iSlowDownTime,
    const CRangef &rfFalloffStart,
    const CRangef &rfFalloffEnd
) :
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_iFadeIn(iFadeIn),
m_iFadeOutStartTime(iFadeOutStartTime),
m_iFadeOut(iFadeOut),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_rfFalloff(rfFalloff),
m_rfVolume(rfVolume),
m_trfPitch(trfPitch),
m_uiSoundFlags(uiSoundFlags),
m_hSample(hSample),
m_rfSpeed1(rfSpeed1),
m_rfSpeed2(rfSpeed2),
m_iSpeedUpTime(iSpeedUpTime),
m_iSlowDownTime(iSlowDownTime),
m_rfFalloffStart(rfFalloffStart),
m_rfFalloffEnd(rfFalloffEnd)
{
}


/*====================
  CSoundEmitterDef::Spawn
  ====================*/
IEmitter*   CSoundEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CSoundEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CSoundEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CSoundEmitter::~CSoundEmitter
  ====================*/
CSoundEmitter::~CSoundEmitter()
{
    if (m_hSoundHandle != INVALID_INDEX)
        K2SoundManager.StopHandle(m_hSoundHandle);
}


/*====================
  CSoundEmitter::CSoundEmitter
  ====================*/
CSoundEmitter::CSoundEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CSoundEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    TSNULL,
    _T("source"),
    eSettings.GetBone(),
    eSettings.GetPos(),
    eSettings.GetOffset(),
    DIRSPACE_GLOBAL,
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_rfFalloff(eSettings.GetFalloff()),
m_rfVolume(eSettings.GetVolume()),
m_tfPitch(eSettings.GetPitch()),
m_iSoundFlags(eSettings.GetSoundFlags()),
m_hSample(eSettings.GetSample()),
m_iFadeIn(eSettings.GetFadeIn()),
m_iFadeOutStartTime(eSettings.GetFadeOutStartTime()),
m_iFadeOut(eSettings.GetFadeOut()),
m_rfSpeed1(eSettings.GetSpeed1()),
m_rfSpeed2(eSettings.GetSpeed2()),
m_iSpeedUpTime(eSettings.GetSpeedUpTime()),
m_iSlowDownTime(eSettings.GetSlowDownTime()),
m_rfFalloffStart(eSettings.GetFalloffStart()),
m_rfFalloffEnd(eSettings.GetFalloffEnd()),
m_bStarted(false)
{
    m_v3LastPos = GetPosition();
    m_fLastScale = GetScale();

    /*if (!m_iDelay)
    {
        if (m_iSoundFlags & SND_2D)
            m_hSoundHandle = K2SoundManager.Play2DSFXSound(m_hSample, m_rfVolume, -1, 128, (m_iSoundFlags & SND_LOOP) == SND_LOOP, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime);
        else if (m_iSoundFlags & (SND_LINEARFALLOFF | SND_SQUAREDFALLOFF))
            m_hSoundHandle = K2SoundManager.PlaySFXSound(m_hSample, &m_v3LastPos, nullptr, m_rfVolume, m_rfFalloffStart, -1, 128, m_iSoundFlags, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime, m_rfFalloffEnd);
        else
            m_hSoundHandle = K2SoundManager.PlaySFXSound(m_hSample, &m_v3LastPos, nullptr, m_rfVolume, m_rfFalloff, -1, 128, m_iSoundFlags, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime, m_rfFalloffEnd);
        m_bStarted = true;
    }*/

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CSoundEmitter::Update
  ====================*/
bool    CSoundEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CSoundEmitter::Update");

    if (m_uiPauseBegin)
        ResumeFromPause(uiMilliseconds);

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

    if (iDeltaTime <= 0)
    {
        UpdateNextEmitter(uiMilliseconds, pfnTrace);
        return true;
    }

    if (!m_bStarted)
    {
        if (m_iSoundFlags & SND_2D)
            m_hSoundHandle = K2SoundManager.Play2DSFXSound(m_hSample, m_rfVolume, -1, 128, (m_iSoundFlags & SND_LOOP) == SND_LOOP, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime);
        else if (m_iSoundFlags & (SND_LINEARFALLOFF | SND_SQUAREDFALLOFF))
            m_hSoundHandle = K2SoundManager.PlaySFXSound(m_hSample, &m_v3LastPos, nullptr, m_rfVolume, m_rfFalloffStart, -1, 128, m_iSoundFlags, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime, m_rfFalloffEnd);
        else
            m_hSoundHandle = K2SoundManager.PlaySFXSound(m_hSample, &m_v3LastPos, nullptr, m_rfVolume, m_rfFalloff, -1, 128, m_iSoundFlags, m_iFadeIn, m_iFadeOutStartTime, m_iFadeOut, m_iSpeedUpTime, m_rfSpeed1, m_rfSpeed2, m_iSlowDownTime, m_rfFalloffEnd);
        m_bStarted = true;
    }

    // mute the sound if the particle system is not visible
    K2SoundManager.SetMute(m_hSoundHandle, !m_pParticleSystem->GetCustomVisibility());
    
    // Kill us if we've lived out our entire life
    if (m_iLife != -1 && (uiMilliseconds > m_iLife + m_uiStartTime))
    {
        if (m_bLoop)
        {
            m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
        }
        else
        {
            m_bActive = false;
        }
        return false;
    }

    //float fDeltaTime(iDeltaTime * SEC_PER_MS);

    m_uiLastUpdateTime = uiMilliseconds;

    m_bActive = m_pParticleSystem->GetActive() && m_uiExpireTime == INVALID_TIME;

    if (!GetVisibility())
        m_bActive = false;

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);

    m_aLastAxis = aAxis;
    m_v3LastPos = v3Pos;
    m_fLastScale = fScale;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
        return false;

    CVec3f v3WorldPos(m_v3LastPos);

    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
        } break;
    case BONE_SPACE:
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            v3WorldPos = TransformPoint(v3WorldPos, aAxis, v3Pos, fScale);
        } break;
    }

    return K2SoundManager.UpdateHandle(m_hSoundHandle, v3WorldPos, V3_ZERO, true);
}
