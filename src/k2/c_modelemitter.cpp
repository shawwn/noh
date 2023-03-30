// (C)2006 S2 Games
// c_modelemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_modelemitter.h"

#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_skeleton.h"
#include "c_model.h"
#include "c_effectthread.h"
#include "c_effect.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CModelEmitterDef::~CModelEmitterDef
  ====================*/
CModelEmitterDef::~CModelEmitterDef()
{
}


/*====================
  CModelEmitterDef::CModelEmitterDef
  ====================*/
CModelEmitterDef::CModelEmitterDef
(
    const tstring &sName,
    const tstring &sOwner,
    const CRangei &riLife,
    const CRangei &riExpireLife,
    const CRangei &riTimeNudge,
    const CRangei &riDelay,
    bool bLoop,
    EDirectionalSpace eDirectionalSpace,
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
    const CTemporalPropertyv3 &tv3Color,
    const CTemporalPropertyf &tfAlpha,
    const CTemporalPropertyRangef &trfPitch,
    const CTemporalPropertyRangef &trfRoll,
    const CTemporalPropertyRangef &trfYaw,
    const CTemporalPropertyRangef &trfScale,
    const CTemporalPropertyRangef &trfParam0,
    const CTemporalPropertyRangef &trfParam1,
    const CTemporalPropertyRangef &trfParam2,
    const CTemporalPropertyRangef &trfParam3,
    ResHandle hModel,
    SkinHandle hSkin,
    ResHandle hMaterial,
    const tstring &sAnim,
    bool bParentModel,
    bool bParentSkeleton,
    const tsvector &vEmitters
) :
m_sName(sName),
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_eDirectionalSpace(eDirectionalSpace),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_trfPitch(trfPitch),
m_trfRoll(trfRoll),
m_trfYaw(trfYaw),
m_trfScale(trfScale),
m_trfParam0(trfParam0),
m_trfParam1(trfParam1),
m_trfParam2(trfParam2),
m_trfParam3(trfParam3),
m_hModel(hModel),
m_hSkin(hSkin),
m_hMaterial(hMaterial),
m_sAnim(sAnim),
m_bParentModel(bParentModel),
m_bParentSkeleton(bParentSkeleton),
m_vEmitters(vEmitters)
{
}


/*====================
  CModelEmitterDef::Spawn
  ====================*/
IEmitter*   CModelEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CModelEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CModelEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CModelEmitterDef::AddEmitterDef
  ====================*/
void    CModelEmitterDef::AddEmitterDef(IEmitterDef *pEmitterDef)
{
    m_vEmitterDefs.push_back(pEmitterDef);
}


/*====================
  CModelEmitter::~CModelEmitter
  ====================*/
CModelEmitter::~CModelEmitter()
{
    SAFE_DELETE(m_pSkeleton);
    SAFE_DELETE(m_pImbeddedEmitter);
}


/*====================
  CModelEmitter::CModelEmitter
  ====================*/
