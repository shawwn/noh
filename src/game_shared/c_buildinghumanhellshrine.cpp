// (C)2006 S2 Games
// c_buildinghumanhellshrine.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_buildinghumanhellshrine.h"
#include "c_teaminfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Building, HumanHellShrine);
//=============================================================================

/*====================
  CBuildingHumanHellShrine::CBuildingHumanHellShrine
  ====================*/
CBuildingHumanHellShrine::CBuildingHumanHellShrine() :
IBuildingEntity(GetEntityConfig())
{
}

/*====================
  CBuildingHumanHellShrine::Spawn
  ====================*/
void	CBuildingHumanHellShrine::Spawn()
{
	IBuildingEntity::Spawn();

	if (Game.IsServer())
	{
		CBufferFixed<2> buffer;
		buffer << GAME_CMD_HELLSHRINE_BUILDING;
		Game.BroadcastGameData(buffer, true);
		Game.GetTeam(GetTeam())->HellShrineConstructionStarted();
	}
}


/*====================
  CBuildingHumanHellShrine::Kill
  ====================*/
void	CBuildingHumanHellShrine::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
	IBuildingEntity::Kill(pAttacker, unKillingObjectID);

	if (Game.IsServer())
	{
		Game.GetTeam(GetTeam())->HellShrineDestroyed();
	}
}


/*====================
  CBuildingHumanHellShrine::Use
  ====================*/
void	CBuildingHumanHellShrine::Use(IGameEntity *pActivator)
{
	if (GetStatus() == ENTITY_STATUS_ACTIVE &&
		pActivator->IsPlayer() &&
		pActivator->GetAsPlayerEnt()->GetTeam() == m_iTeam &&
		!pActivator->GetAsPlayerEnt()->GetIsHellbourne())
	{
		pActivator->GetAsPlayerEnt()->SetNetFlags(ENT_NET_FLAG_SACRIFICE_MENU);
	}
}

