// (C)2007 S2 Games
// c_buildingsublair.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_buildingsublair.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Building, SubLair)
//=============================================================================

/*====================
  CBuildingSubLair::Use
  ====================*/
void    CBuildingSubLair::Use(IGameEntity *pActivator)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
    {
        IBuildingEntity::Use(pActivator);
        return;
    }

    if (Game.IsServer())
    {
        IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
        if (pPlayer != NULL && pPlayer->GetTeam() == m_iTeam && pPlayer->GetCanEnterLoadout())
            pPlayer->EnterLoadout();
    }
}
