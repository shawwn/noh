// (C)2006 S2 Games
// c_particlesystem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_particlesystem.h"
#include "i_emitter.h"
#include "c_effectthread.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CParticleSystemDef::~CParticleSystemDef
  ====================*/
CParticleSystemDef::~CParticleSystemDef()
{
}


/*====================
  CParticleSystemDef::CParticleSystemDef
  ====================*/
CParticleSystemDef::CParticleSystemDef
(
    ESystemSpace eSpace,
    float fScale, 
    const CVec3f &v3Color
) :
m_eSpace(eSpace),
m_fScale(fScale),
m_v3Color(v3Color)
{
}


/*====================
  CParticleSystemDef::AddEmitterDef
  ====================*/
void    CParticleSystemDef::AddEmitterDef(IEmitterDef *pEmitterDef)
{
    m_vEmitterDefs.push_back(pEmitterDef);
}


/*====================
  CParticleSystem::~CParticleSystem
  ====================*/
CParticleSystem::~CParticleSystem()
{
    for (EmitterList::iterator it(m_vEmitters.begin()); it != m_vEmitters.end(); ++it)
        K2_DELETE(*it);
}


/*====================
  CParticleSystem::CParticleSystem
  ====================*/
CParticleSystem::CParticleSystem()
{
}


/*====================
  CParticleSystem::CParticleSystem
  ====================*/
CParticleSystem::CParticleSystem(uint uiStartTime, CEffectThread *pEffectThread, const CParticleSystemDef &psSettings) :
IEffectInstance(pEffectThread),
m_uiStartTime(uiStartTime),
m_uiLastUpdateTime(uiStartTime),
m_eSpace(psSettings.GetSpace()),
m_fScale(psSettings.GetScale()),
m_v3Color(psSettings.GetColor() * pEffectThread->GetColor()),
m_v3CustomPos(V3_ZERO),
m_aCustomAxis(0.0f, 0.0f, 0.0f),
m_fCustomScale(1.0f)
{
    PROFILE("CParticleSysem::CParticleSystem");

    if (pEffectThread == NULL)
        return;

    for (EmitterDefList::const_iterator it(psSettings.GetEmitterDefs().begin()); it != psSettings.GetEmitterDefs().end(); ++it)
    {
        for (int i(0); i < (*it)->GetCount(); ++i)
        {
            IEmitter *pNewEmitter((*it)->Spawn(uiStartTime, this, NULL));

            if (!pNewEmitter)
                continue;

            if (!pNewEmitter->GetName().empty())
                m_mapNamedEmitters[pNewEmitter->GetName()] = pNewEmitter;

            m_vEmitters.push_back(pNewEmitter);
        }
    }
}


/*====================
  CParticleSystem::Update
  ====================*/
bool    CParticleSystem::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CParticleSysem::Update");

    EmitterList::iterator it(m_vEmitters.begin());
    while (it != m_vEmitters.end())
    {
        IEmitter *pEmitter(*it);
        if (pEmitter->Update(uiMilliseconds, pfnTrace))
            ++it;
        else
        {
            for (EmitterList::iterator itB(m_vEmitters.begin()); itB != m_vEmitters.end(); ++itB)
                (*itB)->OnDelete(pEmitter);

            if (!pEmitter->GetName().empty())
                m_mapNamedEmitters.erase(pEmitter->GetName());

            if (pEmitter->GetNextEmitter() != NULL)
            {
                *it = pEmitter->GetNextEmitter();
                pEmitter->SetNextEmitter(NULL);
                K2_DELETE(pEmitter);
                ++it;
            }
            else
            {
                K2_DELETE(pEmitter);
                it = m_vEmitters.erase(it);
            }
        }
    }

    return m_vEmitters.size() != 0;
}


/*====================
  CParticleSystem::Update
  ====================*/
bool    CParticleSystem::UpdatePaused(uint uiMilliseconds)
{
    EmitterList::iterator it(m_vEmitters.begin());
    while (it != m_vEmitters.end())
    {
        (*it)->UpdatePaused(uiMilliseconds);
        ++it;
    }

    return m_vEmitters.size() != 0;
}


