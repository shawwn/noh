// (C)2006 S2 Games
// c_consumableStaminaMajor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_consumablestaminamajor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Consumable, StaminaMajor);

CCvarf  CConsumableStaminaMajor::s_cvarStaminaAmount(_T("Consumable_StaminaMajor_StaminaAmount"),   100.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CConsumableStaminaMajor::ActivatePrimary
  ====================*/
bool    CConsumableStaminaMajor::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (IsReady() && pOwner->GetAmmoCount(m_ySlot) > 0)
    {
        pOwner->SetStamina(MIN(pOwner->GetMaxStamina(), pOwner->GetStamina() + s_cvarStaminaAmount.GetValue()));

        SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
        pOwner->UseItem(m_ySlot, 1);
        return true;
    }

    return false;
}
