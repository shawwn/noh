// (C)2006 S2 Games
// c_game_server_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"

#include "../hon_shared/c_teamdefinition.h"
#include "../hon_shared/c_entityclientinfo.h"
#include "../hon_shared/c_replaymanager.h"
//=============================================================================

/*--------------------
  Damage
  --------------------*/
SERVER_CMD(Damage)
{
    int iClientNum(0);
    if (vArgList.size() > 1)
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    float fDamage(25.0f);
    if (vArgList.size() == 1)
        fDamage = MAX(AtoF(vArgList[0]), 0.0f);
    else if (vArgList.size() > 1)
        fDamage = MAX(AtoF(vArgList[1]), 0.0f);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget != nullptr)
        pTarget->Damage(fDamage, 0, nullptr, INVALID_ENT_TYPE);

    return true;
}


/*--------------------
  Armageddon
  --------------------*/
SERVER_CMD(Armageddon)
{
    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt)
    {
        if (pEnt->IsNpc())
            pEnt->Kill();

        pEnt = Game.GetNextEntity(pEnt);
    }

    return true;
}


/*--------------------
  GiveExp
  --------------------*/
SERVER_CMD(GiveExp)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    float fExperience(25.0f);
    if (vArgList.size() > 1)
        fExperience = MAX(AtoF(vArgList[1]), 0.0f);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget == nullptr)
    {
        Console << _T("Could not find client: ") << iClientNum << newl;
        return false;
    }

    pTarget->GiveExperience(fExperience, pTarget->GetPosition() + pTarget->GetBounds().GetMid());

    return true;
}


/*--------------------
  ResetExp
  --------------------*/
SERVER_CMD(ResetExp)
{   
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
    if (pClient == nullptr)
    {
        Console << _T("Could not find client: ") << iClientNum << newl;
        return false;
    }

    pClient->ResetExperience();
    return true;
}


/*--------------------
  GiveGold
  --------------------*/
SERVER_CMD(GiveGold)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    ushort unGold(25);
    if (vArgList.size() > 1)
        unGold = MAX(AtoI(vArgList[1]), 0);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget)
        pTarget->GiveGold(unGold, pTarget->GetPosition() + pTarget->GetBounds().GetMid());

    return true;
}


/*--------------------
  GiveAmmo
  --------------------*/
SERVER_CMD(GiveAmmo)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    ushort unGold(25);
    if (vArgList.size() > 1)
        unGold = MAX(AtoI(vArgList[1]), 0);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget)
    {
        // Replenish ammo and check for availability of a weapon
        for (int i(0); i < INVENTORY_START_BACKPACK; ++i)
        {
            IInventoryItem *pItem(pTarget->GetItem(i));

            if (!pItem)
                continue;

            pItem->SetAmmo(pItem->GetAdjustedAmmoCount());
        }
    }
        
    return true;
}


/*--------------------
  UnlockItems
  --------------------*/
SERVER_CMD(UnlockItems)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    ushort unGold(25);
    if (vArgList.size() > 1)
        unGold = MAX(AtoI(vArgList[1]), 0);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget)
    {
        for (int i(0); i < MAX_INVENTORY; ++i)
        {
            IInventoryItem *pItem(pTarget->GetItem(i));

            if (!pItem)
                continue;

            pItem->Enable();
            pItem->ActivatePassive();
        }
    }
        
    return true;
}


/*--------------------
  Refresh
  --------------------*/
SERVER_CMD(Refresh)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget)
    {
        pTarget->SetHealth(pTarget->GetMaxHealth());
        pTarget->SetMana(pTarget->GetMaxMana());
        pTarget->SetStamina(pTarget->GetMaxStamina());
        pTarget->ResetDash();

        for (int i(0); i < MAX_INVENTORY; ++i)
        {
            IInventoryItem *pItem(pTarget->GetItem(i));

            if (!pItem)
                continue;

            pItem->SetCooldownTimer(INVALID_TIME, INVALID_TIME);
            pItem->SetAmmo(pItem->GetAdjustedAmmoCount());
        }
    }
        
    return true;
}


/*--------------------
  GiveRandomItem
  --------------------*/
SERVER_CMD(GiveRandomItem)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    GameServer.GiveItem(iClientNum, EntityRegistry.LookupName(GameServer.GetRandomItem()));
            
    return true;
}


