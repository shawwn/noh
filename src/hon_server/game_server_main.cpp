// (C)2005 S2 Games
// game_server_main.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"

#include "../hon_shared/i_heroentity.h"

#include "../k2/c_hostserver.h"
//=============================================================================

/*====================
  SV_GetTypeName
  ====================*/
const tstring&  SV_GetTypeName()
{
    return GameServer.GetTypeName();
}


/*====================
  SV_SetGamePointer
  ====================*/
void    SV_SetGamePointer()
{
    GameServer.SetGamePointer();
}


/*====================
  SV_Init
  ====================*/
bool    SV_Init(CHostServer *pHostServer)
{
    return GameServer.Initialize(pHostServer);
}


/*====================
  SV_Frame
  ====================*/
void    SV_Frame()
{
    GameServer.Frame();
}


/*====================
  SV_Shutdown
  ====================*/
void    SV_Shutdown()
{
    GameServer.Shutdown();
    GameServer.Release();
}


/*====================
  SV_LoadWorld
  ====================*/
bool    SV_LoadWorld(const tstring &sName, const tstring &sGameMode)
{
    return GameServer.LoadWorld(sName, sGameMode);
}


/*====================
  SV_AddClient
  ====================*/
bool    SV_AddClient(CClientConnection *pClientConnection)
{
    return GameServer.AddClient(pClientConnection);
}


/*====================
  SV_RemoveClient
  ====================*/
void    SV_RemoveClient(int iClientNum, const tstring &sReason)
{
    GameServer.RemoveClient(iClientNum, sReason);
}

/*====================
  SV_ClientTimingOut
  ====================*/
void    SV_ClientTimingOut(int iClientNum)
{
    GameServer.ClientTimingOut(iClientNum);
}


/*====================
  SV_GetMaxClients
  ====================*/
uint    SV_GetMaxClients()
{
    return GameServer.GetMaxClients();
}


/*====================
  SV_ProcessClientSnapshot
  ====================*/
uint    SV_ProcessClientSnapshot(int iClientNum, CClientSnapshot &snapshot)
{
    return GameServer.ProcessClientSnapshot(iClientNum, snapshot);
}


/*====================
  SV_ProcessGameData
  ====================*/
bool    SV_ProcessGameData(int iClientNum, CPacket &pkt)
{
    return GameServer.ProcessGameData(iClientNum, pkt);
}


/*====================
  SV_GetSnapshot
  ====================*/
void    SV_GetSnapshot(CSnapshot &snapshot)
{
    GameServer.GetSnapshot(snapshot);
}


/*====================
  SV_GetCurrentGameLength
  ====================*/
uint    SV_GetCurrentGameLength()
{
    return GameServer.GetMatchTime();
}


/*====================
  SV_ReauthClient
  ====================*/
void    SV_ReauthClient(CClientConnection *pClientConnection)
{
    GameServer.ReauthClient(pClientConnection);
}


/*====================
  SV_StartReplay
  ====================*/
bool    SV_StartReplay(const tstring &sFilename)
{
    return GameServer.StartReplay(sFilename);
}


/*====================
  SV_StopReplay
  ====================*/
void    SV_StopReplay()
{
    return GameServer.StopReplay();
}


/*====================
  SV_StateStringChanged
  ====================*/
void    SV_StateStringChanged(uint uiID, const CStateString &ss)
{
    GameServer.StateStringChanged(uiID, ss);
}


/*====================
  SV_StateBlockChanged
  ====================*/
void    SV_StateBlockChanged(uint uiID, const IBuffer &buffer)
{
    GameServer.StateBlockChanged(uiID, buffer);
}


/*====================
  SV_UnloadWorld
  ====================*/
void    SV_UnloadWorld()
{
    GameServer.UnloadWorld();
}


/*====================
  SV_GetEntity
  ====================*/
void*   SV_GetEntity(uint uiIndex)
{
    return GameServer.GetEntity(uiIndex);
}


/*====================
  SV_GetServerInfo
  ====================*/
void    SV_GetServerInfo(CPacket &pkt)
{
    GameServer.GetServerInfo(pkt);
}


/*====================
  SV_GetReconnectInfo
  ====================*/
void    SV_GetReconnectInfo(CPacket &pkt, uint uiMatchID, uint uiAccountID, ushort unConnectionID)
{
    GameServer.GetReconnectInfo(pkt, uiMatchID, uiAccountID, unConnectionID);
}


