// (C)2006 S2 Games
// game_server_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"

#include "../hon_shared/c_player.h"
#include "../hon_shared/c_replaymanager.h"
#include "../hon_shared/i_unitentity.h"
#include "../hon_shared/i_heroentity.h"
#include "../hon_shared/i_creepentity.h"
#include "../hon_shared/i_entityitem.h"
#include "../hon_shared/i_entityability.h"
#include "../hon_shared/c_entityneutralcampcontroller.h"
#include "../hon_shared/c_entitykongorcontroller.h"

#include "../k2/c_hostserver.h"
#include "../k2/c_uicmd.h"
//=============================================================================

/*--------------------
  Damage
  --------------------*/
SERVER_CMD(Damage)
{
	if (vArgList.size() < 2)
		return false;

	uint uiIndex(AtoI(vArgList[0]));
	float fDamage(AtoF(vArgList[1]));

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	CDamageEvent dmg;
	dmg.SetAmount(fDamage);
	dmg.SetTargetIndex(pTarget->GetIndex());
	dmg.ApplyDamage();
	return true;
}


/*--------------------
  Kill
  --------------------*/
SERVER_CMD(Kill)
{
	if (vArgList.size() < 1)
		return false;

	uint uiIndex(AtoI(vArgList[0]));

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->Kill();
	return true;
}


/*--------------------
  Armageddon
  --------------------*/
SERVER_CMD(Armageddon)
{
	IGameEntity *pEnt(Game.GetFirstEntity());
	while (pEnt != NULL)
	{
		IUnitEntity *pUnit(pEnt->GetAsUnit());
		if (pUnit != NULL && !pUnit->IsBuilding() && !pUnit->IsBit())
			pUnit->Kill();

		pEnt = Game.GetNextEntity(pEnt);
	}

	return true;
}


/*--------------------
  GiveExp
  --------------------*/
SERVER_CMD(GiveExp)
{
	if (vArgList.size() < 2)
		return false;

	uint uiIndex(AtoI(vArgList[0]));
	float fExperience(AtoF(vArgList[1]));

	IHeroEntity *pTarget(GameServer.GetHeroEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->GiveExperience(fExperience);
	return true;
}


/*--------------------
  LevelUp
  --------------------*/
SERVER_CMD(LevelUp)
{
	if (vArgList.size() < 1)
		return false;

	uint uiIndex(AtoI(vArgList[0]));

	IHeroEntity *pTarget(GameServer.GetHeroEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	float fExperience(pTarget->GetExperienceForNextLevel() - pTarget->GetExperience());
	pTarget->GiveExperience(fExperience);
	return true;
}


/*--------------------
  LevelMax
  --------------------*/
SERVER_CMD(LevelMax)
{
	if (vArgList.size() < 1)
		return false;

	uint uiIndex(AtoI(vArgList[0]));

	IHeroEntity *pTarget(GameServer.GetHeroEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	float fExperience(pTarget->GetExperienceForLevel(GameServer.GetHeroMaxLevel()));
	pTarget->GiveExperience(fExperience);

	for (int iSlot(INVENTORY_START_ABILITIES); iSlot <= INVENTORY_END_ABILITIES; ++iSlot)
	{
		IEntityAbility *pAbility(pTarget->GetAbility(iSlot));
		if (pAbility == NULL)
			continue;

		while (pAbility->CanLevelUp()) pAbility->LevelUp();
	}

	return true;
}


/*--------------------
  ResetExp
  --------------------*/
SERVER_CMD(ResetExp)
{	
	if (vArgList.size() < 1)
		return false;

	uint uiIndex(AtoI(vArgList[0]));
	IHeroEntity *pTarget(GameServer.GetHeroEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->ResetExperience();
	return true;
}


/*--------------------
  GiveGold
  --------------------*/
SERVER_CMD(GiveGold)
{
	if (vArgList.size() < 2)
		return false;

	int iClientNum(AtoI(vArgList[0]));
	ushort unGold(AtoI(vArgList[1]));
	CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(iClientNum));
	if (pPlayer == NULL)
		return false;
		
	pPlayer->GiveGold(unGold, pPlayer->GetHero());
	return true;
}



/*--------------------
  TakeGold
  --------------------*/
SERVER_CMD(TakeGold)
{
	if (vArgList.size() < 2)
		return false;

	int iClientNum(AtoI(vArgList[0]));
	ushort unGold(AtoI(vArgList[1]));
	CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(iClientNum));
	if (pPlayer == NULL)
		return false;
		
	pPlayer->TakeGold(unGold);
	return true;
}


/*--------------------
  Refresh
  --------------------*/
SERVER_CMD(Refresh)
{
	uint uiIndex(INVALID_INDEX);
	if (vArgList.size() > 0)
		uiIndex = AtoI(vArgList[0]);
	else
	{
		CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(0));
		if (pPlayer)
			uiIndex = pPlayer->GetHeroIndex();
	}

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->SetHealth(pTarget->GetMaxHealth());
	pTarget->SetMana(pTarget->GetMaxMana());

	for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_BACKPACK; ++i)
	{
		IEntityTool *pItem(pTarget->GetTool(i));
		if (pItem == NULL)
			continue;

		pItem->ResetCooldown();
	}

	pTarget->ResetCooldowns();
		
	return true;
}


