// (C)2006 S2 Games
// c_gadgetammodepot.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetammodepot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, AmmoDepot);
//=============================================================================

/*====================
  CGadgetAmmoDepot::UseEffect
  ====================*/
bool    CGadgetAmmoDepot::UseEffect(IGameEntity *pActivator)
{
    IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
    if (pPlayer == NULL)
        return false;

    return pPlayer->RefillAmmo();
}