CModelEmitter::CModelEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CModelEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    eSettings.GetName(),
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
m_pSkeleton(NULL),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfPitch(eSettings.GetPitch()),
m_tfRoll(eSettings.GetRoll()),
m_tfYaw(eSettings.GetYaw()),
m_tfScale(eSettings.GetScale()),
m_tfParam0(eSettings.GetParam0()),
m_tfParam1(eSettings.GetParam1()),
m_tfParam2(eSettings.GetParam2()),
m_tfParam3(eSettings.GetParam3()),
m_hModel(eSettings.GetModel()),
m_hSkin(eSettings.GetSkin()),
m_hMaterial(eSettings.GetMaterial()),
m_bParentModel(eSettings.GetParentModel()),
m_bParentSkeleton(eSettings.GetParentSkeleton()),
m_pImbeddedEmitter(NULL)
{
    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    const tstring &sAnim(eSettings.GetAnim());

    if (!sAnim.empty() && (m_hModel != INVALID_RESOURCE || m_bParentModel) && !m_bParentSkeleton)
    {
        m_pSkeleton = K2_NEW(ctx_Effects,  CSkeleton)();

        m_pSkeleton->SetModel(m_bParentModel && GetModel() ? GetModel()->GetHandle() : m_hModel);
        m_pSkeleton->StartAnim(sAnim, uiStartTime, 0);
    }

    Update(INVALID_TIME, NULL);

    IEmitter *pCurrentEmitter(NULL);
    const tsvector &vEmitters(eSettings.GetEmitters());
    if (!vEmitters.empty())
    {
        CEffect *pEffect(m_pParticleSystem->GetEffect());
        
        tsvector_cit cit(vEmitters.begin());

        IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));

        if (pEmitterDef != NULL)
            m_pImbeddedEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this);

        if (m_pImbeddedEmitter != NULL)
        {
            ++cit;

            pCurrentEmitter = m_pImbeddedEmitter;

            for (; cit != vEmitters.end(); ++cit)
            {
                IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));
                IEmitter *pNewEmitter(NULL);

                if (pEmitterDef != NULL)
                    pNewEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this);

                pCurrentEmitter->SetNextEmitter(pNewEmitter);
                pCurrentEmitter = pNewEmitter;
            }
        }
    }

    vector<IEmitterDef *>::const_iterator itEnd(eSettings.GetEmitterDefs().end());
    for (vector<IEmitterDef *>::const_iterator it(eSettings.GetEmitterDefs().begin()); it != itEnd; ++it)
    {
        for (int i(0); i < (*it)->GetCount(); ++i)
        {
            IEmitter *pNewEmitter((*it)->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this));
            if (pNewEmitter != NULL)
            {
                if (pCurrentEmitter == NULL)
                {
                    m_pImbeddedEmitter = pNewEmitter;
                    pCurrentEmitter = m_pImbeddedEmitter;
                }
                else
                {
                    pCurrentEmitter->SetNextEmitter(pNewEmitter);
                    pCurrentEmitter = pNewEmitter;
                }
            }
        }
    }

    m_bActive = false;
    m_bVisible = false;
}


/*====================
  CModelEmitter::UpdateEmbeddedEmitter
  ====================*/
inline
bool    CModelEmitter::UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, bool bExpire)
{
    if (bExpire)
        m_pImbeddedEmitter->Expire(uiMilliseconds);

    bool bRet(m_pImbeddedEmitter->Update(uiMilliseconds, pfnTrace));

    return bRet;
}


/*====================
  CModelEmitter::Update
  ====================*/
bool    CModelEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CModelEmitter::Update");

    if (uiMilliseconds == INVALID_TIME)
    {
        uiMilliseconds = m_uiStartTime;
    }
    else
    {
        if (m_uiPauseBegin)
            ResumeFromPause(uiMilliseconds);

        int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

        if (iDeltaTime <= 0)
        {
            UpdateNextEmitter(uiMilliseconds, pfnTrace);
            return true;
        }

        m_uiLastUpdateTime = uiMilliseconds;
    }

    m_bVisible = GetVisibility();

    // Calculate temporal properties
    float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= uiMilliseconds && (m_iLife == -1 || m_bLoop) && m_iExpireLife != 0.0f)
        fLerp = m_iExpireLife != -1 ? MIN(float(uiMilliseconds - m_uiExpireTime) / m_iExpireLife, 1.0f) : 0.0f;
    else
        fLerp = m_iLife != -1 ? MIN(float(uiMilliseconds - m_uiStartTime) / m_iLife, 1.0f) : 0.0f;

    bool bDead(false);

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
    {
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
        {
            bDead = true;
            m_bVisible = false;
        }
    }

    // Kill us if we've lived out our entire life
    if (m_iLife != -1 && (uiMilliseconds > m_iLife + m_uiStartTime))
    {
        if (m_bLoop)
            m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
        else
        {
            bDead = true;
            m_bVisible = false;
        }
    }

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    if (m_eDirectionalSpace == DIRSPACE_LOCAL)
    {
        CAxis   aBoneAxis;
        CVec3f  v3BonePos;

        GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

        v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
        aAxis = aAxis * aBoneAxis;
        m_aLastAxis = aAxis * CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
    }
    else
    {
        v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
        m_aLastAxis = CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
    }

    m_v3LastPos = v3Pos;
    m_fLastScale = GetScale() * m_tfScale.Evaluate(fLerp, fTime);

    ResHandle hModel(INVALID_RESOURCE);
    if (m_bParentModel)
    {
        CModel *pModel(GetModel());

        if (pModel)
            hModel = pModel->GetHandle();

        if (m_pSkeleton)
            m_pSkeleton->SetModel(hModel);
    }
    else
    {
        hModel = m_hModel;
    }

    if (m_pSkeleton)
        m_pSkeleton->Pose(uiMilliseconds);

    m_bActive = true;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    if (m_pImbeddedEmitter != NULL)
    {
        if (!UpdateEmbeddedEmitter(uiMilliseconds, pfnTrace, !m_bVisible))
        {
            IEmitter *pEmitter(m_pImbeddedEmitter);

            if (pEmitter->GetNextEmitter() != NULL)
            {
                m_pImbeddedEmitter = pEmitter->GetNextEmitter();
                pEmitter->SetNextEmitter(NULL);
            }
            else
            {
                m_pImbeddedEmitter = NULL;
            }

            K2_DELETE(pEmitter);
        }
    }

    m_bbBounds.Clear();

    if (hModel != INVALID_RESOURCE)
    {
        m_bbBounds = g_ResourceManager.GetModelBounds(hModel);
        m_bbBounds.Transform(m_v3LastPos, m_aLastAxis, m_fLastScale);
    }

    if (bDead && m_pImbeddedEmitter == NULL)
        return false;

    return true;
}