/*--------------------
  Stun
  --------------------*/
SERVER_CMD(Stun)
{
	uint uiIndex(INVALID_INDEX);
	if (vArgList.size() > 0)
		uiIndex = AtoI(vArgList[0]);
	else
	{
		CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(0));
		if (pPlayer)
			uiIndex = pPlayer->GetHeroIndex();
	}

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->ApplyState(EntityRegistry.LookupID(_T("State_Stunned")), 1, Game.GetGameTime(), 5000);
		
	return true;
}


/*--------------------
  Silence
  --------------------*/
SERVER_CMD(Silence)
{
	uint uiIndex(INVALID_INDEX);
	if (vArgList.size() > 0)
		uiIndex = AtoI(vArgList[0]);
	else
	{
		CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(0));
		if (pPlayer)
			uiIndex = pPlayer->GetHeroIndex();
	}

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->ApplyState(EntityRegistry.LookupID(_T("State_Defiler_Ability2")), 1, Game.GetGameTime(), 5000);
		
	return true;
}


/*--------------------
  Ministun
  --------------------*/
SERVER_CMD(Ministun)
{
	uint uiIndex(INVALID_INDEX);
	if (vArgList.size() > 0)
		uiIndex = AtoI(vArgList[0]);
	else
	{
		CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(0));
		if (pPlayer)
			uiIndex = pPlayer->GetHeroIndex();
	}

	IUnitEntity *pTarget(GameServer.GetUnitEntity(uiIndex));
	if (pTarget == NULL)
		return false;

	pTarget->ApplyState(EntityRegistry.LookupID(_T("State_Stunned")), 1, Game.GetGameTime(), 100);
		
	return true;
}


/*--------------------
  GiveItem
  --------------------*/
SERVER_CMD(GiveItem)
{
	if (vArgList.size() < 2)
		return false;

	IUnitEntity *pTarget(GameServer.GetUnitEntity(AtoI(vArgList[0])));
	if (pTarget == NULL)
		return false;

	ushort unItemID(EntityRegistry.LookupID(vArgList[1]));

	int iTargetSlot(-1);
	for (int iSlot(INVENTORY_START_BACKPACK); iSlot <= INVENTORY_END_BACKPACK; ++iSlot)
	{
		IEntityItem *pItem(pTarget->GetItem(iSlot));
		if (pItem == NULL)
		{
			if (iTargetSlot == -1)
				iTargetSlot = iSlot;
			continue;
		}
		if (pItem->GetType() == unItemID &&
			pItem->GetRechargeable() &&
			pItem->GetInitialCharges() > 0 &&
			(pItem->GetMaxCharges() == -1 || pItem->GetCharges() + pItem->GetInitialCharges() <= uint(pItem->GetMaxCharges())))
		{
			iTargetSlot = iSlot;
			break;
		}
	}
	
	if (iTargetSlot != -1)
		pTarget->GiveItem(iTargetSlot, unItemID, vArgList.size() > 2 ? AtoB(vArgList[2]) : true);

	return true;
}


