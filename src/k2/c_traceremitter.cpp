// (C)2007 S2 Games
// c_traceremitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_traceremitter.h"
#include "c_particlesystem.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTracerEmitterDef::~CTracerEmitterDef
  ====================*/
CTracerEmitterDef::~CTracerEmitterDef()
{
}


/*====================
  CTracerEmitterDef::CTracerEmitterDef
  ====================*/
CTracerEmitterDef::CTracerEmitterDef
(
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    const tstring &sOwnerA,
    const tstring &sOwnerB,
    const tstring &sBoneA,
    const tstring &sBoneB,
    const CVec3f &v3PosA,
    const CVec3f &v3PosB,
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyf &tfAlpha,
    const CTemporalPropertyRangef &trfWidth,
    const CTemporalPropertyRangef &trfLength,
    const CTemporalPropertyRangef &trfSpeed,
    const CTemporalPropertyRangef &trfAcceleration,
    const CTemporalPropertyRangef &trfTaper,
    const CTemporalPropertyRangef &trfTile,
    const CTemporalPropertyRangef &trfFrame,
    const CTemporalPropertyRangef &trfParam,
    ResHandle hMaterial
) :
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_sOwnerA(sOwnerA),
m_sOwnerB(sOwnerB),
m_sBoneA(sBoneA),
m_sBoneB(sBoneB),
m_v3PosA(v3PosA),
m_v3PosB(v3PosB),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_trfWidth(trfWidth),
m_trfLength(trfLength),
m_trfSpeed(trfSpeed),
m_trfAcceleration(trfAcceleration),
m_trfTaper(trfTaper),
m_trfTile(trfTile),
m_trfFrame(trfFrame),
m_trfParam(trfParam),
m_hMaterial(hMaterial)
{
}


/*====================
  CTracerEmitterDef::Spawn
  ====================*/
IEmitter*   CTracerEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CTracerEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CTracerEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CTracerEmitter::~CTracerEmitter
  ====================*/
CTracerEmitter::~CTracerEmitter()
{
}


/*====================
  CTracerEmitter::CTracerEmitter
  ====================*/
CTracerEmitter::CTracerEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CTracerEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    TSNULL,
    eSettings.GetOwnerA(),
    TSNULL,
    V3_ZERO,
    V3_ZERO,
    DIRSPACE_LOCAL,
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_sBoneA(eSettings.GetBoneA()),
m_sBoneB(eSettings.GetBoneB()),
m_v3PosA(eSettings.GetPosA()),
m_v3PosB(eSettings.GetPosB()),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfWidth(eSettings.GetWidth()),
m_tfLength(eSettings.GetLength()),
m_tfSpeed(eSettings.GetSpeed()),
m_tfAcceleration(eSettings.GetAcceleration()),
m_tfTaper(eSettings.GetTaper()),
m_tfTile(eSettings.GetTile()),
m_tfFrame(eSettings.GetFrame()),
m_tfParam(eSettings.GetParam()),
m_hMaterial(eSettings.GetMaterial())
{
    if (pOwner != nullptr)
    {
        m_pOwnerA = pOwner;
        m_pOwnerB = pOwner;
    }
    else
    {
        m_pOwnerA = GetOwnerPointer(eSettings.GetOwnerA());
        m_pOwnerB = GetOwnerPointer(eSettings.GetOwnerB());
    }

    // Initialize m_v3LastPos
    m_v3LastPosA = TransformPoint(m_v3PosA + GetBonePosition(uiStartTime, m_pOwnerA, m_sBoneA), GetAxis(m_pOwnerA), GetPosition(m_v3PosA, m_pOwnerA));
    m_v3LastPosB = TransformPoint(m_v3PosB + GetBonePosition(uiStartTime, m_pOwnerB, m_sBoneB), GetAxis(m_pOwnerB), GetPosition(m_v3PosA, m_pOwnerB));

    m_fBeamStartPos = 0.0f;

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CTracerEmitter::Update
  ====================*/
bool    CTracerEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CTracerEmitter::Update");

    if (m_uiPauseBegin)
        ResumeFromPause(uiMilliseconds);

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

    if (iDeltaTime <= 0)
    {
        UpdateNextEmitter(uiMilliseconds, pfnTrace);
        return true;
    }

    // Calculate temporal properties
    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
    {
        // Kill us if we've lived out our entire expire life
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
        {
            m_bActive = false;
            return false;
        }
        
        if (m_iExpireLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
        else
            fLerp = 0.0f;
    }
    else
    {
        if (m_iLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
        else
            fLerp = 0.0f;
    }

    float   fSpeed(m_tfSpeed.Evaluate(fLerp, fTime));
    float   fAcceleration(m_tfAcceleration.Evaluate(fLerp, fTime));

    m_uiLastUpdateTime = uiMilliseconds;

    m_bActive = m_pParticleSystem->GetActive() && m_uiExpireTime == INVALID_TIME;

    if (!GetVisibility(m_sBoneA, m_pOwnerA) || !GetVisibility(m_sBoneB, m_pOwnerB))
        m_bActive = false;

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
            return false;
        }
    }

    m_v3LastPosA = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwnerA, m_sBoneA), GetAxis(m_pOwnerA), GetPosition(m_v3PosA, m_pOwnerA));
    m_v3LastPosB = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwnerB, m_sBoneB), GetAxis(m_pOwnerB), GetPosition(m_v3PosB, m_pOwnerB));

    m_fBeamStartPos = fSpeed * fTime + 0.5f * fAcceleration * fTime * fTime;

    if (m_fBeamStartPos > Length(m_v3LastPosB - m_v3LastPosA))
    {
        if (m_bLoop)
        {
            m_fBeamStartPos = 0.0f;

            if (m_iLife != -1)
                m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
            else
                m_uiStartTime = uiMilliseconds;
        }
        else
        {
            m_bActive = false;
            return false;
        }
    }

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    m_bbBounds.Clear();
    m_bbBounds.AddPoint(m_v3LastPosA);
    m_bbBounds.AddPoint(m_v3LastPosB);

    return true;
}


