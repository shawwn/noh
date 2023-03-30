// (C)2006 S2 Games
// c_spellconsumecorpse.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellconsumecorpse.h"
#include "c_stateconsumecorpse.h"
#include "c_gunrottenflesh.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, ConsumeCorpse);
//=============================================================================

/*====================
  CSpellConsumeCorpse::CEntityConfig::CEntityConfig
  ====================*/
CSpellConsumeCorpse::CEntityConfig::CEntityConfig(const tstring &sName) :
ISpellItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRestored, 0.0f)
{
}

/*====================
  CSpellConsumeCorpse::~CSpellConsumeCorpse()
  ====================*/
CSpellConsumeCorpse::~CSpellConsumeCorpse()
{
	ICombatEntity *pOwner(GetOwnerEnt());

	if (pOwner != NULL && m_uiStateSlot != -1 && pOwner->GetState(m_uiStateSlot) != NULL && pOwner->GetState(m_uiStateSlot)->GetType() == State_ConsumeCorpse)
		pOwner->RemoveState(m_uiStateSlot);
}

/*====================
  CSpellConsumeCorpse::CSpellConsumeCorpse
  ====================*/
CSpellConsumeCorpse::CSpellConsumeCorpse() :
ISpellItem(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_uiStateSlot(-1)
{
	ICombatEntity *pOwner(GetOwnerEnt());

	if (pOwner != NULL)
		m_uiStateSlot = pOwner->ApplyState(State_ConsumeCorpse, Game.GetGameTime(), INVALID_TIME);
}

/*====================
  CSpellConsumeCorpse::ImpactEntity
  ====================*/
bool	CSpellConsumeCorpse::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
	if (!ISpellItem::ImpactEntity(uiTargetIndex, evImpact, bCheckTarget))
		return false;

	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));

	if (pTarget == NULL || pTarget->HasNetFlags(ENT_NET_FLAG_NO_CORPSE))
		return false;

	pTarget->SetNetFlags(ENT_NET_FLAG_NO_CORPSE);

	ICombatEntity *pOwner(GetOwnerEnt());

	if (pOwner == NULL)
		return true;

	IEntityState *pState(NULL);

	if (m_uiStateSlot != -1)
		pState = pOwner->GetState(m_uiStateSlot);

	if (pState == NULL || pState->GetType() != State_ConsumeCorpse)
	{
		m_uiStateSlot = pOwner->ApplyState(State_ConsumeCorpse, Game.GetGameTime(), INVALID_TIME);
		pState = pOwner->GetState(m_uiStateSlot);
	}

	if (pState != NULL && pState->GetType() == State_ConsumeCorpse)
	{
		CStateConsumeCorpse *pConsume(static_cast<CStateConsumeCorpse*>(pState));
		pConsume->ConsumeCorpse();
	}
	
	for (int i(0); i < INVENTORY_START_BACKPACK; i++)
	{
		IInventoryItem *pItem(pOwner->GetItem(i));
			
		if (pItem == NULL)
			continue;

		if (pItem->GetType() != Gun_RottenFlesh)
			continue;

		CGunRottenFlesh *pGun(static_cast<CGunRottenFlesh*>(pItem));
		pGun->ConsumeCorpse();
	}

	pOwner->Heal(m_pEntityConfig->GetHealthRestored(), pOwner);

	return true;
}
