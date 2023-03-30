// (C)2006 S2 Games
// c_consumableHealthMajor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_consumablehealthmajor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Consumable, HealthMajor);

CCvarf	CConsumableHealthMajor::s_cvarHealthAmount(_T("Consumable_HealthMajor_HealthAmount"),	350.0f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CConsumableHealthMajor::ActivatePrimary
  ====================*/
bool	CConsumableHealthMajor::ActivatePrimary(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	if (IsReady() && pOwner->GetAmmoCount(m_ySlot) > 0)
	{
		pOwner->SetHealth(MIN(pOwner->GetMaxHealth(), pOwner->GetHealth() + s_cvarHealthAmount.GetValue()));

		SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
		pOwner->UseItem(m_ySlot, 1);
		return true;
	}

	return false;
}