/*====================
  CModelEmitter::GetNumEntities
  ====================*/
uint    CModelEmitter::GetNumEntities()
{
    if (m_bVisible)
        return 1;
    else
        return 0;
}


/*====================
  CModelEmitter::GetEntity
  ====================*/
bool    CModelEmitter::GetEntity(uint uiIndex, CSceneEntity &outEntity)
{
    if (uiIndex != 0)
        return false;

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
    outEntity.axis = m_aLastAxis;
    outEntity.scale = m_fLastScale;

    outEntity.color = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));

    outEntity.s1 = m_tfParam0.Evaluate(fLerp, fTime);
    outEntity.t1 = m_tfParam1.Evaluate(fLerp, fTime);
    outEntity.s2 = m_tfParam2.Evaluate(fLerp, fTime);
    outEntity.t2 = m_tfParam3.Evaluate(fLerp, fTime);
    
    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
        } break;
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            outEntity.scale *= fScale;
            outEntity.SetPosition(TransformPoint(outEntity.GetPosition(), aAxis, v3Pos, fScale));

            outEntity.axis = aAxis * outEntity.axis;
        } break;
    }
    
    if (m_bParentSkeleton)
    {
        outEntity.skeleton = GetSkeleton();

        if (m_pOwner == OWNER_SOURCE)
            outEntity.scale = m_pParticleSystem->GetEffectThread()->GetSourceScale() * m_tfScale.Evaluate(fLerp, fTime);
        else if (m_pOwner == OWNER_TARGET)
            outEntity.scale = m_pParticleSystem->GetEffectThread()->GetTargetScale() * m_tfScale.Evaluate(fLerp, fTime);
    }
    else
    {
        outEntity.skeleton = m_pSkeleton;
    }

    if (m_bParentModel)
    {
        CModel *pModel(GetModel());

        if (pModel)
            outEntity.hRes = pModel->GetHandle();
        else
            return false;
        
        outEntity.hSkin = 0;
    }
    else
    {
        outEntity.hRes = m_hModel;
        outEntity.hSkin = m_hSkin;
    }
        
    outEntity.objtype = OBJTYPE_MODEL;
    outEntity.flags = SCENEENT_USE_AXIS | SCENEENT_SOLID_COLOR;

    if (m_hMaterial != INVALID_RESOURCE)
    {
        outEntity.flags |= SCENEENT_SINGLE_MATERIAL;
        outEntity.hSkin = m_hMaterial;
    }

    return true;
}


/*====================
  CModelEmitter::GetNumEmitters
  ====================*/
uint    CModelEmitter::GetNumEmitters()
{
    return m_pImbeddedEmitter != NULL ? 1 : 0;
}


/*====================
  CModelEmitter::GetEmitter
  ====================*/
IEmitter*   CModelEmitter::GetEmitter(uint uiIndex)
{
    return uiIndex == 0 ? m_pImbeddedEmitter : NULL;
}


/*====================
  CModelEmitter::Expire
  ====================*/
void    CModelEmitter::Expire(uint uiMilliseconds)
{
    if (m_pImbeddedEmitter != NULL)
        m_pImbeddedEmitter->Expire(uiMilliseconds);

    IEmitter::Expire(uiMilliseconds);
}