/*====================
  CParticleSystem::IsDead
  ====================*/
bool    CParticleSystem::IsDead()
{
    return m_vEmitters.size() == 0;
}


/*====================
  CParticleSystem::GetEmitter
  ====================*/
IEmitter*   CParticleSystem::GetEmitter(const tstring &sName)
{
    NamedEmitterMap::iterator findit(m_mapNamedEmitters.find(sName));

    if (findit != m_mapNamedEmitters.end())
        return findit->second;
    else
        return NULL;
}


/*====================
  CParticleSystem::GetSourceBonePosition
  ====================*/
CVec3f  CParticleSystem::GetSourceBonePosition(const tstring &sBone, uint uiTime)
{
    return m_pEffectThread->GetSourceBonePosition(sBone, uiTime) / (m_fScale * m_pEffectThread->GetSourceEffectScale());
}


/*====================
  CParticleSystem::GetSourceBoneAxisPos
  ====================*/
void    CParticleSystem::GetSourceBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    m_pEffectThread->GetSourceBoneAxisPos(sBone, uiTime, aOutAxis, v3OutPos);
    v3OutPos /= (m_fScale * m_pEffectThread->GetSourceEffectScale());
}


/*====================
  CParticleSystem::GetSourceBoneTransform
  ====================*/
matrix43_t* CParticleSystem::GetSourceBoneTransform(const tstring &sBone)
{
    return m_pEffectThread->GetSourceBoneTransform(sBone);
}


/*====================
  CParticleSystem::GetSourceVisibility
  ====================*/
bool    CParticleSystem::GetSourceVisibility()
{
    return m_pEffectThread->GetSourceVisibility();
}


/*====================
  CParticleSystem::GetSourceVisibility
  ====================*/
bool    CParticleSystem::GetSourceVisibility(const tstring &sBone)
{
    return m_pEffectThread->GetSourceVisibility(sBone);
}


/*====================
  CParticleSystem::SourceTransformAxis
  ====================*/
CAxis   CParticleSystem::SourceTransformAxis(const CAxis &aAxis)
{
    if (m_eSpace == WORLD_SPACE)
        return m_pEffectThread->SourceTransformAxis(aAxis);
    else
        return aAxis;
}


/*====================
  CParticleSystem::SourceTransformPosition
  ====================*/
CVec3f  CParticleSystem::SourceTransformPosition(const CVec3f &v3Pos)
{
    if (m_eSpace == WORLD_SPACE)
        return m_pEffectThread->SourceTransformPosition(v3Pos);
    else
        return v3Pos;
}


/*====================
  CParticleSystem::GetSourceMeshPosition
  ====================*/
CVec3f  CParticleSystem::GetSourceMeshPosition(const tstring &sMesh)
{
    return m_pEffectThread->GetSourceMeshPosition(sMesh);
}


/*====================
  CParticleSystem::GetSourceRandomPositionOnMesh
  ====================*/
CVec3f  CParticleSystem::GetSourceRandomPositionOnMesh(const tstring &sMesh)
{
    return m_pEffectThread->GetSourceRandomPositionOnMesh(sMesh);
}


/*====================
  CParticleSystem::GetSourceRandomPositionWithNormalOnMesh
  ====================*/
CVec3f  CParticleSystem::GetSourceRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Dir)
{
    return m_pEffectThread->GetSourceRandomPositionWithNormalOnMesh(sMesh, v3Dir);
}


/*====================
  CParticleSystem::GetSourceRandomPositionOnMesh
  ====================*/
CVec3f  CParticleSystem::GetSourceRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir)
{
    return m_pEffectThread->GetSourceRandomPositionOnMesh(sMesh, v3Dir);
}


/*====================
  CParticleSystem::GetSourceRandomPositionOnSkeleton
  ====================*/
CVec3f  CParticleSystem::GetSourceRandomPositionOnSkeleton()
{
    return m_pEffectThread->GetSourceRandomPositionOnSkeleton();
}


/*====================
  CParticleSystem::GetSourcePosition
  ====================*/
const CVec3f&   CParticleSystem::GetSourcePosition() const
{
    return m_pEffectThread->GetSourcePosition();
}


