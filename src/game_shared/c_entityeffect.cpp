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
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	CEntityEffect::s_pvFields;

DEFINE_ENT_ALLOCATOR(Entity, Effect)
//=============================================================================


/*====================
  CEntityEffect::CEntityEffect
  ====================*/
CEntityEffect::CEntityEffect() :
IVisualEntity(NULL),

m_uiSourceEntityIndex(INVALID_INDEX),

m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO)
{
}


/*====================
  CEntityEffect::Baseline
  ====================*/
void	CEntityEffect::Baseline()
{
	m_v3Position = V3_ZERO;
	m_v3Angles = V3_ZERO;
	m_ahEffect[0] = INVALID_INDEX;
	m_ahEffect[1] = INVALID_INDEX;

	m_uiSourceEntityIndex = INVALID_INDEX;
	
	m_uiTargetEntityIndex = INVALID_INDEX;
	m_v3TargetPosition = V3_ZERO;
	m_v3TargetAngles = V3_ZERO;
}


/*====================
  CEntityEffect::GetSnapshot
  ====================*/
void	CEntityEffect::GetSnapshot(CEntitySnapshot &snapshot) const
{
	snapshot.AddField(m_v3Position);
	snapshot.AddField(m_v3Angles);
	snapshot.AddResHandle(m_ahEffect[0]);
	snapshot.AddResHandle(m_ahEffect[1]);

	snapshot.AddGameIndex(m_uiSourceEntityIndex);
	
	snapshot.AddGameIndex(m_uiTargetEntityIndex);
	snapshot.AddRoundPosition(m_v3TargetPosition);
	snapshot.AddField(m_v3TargetAngles);
}


/*====================
  CEntityEffect::ReadSnapshot
  ====================*/
bool	CEntityEffect::ReadSnapshot(CEntitySnapshot &snapshot)
{
	snapshot.ReadNextField(m_v3Position);
	snapshot.ReadNextField(m_v3Angles);
	snapshot.ReadNextResHandle(m_ahEffect[0]);
	snapshot.ReadNextResHandle(m_ahEffect[1]);

	snapshot.ReadNextGameIndex(m_uiSourceEntityIndex);
	
	snapshot.ReadNextGameIndex(m_uiTargetEntityIndex);
	snapshot.ReadNextRoundPosition(m_v3TargetPosition);
	snapshot.ReadNextField(m_v3TargetAngles);

	Validate();
	
	return true;
}


/*====================
  CEntityEffect::GetTypeVector

  You'd better have a good reason for adding extra fields to this (8 is a happy number)
  ====================*/
const vector<SDataField>&	CEntityEffect::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->push_back(SDataField(_T("m_v3Position"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_v3Angles"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_ahEffect[0]"), FIELD_PUBLIC, TYPE_RESHANDLE));
		s_pvFields->push_back(SDataField(_T("m_ahEffect[1]"), FIELD_PUBLIC, TYPE_RESHANDLE));

		s_pvFields->push_back(SDataField(_T("m_uiSourceEntityIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
	
		s_pvFields->push_back(SDataField(_T("m_uiTargetEntityIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
		s_pvFields->push_back(SDataField(_T("m_v3TargetPosition"), FIELD_PUBLIC, TYPE_ROUNDPOSITION));
		s_pvFields->push_back(SDataField(_T("m_v3TargetAngles"), FIELD_PUBLIC, TYPE_V3F));
	}

	return *s_pvFields;
}


/*====================
  CEntityEffect::Copy
  ====================*/
void	CEntityEffect::Copy(const IGameEntity &B)
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
void	CEntityEffect::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
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
void	CEntityEffect::UpdateEffectThread(CEffectThread *pEffectThread)
{
	IVisualEntity *pSourceEntity(m_uiSourceEntityIndex != INVALID_INDEX ? Game.GetVisualEntity(m_uiSourceEntityIndex) : NULL);
	if (pSourceEntity)
	{
		pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pSourceEntity->GetModelHandle()));
		pEffectThread->SetSourceSkeleton(pSourceEntity->GetSkeleton());

		pEffectThread->SetSourcePos(pSourceEntity->GetPosition());
		pEffectThread->SetSourceAxis(pSourceEntity->GetAxis());

		if (pEffectThread->GetUseEntityEffectScale())
			pEffectThread->SetSourceScale(pSourceEntity->GetScale() * pSourceEntity->GetScale2() * GetEffectScale());
		else
			pEffectThread->SetSourceScale(pSourceEntity->GetScale() * pSourceEntity->GetScale2());
	}
	else
	{
		pEffectThread->SetSourceModel(NULL);
		pEffectThread->SetSourceSkeleton(NULL);

		pEffectThread->SetSourcePos(m_v3Position);
		pEffectThread->SetSourceAxis(CAxis(m_v3Angles));
	}

	IVisualEntity *pTargetEntity(m_uiTargetEntityIndex != INVALID_INDEX ? Game.GetVisualEntity(m_uiTargetEntityIndex) : NULL);
	if (pTargetEntity)
	{
		pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pTargetEntity->GetModelHandle()));
		pEffectThread->SetTargetSkeleton(pTargetEntity->GetSkeleton());

		pEffectThread->SetTargetPos(pTargetEntity->GetPosition());
		pEffectThread->SetTargetAxis(pTargetEntity->GetAxis());

		if (pEffectThread->GetUseEntityEffectScale())
			pEffectThread->SetTargetScale(pTargetEntity->GetScale() * pTargetEntity->GetScale2() * GetEffectScale());
		else
			pEffectThread->SetTargetScale(pTargetEntity->GetScale() * pTargetEntity->GetScale2());
	}
	else
	{
		pEffectThread->SetTargetModel(NULL);
		pEffectThread->SetTargetSkeleton(NULL);

		pEffectThread->SetTargetPos(m_v3TargetPosition);
		pEffectThread->SetTargetAxis(CAxis(m_v3TargetAngles));
	}
}
