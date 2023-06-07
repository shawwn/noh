// (C)2006 S2 Games
// c_consumableManaMajor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_consumablemanamajor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Consumable, ManaMajor);

CCvarf  CConsumableManaMajor::s_cvarManaAmount(_T("Consumable_ManaMajor_ManaAmount"),   150.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CConsumableManaMajor::ActivatePrimary
  ====================*/
bool    CConsumableManaMajor::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (IsReady() && pOwner->GetAmmoCount(m_ySlot) > 0)
    {
        pOwner->SetMana(MIN(pOwner->GetMaxMana(), pOwner->GetMana() + s_cvarManaAmount.GetValue()));

        SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
        pOwner->UseItem(m_ySlot, 1);
        return true;
    }

    return false;
}