/*====================
  CParticleSystem::GetSourceAxis
  ====================*/
const CAxis&    CParticleSystem::GetSourceAxis() const
{
    return m_pEffectThread->GetSourceAxis();
}


/*====================
  CParticleSystem::GetSourceScale
  ====================*/
float   CParticleSystem::GetSourceScale() const
{
    return m_pEffectThread->GetSourceScale() * m_pEffectThread->GetSourceEffectScale();
}


/*====================
  CParticleSystem::GetSourceSkeleton
  ====================*/
CSkeleton*  CParticleSystem::GetSourceSkeleton() const
{
    return m_pEffectThread->GetSourceSkeleton();
}


/*====================
  CParticleSystem::GetSourceModel
  ====================*/
CModel* CParticleSystem::GetSourceModel() const
{
    return m_pEffectThread->GetSourceModel();
}


/*====================
  CParticleSystem::GetTargetBonePosition
  ====================*/
CVec3f  CParticleSystem::GetTargetBonePosition(const tstring &sBone, uint uiTime)
{
    return m_pEffectThread->GetTargetBonePosition(sBone, uiTime) / (m_fScale * m_pEffectThread->GetTargetEffectScale());
}


/*====================
  CParticleSystem::GetTargetBoneAxisPos
  ====================*/
void    CParticleSystem::GetTargetBoneAxisPos(const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    m_pEffectThread->GetTargetBoneAxisPos(sBone, uiTime, aOutAxis, v3OutPos);
    v3OutPos /= (m_fScale * m_pEffectThread->GetTargetEffectScale());
}


/*====================
  CParticleSystem::GetTargetBoneTransform
  ====================*/
matrix43_t* CParticleSystem::GetTargetBoneTransform(const tstring &sBone)
{
    return m_pEffectThread->GetTargetBoneTransform(sBone);
}


/*====================
  CParticleSystem::GetTargetVisibility
  ====================*/
bool    CParticleSystem::GetTargetVisibility()
{
    return m_pEffectThread->GetTargetVisibility();
}


/*====================
  CParticleSystem::GetTargetVisibility
  ====================*/
bool    CParticleSystem::GetTargetVisibility(const tstring &sBone)
{
    return m_pEffectThread->GetTargetVisibility(sBone);
}


/*====================
  CParticleSystem::TargetTransformAxis
  ====================*/
CAxis   CParticleSystem::TargetTransformAxis(const CAxis &aAxis)
{
    if (m_eSpace == WORLD_SPACE)
        return m_pEffectThread->TargetTransformAxis(aAxis);
    else
        return aAxis;
}


/*====================
  CParticleSystem::TargetTransformPosition
  ====================*/
CVec3f  CParticleSystem::TargetTransformPosition(const CVec3f &v3Pos)
{
    if (m_eSpace == WORLD_SPACE)
        return m_pEffectThread->TargetTransformPosition(v3Pos);
    else
        return v3Pos;
}


/*====================
  CParticleSystem::GetTargetMeshPosition
  ====================*/
CVec3f  CParticleSystem::GetTargetMeshPosition(const tstring &sMesh)
{
    return m_pEffectThread->GetTargetMeshPosition(sMesh);
}


/*====================
  CParticleSystem::GetTargetRandomPositionOnMesh
  ====================*/
CVec3f  CParticleSystem::GetTargetRandomPositionOnMesh(const tstring &sMesh)
{
    return m_pEffectThread->GetTargetRandomPositionOnMesh(sMesh);
}


/*====================
  CParticleSystem::GetTargetRandomPositionWithNormalOnMesh
  ====================*/
CVec3f  CParticleSystem::GetTargetRandomPositionWithNormalOnMesh(const tstring &sMesh, CVec3f &v3Dir)
{
    return m_pEffectThread->GetTargetRandomPositionWithNormalOnMesh(sMesh, v3Dir);
}


/*====================
  CParticleSystem::GetTargetRandomPositionOnMesh
  ====================*/
CVec3f  CParticleSystem::GetTargetRandomPositionOnMesh(const tstring &sMesh, const CVec3f &v3Dir)
{
    return m_pEffectThread->GetTargetRandomPositionOnMesh(sMesh, v3Dir);
}


