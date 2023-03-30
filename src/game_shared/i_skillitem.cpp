// (C)2006 S2 Games
// i_skillitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_skillitem.h"
#include "i_skilltoggle.h"

#include "../k2/c_skeleton.h"
#include "../k2/c_clientsnapshot.h"
//=============================================================================

/*====================
  ISkillItem::CEntityConfig::CEntityConfig
  ====================*/
ISkillItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(AnimName, _T("")),
INIT_ENTITY_CVAR(CanUseWithMelee, true),
INIT_ENTITY_CVAR(CanUseWithRanged, false),
INIT_ENTITY_CVAR(Duration, 2000),
INIT_ENTITY_CVAR(ActivationTime, 0),
INIT_ENTITY_CVAR(Freeze, true)
{
}

const ISkillToggle*		ISkillItem::GetAsSkillToggle() const	{ if (!IsToggleSkill()) return NULL; else return static_cast<const ISkillToggle*>(this); }
ISkillToggle*			ISkillItem::GetAsSkillToggle()			{ if (!IsToggleSkill()) return NULL; else return static_cast<ISkillToggle*>(this); }
/*====================
  ISkillItem::ActivatePrimary
  ====================*/
bool	ISkillItem::ActivatePrimary(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return true;

	if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
		return false;

	// Check cooldown timer
	if (IsDisabled() || !IsReady())
		return false;
	
	// Check what mode the player is in
	IInventoryItem *pItem(pOwner->GetCurrentItem());
	if (pItem != NULL)
	{
		if ((pItem->IsMelee() && !GetCanUseWithMelee()) ||
			(pItem->IsGun() && !GetCanUseWithRanged()))
			return false;
	}

	// Check to make sure the owner is idle
	if (!pOwner->IsIdle())
		return false;

	// Check mana cost
	if (!pOwner->SpendMana(GetManaCost()))
		return false;

	SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());

	// Animation
	if (!GetAnimName().empty())
		pOwner->StartAnimation(GetAnimName(), 1);

	// Create an event for the player activating this
	CSkillActivateEvent &activate(pOwner->GetSkillActivateEvent());
	activate.Clear();
	activate.SetOwner(pOwner);
	activate.SetSlot(m_ySlot);
	activate.SetActivateTime(Game.GetGameTime() + GetActivationTime());

	// Set the player's new action
	int iAction(PLAYER_ACTION_SKILL);
	if (GetFreeze())
	{
		pOwner->StopAnimation(0);
		iAction |= PLAYER_ACTION_IMMOBILE;
	}
	
	pOwner->SetAction(iAction, Game.GetGameTime() + GetDuration());
//	Game.SelectItem(pOwner->GetDefaultInventorySlot());
	return true;
}
