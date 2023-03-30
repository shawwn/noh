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
  CL_GetTypeName
  ====================*/
const tstring&  CL_GetTypeName(uint uiIndex)
{
    map<uint, CGameClient*>::iterator it(g_mapGameClientInstances.find(uiIndex));
    if (it == g_mapGameClientInstances.end())
        EX_FATAL(_T("CL_GetTypeName() - Invalid instance index: ") + XtoA(uiIndex));

    return g_mapGameClientInstances[uiIndex]->GetTypeName();
}


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
    CGameClient *pNewGameClient(K2_NEW(ctx_Singleton,   CGameClient));
    g_mapGameClientInstances[pHostClient->GetIndex()] = pNewGameClient;
    pNewGameClient->SetGamePointer();
    return pNewGameClient->Initialize(pHostClient);
}


/*====================
  CL_StartLoadingWorld
  ====================*/
void    CL_StartLoadingWorld()
{
    GameClient.StartLoadingWorld();
}


/*====================
  CL_StartPreloadingWorld
  ====================*/
void    CL_StartPreloadingWorld()
{
    GameClient.StartPreloadingWorld();
}



/*====================
  CL_FinishedLoadingWorld
  ====================*/
void    CL_FinishedLoadingWorld()
{
    GameClient.FinishedLoadingWorld();
}


/*====================
  CL_SpawnNextWorldEntity
  ====================*/
void    CL_SpawnNextWorldEntity()
{
    GameClient.SpawnNextWorldEntity();
}


/*====================
  CL_PrecacheNextWorldEntity
  ====================*/
void    CL_PrecacheNextWorldEntity()
{
    GameClient.PrecacheNextWorldEntity();
}


/*====================
  CL_IsSpawningEntities
  ====================*/
bool    CL_IsSpawningEntities()
{
    return GameClient.IsSpawningEntities();
}


/*====================
  CL_IsFinishedSpawningEntities
  ====================*/
bool    CL_IsFinishedSpawningEntities()
{
    return GameClient.IsFinishedSpawningEntities();
}


/*====================
  CL_GetEntitySpawningProgress
  ====================*/
float   CL_GetEntitySpawningProgress()
{
    return GameClient.GetEntitySpawningProgress();
}


/*====================
  CL_StartLoadingResources
  ====================*/
void    CL_StartLoadingResources()
{
    GameClient.StartLoadingResources();
}


/*====================
  CL_LoadNextResource
  ====================*/
void    CL_LoadNextResource()
{
    GameClient.LoadNextResource();
}


/*====================
  CL_IsFinishedLoadingResources
  ====================*/
bool    CL_IsFinishedLoadingResources()
{
    return GameClient.IsFinishedLoadingResources();
}


/*====================
  CL_GetResourceLoadingProgress
  ====================*/
float   CL_GetResourceLoadingProgress()
{
    return GameClient.GetResourceLoadingProgress();
}


/*====================
  CL_StartLoadingEntityResources
  ====================*/
void    CL_StartLoadingEntityResources()
{
    GameClient.StartLoadingEntityResources();
}


/*====================
  CL_LoadNextEntityResource
  ====================*/
void    CL_LoadNextEntityResource()
{
    GameClient.LoadNextEntityResource();
}


/*====================
  CL_IsFinishedLoadingEntityResources
  ====================*/
bool    CL_IsFinishedLoadingEntityResources()
{
    return GameClient.IsFinishedLoadingEntityResources();
}


/*====================
  CL_GetEntityResourceLoadingProgress
  ====================*/
