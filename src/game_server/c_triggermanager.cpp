// (C)2007 S2 Games
// c_triggermanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"
#include "c_triggermanager.h"

#include "../k2/c_function.h"
#include "../k2/c_script.h"
#include "../k2/c_npcdefinition.h"

#include "../game_shared/c_entityclientinfo.h"
#include "../game_shared/c_playercommander.h"
#include "../game_shared/i_visualentity.h"
#include "../game_shared/c_entitysoul.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CTriggerManager&	TriggerManager(*CTriggerManager::GetInstance());
SINGLETON_INIT(CTriggerManager)
//=============================================================================

/*====================
  CTriggerManager::CTriggerManager
  ====================*/
CTriggerManager::CTriggerManager()
{
}

/*====================
  CTriggerManager::RegisterTriggerParam
  ====================*/
void	CTriggerManager::RegisterTriggerParam(const tstring &sName, const tstring &sValue)
{
	m_mapTriggerParams[sName] = sValue;
}

/*====================
  CTriggerManager::RegisterEntityScript
  ====================*/
void	CTriggerManager::RegisterEntityScript(uint uiIndex, const tstring &sName, const tstring &sScript)
{
	map<uint, smaps>::iterator findit(m_mapEntityScripts.find(uiIndex));

	if (findit != m_mapEntityScripts.end())
		findit->second[sName] = sScript;
	else
	{
		smaps mapScript;
		mapScript[sName] = sScript;

		m_mapEntityScripts[uiIndex] = mapScript;
	}
}


/*====================
  CTriggerManager::CopyEntityScripts
  ====================*/
void	CTriggerManager::CopyEntityScripts(uint uiFromIndex, uint uiToIndex)
{
	ClearEntityScripts(uiToIndex);

	map<uint, smaps>::iterator findit(m_mapEntityScripts.find(uiFromIndex));

	if (findit != m_mapEntityScripts.end())
		m_mapEntityScripts[uiToIndex] = findit->second;
}


/*====================
  CTriggerManager::TriggerEntityScript
  ====================*/
bool	CTriggerManager::TriggerEntityScript(uint uiIndex, const tstring &sName)
{
	IGame *pGame(Game.GetCurrentGamePointer());
	Game.SetCurrentGamePointer(CGameServer::GetInstance());

	map<uint, smaps>::iterator findit(m_mapEntityScripts.find(uiIndex));

	if (findit != m_mapEntityScripts.end())
	{
		smaps::iterator it(findit->second.find(sName));

		if (it != findit->second.end() && !it->second.empty())
		{
			bool bWasStoring(ICvar::StoreCvars());
			ICvar::StoreCvars(false);

			Console.ExecuteScript(it->second, false, &m_mapTriggerParams);

			ICvar::StoreCvars(bWasStoring);
		}
	}
	
	// Clear the parameters used for this script
	m_mapTriggerParams.clear();

	Game.SetCurrentGamePointer(pGame);

	return true;
}

/*====================
  CTriggerManager::TriggerGlobalScript
  ====================*/
bool	CTriggerManager::TriggerGlobalScript(const tstring &sName)
{
	IGame *pGame(Game.GetCurrentGamePointer());
	Game.SetCurrentGamePointer(CGameServer::GetInstance());

	smaps::iterator findit(m_mapGlobalScripts.find(sName));

	if (findit != m_mapGlobalScripts.end() && !findit->second.empty())
	{
		bool bWasStoring(ICvar::StoreCvars());
		ICvar::StoreCvars(false);

		Console.ExecuteScript(findit->second, false, &m_mapTriggerParams);

		ICvar::StoreCvars(bWasStoring);
	}

	// Clear the parameters used for this script
	m_mapTriggerParams.clear();

	Game.SetCurrentGamePointer(pGame);

	return true;
}

//=============================================================================

/*--------------------
  GetPosX
  --------------------*/
TRIGGER_FCN(GetPosX, Visual, 1)
{
	return XtoA(pEnt->GetPosition()[X]);
}

/*--------------------
  GetPosY
  --------------------*/
TRIGGER_FCN(GetPosY, Visual, 1)
{
	return XtoA(pEnt->GetPosition()[Y]);
}

/*--------------------
  GetPosZ
  --------------------*/
TRIGGER_FCN(GetPosZ, Visual, 1)
{
	return XtoA(pEnt->GetPosition()[Z]);
}

/*--------------------
  GetPitch
  --------------------*/
TRIGGER_FCN(GetPitch, Visual, 1)
{
	return XtoA(pEnt->GetAngles()[PITCH]);
}

/*--------------------
  GetRoll
  --------------------*/
TRIGGER_FCN(GetRoll, Visual, 1)
{
	return XtoA(pEnt->GetAngles()[ROLL]);
}

/*--------------------
  GetYaw
  --------------------*/
TRIGGER_FCN(GetYaw, Visual, 1)
{
	return XtoA(pEnt->GetAngles()[YAW]);
}

/*--------------------
  GetClientNumFromIndex
  --------------------*/
TRIGGER_FCN(GetClientNumFromIndex, Player, 1)
{
	return XtoA(pEnt->GetClientID());
}

/*--------------------
  GetHealth
  --------------------*/
TRIGGER_FCN(GetHealth, Visual, 1)
{
	return XtoA(pEnt->GetHealth());
}

/*--------------------
  GetMaxHealth
  --------------------*/
TRIGGER_FCN(GetMaxHealth, Visual, 1)
{
	return XtoA(pEnt->GetMaxHealth());
}

/*--------------------
  GetMana
  --------------------*/
TRIGGER_FCN(GetMana, Combat, 1)
{
	return XtoA(pEnt->GetMana());
}

/*--------------------
  GetMaxMana
  --------------------*/
TRIGGER_FCN(GetMaxMana, Combat, 1)
{
	return XtoA(pEnt->GetMaxMana());
}

/*--------------------
  GetStamina
  --------------------*/
TRIGGER_FCN(GetStamina, Combat, 1)
{
	return XtoA(pEnt->GetStamina());
}

/*--------------------
  GetMaxStamina
  --------------------*/
TRIGGER_FCN(GetMaxStamina, Combat, 1)
{
	return XtoA(pEnt->GetMaxStamina());
}

/*--------------------
  IsStunned
  --------------------*/
TRIGGER_FCN(IsStunned, Combat, 1)
{
	return XtoA((pEnt->GetAction() & PLAYER_ACTION_STUNNED) != 0, true);
}

/*--------------------
  IsInvulnerable
  --------------------*/
TRIGGER_FCN(IsInvulnerable, Visual, 1)
{
	return XtoA(pEnt->IsInvulnerable(), true);
}

/*--------------------
  GetTerrainHeight
  --------------------*/
SERVER_FCN(GetTerrainHeight)
{
	if (vArgList.size() < 2)
		return _T("0");

	return XtoA(GameServer.GetTerrainHeight(AtoF(vArgList[0]), AtoF(vArgList[1])));
}

/*--------------------
  IsValidPosition
  --------------------*/