/*--------------------
  GiveClientItem
  --------------------*/
SERVER_CMD(GiveClientItem)
{
    if (vArgList.size() < 2)
        return false;

    int iClientNum(GameServer.GetClientNumFromName(vArgList[0]));

    if (iClientNum == -1)
        iClientNum = AtoI(vArgList[0]);

    GameServer.GiveItem(iClientNum, vArgList[1]);
            
    return true;
}


/*--------------------
  GiveSoul
  --------------------*/
SERVER_CMD(GiveSoul)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    CEntityClientInfo *pClient(GameServer.GetClientInfo(iClientNum));
    if (pClient != nullptr)
        pClient->AddSoul();

    return true;
}


/*--------------------
  GiveTeamGold
  --------------------*/
SERVER_CMD(GiveTeamGold)
{
    int iTeam(0);
    if (!vArgList.empty())
        iTeam = AtoI(vArgList[0]);

    ushort unGold(25);
    if (vArgList.size() > 1)
        unGold = MAX(AtoI(vArgList[1]), 0);

    CTeamInfo *pTeamInfo(GameServer.GetTeam(iTeam));
    if (pTeamInfo)
        pTeamInfo->GiveGold(unGold);

    return true;
}


/*--------------------
  Stun
  --------------------*/
SERVER_CMD(Stun)
{
    int iClientNum(0);
    if (!vArgList.empty())
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    uint uiDuration(2000);
    if (vArgList.size() > 1)
        uiDuration = MAX(AtoI(vArgList[1]), 0);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget == nullptr)
    {
        Console << _T("Could not find client: ") << iClientNum << newl;
        return false;
    }

    pTarget->Stun(GameServer.GetGameTime() + uiDuration);

    return true;
}


/*--------------------
  ApplyState
  --------------------*/
SERVER_CMD(ApplyState)
{
    if (vArgList.empty())
    {
        Console << _T("No state specified") << newl;
        return false;
    }

    const tstring sState(vArgList[0]);

    int iClientNum(0);
    if (vArgList.size() > 1)
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[1]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[1]);
    }

    uint uiDuration(10000);
    if (vArgList.size() > 2)
        uiDuration = MAX(AtoI(vArgList[2]), 0);

    IPlayerEntity *pTarget(GameServer.GetPlayerEntityFromClientID(iClientNum));
    if (pTarget != nullptr)
        pTarget->ApplyState(EntityRegistry.LookupID(sState), GameServer.GetGameTime(), uiDuration);

    return true;
}


/*--------------------
  MakeOfficer
  --------------------*/
SERVER_CMD(MakeOfficer)
{   
    int iClientNum(0);
    if (vArgList.size() > 0)
    {
        iClientNum = GameServer.GetClientNumFromName(vArgList[0]);

        if (iClientNum == -1)
            iClientNum = AtoI(vArgList[0]);
    }

    CEntityClientInfo *pTargetClient(GameServer.GetClientInfo(iClientNum));
    if (pTargetClient == nullptr)
    {
        Console << _T("Could not find client: ") << iClientNum << newl;
        return false;
    }

    byte ySquad(pTargetClient->GetSquad());
    if (vArgList.size() > 1)
        ySquad = AtoI(vArgList[1]);
    if (ySquad == INVALID_SQUAD)
        ySquad = 0;

    pTargetClient->SetSquad(ySquad);
    pTargetClient->SetFlags(CLIENT_INFO_IS_OFFICER);
    CTeamInfo *pTeam(GameServer.GetTeam(pTargetClient->GetTeam()));
    if (pTeam != nullptr)
        pTeam->SortClientList();

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
    buffer << GAME_CMD_SERVERCHAT_ALL << ConcatinateArgs(vArgList, _T(" ")) << byte(0);
    GameServer.BroadcastGameData(buffer, true);

    Console.Server << _T("Server Message: ") << ConcatinateArgs(vArgList, _T(" ")) << newl;
            
    return true;
}


/*--------------------
  StartGame
  --------------------*/
SERVER_CMD(StartGame)
{
    GameServer.StartGame();

    return true;
}


/*--------------------
  EndGame
  --------------------*/
SERVER_CMD(EndGame)
{
    if (vArgList.empty())
        return false;

    int iLosingTeam(AtoI(vArgList[0]));
    GameServer.EndGame(iLosingTeam);

    return true;
}