/*====================
  SV_IsPlayerReconnecting
  ====================*/
bool    SV_IsPlayerReconnecting(int iAccountID)
{
    return GameServer.IsPlayerReconnecting(iAccountID);
}


/*====================
  SV_IsDuplicateAccountInGame
  ====================*/
bool    SV_IsDuplicateAccountInGame(int iAccountID)
{
    return GameServer.IsDuplicateAccountInGame(iAccountID);
}


/*====================
  SV_RemoveDuplicateAccountsInGame
  ====================*/
bool    SV_RemoveDuplicateAccountsInGame(int iAccountID)
{
    return GameServer.RemoveDuplicateAccountsInGame(iAccountID);
}


/*====================
  SV_GetGameStatus
  ====================*/
void    SV_GetGameStatus(CPacket &pkt)
{
    GameServer.GetGameStatus(pkt);
}


#ifndef K2_CLIENT
/*====================
  SV_GetHeartbeatInfo
  ====================*/
void    SV_GetHeartbeatInfo(CHTTPRequest *pHeartbeat)
{
    GameServer.GetHeartbeatInfo(pHeartbeat);
}
#endif


#ifndef K2_CLIENT
/*====================
  SV_ProcessAuthData
  ====================*/
void    SV_ProcessAuthData(int iAccountID, const CPHPData *pData)
{
    GameServer.ProcessAuthData(iAccountID, pData);
}
#endif


#ifndef K2_CLIENT
/*====================
  SV_ProcessAuxData
  ====================*/
void    SV_ProcessAuxData(int iClientNum, const CPHPData *pData)
{
    GameServer.ProcessAuxData(iClientNum, pData);
}
#endif

/*====================
  SV_LongServerFrame
  ====================*/
void    SV_LongServerFrame(uint uiFrameLength)
{
    GameServer.LongServerFrame(uiFrameLength);
}


/*====================
  SV_Reset
  ====================*/
void    SV_Reset()
{
    GameServer.Reset();
}


/*====================
  SV_EndFrame
  ====================*/
void    SV_EndFrame(PoolHandle hSnapshot)
{
    GameServer.EndFrame(hSnapshot);
}


/*====================
  SV_ClientStateChange
  ====================*/
void    SV_ClientStateChange(int iClientNum, EClientConnectionState eState)
{
    GameServer.ClientStateChange(iClientNum, eState);
}


/*====================
  SV_GetGameInfoInt
  ====================*/
uint    SV_GetGameInfoInt(const tstring &sType)
{
    if (sType == _T("GetGameOptions"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;

        return pGameInfo->GetGameOptions();
    }
    else if (sType == _T("GetGameMode"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;

        return pGameInfo->GetGameMode();
    }
    else if (sType == _T("GetTeamSize"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;

        return pGameInfo->GetTeamSize();
    }
    else if (sType == _T("GetAllHeroes"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_ALL_HEROES))
            return 1;
        else
            return 0;
    }   
    else if (sType == _T("GetEasyMode"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_EASY_MODE))
            return 1;
        else
            return 0;
    }   
    else if (sType == _T("GetCasualMode"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_CASUAL))
            return 1;
        else
            return 0;
    }   
    else if (sType == _T("GetForceRandom"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_FORCE_RANDOM))
            return 1;
        else
            return 0;
    }   
    else if (sType == _T("GetAutoBalanced"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
            return 1;
        else
            return 0;
    }
    else if (sType == _T("GetHardcore"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_HARDCORE))
            return 1;
        else
            return 0;
    }
    else if (sType == _T("GetDevHeroes"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_DEV_HEROES))
            return 1;
        else
            return 0;
    }
    else if (sType == _T("GetAdvancedOptions"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
                
        if (pGameInfo->HasGameOptions(GAME_OPTION_ALTERNATE_SELECTION) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_REPICK) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_SWAP) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_AGILITY) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_INTELLIGENCE) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_STRENGTH) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_RESPAWN_TIMER) ||
            pGameInfo->HasGameOptions(GAME_OPTION_DROP_ITEMS) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_POWERUPS) ||
            pGameInfo->HasGameOptions(GAME_OPTION_SUPER_CREEPS) ||
            pGameInfo->HasGameOptions(GAME_OPTION_DUPLICATE_HEROES) ||
            pGameInfo->HasGameOptions(GAME_OPTION_REVERSE_SELECTION) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_TOP_LANE) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_MIDDLE_LANE) ||
            pGameInfo->HasGameOptions(GAME_OPTION_NO_BOTTOM_LANE) ||
            pGameInfo->HasGameOptions(GAME_OPTION_ALLOW_VETO) ||
            pGameInfo->HasGameOptions(GAME_OPTION_SHUFFLE_TEAMS) ||
            pGameInfo->HasGameOptions(GAME_OPTION_TOURNAMENT_RULES))
            return 1;
        else
            return 0;
    }
    else if (sType == _T("GetCurrentGameTime"))
    {
        return GameServer.GetMatchTime();
    }
    else if (sType == _T("GetCurrentGamePhase"))
    {
        return GameServer.GetGamePhase();
    }
    else if (sType == _T("GetMatchID"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return 0;
    
        return pGameInfo->GetMatchID();
    }
    else
    {
        return 0;
    }
}


