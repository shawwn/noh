// (C)2006 S2 Games
// c_groundspriteemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_groundspriteemitter.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CGroundSpriteEmitterDef::~CGroundSpriteEmitterDef
  ====================*/
CGroundSpriteEmitterDef::~CGroundSpriteEmitterDef()
{
}


/*====================
  CGroundSpriteEmitterDef::CGroundSpriteEmitterDef
  ====================*/
CGroundSpriteEmitterDef::CGroundSpriteEmitterDef
(
    const tstring &sOwner,
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyf &tfAlpha,
    const CTemporalPropertyRangef &trfPitch,
    const CTemporalPropertyRangef &trfRoll,
    const CTemporalPropertyRangef &trfYaw,
    const CTemporalPropertyRangef &trfWidth,
    const CTemporalPropertyRangef &trfHeight,
    const CTemporalPropertyRangef &trfScale,
    const CTemporalPropertyRangef &trfFrame,
    const CTemporalPropertyRangef &trfParam,
    ResHandle hMaterial,
    EDirectionalSpace eDirectionalSpace
) :
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_trfPitch(trfPitch),
m_trfRoll(trfRoll),
m_trfYaw(trfYaw),
m_trfWidth(trfWidth),
m_trfHeight(trfHeight),
m_trfScale(trfScale),
m_trfFrame(trfFrame),
m_trfParam(trfParam),
m_hMaterial(hMaterial),
m_eDirectionalSpace(eDirectionalSpace)
{
}


/*====================
  CGroundSpriteEmitterDef::Spawn
  ====================*/
IEmitter*   CGroundSpriteEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CGroundSpriteEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CGroundSpriteEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CGroundSpriteEmitter::~CGroundSpriteEmitter
  ====================*/
CGroundSpriteEmitter::~CGroundSpriteEmitter()
{
}


/*====================
  CGroundSpriteEmitter::CGroundSpriteEmitter
  ====================*/
CGroundSpriteEmitter::CGroundSpriteEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CGroundSpriteEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    TSNULL,
    eSettings.GetOwner(),
    eSettings.GetBone(),
    eSettings.GetPos(),
    eSettings.GetOffset(),
    eSettings.GetDirectionalSpace(),
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfPitch(eSettings.GetPitch()),
m_tfRoll(eSettings.GetRoll()),
m_tfYaw(eSettings.GetYaw()),
m_tfWidth(eSettings.GetWidth()),
m_tfHeight(eSettings.GetHeight()),
m_tfScale(eSettings.GetScale()),
m_tfFrame(eSettings.GetFrame()),
m_tfParam(eSettings.GetParam()),
m_hMaterial(eSettings.GetMaterial())
{
    m_aLastAxis = GetAxis();
    m_v3LastPos = GetPosition();
    m_fLastScale = GetScale();

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CGroundSpriteEmitter::Update
  ====================*/
bool    CGroundSpriteEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CGroundSpriteEmitter::Update");

    if (m_uiPauseBegin)
        ResumeFromPause(uiMilliseconds);

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

    if (iDeltaTime <= 0)
    {
        UpdateNextEmitter(uiMilliseconds, pfnTrace);
        return true;
    }
    
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

    m_uiLastUpdateTime = uiMilliseconds;

    m_bActive = GetVisibility();

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    CAxis   aBoneAxis;
    CVec3f  v3BonePos;

    GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

    v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);

    aAxis = aAxis * aBoneAxis;
    
    m_aLastAxis = aAxis;
    m_v3LastPos = v3Pos;
    m_fLastScale = fScale;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

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

    m_bbBounds.Clear();

    float fSize(MAX(m_tfWidth.Evaluate(fLerp, fTime), m_tfHeight.Evaluate(fLerp, fTime)) * m_tfScale.Evaluate(fLerp, fTime) * m_fLastScale * DIAG);

    m_bbBounds.AddBox(CBBoxf(CVec3f(m_v3LastPos.x - fSize, m_v3LastPos.y - fSize, m_v3LastPos.z - fSize),
        CVec3f(m_v3LastPos.x + fSize, m_v3LastPos.y + fSize, m_v3LastPos.z + fSize)));

    return true;
}


/*====================
  CGroundSpriteEmitter::GetNumEntities
  ====================*/
uint    CGroundSpriteEmitter::GetNumEntities()
{
    if (m_bActive)
        return 1;
    else
        return 0;
}


/*====================
  CGroundSpriteEmitter::GetEntity
  ====================*/
bool    CGroundSpriteEmitter::GetEntity(uint uiIndex, CSceneEntity &outEntity)
{
    if (uiIndex != 0)
        return false;

    // Calculate temporal properties
    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
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

    outEntity.SetPosition(m_v3LastPos);
    outEntity.color = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));
    outEntity.angle = CVec3f(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
    outEntity.width = m_tfWidth.Evaluate(fLerp, fTime);
    outEntity.height = m_tfHeight.Evaluate(fLerp, fTime);
    outEntity.scale = m_tfScale.Evaluate(fLerp, fTime) * m_fLastScale;
    outEntity.frame = m_tfFrame.Evaluate(fLerp, fTime);
    outEntity.param = m_tfParam.Evaluate(fLerp, fTime);

    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
            if (m_eDirectionalSpace == DIRSPACE_LOCAL)
            {
                CVec3f v3Dir(M_GetForwardVecFromAngles(outEntity.angle));

                v3Dir = TransformPoint(v3Dir, m_aLastAxis);

                outEntity.angle = M_GetAnglesFromForwardVec(v3Dir);
            }
        } break;
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            outEntity.scale *= fScale;
            outEntity.SetPosition(TransformPoint(outEntity.GetPosition(), aAxis, v3Pos, fScale));

            if (m_eDirectionalSpace == DIRSPACE_LOCAL)
            {
                CVec3f v3Dir(M_GetForwardVecFromAngles(outEntity.angle));

                v3Dir = TransformPoint(v3Dir, aAxis * m_aLastAxis);

                outEntity.angle = M_GetAnglesFromForwardVec(v3Dir);
            }

        } break;
    }

    outEntity.hRes = m_hMaterial;
    outEntity.objtype = OBJTYPE_GROUNDSPRITE;
    outEntity.flags = SCENEENT_USE_AXIS;

    return true;
}