TRIGGER_FCN(IsValidPosition, Visual, 1)
{
	CVec3f v3Start(pEnt->GetPosition());
	STraceInfo trace;
	uiset setIgnore;

	if (v3Start[Z] < GameServer.GetTerrainHeight(v3Start[X], v3Start[Y]))
		return _T("0");

	for (svector_cit it(vArgList.begin() + 1); it != vArgList.end(); it++)
	{
		IVisualEntity *pVis(GameServer.GetVisualEntity(AtoI(*it)));

		if (pVis != NULL)
		{
			CWorldEntity *pWorld(Game.GetWorldEntity(pVis->GetWorldIndex()));
			
			if (pWorld != NULL)
			{
				setIgnore.insert(pVis->GetWorldIndex());
				pWorld->SetSurfFlags(pWorld->GetSurfFlags() | SURF_IGNORE);
			}
		}
	}

	Game.TraceBox(trace, v3Start, v3Start, pEnt->GetBounds(), TRACE_PLAYER_MOVEMENT | SURF_IGNORE, pEnt->GetWorldIndex());

	for (uiset::iterator it(setIgnore.begin()); it != setIgnore.end(); it++)
	{
		CWorldEntity *pWorld(Game.GetWorldEntity(*it));
		
		if (pWorld != NULL)
			pWorld->SetSurfFlags(pWorld->GetSurfFlags() & ~SURF_IGNORE);
	}

	return XtoA(trace.bStartedInSurface, true);
}

/*--------------------
  GetIndexFromClientNum
  --------------------*/
SERVER_FCN(GetIndexFromClientNum)
{
	if (vArgList.size() < 1)
		return _T("");

	uint uiNum(AtoI(vArgList[0]));

	IPlayerEntity *pEnt(GameServer.GetPlayerEntityFromClientID(uiNum));

	if (pEnt == NULL)
		return _T("");

	return XtoA(pEnt->GetIndex());
}

/*--------------------
  EntityExists
  --------------------*/
SERVER_FCN(EntityExists)
{
	if (vArgList.size() < 1)
		return _T("0");

	uint uiIndex(AtoI(vArgList[0]));

	return XtoA(GameServer.GetEntity(uiIndex) != NULL, true);
}

/*--------------------
  GetGameTime
  --------------------*/
SERVER_FCN(GetGameTime)
{
	return XtoA(GameServer.GetGameTime());
}

/*--------------------
  GetFrameLength
  --------------------*/
SERVER_FCN(GetFrameLength)
{
	return XtoA(GameServer.GetFrameLength());
}

/*--------------------
  GetIndexFromName
  --------------------*/
SERVER_FCN(GetIndexFromName)
{
	if (vArgList.size() < 1)
		return _T("");

	tstring sName(ConcatinateArgs(vArgList));

	WorldEntMap map(GameServer.GetWorldEntityMap());

	for (WorldEntMap_it it(map.begin()); it != map.end(); it++)
	{
		CWorldEntity *pWorldEntity(GameServer.GetWorldEntity(it->first));
		if (!pWorldEntity)
			EX_ERROR(_T("Failed world entity lookup on #") + XtoA(it->first));

		if (pWorldEntity->GetName() == sName)
			return XtoA(pWorldEntity->GetGameIndex());
	}

	return _T("");
}


/*--------------------
  GetClientNameFromClientNum
  --------------------*/
SERVER_FCN(GetClientNameFromClientNum)
{
	if (vArgList.size() < 1)
		return _T("");

	int iClientNum(AtoI(vArgList[0]));

	CEntityClientInfo *pClient(Game.GetClientInfo(iClientNum));

	if (pClient == NULL)
		return _T("");

	return pClient->GetName();
}

/*--------------------
  GetNameFromIndex
  --------------------*/
TRIGGER_FCN(GetNameFromIndex, Visual, 1)
{
	CWorldEntity *pWorld(GameServer.GetWorldEntity(pEnt->GetWorldIndex()));

	if (pWorld == NULL)
		return _T("");

	return pWorld->GetName();
}

/*--------------------
  IsGunItem
  --------------------*/
TRIGGER_FCN(IsGunItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsGun(), true);
}

/*--------------------
  IsSpellItem
  --------------------*/
TRIGGER_FCN(IsSpellItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsSpell(), true);
}

/*--------------------
  IsSkillItem
  --------------------*/
TRIGGER_FCN(IsSkillItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsSkill(), true);
}

/*--------------------
  IsMeleeItem
  --------------------*/
TRIGGER_FCN(IsMeleeItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsMelee(), true);
}

/*--------------------
  IsConsumableItem
  --------------------*/
TRIGGER_FCN(IsConsumableItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsConsumable(), true);
}

/*--------------------
  IsPersistantItem
  --------------------*/
TRIGGER_FCN(IsPersistantItem, Combat, 2)
{
	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem == NULL)
		return _T("0");

	return XtoA(pItem->IsPersistant(), true);
}

/*--------------------
  GetMaxItems
  --------------------*/
SERVER_FCN(GetMaxItems)
{
	return XtoA(INVENTORY_END_BACKPACK);
}

/*--------------------
  HasState
  --------------------*/
TRIGGER_FCN(HasState, Visual, 2)
{
	ushort unID(g_EntityRegistry.LookupID(vArgList[1]));

	if (unID == INVALID_ENT_TYPE)
		return _T("0");

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; i++)
		if (pEnt->GetState(i) != NULL && pEnt->GetState(i)->GetType() == unID)
			return _T("1");

	return _T("0");
}


/*--------------------
  IsEntityActive
  --------------------*/
TRIGGER_FCN(IsEntityActive, Visual, 1)
{
	return XtoA(pEnt->GetStatus() == ENTITY_STATUS_ACTIVE, true);
}

/*--------------------
  GetNpcProperty
  --------------------*/
TRIGGER_FCN(GetNpcProperty, Npc, 2)
{
	ResHandle hDefinition(pEnt->GetDefinition());
	CNpcDefinition *pDefinition(g_ResourceManager.GetNpcDefiniton(hDefinition));

	if (pDefinition == NULL)
		return _T("");

	return pDefinition->GetProperty(vArgList[1]);
}

/*--------------------
  SetNpcProperty
  --------------------*/
TRIGGER_CMD(SetNpcProperty, Npc, 3)
{
	ResHandle hDefinition(pEnt->GetDefinition());
	CNpcDefinition *pDefinition(g_ResourceManager.GetNpcDefiniton(hDefinition));

	if (pDefinition == NULL)
		return false;

	pDefinition->SetProperty(vArgList[1], vArgList[2]);
	return true;
}

/*--------------------
  IsDashing
  --------------------*/
TRIGGER_FCN(IsDashing, Combat, 1)
{
	return XtoA((pEnt->GetMoveFlags() & PLAYER_MOVE_DASH) != 0, true);
}


/*--------------------
  ForceSpawn
  --------------------*/
TRIGGER_CMD(ForceSpawn, Player, 1)
{
	pEnt->Spawn3();

	return true;
}


/*--------------------
  ResetAttributes
  --------------------*/
TRIGGER_CMD(ResetAttributes, Player, 1)
{
	CEntityClientInfo *pClient(Game.GetClientInfo(pEnt->GetClientID()));

	if (pClient == NULL)
		return true;

	pClient->ResetAttributes();

	return true;
}


/*--------------------
  GiveExperience
  --------------------*/
TRIGGER_CMD(GiveExperience, Player, 2)
{
	CEntityClientInfo *pClient(Game.GetClientInfo(pEnt->GetClientID()));

	if (pClient == NULL)
		return true;

	pClient->GiveExperience(AtoF(vArgList[1]));
	pClient->SetInitialExperience(pClient->GetInitialExperience() + AtoF(vArgList[1]));

	return true;
}

/*--------------------
  SetExperience
  --------------------*/
TRIGGER_CMD(SetExperience, Player, 2)
{
	CEntityClientInfo *pClient(Game.GetClientInfo(pEnt->GetClientID()));

	if (pClient == NULL)
		return true;

	pClient->SetInitialExperience(MAX(pClient->GetInitialExperience() + (AtoF(vArgList[1]) - pClient->GetExperience()), 0.0f));
	pClient->ResetExperience();
	pClient->GiveExperience(AtoF(vArgList[1]));
	

	return true;
}