float   CL_GetEntityResourceLoadingProgress()
{
    return GameClient.GetEntityResourceLoadingProgress();
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
  CL_StateStringChanged
  ====================*/
void    CL_StateStringChanged(uint uiID, const CStateString &ss)
{
    GameClient.StateStringChanged(uiID, ss);
}


/*====================
  CL_StateBlockChanged
  ====================*/
void    CL_StateBlockChanged(uint uiID, const CStateBlock &block)
{
    GameClient.StateBlockChanged(uiID, block);
}


/*====================
  CL_SendCreateGameRequest
  ====================*/
void    CL_SendCreateGameRequest(const tstring &sName, const tstring &sSettings)
{
    GameClient.SendCreateGameRequest(sName, sSettings);
}


/*====================
  CL_GetGameModeName
  ====================*/
tstring CL_GetGameModeName(uint uiMode)
{
    return CGameInfo::GetGameModeName(uiMode);
}


/*====================
  CL_GetGameModeFromString
  ====================*/
uint    CL_GetGameModeFromString(const tstring &sMode)
{
    return CGameInfo::GetGameModeFromString(sMode);
}


/*====================
  CL_GetGameModeString
  ====================*/
tstring CL_GetGameModeString(uint uiMode)
{
    return CGameInfo::GetGameModeString(uiMode);
}


/*====================
  CL_GetGameOptionName
  ====================*/
tstring CL_GetGameOptionName(uint uiOption)
{
    return CGameInfo::GetGameOptionName(uiOption);
}


/*====================
  CL_GetGameOptionFromString
  ====================*/
uint    CL_GetGameOptionFromString(const tstring &sOption)
{
    return CGameInfo::GetGameOptionFromString(sOption);
}


/*====================
  CL_GetGameOptionsString
  ====================*/
tstring CL_GetGameOptionsString(uint uiOptions)
{
    return CGameInfo::GetGameOptionsString(uiOptions);
}


/*====================
  CL_StartLoading
  ====================*/
void    CL_StartLoading(const tstring &sWorldName)
{
    GameClient.StartLoading(sWorldName);
}


/*====================
  InitLibrary
  ====================*/
#ifdef __GNUC__
extern "C" void __attribute__ ((visibility("default"))) InitLibrary(CClientGameLib &GameLib)
#else
void    InitLibrary(CClientGameLib &GameLib)
#endif
{
    GameLib.SetName(_T("Heroes of Newerth - Game Client"));
    GameLib.SetTypeName(_T("honclient"));
    GameLib.SetMajorVersion(1);
    GameLib.SetMinorVersion(0);
    GameLib.AssignSetGamePointerFn(CL_SetGamePointer);
    GameLib.AssignInitFn(CL_Init);
    GameLib.AssignStartLoadingWorldFn(CL_StartLoadingWorld);
    GameLib.AssignStartPreloadingWorldFn(CL_StartPreloadingWorld);
    GameLib.AssignFinishedLoadingWorldFn(CL_FinishedLoadingWorld);
    GameLib.AssignSpawnNextWorldEntityFn(CL_SpawnNextWorldEntity);
    GameLib.AssignPrecacheNextWorldEntityFn(CL_PrecacheNextWorldEntity);

    GameLib.AssignStartLoadingResourcesFn(CL_StartLoadingResources);
    GameLib.AssignLoadNextResourceFn(CL_LoadNextResource);
    GameLib.AssignIsFinishedLoadingResourcesFn(CL_IsFinishedLoadingResources);
    GameLib.AssignGetResourceLoadingProgressFn(CL_GetResourceLoadingProgress);

    GameLib.AssignIsSpawningEntitiesFn(CL_IsSpawningEntities);
    GameLib.AssignIsFinishedSpawningEntitiesFn(CL_IsFinishedSpawningEntities);
    GameLib.AssignGetEntitySpawningProgressFn(CL_GetEntitySpawningProgress);

    GameLib.AssignStartLoadingEntityResourcesFn(CL_StartLoadingEntityResources);
    GameLib.AssignLoadNextEntityResourceFn(CL_LoadNextEntityResource);
    GameLib.AssignIsFinishedLoadingEntityResourcesFn(CL_IsFinishedLoadingEntityResources);
    GameLib.AssignGetEntityResourceLoadingProgressFn(CL_GetEntityResourceLoadingProgress);

    GameLib.AssignPreFrameFn(CL_PreFrame);
    GameLib.AssignFrameFn(CL_Frame);
    GameLib.AssignGameDataFn(CL_GameData);
    GameLib.AssignShutdownFn(CL_Shutdown);
    GameLib.AssignGameEventFn(CL_ProcessGameEvents);
    GameLib.AssignSnapshotFn(CL_ProcessSnapshot);
    GameLib.AssignReinitializeFn(CL_Reinitialize);
    GameLib.AssignLoadAllResourcesFn(CL_LoadAllResources);
    GameLib.AssignConnectFn(CL_Connect);
    GameLib.AssignStateStringChangedFn(CL_StateStringChanged);
    GameLib.AssignStateBlockChangedFn(CL_StateBlockChanged);
    GameLib.AssignSendCreateGameRequestFn(CL_SendCreateGameRequest);
    GameLib.AssignGetGameModeNameFn(CL_GetGameModeName);
    GameLib.AssignGetGameModeFromStringFn(CL_GetGameModeFromString);
    GameLib.AssignGetGameModeStringFn(CL_GetGameModeString);
    GameLib.AssignGetGameOptionNameFn(CL_GetGameOptionName);
    GameLib.AssignGetGameOptionFromStringFn(CL_GetGameOptionFromString);
    GameLib.AssignGetGameOptionsStringFn(CL_GetGameOptionsString);
    GameLib.AssignStartLoadingFn(CL_StartLoading);
}
