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
bool    SV_LoadWorld()
{
    return GameServer.LoadWorld();
}


/*====================
  SV_AddClient
  ====================*/
void    SV_AddClient(CClientConnection *pClientConnection)
{
    GameServer.AddClient(pClientConnection);
}


/*====================
  SV_RemoveClient
  ====================*/
void    SV_RemoveClient(int iClientNum)
{
    GameServer.RemoveClient(iClientNum);
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
    return GameServer.GetCurrentGameLength();
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
  SV_UnloadWorld
  ====================*/
void    SV_UnloadWorld()
{
    GameServer.UnloadWorld();
}


/*====================
  InitLibrary
  ====================*/
K2_DLL_EXPORT void InitLibrary(SServerGameLib &GameLib)
{
    GameLib.sName = _T("Savage 2 - Base Game Server");
    GameLib.iMajorVersion = 1;
    GameLib.iMinorVersion = 0;
    GameLib.SetGamePointer = SV_SetGamePointer;
    GameLib.Init = SV_Init;
    GameLib.Frame = SV_Frame;
    GameLib.LoadWorld = SV_LoadWorld;
    GameLib.AddClient = SV_AddClient;
    GameLib.RemoveClient = SV_RemoveClient;
    GameLib.ProcessClientSnapshot = SV_ProcessClientSnapshot;
    GameLib.ProcessGameData = SV_ProcessGameData;
    GameLib.GetSnapshot = SV_GetSnapshot;
    GameLib.Shutdown = SV_Shutdown;
    GameLib.GetCurrentGameLength = SV_GetCurrentGameLength;
    GameLib.ReauthClient = SV_ReauthClient;
    GameLib.StartReplay = SV_StartReplay;
    GameLib.StopReplay = SV_StopReplay;
    GameLib.StateStringChanged = SV_StateStringChanged;
    GameLib.UnloadWorld = SV_UnloadWorld;
}