/*--------------------
  ChangeUnit
  --------------------*/
TRIGGER_CMD(ChangeUnit, Player, 2)
{
	/*if (vArgList.size() < 9)
	{
		Console << _T("Syntax: ChangeUnit <index> <new> <spawn> <pos> <hp> <kill> <check rules> <damage record> <refund>") << newl;
		Console << _T("        INDEX: Index of unit to change") << newl;
		Console << _T("        NEW: Type of unit to change to") << newl;
		Console << _T("        SPAWN: True/false, determines if unit starts spawned (true) or in loadout (false)") << newl;
		Console << _T("        POS: True/false, determines if unit inherits the old unit's position") << newl;
		Console << _T("        HP: True/false, determines if unit inherits the old unit's HP %") << newl;
		Console << _T("        KILL: True/false, determines if old unit is killed (true) or simply removed (false)") << newl;
		Console << _T("        CHECK RULES: True/false, determines if they must be able to switch units before doing so") << newl;
		Console << _T("        DAMAGE RECORD: True/false, determines if unit inherits old unit's damage record") << newl;
		Console << _T("        REFUND: True/false, determines if player recieves a refund for their old unit") << newl;
		return false;
	}*/

	int iFlags(0);

	ushort unID(EntityRegistry.LookupID(vArgList[1]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("ChangeUnit: New unit type not found.") << newl;
		return true;
	}

	if (vArgList.size() >= 3 && AtoB(vArgList[2]))
		iFlags |= CHANGE_UNIT_SPAWN;

	if (vArgList.size() >= 4 &&AtoB(vArgList[3]))
		iFlags |= CHANGE_UNIT_INHERIT_POS;

	if (vArgList.size() >= 5 &&AtoB(vArgList[4]))
		iFlags |= CHANGE_UNIT_INHERIT_HP;

	if (vArgList.size() >= 6 &&AtoB(vArgList[5]))
		iFlags |= CHANGE_UNIT_KILL;

	if (vArgList.size() >= 7 &&AtoB(vArgList[6]))
		iFlags |= CHANGE_UNIT_CHECK_RULES;

	if (vArgList.size() >= 8 &&AtoB(vArgList[7]))
		iFlags |= CHANGE_UNIT_INHERIT_DAMAGE_RECORD;

	if (vArgList.size() >= 9 &&AtoB(vArgList[8]))
		iFlags |= CHANGE_UNIT_REFUND_GOLD;

	IPlayerEntity *pPlayer(GameServer.ChangeUnit(pEnt->GetClientID(), unID, iFlags));
	if (pPlayer != NULL && g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("changedindex"), XtoA(pPlayer->GetIndex()));

	return true;
}


/*--------------------
  StartAnim
  --------------------*/
TRIGGER_CMD(StartAnim, Visual, 3)
{
	/*if (vArgList.size() < 3)
	
		Console << _T("Syntax: StartAnim <index> <channel> <animation name> [speed - optional] [length - optional]") << newl;
		return false;
	}*/

	if (vArgList.size() > 4)
		pEnt->StartAnimation(vArgList[2], AtoI(vArgList[1]), AtoF(vArgList[3]), AtoF(vArgList[4]));
	else if (vArgList.size() > 3)
		pEnt->StartAnimation(vArgList[2], AtoI(vArgList[1]), AtoF(vArgList[3]));
	else
		pEnt->StartAnimation(vArgList[2], AtoI(vArgList[1]));

	return true;
}


/*--------------------
  SetAngles
  --------------------*/
TRIGGER_CMD(SetAngles, Visual, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: SetAngles <index> <pitch> <roll> <yaw>") << newl;
		return false;
	}*/

	pEnt->SetAngles(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]));

	return true;
}


/*--------------------
  SetPosition
  --------------------*/
TRIGGER_CMD(SetPosition, Visual, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: SetPosition <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->SetPosition(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]));

	return true;
}


/*--------------------
  SetToValidPosition
  --------------------*/
TRIGGER_CMD(SetToValidPosition, Visual, 1)
{
	CVec3f v3Spawn(pEnt->GetPosition());
	CVec3f v3Start(v3Spawn);
	STraceInfo trace;
	uiset setIgnore;

	if (v3Start[Z] < GameServer.GetTerrainHeight(v3Start[X], v3Start[Y]))
		v3Start[Z] = GameServer.GetTerrainHeight(v3Start[X], v3Start[Y]);

	for (svector_cit it(vArgList.begin() + 1); it != vArgList.end(); it++)
	{
		IVisualEntity *pVis(GameServer.GetVisualEntity(AtoI(*it)));

		if (pVis != NULL)
		{
			CWorldEntity *pWorld(Game.GetWorldEntity(pVis->GetWorldIndex()));
			
			if (pWorld != NULL)
			{
				setIgnore.insert(pVis->GetWorldIndex());
				pWorld->SetSurfFlags(pWorld->GetSurfFlags() | SURF_IGNORE);
			}
		}
	}

	Game.TraceBox(trace, v3Start, v3Spawn, pEnt->GetBounds(), TRACE_PLAYER_MOVEMENT | SURF_IGNORE, pEnt->GetWorldIndex());

	while (trace.bStartedInSurface)
	{
		v3Start[Z] += 25.0f;
		Game.TraceBox(trace, v3Start, v3Spawn, pEnt->GetBounds(), TRACE_PLAYER_MOVEMENT | SURF_IGNORE, pEnt->GetWorldIndex());
	}

	pEnt->SetPosition(trace.v3EndPos);

	for (uiset::iterator it(setIgnore.begin()); it != setIgnore.end(); it++)
	{
		CWorldEntity *pWorld(Game.GetWorldEntity(*it));
		
		if (pWorld != NULL)
			pWorld->SetSurfFlags(pWorld->GetSurfFlags() & ~SURF_IGNORE);
	}

	return true;
}


/*--------------------
  SetTeam
  --------------------*/
TRIGGER_CMD(SetTeam, Visual, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetTeam <index> <team>") << newl;
		return false;
	}*/

	if (pEnt->IsPlayer())
		GameServer.ChangeTeam(pEnt->GetAsPlayerEnt()->GetClientID(), AtoI(vArgList[1]));
	else
		pEnt->SetTeam(AtoI(vArgList[1]));

	return true;
}

/*--------------------
  GetTeam
  --------------------*/
TRIGGER_FCN(GetTeam, Visual, 1)
{
	return XtoA(pEnt->GetTeam());
}

/*--------------------
  GetType
  --------------------*/
TRIGGER_FCN(GetType, Visual, 1)
{
	return pEnt->GetTypeName();
}

/*--------------------
  IsClientConnected
  --------------------*/
SERVER_FCN(IsClientConnected)
{
	if (vArgList.size() < 1)
		return _T("0");

	CEntityClientInfo *pClient(Game.GetClientInfo(AtoI(vArgList[0])));

	if (pClient == NULL)
		return _T("0");

	return XtoA(!pClient->IsDisconnected(), true);
}

/*--------------------
  SetInvulnerable
  --------------------*/
TRIGGER_CMD(SetInvulnerable, Visual, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetInvulnerability <index> <value>") << newl;
		return false;
	}*/

	pEnt->SetInvulnerable(AtoB(vArgList[1]));

	return true;
}


/*--------------------
  SetHealth
  --------------------*/
TRIGGER_CMD(SetHealth, Visual, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetHealth <index> <value>") << newl;
		return false;
	}*/

	pEnt->SetHealth(AtoF(vArgList[1]));

	return true;
}


/*--------------------
  SetMana
  --------------------*/