/*====================
  SV_GetGameInfoString
  ====================*/
tstring SV_GetGameInfoString(const tstring &sType)
{
    if (sType == _T("GetGameName"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return TSNULL;

        return pGameInfo->GetGameName();
    }
    else if (sType == _T("GetGameModeName"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == nullptr)
            return TSNULL;

        return pGameInfo->GetGameModeName(pGameInfo->GetGameMode());
    }
    else if (sType == _T("IsCasual"))
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
        
        if (pGameInfo == nullptr)
            return _T("0");

        if (pGameInfo->HasGameOptions(GAME_OPTION_CASUAL))
            return _T("1");
        else
            return _T("0");
    }
    else if (sType.substr(0, 11) == _T("GetTeamInfo"))
    {
        CTeamInfo *pTeam(nullptr);

        if (sType.substr(11, 1) == _T("1"))
            pTeam = GameServer.GetTeam(TEAM_1);
        else if (sType.substr(11, 1) == _T("2"))
            pTeam = GameServer.GetTeam(TEAM_2);

        if (pTeam == nullptr)
            return TSNULL;

        tstring sTeamInfo;

        sTeamInfo += XtoA(sType.substr(11, 1));                                                 // Team
        sTeamInfo += XtoA(_T("|") + XtoA(pTeam->GetWinChance(), FMT_NONE, 3, 2));               // Win Chance
        sTeamInfo += XtoA(_T("|") +  XtoA(pTeam->GetStartingTowerCount()));                     // Starting Tower Count
        sTeamInfo += XtoA(_T("|") +  XtoA(pTeam->GetCurrentTowerCount()));                      // Current Tower Count
        sTeamInfo += XtoA(_T("|") +  XtoA(pTeam->GetCurrentMeleeCount()));                      // Current Melee Rax Count
        sTeamInfo += XtoA(_T("|") +  XtoA(pTeam->GetCurrentRangedCount()));                     // Current Ranged Rax Count
        sTeamInfo += XtoA(_T("|") +  XtoA(pTeam->GetBaseHealthPercent(), FMT_NONE, 3, 2));      // Base Health Percent

        return sTeamInfo;
    }   
    else if (sType.substr(0, 13) == _T("GetPlayerInfo"))
    {
        const uint uiPlayerIndex(AtoI(sType.substr(13, 1)));

        CTeamInfo *pTeam(nullptr);

        if (uiPlayerIndex < 5)
            pTeam = GameServer.GetTeam(TEAM_1);
        else
            pTeam = GameServer.GetTeam(TEAM_2);

        if (pTeam == nullptr)
            return TSNULL;

        CPlayer *pPlayer(nullptr);

        if (pTeam->GetTeamID() == TEAM_1)
            pPlayer = GameServer.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayerIndex));
        else if (pTeam->GetTeamID() == TEAM_2)
            pPlayer = GameServer.GetPlayer(pTeam->GetClientIDFromTeamIndex(uiPlayerIndex - 5));

        if (pPlayer == nullptr)
            return TSNULL;

        IHeroEntity *pHero(pPlayer->GetHero());
        CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pPlayer->GetSelectedHero()));

        if (pHero == nullptr || pHeroDef == nullptr)
            return TSNULL;

        tstring sPlayerInfo;
        
        sPlayerInfo += XtoA(pPlayer->GetAccountID());                                   // The Players AccountID
        sPlayerInfo += XtoA(_T("|") + XtoA(pPlayer->GetName()));                        // Player Name
        sPlayerInfo += XtoA(_T("|") +  XtoA(pHeroDef->GetIconPath(0)));                 // Hero icon path
        sPlayerInfo += XtoA(_T("|") +  XtoA(pHero->GetLevel()));                        // Player Level
        sPlayerInfo += XtoA(_T("|") +  XtoA(pPlayer->GetStat(PLAYER_STAT_HERO_KILLS))); // Player Kills
        sPlayerInfo += XtoA(_T("|") +  XtoA(pPlayer->GetStat(PLAYER_STAT_DEATHS)));     // Player Deaths
        sPlayerInfo += XtoA(_T("|") +  XtoA(pPlayer->GetStat(PLAYER_STAT_ASSISTS)));    // Player Assists

        return sPlayerInfo;
    }
    else
    {
        return TSNULL;
    }
}