/*====================
  CTracerEmitter::GetNumBeams
  ====================*/
uint    CTracerEmitter::GetNumBeams()
{
    if (m_bActive)
        return 1;
    else
        return 0;
}


/*====================
  CTracerEmitter::GetBeam
  ====================*/
bool    CTracerEmitter::GetBeam(uint uiIndex, SBeam &outBeam)
{
    if (uiIndex != 0)
        return false;

    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
    {
        if (m_iExpireLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
        else
            fLerp = 0.0f;
    }
    else
    {
        if (m_iLife != -1)
            fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
        else
            fLerp = 0.0f;
    }

    CVec3f  v3Dir;
    float   fLength;
    Decompose(m_v3LastPosB - m_v3LastPosA, v3Dir, fLength);

    outBeam.v3Start = m_v3LastPosA + v3Dir * MAX(m_fBeamStartPos - m_tfLength.Evaluate(fLerp, fTime), 0.0f);
    outBeam.v3End = m_v3LastPosA + v3Dir * MIN(m_fBeamStartPos, fLength);
    outBeam.v4StartColor = outBeam.v4EndColor = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));
    outBeam.fStartSize = outBeam.fEndSize = m_tfWidth.Evaluate(fLerp, fTime);
    outBeam.fTaper = m_tfTaper.Evaluate(fLerp, fTime);
    outBeam.fTile = m_tfTile.Evaluate(fLerp, fTime);
    outBeam.fStartFrame = outBeam.fEndFrame = m_tfFrame.Evaluate(fLerp, fTime);
    outBeam.fStartParam = outBeam.fEndParam = m_tfParam.Evaluate(fLerp, fTime);

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

            outBeam.v3Start = TransformPoint(outBeam.v3Start, aAxis, v3Pos, fScale);
            outBeam.v3End = TransformPoint(outBeam.v3End, aAxis, v3Pos, fScale);

        } break;
    }

    outBeam.hMaterial = m_hMaterial;

    return true;
}


/*====================
  CTracerEmitter::OnDelete
  ====================*/
void    CTracerEmitter::OnDelete(IEmitter *pEmitter)
{
    if (m_pOwnerA == pEmitter)
        m_pOwnerA = nullptr;

    if (m_pOwnerB == pEmitter)
        m_pOwnerB = nullptr;
}
