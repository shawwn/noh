// (C)2007 S2 Games
// c_modifier.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_sceneentitymodifier.h"

#include "i_emitter.h"
#include "c_effectthread.h"
#include "c_sceneentity.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CModifierDef::~CModifierDef
  ====================*/
CModifierDef::~CModifierDef()
{
}


/*====================
  CModifierDef::CModifierDef
  ====================*/
CModifierDef::CModifierDef
(
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyf &tfAlpha,
    const CTemporalPropertyf &tfParam0,
    const CTemporalPropertyf &tfParam1,
    const CTemporalPropertyf &tfParam2,
    const CTemporalPropertyf &tfParam3,
    ResHandle hMaterial,
    const CTemporalPropertyv3 &tv3Offset,
    const tstring &sSkin
) :
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_tfParam0(tfParam0),
m_tfParam1(tfParam1),
m_tfParam2(tfParam2),
m_tfParam3(tfParam3),
m_hMaterial(hMaterial),
m_tv3Offset(tv3Offset),
m_sSkin(sSkin)
{
}


/*====================
  CSceneEntityModifier::~CSceneEntityModifier
  ====================*/
CSceneEntityModifier::~CSceneEntityModifier()
{
}


/*====================
  CSceneEntityModifier::CSceneEntityModifier
  ====================*/
CSceneEntityModifier::CSceneEntityModifier()
{
}


/*====================
  CSceneEntityModifier::CSceneEntityModifier
  ====================*/
CSceneEntityModifier::CSceneEntityModifier(uint uiStartTime, CEffectThread *pEffectThread, const CModifierDef &eSettings) :
IEffectInstance(pEffectThread),
m_uiStartTime(uiStartTime),
m_uiLastUpdateTime(uiStartTime),
m_bDead(false),
m_iLife(eSettings.GetLife()),
m_iTimeNudge(eSettings.GetTimeNudge()),
m_iDelay(eSettings.GetDelay()),
m_bLoop(eSettings.GetLoop()),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfParam0(eSettings.GetParam0()),
m_tfParam1(eSettings.GetParam1()),
m_tfParam2(eSettings.GetParam2()),
m_tfParam3(eSettings.GetParam3()),
m_hMaterial(eSettings.GetMaterial()),
m_tv3Offset(eSettings.GetOffset()),
m_sSkin(eSettings.GetSkin())
{
    m_pEffectThread = pEffectThread;
    m_bActive = false;
}


/*====================
  CSceneEntityModifier::Update
  ====================*/
bool    CSceneEntityModifier::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CSceneEntityModifier::Update");

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

    if (iDeltaTime <= 0)
        return true;
    
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
            m_bDead = true;
            return false;
        }
    }

    m_uiLastUpdateTime = uiMilliseconds;

    m_bActive = GetActive();

    if (m_pEffectThread->GetExpire() && m_iLife == -1)
    {
        m_bDead = true;
        return false;
    }

    return true;
}


/*====================
  CSceneEntityModifier::IsDead
  ====================*/
bool    CSceneEntityModifier::IsDead()
{
    return m_bDead;
}


/*====================
  CSceneEntityModifier::Modify
  ====================*/
void    CSceneEntityModifier::Modify(CSceneEntity &cEntity) const
{
    if (!m_bActive)
        return;

    float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);

    float fLerp;
    if (m_iLife != -1)
        fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
    else
        fLerp = 0.0f;

    cEntity.color *= CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));
    cEntity.s1 = m_tfParam0.Evaluate(fLerp, fTime);
    cEntity.t1 = m_tfParam1.Evaluate(fLerp, fTime);
    cEntity.s2 = m_tfParam2.Evaluate(fLerp, fTime);
    cEntity.t2 = m_tfParam3.Evaluate(fLerp, fTime);

    if (m_hMaterial != INVALID_RESOURCE)
    {
        cEntity.flags |= SCENEENT_SINGLE_MATERIAL;
        cEntity.hSkin = m_hMaterial;
    }
    else if (!m_sSkin.empty())
    {
        cEntity.hSkin = g_ResourceManager.GetSkin(cEntity.hRes, m_sSkin);
    }

    cEntity.SetPosition(cEntity.GetPosition() + m_tv3Offset.Evaluate(fLerp, fTime));
}
