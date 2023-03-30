// (C)2006 S2 Games
// c_consumableStaminaMinor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_consumablestaminaminor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Consumable, StaminaMinor);

CCvarf  CConsumableStaminaMinor::s_cvarStaminaAmount(_T("Consumable_StaminaMinor_StaminaAmount"),   50.0f,  CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CConsumableStaminaMinor::ActivatePrimary
  ====================*/
bool    CConsumableStaminaMinor::ActivatePrimary(int iButtonStatus)
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