TRIGGER_CMD(SetMana, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetMana <index> <value>") << newl;
		return false;
	}*/

	pEnt->SetMana(AtoF(vArgList[1]));

	return true;
}


/*--------------------
  SetStamina
  --------------------*/
TRIGGER_CMD(SetStamina, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetStamina <index> <value>") << newl;
		return false;
	}*/

	pEnt->SetStamina(AtoF(vArgList[1]));

	return true;
}


/*--------------------
  RefillHealth
  --------------------*/
TRIGGER_CMD(RefillHealth, Visual, 1)
{
	pEnt->SetHealth(pEnt->GetMaxHealth());

	return true;
}


/*--------------------
  RefillMana
  --------------------*/
TRIGGER_CMD(RefillMana, Combat, 1)
{
	pEnt->SetMana(pEnt->GetMaxMana());

	return true;
}


/*--------------------
  RefillStamina
  --------------------*/
TRIGGER_CMD(RefillStamina, Combat, 1)
{
	pEnt->SetStamina(pEnt->GetMaxStamina());

	return true;
}


/*--------------------
  SetName
  --------------------*/
TRIGGER_CMD(SetName, Visual, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetName <index> <name>") << newl;
		return false;
	}*/

	CWorldEntity *pWorld(GameServer.GetWorldEntity(pEnt->GetWorldIndex()));

	if (pWorld == NULL)
		return true;

	pWorld->SetName(vArgList[1]);

	return true;
}


#if 0
/*--------------------
  AddNPCJobFollow
  --------------------*/
TRIGGER_CMD(AddNPCJobFollow, Npc, 2)
{
/*	if (vArgList.size() < 2)
	{
		Console << _T("Syntax: AddNPCJobFollow <index> <target index>") << newl;
		return false;
	}*/

	IVisualEntity *pTargetEnt(GameServer.GetVisualEntity(AtoI(vArgList[1])));

	if (pTargetEnt == NULL)
	{
		Console << _T("AddNPCJobFollow: Invalid target world index.") << newl;
		return true;
	}

	pEnt->AddJob(K2_NEW(global,   CAIJobFollow)(pEnt, pTargetEnt->GetIndex()));

	return true;
}


/*--------------------
  AddNPCJobMove
  --------------------*/
TRIGGER_CMD(AddNPCJobMove, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobMove <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->AddJob(K2_NEW(global,   CAIJobMove)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}


/*--------------------
  AddNPCJobGuard
  --------------------*/
TRIGGER_CMD(AddNPCJobGuard, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobGuard <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->AddJob(K2_NEW(global,   CAIJobGuardPos)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}


/*--------------------
  AddNPCJobAttackMove
  --------------------*/
TRIGGER_CMD(AddNPCJobAttackMove, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobAttackMove <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->AddJob(K2_NEW(global,   CAIJobAttackMove)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}

/*--------------------
  SetNPCJobFollow
  --------------------*/
TRIGGER_CMD(SetNPCJobFollow, Npc, 2)
{
/*	if (vArgList.size() < 2)
	{
		Console << _T("Syntax: AddNPCJobFollow <index> <target index>") << newl;
		return false;
	}*/

	IVisualEntity *pTargetEnt(GameServer.GetVisualEntity(AtoI(vArgList[1])));

	if (pTargetEnt == NULL)
	{
		Console << _T("AddNPCJobFollow: Invalid target world index.") << newl;
		return true;
	}

	pEnt->SetJob(K2_NEW(global,   CAIJobFollow)(pEnt, pTargetEnt->GetIndex()));

	return true;
}


/*--------------------
  SetNPCJobMove
  --------------------*/
TRIGGER_CMD(SetNPCJobMove, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobMove <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->SetJob(K2_NEW(global,   CAIJobMove)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}


/*--------------------
  SetNPCJobGuard
  --------------------*/
TRIGGER_CMD(SetNPCJobGuard, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobGuard <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->SetJob(K2_NEW(global,   CAIJobGuardPos)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}


/*--------------------
  SetNPCJobAttackMove
  --------------------*/
TRIGGER_CMD(SetNPCJobAttackMove, Npc, 4)
{
	/*if (vArgList.size() < 4)
	{
		Console << _T("Syntax: AddNPCJobAttackMove <index> <X> <Y> <Z>") << newl;
		return false;
	}*/

	pEnt->SetJob(K2_NEW(global,   CAIJobAttackMove)(pEnt, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]))));

	return true;
}
#endif

/*--------------------
  SetNpcJobPatrol
  --------------------*/
TRIGGER_CMD(SetNpcJobPatrol, Npc, 4)
{
	pEnt->PlayerCommand(NPCCMD_PATROL, INVALID_INDEX, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));

	return true;
}

/*--------------------
  SetNpcJobMove
  --------------------*/
TRIGGER_CMD(SetNpcJobMove, Npc, 4)
{
	pEnt->PlayerCommand(NPCCMD_MOVE, INVALID_INDEX, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));

	return true;
}

/*--------------------
  SetPetJobPatrol
  --------------------*/
TRIGGER_CMD(SetPetJobPatrol, Pet, 4)
{
	pEnt->PlayerCommand(PETCMD_PATROL, INVALID_INDEX, CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));

	return true;
}

/*--------------------
  AddNPCAggro
  --------------------*/
TRIGGER_CMD(AddNPCAggro, Npc, 3)
{
	/*if (vArgList.size() < 3)
	{
		Console << _T("Syntax: AddNPCAggro <index> <target index> <aggro amount>") << newl;
		return false;
	}*/

	IVisualEntity *pTargetEnt(GameServer.GetVisualEntity(AtoI(vArgList[1])));

	if (pTargetEnt == NULL)
	{
		Console << _T("AddNPCAggro: Invalid target world index.") << newl;
		return true;
	}

	pEnt->AddAggro(pTargetEnt->GetIndex(), AtoF(vArgList[2]), false);

	return true;
}


/*--------------------
  RemoveEntity
  --------------------*/
TRIGGER_CMD(RemoveEntity, Visual, 1)
{
	/*if (vArgList.size() < 1)
	{
		Console << _T("Syntax: RemoveEntity <index>") << newl;
		return false;
	}*/

	GameServer.DeleteEntity(pEnt);

	return true;
}

/*--------------------
  KillEntity
  --------------------*/
TRIGGER_CMD(KillEntity, Visual, 1)
{
	/*if (vArgList.size() < 1)
	{
		Console << _T("Syntax: KillEntity <index> [killing index - optional]") << newl;
		return false;
	}*/

	IVisualEntity *pKillingEnt(NULL);

	if (vArgList.size() > 1)
	{
		pKillingEnt = GameServer.GetVisualEntity(AtoI(vArgList[1]));

		if (pKillingEnt == NULL)
		{
			Console << _T("KillEntity: Invalid killing index.") << newl;
			return true;
		}
	}

	if (pEnt->GetStatus() != ENTITY_STATUS_CORPSE && pEnt->GetStatus() != ENTITY_STATUS_DEAD)
		pEnt->Kill(pKillingEnt);

	return true;
}

/*--------------------
  SetOrder
  --------------------*/