/*--------------------
  NextPhase
  --------------------*/
SERVER_CMD(NextPhase)
{
    uint uiLength(SecToMs(30u));
    if (!vArgList.empty())
        uiLength = AtoI(vArgList[0]);

    int iPhase(GameServer.GetGamePhase() + 1);
    if (iPhase >= NUM_GAME_PHASES)
        iPhase = 0;

    GameServer.SetGamePhase(EGamePhase(iPhase), uiLength);

    return true;
}


/*--------------------
  PrevPhase
  --------------------*/
SERVER_CMD(PrevPhase)
{
    uint uiLength(SecToMs(30u));
    if (!vArgList.empty())
        uiLength = AtoI(vArgList[0]);

    int iPhase(GameServer.GetGamePhase() - 1);
    if (iPhase < 0)
        iPhase = 0;

    GameServer.SetGamePhase(EGamePhase(iPhase), uiLength);

    return true;
}


/*--------------------
  SetRace
  --------------------*/
SERVER_CMD(SetRace)
{
    if (vArgList.size() < 2)
    {
        Console << _T("Syntax: SetRace <team number> <race name>") << newl;
        return false;
    }

    int iTeam(AtoI(vArgList[0]));
    if (iTeam < 1 || iTeam > 2)
    {
        Console << _T("Invalid team") << newl;
        return false;
    }

    GameServer.SetRace(iTeam, vArgList[1]);

    return true;
}


/*--------------------
  SetMaxTeams
  --------------------*/
SERVER_CMD(SetMaxTeams)
{
    if (vArgList.size() < 1)
    {
        Console << _T("Syntax: SetMaxTeams <value>") << newl;
        return false;
    }

    int iMax(AtoI(vArgList[0]) + 1);

    while (GameServer.GetNumTeams() > iMax)
        GameServer.RemoveTeam(GameServer.GetNumTeams() - 1);

    while (GameServer.GetNumTeams() < iMax)
        GameServer.AddTeam(_T("Team ") + XtoA(GameServer.GetNumTeams()), g_teamdefHuman);

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
  Kick
  --------------------*/
SERVER_CMD(Kick)
{
    if (vArgList.empty())
    {
        Console << _T("Must specify client to be kicked") << newl;
        return false;
    }

    int iClientNum(GameServer.GetClientNumFromName(vArgList[0]));

    if (iClientNum == -1)
        iClientNum = AtoI(vArgList[0]);

    tstring sReason(_T("No reason given"));
    if (vArgList.size() > 1)
        sReason = vArgList[1];

    GameServer.Kick(iClientNum, sReason);
    return true;
}


/*--------------------
  Ban
  --------------------*/
SERVER_CMD(Ban)
{
    if (vArgList.empty())
    {
        Console << _T("Must specify client to be banned") << newl;
        return false;
    }

    int iClientNum(GameServer.GetClientNumFromName(vArgList[0]));

    if (iClientNum == -1)
        iClientNum = AtoI(vArgList[0]);

    int iTime(0);
    if (vArgList.size() > 1)
        iTime = AtoI(vArgList[1]);

    tstring sReason(_T("No reason given"));
    if (vArgList.size() > 2)
        sReason = vArgList[2];

    GameServer.Ban(iClientNum, iTime, sReason);
    return true;
}


/*--------------------
  SetDemoAccount
  --------------------*/
SERVER_CMD(SetDemoAccount)
{
    if (vArgList.size() < 2)
        return false;

    CEntityClientInfo *pClient(Game.GetClientInfo(AtoI(vArgList[0])));

    if (pClient == nullptr)
        return false;

    if (AtoB(vArgList[1]))
        pClient->SetDemoTimeRemaining(vArgList.size() > 2 ? AtoI(vArgList[2]) : 18000000);
    else
        pClient->SetDemoTimeRemaining(INVALID_TIME);

    return true;
}


/*--------------------
  ReplayRecordStart
  --------------------*/
SERVER_CMD(ReplayRecordStart)
{
    if (vArgList.size() < 1)
        Console << "syntax: ReplayRecordStart <filename>" << newl;

    ReplayManager.StartRecording(_TS("~/replays/") + vArgList[0]);

    Console << "Recording " << vArgList[0] << newl;
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

