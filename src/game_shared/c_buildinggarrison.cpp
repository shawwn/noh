// (C)2006 S2 Games
// c_buildinggarrison.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_buildinggarrison.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Building, Garrison);
//=============================================================================

/*====================
  CBuildingGarrison::CBuildingGarrison
  ====================*/
CBuildingGarrison::CBuildingGarrison() :
IBuildingEntity(GetEntityConfig())
{
}


/*====================
  CBuildingGarrison::Use
  ====================*/
void	CBuildingGarrison::Use(IGameEntity *pActivator)
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