TRIGGER_CMD(SetOrder, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetOrder <index> <target index>") << newl;
		return false;
	}*/

	if (vArgList.size() == 2)
	{
		IVisualEntity *pTargetEnt(GameServer.GetVisualEntity(AtoI(vArgList[1])));
		ECommanderOrder eOrder;

		if (pTargetEnt == NULL)
		{
			Console << _T("SetOrder: Invalid target world index.") << newl;
			return true;
		}

		if (pEnt->IsEnemy(pTargetEnt))
			eOrder = CMDR_ORDER_ATTACK;
		else
			eOrder = CMDR_ORDER_MOVE;

		if (pEnt->IsPlayer())
		{
			IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

			pPlayer->ClearOrders();
			pPlayer->AddOrder(eOrder, AtoI(vArgList[1]), V3_ZERO);
		}
		else if (pEnt->IsNpc())
		{
			if (pEnt->IsEnemy(pTargetEnt))
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_ATTACK, AtoI(vArgList[1]), V3_ZERO);
			else if (pTargetEnt->IsBuilding() && (pTargetEnt->GetStatus() == ENTITY_STATUS_SPAWNING || pTargetEnt->GetHealthPercent() < 1.0f))
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_REPAIR, AtoI(vArgList[1]), V3_ZERO);
			else
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_FOLLOW, AtoI(vArgList[1]), V3_ZERO);
		}
		else if (pEnt->IsPet())
		{
			if (pEnt->IsEnemy(pTargetEnt))
				pEnt->GetAsPet()->PlayerCommand(PETCMD_ATTACK, AtoI(vArgList[1]), V3_ZERO);
			else if (pTargetEnt->IsBuilding() && (pTargetEnt->GetStatus() == ENTITY_STATUS_SPAWNING || pTargetEnt->GetHealthPercent() < 1.0f))
				pEnt->GetAsPet()->PlayerCommand(PETCMD_REPAIR, AtoI(vArgList[1]), V3_ZERO);
			else
				pEnt->GetAsPet()->PlayerCommand(PETCMD_FOLLOW, AtoI(vArgList[1]), V3_ZERO);
		}

		return true;
	}
	else if (vArgList.size() >= 4)
	{
		CVec3f v3Position(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]));

		if (pEnt->IsPlayer())
		{
			IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

			pPlayer->ClearOrders();
			pPlayer->AddOrder(CMDR_ORDER_MOVE, INVALID_INDEX, v3Position);
		}
		else if (pEnt->IsNpc())
		{
			INpcEntity *pNpc(pEnt->GetAsNpc());

			pNpc->PlayerCommand(NPCCMD_MOVE, INVALID_INDEX, v3Position);
		}
		else if (pEnt->IsPet())
		{
			IPetEntity *pPet(pEnt->GetAsPet());

			pPet->PlayerCommand(PETCMD_MOVE, INVALID_INDEX, v3Position);
		}
		
		return true;
	}

	return false;
}

/*--------------------
  AddOrder
  --------------------*/
TRIGGER_CMD(AddOrder, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: SetOrder <index> <target index>") << newl;
		return false;
	}*/

	if (vArgList.size() == 2)
	{
		IVisualEntity *pTargetEnt(GameServer.GetVisualEntity(AtoI(vArgList[1])));
		ECommanderOrder eOrder;

		if (pTargetEnt == NULL)
		{
			Console << _T("SetOrder: Invalid target world index.") << newl;
			return true;
		}

		if (pEnt->IsEnemy(pTargetEnt))
			eOrder = CMDR_ORDER_ATTACK;
		else
			eOrder = CMDR_ORDER_MOVE;

		if (pEnt->IsPlayer())
		{
			IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());
			pPlayer->AddOrder(eOrder, AtoI(vArgList[1]), V3_ZERO);
		}
		else if (pEnt->IsNpc())
		{
			if (pEnt->IsEnemy(pTargetEnt))
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_ATTACK, AtoI(vArgList[1]), V3_ZERO);
			else if (pTargetEnt->IsBuilding() && (pTargetEnt->GetStatus() == ENTITY_STATUS_SPAWNING || pTargetEnt->GetHealthPercent() < 1.0f))
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_REPAIR, AtoI(vArgList[1]), V3_ZERO);
			else
				pEnt->GetAsNpc()->PlayerCommand(NPCCMD_FOLLOW, AtoI(vArgList[1]), V3_ZERO);
		}
		else if (pEnt->IsPet())
		{
			if (pEnt->IsEnemy(pTargetEnt))
				pEnt->GetAsPet()->PlayerCommand(PETCMD_ATTACK, AtoI(vArgList[1]), V3_ZERO);
			else if (pTargetEnt->IsBuilding() && (pTargetEnt->GetStatus() == ENTITY_STATUS_SPAWNING || pTargetEnt->GetHealthPercent() < 1.0f))
				pEnt->GetAsPet()->PlayerCommand(PETCMD_REPAIR, AtoI(vArgList[1]), V3_ZERO);
			else
				pEnt->GetAsPet()->PlayerCommand(PETCMD_FOLLOW, AtoI(vArgList[1]), V3_ZERO);
		}

		return true;
	}
	else if (vArgList.size() >= 4)
	{
		CVec3f v3Position(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3]));

		if (pEnt->IsPlayer())
		{
			IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());
			pPlayer->AddOrder(CMDR_ORDER_MOVE, INVALID_INDEX, v3Position);
		}
		else if (pEnt->IsNpc())
		{
			INpcEntity *pNpc(pEnt->GetAsNpc());

			pNpc->PlayerCommand(NPCCMD_MOVE, INVALID_INDEX, v3Position);
		}
		else if (pEnt->IsPet())
		{
			IPetEntity *pPet(pEnt->GetAsPet());

			pPet->PlayerCommand(PETCMD_MOVE, INVALID_INDEX, v3Position);
		}
		
		return true;
	}

	return false;
}

/*--------------------
  ClearOrders
  --------------------*/
TRIGGER_CMD(ClearOrders, Combat, 1)
{
	if (pEnt->IsPlayer())
	{
		IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

		pPlayer->ClearOrders();
	}
	else if (pEnt->IsNpc())
	{
		INpcEntity *pNpc(pEnt->GetAsNpc());

		pNpc->PlayerCommand(NPCCMD_STOP, INVALID_INDEX, V3_ZERO);
	}
	else if (pEnt->IsPet())
	{
		IPetEntity *pPet(pEnt->GetAsPet());

		pPet->PlayerCommand(PETCMD_STOP, INVALID_INDEX, V3_ZERO);
	}
		
	return true;
}

/*--------------------
  TriggerEntity
  --------------------*/
TRIGGER_CMD(TriggerEntity, Trigger, 3)
{
	/*if (vArgList.size() < 3)
	{
		Console << _T("Syntax: Trigger <trigger index> <triggering index> <play effect, true/false>") << newl;
		return false;
	}*/

	pEnt->Trigger(AtoI(vArgList[1]), _T(""), AtoB(vArgList[2]));

	return true;
}

/*--------------------
  EnableTrigger
  --------------------*/
TRIGGER_CMD(EnableTrigger, Trigger, 1)
{
	/*if (vArgList.size() < 1)
	{
		Console << _T("Syntax: EnableTrigger <index> [amount of time - optional]") << newl;
		return false;
	}*/

	if (vArgList.size() > 1)
		pEnt->Enable(AtoI(vArgList[1]));
	else
		pEnt->Enable();

	return true;
}

/*--------------------
  DisableTrigger
  --------------------*/
TRIGGER_CMD(DisableTrigger, Trigger, 1)
{
	/*if (vArgList.size() < 1)
	{
		Console << _T("Syntax: DisableTrigger <index> [amount of time - optional]") << newl;
		return false;
	}*/

	if (vArgList.size() > 1)
		pEnt->Disable(AtoI(vArgList[1]));
	else
		pEnt->Disable();

	return true;
}


/*--------------------
  EnableItem
  --------------------*/
TRIGGER_CMD(EnableItem, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: EnableItem <index> <slot>") << newl;
		return false;
	}*/

	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem != NULL)
		pItem->Enable();

	return true;
}


/*--------------------
  DisableItem
  --------------------*/
