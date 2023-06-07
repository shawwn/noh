// (C)2005 S2 Games
// game_client_main.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"
#include "c_gameinterfacemanager.h"

#include "../k2/c_hostclient.h"
#include "../k2/c_clientgamelib.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
map<uint, CGameClient*> g_mapGameClientInstances;
//=============================================================================

/*====================
  CL_SetGamePointer
  ====================*/
void    CL_SetGamePointer(uint uiIndex)
{
    map<uint, CGameClient*>::iterator it(g_mapGameClientInstances.find(uiIndex));
    if (it == g_mapGameClientInstances.end())
        EX_FATAL(_T("CL_SetGamePointer() - Invalid instance index: ") + XtoA(uiIndex));

    g_mapGameClientInstances[uiIndex]->SetGamePointer();
}


/*====================
  CL_Init
  ====================*/
bool    CL_Init(CHostClient *pHostClient)
{
    CGameClient *pNewGameClient(K2_NEW(global,   CGameClient));
    g_mapGameClientInstances[pHostClient->GetIndex()] = pNewGameClient;
    pNewGameClient->SetGamePointer();
    return pNewGameClient->Initialize(pHostClient);
}


/*====================
  CL_LoadWorld
  ====================*/
bool    CL_LoadWorld()
{
    return GameClient.LoadWorld();
}


/*====================
  CL_LoadResources
  ====================*/
bool    CL_LoadResources()
{
    return GameClient.LoadResources();
}


/*====================
  CL_PreFrame
  ====================*/
void    CL_PreFrame()
{
    GameClient.PreFrame();
}


/*====================
  CL_Frame
  ====================*/
void    CL_Frame()
{
    GameClient.Frame();
}


/*====================
  CL_GameData
  ====================*/
bool    CL_GameData(CPacket &pkt)
{
    return GameClient.ProcessGameData(pkt);
}


/*====================
  CL_Shutdown
  ====================*/
void    CL_Shutdown()
{
    uint uiIndex(GameClient.Shutdown());
    map<uint, CGameClient*>::iterator it(g_mapGameClientInstances.find(uiIndex));
    if (it != g_mapGameClientInstances.end())
    {
        SAFE_DELETE(it->second);
        g_mapGameClientInstances.erase(it);
    }
}


/*====================
  CL_ProcessGameEvents
  ====================*/
bool    CL_ProcessGameEvents(CSnapshot &snapshot)
{
    PROFILE("CL_ProcessGameEvents");

    return GameClient.ProcessGameEvents(snapshot);
}


/*====================
  CL_ProcessSnapshot
  ====================*/
bool    CL_ProcessSnapshot(CSnapshot &snapshot)
{
    PROFILE("CL_ProcessSnapshot");

    return GameClient.ProcessSnapshot(snapshot);
}


/*====================
  CL_Reinitialize
  ====================*/
void    CL_Reinitialize()
{
    GameClient.Reinitialize();
}


/*====================
  CL_LoadAllResources
  ====================*/
void    CL_LoadAllResources()
{
    GameClient.PrecacheAll();
}


/*====================
  CL_Connect
  ====================*/
void    CL_Connect(const tstring &sAddr)
{
    GameClient.Connect(sAddr);
}


/*====================
  InitLibrary
  ====================*/
K2_DLL_EXPORT void InitLibrary(CClientGameLib &GameLib)
{
    GameLib.SetName(_T("Savage 2 - Base Game Client"));
    GameLib.SetMajorVersion(1);
    GameLib.SetMinorVersion(0);
    GameLib.AssignSetGamePointerFn(CL_SetGamePointer);
    GameLib.AssignInitFn(CL_Init);
    GameLib.AssignLoadWorldFn(CL_LoadWorld);
    GameLib.AssignLoadResourcesFn(CL_LoadResources);
    GameLib.AssignPreFrameFn(CL_PreFrame);
    GameLib.AssignFrameFn(CL_Frame);
    GameLib.AssignGameDataFn(CL_GameData);
    GameLib.AssignShutdownFn(CL_Shutdown);
    GameLib.AssignGameEventFn(CL_ProcessGameEvents);
    GameLib.AssignSnapshotFn(CL_ProcessSnapshot);
    GameLib.AssignReinitializeFn(CL_Reinitialize);
    GameLib.AssignLoadAllResourcesFn(CL_LoadAllResources);
    GameLib.AssignConnectFn(CL_Connect);
}
