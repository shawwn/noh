// (C)2007 S2 Games
// i_spellrevive.h
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spellrevive.h"
//=============================================================================

/*====================
  ISpellRevive::CEntityConfig::CEntityConfig
  ====================*/
ISpellRevive::CEntityConfig::CEntityConfig(const tstring &sName) :
ISpellItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(TargetFreezeTime, 2000),
INIT_ENTITY_CVAR(TargetHealthPercent, 0.5f),
INIT_ENTITY_CVAR(TargetStaminaPercent, 0.5f),
INIT_ENTITY_CVAR(TargetManaPercent, 0.4f)
{
}


/*====================
  ISpellRevive::IsValidTarget
  ====================*/
bool	ISpellRevive::IsValidTarget(IGameEntity *pEntity, bool bImpact)
{
	if (!ISpellItem::IsValidTarget(pEntity, bImpact))
		return false;

	return !pEntity->HasNetFlags(ENT_NET_FLAG_NO_RESURRECT);
}


/*====================
  ISpellRevive::ImpactEntity
  ====================*/
bool	ISpellRevive::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	if (pTarget == NULL)
		return false;
	if (bCheckTarget && !IsValidTarget(pTarget, true))
		return false;

	pTarget->SetStatus(ENTITY_STATUS_ACTIVE);
	pTarget->ApplyState(EntityRegistry.LookupID(m_pEntityConfig->GetTargetState()), Game.GetGameTime(), m_pEntityConfig->GetTargetStateDuration(), (pOwner != NULL ? pOwner->GetIndex() : INVALID_INDEX));
	pTarget->StartAnimation(_T("resurrected"), -1);
	pTarget->GetAsPlayerEnt()->SetAction(PLAYER_ACTION_IMMOBILE, Game.GetGameTime() + m_pEntityConfig->GetTargetFreezeTime());

	if (pTarget->IsPlayer())
	{
		pTarget->GetAsPlayerEnt()->SetMana(MIN(pTarget->GetAsPlayerEnt()->GetMaxMana() * m_pEntityConfig->GetTargetManaPercent(), pTarget->GetAsPlayerEnt()->GetMaxMana()));
		pTarget->GetAsPlayerEnt()->SetStamina(MIN(pTarget->GetAsPlayerEnt()->GetMaxStamina() * m_pEntityConfig->GetTargetStaminaPercent(), pTarget->GetAsPlayerEnt()->GetMaxStamina()));

		pTarget->ClearStates();

		// Apply passive states from inventory
		for (int i = 0; i < MAX_INVENTORY; ++i)
		{
			IInventoryItem *pItem = pTarget->GetAsPlayerEnt()->GetItem(i);
			
			if (pItem != NULL)
				pItem->ActivatePassive();
		}
	}

	pTarget->SetHealth(MIN(pTarget->GetMaxHealth() * m_pEntityConfig->GetTargetHealthPercent(), pTarget->GetMaxHealth()));

	if (pOwner != NULL)
	{
		int iClientID(-1);
		if (pTarget->IsPlayer())
			iClientID = pTarget->GetAsPlayerEnt()->GetClientID();

		if (pOwner->IsPlayer())
			Game.MatchStatEvent(pOwner->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_RESURRECTS, 1, iClientID, GetType(), INVALID_ENT_TYPE, Game.GetGameTime());

		pOwner->GiveExperience(m_pEntityConfig->GetCastExperience(), pTarget->GetPosition());
	}

	evImpact.SetSourcePosition(pTarget->GetPosition());
	evImpact.SetSourceAngles(pTarget->GetAngles());

	return true;
}
