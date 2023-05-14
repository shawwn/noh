// (C)2007 S2 Games
// c_entityeffect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityeffect.h"

#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR(Entity, Effect)

DEFINE_ENTITY_DESC(CEntityEffect, 1)
{
    //
    // You'd better have a good reason for adding extra fields to this (8 is a happy number)
    //

    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_ROUNDPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles"), TYPE_V3F, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ahEffect[0]"), TYPE_RESHANDLE, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ahEffect[1]"), TYPE_RESHANDLE, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiSourceEntityIndex"), TYPE_GAMEINDEX, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTargetEntityIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3TargetPosition"), TYPE_ROUNDPOS3D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3TargetAngles"), TYPE_V3F, 0, 0));
}
//=============================================================================


/*====================
  CEntityEffect::CEntityEffect
  ====================*/
CEntityEffect::CEntityEffect() :
m_uiSourceEntityIndex(INVALID_INDEX),

m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO)
{
}


/*====================
  CEntityEffect::Baseline
  ====================*/
void    CEntityEffect::Baseline()
{
    m_v3Position = V3_ZERO;
    m_v3Angles = V3_ZERO;
    m_ahEffect[0] = INVALID_RESOURCE;
    m_ahEffect[1] = INVALID_RESOURCE;

    m_uiSourceEntityIndex = INVALID_INDEX;
    
    m_uiTargetEntityIndex = INVALID_INDEX;
    m_v3TargetPosition = V3_ZERO;
    m_v3TargetAngles = V3_ZERO;
}


/*====================
  CEntityEffect::GetSnapshot
  ====================*/
void    CEntityEffect::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    snapshot.WriteRoundPos3D(m_v3Position);
    snapshot.WriteField(m_v3Angles);
    snapshot.WriteResHandle(m_ahEffect[0]);
    snapshot.WriteResHandle(m_ahEffect[1]);

    snapshot.WriteGameIndex(m_uiSourceEntityIndex);
    
    snapshot.WriteGameIndex(m_uiTargetEntityIndex);
    snapshot.WriteRoundPos3D(m_v3TargetPosition);
    snapshot.WriteField(m_v3TargetAngles);
}


/*====================
  CEntityEffect::ReadSnapshot
  ====================*/
bool    CEntityEffect::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    snapshot.ReadRoundPos3D(m_v3Position);
    snapshot.ReadField(m_v3Angles);
    snapshot.ReadResHandle(m_ahEffect[0]);
    snapshot.ReadResHandle(m_ahEffect[1]);

    snapshot.ReadGameIndex(m_uiSourceEntityIndex);
    
    snapshot.ReadGameIndex(m_uiTargetEntityIndex);
    snapshot.ReadRoundPos3D(m_v3TargetPosition);
    snapshot.ReadField(m_v3TargetAngles);

    Validate();
    
    return true;
}


/*====================
  CEntityEffect::Copy
  ====================*/
void    CEntityEffect::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    if (B.GetType() != Entity_Effect)
        return;

    const CEntityEffect *pB(static_cast<const CEntityEffect *>(&B));
    if (!pB)
        return;

    const CEntityEffect &C(*pB);
    
    m_uiSourceEntityIndex = C.m_uiSourceEntityIndex;
    m_uiTargetEntityIndex = C.m_uiTargetEntityIndex;
    m_v3TargetPosition = C.m_v3TargetPosition;
    m_v3TargetAngles = C.m_v3TargetAngles;
}


/*====================
  CEntityEffect::Interpolate
  ====================*/
void    CEntityEffect::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
    CEntityEffect *pPrevStateEffect(static_cast<CEntityEffect *>(pPrevState));
    CEntityEffect *pNextStateEffect(static_cast<CEntityEffect *>(pNextState));

    m_v3Angles = M_LerpAngles(fLerp, pPrevState->GetAngles(), pNextState->GetAngles());
    m_v3Position = LERP(fLerp, pPrevState->GetPosition(), pNextState->GetPosition());
    m_fScale = LERP(fLerp, pPrevState->GetScale(), pNextState->GetScale());
    m_v3TargetPosition = LERP(fLerp, pPrevStateEffect->GetTargetPosition(), pNextStateEffect->GetTargetPosition());
    m_v3TargetAngles = M_LerpAngles(fLerp, pPrevStateEffect->GetTargetAngles(), pNextStateEffect->GetTargetAngles());
}


/*====================
  CEntityEffect::UpdateEffectThread
  ====================*/
void    CEntityEffect::UpdateEffectThread(CEffectThread *pEffectThread)
{
    IVisualEntity *pSourceEntity(m_uiSourceEntityIndex != INVALID_INDEX ? Game.GetVisualEntity(m_uiSourceEntityIndex) : nullptr);
    if (pSourceEntity)
    {
        pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pSourceEntity->GetModel()));
        pEffectThread->SetSourceSkeleton(pSourceEntity->GetSkeleton());

        pEffectThread->SetSourcePos(pSourceEntity->GetPosition());
        pEffectThread->SetSourceAxis(pSourceEntity->GetAxis());

        pEffectThread->SetSourceScale(pSourceEntity->GetBaseScale() * pSourceEntity->GetScale());

        if (pEffectThread->GetUseEntityEffectScale())
            pEffectThread->SetSourceEffectScale(pSourceEntity->GetEffectScale() / (pSourceEntity->GetBaseScale() * pSourceEntity->GetScale()));
        else
            pEffectThread->SetSourceEffectScale(1.0f);
    }
    else
    {
        pEffectThread->SetSourceModel(nullptr);
        pEffectThread->SetSourceSkeleton(nullptr);

        pEffectThread->SetSourcePos(m_v3Position);
        pEffectThread->SetSourceAxis(CAxis(m_v3Angles));
    }

    IVisualEntity *pTargetEntity(m_uiTargetEntityIndex != INVALID_INDEX ? Game.GetVisualEntity(m_uiTargetEntityIndex) : nullptr);
    if (pTargetEntity)
    {
        pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pTargetEntity->GetModel()));
        pEffectThread->SetTargetSkeleton(pTargetEntity->GetSkeleton());

        pEffectThread->SetTargetPos(pTargetEntity->GetPosition());
        pEffectThread->SetTargetAxis(pTargetEntity->GetAxis());

        pEffectThread->SetTargetScale(pTargetEntity->GetBaseScale() * pTargetEntity->GetScale());

        if (pEffectThread->GetUseEntityEffectScale())
            pEffectThread->SetTargetEffectScale(pTargetEntity->GetEffectScale() / (pTargetEntity->GetBaseScale() * pTargetEntity->GetScale()));
        else
            pEffectThread->SetTargetEffectScale(1.0f);
    }
    else
    {
        pEffectThread->SetTargetModel(nullptr);
        pEffectThread->SetTargetSkeleton(nullptr);

        pEffectThread->SetTargetPos(m_v3TargetPosition);
        pEffectThread->SetTargetAxis(CAxis(m_v3TargetAngles));
    }
}
