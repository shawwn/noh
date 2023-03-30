// (C)2006 S2 Games
// c_propbasebuilding.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propbasebuilding.h"
#include "c_teaminfo.h"
#include "c_teamdefinition.h"

#include "../k2/c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, BaseBuilding);
//=============================================================================

/*====================
  CPropBaseBuilding::CPropBaseBuilding
  ====================*/
CPropBaseBuilding::CPropBaseBuilding() :
IPropEntity(GetEntityConfig()),
m_uiBaseUID(INVALID_INDEX)
{
}


/*====================
  CPropBaseBuilding::GameStart
  ====================*/
void	CPropBaseBuilding::GameStart()
{
	// Client will receive the proper entity from the server
	if (Game.IsClient())
		return;

	// Get the info for the team that this belongs to
	CEntityTeamInfo *pTeam(Game.GetTeam(m_iTeam));
	if (pTeam == NULL)
	{
		Console.Warn << _T("CPropBaseBuilding::Spawn() - Team does not exist: ") << int(m_iTeam) << newl;
		return;
	}

	// Create a new building of the appropriate type
	tstring sBuildingName(pTeam->GetDefinition()->GetBaseBuildingName());
	IGameEntity *pNewEnt(Game.AllocateEntity(sBuildingName));
	if (pNewEnt == NULL)
	{
		Console.Warn << _T("CPropBaseBuilding::Spawn() - Failed to allocate entity: ") << sBuildingName << newl;
		return;
	}
	
	if (pNewEnt->IsVisual())
	{
		IVisualEntity *pVisEnt(pNewEnt->GetAsVisualEnt());
		
		pVisEnt->SetPosition(GetPosition());
		pVisEnt->SetAngles(GetAngles());
		pVisEnt->SetTeam(GetTeam());
	}

	pNewEnt->Spawn();

	Console << _T("Adding BaseBuilding #") << m_uiWorldIndex << _T(" as entity #") << m_uiIndex << _T(" to team ") << int(m_iTeam) << newl;
	pTeam->SetBaseBuildingIndex(pNewEnt->GetIndex());

	m_uiBaseUID = pNewEnt->GetUniqueID();
}


/*====================
  CPropBaseBuilding::WarmupStart
  ====================*/
void	CPropBaseBuilding::WarmupStart()
{
	// Client will receive the proper entity from the server
	if (Game.IsClient())
		return;

	// Get the info for the team that this belongs to
	CEntityTeamInfo *pTeam(Game.GetTeam(m_iTeam));
	if (pTeam == NULL)
	{
		Console.Warn << _T("CPropBaseBuilding::Spawn() - Team does not exist: ") << int(m_iTeam) << newl;
		return;
	}

	// Create a new building of the appropriate type
	tstring sBuildingName(pTeam->GetDefinition()->GetBaseBuildingName());
	IGameEntity *pNewEnt(Game.AllocateEntity(sBuildingName));
	if (pNewEnt == NULL)
	{
		Console.Warn << _T("CPropBaseBuilding::Spawn() - Failed to allocate entity: ") << sBuildingName << newl;
		return;
	}
	
	if (pNewEnt->IsVisual())
	{
		IVisualEntity *pVisEnt(pNewEnt->GetAsVisualEnt());
		
		pVisEnt->SetPosition(GetPosition());
		pVisEnt->SetAngles(GetAngles());
		pVisEnt->SetTeam(GetTeam());

		pVisEnt->SetInvulnerable(true);
	}

	pNewEnt->Spawn();

	Console << _T("Adding BaseBuilding #") << m_uiWorldIndex << _T(" as entity #") << m_uiIndex << _T(" to team ") << int(m_iTeam) << newl;
	pTeam->SetBaseBuildingIndex(pNewEnt->GetIndex());

	m_uiBaseUID = pNewEnt->GetUniqueID();
}
