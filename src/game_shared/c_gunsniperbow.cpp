// (C)2006 S2 Games
// c_gunsniperbow.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gunsniperbow.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gun, SniperBow);

CVAR_FLOATF(Gun_SniperBow_ZoomFov, 20.0f, CVAR_TRANSMIT | CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CGunSniperBow::ActivateSecondary
  ====================*/
bool    CGunSniperBow::ActivateSecondary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    if (pOwner->GetFov() != Gun_SniperBow_ZoomFov)
        pOwner->SetFov(Gun_SniperBow_ZoomFov);
    else
        pOwner->SetFov(90.0f);

    return true;
}