/*====================
  SV_UpdateUpgrades
  ====================*/
void    SV_UpdateUpgrades(int iClientNum)
{
    GameServer.UpdateUpgrades(iClientNum);
}


/*====================
  InitLibrary
  ====================*/
K2_DLL_EXPORT void InitLibrary(CServerGameLib &GameLib)
{
    GameLib.SetName(K2System.GetGameName() + _T(" - Game Server"));
    GameLib.SetTypeName(_T("honserver"));
    GameLib.SetVersion(1, 0);
    GameLib.AssignSetGamePointerFn(SV_SetGamePointer);
    GameLib.AssignInitFn(SV_Init);
    GameLib.AssignFrameFn(SV_Frame);
    GameLib.AssignLoadWorldFn(SV_LoadWorld);
    GameLib.AssignAddClientFn(SV_AddClient);
    GameLib.AssignRemoveClientFn(SV_RemoveClient);
    GameLib.AssignClientTimingOutFn(SV_ClientTimingOut);
    GameLib.AssignGetMaxClientsFn(SV_GetMaxClients);
    GameLib.AssignProcessClientSnapshotFn(SV_ProcessClientSnapshot);
    GameLib.AssignProcessGameDataFn(SV_ProcessGameData);
    GameLib.AssignGetSnapshotFn(SV_GetSnapshot);
    GameLib.AssignShutdownFn(SV_Shutdown);
    GameLib.AssignGetMatchTimeFn(SV_GetCurrentGameLength);
    GameLib.AssignReauthClientFn(SV_ReauthClient);
    GameLib.AssignStartReplayFn(SV_StartReplay);
    GameLib.AssignStopReplayFn(SV_StopReplay);
    GameLib.AssignStateStringChangedFn(SV_StateStringChanged);
    GameLib.AssignStateBlockChangedFn(SV_StateBlockChanged);
    GameLib.AssignUnloadWorldFn(SV_UnloadWorld);
    GameLib.AssignGetEntityFn(SV_GetEntity);
    GameLib.AssignGetServerInfoFn(SV_GetServerInfo);
    GameLib.AssignGetReconnectInfoFn(SV_GetReconnectInfo);
    GameLib.AssignIsPlayerReconnectingFn(SV_IsPlayerReconnecting);
    GameLib.AssignIsDuplicateAccountInGameFn(SV_IsDuplicateAccountInGame);
    GameLib.AssignRemoveDuplicateAccountsInGameFn(SV_RemoveDuplicateAccountsInGame);
    GameLib.AssignGetGameStatusFn(SV_GetGameStatus);
#ifndef K2_CLIENT
    GameLib.AssignGetHeartbeatInfoFn(SV_GetHeartbeatInfo);
    GameLib.AssignProcessAuthDataFn(SV_ProcessAuthData);
    GameLib.AssignProcessAuxDataFn(SV_ProcessAuxData);
#endif
    GameLib.AssignLongServerFrameFn(SV_LongServerFrame);
    GameLib.AssignResetFn(SV_Reset);
    GameLib.AssignEndFrameFn(SV_EndFrame);
    GameLib.AssignClientStateChangeFn(SV_ClientStateChange);
    GameLib.AssignGetGameInfoIntFn(SV_GetGameInfoInt);
    GameLib.AssignGetGameInfoStringFn(SV_GetGameInfoString);
    GameLib.AssignUpdateUpgradesFn(SV_UpdateUpgrades);
}