/*--------------------
  RemoveHero
  --------------------*/
SERVER_CMD(RemoveHero)
{
	if (vArgList.empty())
		return false;

	GameServer.RemoveHero(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  ResetPicks
  --------------------*/
SERVER_CMD(ResetPicks)
{
	if (vArgList.empty())
		return false;

	GameServer.ResetPicks(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  MorphHero
  --------------------*/
SERVER_CMD(MorphHero)
{
	if (vArgList.size() < 2)
		return false;

	int iClientNum(AtoI(vArgList[0]));
	CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(iClientNum));
	if (pPlayer == NULL)
		return false;

	IHeroEntity *pHero(pPlayer->GetHero());
	if (pHero == NULL)
		return true;

	pHero->MorphDynamicType(EntityRegistry.LookupID(vArgList[1]));
	pHero->UpdateInventory();
	
	return true;
}


/*--------------------
  SpawnUnit
  --------------------*/
SERVER_CMD(SpawnUnit)
{
	if (vArgList.size() < 4)
		return false;

	ushort unUnitID(EntityRegistry.LookupID(vArgList[0]));
	if (unUnitID == INVALID_ENT_TYPE)
		return false;

	IGameEntity *pTarget(GameServer.GetEntityFromName(vArgList[1]));
	if (pTarget == NULL || !pTarget->IsVisual())
		return false;
	CVec3f v3Position(pTarget->GetAsVisual()->GetPosition());

	int iClientNum(AtoI(vArgList[2]));
	CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(iClientNum));
	if (pPlayer == NULL)
		return false;

	int iTeam(CLAMP(AtoI(vArgList[3]), 0, 2));

	IGameEntity *pNewEntity(GameServer.AllocateEntity(unUnitID));
	if (pNewEntity == NULL)
		return false;

	IUnitEntity *pUnit(pNewEntity->GetAsUnit());
	if (pUnit == NULL)
	{
		GameServer.DeleteEntity(pNewEntity);
		return false;
	}

	pUnit->SetTeam(iTeam);
	pUnit->SetPosition(v3Position);
	pUnit->SetLevel(1);
	pPlayer->AddPet(pUnit, 0, INVALID_INDEX);
	pUnit->Spawn();
	return true;
}


/*--------------------
  SpawnUnit2

  <type> <owner> <team> <x> <y> <avatar>
  --------------------*/
SERVER_CMD(SpawnUnit2)
{
	if (vArgList.size() < 5)
		return false;

	const tstring &sHeroName(vArgList[0].substr(0, vArgList[0].find_first_of(_T('.'))));
	const tstring &sAvatar(vArgList[0].substr(vArgList[0].find_first_of(_T('.')) + 1));

	ushort unUnitID(EntityRegistry.LookupID(sHeroName));
	if (unUnitID == INVALID_ENT_TYPE)
		return false;

	int iClientNum(AtoI(vArgList[1]));
	CPlayer *pPlayer(GameServer.GetPlayerFromClientNumber(iClientNum));

	int iTeam(CLAMP(AtoI(vArgList[2]), int(TEAM_NEUTRAL), 2));

	IGameEntity *pNewEntity(GameServer.AllocateEntity(unUnitID));
	if (pNewEntity == NULL)
		return false;

	IUnitEntity *pUnit(pNewEntity->GetAsUnit());
	if (pUnit == NULL)
	{
		GameServer.DeleteEntity(pNewEntity);
		return false;
	}

	GameServer.Precache(sHeroName, PRECACHE_ALL, sAvatar);

	pUnit->SetTeam(iTeam);
	pUnit->SetPosition(Game.GetTerrainPosition(CVec2f(AtoF(vArgList[3]), AtoF(vArgList[4]))));
	pUnit->SetLevel(1);

	uivector vModifierKeys;
	uint uiAvatarKey(EntityRegistry.LookupModifierKey(sAvatar));

	if (uiAvatarKey != INVALID_INDEX)
	{
		vModifierKeys.push_back(uiAvatarKey);
		pUnit->SetPersistentModifierKeys(vModifierKeys);
	}

	if (pPlayer != NULL)
		pPlayer->AddPet(pUnit, 0, INVALID_INDEX);

	pUnit->Spawn();
	return true;
}


/*--------------------
  ServerChat
  --------------------*/
SERVER_CMD(ServerChat)
{
	if (vArgList.empty())
		return false;

	CBufferDynamic buffer;
	buffer << GAME_CMD_SERVERCHAT_ALL << TStringToUTF8(ConcatinateArgs(vArgList, _T(" "))) << byte(0);
	GameServer.BroadcastGameData(buffer, true);

	Console.Server << _T("Server Message: ") << ConcatinateArgs(vArgList, _T(" ")) << newl;
			
	return true;
}


/*--------------------
  StartMatch
  --------------------*/
SERVER_CMD(StartMatch)
{
	GameServer.RequestStartGame(-1);
	return true;
}


/*--------------------
  EndMatch
  --------------------*/
SERVER_CMD(EndMatch)
{
	if (vArgList.empty())
		return false;

	GameServer.EndMatch(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  ServerStatus
  --------------------*/
SERVER_CMD(ServerStatus)
{
	Console << GameServer.GetServerStatus() << newl;
	return true;
}


/*--------------------
  ReplayRecordStart
  --------------------*/
SERVER_CMD(ReplayRecordStart)
{
	if (vArgList.size() < 1)
		Console << _T("syntax: ReplayRecordStart <filename>") << newl;

	ReplayManager.StartRecording(_TS("~/replays/") + vArgList[0]);

	Console << _T("Recording ") << vArgList[0] << newl;
	return true;
}


/*--------------------
  ReplayRecordStop
  --------------------*/
SERVER_CMD(ReplayRecordStop)
{
	ReplayManager.StopRecording();
	return true;
}


/*--------------------
  SpawnCreeps
  --------------------*/
SERVER_CMD(SpawnCreeps)
{
	GameServer.SpawnCreeps();
	return true;
}


/*--------------------
  KillCreeps
  --------------------*/
SERVER_CMD(KillCreeps)
{
	IGameEntity *pEnt(Game.GetFirstEntity());
	while (pEnt != NULL)
	{
		ICreepEntity *pCreep(pEnt->GetAsCreep());
		if (pCreep != NULL)
			pCreep->Kill();

		pEnt = Game.GetNextEntity(pEnt);
	}

	return true;
}


/*--------------------
  KillTrees
  --------------------*/
SERVER_CMD(KillTrees)
{
	GameServer.KillTrees();
	return true;
}


/*--------------------
  SpawnTrees
  --------------------*/
SERVER_CMD(SpawnTrees)
{
	GameServer.SpawnTrees();
	return true;
}


/*--------------------
  SpawnPowerup
  --------------------*/
SERVER_CMD(SpawnPowerup)
{
	GameServer.SpawnPowerup();
	return true;
}


/*--------------------
  SpawnNeutrals
  --------------------*/
SERVER_CMD(SpawnNeutrals)
{
	static uivector vControllers;
	vControllers.clear();

	GameServer.GetEntities(vControllers, Entity_NeutralCampController);

	for (uivector_it it(vControllers.begin()), itEnd(vControllers.end()); it != itEnd; ++it)
	{
		CEntityNeutralCampController *pController(Game.GetEntityAs<CEntityNeutralCampController>(*it));
		if (pController == NULL)
			continue;

		pController->AttemptSpawn();
	}

	return true;
}


/*--------------------
  SpawnKongor
  --------------------*/
SERVER_CMD(SpawnKongor)
{
	static uivector vControllers;
	vControllers.clear();

	GameServer.GetEntities(vControllers, Entity_BossController);

	for (uivector_it it(vControllers.begin()), itEnd(vControllers.end()); it != itEnd; ++it)
	{
		CEntityBossController *pController(Game.GetEntityAs<CEntityBossController>(*it));
		if (pController == NULL)
			continue;

		pController->AttemptSpawn();
	}

	return true;
}


/*--------------------
  SpawnCritters
  --------------------*/
SERVER_CMD(SpawnCritters)
{
	GameServer.SpawnCritters();
	return true;
}


/*--------------------
  Demote
  --------------------*/
SERVER_CMD(Demote)
{
	if (vArgList.empty())
	{
		Console << _T("Must specify client to be demoted") << newl;
		return false;
	}

	int iClientNum(GameServer.GetClientNumFromName(vArgList[0]));
	if (iClientNum == -1)
		iClientNum = AtoI(vArgList[0]);
	if (iClientNum == -1)
	{
		Console << _T("Invalid client: ") << vArgList[0] << newl;
		return false;
	}

	CPlayer *pPlayer(GameServer.GetPlayer(iClientNum));
	if (pPlayer != NULL)
		pPlayer->RemoveFlags(PLAYER_FLAG_HOST);
	CClientConnection *pClient(GameServer.GetHostServer()->GetClient(iClientNum));
	if (pClient != NULL)
		pClient->RemoveFlags(CLIENT_CONNECTION_LOCAL);
	GameServer.ChangeTeam(iClientNum, TEAM_INVALID);
	return true;
}


/*--------------------
  ReplayProfile
  --------------------*/
SERVER_CMD(ReplayProfile)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplayProfile <filename>") << newl;
		return false;
	}

	ReplayManager.Profile(vArgList[0], vArgList.size() > 1 ? AtoI(vArgList[1]) : -1);
	return true;
}


/*--------------------
  ReplayEncode
  --------------------*/
SERVER_CMD(ReplayEncode)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplayEncode <in> <out>") << newl;
		return false;
	}

	ReplayManager.Encode(vArgList[0], vArgList[1]);
	return true;
}


