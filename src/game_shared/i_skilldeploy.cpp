// (C)2006 S2 Games
// i_skilldeploy.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_skilldeploy.h"
//=============================================================================

/*====================
  ISkillDeploy::CEntityConfig::CEntityConfig
  ====================*/
ISkillDeploy::CEntityConfig::CEntityConfig(const tstring &sName) :
ISkillItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(GadgetName, _T("")),
INIT_ENTITY_CVAR(Offset, V_ZERO)
{
}


/*====================
  ISkillDeploy::Impact
  ====================*/
void	ISkillDeploy::Impact()
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return;

	if (Game.IsClient())
		return;

	IGameEntity *pNewEnt(Game.AllocateEntity(GetGadgetName()));
	if (pNewEnt == NULL)
	{
		Console.Warn << _T("Failed to spawn gadget: ") << GetGadgetName() << newl;
		return;
	}

	CVec3f v3Angles(pOwner->GetAngles());
	v3Angles[PITCH] = 0.0f;
	CAxis axis(v3Angles);
	CVec3f v3Offset(GetOffset());
	v3Offset[Z] = 0.0f;
	v3Offset = TransformPoint(v3Offset, axis, pOwner->GetPosition());
	v3Offset[Z] = Game.GetTerrainHeight(v3Offset[X], v3Offset[Y]) + GetOffset().z;

	if (pNewEnt->IsGadget())
	{
		pNewEnt->GetAsGadget()->SetOwner(pOwner->GetIndex());
		IPlayerEntity *pPlayer(pOwner->GetAsPlayerEnt());
		if (pPlayer != NULL)
			pNewEnt->GetAsGadget()->SetOwnerClientNumber(pPlayer->GetClientID());
	}

	if (pNewEnt->IsVisual())
	{
		pNewEnt->GetAsVisualEnt()->SetTeam(pOwner->GetTeam());
		pNewEnt->GetAsVisualEnt()->SetPosition(v3Offset);
		pNewEnt->GetAsVisualEnt()->SetAngles(0.0f, 0.0f, v3Angles[YAW]);
	}

	pNewEnt->Spawn();
}


/*====================
  ISkillDeploy::ClientPrecache
  ====================*/
void	ISkillDeploy::ClientPrecache(CEntityConfig *pConfig)
{
	IInventoryItem::ClientPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetGadgetName().empty())
		EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));
}


/*====================
  ISkillDeploy::ServerPrecache
  ====================*/
void	ISkillDeploy::ServerPrecache(CEntityConfig *pConfig)
{
	IInventoryItem::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetGadgetName().empty())
		EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetGadgetName()));
}