/*====================
  CParticleSystem::GetTargetRandomPositionOnSkeleton
  ====================*/
CVec3f  CParticleSystem::GetTargetRandomPositionOnSkeleton()
{
    return m_pEffectThread->GetTargetRandomPositionOnSkeleton();
}


/*====================
  CParticleSystem::GetTargetPosition
  ====================*/
const CVec3f&   CParticleSystem::GetTargetPosition() const
{
    return m_pEffectThread->GetTargetPosition();
}


/*====================
  CParticleSystem::GetTargetAxis
  ====================*/
const CAxis&    CParticleSystem::GetTargetAxis() const
{
    return m_pEffectThread->GetTargetAxis();
}


/*====================
  CParticleSystem::GetTargetScale
  ====================*/
float   CParticleSystem::GetTargetScale() const
{
    return m_pEffectThread->GetTargetScale() * m_pEffectThread->GetTargetEffectScale();
}

/*====================
  CParticleSystem::GetTargetSkeleton
  ====================*/
CSkeleton*  CParticleSystem::GetTargetSkeleton() const
{
    return m_pEffectThread->GetTargetSkeleton();
}


/*====================
  CParticleSystem::GetTargetModel
  ====================*/
CModel* CParticleSystem::GetTargetModel() const
{
    return m_pEffectThread->GetTargetModel();
}


/*====================
  CParticleSystem::GetCustomBonePosition
  ====================*/
CVec3f  CParticleSystem::GetCustomBonePosition(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime)
{
    if (m_eSpace != BONE_SPACE)
        return m_pEffectThread->GetCustomBonePosition(pSkeleton, sBone, uiTime);
    else
        return CVec3f(0.0f, 0.0f, 0.0f);
}


/*====================
  CParticleSystem::GetCustomVisibility
  ====================*/
bool    CParticleSystem::GetCustomVisibility(CSkeleton *pSkeleton, const tstring &sBone)
{
    return m_pEffectThread->GetCustomVisibility(pSkeleton, sBone);
}


/*====================
  CParticleSystem::GetCustomBoneAxisPos
  ====================*/
void    CParticleSystem::GetCustomBoneAxisPos(CSkeleton *pSkeleton, const tstring &sBone, uint uiTime, CAxis &aOutAxis, CVec3f &v3OutPos)
{
    if (m_eSpace != BONE_SPACE)
    {
        m_pEffectThread->GetCustomBoneAxisPos(pSkeleton, sBone, uiTime, aOutAxis, v3OutPos);
        return;
    }
    else
    {
        aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
        v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
    }
}


/*====================
  CParticleSystem::GetCamera
  ====================*/
CCamera*    CParticleSystem::GetCamera()
{
    return m_pEffectThread->GetCamera();
}


/*====================
  CParticleSystem::GetWorld
  ====================*/
CWorld* CParticleSystem::GetWorld()
{
    return m_pEffectThread->GetWorld();
}


/*====================
  CParticleSystem::GetBounds
  ====================*/
CBBoxf  CParticleSystem::GetBounds() const
{
    CBBoxf bbBounds;

    EmitterList::const_iterator itEnd(m_vEmitters.end());
    for (EmitterList::const_iterator it(m_vEmitters.begin()); it != itEnd; ++it)
        bbBounds.AddBox((*it)->GetBounds());

    if (m_eSpace == ENTITY_SPACE)
        bbBounds.Transform(GetSourcePosition(), GetSourceAxis(), GetSourceScale());

    return bbBounds;
}

/*====================
  CParticleSystem::GetCustomVisibility
  ====================*/
bool    CParticleSystem::GetCustomVisibility() const
{
    return GetEffectThread()->GetCustomVisibility();
}

/*====================
  CParticleSystem::Expire
  ====================*/
void    CParticleSystem::Expire(uint uiMilliseconds)
{
    EmitterList::const_iterator itEnd(m_vEmitters.end());
    for (EmitterList::const_iterator it(m_vEmitters.begin()); it != itEnd; ++it)
        (*it)->Expire(uiMilliseconds);
}