/*--------------------
  ReplayEncode2
  --------------------*/
SERVER_CMD(ReplayEncode2)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplayEncode2 <in> <out>") << newl;
		return false;
	}

	ReplayManager.Encode2(vArgList[0], vArgList[1]);
	return true;
}


/*--------------------
  ReplayParse
  --------------------*/
SERVER_CMD(ReplayParse)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplayParse <filename>") << newl;
		return false;
	}

	uint uiStartTime(K2System.Milliseconds());

	ReplayManager.Parse(vArgList[0]);

	Console << _T("Replay parse took ") << MsToSec(K2System.Milliseconds() - uiStartTime) << _T(" secs") << newl;
	return true;
}


/*--------------------
  ListPlayers
  --------------------*/
SERVER_CMD(ListPlayers)
{
	Console << _CWS(" #       Status  Index    Player Name    Hero Name   Last Input   Ping ") << newl;
	Console << _CWS("-- ------------ ------ -------------- ------------ ------------ ------ ") << newl;
	const PlayerMap &mapPlayers(GameServer.GetPlayerMap());
	for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
	{
		CPlayer *pPlayer(itPlayer->second);

		Console << XtoA(itPlayer->first, FMT_NONE, 2) << SPACE;

		if (pPlayer == NULL)
		{
			Console << XtoA(_CWS("Invalid"), FMT_NONE, 12) << newl;
			continue;
		}

		Console << XtoA(pPlayer->GetIndex(), FMT_NONE, 6) << SPACE;

		if (pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
			Console << XtoA(_CWS("Terminated"), FMT_NONE, 12) << SPACE;
		else if (pPlayer->HasFlags(PLAYER_FLAG_DISCONNECTED))
			Console << XtoA(_CWS("Disconnected"), FMT_NONE, 12) << SPACE;
		else if (pPlayer->HasFlags(PLAYER_FLAG_LOADING))
			Console << XtoA(_CWS("Loading (") + XtoA(pPlayer->GetLoadingProgress() * 100.0f, FMT_PADZERO, 2, 0) + _CWS("%)"), FMT_NONE, 12) << SPACE;
		else
			Console << XtoA(_CWS("Connected"), FMT_NONE, 12) << SPACE;

		Console << XtoA(pPlayer->GetName(), FMT_NONE, 14) << SPACE;

		IHeroEntity *pHero(pPlayer->GetHero());
		Console << XtoA(pHero ? pHero->GetDisplayName() : _CWS("<none>") , FMT_NONE, 14) << SPACE;

		Console << XtoA(Game.GetGameTime() - pPlayer->GetLastInputTime(), FMT_NONE, 12) << SPACE;

		Console << XtoA(pPlayer->GetPing(), FMT_NONE, 6) << SPACE;
	}

	return true;
}


/*--------------------
  Remake
  --------------------*/
SERVER_CMD(Remake)
{
	GameServer.Remake();
	return true;
}


/*--------------------
  Concede
  --------------------*/
SERVER_CMD(Concede)
{
	if (vArgList.empty())
		return false;

	GameServer.Concede(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  BalanceTeams
  --------------------*/
SERVER_CMD(BalanceTeams)
{
	GameServer.BalanceTeams();
	return true;
}


/*--------------------
  SwapPlayerSlots
  --------------------*/
SERVER_CMD(SwapPlayerSlots)
{
	if (vArgList.size() < 4)
		return false;

	int iTeam1(AtoI(vArgList[0]));
	int iSlot1(AtoI(vArgList[1]));
	int iTeam2(AtoI(vArgList[2]));
	int iSlot2(AtoI(vArgList[3]));

	GameServer.SwapPlayerSlots(iTeam1, iSlot1, iTeam2, iSlot2);
	return true;
}


/*--------------------
  LockSlot
  --------------------*/
SERVER_CMD(LockSlot)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iSlot(AtoI(vArgList[1]));

	GameServer.LockSlot(iTeam, iSlot);
	return true;
}


/*--------------------
  UnlockSlot
  --------------------*/
SERVER_CMD(UnlockSlot)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iSlot(AtoI(vArgList[1]));

	GameServer.UnlockSlot(iTeam, iSlot);
	return true;
}


/*--------------------
  ToggleSlotLock
  --------------------*/
SERVER_CMD(ToggleSlotLock)
{
	if (vArgList.size() < 2)
		return false;

	int iTeam(AtoI(vArgList[0]));
	int iSlot(AtoI(vArgList[1]));

	GameServer.ToggleSlotLock(iTeam, iSlot);
	return true;
}


/*--------------------
  cmdReplayRestart
  --------------------*/
SERVER_CMD(ReplayRestart)
{
	ReplayManager.SetPlaybackFrame(0);
	return true;
}


/*--------------------
  cmdReplaySetFrame
  --------------------*/
SERVER_CMD(ReplaySetFrame)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplaySetFrame <frame>") << newl;
		return false;
	}

	ReplayManager.SetPlaybackFrame(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  ReplaySetFrame
  --------------------*/
UI_VOID_CMD(ReplaySetFrame, 1)
{
	cmdReplaySetFrame(vArgList[0]->Evaluate());
}


/*--------------------
  cmdReplayIncFrame
  --------------------*/
SERVER_CMD(ReplayIncFrame)
{
	if (vArgList.size() < 1)
	{
		Console << _T("syntax: ReplayIncFrame <numframes>") << newl;
		return false;
	}

	ReplayManager.SetPlaybackFrame(ReplayManager.GetFrame() + AtoI(vArgList[0]));
	return true;
}


/*--------------------
  ReplayIncFrame
  --------------------*/
UI_VOID_CMD(ReplayIncFrame, 1)
{
	cmdReplayIncFrame(vArgList[0]->Evaluate());
}


/*--------------------
  AddFakePlayer
  --------------------*/
SERVER_CMD(AddFakePlayer)
{
	if (vArgList.empty())
		return false;

	GameServer.AddFakePlayer(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  PrintStatsStatus
  --------------------*/
SERVER_CMD(PrintStatsStatus)
{
	Console << _T("Stats Status: ") << g_aStatsStatusNames[GameServer.GetStatsStatus()] << newl;
	return true;
}