TRIGGER_CMD(DisableItem, Combat, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: DisableItem <index> <slot>") << newl;
		return false;
	}*/

	IInventoryItem *pItem(pEnt->GetItem(AtoI(vArgList[1]) - 1));

	if (pItem != NULL)
		pItem->Disable();

	return true;
}

/*--------------------
  RegisterEntityScript
  --------------------*/
TRIGGER_CMD(RegisterEntityScript, Visual, 3)
{
	TriggerManager.RegisterEntityScript(pEnt->GetIndex(), vArgList[1], ConcatinateArgs(vArgList.begin() + 2, vArgList.end()));

	return true;
}

/*--------------------
  RegisterGlobalScript
  --------------------*/
SERVER_CMD(RegisterGlobalScript)
{
	if (vArgList.size() < 2)
		return false;

	TriggerManager.RegisterGlobalScript(vArgList[2], ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));

	return true;
}

/*--------------------
  StartEffect
  --------------------*/
SERVER_CMD(StartEffect)
{
	if (vArgList.size() < 4)
	{
		Console << _T("Syntax: StartEffect <effect> <X> <Y> <Z>") << newl;
		return false;
	}

	CGameEvent ev;
	ev.SetSourcePosition(CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));
	ev.SetEffect(g_ResourceManager.Register(vArgList[0], RES_EFFECT));
	ev.Spawn();
	GameServer.AddEvent(ev);

	return true;
}

/*--------------------
  StartEffectOnObject
  --------------------*/
TRIGGER_CMD(StartEffectOnObject, Visual, 2)
{
	/*if (vArgList.size() < 2)
	{
		Console << _T("Syntax: StartEffectOnObject <index> <effect>") << newl;
		return false;
	}*/

	pEnt->SetEffect(EFFECT_CHANNEL_TRIGGER, g_ResourceManager.Register(vArgList[1], RES_EFFECT));
	pEnt->IncEffectSequence(EFFECT_CHANNEL_TRIGGER);

	return true;
}

/*--------------------
  DamageEntity
  --------------------*/
TRIGGER_CMD(DamageEntity, Visual, 2)
{
	pEnt->Damage(AtoF(vArgList[1]), DAMAGE_FLAG_DIRECT);

	return true;
}

/*--------------------
  SpawnObject
  --------------------*/
