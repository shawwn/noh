// (C)2005 S2 Games
// game_server_main.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gameserver.h"

#include "../k2/c_hostserver.h"
//=============================================================================

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
    if (CompareNoCase(sType, _T("GetGameOptions")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;

        return pGameInfo->GetGameOptions();
    }
    else if (CompareNoCase(sType, _T("GetGameMode")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;

        return pGameInfo->GetGameMode();
    }
    else if (CompareNoCase(sType, _T("GetTeamSize")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;

        return pGameInfo->GetTeamSize();
    }
    else if (CompareNoCase(sType, _T("GetAllHeroes")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_ALL_HEROES))
            return 1;
        else
            return 0;
    }   
    else if (CompareNoCase(sType, _T("GetEasyMode")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_EASY_MODE))
            return 1;
        else
            return 0;
    }   
    else if (CompareNoCase(sType, _T("GetForceRandom")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_FORCE_RANDOM))
            return 1;
        else
            return 0;
    }   
    else if (CompareNoCase(sType, _T("GetAutoBalanced")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_AUTOBALANCE_TEAMS))
            return 1;
        else
            return 0;
    }
    else if (CompareNoCase(sType, _T("GetHardcore")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_HARDCORE))
            return 1;
        else
            return 0;
    }
    else if (CompareNoCase(sType, _T("GetDevHeroes")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return 0;
    
        if (pGameInfo->HasGameOptions(GAME_OPTION_DEV_HEROES))
            return 1;
        else
            return 0;
    }
    else if (CompareNoCase(sType, _T("GetAdvancedOptions")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
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
    if (CompareNoCase(sType, _T("GetGameName")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return TSNULL;

        return pGameInfo->GetGameName();
    }
    else if (CompareNoCase(sType, _T("GetGameModeName")) == 0)
    {
        CGameInfo *pGameInfo(GameServer.GetGameInfo());
                
        if (pGameInfo == NULL)
            return TSNULL;

        return pGameInfo->GetGameModeName(pGameInfo->GetGameMode());
    }
    else
    {
        return TSNULL;
    }
}


/*====================
  InitLibrary
  ====================*/
#ifdef __GNUC__
extern "C" void __attribute__ ((visibility("default")))
#elif defined(WIN32)
extern "C" __declspec(dllexport)
#endif
void InitLibrary(CServerGameLib &GameLib)
{
    GameLib.SetName(_T("Heroes of Newerth - Game Server"));
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
#endif
    GameLib.AssignLongServerFrameFn(SV_LongServerFrame);
    GameLib.AssignResetFn(SV_Reset);
    GameLib.AssignEndFrameFn(SV_EndFrame);
    GameLib.AssignClientStateChangeFn(SV_ClientStateChange);
    GameLib.AssignGetGameInfoIntFn(SV_GetGameInfoInt);
    GameLib.AssignGetGameInfoStringFn(SV_GetGameInfoString);
}