SERVER_CMD(SpawnObject)
{
	if (vArgList.size() < 5)
	{
		Console << _T("Syntax: SpawnObject <object> <X> <Y> <Z> <team> [Model - Optional] [Definition - Optional]") << newl;
		return false;
	}

	ushort unID(EntityRegistry.LookupID(vArgList[0]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("SpawnObject: Invalid object specified.") << newl;
		return true;
	}

	IGameEntity* pNewEnt(GameServer.AllocateEntity(unID));

	if (pNewEnt == NULL)
	{
		Console << _T("SpawnObject: Object could not be allocated.") << newl;
		return true;
	}

	IVisualEntity *pEnt(pNewEnt->GetAsVisualEnt());

	if (pEnt == NULL)
	{
		Console << _T("SpawnObject: Improper entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiWorldIndex(GameServer.AllocateNewWorldEntity());
	CWorldEntity *pWorldEnt(GameServer.GetWorldEntity(uiWorldIndex));

	if (pWorldEnt == NULL)
	{
		Console << _T("SpawnObject: World entity could not be created.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	pWorldEnt->SetType(vArgList[0]);
	pWorldEnt->SetTeam(AtoI(vArgList[4]));
	pWorldEnt->SetPosition(CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));
	pWorldEnt->SetGameIndex(pEnt->GetIndex());

	if (vArgList.size() > 5)
	{
		pWorldEnt->SetModelPath(vArgList[5]);
		pWorldEnt->SetModelHandle(GameServer.RegisterModel(vArgList[5]));
	}

	if (vArgList.size() > 6)
		pWorldEnt->SetProperty(_T("definition"), vArgList[6]);

	if (pEnt->GetWorldIndex() != INVALID_INDEX)
		GameServer.DeleteWorldEntity(pEnt->GetWorldIndex());

	pEnt->ApplyWorldEntity(*pWorldEnt);
	pEnt->Spawn();

	pEnt->Validate();

	if (g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("spawnedindex"), XtoA(pEnt->GetIndex()));

	return true;
}


/*--------------------
  SpawnFromObject
  --------------------*/
TRIGGER_CMD(SpawnFromObject, Visual, 3)
{
	ushort unID(EntityRegistry.LookupID(vArgList[1]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("SpawnFromObject: Invalid object specified.") << newl;
		return true;
	}

	IGameEntity* pNewEnt(GameServer.AllocateEntity(unID, pEnt->GetIndex() + 1));

	if (pNewEnt == NULL)
	{
		Console << _T("SpawnFromObject: Object could not be allocated.") << newl;
		return true;
	}

	IVisualEntity *pVisEnt(pNewEnt->GetAsVisualEnt());

	if (pVisEnt == NULL)
	{
		Console << _T("SpawnFromObject: Improper entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiWorldIndex(GameServer.AllocateNewWorldEntity());
	CWorldEntity *pWorldEnt(GameServer.GetWorldEntity(uiWorldIndex));

	if (pWorldEnt == NULL)
	{
		Console << _T("SpawnFromObject: World entity could not be created.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	pWorldEnt->SetType(vArgList[1]);
	pWorldEnt->SetTeam(AtoI(vArgList[2]));
	pWorldEnt->SetPosition(pEnt->GetPosition());
	pWorldEnt->SetAngles(pEnt->GetAngles());
	pWorldEnt->SetGameIndex(pVisEnt->GetIndex());

	if (vArgList.size() > 3)
	{
		pWorldEnt->SetModelPath(vArgList[3]);
		pWorldEnt->SetModelHandle(GameServer.RegisterModel(vArgList[3]));
	}

	if (vArgList.size() > 4)
		pWorldEnt->SetProperty(_T("definition"), vArgList[4]);

	if (pVisEnt->GetWorldIndex() != INVALID_INDEX)
		GameServer.DeleteWorldEntity(pVisEnt->GetWorldIndex());

	pVisEnt->ApplyWorldEntity(*pWorldEnt);
	pVisEnt->Spawn();

	pVisEnt->Validate();

	if (g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("spawnedindex"), XtoA(pVisEnt->GetIndex()));

	return true;
}

/*--------------------
  CopyObject
  --------------------*/
TRIGGER_CMD(CopyObject, Visual, 1)
{
	/*if (vArgList.size() < 1)
	{
		Console << _T("Syntax: CopyObject <target index>") << newl;
		return false;
	}*/

	IGameEntity* pNewEnt(GameServer.AllocateEntity(pEnt->GetType()));

	if (pNewEnt == NULL)
	{
		Console << _T("CopyObject: Object could not be allocated.") << newl;
		return true;
	}

	IVisualEntity *pVisEnt(pNewEnt->GetAsVisualEnt());

	if (pVisEnt == NULL || pVisEnt->GetType() != pEnt->GetType())
	{
		Console << _T("CopyObject: Improper entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiWorldIndex(GameServer.AllocateNewWorldEntity());
	CWorldEntity *pWorldEnt(GameServer.GetWorldEntity(uiWorldIndex));

	if (pWorldEnt == NULL)
	{
		Console << _T("CopyObject: World entity could not be created.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiNewIndex(pEnt->GetWorldIndex());
	CWorldEntity *pTargetEnt(GameServer.GetWorldEntity(uiNewIndex));

	(*pWorldEnt) = (*pTargetEnt);
	pWorldEnt->SetIndex(uiNewIndex);
	pWorldEnt->SetGameIndex(pVisEnt->GetIndex());

	pVisEnt->ApplyWorldEntity(*pWorldEnt);
	pVisEnt->Spawn();

	pVisEnt->Validate();

	if (g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("spawnedindex"), XtoA(pVisEnt->GetIndex()));

	return true;
}


/*--------------------
  SpawnEntity
  --------------------*/
SERVER_CMD(SpawnEntity)
{
	if (vArgList.size() < 1 || (int(vArgList.size()) - 1) % 2 != 0)
	{
		Console << _T("Syntax: SpawnEntity <type> [property1] [value1] [property2] [value2] ... [propertyN] [valueN]") << newl;
		return false;
	}

	ushort unID(EntityRegistry.LookupID(vArgList[0]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("SpawnEntity: Invalid entity specified.") << newl;
		return true;
	}

	IGameEntity* pNewEnt(GameServer.AllocateEntity(unID));

	if (pNewEnt == NULL)
	{
		Console << _T("SpawnEntity: Entity could not be allocated.") << newl;
		return true;
	}

	IVisualEntity *pEnt(pNewEnt->GetAsVisualEnt());

	if (pEnt == NULL)
	{
		Console << _T("SpawnEntity: Improper entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiWorldIndex(GameServer.AllocateNewWorldEntity());
	CWorldEntity *pWorldEnt(GameServer.GetWorldEntity(uiWorldIndex));

	if (pWorldEnt == NULL)
	{
		Console << _T("SpawnEntity: World entity could not be created.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	pWorldEnt->SetType(vArgList[0]);
	pWorldEnt->SetGameIndex(pNewEnt->GetIndex());

	int iNumProperties((int(vArgList.size()) - 1) / 2);

	for (int i(0); i < iNumProperties; ++i)
	{
		const tstring &sProperty(vArgList[1 + i * 2]);
		const tstring &sValue(vArgList[1 + i * 2 + 1]);

		if (sProperty == _T("position"))
		{
			pWorldEnt->SetPosition(AtoV3(sValue));
		}
		else if (sProperty == _T("angles"))
		{
			pWorldEnt->SetAngles(AtoV3(sValue));
		}
		else if (sProperty == _T("scale"))
		{
			pWorldEnt->SetScale(AtoF(sValue));
		}
		else if (sProperty == _T("team"))
		{
			pWorldEnt->SetTeam(AtoI(sValue));
		}
		else if (sProperty == _T("model"))
		{
			pWorldEnt->SetModelPath(sValue);
		}
		else if (sProperty == _T("name"))
		{
			pWorldEnt->SetName(sValue);
		}
		else
		{
			pWorldEnt->SetProperty(sProperty, sValue);
		}
	}

	// This probably isn't needed, but just to be sure
	if (pEnt->GetWorldIndex() != INVALID_INDEX)
		GameServer.DeleteWorldEntity(pEnt->GetWorldIndex());

	pEnt->ApplyWorldEntity(*pWorldEnt);
	pEnt->Spawn();

	pEnt->Validate();

	if (g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("spawnedindex"), XtoA(pEnt->GetIndex()));

	return true;
}


/*--------------------
  SpawnEntityAtEntity
  --------------------*/
TRIGGER_CMD(SpawnEntityAtEntity, Visual, 2)
{
	if ((int(vArgList.size()) - 2) % 2 != 0)
	{
		Console << _T("Syntax: SpawnEntityAtEntity <entity> <type> [property1] [value1] [property2] [value2] ... [propertyN] [valueN]") << newl;
		return false;
	}

	ushort unID(EntityRegistry.LookupID(vArgList[1]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("SpawnEntityAtEntity: Invalid entity specified.") << newl;
		return true;
	}

	IGameEntity* pNewEnt(GameServer.AllocateEntity(unID));

	if (pNewEnt == NULL)
	{
		Console << _T("SpawnEntityAtEntity: Object could not be allocated.") << newl;
		return true;
	}

	IVisualEntity *pVisEnt(pNewEnt->GetAsVisualEnt());

	if (pVisEnt == NULL)
	{
		Console << _T("SpawnEntityAtEntity: Improper entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	uint uiWorldIndex(GameServer.AllocateNewWorldEntity());
	CWorldEntity *pWorldEnt(GameServer.GetWorldEntity(uiWorldIndex));

	if (pWorldEnt == NULL)
	{
		Console << _T("SpawnFromObject: World entity could not be created.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	pWorldEnt->SetType(vArgList[1]);
	pWorldEnt->SetGameIndex(pNewEnt->GetIndex());

	// Use <entity> position and angles as defaults
	pWorldEnt->SetPosition(pEnt->GetPosition());
	pWorldEnt->SetAngles(pEnt->GetAngles());

	int iNumProperties((int(vArgList.size()) - 2) / 2);

	for (int i(0); i < iNumProperties; ++i)
	{
		const tstring &sProperty(vArgList[2 + i * 2]);
		const tstring &sValue(vArgList[2 + i * 2 + 1]);

		if (sProperty == _T("position"))
		{
			pWorldEnt->SetPosition(AtoV3(sValue));
		}
		else if (sProperty == _T("angles"))
		{
			pWorldEnt->SetAngles(AtoV3(sValue));
		}
		else if (sProperty == _T("scale"))
		{
			pWorldEnt->SetScale(AtoF(sValue));
		}
		else if (sProperty == _T("team"))
		{
			pWorldEnt->SetTeam(AtoI(sValue));
		}
		else if (sProperty == _T("model"))
		{
			pWorldEnt->SetModelPath(sValue);
		}
		else if (sProperty == _T("name"))
		{
			pWorldEnt->SetName(sValue);
		}
		else
		{
			pWorldEnt->SetProperty(sProperty, sValue);
		}
	}

	pVisEnt->ApplyWorldEntity(*pWorldEnt);
	pVisEnt->Spawn();

	pVisEnt->Validate();

	if (g_pCurrentScript != NULL)
		g_pCurrentScript->AddParameter(_T("spawnedindex"), XtoA(pVisEnt->GetIndex()));

	return true;
}


/*--------------------
  SetAmmo
  --------------------*/
TRIGGER_CMD(SetAmmo, Combat, 3)
{
	pEnt->SetAmmo(AtoI(vArgList[1]) - 1, AtoI(vArgList[2]));
	return true;
}

/*--------------------
  RefillAmmo
  --------------------*/
TRIGGER_CMD(RefillAmmo, Combat, 1)
{
	if (vArgList.size() > 1)
		pEnt->RefillAmmo(AtoI(vArgList[1]) - 1);
	else
		pEnt->RefillAmmo();

	return true;
}

/*--------------------
  GiveItem
  --------------------*/
TRIGGER_CMD(GiveItem, Visual, 3)
{
	ushort unID(EntityRegistry.LookupID(vArgList[2]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("GiveItem: Invalid item type.");
		return true;
	}

	pEnt->GiveItem(AtoI(vArgList[1]) - 1, unID);

	return true;
}

/*--------------------
  TakeItem
  --------------------*/
TRIGGER_CMD(TakeItem, Visual, 2)
{
	pEnt->RemoveItem(AtoI(vArgList[1]) - 1);

	return true;
}


/*--------------------
  SwapItem
  --------------------*/
TRIGGER_CMD(SwapItem, Visual, 3)
{
	pEnt->SwapItem(AtoI(vArgList[1]) - 1, AtoI(vArgList[2]) - 1);

	return true;
}


/*--------------------
  ExecScript
  --------------------*/
SERVER_CMD(ExecScript)
{
	if (vArgList.size() < 1)
		return false;

	uint uiParam(1);
	while (uiParam + 1 < INT_SIZE(vArgList.size()))
	{
		TriggerManager.RegisterTriggerParam(vArgList[uiParam], vArgList[uiParam + 1]);
		uiParam += 2;
	}

	TriggerManager.TriggerGlobalScript(vArgList[0]);

	return true;
}

/*--------------------
  ClientExecScript
  --------------------*/
SERVER_CMD(ClientExecScript)
{
	if (vArgList.size() < 2)
		return false;

	int iClientNum(AtoI(vArgList[0]));

	CBufferDynamic buffer;

	buffer << GAME_CMD_EXEC_SCRIPT << vArgList[1] << byte(0);

	// Add # of arguments to buffer
	buffer << short(INT_FLOOR((vArgList.size() - 2) / 2));

	uint uiParam(2);
	while (uiParam + 1 < INT_SIZE(vArgList.size()))
	{
		buffer << vArgList[uiParam] << byte(0) << vArgList[uiParam + 1] << byte(0);
		uiParam += 2;
	}

	if (iClientNum >= 0)
		GameServer.SendGameData(iClientNum, buffer, true);
	else
		GameServer.BroadcastGameData(buffer, true);

	return true;
}

/*--------------------
  SendMessage
  --------------------*/
SERVER_CMD(SendMessage)
{
	if (vArgList.size() < 2)
		return false;

	CBufferDynamic buffer;

	buffer << GAME_CMD_SCRIPT_MESSAGE << ConcatinateArgs(vArgList.begin() + 1, vArgList.end()) << byte(0);

	int iClientNum(AtoI(vArgList[0]));

	if (iClientNum >= 0)
		GameServer.SendGameData(iClientNum, buffer, true);
	else
		GameServer.BroadcastGameData(buffer, true);

	return true;
}

/*--------------------
  IsAlliedTeam
  --------------------*/
SERVER_FCN(IsAlliedTeam)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iTargetTeam(AtoI(vArgList[1]));

	if (GameServer.GetTeam(iTeam) == NULL)
		return _T("");

	return XtoA(GameServer.GetTeam(iTeam)->IsAlliedTeam(iTargetTeam), true);
}

/*--------------------
  AddAlliedTeam
  --------------------*/
SERVER_CMD(AddAlliedTeam)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iTargetTeam(AtoI(vArgList[1]));

	if (GameServer.GetTeam(iTeam) == NULL)
		return true;

	GameServer.GetTeam(iTeam)->AddAlliedTeam(iTargetTeam);
	return true;
}

/*--------------------
  RemoveAlliedTeam
  --------------------*/
SERVER_CMD(RemoveAlliedTeam)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iTargetTeam(AtoI(vArgList[1]));

	if (GameServer.GetTeam(iTeam) == NULL)
		return true;

	GameServer.GetTeam(iTeam)->RemoveAlliedTeam(iTargetTeam);
	return true;
}

/*--------------------
  GetMaxTeams
  --------------------*/
SERVER_FCN(GetMaxTeams)
{
	return XtoA(Game.GetNumTeams());
}

/*--------------------
  FileExists
  --------------------*/
SERVER_FCN(FileExists)
{
	if (vArgList.size() < 1)
		return false;

	return XtoA(FileManager.Exists(vArgList[0]), true);
}

/*--------------------
  GetNumClients
  --------------------*/
SERVER_FCN(GetNumClients)
{
	if (vArgList.size() < 1)
		return XtoA(GameServer.GetConnectedClientCount());

	return XtoA(GameServer.GetConnectedClientCount(AtoI(vArgList[0])));
}

/*--------------------
  ClearStates
  --------------------*/
TRIGGER_CMD(ClearStates, Visual, 1)
{
	pEnt->ClearStates();

	for (int i(0); i < MAX_INVENTORY; ++i)
	{
		IInventoryItem *pItem(pEnt->GetItem(i));

		if (!pItem)
			continue;

		pItem->ActivatePassive();
	}

	return true;
}

/*--------------------
  RefreshCooldowns
  --------------------*/
TRIGGER_CMD(RefreshCooldowns, Visual, 1)
{
	for (int i(0); i < MAX_INVENTORY; ++i)
	{
		IInventoryItem *pItem(pEnt->GetItem(i));

		if (!pItem)
			continue;

		pItem->SetCooldownTimer(INVALID_TIME, INVALID_TIME);
	}

	return true;
}

/*--------------------
  SetStatus
  --------------------*/
TRIGGER_CMD(SetStatus, Visual, 2)
{
	int iStatus;
	tstring sStatus(LowerString(vArgList[1]));

	if (sStatus == _T("preview"))
		iStatus = ENTITY_STATUS_PREVIEW;
	else if (sStatus == _T("dormant"))
		iStatus = ENTITY_STATUS_DORMANT;
	else if (sStatus == _T("dead"))
		iStatus = ENTITY_STATUS_DEAD;
	else if (sStatus == _T("corpse"))
		iStatus = ENTITY_STATUS_CORPSE;
	else if (sStatus == _T("spawning"))
		iStatus = ENTITY_STATUS_SPAWNING;
	else if (sStatus == _T("hidden"))
		iStatus = ENTITY_STATUS_HIDDEN;
	else
		iStatus = ENTITY_STATUS_ACTIVE;

	pEnt->SetStatus(iStatus);

	return true;
}

/*--------------------
  SetBuildPercent
  --------------------*/
TRIGGER_CMD(SetBuildPercent, Building, 2)
{
	pEnt->SetBuildPercent(AtoF(vArgList[1]));

	return true;
}

/*--------------------
  SpawnSoul
  --------------------*/
TRIGGER_CMD(SpawnSoul, Player, 4)
{
	CEntitySoul *pSoul(static_cast<CEntitySoul*>(Game.AllocateEntity(_T("Entity_Soul"))));
	if (pSoul == NULL)
	{
		Console.Warn << _T("Failed to create soul entity") << newl;
		return false;
	}
	else
	{
		pSoul->SetPosition(CVec3f(AtoF(vArgList[1]), AtoF(vArgList[2]), AtoF(vArgList[3])));
		pSoul->SetTarget(pEnt->GetIndex());
		pSoul->Spawn();
	}

	return true;
}

/*--------------------
  CastSpell
  --------------------*/
TRIGGER_CMD(CastSpell, Combat, 2)
{
	ushort unID(EntityRegistry.LookupID(vArgList[1]));

	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("CastSpell: Invalid spell type.") << newl;
		return true;
	}

	IInventoryItem *pNewEnt;
	
	pNewEnt = static_cast<IInventoryItem *>(GameServer.AllocateEntity(unID));

	if (pNewEnt == NULL)
	{
		Console << _T("CastSpell: Could not allocate spell.") << newl;
		return true;
	}

	if (!pNewEnt->IsSpell() || pNewEnt->GetType() != unID)
	{
		Console << _T("CastSpell: Failed to allocate correct entity type.") << newl;
		GameServer.DeleteEntity(pNewEnt);
		return true;
	}

	ISpellItem *pSpell(pNewEnt->GetAsSpell());
	CGameEvent ev;

	if (pSpell->IsSnapcast())
		pSpell->ImpactEntity(pEnt->GetIndex(), ev, false);
	else
	{
		pSpell->SetOwner(pEnt->GetIndex());
		pSpell->ImpactPosition(pEnt->GetPosition(), ev);
	}

	// Impact event
	if (!pSpell->GetImpactEffectPath().empty())
	{
		ev.SetEffect(g_ResourceManager.Register(pSpell->GetImpactEffectPath(), RES_EFFECT));
		Game.AddEvent(ev);
	}

	GameServer.DeleteEntity(pNewEnt);

	return true;
}
