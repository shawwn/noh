// (C)2005 S2 Games
// c_gameclient.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"

#include "c_cliententitydirectory.h"
#include "c_gameinterfacemanager.h"
#include "c_clientcommander.h"

#include "../hon_shared/c_teaminfo.h"
#include "../hon_shared/c_replaymanager.h"
#include "../hon_shared/c_gameinfo.h"
#include "../hon_shared/c_entitynpccontroller.h"
#include "../hon_shared/c_player.h"
#include "../hon_shared/c_entityeffect.h"
#include "../hon_shared/i_light.h"
#include "../hon_shared/i_buildingentity.h"
#include "../hon_shared/i_entitystate.h"
#include "../hon_shared/i_gadgetentity.h"
#include "../hon_shared/i_powerupentity.h"
#include "../hon_shared/i_entityitem.h"
#include "../hon_shared/i_heroentity.h"
#include "../hon_shared/i_bitentity.h"
#include "../hon_shared/i_shopentity.h"
#include "../hon_shared/c_shopdefinition.h"
#include "../hon_shared/c_gamemechanicsresource.h"
#include "../hon_shared/c_shopinfo.h"
#include "../hon_shared/c_shopiteminfo.h"
#include "../hon_shared/c_entitycamera.h"

#include "../k2/c_host.h"
#include "../k2/c_hostclient.h"
#include "../k2/c_world.h"
#include "../k2/c_camera.h"
#include "../k2/c_vid.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_actionregistry.h"
#include "../k2/c_buffer.h"
#include "../k2/c_snapshot.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_interface.h"
#include "../k2/c_soundmanager.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_texture.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_worldlight.h"
#include "../k2/c_worldsound.h"
#include "../k2/intersection.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_sample.h"
#include "../k2/c_xmlmanager.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_chatmanager.h"
#include "../k2/c_stringtable.h"
#include "../k2/c_statestring.h"
#include "../k2/c_transmitflags.h"
#include "../k2/c_filehttp.h"
#include "../k2/c_httpmanager.h"
#include "../k2/c_httprequest.h"
#include "../k2/c_uitriggerregistry.h"
#include "../k2/c_checksumtable.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLF  (res_unloadHeroResources,       true,       CVAR_SAVECONFIG);
CVAR_BOOLF  (res_unloadMapResources,        true,       CVAR_SAVECONFIG);
CVAR_BOOL   (cg_debugPrediction,                false);

CVAR_BOOL   (cg_prediction,                     true);
CVAR_BOOLF  (cg_debugClientSnapshot,            false,      CONEL_DEV);

CVAR_UINTF  (cg_buildingAttackAlertTime,        5000,       CVAR_SAVECONFIG);
CVAR_UINTF  (cg_mapPingLifetime,                5000,       CVAR_SAVECONFIG);

//CVAR_BOOLF    (cg_precacheEntities,               true,       CVAR_SAVECONFIG);

CVAR_BOOL   (cg_debugGameEvent,                 false);
CVAR_BOOLF  (cg_showLongServerFrames,           true,       CVAR_SAVECONFIG);

CVAR_INT    (cg_serverSnapshotCacheSize,        30);

CVAR_STRINGF(cg_worldAmbientSound,              "",         CVAR_WORLDCONFIG);
CVAR_FLOATF (cg_worldAmbientSoundVolume,        1.0,        CVAR_WORLDCONFIG);

CVAR_BOOLF  (cg_drawMinimapOutlines,            true,       CVAR_SAVECONFIG);

CVAR_BOOL   (cg_drawSelectedStats,              false);
CVAR_BOOL   (cg_drawSelectedPath,               false);

EXTERN_CVAR (bool, cg_debugEntities);

CVAR_BOOL   (cg_debugGameData,                  false);

CVAR_UINTF  (cam_endGameLerpTime,               3000,                           CVAR_GAMECONFIG);
CVAR_FLOATF (cam_endGameMinHeight,              10.0f,                          CVAR_GAMECONFIG);
CVAR_VEC3F  (cam_endGameOffset,                 CVec3f(0.0f, 2000.0f, 1000.0f), CVAR_GAMECONFIG);

CVAR_BOOL   (cg_replayServerStats,              false);

CVAR_BOOL   (cg_drawNavGrid,                    false);
CVAR_INT    (cg_drawNavGridDownsize,            0);
CVAR_INT    (cg_pathDisplaySize,                32);

CVAR_UINTF  (cg_minimapFPS,                     20,                     CVAR_SAVECONFIG);
CVAR_FLOAT  (cg_maxAspect,                      2.15f);

CVAR_FLOAT  (cg_soundPosition,                  0.3f);

CVAR_UINTF  (cg_createGameTimeOut,              90000,                  CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_menuTransitions,                true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_disableBackgroundEffects,       false,                  CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_drawRewardExp,                  true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_drawRewardGold,                 true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_drawPopupDeflection,            true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_drawPopupCritical,              true,                   CVAR_SAVECONFIG);

CVAR_FLOAT  (cg_fogofwarDarkness,               0.5f);
CVAR_BOOL   (cg_fogofwar,                       true);

CVAR_BOOLF  (cg_drawEntityTypeNames,            false,                  CONEL_DEV);
CVAR_BOOLF  (cg_dev,                            false,                  CONEL_DEV);

CVAR_BOOLF  (cg_unitPlayerColors,               true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_heroIndicators,                 true,                   CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_unitVoiceResponses,             true,                   CVAR_SAVECONFIG);
CVAR_UINTF  (cg_unitVoiceResponsesDelay,        3000,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_muteAnnouncerVoice,             false,                  CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_alwaysShowHealthBars,           true,                   CVAR_SAVECONFIG);
CVAR_BOOL   (cg_drawPing,                       false);
CVAR_BOOL   (cg_noLerp,                         false);
CVAR_BOOLF  (cg_hoverHighlight,                 false,                  CVAR_SAVECONFIG);
CVAR_FLOAT  (cg_hoverHighlightStrength,         1.3f);
CVAR_VEC3   (cg_hoverHighlightBase,             CVec3f(0.35f, 0.35f, 0.35f));

CVAR_BOOL   (cg_cameraSmoothing,                false);
CVAR_BOOLF  (cg_cameraCenterOnRespawn,          true,                   CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_mapPingEnable,                  true,                   CVAR_SAVECONFIG);

CVAR_BOOL   (cg_lockCamera,                     false);
CVAR_BOOL   (cg_altLockCamera,                  false);
CVAR_BOOLF  (cg_lockCameraGame,                 false,                  CVAR_SAVECONFIG);
CVAR_FLOATF (cg_lockCameraMod,                  1.0f,                   CVAR_SAVECONFIG);

CVAR_BOOL   (cg_drawChat,                       true);
CVAR_BOOL   (cg_drawMessages,                   true);

CVAR_BOOLF  (cg_showGenericHealthLerp,          false,          CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_showCreepHealthLerp,            false,          CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_showHeroHealthLerp,             true,           CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_showBuildingHealthLerp,         false,          CVAR_SAVECONFIG);
CVAR_BOOLF  (cg_showExperienceRangeIndicator,   true,           CVAR_SAVECONFIG);

CVAR_UINTF  (cg_lagThreshold,                   1000,           CVAR_SAVECONFIG);

CVAR_INT    (replay_client,                     -1);

CVAR_BOOL   (cg_replayPrecacheAll,              false);

CVAR_UINTF  (sound_lobbyFadeout_delayFrames,    3,          CVAR_SAVECONFIG);

CVAR_STRINGF(cg_current_weather,                "",         CVAR_SAVECONFIG);

UI_TRIGGER(Minimap);
UI_TRIGGER(MinimapHover);
UI_TRIGGER(GameMessage);
UI_TRIGGER(Scores);
UI_TRIGGER(MapEffect);
UI_TRIGGER(PlayerList);

UI_TRIGGER(EventActiveLevelup);
UI_TRIGGER(EventActiveExperience);
UI_TRIGGER(EventActiveInventoryReady0);
UI_TRIGGER(EventActiveInventoryReady1);
UI_TRIGGER(EventActiveInventoryReady2);
UI_TRIGGER(EventActiveInventoryReady3);
UI_TRIGGER(EventActiveInventoryReady4);
UI_TRIGGER(EventActiveInventoryReady5);
UI_TRIGGER(EventActiveInventoryReady6);
UI_TRIGGER(EventActiveInventoryReady7);
UI_TRIGGER(EventActiveInventoryReady8);
UI_TRIGGER(EventActiveInventoryReady9);
UI_TRIGGER(EventActiveInventoryReady25);
UI_TRIGGER(EventActiveInventoryReady26);
UI_TRIGGER(EventActiveInventoryReady27);
UI_TRIGGER(EventActiveInventoryReady28);
UI_TRIGGER(EventActiveInventoryReady29);
UI_TRIGGER(EventActiveInventoryReady30);
UI_TRIGGER(EventActiveInventoryUpgrade0);
UI_TRIGGER(EventActiveInventoryUpgrade1);
UI_TRIGGER(EventActiveInventoryUpgrade2);
UI_TRIGGER(EventActiveInventoryUpgrade3);
UI_TRIGGER(EventActiveInventoryUpgrade4);
UI_TRIGGER(EventActiveInventoryUpgrade5);
UI_TRIGGER(EventActiveInventoryUpgrade6);
UI_TRIGGER(EventActiveInventoryUpgrade7);
UI_TRIGGER(EventActiveInventoryUpgrade8);
UI_TRIGGER(EventActiveInventoryUpgrade9);
UI_TRIGGER(EventActiveInventoryUpgrade25);
UI_TRIGGER(EventActiveInventoryUpgrade26);
UI_TRIGGER(EventActiveInventoryUpgrade27);
UI_TRIGGER(EventActiveInventoryUpgrade28);
UI_TRIGGER(EventActiveInventoryUpgrade29);
UI_TRIGGER(EventActiveInventoryUpgrade30);

UI_TRIGGER(EventHeroLevelup);
UI_TRIGGER(EventHeroExperience);
UI_TRIGGER(EventPlayerGold);
UI_TRIGGER(EventGamePhaseChanged);

UI_TRIGGER(EventKill);
UI_TRIGGER(EventStartLoading);

UI_TRIGGER(ReplayDownloadFinished);
UI_TRIGGER(ReplayDownloadPercent);
UI_TRIGGER(ReplayDownloadProgress);
UI_TRIGGER(ReplayDownloadSpeed);
UI_TRIGGER(ReplayDownloadTime);

INPUT_STATE_BOOL(AltInfoAllies);
INPUT_STATE_BOOL(AltInfoEnemies);
INPUT_STATE_BOOL(Center);
INPUT_STATE_BOOL(PortraitCenter);
INPUT_STATE_BOOL(CenterInfo);
INPUT_STATE_BOOL(PortraitCenterInfo);

SVoiceSet   g_NullVoiceSet;

EXTERN_CVAR_STRING(cg_avatarHero);
//=============================================================================

/*====================
  CGameClient::~CGameClient
  ====================*/
CGameClient::~CGameClient()
{
    Console << _T("GameClient released") << newl;

    SAFE_DELETE(m_pCamera);
    SAFE_DELETE(m_pClientEntityDirectory);
    SAFE_DELETE(m_pClientCommander);
    SAFE_DELETE(m_pMinimapBitmap);

    SAFE_DELETE(m_pInterfaceManager);

    SAFE_DELETE(m_pReplayDownload);

    StopWorldSounds();
    bool bFadeOutMusic(false);
    K2SoundManager.StopMusic(bFadeOutMusic);

    if (m_ahResources[CLIENT_RESOURCE_ALT_INFO_INTERFACE] != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_ahResources[CLIENT_RESOURCE_ALT_INFO_INTERFACE], UNREG_RESERVE_HANDLE);
    if (m_ahResources[CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE] != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_ahResources[CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE], UNREG_RESERVE_HANDLE);
    if (m_ahResources[CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE] != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_ahResources[CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE], UNREG_RESERVE_HANDLE);
    if (m_ahResources[CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE] != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_ahResources[CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE], UNREG_RESERVE_HANDLE);

    for (vector<PoolHandle>::iterator it(m_vServerSnapshots.begin()); it != m_vServerSnapshots.end(); ++it)
        SAFE_DELETE_SNAPSHOT(*it);

    SAFE_DELETE_SNAPSHOT(m_hServerSnapshotFallback);

    if (m_hMinimapTexture != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_hMinimapTexture, UNREG_RESERVE_HANDLE);
    if (m_hLoadingTexture != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_hLoadingTexture, UNREG_RESERVE_HANDLE);
}


/*====================
  CGameClient::CGameClient
  ====================*/
CGameClient::CGameClient() : IGame(_T("client")),
m_pHostClient(nullptr),
m_pClientEntityDirectory(nullptr),
m_pInterfaceManager(nullptr),
m_pCamera(K2_NEW(ctx_GameClient,   CCamera)),

m_v3CameraPosition(V3_ZERO),
m_v3CameraAngles(V3_ZERO),
m_uiCameraFrame(-1),
m_uiCameraIndex(INVALID_INDEX),

m_vServerSnapshots(cg_serverSnapshotCacheSize, INVALID_POOL_HANDLE),
m_hServerSnapshotFallback(INVALID_POOL_HANDLE),
m_uiSnapshotBufferPos(0),

m_bShowMenu(false),
m_bShowLobby(false),
m_uiLastGamePhase(GAME_PHASE_IDLE),
m_eLastInterface(CG_INTERFACE_INVALID),

m_pLocalPlayer(nullptr),
m_pCurrentEntity(nullptr),
m_pClientCommander(K2_NEW(ctx_GameClient,   CClientCommander)),

m_bStartedLoadingResources(false),
m_uiTotalResourcesToLoad(0),

m_bStartedLoadingHeroes(false),

m_bStartedSpawningEntities(false),
m_uiTotalWorldThings(0),

m_hMinimapReference(INVALID_RESOURCE),
m_hMinimapTexture(INVALID_RESOURCE),
m_pMinimapBitmap(nullptr),
m_hLoadingTexture(INVALID_RESOURCE),

m_v3CameraEffectAngleOffset(V3_ZERO),
m_v3CameraEffectOffset(V3_ZERO),
m_uiOrderEvent(INVALID_INDEX),
m_yOrderUniqueID(-1),

m_uiLastConfirmAttackSoundTime(0),
m_uiLastConfirmMoveSoundTime(0),
m_uiLastSelectedSoundTime(0),

m_bPinging(false),

m_uiWeatherEffectCounter(0),

m_uiItemCursorIndex(INVALID_INDEX),
m_pFogofWarBitmap(nullptr),
m_uiLastFogofWarUpdate(INVALID_TIME),
m_cVisRaster(SQR(RASTER_BUFFER_SPAN)),
m_cOccRaster(SQR(RASTER_BUFFER_SPAN)),
m_uiLastMinimapUpdateTime(0),
m_uiMinimapHoverUnit(-1),
m_bLevelup(false),
m_uiLevelupIndex(INVALID_INDEX),

m_unLastHeroType(INVALID_ENT_TYPE),
m_bDownloadingReplay(false),
m_pReplayDownload(nullptr),
m_bProcessedFirstSnapshot(false),
m_pReplaySpectator(nullptr),

m_uiDelayHeroLoading(0),

m_bJustStartedGame(false),
m_bWasUsingCustomFiles(false),
m_bWereCoreFilesModified(false),
m_hClientSounds(INVALID_RESOURCE),
m_bMouseDownOverMiniMap(false),
m_bStartedLoadingEntityResources(false)
{
    m_pInterfaceManager = K2_NEW(ctx_GameClient,   CGameInterfaceManager);
    m_pClientEntityDirectory = K2_NEW(ctx_GameClient,   CClientEntityDirectory);
    m_pReplayDownload = K2_NEW(ctx_GameClient,   CFileHTTP);

    for (int i(0); i < NUM_CLIENT_GAME_EFFECT_THREADS; ++i)
        m_apEffectThreads[i] = nullptr;

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        m_ahSoundHandle[i] = INVALID_INDEX;

    for (int i(0); i < NUM_CLIENT_RESOURCES; ++i)
        m_ahResources[i] = INVALID_RESOURCE;
}


/*====================
  CGameClient::Initialize
  ====================*/
bool    CGameClient::Initialize(CHostClient *pHostClient)
{
    PROFILE("CGameClient::Initialize");

    try
    {
        // Get pointer to host
        m_pHostClient = pHostClient;
        if (m_pHostClient == nullptr)
            EX_ERROR(_T("Invalid CHostClient"));

        // Setup IGame members
        SetWorldPointer(m_pHostClient->GetWorld());
        SetEntityDirectory(m_pClientEntityDirectory);
        Validate();

        // Create a camera
        if (m_pCamera == nullptr)
            EX_ERROR(_T("Failed to allocate a CCamera"));
        m_pCamera->DefaultCamera(float(Vid.GetScreenW()), float(Vid.GetScreenH()));

        ActionRegistry.BindAxis(BINDTABLE_GAME,     AXIS_MOUSE_X,   BIND_MOD_NONE,  _T("CameraYaw"),        _T(""));
        ActionRegistry.BindAxis(BINDTABLE_GAME,     AXIS_MOUSE_Y,   BIND_MOD_NONE,  _T("CameraPitch"),      _T(""));
        ActionRegistry.BindImpulse(BINDTABLE_UI,    BUTTON_F6,      BIND_MOD_NONE,  _T("ToggleMenu"),       _T(""));

        m_hMinimapReference = g_ResourceManager.Register(_T("!minimap_texture"), RES_REFERENCE);
        g_ResourceManager.UpdateReference(m_hMinimapReference, g_ResourceManager.GetBlackTexture());

        m_bShowMenu = false;
        m_bShowLobby = false;

        m_deqHeroAnnouncements.clear();
        m_hHeroAnnounce = INVALID_INDEX;

        ChatManager.InitCensor();

        // Load client game resources
        XMLManager.Process(_T("/game.resources"), _T("resourcelist"));
        
        // load the entity definitions on initialization so the public games list can read them and display specific advanced game options tooltips
        m_ahResources[CLIENT_RESOURCE_ENTITY_STRING_TABLE] = g_ResourceManager.Register(_T("/stringtables/entities.str"), RES_STRINGTABLE);
        AddResourceToLoadingQueue(CLIENT_RESOURCE_ENTITY_STRING_TABLE, _T("/stringtables/entities.str"), RES_STRINGTABLE);

        // load the game_messages definitions on initialization so the "weather" option can read the localized name of the weather effect
        m_ahResources[CLIENT_RESOURCE_GAME_MESSAGE_TABLE] = g_ResourceManager.Register(_T("/stringtables/game_messages.str"), RES_STRINGTABLE);
        AddResourceToLoadingQueue(CLIENT_RESOURCE_GAME_MESSAGE_TABLE, _T("/stringtables/game_messages.str"), RES_STRINGTABLE);

        m_pInterfaceManager->LoadMainInterfaces();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameClient::Initialize() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameClient::Reinitialize
  ====================*/
void    CGameClient::Reinitialize()
{
    SetGameTime(0);
    SetGameInfo(nullptr);

    m_v3CameraPosition = V3_ZERO;
    m_v3CameraAngles = V3_ZERO;
    m_uiCameraFrame = -1;
    m_uiCameraIndex = INVALID_INDEX;

    m_bShowMenu = false;
    m_bShowLobby = false;

    m_pLocalPlayer = nullptr;
    m_pCurrentEntity = nullptr;

    m_v3CameraEffectAngleOffset = V3_ZERO;
    m_v3CameraEffectOffset = V3_ZERO;
    m_uiOrderEvent = INVALID_INDEX;

    m_bPinging = false;

    m_pClientEntityDirectory->Clear();

    Game.ClearEventList();

    for (int i(0); i < NUM_CLIENT_GAME_EFFECT_THREADS; ++i)
        SAFE_DELETE(m_apEffectThreads[i]);

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
    {
        if (m_ahSoundHandle[i] != INVALID_INDEX)
        {
            K2SoundManager.StopHandle(m_ahSoundHandle[i]);
            m_ahSoundHandle[i] = INVALID_INDEX;
        }
    }

    StopWorldSounds();
    K2SoundManager.StopMusic();

    ClearTeams();

    SetWinningTeam(TEAM_INVALID);
    SetFinalMatchTime(INVALID_TIME);

    m_mapClients.clear();
    m_uiItemCursorIndex = INVALID_INDEX;
    m_uiLastFogofWarUpdate = INVALID_TIME;
    SetLevelup(false);

    m_pClientCommander->Reinitialize();

    m_deqHeroAnnouncements.clear();
    m_hHeroAnnounce = INVALID_INDEX;

    Input.SetCursor(CURSOR_GAME, INVALID_RESOURCE);
    Input.SetCursorRecenter(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorHidden(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorFrozen(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstrained(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstraint(CURSOR_GAME, CRectf(0.0f, 0.0f, 0.0f, 0.0f));

    CWorld *pWorld(GetWorldPointer());
    if (pWorld != nullptr)
        pWorld->SetVisibilitySize(int(g_fogofwarSize));

    m_bProcessedFirstSnapshot = false;
    m_pReplaySpectator = nullptr;

    UIManager.RemoveOverlayInterface(_T("game_replay_control"));

    m_uiDelayHeroLoading = 0;
    m_hClientSounds = INVALID_RESOURCE;
}


/*====================
  CGameClient::StartLoading
  ====================*/
void    CGameClient::StartLoading(const tstring &sWorldName)
{
    Console << _T("Start loading ") << sWorldName << newl;

    m_bStartedLoadingResources = false;
    m_bStartedLoadingEntityResources = false;
    m_uiTotalResourcesToLoad = 0;
    m_deqClientResources.clear();
    m_bStartedSpawningEntities = false;
    m_deqWorldThings.clear();

    EventStartLoading.Trigger(TSNULL);

    if (!IsWorldPreloaded(sWorldName))
    {
        g_ResourceInfo.ExecCommandLine(_T("context move client:curgame_world client:prevgame_world"));
        g_ResourceInfo.SetGameContextCategoryLoaded(_T("world"), false);
    }
}


/*====================
  CGameClient::StartLoadingWorld
  ====================*/
void    CGameClient::StartLoadingWorld()
{
    PROFILE_EX("CGameClient::LoadWorld", PROFILE_GAME_LOAD_WORLD);

    if (!IsWorldLoaded())
    {
        Console.Err << _T("Host has not loaded the world yet") << newl;
        return;
    }

#if 0
    if (m_hLoadingTexture != INVALID_RESOURCE)
    {
        g_ResourceManager.Unregister(m_hLoadingTexture);
        m_hLoadingTexture = INVALID_RESOURCE;
    }

    tstring sLoadingFilename(Filename_StripExtension(GetWorldPointer()->GetPath()) + _T("/loading.tga"));
    if (UITextureRegistry.TextureExists(sLoadingFilename, 0))
        UITextureRegistry.Register(sLoadingFilename, 0, m_hLoadingTexture);
#endif

    m_pCamera->SetWorld(GetWorldPointer());

    int iSize(1 << int(g_fogofwarSize));

    m_uiVisibilitySize = iSize;
    m_fVisibilityScale = m_uiVisibilitySize * GetWorldPointer()->GetScale(); 
    m_cVisibilityMap.Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    SAFE_DELETE(m_pFogofWarBitmap);
    m_pFogofWarBitmap = K2_NEW(ctx_GameClient,   CBitmap)(m_cVisibilityMap.GetWidth(), m_cVisibilityMap.GetHeight(), BITMAP_ALPHA);

    Vid.Notify(VID_NOTIFY_FOG_OF_WAR, m_cVisibilityMap.GetWidth(), m_cVisibilityMap.GetHeight(), int(g_fogofwarSize), nullptr);

    StopWorldSounds();
    K2SoundManager.MuteSFX(true);

    UpdateMinimapTexture();

    m_bStartedSpawningEntities = true;

    if (!cg_worldAmbientSound.empty())
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(cg_worldAmbientSound, SND_2D), RES_SAMPLE));
            if (hSample != INVALID_INDEX)
                m_hWorldAmbientSound = K2SoundManager.Play2DSFXSound(hSample, cg_worldAmbientSoundVolume, -1, 100, true);
        }
    }

    // Populate world thing list
    // Entities
    WorldEntList &vWorldEntities(GetWorldEntityList());
    for (WorldEntList_it it(vWorldEntities.begin()); it != vWorldEntities.end(); ++it)
        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_ENTITY, *it, 0));

    // Lights
    WorldLightsMap &mapWorldLights(GetWorldLightsMap());
    for (WorldLightsMap_it it(mapWorldLights.begin()); it != mapWorldLights.end(); ++it)
        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_LIGHT, it->second->GetIndex(), it->first));

    // Sounds
    WorldSoundsMap &mapWorldSounds(GetWorldSoundsMap());
    for (WorldSoundsMap_it it(mapWorldSounds.begin()); it != mapWorldSounds.end(); it++)
        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_SOUND, it->first, 0, it->second->GetSound()));

    // Bit entities
    const IBuffer &cBitEntityMap(m_pHostClient->GetStateBlock(STATE_BLOCK_BIT)->GetBuffer());
    cBitEntityMap.Rewind();
    uint uiNumBitEntities(cBitEntityMap.GetLength() / 4);
    for (uint ui(0); ui < uiNumBitEntities; ++ui)
    {
        uint uiGameIndex(cBitEntityMap.ReadShort());
        uint uiWorldIndex(cBitEntityMap.ReadShort());

        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_BIT_ENTITY, uiWorldIndex, uiGameIndex, ui));
    }

    m_uiTotalWorldThings = INT_SIZE(m_deqWorldThings.size());

    // Load client game resources (if map overrides)
    XMLManager.Process(_T("/game.resources"), _T("resourcelist"));
}


/*====================
  CGameClient::SpawnNextWorldEntity
  ====================*/
void    CGameClient::SpawnNextWorldEntity()
{
    PROFILE_EX("CGameClient::SpawnNextWorldEntity", PROFILE_GAME_LOAD_WORLD);

    if (!IsWorldLoaded() || m_deqWorldThings.empty())
        return;

    SWorldThingSpawnRequest &thing(m_deqWorldThings.front());
    switch (thing.m_eType)
    {
    case WORLD_THING_ENTITY:
        {
            uint uiHandle(thing.m_uiWorldIndex);
            if (uiHandle == INVALID_POOL_HANDLE)
                break;

            CWorldEntity *pWorldEntity(GameClient.GetWorldPointer()->GetEntityByHandle(uiHandle));
            if (pWorldEntity == nullptr)
            {
                Console << _T("Failed world entity lookup") << newl;
                break;
            }

            if (pWorldEntity->GetType() == _T("Prop_Cliff"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Cliff2"))
            {
                GameClient.SpawnCliff(pWorldEntity);
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Static"))
            {
                GameClient.SpawnStaticProp(pWorldEntity);
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Water"))
            {
                GameClient.SpawnWater(pWorldEntity);
                break;
            }

            ushort uiID(EntityRegistry.LookupID(pWorldEntity->GetType()));
            if (uiID == ushort(-1))
                break;

            if (uiID == Entity_NeutralCampSpawner || uiID == Entity_BossSpawner)
                GameClient.Precache(pWorldEntity->GetProperty(_T("target0")), PRECACHE_ALL, TSNULL);

            IGameEntity* pNew(m_pClientEntityDirectory->AllocateLocal(uiID));
            if (pNew == nullptr)
            {
                Console.Err << _T("Failed to allocate a client-side game entity for world entity #") << pWorldEntity->GetIndex() << newl;
                break;
            }

            GameClient.Precache(pNew->GetType(), PRECACHE_ALL, TSNULL);

            if (!pNew->IsStatic())
            {
                size_t zPos(m_uiTotalWorldThings - m_deqWorldThings.size());

                if (!pNew->IsBit())
                    GameClient.GetWorldPointer()->FreeEntityByHandle(uiHandle);

                if (zPos >= m_uiTotalWorldThings)
                    m_deqWorldThings.clear();

                m_pClientEntityDirectory->Delete(pNew->GetIndex());
                break;
            }

            pWorldEntity->SetGameIndex(pNew->GetIndex());
                
            pNew->ApplyWorldEntity(*pWorldEntity);
            pNew->Spawn();
            pNew->Validate();

            if (pNew->IsVisual())
            {
                CClientEntity* pNewClientEnt(m_pClientEntityDirectory->GetClientEntity(pNew->GetIndex()));
                if (pNewClientEnt == nullptr)
                {
                    Console.Err << _T("Failed to allocate a client game entity for world entity #") << pWorldEntity->GetIndex() << newl;
                    break;
                }

                // Setup previous and current states
                pNewClientEnt->CopyNextToPrev();
                pNewClientEnt->CopyNextToCurrent();
                pNewClientEnt->Interpolate(1.0f);
            }
        }
        break;

    case WORLD_THING_LIGHT:
        {
            IGameEntity* pNew(m_pClientEntityDirectory->AllocateLocal(Light_Static));
            CClientEntity* pNewClientEnt(m_pClientEntityDirectory->GetClientEntity(pNew->GetIndex()));
            if (pNewClientEnt == nullptr)
            {
                Console.Err << _T("Failed to allocate a light for world light #") << thing.m_uiGameIndex << newl;
                break;
            }

            IGameEntity *pNextEnt(pNewClientEnt->GetNextEntity());
            if (!pNextEnt->IsStatic())
            {
                m_pClientEntityDirectory->Delete(pNextEnt);
                break;
            }

            ILight *pLight(pNextEnt->GetAsLight());
            if (pLight == nullptr)
            {
                m_pClientEntityDirectory->Delete(pNextEnt);
                break;
            }

            pLight->SetWorldIndex(thing.m_uiWorldIndex);
            pLight->Spawn();
            //Console.ServerGame << _T("Spawned new light #") << pLight->GetIndex() << _T(" @ ") << pLight->GetPosition() << newl;

            pLight->Validate();

            pNewClientEnt->CopyNextToPrev();
            pNewClientEnt->CopyNextToCurrent();
            pNewClientEnt->Interpolate(1.0f);
        }
        break;

    case WORLD_THING_SOUND:
        {
            ResHandle hSample(INVALID_RESOURCE);

            K2_WITH_GAME_RESOURCE_SCOPE()
            {
                hSample = g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(thing.m_sName, 0), RES_SAMPLE);
                if (hSample == INVALID_RESOURCE)
                    break;
            }
            m_mapWorldSounds[thing.m_uiWorldIndex] = {INVALID_INDEX, hSample};
        }
        break;

    case WORLD_THING_BIT_ENTITY:
        {
            CWorldEntity *pWorldEntity(GetWorldEntity(thing.m_uiWorldIndex));
            if (pWorldEntity == nullptr)
                break;

            IGameEntity *pEntity(m_pClientEntityDirectory->Allocate(thing.m_uiGameIndex, pWorldEntity->GetType()));
            if (pEntity == nullptr)
                break;

            pWorldEntity->SetGameIndex(thing.m_uiGameIndex);

            pEntity->ApplyWorldEntity(*pWorldEntity);
            pEntity->Spawn();
            pEntity->Validate();

            IBitEntity *pBit(pEntity->GetAsBit());
            if (pBit != nullptr)
            {
                pBit->SetBitIndex(thing.m_uiBitIndex);
                m_pClientEntityDirectory->AddBitEntity(pEntity->GetAsBit());
            }
        }
        break;
    }

    m_deqWorldThings.pop_front();
}


/*====================
  CGameClient::StartPreloadingWorld
  ====================*/
void    CGameClient::StartPreloadingWorld()
{
    PROFILE_EX("CGameClient::PreloadWorld", PROFILE_GAME_LOAD_WORLD);

    if (!IsWorldLoaded())
    {
        Console.Err << _T("Host has not loaded the world yet") << newl;
        return;
    }

    m_pCamera->SetWorld(GetWorldPointer());

    int iSize(1 << int(g_fogofwarSize));

    m_uiVisibilitySize = iSize;
    m_fVisibilityScale = m_uiVisibilitySize * GetWorldPointer()->GetScale(); 
    m_cVisibilityMap.Initialize(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize, GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    SAFE_DELETE(m_pFogofWarBitmap);
    m_pFogofWarBitmap = K2_NEW(ctx_GameClient,   CBitmap)(m_cVisibilityMap.GetWidth(), m_cVisibilityMap.GetHeight(), BITMAP_ALPHA);

    Vid.Notify(VID_NOTIFY_FOG_OF_WAR, m_cVisibilityMap.GetWidth(), m_cVisibilityMap.GetHeight(), int(g_fogofwarSize), nullptr);

    StopWorldSounds();
    K2SoundManager.MuteSFX(true);

    UpdateMinimapTexture();

    m_bStartedSpawningEntities = true;

    if (!cg_worldAmbientSound.empty())
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            ResHandle hSample(g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(cg_worldAmbientSound, SND_2D), RES_SAMPLE));
            if (hSample != INVALID_INDEX)
                m_hWorldAmbientSound = K2SoundManager.Play2DSFXSound(hSample, cg_worldAmbientSoundVolume, -1, 100, true);
        }
    }

    // Populate world thing list
    // Entities
    WorldEntList &vWorldEntities(GetWorldEntityList());
    for (WorldEntList_it it(vWorldEntities.begin()); it != vWorldEntities.end(); ++it)
        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_ENTITY, *it, 0));

    // Sounds
    WorldSoundsMap &mapWorldSounds(GetWorldSoundsMap());
    for (WorldSoundsMap_it it(mapWorldSounds.begin()); it != mapWorldSounds.end(); it++)
        m_deqWorldThings.push_back(SWorldThingSpawnRequest(WORLD_THING_SOUND, it->first, 0, it->second->GetSound()));

    m_uiTotalWorldThings = INT_SIZE(m_deqWorldThings.size());

    // Load client game resources (if map overrides)
    XMLManager.Process(_T("/game.resources"), _T("resourcelist"));
}


/*====================
  CGameClient::PrecacheNextWorldEntity
  ====================*/
void    CGameClient::PrecacheNextWorldEntity()
{
    PROFILE_EX("CGameClient::PrecacheNextWorldEntity", PROFILE_GAME_LOAD_WORLD);

    if (!IsWorldLoaded() || m_deqWorldThings.empty())
        return;

    SWorldThingSpawnRequest &thing(m_deqWorldThings.front());
    switch (thing.m_eType)
    {
    case WORLD_THING_ENTITY:
        {
            uint uiHandle(thing.m_uiWorldIndex);
            if (uiHandle == INVALID_POOL_HANDLE)
                break;

            CWorldEntity *pWorldEntity(GameClient.GetWorldPointer()->GetEntityByHandle(uiHandle));
            if (pWorldEntity == nullptr)
            {
                Console << _T("Failed world entity lookup") << newl;
                break;
            }

            if (pWorldEntity->GetType() == _T("Prop_Cliff"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Cliff2"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Static"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Water"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Scenery"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Tree"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }
            else if (pWorldEntity->GetType() == _T("Prop_Dynamic"))
            {
                GameClient.RegisterModel(pWorldEntity->GetModelPath());
                break;
            }

            ushort uiID(EntityRegistry.LookupID(pWorldEntity->GetType()));
            if (uiID == ushort(-1))
                break;

            if (uiID == Entity_NeutralCampSpawner || uiID == Entity_BossSpawner)
                GameClient.Precache(pWorldEntity->GetProperty(_T("target0")), PRECACHE_ALL, TSNULL);

            GameClient.Precache(uiID, PRECACHE_ALL, TSNULL);
        }
        break;

    case WORLD_THING_SOUND:
        {
            ResHandle hSample(INVALID_RESOURCE);

            K2_WITH_GAME_RESOURCE_SCOPE()
            {
                hSample = g_ResourceManager.Register(K2_NEW(ctx_GameClient, CSample)(thing.m_sName, 0), RES_SAMPLE);
                if (hSample == INVALID_RESOURCE)
                    break;
            }
        }
        break;
    case WORLD_THING_LIGHT:
        {
        }
        break;
    case WORLD_THING_BIT_ENTITY:
        {
        }
        break;
    }

    m_deqWorldThings.pop_front();
}


/*====================
  CGameClient::GetVoicePath
  ====================*/
tstring     CGameClient::GetVoicePath(const tstring &sSet, const tstring &sKey)
{
    CStringTable *pClientSounds(g_ResourceManager.GetStringTable(m_hClientSounds));
    if (pClientSounds == nullptr)
        return TSNULL;

    tstring sMessage(pClientSounds->Get(sKey));

    size_t zOffset(0);
    while (zOffset != tstring::npos)
    {
        size_t zStart(sMessage.find(_T('{'), zOffset));
        if (zStart == tstring::npos)
            break;
        size_t zEnd(sMessage.find(_T('}'), zStart));
        if (zEnd == tstring::npos)
            break;

        // Default parameter
        size_t zMid(sMessage.find(_T('='), zStart));
        if (zMid < zEnd)
        {
            const tstring &sToken(sMessage.substr(zStart + 1, zMid - zStart - 1));
            if (TStringCompare(sToken, _T("set")) == 0)
            {
                const tstring &sValue(sSet);
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            else
            {
                const tstring &sValue(sMessage.substr(zMid + 1, zEnd - zMid - 1));
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            continue;
        }

        const tstring &sToken(sMessage.substr(zStart + 1, zEnd - zStart - 1));
        const tstring &sValue(TStringCompare(sToken, _T("set")) != 0 ? TSNULL : sSet);
        zOffset = zStart + sValue.length();
        sMessage.replace(zStart, zEnd - zStart + 1, sValue);
    }

    return FileManager.SanitizePath(sMessage);
}


/*====================
  CGameClient::AddAnnouncerVoiceSampleToLoadingQueue

  TODO: Do this in a way to doesn't leave resources failed to load in the res lib
  ====================*/
void    CGameClient::AddAnnouncerVoiceSampleToLoadingQueue(const tstring &sSet, const tstring &sKey)
{
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, GetVoicePath(sSet, sKey), RES_SAMPLE);
}


/*====================
  CGameClient::LookupAnnouncerVoiceSample
  ====================*/
ResHandle   CGameClient::LookupAnnouncerVoiceSample(const tstring &sSet, const tstring &sKey)
{
    ResHandle hSound(g_ResourceManager.LookUpPath(GetVoicePath(sSet, sKey)));

    IResource *pResource(g_ResourceManager.Get(hSound));
    if (pResource == nullptr || pResource->HasFlags(RES_LOAD_FAILED))
        hSound = INVALID_RESOURCE;

    if (hSound == INVALID_RESOURCE && !sSet.empty())
        return LookupAnnouncerVoiceSample(TSNULL, sKey);
    else
        return hSound;
}


/*====================
  CGameClient::StartLoadingResources
  ====================*/
void    CGameClient::StartLoadingResources()
{
    m_deqReplayHeroResources.clear();
    if (ReplayManager.IsPlaying())
    {
        // Get the list of hero resources that need to be precached
        for (uint i(0); i < 10; ++i)
        {
            const CXMLNode& cPlayerInfo(ReplayManager.GetReplayInfo().GetPlayerInfo(i));

            const tstring &sHero(cPlayerInfo.GetProperty(_T("heroname"), TSNULL));
            if (sHero == TSNULL)
                continue;

            IEntityDefinition *pDefinition(EntityRegistry.GetDefinition<IEntityDefinition>(sHero));
            if (pDefinition == nullptr)
                continue;

            const tstring &sAvatar(cPlayerInfo.GetProperty(_T("avatar"), TSNULL));

            GetPrecacheList(sHero, PRECACHE_ALL, sAvatar, m_deqReplayHeroResources);
        }
    }

    // Stringtable must be loaded before precaching entities
    PrecacheEntities();

    // Precache misc. effects not directly attached to an entity
    AddResourceToLoadingQueue(CLIENT_RESOURCE_MOVE_INDICATOR_EFFECT, _T("/shared/effects/move_indicator.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_ATTACK_INDICATOR_EFFECT, _T("/shared/effects/attack_indicator.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_CAST_INDICATOR_EFFECT, _T("/shared/effects/cast_indicator.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_EXP_INDICATOR_EFFECT, _T("/shared/effects/exp/exp.effect"), RES_EFFECT);

    // client sounds stringtable
    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        m_hClientSounds = g_ResourceManager.Register(_T("/stringtables/client_sounds.str"), RES_STRINGTABLE);
    }
    CStringTable *pClientSounds(g_ResourceManager.GetStringTable(m_hClientSounds));

    AddResourceToLoadingQueue(CLIENT_RESOURCE_PLAYER_TALKING_EFFECT, _T("/shared/effects/talking.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_PLAYER_DISCONNECTED_EFFECT, _T("/shared/effects/disconnected.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_ILLUSION_EFFECT, _T("/shared/effects/illusion.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_PING_PROTECT, _T("/shared/effects/ally_target.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_PING_ATTACK, _T("/shared/effects/enemy_target.effect"), RES_EFFECT);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_PING_ATTACK_HERO, _T("/shared/effects/Enemy_Hero_Target.effect"), RES_EFFECT);

    uint uiNumVoices(Host.GetNumAnnouncerVoices());

    if (pClientSounds)
    {
        for (uint ui(0); ui <= uiNumVoices; ++ui)
        {
            const tstring &sSet(ui == uiNumVoices ? TSNULL : Host.GetAnnouncerVoiceSet(ui));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("start_game"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("first_blood"));
            
            for (int i(0); i < 9; ++i)
                AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("killstreak") + XtoA(i));

            for (int i(0); i < 4; ++i)
                AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("multikill") + XtoA(i));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("team_killstreak"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("team_wipe"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("denied"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("get_it_on"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("hellbourne_wins"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("legion_wins"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("victory"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("defeat"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("rival"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("payback"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("smackdown"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("humiliation"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("immortal"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("rage_quit"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("hellbourne_destroyed_tower"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("legion_destroyed_tower"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("hellbourne_barracks_destroyed"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("legion_barracks_destroyed"));       
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("base_under_attack"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("ally_hero_under_attack"));
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("our_hero_under_attack"));

            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("unpausing"));
            
            AddAnnouncerVoiceSampleToLoadingQueue(sSet, _T("kongor_slain"));
        }

        AddResourceToLoadingQueue(CLIENT_RESOURCE_SELECT_ITEM_SAMPLE, pClientSounds->Get(_T("select_item")), RES_SAMPLE);
        AddResourceToLoadingQueue(CLIENT_RESOURCE_PLACE_ITEM_SAMPLE, pClientSounds->Get(_T("place_item")), RES_SAMPLE);
        AddResourceToLoadingQueue(CLIENT_RESOURCE_PICKUP_ITEM_SAMPLE, pClientSounds->Get(_T("pickup_item")), RES_SAMPLE);

        AddResourceToLoadingQueue(CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE, pClientSounds->Get(_T("countdown_started")), RES_SAMPLE);
    }

    AddResourceToLoadingQueue(CLIENT_RESOURCE_ALT_INFO_INTERFACE, _T("/ui/alt_info.interface"), RES_INTERFACE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE, _T("/ui/alt_info_creep.interface"), RES_INTERFACE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE, _T("/ui/alt_info_hero.interface"), RES_INTERFACE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE, _T("/ui/alt_info_building.interface"), RES_INTERFACE);

    AddResourceToLoadingQueue(CLIENT_RESOURCE_PROP_TYPE_TABLE, _T("/world/props/PropTypes.str"), RES_STRINGTABLE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_SELECTION_INDICATOR, g_unitSelectionIndicatorPath, RES_MATERIAL);

    m_pClientCommander->LoadResources(m_hClientSounds);

    m_pInterfaceManager->LoadGameInterfaces();

    m_hPopupFont = g_ResourceManager.LookUpName(_T("littletextpopup"), RES_FONTMAP);

    AddResourceToLoadingQueue(CLIENT_RESOURCE_PING_EFFECT, _T("/shared/effects/ping.effect"), RES_EFFECT);

    AddResourceToLoadingQueue(CLIENT_RESOURCE_HERO_SELECT_SAMPLE, _T("/shared/sounds/ui/hero_select/button_ready2.wav"), RES_SAMPLE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_HERO_BAN_SAMPLE, _T("/shared/sounds/ui/hero_select/button_ban.wav"), RES_SAMPLE);

    AddResourceToLoadingQueue(CLIENT_RESOURCE_VOTE_CALLED_SAMPLE, _T("/shared/sounds/symbolic/vote_call.wav"), RES_SAMPLE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_VOTE_PASSED_SAMPLE, _T("/shared/sounds/symbolic/vote_pass.wav"), RES_SAMPLE);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_VOTE_FAILED_SAMPLE, _T("/shared/sounds/symbolic/vote_fail.wav"), RES_SAMPLE);

    (void)std::unique(m_deqClientResources.begin(), m_deqClientResources.end());

    // Dynamic entity definitions
    CStateBlock *pEntityStateBlock(m_pHostClient->GetStateBlock(STATE_BLOCK_ENTITY_TYPES));
    if (pEntityStateBlock != nullptr)
    {
        IBuffer &cDynamicEntities(pEntityStateBlock->GetBuffer());
        cDynamicEntities.Rewind();
        uint uiNumDynamicEntities(cDynamicEntities.GetLength() / 6);
        for (uint ui(0); ui < uiNumDynamicEntities; ++ui)
        {
            ushort unTypeID(cDynamicEntities.ReadShort());
            uint uiNetResIndex(cDynamicEntities.ReadInt());

            EntityRegistry.RegisterDynamicEntity(unTypeID, NetworkResourceManager.GetLocalHandle(uiNetResIndex));
        }
    }

    m_uiTotalResourcesToLoad = INT_SIZE(m_deqClientResources.size());
    m_uiTotalResourcesToLoad += INT_SIZE(m_deqReplayHeroResources.size());
    m_bStartedLoadingResources = true;
}


/*====================
  CGameClient::LoadNextResource
  ====================*/
void    CGameClient::LoadNextResource()
{
    // Precache replay resources if necessary
    if (!m_deqReplayHeroResources.empty())
    {
        tstring sPrevCtxCategory(g_ResourceInfo.GetGameContextCategory());
        g_ResourceInfo.SetGameContextCategory(_T("heroes"));
        {
            uint uiStartTime(K2System.Milliseconds());
            while (!m_deqReplayHeroResources.empty() && K2System.Milliseconds() - uiStartTime < 50)
            {
                const SHeroPrecache& sHeroPrecache(m_deqReplayHeroResources.front());
                if (!sHeroPrecache.sName.empty())
                    EntityRegistry.ClientPrecache(EntityRegistry.LookupID(sHeroPrecache.sName), sHeroPrecache.eScheme, sHeroPrecache.sModifier);
                m_deqReplayHeroResources.pop_front();
            }
        }
        g_ResourceInfo.SetGameContextCategory(sPrevCtxCategory);
    }

    // If we're not done precaching replay resources...
    if (!m_deqReplayHeroResources.empty())
        return;

    if (m_deqClientResources.empty())
        return;

    SClientResourceLoadRequest &resource(m_deqClientResources.front());
    switch (resource.m_iType)
    {
    case RES_SAMPLE:
        if (resource.m_eID != CLIENT_RESOURCE_UNTRACKED)
        {
            K2_WITH_GAME_RESOURCE_SCOPE()
                m_ahResources[resource.m_eID] = g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(resource.m_sName, SND_2D), RES_SAMPLE);
        }
        else
        {
            m_ahResources[resource.m_eID] = RegisterSample(resource.m_sName);
        }
        break;

    case RES_INTERFACE:
        m_ahResources[resource.m_eID] = m_pInterfaceManager->LoadGameInterface(resource.m_sName);
        if (resource.m_eID != CLIENT_RESOURCE_UNTRACKED)
        {
            CInterface *pInterface(UIManager.GetInterface(m_ahResources[resource.m_eID]));
            if (pInterface != nullptr)
                pInterface->Show();
        }
        break;

    case RES_TEXTURE:
        m_ahResources[resource.m_eID] = RegisterIcon(resource.m_sName);
        break;

    case RES_ENTITY_DEF:
        //Console << _T("Precache RES_ENTITY_DEF ") << SingleQuoteStr(resource.m_sName) << newl;
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(resource.m_sName), PRECACHE_ALL, TSNULL);
        break;

    default:
        K2_WITH_GAME_RESOURCE_SCOPE()
            m_ahResources[resource.m_eID] = g_ResourceManager.Register(resource.m_sName, resource.m_iType);
        break;
    }

    if (resource.m_eID == CLIENT_RESOURCE_ENTITY_STRING_TABLE)
    {
        static CEntityStringTableWatcher s_EntityStringTableWatcher;
        CStringTable *pEntityStringTable(g_ResourceManager.GetStringTable(m_ahResources[resource.m_eID]));
        if (pEntityStringTable != nullptr)
            g_ResourceManager.AddResourceWatcher(&s_EntityStringTableWatcher, m_ahResources[resource.m_eID]);
    }

    m_deqClientResources.pop_front();

    if (!m_bStartedLoadingEntityResources)
    {
        if (m_deqClientResources.empty())
        {
            EntityRegistry.PrecacheScripts();

            CGameMechanics *pGameMechanics(GameClient.GetGameMechanics());
            if (pGameMechanics != nullptr)
                pGameMechanics->PostLoad();

            PostProcessEntities();
        }

        if (m_deqClientResources.empty())
        {
            FinishLoading();
        }
    }
}


/*====================
  CGameClient::FinishLoading
  ====================*/
void    CGameClient::FinishLoading()
{
    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        uint uiNumVoices(Host.GetNumAnnouncerVoices());

        m_vAnnouncerVoices.resize(uiNumVoices + 1);

        // Loads base set first so other sets can fallback on omitted files
        for (uint ui(0); ui <= uiNumVoices; ++ui)
        {
            const tstring &sSet(ui == uiNumVoices ? TSNULL : Host.GetAnnouncerVoiceSet(ui));

            SVoiceSet &cSet(m_vAnnouncerVoices[ui]);

            cSet.sArcadeText = ui == uiNumVoices ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(ui);

            cSet.hStartGame = LookupAnnouncerVoiceSample(sSet, _T("start_game"));
            cSet.hFirstBlood = LookupAnnouncerVoiceSample(sSet, _T("first_blood"));
            
            for (int i(0); i < 9; ++i)
                cSet.hKillstreak[i] = LookupAnnouncerVoiceSample(sSet, _T("killstreak") + XtoA(i));

            for (int i(0); i < 4; ++i)
                cSet.hMultikill[i] = LookupAnnouncerVoiceSample(sSet, _T("multikill") + XtoA(i));

            cSet.hTeamKillstreak = LookupAnnouncerVoiceSample(sSet, _T("team_killstreak"));
            cSet.hTeamWipe = LookupAnnouncerVoiceSample(sSet, _T("team_wipe"));

            cSet.hDenied = LookupAnnouncerVoiceSample(sSet, _T("denied"));

            cSet.hGetItOn = LookupAnnouncerVoiceSample(sSet, _T("get_it_on"));

            cSet.hHellbourneWins = LookupAnnouncerVoiceSample(sSet, _T("hellbourne_wins"));
            cSet.hLegionWins = LookupAnnouncerVoiceSample(sSet, _T("legion_wins"));
            cSet.hVictory = LookupAnnouncerVoiceSample(sSet, _T("victory"));
            cSet.hDefeat = LookupAnnouncerVoiceSample(sSet, _T("defeat"));

            cSet.hRival = LookupAnnouncerVoiceSample(sSet, _T("rival"));
            cSet.hPayback = LookupAnnouncerVoiceSample(sSet, _T("payback"));

            cSet.hSmackdown = LookupAnnouncerVoiceSample(sSet, _T("smackdown"));
            cSet.hHumiliation = LookupAnnouncerVoiceSample(sSet, _T("humiliation"));

            cSet.hImmortal = LookupAnnouncerVoiceSample(sSet, _T("immortal"));

            cSet.hRageQuit = LookupAnnouncerVoiceSample(sSet, _T("rage_quit"));

            cSet.hHellbourneDestroyedTower = LookupAnnouncerVoiceSample(sSet, _T("hellbourne_destroyed_tower"));
            cSet.hLegionDestroyedTower = LookupAnnouncerVoiceSample(sSet, _T("legion_destroyed_tower"));
            cSet.hHellbourneBarracksDestroyed = LookupAnnouncerVoiceSample(sSet, _T("hellbourne_barracks_destroyed"));
            cSet.hLegionBarracksDestroyed = LookupAnnouncerVoiceSample(sSet, _T("legion_barracks_destroyed"));      
            cSet.hBaseUnderAttack = LookupAnnouncerVoiceSample(sSet, _T("base_under_attack"));

            cSet.hAllyHeroUnderAttack = LookupAnnouncerVoiceSample(sSet, _T("ally_hero_under_attack"));
            cSet.hOurHeroUnderAttack = LookupAnnouncerVoiceSample(sSet, _T("our_hero_under_attack"));

            cSet.hUnpausing = LookupAnnouncerVoiceSample(sSet, _T("unpausing"));
            
            cSet.hKongorSlain = LookupAnnouncerVoiceSample(sSet, _T("kongor_slain"));
        }
    }
}


/*====================
  CGameClient::StartLoadingEntityResources
  ====================*/
void    CGameClient::StartLoadingEntityResources()
{
    // Precache each every possible (terraintype,footstep) combination
    {
        TStringSet setTerrainTypes;

        // Get the terrain types
        CWorld* pWorld(GetWorldPointer());
        CStringTable* pTerrainTypesStrTable(pWorld ? pWorld->GetTerrainTypesTable() : nullptr);
        assert(pWorld != nullptr && pTerrainTypesStrTable != nullptr);
        bool bErr(true);
        if (pWorld != nullptr)
        {
            if (pTerrainTypesStrTable != nullptr)
            {
                // Get the terrain types from the terraintypes stringtable
                pTerrainTypesStrTable->GetValues(setTerrainTypes);
                bErr = false;
            }

            // Examine each entity in the world entity list, looking for terrain types that weren't
            // specified in the terraintypes stringtable.
            const WorldEntList& vEntities(pWorld->GetEntityList());
            for (WorldEntList::const_iterator it(vEntities.begin()), itEnd(vEntities.end());
                it != itEnd;
                ++it)
            {
                PoolHandle hEntity(*it);

                CWorldEntity* pEntity(pWorld->GetEntityByHandle(hEntity));
                if (pEntity == nullptr)
                    continue;

                const tstring &sTerrainType(pEntity->GetProperty(_T("effecttype")));
                if (sTerrainType.empty())
                    continue;

                setTerrainTypes.insert(sTerrainType);
            }
        }

        // Default to 'grass' terrain type
        if (bErr)
            setTerrainTypes.insert(tstring(_T("grass")));

        // Generate the final precache list
        tsvector vPrecacheList;

        tsvector vFootstepResPaths;
        vFootstepResPaths.push_back(_T("/shared/effects/footstep_l_*.effect"));
        vFootstepResPaths.push_back(_T("/shared/effects/footstep_r_*.effect"));
        vFootstepResPaths.push_back(_T("/shared/effects/waterhack*.effect"));
        vFootstepResPaths.push_back(_T("/shared/effects/creeps/footstep_l_*.effect"));
        vFootstepResPaths.push_back(_T("/shared/effects/creeps/footstep_r_*.effect"));
        vFootstepResPaths.push_back(_T("/shared/effects/creeps/waterhack*.effect"));
        vFootstepResPaths.push_back(_T("/shared/sounds/footsteps/*_%.wav"));

        // foreach terraintype...
        for (TStringSet_cit it(setTerrainTypes.begin()), itEnd(setTerrainTypes.end());
            it != itEnd;
            ++it)
        {
            const tstring &sTerrainType(*it);
            assert(!sTerrainType.empty());
            if (sTerrainType.empty())
                continue;

            // foreach footstep resource path template...
            for (tsvector_cit it2(vFootstepResPaths.begin()), it2End(vFootstepResPaths.end());
                it2 != it2End;
                ++it2)
            {
                assert(!it2->empty());
                if (it2->empty())
                    continue;

                vPrecacheList.push_back(*it2);
                tstring& sPath(vPrecacheList.back());

                StrReplace(sPath, _T("*"), sTerrainType);
            }
        }

        for (tsvector_it it(vPrecacheList.begin()); it != vPrecacheList.end(); ++it)
        {
            const tstring& vPrecache(*it);

            if (EndsWith(vPrecache, _T(".effect")))
                AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, vPrecache, RES_EFFECT);
            else if (EndsWith(vPrecache, _T(".wav")))
                AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, vPrecache, RES_SAMPLE);
            else
                EX_FATAL(_T("Programmer error: unexpected resource type in gameclient precache list"));
        }
    }

    m_bStartedLoadingEntityResources = true;
    m_uiTotalResourcesToLoad = INT_SIZE(m_deqClientResources.size());
}


/*====================
  CGameClient::LoadNextEntityResource
  ====================*/
void    CGameClient::LoadNextEntityResource()
{
    LoadNextResource();
}


/*====================
  CGameClient::FinishedLoadingWorld
  ====================*/
void    CGameClient::FinishedLoadingWorld()
{
    g_ResourceInfo.SetGameContextCategoryLoaded(_T("world"), true);
    if (res_unloadMapResources)
    {
        g_ResourceInfo.ExecCommandLine(_T("context delete client:prevgame_world"));
        g_ResourceInfo.ExecCommandLine(_T("orphans unregister"));
    }
}


/*====================
  CGameClient::PreFrame
  ====================*/
void    CGameClient::PreFrame()
{
    PROFILE("CGameClient::PreFrame");

    // This needs to be reset before the interface input code runs, so that
    // events in the interface will not be lost
    m_CurrentClientSnapshot.SavePrevButtonStates();
    m_CurrentClientSnapshot.SetActivate(NO_SELECTION);
}


/*====================
  CGameClient::FrameDownloadingReplay
  ====================*/
void    CGameClient::FrameDownloadingReplay()
{
    m_pReplayDownload->Open(m_sLastReplay, FILE_HTTP_WRITETOFILE);

    if (m_pReplayDownload->IsOpen())
    {
        if (m_pReplayDownload->GetFileTarget() != nullptr)
            m_pReplayDownload->GetFileTarget()->Close();

        m_bDownloadingReplay = false;
        ReplayDownloadFinished.Trigger(TSNULL);

        if (m_pReplayDownload->ErrorEncountered())
            FileManager.Delete(_T("~/replays/") + Filename_StripPath(m_sLastReplay));

        ReplayDownloadPercent.Trigger(XtoA(1.0f));

        return;
    }

    uint uiDownloaded(INT_ROUND(m_pReplayDownload->GetProgress().dDownloaded));
    uint uiSize(INT_ROUND(m_pReplayDownload->GetProgress().dSize));

    // Progress
    tsvector vsParams;
    vsParams.push_back(XtoA(float(MIN(uiDownloaded, uiSize)) / 1048576, FMT_NOPREFIX, 0, 2));
    vsParams.push_back(XtoA(float(uiSize) / 1048576, FMT_NOPREFIX, 0, 2));

    ReplayDownloadProgress.Trigger(vsParams);

    if (m_uiLastSpeedUpdate == 0)
    {
        ReplayDownloadTime.Trigger(_T("??:??:??"));
        ReplayDownloadSpeed.Trigger(_T("??"));

        m_uiLastSpeedUpdate = K2System.Milliseconds();
    }

    // Calculate download speed
    if (m_uiLastSpeedUpdate + 1000 < K2System.Milliseconds())
    {
        if (m_uiFirstSpeedUpdate == 0)
            m_uiFirstSpeedUpdate = uiDownloaded;

        float fDownloadSpeed(float(uiDownloaded - m_uiFirstSpeedUpdate) / float(m_uiNumSpeedUpdates));
        
        uint uiHoursRemaining;
        uint uiMinutesRemaining;
        uint uiSecondsRemaining;

        m_uiLastSpeedUpdate += 1000;

        if (fDownloadSpeed > 0)
        {
            uiHoursRemaining = (float(uiSize - uiDownloaded) / fDownloadSpeed) / 3600;
            uiMinutesRemaining = ((float(uiSize - uiDownloaded) / fDownloadSpeed) - (uiHoursRemaining * 3600)) / 60;
            uiSecondsRemaining = (float(uiSize - uiDownloaded) / fDownloadSpeed) - (uiMinutesRemaining * 60) - (uiHoursRemaining * 3600);

            ReplayDownloadTime.Trigger(XtoA(uiHoursRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutesRemaining, FMT_PADZERO, 2) + _T(":") + XtoA(uiSecondsRemaining, FMT_PADZERO, 2));
        }
        else
            ReplayDownloadTime.Trigger(_T("??:??:??"));


        ReplayDownloadSpeed.Trigger(XtoA(fDownloadSpeed / 1024, FMT_NOPREFIX, 0, 2));
        m_uiNumSpeedUpdates++;
    }

    ReplayDownloadPercent.Trigger(XtoA(m_pReplayDownload->GetProgressPercent()));
}


/*====================
  CGameClient::Frame
  ====================*/
void    CGameClient::Frame()
{
    PROFILE("CGameClient::Frame");

    FetchGameMechanics();

    m_pClientCommander->SetPlayer(m_pLocalPlayer);
    m_pClientCommander->SetCamera(m_pCamera);
    m_pClientCommander->SetCursorPos(Input.GetCursorPos());

    if (m_pHostClient->GetIndex() != Host.GetActiveClientIndex())
    {
        BackgroundFrame(false);
        return;
    }

    if (m_bDownloadingReplay)
        FrameDownloadingReplay();

    if (m_pHostClient->GetState() < CLIENT_STATE_IN_GAME)
    {
        BackgroundFrame(true);
        m_pInterfaceManager->Update();
        return;
    }

    if (m_uiLastGamePhase != GetGamePhase())
    {
        GamePhaseChanged();
        m_uiLastGamePhase = GetGamePhase();
    }

    if (!m_deqHeroAnnouncements.empty() && (m_hHeroAnnounce == INVALID_INDEX || !K2SoundManager.IsHandleActive(m_hHeroAnnounce)))
    {
        m_hHeroAnnounce = K2SoundManager.Play2DSound(m_deqHeroAnnouncements.front());
        m_deqHeroAnnouncements.pop_front();
    }

    SetReplayClient(replay_client);

    ScriptFrame();

    switch (GetGamePhase())
    {
    case GAME_PHASE_IDLE:
    case GAME_PHASE_WAITING_FOR_PLAYERS:
    case GAME_PHASE_HERO_BAN:
    case GAME_PHASE_HERO_SELECT:
        SetupFrame();
        break;
    case GAME_PHASE_HERO_LOADING:
        LoadHeroesFrame();
        break;

    case GAME_PHASE_PRE_MATCH:
    case GAME_PHASE_ACTIVE:
    case GAME_PHASE_ENDED:
        ActiveFrame();
        break;
    }
}


/*====================
  CGameClient::PrecacheAll
  ====================*/
void    CGameClient::PrecacheAll()
{
    ICvar::UnprotectTransmitCvars();

    Console.ExecuteScript(_T("/game_settings.cfg"));

    PrecacheClientResources();

    //
    // World Resources
    //

    class CLoadWorldFunctions : public CLoadJob<tsvector>::IFunctions
    {
    private:
        tstring         m_sTitle;
        EResourceType   m_eType;

    public:
        CLoadWorldFunctions(const tstring &sTitle, EResourceType e) : m_sTitle(sTitle), m_eType(e)  {}

        float   Frame(tsvector_it &it, float f) const       { SetTitle(m_sTitle); SetProgress(f); return 0.0f; }
        float   PostFrame(tsvector_it &it, float f) const
        {
            ResHandle h(g_ResourceManager.Register(*it, m_eType));
            if (m_eType == RES_MODEL)
                g_ResourceManager.PrecacheSkin(h, uint(-1));
            ++it;
            return 1.0f;
        }
    };

    class CLoadTextures : public CLoadJob<tsvector>::IFunctions
    {
    private:
        tstring         m_sTitle;
        EResourceType   m_eType;

    public:
        CLoadTextures(const tstring &sTitle, EResourceType e) : m_sTitle(sTitle), m_eType(e)    {}
        ~CLoadTextures() {}

        float   Frame(tsvector_it &it, float f) const       { SetTitle(m_sTitle); SetProgress(f); return 0.0f; }
        float   PostFrame(tsvector_it &it, float f) const
        {
            tstring sShortName(Filename_GetName(*it));
            bool bNormalmap(sShortName.length() >= 2 && sShortName.substr(sShortName.length() - 2, tstring::npos) == _T("_n"));
            g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CTexture)(*it, TEXTURE_2D, 0, bNormalmap ? TEXFMT_NORMALMAP : TEXFMT_A8R8G8B8), RES_TEXTURE);
            ++it;
            return 1.0f;
        }
    };

    class CLoadEffects : public CLoadJob<tsvector>::IFunctions
    {
    private:
        tstring         m_sTitle;
        EResourceType   m_eType;

    public:
        CLoadEffects(const tstring &sTitle, EResourceType e) : m_sTitle(sTitle), m_eType(e) {}
        ~CLoadEffects() {}

        float   Frame(tsvector_it &it, float f) const       { SetTitle(m_sTitle); SetProgress(f); return 0.0f; }
        float   PostFrame(tsvector_it &it, float f) const
        {
            g_ResourceManager.Register(*it, RES_EFFECT);
            ++it;
            return 1.0f;
        }
    };

    tsvector vFileList;

    // Load Textures
    vFileList.clear();
    FileManager.GetFileList(_T("/world/foliage/textures/"), _T("*"), false, vFileList);
    FileManager.GetFileList(_T("/world/terrain/textures/"), _T("*"), false, vFileList);
    CLoadTextures fnLoadTerrain(_T("Loading textures"), RES_TEXTURE);
    CLoadJob<tsvector>  jobTerrain(vFileList, &fnLoadTerrain, LOADING_DISPLAY_LOGO);
    jobTerrain.Execute(vFileList.size());

    // Load Models
    vFileList.clear();
    FileManager.GetFileList(_T("/world/props/"), _T("*.mdf"), true, vFileList);
    FileManager.GetFileList(_T("/world/river/"), _T("*.mdf"), true, vFileList);
    FileManager.GetFileList(_T("/world/cliffs/"), _T("*.mdf"), true, vFileList);
    FileManager.GetFileList(_T("/buildings/"), _T("*.mdf"), true, vFileList);
    FileManager.GetFileList(_T("/tools/"), _T("*.mdf"), true, vFileList);
    CLoadWorldFunctions fnLoadProps(_T("Loading models"), RES_MODEL);
    CLoadJob<tsvector>  jobProps(vFileList, &fnLoadProps, LOADING_DISPLAY_LOGO);
    jobProps.Execute(vFileList.size());

    // Load Materials
    vFileList.clear();
    FileManager.GetFileList(_T("/world/sky/"), _T("*.material"), true, vFileList);
    vFileList.push_back(_T("/shared/materials/area_affector.material"));
    CLoadWorldFunctions fnLoadSkys(_T("Loading materials"), RES_MATERIAL);
    CLoadJob<tsvector>  jobSkys(vFileList, &fnLoadSkys, LOADING_DISPLAY_LOGO);
    jobSkys.Execute(vFileList.size());

    // Load Effects
    vFileList.clear();
    FileManager.GetFileList(_T("/shared/effects/weather/"), _T("*.effect"), true, vFileList);
    CLoadWorldFunctions fnLoadWeather(_T("Loading effects"), RES_EFFECT);
    CLoadJob<tsvector>  jobWeather(vFileList, &fnLoadWeather, LOADING_DISPLAY_LOGO);
    jobWeather.Execute(vFileList.size());

    //
    // Entity Resources
    //
    {
        map<ushort, tstring> mapEntities;
        EntityRegistry.GetEntityList(mapEntities);

        vector<ushort> vPrecacheList;
        for (map<ushort, tstring>::iterator it(mapEntities.begin()); it != mapEntities.end(); ++it)
            vPrecacheList.push_back(it->first);

        class CLoadEntityFunctions : public CLoadJob<vector<ushort> >::IFunctions
        {
        public:
            float   Frame(vector<ushort>::iterator &it, float f) const
            {
                SetTitle(_T("Loading entity resources"));
                SetProgress(f);
                return 0.0f;
            }

            float   PostFrame(vector<ushort>::iterator &it, float f) const  { EntityRegistry.ClientPrecache(*it, PRECACHE_ALL, _T("All")); ++it; return 1.0f; }
        };
        CLoadEntityFunctions fnLoadEntity;
        CLoadJob<vector<ushort> > jobLoadEntities(vPrecacheList, &fnLoadEntity, LOADING_DISPLAY_LOGO);
        jobLoadEntities.Execute(vPrecacheList.size());
    }

    //
    // Minimaps and loading images
    //
    {
        vFileList.clear();
        FileManager.GetFileList(_T("/maps/"), _T("*.tga"), true, vFileList);

        class CLoadFunctions : public CLoadJob<tsvector>::IFunctions
        {
        public:
            float   Frame(tsvector_it &it, float f) const
            {
                SetTitle(_T("Loading entity resources"));
                SetProgress(f);
                return 0.0f;
            }

            float   PostFrame(tsvector_it &it, float f) const
            {
                g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CTexture)(*it, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
                ++it;
                return 1.0f;
            }
        };
        CLoadFunctions fnLoad;
        CLoadJob<tsvector> jobLoad(vFileList, &fnLoad, LOADING_DISPLAY_LOGO);
        jobLoad.Execute(vFileList.size());
    }

    UIManager.LoadInterface(_T("/ui/updater.interface"));
    UIManager.LoadInterface(_T("/ui/game_replay_control.interface"));

    //
    // Upgrades
    //
    {
        vFileList.clear();

        uint uiNumChatSymbols(Host.GetNumChatSymbols());
        for (uint ui(0); ui < uiNumChatSymbols; ++ui)
            vFileList.push_back(Host.GetChatSymbolTexturePath(ui));

        uint uiNumChatNameColors(Host.GetNumChatNameColors());
        for (uint ui(0); ui < uiNumChatNameColors; ++ui)
            vFileList.push_back(Host.GetChatNameColorTexturePath(ui));

        uint uiNumAccountIcons(Host.GetNumAccountIcons());
        for (uint ui(0); ui < uiNumAccountIcons; ++ui)
            vFileList.push_back(Host.GetAccountIconTexturePath(ui));

        class CLoadFunctions : public CLoadJob<tsvector>::IFunctions
        {
        public:
            float   Frame(tsvector_it &it, float f) const
            {
                SetTitle(_T("Loading upgrade icons"));
                SetProgress(f);
                return 0.0f;
            }

            float   PostFrame(tsvector_it &it, float f) const
            {
                g_ResourceManager.Register(K2_NEW(ctx_GameClient, CTexture)(*it, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
                ++it;
                return 1.0f;
            }
        };
        CLoadFunctions fnLoad;
        CLoadJob<tsvector> jobLoad(vFileList, &fnLoad, LOADING_DISPLAY_LOGO);
        jobLoad.Execute(vFileList.size());
    }
}


/*====================
  CGameClient::LoadHeroesFrame
  ====================*/
CVAR_BOOL(_test_load_heroes, false);
CVAR_BOOL(_test_load_heroes_list, false);
CVAR_STRING(_test_heroes_list,      "Hero_Accursed,Hero_Devourer,Hero_Fade,Hero_Scout,Hero_Tempest,Hero_Arachna,Hero_Magmar,Hero_Bombardier,Hero_Bubbles");
CVAR_STRING(_test_heroes_list2,     "Hero_Pyromancer,Hero_Magmar,Hero_Predator,Hero_Scout,Hero_Frosty,Hero_Hellbringer,Hero_Tempest,Hero_Devourer,Hero_Mumra");
void    CGameClient::LoadHeroesFrame()
{
    PROFILE("CGameClient::LoadHeroesFrame");

    m_bShowLobby = false;

    Draw2D.SetColor(BLACK);
    Draw2D.Clear();

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    if (m_uiDelayHeroLoading > 0)
    {
        if (m_pHostClient->GetClientFrameLength() > m_uiDelayHeroLoading)
            m_uiDelayHeroLoading = 0;
        else
            m_uiDelayHeroLoading -= m_pHostClient->GetClientFrameLength();
    }

    if (!m_bStartedLoadingHeroes)
    {
        g_ResourceInfo.ExecCommandLine(_T("context move client:curgame_heroes client:prevgame_heroes"));
        g_ResourceInfo.SetGameContextCategoryLoaded(_T("heroes"), false);

        int iSelfTeam(TEAM_INVALID);
        if (m_pLocalPlayer != nullptr)
            iSelfTeam = m_pLocalPlayer->GetTeam();

        m_zTotalHeroesToLoad =  0;
        const PlayerMap &mapPlayers(GetPlayerMap());
        for (PlayerMap_cit it(mapPlayers.begin()); it != mapPlayers.end(); ++it)
        {
            CPlayer *pPlayer(it->second);

            if (pPlayer->GetTeam() == TEAM_SPECTATOR)
                continue;

            ushort unHeroID(pPlayer->GetSelectedHero());
            if (unHeroID == INVALID_ENT_TYPE)
                continue;

            EPrecacheScheme eScheme(PRECACHE_ALL);
            if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
                eScheme = PRECACHE_ALL;
            else if (pPlayer == m_pLocalPlayer)
                eScheme = PRECACHE_SELF;
            else if (pPlayer->GetTeam() == iSelfTeam)
                eScheme = PRECACHE_ALLY;
            else
                eScheme = PRECACHE_OTHER;

            CHeroDefinition *pBaseHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHeroID));
            CHeroDefinition *pHeroDef = pBaseHeroDef;
            if (pHeroDef != nullptr && pPlayer->HasSelectedAvatar())
                pHeroDef = static_cast<CHeroDefinition*>(pHeroDef->GetModifiedDefinition(pPlayer->GetSelectedAvatar()));

            if (pHeroDef == nullptr)
                pHeroDef = pBaseHeroDef;

            GetPrecacheList(pHeroDef->GetName(), eScheme, EntityRegistry.LookupModifierKey(pHeroDef->GetModifierID()), m_deqHeroesToLoad);
        }

        if (_test_load_heroes)
        {
            GetPrecacheList(_T("Hero_Pyromancer"), PRECACHE_ALLY, _T("Female"), m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Magmar"), PRECACHE_ALLY, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Predator"), PRECACHE_ALLY, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Scout"), PRECACHE_ALLY, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Frosty"), PRECACHE_OTHER, _T("Female"), m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Hellbringer"), PRECACHE_OTHER, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Tempest"), PRECACHE_OTHER, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Devourer"), PRECACHE_OTHER, TSNULL, m_deqHeroesToLoad);
            GetPrecacheList(_T("Hero_Mumra"), PRECACHE_OTHER, TSNULL, m_deqHeroesToLoad);
        }
        else if (_test_load_heroes_list)
        {
            tsvector vHeroes;
            SplitBy(vHeroes, _test_heroes_list, _T(","));
            for (size_t i(0); i < 5 && i < vHeroes.size(); ++i)
            {
                const tstring& vAllyHero(vHeroes[i]);
                GetPrecacheList(vAllyHero, PRECACHE_ALLY, TSNULL, m_deqHeroesToLoad);
            }
            for (size_t i(5); i < vHeroes.size(); ++i)
            {
                const tstring& vEnemyHero(vHeroes[i]);
                GetPrecacheList(vEnemyHero, PRECACHE_OTHER, TSNULL, m_deqHeroesToLoad);
            }
        }

        m_zTotalHeroesToLoad = m_deqHeroesToLoad.size();
        m_bStartedLoadingHeroes = true;

        (void)std::unique(m_deqHeroesToLoad.begin(), m_deqHeroesToLoad.end());

        // Notify the chat server the match has started... Weird place to put it, but better than adding some hacky code elsewhere.
        ChatManager.MatchStarted();
    }
    else if (m_uiDelayHeroLoading == 0)
    {
        uint uiStartTime(K2System.Milliseconds());

        while (K2System.Milliseconds() - uiStartTime < 50)
        {
            if (!m_deqHeroesToLoad.empty())
            {
                tstring sPrevCtxCategory(g_ResourceInfo.GetGameContextCategory());
                g_ResourceInfo.SetGameContextCategory(_T("heroes"));
                Precache(m_deqHeroesToLoad.front().sName, m_deqHeroesToLoad.front().eScheme, m_deqHeroesToLoad.front().sModifier);
                g_ResourceInfo.SetGameContextCategory(sPrevCtxCategory);

                m_deqHeroesToLoad.pop_front();
            }
        }

        CBufferFixed<5> buffer;
        float fProgress(1.0f - (m_deqHeroesToLoad.size() / float(m_zTotalHeroesToLoad)));
        buffer << GAME_CMD_LOADING_PROGRESS << fProgress;
        SendGameData(buffer, true);

        if (m_deqHeroesToLoad.empty())
        {
            g_ResourceInfo.SetGameContextCategoryLoaded(_T("heroes"), true);
            if (res_unloadMapResources)
            {
                g_ResourceInfo.ExecCommandLine(_T("context delete client:prevgame_heroes"));
                g_ResourceInfo.ExecCommandLine(_T("orphans unregister"));
            }

            CBufferFixed<1> buffer;
            buffer << GAME_CMD_FINISHED_LOADING_HEROES;
            SendGameData(buffer, true);

            // after we've finished loading, mark all gameplay options so that their current values
            // are sent to the server.  This ensures that the server always knows the correct values
            // for the gameplay options.
            if (m_pClientCommander)
                m_pClientCommander->ForceResendGameplayOptions();

            PlayerList.Trigger(TSNULL);
        }
    }

    m_pInterfaceManager->Update();

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
    }

    Input.ExecuteBinds(BINDTABLE_GAME, 0);
}


/*====================
  CGameClient::SetupFrame
  ====================*/
void    CGameClient::SetupFrame()
{
    PROFILE("CGameClient::SetupFrame");

    // reset variables once when the match starts
    m_bShowLobby = false;   
    m_bJustStartedGame = true;
    cg_lockCamera = false;
    cg_fogofwar = true;

    UpdateNotifyFlags();

    Draw2D.SetColor(BLACK);
    Draw2D.Clear();

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(m_pHostClient->GetLerpValue());

    m_pInterfaceManager->Update();

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
    }

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    // Generate a new client snapshot
    if (m_pHostClient->GetServerFrame() > 1)
        SendClientSnapshot();

    // Check for countdown start
    if (GetGamePhase() == GAME_PHASE_WAITING_FOR_PLAYERS)
    {
        if (GetPhaseEndTime() != INVALID_TIME)
        {
            ChatManager.SetFocusedChannel(-2, true);

            const uint uiRemainingTime(GetPhaseEndTime() - GetGameTime());
            if (uiRemainingTime <= 5000 && uiRemainingTime / 1000 != (uiRemainingTime + GetFrameLength()) / 1000)
            {
                if (cg_drawMessages)
                    GameMessage.Trigger(XtoA((uiRemainingTime / 1000) + 1) + _T("..."));
                    
                if (m_ahResources[CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE] != INVALID_RESOURCE)              
                    K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE]);
            }
        }
    }
}


/*====================
  CGameClient::InterfaceNeedsUpdate
  ====================*/
bool    CGameClient::InterfaceNeedsUpdate()
{
    if (UIManager.GetActiveInterface() == nullptr)
        return false;

    bool bUpdate(UIManager.GetActiveInterface()->NeedsRefresh());

    UIManager.GetActiveInterface()->NeedsRefresh(false);

    return bUpdate;
}


/*====================
  CGameClient::GetCurrentInterface
  ====================*/
EGameInterface  CGameClient::GetCurrentInterface() const
{
    if (m_pHostClient->GetState() == CLIENT_STATE_IDLE)
        return CG_INTERFACE_MAIN;

    if (m_pHostClient->GetState() < CLIENT_STATE_IN_GAME)
    {
        if (m_pHostClient->IsSilentConnect())
            return CG_INTERFACE_MAIN;
        else
            return CG_INTERFACE_LOADING;
    }

    if (GetGamePhase() < GAME_PHASE_HERO_BAN)
        return CG_INTERFACE_LOBBY;

    if (m_pLocalPlayer == nullptr)
        return CG_INTERFACE_LOBBY;

    if (GetGamePhase() == GAME_PHASE_HERO_LOADING || !IsFinishedLoadingHeroes())
        return CG_INTERFACE_HERO_LOADING;

    if (GetGamePhase() < GAME_PHASE_PRE_MATCH)
        return CG_INTERFACE_HERO_SELECT;

    if (m_pLocalPlayer->GetHero() == nullptr && m_pLocalPlayer->GetTeam() != TEAM_SPECTATOR && !GetGameInfo()->GetNoHeroSelect())
        return CG_INTERFACE_HERO_SELECT;

    return m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR ? CG_INTERFACE_GAME_SPECTATOR : CG_INTERFACE_GAME;
}


/*====================
  CGameClient::UpdateCamera
  ====================*/
void    CGameClient::UpdateCamera()
{
    if (m_pLocalPlayer->GetCameraIndex() != INVALID_INDEX)
    {
        IGameEntity *pEntity(GetEntity(m_pLocalPlayer->GetCameraIndex()));
        if (pEntity != nullptr && pEntity->IsType<CEntityCamera>())
        {
            CEntityCamera *pCamera(pEntity->GetAs<CEntityCamera>());

            CVec3f v3CenterPos(pCamera->GetPosition());

            v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
            GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
        }
    }
    else
    {
        if (cam_mode == 1)
        {
            if (Center || PortraitCenter)
            {
                IUnitEntity *pUnit(m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR ? m_pClientCommander->GetSelectedInfoEntity() : m_pClientCommander->GetSelectedControlEntity());
                if (pUnit != nullptr)
                {
                    GameClient.GetCurrentSnapshot()->SetAngles(M_GetAnglesFromForwardVec(pUnit->GetPosition() - GameClient.GetCurrentSnapshot()->GetCameraPosition()));
                }
            }
            else if (CenterInfo || PortraitCenterInfo)
            {
                IUnitEntity *pUnit(m_pClientCommander->GetSelectedInfoEntity());
                if (pUnit == nullptr)
                    pUnit = m_pClientCommander->GetSelectedControlEntity();

                if (pUnit != nullptr)
                {
                    GameClient.GetCurrentSnapshot()->SetAngles(M_GetAnglesFromForwardVec(pUnit->GetPosition() - GameClient.GetCurrentSnapshot()->GetCameraPosition()));
                }
            } 
        }
        else
        {
            if (!cg_altLockCamera && (Center || PortraitCenter || cg_lockCamera || cg_lockCameraGame))
            {
                IUnitEntity *pUnit(m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR ? m_pClientCommander->GetSelectedInfoEntity() : m_pClientCommander->GetSelectedControlEntity());
                if (pUnit != nullptr)
                {
                    CVec3f v3CenterPos(pUnit->GetPosition());

                    v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
                    GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
                }
            }
            else if (cg_altLockCamera && (cg_lockCameraGame^Center || PortraitCenter || cg_lockCamera))
            {
                IUnitEntity *pUnit(m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR ? m_pClientCommander->GetSelectedInfoEntity() : m_pClientCommander->GetSelectedControlEntity());
                if (pUnit != nullptr)
                {
                    bool bIsMouseOverGui(false);
                    int WType(WIDGET_INVALID);
                        //get the current widget under the mouse
                    CInterface *pInterface(UIManager.GetActiveInterface());
                    if (pInterface)
                    {
                        IWidget *pWidget(pInterface->GetHoverWidget());
                        if(pWidget)
                        {
                                //we need the type to check for the mini map
                            WType = pWidget->GetType();
                        }
                    }
                        
                        //Check to see if were over the gui and the mouse is free and were alive
                    if(!Input.IsCursorFrozen() && !bIsMouseOverGui && pUnit->GetStatus() == ENTITY_STATUS_ACTIVE && 
                            //If the mod is 0 then just center
                        cg_lockCameraMod &&
                        !m_pInterfaceManager->IsShopVisible() &&
                        !pInterface->GetExclusiveWidget() &&
                            //Check to see if the mouse is in bounds of the game window
                        ( Input.GetCursorPos().x > UIManager.GetActiveInterface()->GetSceneX() - 1.0f &&  
                        Input.GetCursorPos().y > UIManager.GetActiveInterface()->GetSceneY() - 1.0f &&
                        Input.GetCursorPos().x < UIManager.GetActiveInterface()->GetSceneWidth() && 
                        Input.GetCursorPos().y < UIManager.GetActiveInterface()->GetSceneHeight() - 25.0f ) // The gui overlays a little bit
                            //Check to see if were over the MiniMap
                        && WType != WIDGET_MAP)
                    {
                            //Reset Mouse Down test
                        m_bMouseDownOverMiniMap = false;
                            //MikeG If Centered offset the view by the mouse pos like Bloodline of Champions
                        CVec3f v3OffsetPos = CVec3f(
                            ((Input.GetCursorPos().x / UIManager.GetActiveInterface()->GetSceneWidth())-0.5f)*(1200.0f*cg_lockCameraMod), 
                                //Get the Interface Height instead of screen
                            -((Input.GetCursorPos().y / UIManager.GetActiveInterface()->GetSceneHeight())-0.5f)*(1000.0f*cg_lockCameraMod), 
                            pUnit->GetPosition().z ) + pUnit->GetPosition();

                        CVec3f v3CamPos = GameClient.GetCurrentSnapshot()->GetCameraPosition();
                            //MikeG Make it a smooth transition
                        CVec3f v3NewPos = v3CamPos - ((v3CamPos - v3OffsetPos) * 0.15f);
                        v3NewPos.z = v3CamPos.z;
                        GameClient.GetCurrentSnapshot()->SetCameraPosition(v3NewPos); 
                    }
                    else if(pUnit->GetStatus() != ENTITY_STATUS_ACTIVE || (WType == WIDGET_MAP && m_bMouseDownOverMiniMap))
                    {}      //Do Nothing
                    else    //Just Center on the Player
                    { 
                            //Dont let the view go till the mouse button is down altest once
                        m_bMouseDownOverMiniMap = Input.IsButtonDown(BUTTON_MOUSEL) ? true : m_bMouseDownOverMiniMap;
                        CVec3f v3CamPos = GameClient.GetCurrentSnapshot()->GetCameraPosition();

                            //MikeG Make it a smooth transition
                        CVec3f v3NewPos = v3CamPos - ((v3CamPos - pUnit->GetPosition()) * 0.15f);
                        v3NewPos.z = v3CamPos.z;
                        GameClient.GetCurrentSnapshot()->SetCameraPosition(v3NewPos);
                    }
                }
            } 
            else if (CenterInfo || PortraitCenterInfo)
            {
                IUnitEntity *pUnit(m_pClientCommander->GetSelectedInfoEntity());
                if (pUnit == nullptr)
                    pUnit = m_pClientCommander->GetSelectedControlEntity();

                if (pUnit != nullptr)
                {
                    CVec3f v3CenterPos(pUnit->GetPosition());

                    v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
                    GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
                }
            }
        }
    }

    // Camera
    if (GetGamePhase() == GAME_PHASE_ENDED && GameClient.GetGameTime() - Game.GetPhaseStartTime() < cg_endGameInterfaceDelay)
    {
        CVec3f v3CamTarget(GameClient.GetCurrentSnapshot()->GetCameraPosition());

        int iWinningTeam(GetWinningTeam());
        CTeamInfo *pTeam(GetTeam(iWinningTeam ^ 3));
        if (pTeam != nullptr)
        {
            IBuildingEntity *pTargetBuilding(GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
            if (pTargetBuilding != nullptr)
                v3CamTarget = pTargetBuilding->GetPosition();
        }

        CVec3f v3CenterPos(GameClient.GetCurrentSnapshot()->GetCameraPosition());

        v3CenterPos.x = v3CamTarget.x;
        v3CenterPos.y = v3CamTarget.y;
        v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
        GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
    }
}


/*====================
  CGameClient::PrecacheClientResources
  ====================*/
void    CGameClient::PrecacheClientResources()
{
    //
    // Client Resources
    //

    //bool bPrecache(cg_precacheEntities);
    //cg_precacheEntities = true;
    StartLoadingResources();
    while (!IsFinishedLoadingResources())
        LoadNextResource();
    //cg_precacheEntities = bPrecache;
}


/*====================
  CGameClient::UpdateNotifyFlags
  ====================*/
void    CGameClient::UpdateNotifyFlags()
{
    if (Host.IsReplay())
        return;

    // extremely basic mod detection
    if (!m_bWasUsingCustomFiles && FileManager.GetUsingCustomFiles())
    {
        CBufferDynamic buffer;
        buffer << GAME_CMD_NOTIFICATION_FLAGS << int(NOTIFY_FLAG_CUSTOM_FILES) << byte(1);

        // write the reserved fields.
        buffer << int(1) << byte(0);

        SendGameData(buffer, true);

        m_bWasUsingCustomFiles = true;
    }

    // detect when a player has modified their core game files.
    if (!m_bWereCoreFilesModified && FileManager.GetCoreFilesModified())
    {
        CBufferDynamic buffer;
        buffer << GAME_CMD_NOTIFICATION_FLAGS << int(NOTIFY_FLAG_MODIFIED_CORE_FILES) << byte(1);

        // write the reserved fields.
        buffer << int(1) << byte(0);

        SendGameData(buffer, true);

        m_bWereCoreFilesModified = true;
    }
}


/*====================
  CGameClient::ActiveFrame
  ====================*/
void    CGameClient::ActiveFrame()
{
    PROFILE("CGameClient::ActiveFrame");

    if (!IsFinishedLoadingHeroes() || !m_bStartedLoadingHeroes)
    {
        LoadHeroesFrame();
        return;
    }

    // keep this above the following "if (m_bJustStartedGame) { ... }" block.
    UpdateNotifyFlags();
    // if the game has just started, then clear the game list so that the player
    // won't see outdated games in the list when he tries to join another game.
    if (m_bJustStartedGame)
    {
        CHostClient *pClient(Host.GetActiveClient());
        if (pClient != nullptr)
        {
            pClient->ClearServerList();
        }

        // detect whether resources0.s2z has been tampered with.
        byte yVersion(0);
        byte pBaseResources0Checksum[CHECKSUM_SIZE];
        byte pGameResources0Checksum[CHECKSUM_SIZE];
        MemManager.Set(pBaseResources0Checksum, 0, CHECKSUM_SIZE);
        MemManager.Set(pGameResources0Checksum, 0, CHECKSUM_SIZE);
        CArchive *pBaseResources0(FileManager.GetArchive(_T("/resources0.s2z"), _T("base")));
        CArchive *pGameResources0(FileManager.GetArchive(_T("/resources0.s2z"), _T("game")));
        if (pBaseResources0)
            pBaseResources0->HashChecksums(pBaseResources0Checksum);
        if (pGameResources0)
            pGameResources0->HashChecksums(pGameResources0Checksum);
        CBufferFixed<1 + 1 + 16 + 16> buffer;
        buffer << GAME_CMD_VERIFY_FILES << yVersion;
        buffer.Append(pBaseResources0Checksum, CHECKSUM_SIZE);
        buffer.Append(pGameResources0Checksum, CHECKSUM_SIZE);
        SendGameData(buffer, true);

        // Now that the game has started, begin playing6 the desired weather effect
        cg_current_weather.SetModified(true);

        m_bJustStartedGame = false;
    }

    // If the current weather effect was changed, then activate it
    if (cg_current_weather.IsModified())
    {
        // Stop playing all current weather effects
        GameClient.StopAllWeather();
        // Start the selected weather effect
        GameClient.StartWeatherByName(cg_current_weather);
        // Free any resources that were used by the previous weather effect, and aren't used now
        g_ResourceInfo.ExecCommandLine(_TS("orphans unregister"));

        cg_current_weather.SetModified(false);
    }

    uint uiLastGameTime(GetGameTime());

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    if (Host.IsReplay() && GetGameTime() < uiLastGameTime)
    {
        Console.Dev << _T("Rewinding replay to ") << GetGameTime() << _T(" milliseconds") << newl;

        m_pClientEntityDirectory->Rewind();
    }

    if (m_pHostClient->GetState() != CLIENT_STATE_IN_GAME || m_pLocalPlayer == nullptr)
    {
        // This is very bad if it ever happens...
        if (m_pHostClient->GetState() != CLIENT_STATE_IN_GAME)
            Console.Err << _T("Client state invalid! State: ") << XtoA(m_pHostClient->GetState()) << newl;

        if (m_pLocalPlayer == nullptr)
            Console.Err << _T("Local client invalid!") << newl;

        Draw2D.SetColor(BLACK);
        Draw2D.Clear();
        return;
    }

    m_v3CameraEffectAngleOffset.Clear(); 
    m_v3CameraEffectOffset.Clear();
    m_bActiveReflection = false;

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(cg_noLerp ? 1.0f : m_pHostClient->GetLerpValue());
    
    m_pClientCommander->ValidateSelections();

    if (m_uiItemCursorIndex != INVALID_INDEX)
    {
        IGameEntity *pItem(GetEntity(m_uiItemCursorIndex));
        if (pItem == nullptr || !pItem->IsItem())
            m_uiItemCursorIndex = INVALID_INDEX;
    }

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
    }

    if (m_pInterfaceManager->IsShopVisible())
        Input.ExecuteBinds(BINDTABLE_GAME_SHOP, 0);

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    bool bMuteSFX(false);
    switch (GetCurrentInterface())
    {
    case CG_INTERFACE_LOBBY:
        bMuteSFX = true;
        /* fallthrough */

    case CG_INTERFACE_GAME:
    case CG_INTERFACE_GAME_SPECTATOR:
        {
            if (m_bShowMenu)
            {
                m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
                m_CurrentClientSnapshot.SetCursorPosition(V2_ZERO);
            }
            else
            {
                Input.ExecuteBinds(BINDTABLE_GAME_COMMANDER, 0);

                if (GetGamePhase() != GAME_PHASE_ENDED || GameClient.GetGameTime() - Game.GetPhaseStartTime() >= cg_endGameInterfaceDelay)
                {
                    m_CurrentClientSnapshot.SetButton(GAME_CMDR_BUTTON_MOUSELOOK, g_bMouseLook);

                    if (m_pLocalPlayer != nullptr)
                        m_pLocalPlayer->PrepareClientSnapshot(m_CurrentClientSnapshot);

                    m_pClientCommander->PrepareClientSnapshot(m_CurrentClientSnapshot);
                    m_pClientCommander->ValidateSelections();
                }
            }
        }
        break;
    default:
        break;
    }

    UpdateCamera();

    ValidateLevelup();

    // Sound
    WorldSoundsFrame();
    K2SoundManager.MuteSFX(bMuteSFX);

    // Remove any input we used above but didn't discard
    Input.FlushByTable(BINDTABLE_GAME, 0);

    // Generate a new client snapshot
    SendClientSnapshot();

    SceneManager.Clear();

    Draw2D.SetColor(BLACK);
    Draw2D.Clear();

    // Local client
    if (m_pLocalPlayer != nullptr)
    {
        PROFILE("Local client");

        if (ReplayManager.IsPlaying())
        {
            //const CVec3f &v3Angles(m_pLocalPlayer->GetAngles());
            //m_CurrentClientSnapshot.SetCameraAngles(v3Angles[YAW], v3Angles[PITCH]);

            m_pLocalPlayer->ReadClientSnapshot(m_CurrentClientSnapshot);
        }
        else
        {
            m_pLocalPlayer->ReadClientSnapshot(m_CurrentClientSnapshot);
        }

        m_pLocalPlayer->SetFullVision(!cg_fogofwar);

        // Update camera
        CVec3f v3CameraAngles(m_pLocalPlayer->GetAngles());
        CVec3f v3CameraPosition(m_pLocalPlayer->GetPosition());
        
        if (!cg_cameraSmoothing)
        {
            m_v3CameraPosition = v3CameraPosition;              
            m_v3CameraAngles = v3CameraAngles;
        }
        else
        {
            for (int i(X); i <= Z; ++i)
            {
                while (m_v3CameraAngles[i] - v3CameraAngles[i] > 180.0f)
                    m_v3CameraAngles[i] -= 360.0f;

                while (m_v3CameraAngles[i] - v3CameraAngles[i] < -180.0f)
                    m_v3CameraAngles[i] += 360.0f;
            }

            if (m_pHostClient->GetFrameCount() == m_uiCameraFrame + 1 &&
                m_pLocalPlayer->GetIndex() == m_uiCameraIndex)
            {
                m_v3CameraPosition = DECAY(m_v3CameraPosition, v3CameraPosition, ICvar::GetFloat(_T("cam_smoothPositionHalfLife")), GetFrameLength() * SEC_PER_MS);
                m_v3CameraAngles = DECAY(m_v3CameraAngles, v3CameraAngles, ICvar::GetFloat(_T("cam_smoothAnglesHalfLife")), GetFrameLength() * SEC_PER_MS);
            }
            else
            {
                m_v3CameraPosition = v3CameraPosition;
                m_v3CameraAngles = v3CameraAngles;
            }

            m_uiCameraFrame = m_pHostClient->GetFrameCount();
            m_uiCameraIndex = m_pLocalPlayer->GetIndex();
        }

        m_pLocalPlayer->SetIsolated(m_pLocalPlayer->IsIsolated());

        v3CameraPosition = m_v3CameraPosition;
        v3CameraAngles = m_v3CameraAngles + m_v3CameraEffectAngleOffset;

        m_pLocalPlayer->SetupCamera(*m_pCamera, v3CameraPosition, v3CameraAngles);

        m_pCamera->AddOffset(m_v3CameraEffectOffset);

        // Camera
        m_pCamera->SetTime(MsToSec(GetGameTime()));

        m_pCamera->AddFlags(CAM_FOG_OF_WAR | CAM_SHADOW_UNIFORM | CAM_SHADOW_NO_FALLOFF | CAM_SHADOW_SCENE_BOUNDS);
        m_pCamera->SetShadowBias(m_pClientCommander->GetCameraDistance());
        m_pCamera->SetShadowMaxFov(180.0f);

        m_pCamera->SetReflect(CPlane(V_UP, CVec3f(0.0f, 0.0f, Game.GetWaterLevel(m_v3CameraCenter))));

        m_pClientCommander->Frame();
    }

    m_pInterfaceManager->Update();

    CInterface *pActiveInterface(UIManager.GetActiveInterface());
    if (pActiveInterface != nullptr)
    {
        float fWidth(pActiveInterface->GetSceneWidth());
        float fHeight(pActiveInterface->GetSceneHeight());
        
        fWidth = MIN(fWidth, fHeight * cg_maxAspect);

        m_pCamera->SetX(pActiveInterface->GetSceneX() + (pActiveInterface->GetSceneWidth() - fWidth) * 0.5f);
        m_pCamera->SetY(pActiveInterface->GetSceneY());
        m_pCamera->SetWidth(fWidth);
        m_pCamera->SetHeight(fHeight);
    }
    else
    {
        m_pCamera->SetX(0.0f);
        m_pCamera->SetY(0.0f);
        m_pCamera->SetWidth(Vid.GetScreenW());
        m_pCamera->SetHeight(Vid.GetScreenH());
    }

    DrawAreaCast();

    // Render the fog of war texture
    DrawFogofWar();

    SceneManager.PrepCamera(*m_pCamera);

    // Update listener position and scene center
    if (m_pLocalPlayer)
    {
        CVec3f v3Pos(m_pCamera->GetOrigin());
        CVec3f v3End(M_PointOnLine(v3Pos, m_pCamera->GetViewAxis(FORWARD), FAR_AWAY));
        STraceInfo trace;

        if (GameClient.TraceLine(trace, v3Pos, v3End, TRACE_TERRAIN))
        {
            K2SoundManager.SetListenerPosition(LERP(cg_soundPosition, trace.v3EndPos, v3Pos), V3_ZERO, m_pCamera->GetViewAxis(FORWARD), m_pCamera->GetViewAxis(UP), false);
            K2SoundManager.SetCenter(m_pLocalPlayer->GetPosition());
            m_v3CameraCenter = m_pLocalPlayer->GetPosition();
        }
        else
        {
            K2SoundManager.SetListenerPosition(v3Pos, V3_ZERO, m_pCamera->GetViewAxis(FORWARD), m_pCamera->GetViewAxis(UP), false);
            K2SoundManager.SetCenter(m_pLocalPlayer->GetPosition());
            m_v3CameraCenter = m_pLocalPlayer->GetPosition();
        }
    }

    // Draw events 
    EventsFrame();

    // Draw entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->PopulateScene();
    RenderWorldEntities();

    AddClientGameEffects();

    // Redo camera (for effect offsets)
    if (m_pLocalPlayer)
    {
        m_pLocalPlayer->SetupCamera(*m_pCamera, m_v3CameraPosition, m_v3CameraAngles + m_v3CameraEffectAngleOffset);
        m_pCamera->AddOffset(m_v3CameraEffectOffset);
    }

    // Set whether we have an active reflection
    if (m_bActiveReflection)
        m_pCamera->RemoveFlags(CAM_NO_REFLECTIONS);
    else
        m_pCamera->AddFlags(CAM_NO_REFLECTIONS);

    m_pCamera->AddFlags(CAM_NO_DEPTH_CLEAR);

    float fTod(Game.GetTimeOfDay());
    float fTransition(float(g_dayTransitionTime) / g_dayLength * 0.5f);

    fTod -= 0.25f; // start at day

    if (fTod < 0.0f)
        fTod += 1.0f;

    if (fTod <= fTransition)
        fTod = 3.0f + (fTod / (fTransition)); // Sunrise->Day
    else if (fTod <= 0.5f - fTransition)
        fTod = 0.0f; // Day
    else if (fTod <= 0.5f + fTransition)
        fTod = 1.0f + ((fTod - 0.5f) / (fTransition)); // Day->Sunset->Night
    else if (fTod <= 1.0f - fTransition)
        fTod = 2.0f; // Night
    else
        fTod = 3.0f + ((fTod - 1.0f) / (fTransition)); // Night->Sunrise

    SceneManager.SetTimeOfDay(fTod);

    SceneManager.DrawSky(*m_pCamera, MsToSec(m_pHostClient->GetClientFrameLength()));

    DrawNavGrid();
    DrawSelectedPath();

    // Render the main scene
    SceneManager.PrepCamera(*m_pCamera);
    SceneManager.Render();

    RenderSelectedPlayerView(m_pClientCommander->GetSelectedControlEntityIndex());

    // Commander 2D Drawing
    m_pClientCommander->Draw();

    DrawPopupMessages();

    if (cg_drawEntityTypeNames && Host.HasServer())
        m_pClientEntityDirectory->DrawScreen();

    DrawVoiceInfo();
    DrawAltInfo();

    // Screen effects
    for (vector<SOverlayInfo>::iterator itOverlay(m_vOverlays.begin()); itOverlay != m_vOverlays.end(); ++itOverlay)
    {
        Draw2D.SetColor(itOverlay->v4Color);
        Draw2D.Rect(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()), itOverlay->hMaterial);
    }
    m_vOverlays.clear();
    
    UpdateMinimap();
    DrawSelectedStats();

    UpdateRecommendedItems();

    //
    // Lag Indicator
    //

    uint uiOldestReliable(m_pHostClient->GetOldestReliable());
    static tsvector vLagParams(2);

    if (uiOldestReliable > cg_lagThreshold)
    {
        vLagParams[0] = _T("true");
        vLagParams[1] = XtoA(uiOldestReliable);
    }
    else
    {
        vLagParams[0] = _T("false");
        vLagParams[1] = XtoA(uiOldestReliable);
    }

    m_pInterfaceManager->Trigger(UITRIGGER_LAG, vLagParams);

#if 0
    if (m_deqClientSnapshots.size() > 127)
    {
        Draw2D.SetColor(BLACK);
        Draw2D.String(2.0f, 2.0f + Draw2D.GetScreenH() * 0.25f, Draw2D.GetScreenW(), Draw2D.GetScreenH(), _T("Connection Interrupted"), g_ResourceManager.LookUpName(_T("gamelarge"), RES_FONTMAP), DRAW_STRING_CENTER);

        Draw2D.SetColor(WHITE);
        Draw2D.String(0.0f, Draw2D.GetScreenH() * 0.25f, Draw2D.GetScreenW(), Draw2D.GetScreenH(), _T("Connection Interrupted"), g_ResourceManager.LookUpName(_T("gamelarge"), RES_FONTMAP), DRAW_STRING_CENTER);
    }
#endif
}


/*====================
  CGameClient::EndedFrame
  ====================*/
void    CGameClient::EndedFrame()
{
    PROFILE("CGameClient::EndedFrame");

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(m_pHostClient->GetLerpValue());

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
    }

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    // Sound
    WorldSoundsFrame();

    // Remove any input we used above but didn't discard
    Input.FlushByTable(BINDTABLE_GAME, 0);

    // Generate a new client snapshot
    SendClientSnapshot();

    SceneManager.Clear();

    Draw2D.SetColor(BLACK);
    Draw2D.Clear();

    // Camera
    if (m_pLocalPlayer != nullptr)
        m_pLocalPlayer->SetupCamera(*m_pCamera, m_pLocalPlayer->GetPosition(), m_pLocalPlayer->GetAngles());

    CVec3f v3CamStart(m_pCamera->GetOrigin());
    CAxis axisStart(m_pCamera->GetViewAxis());

    CVec3f v3CamTarget(V_ZERO);
    CAxis axisTarget(V_ZERO);
    int iWinningTeam(GetWinningTeam());
    CTeamInfo *pTeam(GetTeam(iWinningTeam ^ 3));
    if (pTeam != nullptr)
    {
        IBuildingEntity *pTargetBuilding(GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
        if (pTargetBuilding != nullptr)
        {
            v3CamTarget = pTargetBuilding->GetPosition();
            axisTarget = CAxis(pTargetBuilding->GetAngles());
        }
    }
    CVec3f v3CamEnd(v3CamTarget + cam_endGameOffset * axisTarget);
    
    float fLerp(CLAMP((Game.GetGameTime() - Game.GetPhaseStartTime()) / float(cam_endGameLerpTime), 0.0f, 1.0f));
    CVec3f v3Pos(LERP(fLerp, v3CamStart, v3CamEnd));

    float fHeight(Game.GetCameraHeight(v3Pos.x, v3Pos.y));
    v3Pos.z = MAX(fHeight + cam_endGameMinHeight, v3Pos.z);
    m_pCamera->SetOrigin(v3Pos);
    m_pCamera->SetTarget(v3CamTarget);

    m_pCamera->SetViewAxis(M_QuatToAxis(M_LerpQuat(fLerp, M_AxisToQuat(axisStart), M_AxisToQuat(m_pCamera->GetViewAxis()))));

    m_pCamera->AddOffset(m_v3CameraEffectOffset);

    // Camera
    m_pCamera->SetTime(MsToSec(GetGameTime()));
    m_pCamera->SetWidth(float(Vid.GetScreenW()));
    m_pCamera->SetHeight(float(Vid.GetScreenH()));
    m_pCamera->SetFovXCalc(m_pLocalPlayer == nullptr ? ICvar::GetFloat(_T("cam_fov")) : 90.0f);
    m_pCamera->RemoveFlags(CAM_FOG_OF_WAR | CAM_NO_FOG);

    // Interface updates
    m_pInterfaceManager->Update();

    // Draw the rest of the entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->PopulateScene();
    RenderWorldEntities();

    // Draw events 
    EventsFrame();

    //
    AddClientGameEffects();

    CVec3f v3Dir(V_ZERO);
    if (fLerp < 1.0f)
    {
        v3Dir = v3CamEnd - v3CamStart;
        float fLength(v3Dir.Normalize());
        v3Dir *= fLength * Game.GetFrameLength() / MsToSec(cam_endGameLerpTime);
    }

    K2SoundManager.SetListenerPosition(m_pCamera->GetOrigin(), v3Dir, m_pCamera->GetViewAxis().Forward(), m_pCamera->GetViewAxis().Up(), false);
    K2SoundManager.SetCenter(m_pCamera->GetOrigin());

    SceneManager.DrawSky(*m_pCamera, MsToSec(Game.GetFrameLength()));

    // Render the main scene
    SceneManager.PrepCamera(*m_pCamera);
    SceneManager.Render();

    DrawPopupMessages();

    DrawVoiceInfo();

    // Screen effects
    for (vector<SOverlayInfo>::iterator itOverlay(m_vOverlays.begin()); itOverlay != m_vOverlays.end(); ++itOverlay)
    {
        Draw2D.SetColor(itOverlay->v4Color);
        Draw2D.Rect(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()), itOverlay->hMaterial);
    }
    m_vOverlays.clear();
}


/*====================
  CGameClient::BackgroundFrame
  ====================*/
void    CGameClient::BackgroundFrame(bool bProcessBinds)
{
    PROFILE("CGameClient::BackgroundFrame");

    // Generate a new client snapshot
    if (m_pHostClient->GetServerFrame() > 1)
        SendClientSnapshot();

    if (bProcessBinds)
        Input.ExecuteBinds(BINDTABLE_GAME, 0);
}


/*====================
  CGameClient::GamePhaseChanged
  ====================*/
void    CGameClient::GamePhaseChanged()
{
    switch (GetGamePhase())
    {
    case GAME_PHASE_IDLE:
        break;

    case GAME_PHASE_WAITING_FOR_PLAYERS:
        break;

    case GAME_PHASE_HERO_BAN:
    case GAME_PHASE_HERO_SELECT:
        ChatManager.UnFollow();
        ChatManager.JoinGameLobby(true);
        WriteConnectionInfo();
        break;

    case GAME_PHASE_HERO_LOADING:
        {
            // Fade out the lobby music
            K2SoundManager.StopMusic(true, true, sound_lobbyFadeout_delayFrames);
        }
        break;

    case GAME_PHASE_PRE_MATCH:
        {
            // Begin playing the world's music playlist immediately after hero loading is complete
            if (!GetWorldPointer()->GetMusicList().empty())
                K2SoundManager.PlayPlaylist(GetWorldPointer()->GetMusicList(), GetWorldPointer()->GetMusicShuffle());
        }
        break;

    case GAME_PHASE_ACTIVE:
    case GAME_PHASE_ENDED:
        break;
    }

    tsvector vParams(2);
    vParams[0] = XtoA(GetGamePhase());
    vParams[1] = XtoA(m_uiLastGamePhase);

    EventGamePhaseChanged.Trigger(vParams);
}


/*====================
  CGameClient::WriteConnectionInfo
  ====================*/
void    CGameClient::WriteConnectionInfo()
{
    if (!m_pHostClient)
        return;
    
    if (GetGameInfo() == nullptr)
        return;
    
    m_pHostClient->SetReconnect(m_pHostClient->GetConnectedAddress(), GetGameInfo()->GetMatchID());

    CFileHandle hFile(_CTS("~/reconnect.cfg"), FILE_TEXT | FILE_WRITE | FILE_TRUNCATE);
    if (!hFile.IsOpen())
    {
        Console.Err << _CTS("Failed to write reconnect.cfg") << newl;
        return;
    }

    hFile << _CTS("cl_reconnectAddress ") << m_pHostClient->GetConnectedAddress() << newl;
    hFile << _CTS("cl_reconnectMatchID ") << GetGameInfo()->GetMatchID() << newl;
    //hFile << _CTS("cl_connectionID ") << m_pHostClient->GetConnectionID() << newl;
    //hFile << _CTS("CheckReconnect ") << m_pHostClient->GetConnectedAddress() << SPACE << GetGameInfo()->GetMatchID() << newl;
}


/*====================
  CGameClient::Shutdown
  ====================*/
uint    CGameClient::Shutdown()
{
    Console << _T("Shutting down client...") << newl;

    Input.SetCursor(CURSOR_GAME, INVALID_RESOURCE);
    Input.SetCursorRecenter(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorHidden(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorFrozen(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstrained(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstraint(CURSOR_GAME, CRectf(0.0f, 0.0f, 0.0f, 0.0f));

    return m_pHostClient->GetIndex();
}


/*====================
  CGameClient::ProcessGameEvents
  ====================*/
bool    CGameClient::ProcessGameEvents(CSnapshot &snapshot)
{
    // Read events
    for (int i(0); i < snapshot.GetNumEvents(); ++i)
    {
        CGameEvent ev(snapshot.GetReceivedBuffer());

        if (cg_debugGameEvent)
            ev.Print();
        
        ev.Spawn();
        AddEvent(ev);
    }

    return true;
}


/*====================
  CGameClient::DumpSnapshot
  ====================*/
void    CGameClient::DumpSnapshot(CSnapshot &snapshot)
{
    snapshot.GetReceivedBuffer().Rewind();
    
    CSnapshot cDumpSnapshot(snapshot.GetReceivedBuffer());
    CSnapshot cBaseSnapshot;

    if (cDumpSnapshot.GetPrevFrameNumber() == -1)
    {
        cBaseSnapshot = CSnapshot();
    }
    else
    {
        PROFILE("Snapshot Copy");

        if (m_hServerSnapshotFallback != INVALID_POOL_HANDLE && CSnapshot::GetByHandle(m_hServerSnapshotFallback)->GetFrameNumber() == cDumpSnapshot.GetPrevFrameNumber())
        {
            //Console << _T("Restoring fallback frame ") << m_pServerSnapshotFallback->GetFrameNumber() << newl;
            cBaseSnapshot = *CSnapshot::GetByHandle(m_hServerSnapshotFallback);
        }
        else
        {
            // Search for the proper frame to diff from
            vector<PoolHandle>::const_iterator it(m_vServerSnapshots.begin());
            for (; it != m_vServerSnapshots.end(); ++it)
            {
                if (*it == INVALID_POOL_HANDLE)
                    continue;

                const CSnapshot &cSnapshot(*CSnapshot::GetByHandle(*it));

                if (cSnapshot.IsValid() && cSnapshot.GetFrameNumber() == cDumpSnapshot.GetPrevFrameNumber())
                    break;
            }

            if (it == m_vServerSnapshots.end())
            {
                Console << _T("Invalid previous frame number: ") << cDumpSnapshot.GetPrevFrameNumber() << _T(" (current: ") << cDumpSnapshot.GetFrameNumber() << _T(")") << newl;
            }
            else
            {
                cBaseSnapshot = *CSnapshot::GetByHandle(*it);
            }
        }
    }

    Console << _T("Frame: ") << cDumpSnapshot.GetFrameNumber() << newl;
    Console << _T("Prev: ") << cDumpSnapshot.GetPrevFrameNumber() << newl;
    Console << _T("Timestamp: ") << cDumpSnapshot.GetTimeStamp() << newl;
    Console << _T("State Sequence: ") << cDumpSnapshot.GetStateSequence() << newl;
    Console << _T("Num Events: ") << cDumpSnapshot.GetNumEvents() << newl;

    CBufferBit &cReceivedBuffer(cDumpSnapshot.GetReceivedBuffer());

    byte yNumEvents(cDumpSnapshot.GetNumEvents());

    // Events
    for (int i(0); i < yNumEvents; ++i)
        CGameEvent::AdvanceBuffer2(cReceivedBuffer);

    // Bit entities
    uivector &vCurrentBitEntityBuffer(m_pClientEntityDirectory->GetBitEntityBuffer());
    uint uiNumFields(uint(vCurrentBitEntityBuffer.size()));

    CTransmitFlags<8> cTransmitFlags(uiNumFields);

    cTransmitFlags.ReadTransmitFlags(cReceivedBuffer);

    Console << _T("Bit Entity Flags: ");

    for (uint uiField(0); uiField < uiNumFields; ++uiField)
    {
        if (cTransmitFlags.IsFieldSet(uiField))
        {
            Console << _T("1");
            cReceivedBuffer.ReadBits(32);
        }
        else
        {
            Console << _T("0");
        }
    }

    Console << newl;

    SnapshotVector &vBaseEntities(cBaseSnapshot.GetEntities());

    for (SnapshotVector_it cit(vBaseEntities.begin()), citEnd(vBaseEntities.end()); cit != citEnd; ++cit)
    {
        Console << newl << _T("Base Entity") << newl;
        
        CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(cit->second));

        Console << _T("Index: ") << pBaseSnapshot->GetIndex() << newl;
        Console << _T("Type Name: ") << EntityRegistry.LookupName(pBaseSnapshot->GetType()) << newl;
        Console << _T("Changing: ") << XtoA(pBaseSnapshot->GetChanging(), false) << newl;
    }

    bool bError(false);
    uint uiLastIndex(0);
    static CEntitySnapshot entSnapshot;
    SnapshotVector_it citBase(vBaseEntities.begin());

    // Entities
    while (!bError)
    {
        // Grab a "shell" entity snapshot from the the frame snapshot.
        // The data will be filled in once we know the type.
        entSnapshot.Clear();
        if (!cDumpSnapshot.GetNextEntity(entSnapshot, uiLastIndex))
            break;

        if (!entSnapshot.GetChanged())
        {
            bError = true;
            Console << _T("Invalid entity snapshot") << newl;
            break;
        }

        Console << newl << _T("Entity") << newl;

        if (entSnapshot.GetIndex() == INVALID_INDEX)
        {
            // Look for the index of an implied change
            while (citBase != vBaseEntities.end())
            {
                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                if (pBaseSnapshot->GetChanging())
                {
                    entSnapshot.SetIndex(citBase->first);
                    uiLastIndex = citBase->first;
                    break;
                }
                
                ++citBase;
            }
            if (citBase == vBaseEntities.end())
            {
                if (snapshot.GetReceivedBuffer().GetUnreadBits() >= 8)
                {
                    bError = true;
                    Console << snapshot.GetReceivedBuffer().GetUnreadBits() << _T(" unread bits") << newl;
                }
                
                break; // End of snapshot
            }

            Console << _T("Index: ") << ParenStr(XtoA(entSnapshot.GetIndex())) << newl;
        }
        else
        {
            while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                ++citBase;

            Console << _T("Index: ") << entSnapshot.GetIndex() << newl;
        }

        if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
        {
            //
            // New entity, read from baseline
            //

            if (entSnapshot.GetType() == 0)
                Console << _T("Delete") << newl;
            else
                Console << _T("New: ") << entSnapshot.GetType() << newl;

            ushort unType(entSnapshot.GetType());

            // If the type is nullptr, the entity is dead and should be removed
            if (unType == 0)
                continue;

            const SEntityDesc* pTypeDesc(EntityRegistry.GetTypeDesc(unType));
            if (pTypeDesc == nullptr)
            {
                bError = true;
                Console << _T("Unknown new entity type, bad snapshot") << newl;
                break;
            }

            Console << _T("Type Name: ") << EntityRegistry.LookupName(unType) << newl;
            Console << _T("Changing: ") << XtoA(entSnapshot.GetChanging(), false) << newl;

            entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline);
            ++citBase;
        }
        else if (citBase->first == entSnapshot.GetIndex())
        {
            //
            // Update existing entity
            //

            CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
            ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

            if (unType == 0)
                Console << _T("Delete") << newl;
            else if (entSnapshot.GetTypeChange())
                Console << _T("Type Change: ") << unType << newl;
            else
                Console << _T("Update") << newl;

            // If the type is nullptr, the entity is dead and should be removed
            if (unType == 0)
                continue;

            const SEntityDesc* pTypeDesc(EntityRegistry.GetTypeDesc(unType));
            if (pTypeDesc == nullptr)
            {
                bError = true;
                Console << _T("Unknown updated entity type, bad snapshot") << newl;
                break;
            }

            Console << _T("Type Name: ") << EntityRegistry.LookupName(unType) << newl;
            Console << _T("Changing: ") << XtoA(pBaseSnapshot->GetChanging(), false) << _T("->") << XtoA(entSnapshot.GetChanging(), false) << newl;

            if (entSnapshot.GetTypeChange())
                entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline);
            else
                entSnapshot.ReadBody(cReceivedBuffer, *pTypeDesc->pFieldTypes, pTypeDesc->uiSize);

            Console << _T("Fields: ") << newl;

            for (TypeVector::const_iterator it(pTypeDesc->pFieldTypes->begin()); it != pTypeDesc->pFieldTypes->end(); ++it)
            {
                if (!entSnapshot.IsFieldSet(uint(it - pTypeDesc->pFieldTypes->begin())))
                    continue;
                
                Console << it->sName << newl;
            }

            ++citBase;
        }
    }
}


/*====================
  CGameClient::ProcessFirstSnapshot
  ====================*/
void    CGameClient::ProcessFirstSnapshot()
{
    PlayerList.Trigger(TSNULL);
}


/*====================
  CGameClient::ProcessSnapshot
  ====================*/
bool    CGameClient::ProcessSnapshot(CSnapshot &snapshot)
{
    PROFILE_EX("CGameClient::ProcessSnapshot", PROFILE_GAME_CLIENT_PROCESS_SNAPSHOT);

    uint uiRealGameTime(GetGameTime());
    bool bReplay(ReplayManager.IsPlaying());

    try
    {
        // Setup replay spectator
        if (ReplayManager.IsPlaying() && m_pReplaySpectator == nullptr)
        {
            m_pReplaySpectator = m_pClientEntityDirectory->AllocateLocal(Player)->GetAsPlayer();
            m_pReplaySpectator->SetTeam(TEAM_SPECTATOR);
        }

        // Copy next states into previous states, etc
        m_pClientEntityDirectory->PrepForSnapshot();

        if (snapshot.GetPrevFrameNumber() == -1)
        {
            m_CurrentServerSnapshot = CSnapshot();
        }
        else
        {
            PROFILE("Snapshot Copy");

            if (m_hServerSnapshotFallback != INVALID_POOL_HANDLE && CSnapshot::GetByHandle(m_hServerSnapshotFallback)->GetFrameNumber() < snapshot.GetPrevFrameNumber())
            {
                //Console << _T("Deleting fallback frame ") << m_pServerSnapshotFallback->GetFrameNumber() << newl;
                SAFE_DELETE_SNAPSHOT(m_hServerSnapshotFallback);
            }

            if (m_hServerSnapshotFallback != INVALID_POOL_HANDLE && CSnapshot::GetByHandle(m_hServerSnapshotFallback)->GetFrameNumber() == snapshot.GetPrevFrameNumber())
            {
                //Console << _T("Restoring fallback frame ") << m_pServerSnapshotFallback->GetFrameNumber() << newl;
                m_CurrentServerSnapshot = *CSnapshot::GetByHandle(m_hServerSnapshotFallback);
            }
            else
            {
                // Search for the proper frame to diff from
                vector<PoolHandle>::const_iterator it(m_vServerSnapshots.begin());
                for (; it != m_vServerSnapshots.end(); ++it)
                {
                    if (*it == INVALID_POOL_HANDLE)
                        continue;

                    const CSnapshot &cSnapshot(*CSnapshot::GetByHandle(*it));

                    if (cSnapshot.IsValid() && cSnapshot.GetFrameNumber() == snapshot.GetPrevFrameNumber())
                        break;
                }

                if (it == m_vServerSnapshots.end())
                {
                    EX_ERROR(_T("Invalid previous frame number: ") + XtoA(snapshot.GetPrevFrameNumber()) + _T(" (current: ") + XtoA(snapshot.GetFrameNumber()) + _T(")"));
                }
                else
                {
                    m_CurrentServerSnapshot = *CSnapshot::GetByHandle(*it);
                }
            }
        }

        m_CurrentServerSnapshot.SetValid(true);
        m_CurrentServerSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_CurrentServerSnapshot.SetPrevFrameNumber(uint(-1));
        m_CurrentServerSnapshot.SetTimeStamp(snapshot.GetTimeStamp());

        // Read events
        byte yNumEvents(snapshot.GetNumEvents());
        for (int i(0); i < yNumEvents; ++i)
        {
            CGameEvent ev(snapshot.GetReceivedBuffer());

            if (cg_debugGameEvent)
                ev.Print();
            
            ev.Spawn();
            AddEvent(ev);
        }
            
        // Process this snapshot at the time matching it's timestamp, but
        // restore the client's current time afterwards (not game events)
        SetGameTime(snapshot.GetTimeStamp());

        // Read bit entities
        // Baseline active, might change later
        uivector &vCurrentBitEntityBuffer(m_pClientEntityDirectory->GetBitEntityBuffer());
        uint uiNumFields(uint(vCurrentBitEntityBuffer.size()));

        uivector &vBitEntityBuffer(m_CurrentServerSnapshot.GetStreamBitEntityBuffer(0));
        vBitEntityBuffer.resize(uiNumFields, uint(-1));

        CTransmitFlags<8> cTransmitFlags(uiNumFields);

        cTransmitFlags.ReadTransmitFlags(snapshot.GetReceivedBuffer());

        for (uint uiField(0); uiField < uiNumFields; ++uiField)
        {
            if (cTransmitFlags.IsFieldSet(uiField))
                vBitEntityBuffer[uiField] = snapshot.GetReceivedBuffer().ReadBits(32);
        }
        
        SnapshotVector &vBaseEntities(m_CurrentServerSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        START_PROFILE("Translate Entities");

        uint uiLastIndex(0);

        // Translate entities
        for (;;)
        {
            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, uiLastIndex))
                break;

            if (!entSnapshot.GetChanged())
                EX_ERROR(_T("Invalid entity snapshot"));

            if (entSnapshot.GetIndex() == INVALID_INDEX)
            {
                // Look for the index of an implied change
                while (citBase != vBaseEntities.end())
                {
                    CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                    if (pBaseSnapshot->GetChanging())
                    {
                        entSnapshot.SetIndex(citBase->first);
                        uiLastIndex = citBase->first;
                        break;
                    }
                    
                    ++citBase;
                }
                if (citBase == vBaseEntities.end())
                {
                    if (snapshot.GetReceivedBuffer().GetUnreadBits() >= 8)
                        EX_ERROR(XtoA(snapshot.GetReceivedBuffer().GetUnreadBits()) + _T(" unread bits"));
                    
                    break; // End of snapshot
                }
            }
            else
            {
                while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                    ++citBase;
            }

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                // If the type is nullptr, the entity is dead and should be removed
                if (unType == 0)
                    continue;

                const SEntityDesc* pTypeDesc(bReplay ? ReplayManager.GetCompatTypeDesc(unType) : EntityRegistry.GetTypeDesc(unType));
                if (pTypeDesc == nullptr)
                    EX_ERROR(_T("Unknown new entity type, bad snapshot"));

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline);
                entSnapshot.SetVersion(pTypeDesc->uiVersion);
                entSnapshot.SetApplyToFrame(snapshot.GetFrameNumber());
                citBase = vBaseEntities.insert(citBase, SnapshotEntry(entSnapshot.GetIndex(), CEntitySnapshot::Allocate(entSnapshot)));
                ++citBase;
            }
            else if (citBase->first == entSnapshot.GetIndex())
            {
                //
                // Update existing entity
                //

                CEntitySnapshot *pBaseSnapshot(CEntitySnapshot::GetByHandle(citBase->second));
                ushort unType(entSnapshot.GetTypeChange() ? entSnapshot.GetType() : pBaseSnapshot->GetType());

                // If the type is nullptr, the entity is dead and should be removed
                if (unType == 0)
                {
                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }               

                const SEntityDesc* pTypeDesc(bReplay ? ReplayManager.GetCompatTypeDesc(unType) : EntityRegistry.GetTypeDesc(unType));
                if (pTypeDesc == nullptr)
                    EX_ERROR(_T("Unknown updated entity type, bad snapshot"));

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeDesc->pFieldTypes, pTypeDesc->uiSize, pTypeDesc->pBaseline);
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeDesc->pFieldTypes, pTypeDesc->uiSize);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                pBaseSnapshot->SetVersion(pTypeDesc->uiVersion);
                pBaseSnapshot->SetApplyToFrame(snapshot.GetFrameNumber());
                ++citBase;
            }
        }

        END_PROFILE; // Translate Entities

        START_PROFILE("Process");

        // Process full snapshot
        for (SnapshotVector_cit citEntity(vBaseEntities.begin()); citEntity != vBaseEntities.end(); ++citEntity)
        {
            CEntitySnapshot *pEntitySnapshot(CEntitySnapshot::GetByHandle(citEntity->second));
            if (pEntitySnapshot->GetApplyToFrame() != snapshot.GetFrameNumber())
            {
                // Retrieve the entity
                IGameEntity *pEntity(m_pClientEntityDirectory->GetEntityNext(pEntitySnapshot->GetIndex()));
                if (pEntity)
                    pEntity->Validate();
                else
                    Console.Warn << _T("Unchanged entity exists in server snapshot but not on client") << newl;

                continue;
            }

            pEntitySnapshot->RewindRead();

            // Retrieve the entity
            IGameEntity *pEntity(m_pClientEntityDirectory->GetEntityNext(pEntitySnapshot->GetIndex()));

            // Detach slave incase owner changed but type didn't
            if (pEntity)
            {
                if (pEntity->IsState())
                {
                    IEntityState *pState(pEntity->GetAsState());

                    CClientEntity *pOwner(GetClientEntity(pState->GetOwnerIndex()));
                    if (pOwner != nullptr &&
                        pOwner->GetNextEntity() != nullptr &&
                        pOwner->GetPrevEntity() != nullptr &&
                        pOwner->GetCurrentEntity() != nullptr &&
                        pOwner->GetNextEntity()->IsUnit())
                    {
                        if (pOwner->GetNextEntity()->GetAsUnit() != nullptr) pOwner->GetNextEntity()->GetAsUnit()->RemoveState(pState);
                        if (pOwner->GetPrevEntity()->GetAsUnit() != nullptr) pOwner->GetPrevEntity()->GetAsUnit()->RemoveState(pState);
                        if (pOwner->GetCurrentEntity()->GetAsUnit() != nullptr) pOwner->GetCurrentEntity()->GetAsUnit()->RemoveState(pState);
                    }
                }
                else if (pEntity->IsTool())
                {
                    IEntityTool *pItem(pEntity->GetAsTool());

                    CClientEntity *pOwner(GetClientEntity(pItem->GetOwnerIndex()));
                    if (pOwner != nullptr && pOwner->GetNextEntity() != nullptr && pOwner->GetNextEntity()->IsUnit())
                    {
                        if (pOwner->GetNextEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetNextEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
                            pOwner->GetNextEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), nullptr);
                        if (pOwner->GetPrevEntity() != nullptr &&
                            pOwner->GetPrevEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetPrevEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
                            pOwner->GetPrevEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), nullptr);
                        if (pOwner->GetCurrentEntity() != nullptr &&
                            pOwner->GetCurrentEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetCurrentEntity()->GetAsUnit()->GetTool(pItem->GetSlot()) == pItem)
                            pOwner->GetCurrentEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), nullptr);
                    }
                }
                else if (pEntity->GetType() == Player)
                {
                    if (pEntity == m_pLocalPlayer)
                        m_pLocalPlayer = nullptr;
                }
                else if (pEntity->IsGadget())
                {
                    IGadgetEntity *pGadget(pEntity->GetAsGadget());

                    CClientEntity *pOwner(GetClientEntity(pGadget->GetMountIndex()));
                    if (pOwner != nullptr && pOwner->GetNextEntity() != nullptr && pOwner->GetNextEntity()->IsUnit())
                    {
                        if (pOwner->GetNextEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetNextEntity()->GetAsUnit()->GetMount() == pGadget)
                            pOwner->GetNextEntity()->GetAsUnit()->SetMount(nullptr);
                        if (pOwner->GetPrevEntity() != nullptr &&
                            pOwner->GetPrevEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetPrevEntity()->GetAsUnit()->GetMount() == pGadget)
                            pOwner->GetPrevEntity()->GetAsUnit()->SetMount(nullptr);
                        if (pOwner->GetCurrentEntity() != nullptr &&
                            pOwner->GetCurrentEntity()->GetAsUnit() != nullptr &&
                            pOwner->GetCurrentEntity()->GetAsUnit()->GetMount() == pGadget)
                            pOwner->GetCurrentEntity()->GetAsUnit()->SetMount(nullptr);
                    }
                }
            }

            // If the client does not have an entry for this entity, allocate a new one
            if (pEntity == nullptr)
            {
                pEntity = m_pClientEntityDirectory->Allocate(pEntitySnapshot->GetIndex(), pEntitySnapshot->GetType());

                // Apply the update
                pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
                pEntity->UpdateDefinition();

                if (cg_replayPrecacheAll && ReplayManager.IsPlaying() && pEntity->GetDefinition<IEntityDefinition>() != nullptr)
                    pEntity->GetDefinition<IEntityDefinition>()->Precache(PRECACHE_ALL, _T("All"));

                pEntity->Spawn();

                // Reapply the update to undo things spawn changed
                pEntitySnapshot->RewindRead();
                pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
            }
            else if (pEntity->GetType() != pEntitySnapshot->GetType())
            {
                if (cg_debugEntities)
                    Console << _T("Entity #") << pEntitySnapshot->GetIndex() << _T(" type change from ")
                            << SHORT_HEX_STR(pEntity->GetType()) << _T(" to ")
                            << SHORT_HEX_STR(pEntitySnapshot->GetType()) << newl;

                // Make sure we clear out the client entry
                if (pEntity->GetType() == Player)
                    m_mapClients.erase(static_cast<CPlayer*>(pEntity)->GetClientNumber());

                static ISlaveEntity *s_apInventory[MAX_INVENTORY];
                bool bWasUnit(pEntity->IsUnit());

                // Save inventory slots incase this is a entity type morph
                if (bWasUnit)
                {
                    for (int i(0); i < MAX_INVENTORY; ++i)
                        s_apInventory[i] = pEntity->GetAsUnit()->GetInventorySlot(i);
                }
                
                m_pClientEntityDirectory->Delete(pEntitySnapshot->GetIndex());
                pEntity = m_pClientEntityDirectory->Allocate(pEntitySnapshot->GetIndex(), pEntitySnapshot->GetType());

                // Apply the update
                pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
                pEntity->UpdateDefinition();

                if (cg_replayPrecacheAll && ReplayManager.IsPlaying() && pEntity->GetDefinition<IEntityDefinition>() != nullptr)
                    pEntity->GetDefinition<IEntityDefinition>()->Precache(PRECACHE_ALL, _T("All"));
                
                pEntity->Spawn();

                // Reapply the update to fix things spawn changed
                pEntitySnapshot->RewindRead();
                pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());

                // Inherit the old unit's inventory incase this was a entity type morph.
                // Truely new entities will receive delete messages for the old
                // inventory later in this snapshot
                if (pEntity->IsUnit() && bWasUnit)
                {
                    CClientEntity *pUnit(GetClientEntity(pEntity->GetIndex()));
                    if (pUnit != nullptr && pUnit->GetNextEntity()->IsUnit())
                    {
                        for (int i(0); i < MAX_INVENTORY; ++i)
                        {
                            pUnit->GetNextEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                            pUnit->GetPrevEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                            pUnit->GetCurrentEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                        }
                    }
                }
            }
            else
            {
                byte yResetSequence(0);

                if (pEntity->IsVisual())
                {
                    yResetSequence = pEntity->GetAsVisual()->GetResetSequence();

                    // Apply the update
                    pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
                    pEntity->UpdateDefinition();

                    if (yResetSequence != pEntity->GetAsVisual()->GetResetSequence())
                    {
                        pEntitySnapshot->RewindRead();

                        static ISlaveEntity *s_apInventory[MAX_INVENTORY];
                        bool bWasUnit(pEntity->IsUnit());

                        // Save inventory slots incase this is a unit
                        if (bWasUnit)
                        {
                            for (int i(0); i < MAX_INVENTORY; ++i)
                                s_apInventory[i] = pEntity->GetAsUnit()->GetInventorySlot(i);
                        }
                        
                        m_pClientEntityDirectory->Delete(pEntitySnapshot->GetIndex());
                        pEntity = m_pClientEntityDirectory->Allocate(pEntitySnapshot->GetIndex(), pEntitySnapshot->GetType());

                        // Apply the update
                        pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
                        pEntity->UpdateDefinition();

                        if (cg_replayPrecacheAll && ReplayManager.IsPlaying() && pEntity->GetDefinition<IEntityDefinition>() != nullptr)
                            pEntity->GetDefinition<IEntityDefinition>()->Precache(PRECACHE_ALL, _T("All"));
                        
                        pEntity->Spawn();

                        // Reapply the update to fix things spawn changed
                        pEntitySnapshot->RewindRead();
                        pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());

                        // Inherit the old unit's inventory 
                        if (pEntity->IsUnit() && bWasUnit)
                        {
                            CClientEntity *pUnit(GetClientEntity(pEntity->GetIndex()));
                            if (pUnit != nullptr && pUnit->GetNextEntity()->IsUnit())
                            {
                                for (int i(0); i < MAX_INVENTORY; ++i)
                                {
                                    pUnit->GetNextEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                                    pUnit->GetPrevEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                                    pUnit->GetCurrentEntity()->GetAsUnit()->SetInventorySlot(i, s_apInventory[i]);
                                }
                            }
                        }

                        DeleteRelatedEvents(pEntity->GetIndex());
                    }
                }
                else
                {
                    // Apply the update
                    pEntity->ReadSnapshot(*pEntitySnapshot, pEntitySnapshot->GetVersion());
                    pEntity->UpdateDefinition();
                }
            }

            pEntity->SetFrame(m_CurrentServerSnapshot.GetFrameNumber());
            pEntity->Validate();

            // Check for a game info update
            if (pEntity->IsGameInfo())
            {
                CGameInfo *pGameInfo(static_cast<CGameInfo*>(pEntity));
                SetGameInfo(pGameInfo);

                if (!m_pHostClient->GetPractice())// && !pGameInfo->HasFlags(GAME_FLAG_ARRANGED))
                    ChatManager.FinishedJoiningGame(pGameInfo->GetGameName(), pGameInfo->GetMatchID());

                m_pInterfaceManager->StoreMatchLength(pGameInfo->GetMatchLength());
                
                continue;
            }

            // Check for a team info update
            if (pEntity->GetType() == Info_Team)
            {
                CTeamInfo *pTeamInfo(static_cast<CTeamInfo *>(pEntity));
                CTeamInfo *pOldTeamInfo(Game.GetTeam(pTeamInfo->GetTeamID()));

                if (pOldTeamInfo == nullptr)
                {
                    Game.SetTeam(pTeamInfo->GetTeamID(), pTeamInfo);
                }
                else if (pOldTeamInfo != pTeamInfo)
                {
                    Game.RemoveTeam(pTeamInfo->GetTeamID());

                    K2_DELETE(pOldTeamInfo);

                    Game.SetTeam(pTeamInfo->GetTeamID(), pTeamInfo);

                    // Initialize team list with any clients we've already recieved
                    UpdateTeamRosters();
                }

                continue;
            }

            // Check for a shop info item update
            if (pEntity->GetType() == Shop_ItemInfo)
            {
                CShopItemInfo *pItem(static_cast<CShopItemInfo *>(pEntity));

                pItem->SetItemName(EntityRegistry.LookupName(pItem->GetItemType()));

                CTeamInfo *pTeam(GetTeam(pItem->GetTeam()));

                if (pTeam != nullptr)
                {
                    CShopInfo *pShop(pTeam->GetShopInfo());

                    if (pShop != nullptr)
                        pShop->AddItem(pItem);
                }

                continue;
            }

            // Check for stats and request extended data
            if (pEntity->GetType() == Info_Stats)
            {
                CBufferFixed<5> buffer;
                buffer << GAME_CMD_REQUEST_EXT_ENTITY_DATA << pEntity->GetIndex();
                SendGameData(buffer, true);
            }

            // Store client info and track local client
            if (pEntity->GetType() == Player)
            {
                CPlayer *pClient(static_cast<CPlayer*>(pEntity));
                PlayerMap_it itClient(m_mapClients.find(pClient->GetClientNumber()));
                if (itClient != m_mapClients.end() && itClient->second != pClient)
                    Console.Warn << _T("Client number changed: ") << itClient->second->GetClientNumber() << _T(" -> ") << pClient->GetClientNumber() << newl;
                m_mapClients[pClient->GetClientNumber()] = pClient;

                if (!ReplayManager.IsPlaying() && pClient->GetClientNumber() == m_pHostClient->GetClientNum())
                    m_pLocalPlayer = pClient;

                UpdateTeamRosters();
            }

            // Update attached entity states
            if (pEntity->IsState())
            {
                IEntityState *pState(pEntity->GetAsState());

                CClientEntity *pOwner(GetClientEntity(pState->GetOwnerIndex()));
                if (pOwner != nullptr &&
                    pOwner->GetNextEntity() != nullptr &&
                    pOwner->GetPrevEntity() != nullptr &&
                    pOwner->GetCurrentEntity() != nullptr &&
                    pOwner->GetNextEntity()->IsUnit())
                {
                    if (pOwner->GetNextEntity()->GetAsUnit() != nullptr) pOwner->GetNextEntity()->GetAsUnit()->AddState(pState);
                    if (pOwner->GetPrevEntity()->GetAsUnit() != nullptr) pOwner->GetPrevEntity()->GetAsUnit()->AddState(pState);
                    if (pOwner->GetCurrentEntity()->GetAsUnit() != nullptr) pOwner->GetCurrentEntity()->GetAsUnit()->AddState(pState);
                }
                continue;
            }
            else if (pEntity->IsTool())
            {
                IEntityTool *pItem(pEntity->GetAsTool());

                CClientEntity *pOwner(GetClientEntity(pItem->GetOwnerIndex()));
                if (pOwner != nullptr &&
                    pOwner->GetNextEntity() != nullptr &&
                    pOwner->GetPrevEntity() != nullptr &&
                    pOwner->GetCurrentEntity() != nullptr &&
                    pOwner->GetNextEntity()->IsUnit())
                {
                    if (pOwner->GetNextEntity()->GetAsUnit() != nullptr) pOwner->GetNextEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), pItem);
                    if (pOwner->GetPrevEntity()->GetAsUnit() != nullptr) pOwner->GetPrevEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), pItem);
                    if (pOwner->GetCurrentEntity()->GetAsUnit() != nullptr) pOwner->GetCurrentEntity()->GetAsUnit()->SetInventorySlot(pItem->GetSlot(), pItem);
                }
                continue;
            }
            else if (pEntity->IsGadget())
            {
                IGadgetEntity *pGadget(pEntity->GetAsGadget());

                CClientEntity *pOwner(GetClientEntity(pGadget->GetMountIndex()));
                if (pOwner != nullptr &&
                    pOwner->GetNextEntity() != nullptr &&
                    pOwner->GetPrevEntity() != nullptr &&
                    pOwner->GetCurrentEntity() != nullptr &&
                    pOwner->GetNextEntity()->IsUnit())
                {
                    if (pOwner->GetNextEntity()->GetAsUnit() != nullptr) pOwner->GetNextEntity()->GetAsUnit()->SetMount(pGadget);
                    if (pOwner->GetPrevEntity()->GetAsUnit() != nullptr) pOwner->GetPrevEntity()->GetAsUnit()->SetMount(pGadget);
                    if (pOwner->GetCurrentEntity()->GetAsUnit() != nullptr) pOwner->GetCurrentEntity()->GetAsUnit()->SetMount(pGadget);
                }
            }

            // Use the new state as a base for interpolations
            CClientEntity *pClientEntity(GetClientEntity(pEntity->GetIndex()));
            if (pClientEntity)
                pClientEntity->GetCurrentEntity()->Copy(*pEntity);
        }

        m_pClientEntityDirectory->CleanupEntities();

        END_PROFILE; // Process

        // Give events that just spawned a chance to synch with entities that arrived in the same frame
        SynchNewEvents();

        if (ReplayManager.IsPlaying())
        {
            SetReplayClient(replay_client);

            if (m_pLocalPlayer == nullptr && !m_mapClients.empty())
            {
                PlayerMap_it itClient(m_mapClients.begin());

                for (; itClient != m_mapClients.end() && itClient->second->IsDisconnected(); ++itClient) {}

                if (itClient != m_mapClients.end())
                    SetReplayClient(itClient->first);
            }
        }
                
        SetGameTime(uiRealGameTime);

        if (!m_bProcessedFirstSnapshot)
        {
            ProcessFirstSnapshot();
            m_bProcessedFirstSnapshot = true;
        }

        return true;
    }
    catch (CException &ex)
    {
        //DumpSnapshot(snapshot);
        m_pHostClient->GameError(ex.GetMsg());
        ex.Process(_T("CGameClient::ProcessSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CGameClient::ProcessGameData
  ====================*/
bool    CGameClient::ProcessGameData(CPacket &pkt)
{
    PROFILE("CGameClient::ProcessGameData");

    byte yCmd(pkt.ReadByte());

    if (cg_debugGameData)
    {
        tstring sCmd;
        
        switch (yCmd)
        {
        case GAME_CMD_CHAT_ALL:
            sCmd = _T("GAME_CMD_CHAT_ALL");
            break;

        case GAME_CMD_CHAT_ROLL:
            sCmd = _T("GAME_CMD_CHAT_ROLL");
            break;
            
        case GAME_CMD_CHAT_EMOTE:
            sCmd = _T("GAME_CMD_CHAT_EMOTE");
            break;
            
        case GAME_CMD_CHAT_TEAM:
            sCmd = _T("GAME_CMD_CHAT_TEAM");
            break;

        case GAME_CMD_SERVERCHAT_ALL:
            sCmd = _T("GAME_CMD_SERVERCHAT_ALL");
            break;

        case GAME_CMD_MESSAGE:
            sCmd = _T("GAME_CMD_MESSAGE");
            break;

        default:
            sCmd = XtoA(yCmd, 0, 0, 16);
            break;
        }

        Console << sCmd << newl;

        static map<byte, uint>  mapCmdStats;
        if (mapCmdStats.find(yCmd) == mapCmdStats.end())
            mapCmdStats[yCmd] = 1;
        else
            mapCmdStats[yCmd] = mapCmdStats[yCmd] + 1;
    }
    
    switch (yCmd)
    {
    case GAME_CMD_CHAT_ALL:
    case GAME_CMD_CHAT_ROLL:
    case GAME_CMD_CHAT_EMOTE:
        {
            int iSender(pkt.ReadInt());
            tstring sMsg(pkt.ReadWStringAsTString());

            if (cg_censorChat)
                ChatManager.CensorChat(sMsg);

            if (!cg_drawChat)
                break;

            CPlayer *pSender(GetPlayer(iSender));
            if (pSender == nullptr || Host.IsIgnored(pSender->GetName()) || ChatManager.GetIgnoreChat() == CHAT_IGNORE_ALL || ChatManager.GetIgnoreChat() == CHAT_IGNORE_EVERYONE || (ChatManager.GetIgnoreChat() == CHAT_IGNORE_ENEMY_ALL && GetLocalPlayer()->GetTeam() != pSender->GetTeam()))
                break;
                
            tsmapts mapTokens;
            tstring sHeroName(_T("No Hero"));

            if (pSender->GetHero() != nullptr)
                sHeroName = pSender->GetHero()->GetDisplayName();
            else if (pSender->HasSelectedHero())
            {
                CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pSender->GetSelectedHero()));

                if (pHeroDef != nullptr)
                    sHeroName = pHeroDef->GetDisplayName();
            }

            mapTokens[_T("color")] = GetInlineColorString<tstring>(pSender->GetColor());
            mapTokens[_T("message")] = sMsg;            
            
            tstring sMessage;
            
            if (yCmd == GAME_CMD_CHAT_ALL)
            {
                mapTokens[_T("player")] = pSender->GetName();
                mapTokens[_T("hero")] = sHeroName;              
                sMessage = GetGameMessage(Game.GetGamePhase() < GAME_PHASE_HERO_BAN ? _T("chat_all_lobby") : _T("chat_all"), mapTokens);
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                ChatManager.PlaySound(_T("RecievedChannelMessage"));
            }
            else if (yCmd == GAME_CMD_CHAT_ROLL)
            {
                sMessage = GetGameMessage(Game.GetGamePhase() < GAME_PHASE_HERO_BAN ? _T("chat_roll_lobby") : _T("chat_roll"), mapTokens);
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ROLL, sMessage);
            }
            else if (yCmd == GAME_CMD_CHAT_EMOTE)
            {
                sMessage = GetGameMessage(Game.GetGamePhase() < GAME_PHASE_HERO_BAN ? _T("chat_emote_lobby") : _T("chat_emote"), mapTokens);
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_EMOTE, sMessage);
            }
            
            Console << sMessage << newl;    

        }
        break;

    case GAME_CMD_CHAT_TEAM:
        {
            int iSender(pkt.ReadInt());
            tstring sMsg(pkt.ReadWStringAsTString());

            if (cg_censorChat)
                ChatManager.CensorChat(sMsg);

            if (!cg_drawChat)
                break;

            CPlayer *pSender(GetPlayer(iSender));
            if (pSender == nullptr || Host.IsIgnored(pSender->GetName()) || ChatManager.GetIgnoreChat() == CHAT_IGNORE_TEAM || ChatManager.GetIgnoreChat() == CHAT_IGNORE_EVERYONE)
                break;

            tsmapts mapTokens;
            tstring sHeroName(_T("No Hero"));

            if (pSender->GetHero() != nullptr)
                sHeroName = pSender->GetHero()->GetDisplayName();
            else if (pSender->HasSelectedHero())
            {
                CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pSender->GetSelectedHero()));

                if (pHeroDef != nullptr)
                    sHeroName = pHeroDef->GetDisplayName();
            }

            mapTokens[_T("color")] = GetInlineColorString<tstring>(pSender->GetColor());
            mapTokens[_T("player")] = pSender->GetName();
            mapTokens[_T("hero")] = sHeroName;
            mapTokens[_T("message")] = sMsg;

            const tstring &sMessage(GetGameMessage(Game.GetGamePhase() < GAME_PHASE_HERO_BAN ? _T("chat_team_lobby") : _T("chat_team"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;

    case GAME_CMD_SERVERCHAT_ALL:
        {
            tstring sMsg(pkt.ReadWStringAsTString());

            if (!cg_drawChat)
                break;

            tsmapts mapTokens;
            mapTokens[_T("message")] = sMsg;
            const tstring &sMessage(GetGameMessage(_T("server_message"), mapTokens));

            Console << _T("^yServer Message: ") << sMsg << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_MESSAGE:
    case GAME_CMD_PAUSE_COUNTDOWN:
        {
            tstring sMessage(pkt.ReadWStringAsTString());

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (yCmd == GAME_CMD_PAUSE_COUNTDOWN)
            {
                // Play a tick sound for unpausing
                if (m_ahResources[CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE] != INVALID_RESOURCE)              
                    K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE]);
            }
        }
        break;

    case GAME_CMD_POPUP:
    case GAME_CMD_POPUP_VALUE:
        {
            byte yType(pkt.ReadByte(INVALID_POPUP));
            byte yPlayer(pkt.ReadByte(byte(-1)));
            ushort unIndex(pkt.ReadShort(-1));
            uint uiServerTime(pkt.ReadInt(INVALID_TIME));
            ushort unValue(yCmd == GAME_CMD_POPUP_VALUE ? pkt.ReadShort() : 0);

            if (yType == POPUP_EXPERIENCE)
            {
                IUnitEntity* pUnit(Game.GetUnitEntity(unIndex));
                if (pUnit && pUnit->IsCreep())
                {
                    CClientEntity* pClEnt(GameClient.GetClientEntity(pUnit->GetIndex()));
                    if (pClEnt)
                        pClEnt->StartEffect(_T("/shared/effects/exp/exp.effect"), -1, 0);
                }
            }

            if (yType == POPUP_EXPERIENCE && !cg_drawRewardExp)
                break;
            if (yType == POPUP_GOLD && !cg_drawRewardGold)
                break;
            if (yType == POPUP_DEFLECTION && !cg_drawPopupDeflection)
                break;
            if (yType == POPUP_CRITICAL && !cg_drawPopupCritical)
                break;

            const CPopup *pPopup(GetPopup(yType));
            if (pPopup == nullptr)
                break;

            CVec4f v4Color(pPopup->GetColor());
            if (pPopup->GetUsePlayerColor())
            {
                CPlayer *pPlayer(GetPlayerFromClientNumber(yPlayer));
                if (pPlayer != nullptr)
                    v4Color = pPlayer->GetColor();
            }

            SpawnPopupMessage(pPopup->GetMessage(unValue), unIndex == ushort(-1) ? INVALID_INDEX : unIndex, v4Color, pPopup->GetStartX(), pPopup->GetStartY(), pPopup->GetSpeedX(), pPopup->GetSpeedY(), pPopup->GetDuration(), pPopup->GetFadeTime(), uiServerTime);
        }
        break;

#if 0
    case GAME_CMD_MINIMAP_DRAW:
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());
            int iClientNum(pkt.ReadByte());

            CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNum));
            if (pPlayer == nullptr)
                break;

            CVec4f v4Color(pPlayer->GetColor());
            CBufferFixed<20> buffer;
            buffer << fX << fY << v4Color[R] << v4Color[G] << v4Color[B];

            Minimap.Execute(_T("draw"), buffer);
        }
        break;
#endif

    case GAME_CMD_MAP_PING:
        {
            byte yType(pkt.ReadByte(INVALID_PING));
            byte yPlayer(pkt.ReadByte(byte(-1)));
            float fX(pkt.ReadByte() / float(UCHAR_MAX));
            float fY(pkt.ReadByte() / float(UCHAR_MAX));

            if (pkt.HasFaults())
                break;

            if (cg_mapPingEnable)
            {           
                Ping(yType, Game.GetWorldWidth() * fX, Game.GetWorldWidth() * fY, int(yPlayer));
            }
        }
        break;

    case GAME_CMD_UNIT_PING:
        {
            byte yType(pkt.ReadByte(INVALID_PING));
            byte yPlayer(pkt.ReadByte(byte(-1)));
            uint uiTargetUnit(pkt.ReadInt(-1));

            if (pkt.HasFaults())
                break;

            IUnitEntity* pUnit(Game.GetUnitEntity(uiTargetUnit));
            if (pUnit == nullptr)
                break;

            CPlayer* pPlayer(Game.GetPlayerFromClientNumber(yPlayer));
            CPlayer* pLocalPlayer(Game.GetLocalPlayer());

            if (!pPlayer || !pLocalPlayer)
                break;

            if (pPlayer->GetTeam() != pLocalPlayer->GetTeam())
                break;
            
            CTeamInfo* pTeam(pPlayer->GetTeamInfo());

            if (!pTeam)
                break;

            float fX(pUnit->GetPosition().x);
            float fY(pUnit->GetPosition().y);

            if (cg_mapPingEnable)
            {
                Ping(yType, fX, fY, int(yPlayer));
            }

            tstring sMode(_T("Defend our "));
            tstring sUnitPlayerName(_T(""));
            if (pLocalPlayer->IsEnemy(pUnit) && pUnit->IsBuilding())
                sMode = _T("Attack this ");
            else if (pLocalPlayer->IsEnemy(pUnit) && pUnit->IsHero())
            {
                sMode = _T("Attack ");
                tstring sUnitPlayerColor(GetInlineColorString<tstring>(pUnit->GetOwnerPlayer() != nullptr ? pUnit->GetOwnerPlayer()->GetColor() : WHITE));
                sUnitPlayerName = sUnitPlayerColor + _T("(") + (pUnit->GetOwnerPlayer() != nullptr ? pUnit->GetOwnerPlayer()->GetName() : TSNULL) + _T(")");
            }

            tsmapts mapTokens;
            mapTokens[_T("player_name")] = (pPlayer != nullptr ? pPlayer->GetName() : TSNULL);
            mapTokens[_T("player_hero_name")] = (pPlayer->GetHero() != nullptr ? pPlayer->GetHero()->GetDisplayName() : TSNULL);
            mapTokens[_T("unit_name")] = (pUnit != nullptr ? pUnit->GetDisplayName() : TSNULL);
            mapTokens[_T("unit_player_name")] = sUnitPlayerName;
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer != nullptr ? pPlayer->GetColor() : WHITE);
            mapTokens[_T("player_team_color")] = GetInlineColorString<tstring>(pPlayer != nullptr ? pPlayer->GetHero()->GetTeamColor(pLocalPlayer) : WHITE);
            mapTokens[_T("unit_team_color")] = GetInlineColorString<tstring>(pUnit != nullptr ? pUnit->GetTeamColor(pLocalPlayer) : WHITE);
            mapTokens[_T("mode")] = sMode;

            const tstring &sMessage(GetGameMessage(_T("ping_target"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

#if 0
    case GAME_CMD_BUILDING_ATTACK_ALERT:
        {
            byte yCount(pkt.ReadByte());

            for (byte y(0); y < yCount; ++y)
            {
                uint uiIndex(pkt.ReadInt());
                IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
                if (pBuilding == nullptr)
                    continue;

                pBuilding->MinimapFlash(RED, cg_buildingAttackAlertTime);
                m_pInterfaceManager->BuildingAttackAlert(pBuilding->GetDisplayName());
            }

            if (yCount > 0)
                K2SoundManager.Play2DSound(m_hBuildingAttackedSample);
        }
        break;
#endif

    case GAME_CMD_END_GAME:
        {
            int iTeam(pkt.ReadInt());
            uint uiTime(pkt.ReadInt());

            SetWinningTeam(iTeam);
            SetFinalMatchTime(uiTime);

            if (!cg_muteAnnouncerVoice)
            {
                if (m_pLocalPlayer != nullptr)
                {
                    tsvector vParams(2);
                    vParams[0] = _T("0");
                    vParams[1] = GetLocalVoiceSet().sArcadeText;

                    if (iTeam == m_pLocalPlayer->GetTeam() && GetLocalVoiceSet().hVictory != INVALID_RESOURCE)
                    {
                        if (!cg_muteAnnouncerVoice)
                            m_deqHeroAnnouncements.push_back(GetLocalVoiceSet().hVictory);
                    
                        m_pInterfaceManager->Trigger(UITRIGGER_EVENT_VICTORY, vParams);
                    }
                    else if (GetLocalVoiceSet().hDefeat != INVALID_RESOURCE)
                    {                   
                        if (!cg_muteAnnouncerVoice)
                            m_deqHeroAnnouncements.push_back(GetLocalVoiceSet().hDefeat);

                        m_pInterfaceManager->Trigger(UITRIGGER_EVENT_DEFEAT, vParams);
                    }
                }

                if (iTeam == TEAM_1 && GetLocalVoiceSet().hLegionWins != INVALID_RESOURCE)
                    m_deqHeroAnnouncements.push_back(GetLocalVoiceSet().hLegionWins);
                else if (iTeam == TEAM_2 && GetLocalVoiceSet().hHellbourneWins != INVALID_RESOURCE)
                    m_deqHeroAnnouncements.push_back(GetLocalVoiceSet().hHellbourneWins);
            }

            //m_pInterfaceManager->StoreEndGameStats();
            m_pInterfaceManager->SaveSpectatorPlayers();

            if (!ReplayManager.IsPlaying())
                ChatManager.ShowPostGameStats(Game.GetGameInfo() != nullptr ? Game.GetGameInfo()->GetMatchID() : INVALID_INDEX);
        }
        break;

#if 0
    case GAME_CMD_BUILDING_DESTROYED:
        {
            uint uiType(pkt.ReadShort());
            uint uiTeam(pkt.ReadByte());
            
            CPlayer *pPlayer(GetLocalPlayer());

            if (pPlayer == nullptr)
                break;
            
            ICvar *pPath;
            
            if (pPlayer->GetTeam() == uiTeam)
            {
                pPath = EntityRegistry.GetGameSetting(uiType, _T("DestroyedSoundPath"));
            }
            else
            {
                CTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

                if (pTeam == nullptr)
                    break;
                
                tstring sSetting(_T("DestroyedSoundPath"));
                pPath = EntityRegistry.GetGameSetting(uiType, sSetting);
            }
            
            if (!pPath)
                break;

            tstring sPath(pPath->GetString());
            if (sPath.empty())
                break;

            ResHandle hSample(g_ResourceManager.Register(sPath, RES_SAMPLE));
            if (hSample != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(hSample);
        }
        break;
#endif

#if 0
    case GAME_CMD_BLD_HEALTH_LOW:
        {
            uint uiType(pkt.ReadShort());
            ICvar *pPath(EntityRegistry.GetGameSetting(uiType, _T("LowHealthSoundPath")));

            //Console << pPath << newl;
            
            if (!pPath)
                break;

            tstring sPath(pPath->GetString());
            if (sPath.empty())
                break;

            ResHandle hSample(g_ResourceManager.Register(sPath, RES_SAMPLE));
            if (hSample != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(hSample);
        }
        break;
#endif

    case GAME_CMD_CONFIRM_MOVE:
        {
            uint uiUnitIndex(pkt.ReadShort());

            if (!cg_unitVoiceResponses)
                break;

            if (uiUnitIndex == ushort(-1))
                uiUnitIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiUnitIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == nullptr)
                break;

            if (GetGameTime() - GetLastConfirmMoveSoundTime() > cg_unitVoiceResponsesDelay)
            {
                ResHandle hSample(pUnit->GetConfirmMoveSound());
                if (hSample != INVALID_RESOURCE)
                    pClEntity->PlaySound(hSample, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, false, 0, 1.0f, 1.0f, 0, 17000.0f);

                m_pClientCommander->ResetVoiceSequence();
                
                SetLastConfirmMoveSoundTime(GetGameTime());
            }                       
        }
        break;

    case GAME_CMD_CONFIRM_ATTACK:
        {
            uint uiUnitIndex(pkt.ReadShort());

            if (!cg_unitVoiceResponses)
                break;

            if (uiUnitIndex == ushort(-1))
                uiUnitIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiUnitIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == nullptr)
                break;

            if (GetGameTime() - GetLastConfirmAttackSoundTime() > cg_unitVoiceResponsesDelay)
            {
                ResHandle hSample(pUnit->GetConfirmAttackSound());
                if (hSample != INVALID_RESOURCE)
                    pClEntity->PlaySound(hSample, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, false, 0, 1.0f, 1.0f, 0, 17000.0f);

                m_pClientCommander->ResetVoiceSequence();
                
                SetLastConfirmAttackSoundTime(GetGameTime());
            }                       
        }
        break;

    case GAME_CMD_PICKUP_ITEM:
        {
            uint uiIndex(pkt.ReadShort(-1));

            if  (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            pkt.ReadByte(); // byte ySlot
            pkt.ReadShort(); // ushort unItem

            if (uiIndex == m_pClientCommander->GetSelectedControlEntityIndex())
                K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_PICKUP_ITEM_SAMPLE]);
        }
        break;
        
    case GAME_CMD_SERVER_STATS:
        {
            if (cg_replayServerStats)
                Console.Std << _T("Server Frame: ") << pkt.ReadInt() << newl; // uint uiFrameLength
            else
                pkt.ReadInt();
        }
        break;

    case GAME_CMD_ITEM_FAILED:
        {
            pkt.ReadByte(); // byte ySlot
            pkt.ReadShort(); // ushort unItem
        }
        break;

    case GAME_CMD_ITEM_SUCCEEDED:
        {
            pkt.ReadByte(); // byte ySlot
            pkt.ReadShort(); // ushort unItem
        }
        break;
    
    case GAME_CMD_ORDER_CONFIRMATION:
        {
            uint uiEntIndex(pkt.ReadShort());

            if (m_pLocalPlayer == nullptr)
                break;
            CPlayer *pLocalPlayer(m_pLocalPlayer);
            if (pLocalPlayer == nullptr)
                break;

            IVisualEntity *pEntity(GetVisualEntity(uiEntIndex));

            if (!pEntity)
                break;

            pEntity->SetOrderTime(Game.GetGameTime());
        }
        break;

    case GAME_CMD_KILL_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            int iVictim(pkt.ReadInt(-1));
            ushort unGold(pkt.ReadShort());
            byte yAssists(pkt.ReadByte());

            ivector vAssists;
            for (byte y(0); y < yAssists; ++y)
                vAssists.push_back(pkt.ReadInt(-1));

            CPlayer *pKiller(GetPlayer(iKiller));
            CPlayer *pVictim(GetPlayer(iVictim));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = (pKiller != nullptr ? pKiller->GetName() : TSNULL);
            mapTokens[_T("victim")] = (pVictim != nullptr ? pVictim->GetName() : TSNULL);
            mapTokens[_T("killer_color")] = GetInlineColorString<tstring>(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            mapTokens[_T("victim_color")] = GetInlineColorString<tstring>(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            mapTokens[_T("gold")] = XtoA(unGold);

            for (byte y(0); y < yAssists; ++y)
            {
                CPlayer *pPlayer(GetPlayer(vAssists[y]));
                if (pPlayer == nullptr)
                    continue;

                mapTokens[_T("assist") + XtoA(y)] = pPlayer->GetName();
                mapTokens[_T("assist") + XtoA(y) + _T("_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            }

            const tstring &sMessage(GetGameMessage(_T("kill") + XtoA(yAssists), mapTokens));
            Console.Std << sMessage << newl;
            
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }

            tsvector vParams(8);
            vParams[0] = pKiller != nullptr ? pKiller->GetName() : TSNULL;
            vParams[1] = pVictim != nullptr ? pVictim->GetName() : TSNULL;

            if (m_pLocalPlayer == nullptr || m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            {
                vParams[2] = XtoA(pKiller != nullptr && pKiller->GetTeam() == 1);
                vParams[3] = XtoA(pVictim != nullptr && pVictim->GetTeam() == 1);
            }
            else
            {
                vParams[2] = XtoA(pKiller != nullptr && pKiller->GetTeam() == m_pLocalPlayer->GetTeam());
                vParams[3] = XtoA(pVictim != nullptr && pVictim->GetTeam() == m_pLocalPlayer->GetTeam());
            }

            vParams[4] = XtoA(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            vParams[5] = XtoA(pVictim != nullptr ? pVictim->GetColor() : WHITE);

            IHeroEntity *pKillerHero(pKiller != nullptr ? pKiller->GetHero() : nullptr);
            IHeroEntity *pVictimHero(pVictim != nullptr ? pVictim->GetHero() : nullptr);

            vParams[6] = pKillerHero != nullptr ? pKillerHero->GetIconPath() : TSNULL;
            vParams[7] = pVictimHero != nullptr ? pVictimHero->GetIconPath() : TSNULL;

            if (pKillerHero != nullptr && pVictimHero != nullptr)
                EventKill.Trigger(vParams);
        }
        break;

    case GAME_CMD_HERO_DENY_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            int iVictim(pkt.ReadInt(-1));

            CPlayer *pKiller(GetPlayer(iKiller));
            CPlayer *pVictim(GetPlayer(iVictim));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = (pKiller != nullptr ? pKiller->GetName() : TSNULL);
            mapTokens[_T("victim")] = (pVictim != nullptr ? pVictim->GetName() : TSNULL);
            mapTokens[_T("killer_color")] = GetInlineColorString<tstring>(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            mapTokens[_T("victim_color")] = GetInlineColorString<tstring>(pVictim != nullptr ? pVictim->GetColor() : WHITE);

            const tstring &sMessage(GetGameMessage(_T("deny"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }
        }
        break;

    case GAME_CMD_LOBBY_FORCED_TEAM_SWAP_MESSAGE:
        {
            int iTeam1(pkt.ReadInt(-1));
            int iSlot1(pkt.ReadInt(-1));
            int iTeam2(pkt.ReadInt(-1));
            int iSlot2(pkt.ReadInt(-1));

            tsmapts mapTokens;
            mapTokens[_T("slot1")] = CPlayer::GetColorName(((iTeam1 - 1) * 5) + iSlot1);
            mapTokens[_T("slot2")] = CPlayer::GetColorName(((iTeam2 - 1) * 5) + iSlot2);
            mapTokens[_T("slot1_color")] = GetInlineColorString<tstring>(CPlayer::GetColor(((iTeam1 - 1) * 5) + iSlot1));
            mapTokens[_T("slot2_color")] = GetInlineColorString<tstring>(CPlayer::GetColor(((iTeam2 - 1) * 5) + iSlot2));

            const tstring &sMessage(GetGameMessage(_T("lobby_force_swap"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;

    case GAME_CMD_BAN_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-2));
            ushort unHero(pkt.ReadShort(INVALID_ENT_TYPE));

            tstring sKey;
            tsmapts mapTokens;

            if (iClientNumber == -1)
            {
                sKey = _CTS("hero_ban_random");
            }
            else
            {
                CPlayer *pPlayer(GetPlayer(iClientNumber));
                if (pPlayer == nullptr)
                    break;

                sKey = _CTS("hero_ban");
                mapTokens[_CTS("player")] = pPlayer->GetName();
                mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            }

            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHero));
            if (pHeroDef == nullptr)
                break;

            mapTokens[_CTS("hero_name")] = pHeroDef->GetDisplayName();

            const tstring &sMessage(GetGameMessage(sKey, mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (m_ahResources[CLIENT_RESOURCE_HERO_BAN_SAMPLE] != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_HERO_BAN_SAMPLE]);
        }
        break;

    case GAME_CMD_FIRST_BLOOD_MESSAGE:
    case GAME_CMD_FIRST_BLOOD_MESSAGE2:
        {
            int iKiller(pkt.ReadInt(-1));
            ushort unGold(pkt.ReadShort());
            
            uint uiVoice(INVALID_INDEX);
            
            if (yCmd == GAME_CMD_FIRST_BLOOD_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }
            
            CPlayer *pKiller(GetPlayer(iKiller));
            if (pKiller == nullptr)
                break;          

            tsmapts mapTokens;
            mapTokens[_T("killer")] = (pKiller != nullptr ? pKiller->GetName() : TSNULL);
            mapTokens[_T("killer_color")] = GetInlineColorString<tstring>(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("first_kill"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hFirstBlood);
                
            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_FIRST_KILL, vMiniParams);
        }
        break;

    case GAME_CMD_KILLSTREAK_MESSAGE:
    case GAME_CMD_KILLSTREAK_MESSAGE2:
        {
            int iKiller(pkt.ReadInt(-1));
            byte yKillStreak(MIN<byte>(pkt.ReadByte(0), 15));
            
            uint uiVoice(INVALID_INDEX);
            
            if (yCmd == GAME_CMD_KILLSTREAK_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }       

            CPlayer *pKiller(GetPlayer(iKiller));

            if (yKillStreak < 3)
                break;
            if (yKillStreak >= 15)
                yKillStreak = 8;
            else
                yKillStreak = CLAMP(yKillStreak - 3, 0, 7);

            tstring sKey(_T("killstreak") + XtoA(yKillStreak));

            if (!sKey.empty())
            {
                tsmapts mapTokens;
                mapTokens[_T("killer")] = (pKiller != nullptr ? pKiller->GetName() : TSNULL);
                mapTokens[_T("killer_color")] = GetInlineColorString<tstring>(pKiller != nullptr ? pKiller->GetColor() : WHITE);

                const tstring &sMessage(GetGameMessage(sKey, mapTokens));
                Console.Std << sMessage << newl;
                if (cg_drawMessages)
                    ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            }

            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(yKillStreak);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hKillstreak[yKillStreak]);
        }
        break;

    case GAME_CMD_END_STREAK_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            int iVictim(pkt.ReadInt(-1));
            byte yKillStreak(MIN<byte>(pkt.ReadByte(0), 15));
            ushort unGold(pkt.ReadShort(0));

            CPlayer *pKiller(GetPlayer(iKiller));
            tstring sKillerName(pKiller != nullptr ? pKiller->GetName() : TSNULL);
            tstring sKillerColor(GetInlineColorString<tstring>(pKiller != nullptr ? pKiller->GetColor() : WHITE));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            tstring sVictimColor(GetInlineColorString<tstring>(pVictim != nullptr ? pVictim->GetColor() : WHITE));

            tstring sKey;
            
            switch (yKillStreak)
            {
            case 3:
                sKey = _T("streakend0");
                break;
            case 4:
                sKey = _T("streakend1");
                break;
            case 5:
                sKey = _T("streakend2");
                break;
            case 6:
                sKey = _T("streakend3");
                break;
            case 7:
                sKey = _T("streakend4");
                break;
            case 8:
                sKey = _T("streakend5");
                break;
            case 9:
                sKey = _T("streakend6");
                break;
            }

            if (yKillStreak >= 10)
                sKey = _T("streakend7");

            if (yKillStreak >= 15)
                sKey = _T("streakend8");

            if (!sKey.empty())
            {
                tsmapts mapTokens;
                mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
                mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");
                mapTokens[_T("gold")] = XtoA(unGold);
                
                const tstring &sMessage(GetGameMessage(sKey, mapTokens));
                Console.Std << sMessage << newl;
                if (cg_drawMessages)
                    ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            }

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }
        }
        break;

    case GAME_CMD_MULTIKILL_MESSAGE:
    case GAME_CMD_MULTIKILL_MESSAGE2:
        {
            int iKiller(pkt.ReadInt(-1));
            byte yMultiKill(pkt.ReadByte(0));
            
            uint uiVoice(INVALID_INDEX);
            
            if (yCmd == GAME_CMD_MULTIKILL_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }           

            CPlayer *pKiller(GetPlayer(iKiller));
            tstring sKillerName(pKiller != nullptr ? pKiller->GetName() : TSNULL);
            CVec4f v4KillerColor(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));
            
            if (yMultiKill < 2)
                break;
                
            yMultiKill = CLAMP(yMultiKill - 2, 0, 3);

            tstring sKey(_T("multikill") + XtoA(yMultiKill));

            if (!sKey.empty())
            {
                tsmapts mapTokens;
                mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");

                const tstring &sMessage(GetGameMessage(sKey, mapTokens));
                Console.Std << sMessage << newl;
                if (cg_drawMessages)
                    ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            }

            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(yMultiKill);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hMultikill[yMultiKill]);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_MULTI_KILL, vMiniParams);
        }
        break;

    case GAME_CMD_START_GAME_WARNING:
        K2SoundManager.Play2DSound(GetLocalVoiceSet().hStartGame);
        break;

    case GAME_CMD_KILL_TOWER_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            uint uiRewardTeam(pkt.ReadInt(TEAM_PASSIVE));
            ushort unGold(pkt.ReadShort(0));
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            CPlayer *pKiller(GetPlayer(iKiller));
            tstring sKillerName(pKiller != nullptr ? pKiller->GetName() : TSNULL);
            CVec4f v4KillerColor(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));

            tstring sRewardName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiRewardTeam));
            if (pTeam != nullptr)
                sRewardName = pTeam->GetName();

            CVec4f v4RewardColor(uiRewardTeam == TEAM_1 ? LEGION_RED : uiRewardTeam == TEAM_2 ? HELLBOURNE_GREEN : WHITE);

            tstring sRewardColor(_T("^") +
                XtoA(INT_ROUND(v4RewardColor[R] * 9)) + 
                XtoA(INT_ROUND(v4RewardColor[G] * 9)) +
                XtoA(INT_ROUND(v4RewardColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("reward")] = sRewardColor + sRewardName + _T("^*");
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("killtower"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));

            if (pBuilding != nullptr)
            {
                if (GetLocalPlayer()->GetTeam() == pBuilding->GetTeam())
                    Ping(PING_ALLY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
                else
                    Ping(PING_ENEMY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
            }
        }
        break;

    case GAME_CMD_DENY_TOWER_MESSAGE:
    case GAME_CMD_DENY_TOWER_MESSAGE2:
        {
            int iPlayer(pkt.ReadInt(-1));
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            uint uiVoice(INVALID_INDEX);

            if (yCmd == GAME_CMD_DENY_TOWER_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }

            CPlayer *pPlayer(GetPlayer(iPlayer));
            tstring sPlayerName(pPlayer != nullptr ? pPlayer->GetName() : TSNULL);
            CVec4f v4PlayerColor(pPlayer != nullptr ? pPlayer->GetColor() : WHITE);
            tstring sPlayerColor(_T("^") +
                XtoA(INT_ROUND(v4PlayerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4PlayerColor[G] * 9)) +
                XtoA(INT_ROUND(v4PlayerColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("player")] = sPlayerColor + sPlayerName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("denytower"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hDenied);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_TOWER_DENY, vMiniParams);

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));

            if (pBuilding != nullptr)
                Ping(PING_ALERT, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
        }
        break;

    case GAME_CMD_KILL_COURIER_MESSAGE:
    case GAME_CMD_KILL_COURIER_MESSAGE2:
        {       
            int iKiller(0);
            byte yKiller(0);
            
            if (yCmd == GAME_CMD_KILL_COURIER_MESSAGE)
                iKiller = pkt.ReadInt(-1);
            else
                yKiller = pkt.ReadByte(-1);                         
            
            int iVictim(pkt.ReadInt(-1));
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));           
            
            CPlayer *pKiller(nullptr);
            
            if (yCmd == GAME_CMD_KILL_COURIER_MESSAGE)
                pKiller = GetPlayer(iKiller);
            else
                pKiller = GetPlayer(yKiller);           
            
            tstring sKillerName(pKiller != nullptr ? pKiller->GetName() : TSNULL);
            CVec4f v4KillerColor(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            
            if (yCmd == GAME_CMD_KILL_COURIER_MESSAGE2)
            {
                // It was the team that killed the courier, not a hero, so display the appropriate message
                if (yKiller == byte(TEAM_1))
                {
                    sKillerName = _T("The Legion");
                    v4KillerColor = CVec4f(LEGION_RED);
                }
                else if (yKiller == byte(TEAM_2))
                {
                    sKillerName = _T("The Hellbourne");
                    v4KillerColor = CVec4f(HELLBOURNE_GREEN);
                }
                else if (yKiller == byte(TEAM_NEUTRAL))
                {
                    sKillerName = _T("Neutral Creeps");
                    v4KillerColor = CVec4f(WHITE);
                }
            }
            
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));
            
            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");

            const tstring sMessage(GetGameMessage(_T("killcourier"), mapTokens));           
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_COURIER_KILL, 0);

            IUnitEntity *pUnit(Game.GetUnitEntity(uiIndex));

            // Even though the courier isn't a building, these green/red building kill indicators look nice, and calling 
            // the default PING_ALERT doesn't show the minimap ping for some reason (it just plays the noise) - JT
            if (pUnit != nullptr)
            {
                if (GetLocalPlayer()->GetTeam() == pUnit->GetTeam())
                    Ping(PING_ALLY_BUILDING_KILL, pUnit->GetPosition().x, pUnit->GetPosition().y, -1);
                else
                    Ping(PING_ENEMY_BUILDING_KILL, pUnit->GetPosition().x, pUnit->GetPosition().y, -1);         
            }
        }
        break;
        
    case GAME_CMD_TEAM_KILLSTREAK_MESSAGE:
        {
            int iTeam(pkt.ReadInt(-1));

            if (iTeam < 0 || iTeam > 2)
                iTeam = 0;

            tsmapts mapTokens;

            const tstring &sMessage(GetGameMessage(_T("teamkillstreak") + XtoA(iTeam), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (!cg_muteAnnouncerVoice && GetLocalVoiceSet().hTeamKillstreak != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hTeamKillstreak);
        }
        break;      
        
    case GAME_CMD_TEAM_KILL_MESSAGE:
        {
            uint uiKillerTeam(pkt.ReadInt(TEAM_PASSIVE));
            int iVictim(pkt.ReadInt(-1));
            ushort unGold(pkt.ReadShort());

            tstring sKillerName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiKillerTeam));
            if (pTeam != nullptr)
                sKillerName = pTeam->GetName();

            CVec4f v4KillerColor(uiKillerTeam == TEAM_1 ? LEGION_RED : uiKillerTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("teamkill"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }
        }
        break;

    case GAME_CMD_NEUTRAL_KILL_MESSAGE:
        {
            int iVictim(pkt.ReadInt(-1));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("neutralkill"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }
        }
        break;

    case GAME_CMD_KONGOR_KILL_MESSAGE:
        {
            int iVictim(pkt.ReadInt(-1));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("kongorkill"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SUICIDE_KILL_MESSAGE:
        {
            int iVictim(pkt.ReadInt(-1));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("suicide"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (pVictim != nullptr)
            {
                IHeroEntity *pHero(pVictim->GetHero());

                if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                    Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
            }
        }
        break;

    case GAME_CMD_UNKNOWN_KILL_MESSAGE:
        {
            int iVictim(pkt.ReadInt(-1));

            CPlayer *pVictim(GetPlayer(iVictim));
            tstring sVictimName(pVictim != nullptr ? pVictim->GetName() : TSNULL);
            CVec4f v4VictimColor(pVictim != nullptr ? pVictim->GetColor() : WHITE);
            tstring sVictimColor(_T("^") +
                XtoA(INT_ROUND(v4VictimColor[R] * 9)) + 
                XtoA(INT_ROUND(v4VictimColor[G] * 9)) +
                XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("kill"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            
            IHeroEntity *pHero(pVictim->GetHero());

            if (pHero != nullptr && pHero->IsVisibleOnMap(GetLocalPlayer()))
                Ping(PING_KILL_HERO, pHero->GetPosition().x, pHero->GetPosition().y, pHero->GetOwnerClientNumber());
        }
        break;

    case GAME_CMD_TEAM_KILL_TOWER_MESSAGE:
        {
            uint uiKillerTeam(pkt.ReadInt(TEAM_PASSIVE));
            ushort unGold(pkt.ReadShort(0));
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            tstring sKillerName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiKillerTeam));
            if (pTeam != nullptr)
                sKillerName = pTeam->GetName();

            CVec4f v4KillerColor(uiKillerTeam == TEAM_1 ? LEGION_RED : uiKillerTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("teamkilltower"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));

            if (pBuilding != nullptr)
            {
                if (GetLocalPlayer()->GetTeam() == pBuilding->GetTeam())
                    Ping(PING_ALLY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
                else
                    Ping(PING_ENEMY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
            }
        }
        break;

    case GAME_CMD_TEAM_DENY_TOWER_MESSAGE:
        {
            uint uiDenyingTeam(pkt.ReadInt(TEAM_PASSIVE));
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            tstring sDenierName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiDenyingTeam));
            if (pTeam != nullptr)
                sDenierName = pTeam->GetName();

            CVec4f v4DenierColor(uiDenyingTeam == TEAM_1 ? LEGION_RED : uiDenyingTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sDenierColor(_T("^") +
                XtoA(INT_ROUND(v4DenierColor[R] * 9)) + 
                XtoA(INT_ROUND(v4DenierColor[G] * 9)) +
                XtoA(INT_ROUND(v4DenierColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("denier")] = sDenierColor + sDenierName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("teamdenytower"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (!cg_muteAnnouncerVoice && GetLocalVoiceSet().hDenied != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hDenied);

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));

            if (pBuilding != nullptr)
                Ping(PING_ALERT, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
        }
        break;

    case GAME_CMD_KILL_KONGOR_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            uint uiRewardTeam(pkt.ReadInt(TEAM_PASSIVE));
            ushort unGold(pkt.ReadShort(0));

            CPlayer *pKiller(GetPlayer(iKiller));
            tstring sKillerName(pKiller != nullptr ? pKiller->GetName() : TSNULL);
            CVec4f v4KillerColor(pKiller != nullptr ? pKiller->GetColor() : WHITE);
            tstring sKillerColor(_T("^") +
                XtoA(INT_ROUND(v4KillerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4KillerColor[G] * 9)) +
                XtoA(INT_ROUND(v4KillerColor[B] * 9)));

            tstring sRewardName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiRewardTeam));
            if (pTeam != nullptr)
                sRewardName = pTeam->GetName();

            CVec4f v4RewardColor(uiRewardTeam == TEAM_1 ? LEGION_RED : uiRewardTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sRewardColor(_T("^") +
                XtoA(INT_ROUND(v4RewardColor[R] * 9)) + 
                XtoA(INT_ROUND(v4RewardColor[G] * 9)) +
                XtoA(INT_ROUND(v4RewardColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("reward")] = sRewardColor + sRewardName + _T("^*");
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("killkongor"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                
            if (GetLocalVoiceSet().hKongorSlain != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hKongorSlain);                
        }
        break;

    case GAME_CMD_TEAM_KILL_KONGOR_MESSAGE:
        {
            uint uiKillerTeam(pkt.ReadInt(TEAM_PASSIVE));
            ushort unGold(pkt.ReadShort(0));

            tstring sKillerName(uiKillerTeam == TEAM_1 ? _T("The Legion") : uiKillerTeam == TEAM_2 ? _T("The Hellbourne") : _T("Someone"));
            CVec4f v4KillerColor(uiKillerTeam == TEAM_1 ? LEGION_RED : uiKillerTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sKillerColor(GetInlineColorString<tstring>(v4KillerColor));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("gold")] = XtoA(unGold);

            const tstring &sMessage(GetGameMessage(_T("teamkillkongor"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (GetLocalVoiceSet().hKongorSlain != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hKongorSlain);                
        }
        break;

    case GAME_CMD_CONTROL_SHARE_MESSAGE:
        {
            int iSharingClient(pkt.ReadInt(-1));
            byte yShareFlags(pkt.ReadByte(byte(-1)));
            
            CPlayer *pPlayer(GetPlayer(iSharingClient));
            if (pPlayer == nullptr)
                break;

            if (yShareFlags > 3)
                break;

            tstring sPlayerName(pPlayer != nullptr ? pPlayer->GetName() : TSNULL);
            CVec4f v4PlayerColor(pPlayer != nullptr ? pPlayer->GetColor() : WHITE);
            tstring sPlayerColor(_T("^") +
                XtoA(INT_ROUND(v4PlayerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4PlayerColor[G] * 9)) +
                XtoA(INT_ROUND(v4PlayerColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("player")] = sPlayerColor + sPlayerName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("share") + XtoA(yShareFlags), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_POWERUP_MESSAGE:
        {
            int iPickupClient(pkt.ReadInt(-1));
            ushort unPowerupType(pkt.ReadShort(INVALID_ENT_TYPE));

            CPowerupDefinition *pPowerupDef(EntityRegistry.GetDefinition<CPowerupDefinition>(unPowerupType));
            if (pPowerupDef == nullptr)
                break;

            if (!cg_muteAnnouncerVoice && pPowerupDef->GetTouchSound() != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(pPowerupDef->GetTouchSound());

            CPlayer *pPlayer(GetPlayer(iPickupClient));
            tstring sPlayerName(pPlayer != nullptr ? pPlayer->GetName() : TSNULL);
            CVec4f v4PlayerColor(pPlayer != nullptr ? pPlayer->GetColor() : WHITE);
            tstring sPlayerColor(_T("^") +
                XtoA(INT_ROUND(v4PlayerColor[R] * 9)) + 
                XtoA(INT_ROUND(v4PlayerColor[G] * 9)) +
                XtoA(INT_ROUND(v4PlayerColor[B] * 9)));

            tsmapts mapTokens;
            mapTokens[_T("player")] = sPlayerColor + sPlayerName + _T("^*");
            mapTokens[_T("powerup")] = pPowerupDef->GetDisplayName();

            const tstring &sMessage(GetGameMessage(_T("powerup"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_CREEP_UPGRADE_MESSAGE:
        {
            tsmapts mapTokens;
            const tstring &sMessage(GetGameMessage(_T("creepupgrade"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_MEGACREEP_MESSAGE:
        {
            uint uiKillerTeam(pkt.ReadInt(TEAM_PASSIVE));
            uint uiVictimTeam(pkt.ReadInt(TEAM_PASSIVE));

            tstring sKillerTeamName(_T("Someone"));
            CTeamInfo *pKillerTeam(GetTeam(uiKillerTeam));
            if (pKillerTeam != nullptr)
                sKillerTeamName = pKillerTeam->GetName();

            CVec4f v4KillerColor(uiKillerTeam == TEAM_1 ? LEGION_RED : uiKillerTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sKillerColor(GetInlineColorString<tstring>(v4KillerColor));

            tstring sVictimTeamName(_T("Someone"));
            CTeamInfo *pVictimTeam(GetTeam(uiVictimTeam));
            if (pVictimTeam != nullptr)
                sVictimTeamName = pVictimTeam->GetName();

            CVec4f v4VictimColor(uiVictimTeam == TEAM_1 ? LEGION_RED : uiVictimTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE);
            tstring sVictimColor(GetInlineColorString<tstring>(v4VictimColor));

            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerTeamName + _T("^*");
            mapTokens[_T("victim")] = sVictimColor + sVictimTeamName + _T("^*");;

            const tstring &sMessage(GetGameMessage(_T("megacreeps"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_MEGA_CREEPS, 0);
        }
        break;

    case GAME_CMD_TEAM_WIPE_MESSAGE:
        {
            uint uiTeam(pkt.ReadInt(TEAM_PASSIVE));

            tstring sTeamName(_T("Someone"));
            CTeamInfo *pTeam(GetTeam(uiTeam));
            if (pTeam != nullptr)
                sTeamName = pTeam->GetName();

            CVec4f v4Color(uiTeam == TEAM_1 ? LEGION_RED : uiTeam == TEAM_2 ? CVec4f(HELLBOURNE_GREEN) : WHITE); 
            tstring sColor(GetInlineColorString<tstring>(v4Color));

            tsmapts mapTokens;
            mapTokens[_T("team")] = sColor + sTeamName + _T("^*");

            const tstring &sMessage(GetGameMessage(_T("teamwipe"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (!cg_muteAnnouncerVoice)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hTeamWipe);
        
            tsvector vParams(2);
            vParams[0] = _T("0");
            vParams[1] = GetLocalVoiceSet().sArcadeText;

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_TEAM_WIPE, vParams);
            
        }
        break;

    case GAME_CMD_TERMINATED_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            pkt.ReadInt();  // Deprecated

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("player")] = pPlayer->GetName();
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            tstring sMessage(_T(""));
            if (pPlayer->GetTeam() == TEAM_1 || pPlayer->GetTeam() == TEAM_2)
                sMessage = GetGameMessage(_T("client_terminated"), mapTokens);
            else
                sMessage = GetGameMessage(_T("client_disconnected"), mapTokens);
                
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_DISCONNECT_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;

            CVec4f v4PlayerColor(WHITE);

            if (pPlayer->GetTeam() == TEAM_PASSIVE)
                v4PlayerColor = pPlayer->GetAccountColor();
            else
                v4PlayerColor = pPlayer->GetColor();
                
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(v4PlayerColor);
            mapTokens[_CTS("player")] = pPlayer->GetName();

            const tstring &sMessage(GetGameMessage(_CTS("client_disconnected"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_CONNECT_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            
            CVec4f v4PlayerColor(WHITE);

            if (pPlayer->GetTeam() == TEAM_PASSIVE)
                v4PlayerColor = pPlayer->GetAccountColor();
            else
                v4PlayerColor = pPlayer->GetColor();
                
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(v4PlayerColor);
            mapTokens[_CTS("player")] = pPlayer->GetName();

            const tstring &sMessage(GetGameMessage(_CTS("client_connected"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_RECONNECT_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            uint uiTotalDisconnectedTime(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("time")] = FormatTime(uiTotalDisconnectedTime, 1, 0, FMT_NONE);

            const tstring &sMessage(GetGameMessage(_CTS("client_reconnected"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SELECT_HERO:
        {
            ushort unHeroTypeID(pkt.ReadShort(INVALID_ENT_TYPE));

            cg_avatarHero = EntityRegistry.LookupName(unHeroTypeID);
        }
        break;

    case GAME_CMD_PICK_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            ushort unHero(pkt.ReadShort(INVALID_ENT_TYPE));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHero));

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("hero_name")] = pHeroDef != nullptr ? pHeroDef->GetDisplayName() : TSNULL;

            if (m_ahResources[CLIENT_RESOURCE_HERO_SELECT_SAMPLE] != INVALID_RESOURCE)
                m_deqHeroAnnouncements.push_back(m_ahResources[CLIENT_RESOURCE_HERO_SELECT_SAMPLE]);

            if (pHeroDef != nullptr && !cg_muteAnnouncerVoice)
            {           
                pHeroDef->PrecacheAnnouncerSound();
                ResHandle hAnnouncement(pHeroDef->GetAnnouncerSound());
                if (hAnnouncement != INVALID_RESOURCE)
                    m_deqHeroAnnouncements.push_back(hAnnouncement);
            }

            tstring sMessage;
            if (pHeroDef != nullptr)
                sMessage = GetGameMessage(_CTS("hero_pick"), mapTokens);
            else
            {
                sMessage = GetGameMessage(_CTS("hero_pick_blind"), mapTokens);
                pPlayer->SetBlindPick(true);
            }
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_PICK_POTENTIAL_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            ushort unHero(pkt.ReadShort(INVALID_ENT_TYPE));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHero));
            if (pHeroDef == nullptr)
                break;

            pPlayer->SelectPotentialHero(unHero);
        }
        break;
        
    case GAME_CMD_RANDOM_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            ushort unHero(pkt.ReadShort(INVALID_ENT_TYPE));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHero));

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("hero_name")] = pHeroDef ? pHeroDef->GetDisplayName() : TSNULL;

            if (!HasGameOptions(GAME_OPTION_FORCE_RANDOM) || m_deqHeroAnnouncements.empty())
            {
                if (m_ahResources[CLIENT_RESOURCE_HERO_SELECT_SAMPLE] != INVALID_RESOURCE)
                    m_deqHeroAnnouncements.push_back(m_ahResources[CLIENT_RESOURCE_HERO_SELECT_SAMPLE]);

                if (pHeroDef != nullptr && !cg_muteAnnouncerVoice && !HasGameOptions(GAME_OPTION_FORCE_RANDOM))
                {
                    pHeroDef->PrecacheAnnouncerSound();
                    ResHandle hAnnouncement(pHeroDef->GetAnnouncerSound());
                    if (hAnnouncement != INVALID_RESOURCE)
                        m_deqHeroAnnouncements.push_back(hAnnouncement);
                }
            }

            tstring sMessage;
            if (pHeroDef != nullptr)
                sMessage = GetGameMessage(_CTS("hero_random"), mapTokens);
            else
            {
                sMessage = GetGameMessage(_CTS("hero_random_blind"), mapTokens);
                pPlayer->SetBlindPick(true);
            }
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_REPICK_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            ushort unHero(pkt.ReadShort(INVALID_ENT_TYPE));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(unHero));

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("hero_name")] = pHeroDef != nullptr ? pHeroDef->GetDisplayName() : TSNULL;

            tstring sMessage;
            if (pHeroDef != nullptr)
                sMessage = GetGameMessage(_CTS("hero_repick"), mapTokens);
            else
            {
                sMessage = GetGameMessage(_CTS("hero_repick_blind"), mapTokens);
                pPlayer->SetBlindPick(true);
            }
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SWAP_HERO_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            int iClientNumber2(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            CPlayer *pPlayer2(GetPlayer(iClientNumber2));
            if (pPlayer2 == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("player2")] = pPlayer2->GetName();
            mapTokens[_CTS("player2_color")] = GetInlineColorString<tstring>(pPlayer2->GetColor());

            const tstring &sMessage(GetGameMessage(_CTS("hero_swap"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SWAP_REQUEST_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_CTS("hero_swap_request"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;
        
    case GAME_CMD_MATCH_CANCEL_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("lobby_countdown_cancel")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;      

    case GAME_CMD_READY_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_CTS("player_ready"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_UNREADY_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("player")] = pPlayer->GetName();
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_CTS("player_unready"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_VOTE_CALLED_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));
            byte yVoteType(pkt.ReadByte(VOTE_TYPE_INVALID));
            int iTarget(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tstring sBaseMessage(_CTS("vote_called"));

            tsmapts mapTokens;
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_CTS("player")] = pPlayer->GetName();

            switch (yVoteType)
            {
            case VOTE_TYPE_CONCEDE:
                {
                    sBaseMessage += _CTS("_concede");
                    CTeamInfo *pTeam(GetTeam(iTarget));
                    mapTokens[_CTS("team")] = pTeam ? pTeam->GetName() : TSNULL;
                    mapTokens[_CTS("team_color")] = pTeam ? GetInlineColorString<tstring>(pTeam->GetColor()) : TSNULL;
                }
                break;

            case VOTE_TYPE_REMAKE:
                sBaseMessage += _CTS("_remake");
                break;

            case VOTE_TYPE_KICK:
                {
                    sBaseMessage += _CTS("_kick");
                    CPlayer *pTargetPlayer(GetPlayer(iTarget));
                    mapTokens[_CTS("target")] = pTargetPlayer ? pTargetPlayer->GetName() : TSNULL;
                    mapTokens[_CTS("target_color")] = pTargetPlayer ? GetInlineColorString<tstring>(pTargetPlayer->GetColor()) : TSNULL;
                }
                break;

            case VOTE_TYPE_PAUSE:
                {
                    sBaseMessage += _CTS("_pause");
                    CTeamInfo *pTeam(GetTeam(iTarget));
                    mapTokens[_CTS("team")] = pTeam ? pTeam->GetName() : TSNULL;
                    mapTokens[_CTS("team_color")] = pTeam ? GetInlineColorString<tstring>(pTeam->GetColor()) : TSNULL;
                }
                break;
            }

            if (m_ahResources[CLIENT_RESOURCE_VOTE_CALLED_SAMPLE] != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_VOTE_CALLED_SAMPLE]);

            const tstring &sMessage(GetGameMessage(sBaseMessage, mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_VOTE_PASSED_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("vote_passed")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (m_ahResources[CLIENT_RESOURCE_VOTE_PASSED_SAMPLE] != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_VOTE_PASSED_SAMPLE]);
        }
        break;

    case GAME_CMD_VOTE_FAILED_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("vote_failed")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (m_ahResources[CLIENT_RESOURCE_VOTE_FAILED_SAMPLE] != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_VOTE_FAILED_SAMPLE]);
        }
        break;

    case GAME_CMD_UNPAUSE:
        {
            tstring sName(pkt.ReadWStringAsTString());

            tsmapts mapTokens;
            mapTokens[_CTS("name")] = sName;

            const tstring &sMessage(GetGameMessage(_CTS("game_unpause"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);

            if (GetLocalVoiceSet().hUnpausing != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hUnpausing);
        }
        break;

    case GAME_CMD_CONCEDE_MESSAGE:
        {
            int iTeam(pkt.ReadInt(TEAM_INVALID));

            CTeamInfo *pTeam(GetTeam(iTeam));
            if (pTeam == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_CTS("color")] = GetInlineColorString<tstring>(pTeam->GetColor());
            mapTokens[_CTS("team")] = pTeam->GetName();

            const tstring &sMessage(GetGameMessage(_CTS("team_concede"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_ABANDONED_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("game_abandoned")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_AFK_WARNING_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("afk_warning")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_AFK_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_CTS("afk_message")));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SMACKDOWN_MESSAGE:
    case GAME_CMD_SMACKDOWN_MESSAGE2:
        {
            uint uiVoice(INVALID_INDEX);

            if (yCmd == GAME_CMD_SMACKDOWN_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }

            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hSmackdown);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_SMACKDOWN, vMiniParams);
        }
        break;

    case GAME_CMD_HUMILIATION_MESSAGE:
    case GAME_CMD_HUMILIATION_MESSAGE2:
        {
            uint uiVoice(INVALID_INDEX);

            if (yCmd == GAME_CMD_HUMILIATION_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }
        
            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hHumiliation);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_HUMILIATION, vMiniParams);
        }
        break;

    case GAME_CMD_RAGE_QUIT_MESSAGE:
    case GAME_CMD_RAGE_QUIT_MESSAGE2:
        {
            uint uiVoice(INVALID_INDEX);

            if (yCmd == GAME_CMD_RAGE_QUIT_MESSAGE2)
            {
                byte y(pkt.ReadByte());
                uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            }
        
            tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
            assert(!m_vAnnouncerVoices.empty());
            uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

            tsvector vMiniParams(2);
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = sSet;

            if (!cg_muteAnnouncerVoice)         
                K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hRageQuit);

            m_pInterfaceManager->Trigger(UITRIGGER_EVENT_RAGE_QUIT, vMiniParams);
        }
        break;

    case GAME_CMD_BUILDING_UNDER_ATTACK_MESSAGE:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));

            if (!cg_muteAnnouncerVoice)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hBaseUnderAttack);

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));

            if (pBuilding != nullptr)
                Ping(PING_BUILDING_ATTACK, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
        }
        break;

    case GAME_CMD_HERO_UNDER_ATTACK_MESSAGE:
        {
            int iOwner(pkt.ReadInt(-1));

            if (!cg_muteAnnouncerVoice)
            {
                if (iOwner != GetLocalClientNum())
                    K2SoundManager.Play2DSound(GetLocalVoiceSet().hAllyHeroUnderAttack);
                else
                    K2SoundManager.Play2DSound(GetLocalVoiceSet().hOurHeroUnderAttack);
            }
        }
        break;

    case GAME_CMD_HELLBOURNE_DESTROY_TOWER_MESSAGE:
        {
            if (!cg_muteAnnouncerVoice)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hHellbourneDestroyedTower);
        }
        break;

    case GAME_CMD_LEGION_DESTROY_TOWER_MESSAGE:
        {
            if (!cg_muteAnnouncerVoice)
                K2SoundManager.Play2DSound(GetLocalVoiceSet().hLegionDestroyedTower);
        }
        break;

    case GAME_CMD_TAUNTED_SOUND:
        {
            uint uiUnitIndex(pkt.ReadInt());

            if (!cg_unitVoiceResponses)
                break;

            CClientEntity *pClEntity(GetClientEntity(uiUnitIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == nullptr)
                break;

            ResHandle hSample(pUnit->GetTauntedSound());
            if (hSample != INVALID_RESOURCE)
#if 1
                K2SoundManager.Play2DSound(hSample);
#else
                pClEntity->PlaySound(hSample, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, true, 0, 1.0f, 1.0f, 0, 17000.0f, true);
#endif
        }
        break;

    case GAME_CMD_TAUNT_KILLED_SOUND:
        {
            uint uiUnitIndex(pkt.ReadInt());

            if (!cg_unitVoiceResponses)
                break;

            CClientEntity *pClEntity(GetClientEntity(uiUnitIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
            if (pUnit == nullptr)
                break;

            ResHandle hSample(pUnit->GetTauntKillSound());
            if (hSample != INVALID_RESOURCE)
#if 1
                K2SoundManager.Play2DSound(hSample);
#else
                pClEntity->PlaySound(hSample, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, true, 0, 1.0f, 1.0f, 0, 17000.0f, true);
#endif
        }
        break;

    case GAME_CMD_EXT_ENTITY_DATA:
        {
            uint uiIndex(pkt.ReadInt(INVALID_INDEX));
            ushort unLength(pkt.ReadShort());
            IGameEntity *pEntity(GetEntity(uiIndex));
            if (pEntity == nullptr)
            {
                Console.Warn << _CTS("Bad entity #") << uiIndex << _CTS(" in GAME_CMD_EXTENDED_ENTITY_DATA message") << newl;
                pkt.Advance(unLength);
            }
            else
            {
                pEntity->ReadExtendedData(pkt);

                if (pEntity->GetAsStats() != nullptr)
                {
                    CPlayer *pPlayer(Game.GetPlayer(pEntity->GetAsStats()->GetPlayerClientID()));

                    if (pPlayer != nullptr)
                        m_pInterfaceManager->StoreEndGamePlayerStats(pPlayer);
                }
            }
        }
        break;

    case GAME_CMD_TOUCH:
        {
            uint uiIndex(pkt.ReadShort(-1));
            
            if (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            uint uiActivator(pkt.ReadShort(-1));

            if (uiActivator == ushort(-1))
                uiActivator = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiIndex));
            if (pUnit == nullptr)
                break;

            if (pUnit->IsBuilding() && pUnit->GetAsBuilding()->GetIsShop())
                GameClient.GetInterfaceManager()->SetShopVisible(true);
        }
        break;

    case GAME_CMD_LEVELUP_EVENT:
        {
            uint uiIndex(pkt.ReadShort(-1));
            
            if (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiIndex));
            if (pUnit == nullptr)
                break;

            if (Game.GetActiveControlEntity() == uiIndex)
                EventActiveLevelup.Trigger(TSNULL);
            if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetHeroIndex() == uiIndex)
                EventHeroLevelup.Trigger(TSNULL);
        }
        break;

    case GAME_CMD_EXPERIENCE_EVENT:
        {
            uint uiIndex(pkt.ReadShort(-1));
            
            if (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiIndex));
            if (pUnit == nullptr)
                break;

            if (Game.GetActiveControlEntity() == uiIndex)
                EventActiveExperience.Trigger(TSNULL);
            if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetHeroIndex() == uiIndex)
                EventHeroExperience.Trigger(TSNULL);
        }
        break;

    case GAME_CMD_GOLD_EVENT:
        {
            EventPlayerGold.Trigger(TSNULL);
        }
        break;

    case GAME_CMD_INVENTORY_READY_EVENT:
        {
            uint uiIndex(pkt.ReadShort(-1));
            byte ySlot(pkt.ReadByte());
            
            if (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiIndex));
            if (pUnit == nullptr)
                break;

            if (Game.GetActiveControlEntity() != uiIndex)
                break;

            switch (ySlot)
            {
            case 0:
                EventActiveInventoryReady0.Trigger(TSNULL);
                break;
            case 1:
                EventActiveInventoryReady1.Trigger(TSNULL);
                break;
            case 2:
                EventActiveInventoryReady2.Trigger(TSNULL);
                break;
            case 3:
                EventActiveInventoryReady3.Trigger(TSNULL);
                break;
            case 4:
                EventActiveInventoryReady4.Trigger(TSNULL);
                break;
            case 5:
                EventActiveInventoryReady5.Trigger(TSNULL);
                break;
            case 6:
                EventActiveInventoryReady6.Trigger(TSNULL);
                break;
            case 7:
                EventActiveInventoryReady7.Trigger(TSNULL);
                break;
            case 8:
                EventActiveInventoryReady8.Trigger(TSNULL);
                break;
            case 9:
                EventActiveInventoryReady9.Trigger(TSNULL);
                break;
            case 25:
                EventActiveInventoryReady25.Trigger(TSNULL);
                break;
            case 26:
                EventActiveInventoryReady26.Trigger(TSNULL);
                break;
            case 27:
                EventActiveInventoryReady27.Trigger(TSNULL);
                break;
            case 28:
                EventActiveInventoryReady28.Trigger(TSNULL);
                break;
            case 29:
                EventActiveInventoryReady29.Trigger(TSNULL);
                break;
            case 30:
                EventActiveInventoryReady30.Trigger(TSNULL);
                break;
            }
        }
        break;

    case GAME_CMD_INVENTORY_UPGRADE_EVENT:
        {
            uint uiIndex(pkt.ReadShort(-1));
            byte ySlot(pkt.ReadByte());
            
            if (uiIndex == ushort(-1))
                uiIndex = INVALID_INDEX;

            CClientEntity *pClEntity(GetClientEntity(uiIndex));
            if (pClEntity == nullptr)
                break;

            IUnitEntity *pUnit(GetUnitEntity(uiIndex));
            if (pUnit == nullptr)
                break;

            if (Game.GetActiveControlEntity() != uiIndex)
                break;

            switch (ySlot)
            {
            case 0:
                EventActiveInventoryUpgrade0.Trigger(TSNULL);
                break;
            case 1:
                EventActiveInventoryUpgrade1.Trigger(TSNULL);
                break;
            case 2:
                EventActiveInventoryUpgrade2.Trigger(TSNULL);
                break;
            case 3:
                EventActiveInventoryUpgrade3.Trigger(TSNULL);
                break;
            case 4:
                EventActiveInventoryUpgrade4.Trigger(TSNULL);
                break;
            case 5:
                EventActiveInventoryUpgrade5.Trigger(TSNULL);
                break;
            case 6:
                EventActiveInventoryUpgrade6.Trigger(TSNULL);
                break;
            case 7:
                EventActiveInventoryUpgrade7.Trigger(TSNULL);
                break;
            case 8:
                EventActiveInventoryUpgrade8.Trigger(TSNULL);
                break;
            case 9:
                EventActiveInventoryUpgrade9.Trigger(TSNULL);
                break;
            case 25:
                EventActiveInventoryUpgrade25.Trigger(TSNULL);
                break;
            case 26:
                EventActiveInventoryUpgrade26.Trigger(TSNULL);
                break;
            case 27:
                EventActiveInventoryUpgrade27.Trigger(TSNULL);
                break;
            case 28:
                EventActiveInventoryUpgrade28.Trigger(TSNULL);
                break;
            case 29:
                EventActiveInventoryUpgrade29.Trigger(TSNULL);
                break;
            case 30:
                EventActiveInventoryUpgrade30.Trigger(TSNULL);
                break;
            }
        }
        break;

    case GAME_CMD_LOBBY_CONNECT_MESSAGE:
        {
            tstring sPlayer(pkt.ReadWStringAsTString());
            ushort unFlags(pkt.ReadShort());

            tsmapts mapTokens;
            mapTokens[_T("player")] = sPlayer;

            if (unFlags & PLAYER_FLAG_STAFF)
                mapTokens[_CTS("color")] = sRed;
            else if (unFlags & PLAYER_FLAG_PREMIUM)
                mapTokens[_CTS("color")] = GetInlineColorString<tstring>(GOLDENSHIELD);
            else
                mapTokens[_CTS("color")] = sNoColor;

            const tstring &sMessage(GetGameMessage(_T("lobby_connect"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerJoined"));
        }
        break;

    case GAME_CMD_LOBBY_CONNECT_MESSAGE2:
        {
            tstring sPlayer(pkt.ReadWStringAsTString());
            ushort unFlags(pkt.ReadShort());
            pkt.ReadInt(); // uiAccountIcon
            pkt.ReadInt(); // uiChatSymbol;
            uint uiChatNameColor(pkt.ReadInt());

            tsmapts mapTokens;
            mapTokens[_T("player")] = sPlayer;

            CVec4f v4Color(WHITE);

            if (uiChatNameColor != INVALID_INDEX)
                mapTokens[_CTS("color")] = GetInlineColorString<tstring>(GetColorFromString(Host.GetChatNameColorString(uiChatNameColor)));
            else if (unFlags & PLAYER_FLAG_STAFF)
                mapTokens[_CTS("color")] = sRed;
            else if (unFlags & PLAYER_FLAG_PREMIUM)
                mapTokens[_CTS("color")] = GetInlineColorString<tstring>(GOLDENSHIELD);
            else
                mapTokens[_CTS("color")] = sNoColor;

            const tstring &sMessage(GetGameMessage(_T("lobby_connect"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerJoined"));
        }
        break;

    case GAME_CMD_LOBBY_KICK_MESSAGE:
        {
            int iPlayer(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iPlayer));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_T("player")] = pPlayer->GetName();
            const tstring &sMessage(GetGameMessage(_T("lobby_kick"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerKicked"));
        }
        break;

    case GAME_CMD_LOBBY_BALANCED_MESSAGE:
        {
            const tstring &sMessage(GetGameMessage(_T("lobby_balance")));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("HostAutobalanced"));
        }
        break;

    case GAME_CMD_LOBBY_DISCONNECT_MESSAGE:
        {
            int iPlayer(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iPlayer));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;

            CVec4f v4PlayerColor(WHITE);

            if (pPlayer->GetTeam() == TEAM_PASSIVE)
                v4PlayerColor = pPlayer->GetAccountColor();
            else
                v4PlayerColor = pPlayer->GetColor();
                
            mapTokens[_CTS("color")] = GetInlineColorString<tstring>(v4PlayerColor);
            mapTokens[_CTS("player")] = pPlayer->GetName();

            const tstring &sMessage(GetGameMessage(_T("lobby_disconnect"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerLeft"));
        }
        break;

    case GAME_CMD_GAME_MESSAGE:
        {
            tstring sGameMsg(pkt.ReadWStringAsTString());

            const tstring &sMessage(GetGameMessage(sGameMsg));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;

    case GAME_CMD_LONG_SERVER_FRAME:
        {
            if (cg_showLongServerFrames)
            {       
                uint uiLength(pkt.ReadShort());

                Console.Warn << _T("Long server frame (") << uiLength << _T(" msec)") << newl;
            }
        }
        break;
        
    case GAME_CMD_WALKING_COURIER_PURCHASED_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("player")] = pPlayer->GetName();
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_T("boughtwalkingcourier"), mapTokens));
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_FLYING_COURIER_PURCHASED_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("player")] = pPlayer->GetName();
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_T("boughtflyingcourier"), mapTokens));
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;
        
    case GAME_CMD_BUYBACK:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("player")] = pPlayer->GetName();
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_T("player_boughtback"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;
        
    case GAME_CMD_KILL_RAX_MESSAGE:
        {
            uint uiVictimTeam(pkt.ReadInt(-1));
            uint uiBuildingIndex(pkt.ReadInt(INVALID_INDEX));           
            uint uiRaxTypeFlag(pkt.ReadInt(-1));            

            tstring sVictimTeamName;
            tstring sKillerTeamName;
            
            CVec4f v4KillerColor(WHITE);
            CVec4f v4VictimColor(WHITE);
            
            if (uiVictimTeam == TEAM_1)
            {
                sVictimTeamName = _T("Legion");
                sKillerTeamName = _T("Hellbourne");
                v4VictimColor = CVec4f(LEGION_RED);
                v4KillerColor = CVec4f(HELLBOURNE_GREEN);
            }
            else if (uiVictimTeam == TEAM_2)
            {
                sVictimTeamName = _T("Hellbourne");
                sKillerTeamName = _T("Legion");
                v4VictimColor = CVec4f(HELLBOURNE_GREEN);
                v4KillerColor = CVec4f(LEGION_RED);
            }
            else
            {
                sVictimTeamName = _T("SomeVictim");
                sKillerTeamName = _T("SomeKiller");
            }
                                                
            const tstring sKillerColor(_T("^") + XtoA(INT_ROUND(v4KillerColor[R] * 9)) + XtoA(INT_ROUND(v4KillerColor[G] * 9)) + XtoA(INT_ROUND(v4KillerColor[B] * 9)));
            const tstring sVictimColor(_T("^") + XtoA(INT_ROUND(v4VictimColor[R] * 9)) + XtoA(INT_ROUND(v4VictimColor[G] * 9)) + XtoA(INT_ROUND(v4VictimColor[B] * 9)));

            tsmapts mapTokens;
            
            mapTokens[_T("victim")] = sVictimColor + sVictimTeamName + _T("^*");    
            mapTokens[_T("killer")] = sKillerColor + sKillerTeamName + _T("^*");

            tstring sMessage;
            
            if (uiRaxTypeFlag == 1)
            {
                sMessage = GetGameMessage(_T("killmeleerax"), mapTokens);
            }
            else if (uiRaxTypeFlag == 2)
            {
                sMessage = GetGameMessage(_T("killrangedrax"), mapTokens);
            }
            
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                
            if (uiVictimTeam == TEAM_1)
            {
                if (!cg_muteAnnouncerVoice)
                    K2SoundManager.Play2DSound(GetLocalVoiceSet().hLegionBarracksDestroyed);
            }
            else if (uiVictimTeam == TEAM_2)
            {
                if (!cg_muteAnnouncerVoice)
                    K2SoundManager.Play2DSound(GetLocalVoiceSet().hHellbourneBarracksDestroyed);            
            }

            IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiBuildingIndex));

            if (pBuilding != nullptr)
            {
                if (GetLocalPlayer()->GetTeam() == pBuilding->GetTeam())
                    Ping(PING_ALLY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
                else
                    Ping(PING_ENEMY_BUILDING_KILL, pBuilding->GetPosition().x, pBuilding->GetPosition().y, -1);
            }
        }
        break;
        
    case GAME_CMD_TIMEDOUT_MESSAGE:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;

            CVec4f v4PlayerColor(WHITE);

            if (pPlayer->GetTeam() == TEAM_PASSIVE)
                v4PlayerColor = pPlayer->GetAccountColor();
            else
                v4PlayerColor = pPlayer->GetColor();
                
            mapTokens[_CTS("player_color")] = GetInlineColorString<tstring>(v4PlayerColor);
            mapTokens[_CTS("player")] = pPlayer->GetName();

            const tstring &sMessage(GetGameMessage(_CTS("client_timedout"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;

    case GAME_CMD_SPAWN_CLIENT_THREAD:
        {
            tstring sName(pkt.ReadWStringAsTString());

            Game.SpawnThread(sName, GetGameTime());
        }
        break;

    case GAME_CMD_UI_TRIGGER:
        {
            tstring sName(WStringToTString(pkt.ReadWString()));
            tstring sParam(WStringToTString(pkt.ReadWString()));

            CUITrigger *pUITrigger(UITriggerRegistry.GetUITrigger(sName));
            if (pUITrigger != nullptr)
                pUITrigger->Trigger(sParam);
        }
        break;

    case GAME_CMD_OPEN_SHOP:
        {
            m_pInterfaceManager->SetShopVisible(true, true);
        }
        break;

    case GAME_CMD_CLOSE_SHOP:
        {
            m_pInterfaceManager->SetShopVisible(false, true);
        }
        break;

    case GAME_CMD_LOCK_SHOP:
        {
            m_pInterfaceManager->SetShopLock(true);
        }
        break;

    case GAME_CMD_UNLOCK_SHOP:
        {
            m_pInterfaceManager->SetShopLock(false);
        }
        break;

    case GAME_CMD_SET_ACTIVE_SHOP:
        {
            tstring sShop(WStringToTString(pkt.ReadWString()));

            SetActiveShop(sShop, true);
        }
        break;

    case GAME_CMD_SET_ACTIVE_RECIPE:
        {
            tstring sRecipe(WStringToTString(pkt.ReadWString()));

            SetActiveRecipe(sRecipe, false, true);
        }
        break;
        
    case GAME_CMD_LOBBY_LOCKED_MESSAGE:
        {
            int iTeam(pkt.ReadInt(-1));
            int iSlot(pkt.ReadInt(-1));

            tsmapts mapTokens;
            mapTokens[_T("slot")] = CPlayer::GetColorName(((iTeam - 1) * 5) + iSlot);
            mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(CPlayer::GetColor(((iTeam - 1) * 5) + iSlot));

            const tstring &sMessage(GetGameMessage(_T("lobby_lock"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerLocked"));
        }
        break;
        
    case GAME_CMD_LOBBY_UNLOCKED_MESSAGE:
        {
            int iTeam(pkt.ReadInt(-1));
            int iSlot(pkt.ReadInt(-1));

            tsmapts mapTokens;
            mapTokens[_T("slot")] = CPlayer::GetColorName(((iTeam - 1) * 5) + iSlot);
            mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(CPlayer::GetColor(((iTeam - 1) * 5) + iSlot));

            const tstring &sMessage(GetGameMessage(_T("lobby_unlock"), mapTokens));
            Console.Std << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("PlayerUnlocked"));
        }
        break;
        
    case GAME_CMD_LOBBY_ASSIGNED_HOST_MESSAGE:
        {
            int iPlayer(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iPlayer));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_T("player")] = pPlayer->GetName();
            const tstring &sMessage(GetGameMessage(_T("lobby_assign_host"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;
                    
    case GAME_CMD_LOBBY_ASSIGNED_SPECTATOR_MESSAGE:
        {
            int iPlayer(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iPlayer));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_T("player")] = pPlayer->GetName();
            const tstring &sMessage(GetGameMessage(_T("lobby_assign_spectator"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;
        
    case GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE:
    case GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE2:
        {
            byte yIsReferee(-1);
            if (yCmd == GAME_CMD_LOBBY_ASSIGNED_REFEREE_MESSAGE2)
                yIsReferee = pkt.ReadByte();

            int iPlayer(pkt.ReadInt());

            CPlayer *pPlayer(GetPlayer(iPlayer));
            if (pPlayer == nullptr)
                break;

            if (yIsReferee != byte(-1))
            {
                pPlayer->SetReferee(yIsReferee != 0);
                if (yIsReferee == 0)
                    break; // don't print a message when a player is demoted as a referee.
            }

            tsmapts mapTokens;
            mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
            mapTokens[_T("player")] = pPlayer->GetName();
            const tstring &sMessage(GetGameMessage(_T("lobby_assign_referee"), mapTokens));

            Console << sMessage << newl;
            ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            ChatManager.PlaySound(_T("RecievedChannelMessage"));
        }
        break;

    case GAME_CMD_PHASE:
        {
            byte yPhase(pkt.ReadByte());
            pkt.ReadInt(); // duration
            pkt.ReadInt(); // Start time

            tstring sKey;
            tsmapts mapTokens;

            if (yPhase == GAME_PHASE_HERO_BAN)
                sKey = _T("game_phase_banning");
            else if (yPhase == GAME_PHASE_HERO_SELECT)
                sKey = _T("game_phase_picking");

            if (!sKey.empty())
            {
                const tstring &sMessage(GetGameMessage(sKey, mapTokens));
                
                Console.Std << sMessage << newl;
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                ChatManager.PlaySound(_T("RecievedChannelMessage"));
            }
        }
        break;

#if 0
    case GAME_CMD_PLAYER_TURN:
        {
            int iClientNumber(pkt.ReadInt(-1));

            CPlayer *pPlayer(GetPlayer(iClientNumber));
            if (pPlayer == nullptr)
                break;

            tsmapts mapTokens;
            mapTokens[_T("player")] = pPlayer->GetName();
            mapTokens[_T("player_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());

            const tstring &sMessage(GetGameMessage(_T("player_boughtback"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
        }
        break;
#endif

    case GAME_CMD_ASSIGN_FIRST_BAN_TEAM:
        {
            uint uiTeam(pkt.ReadInt(-1));

            CGameInfo* pGameInfo(Game.GetGameInfo());
            if (pGameInfo != nullptr)
            {
                pGameInfo->SetFirstBanTeam(uiTeam);

                tsmapts mapTokens;
                mapTokens[_T("team")] = GetTeamNameString(uiTeam);
                mapTokens[_T("team_color")] = GetTeamColorString(uiTeam);

                const tstring &sMessage(GetGameMessage(_T("lobby_assign_first_ban"), mapTokens));
                Console.Std << sMessage << newl;
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
            }
        }
        break;

    case GAME_CMD_GENERAL_MESSAGE:
        {
            // Read the game message
            const tstring& sMessage(WStringToTString(pkt.ReadWString()));

            // Read the version number (Currently the version is 1, it's defined in CGameServer::SendGeneralMessage)
            uint uiVersion(pkt.ReadInt());
            K2_UNREFERENCED_PARAMETER(uiVersion);

            // Read the reserved string
            // (This is actually unnecessary because is already a version number,
            // but it unfortunately can't be changed without breaking replays.)
            pkt.ReadWString();

            // Read the player list
            uint uiNumPlayers(CLAMP(pkt.ReadInt(), 0, 127));
            int iPlayerList[128];
            uint i;
            for (i = 0; i < uiNumPlayers; ++i)
                iPlayerList[i] = pkt.ReadInt();

            iPlayerList[i] = INVALID_INDEX;
            for (uint i = 0; i < uiNumPlayers; ++i)
            {
                int iPlayer(iPlayerList[i]);
                CPlayer *pPlayer(GetPlayer(iPlayer));
                if (pPlayer == nullptr)
                    continue;

                tsmapts mapTokens;
                mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
                mapTokens[_T("player")] = pPlayer->GetName();
                const tstring &sGameMessage(GetGameMessage(sMessage, mapTokens));
                Console.Std << sGameMessage << newl;
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sGameMessage);
            }
        }
        break;

    case GAME_CMD_PET_ADDED:
        {
            // Read the version number (Currently set to 1, defined in CPlayer::AddPet)
            // - If we ever want to change the structure of GAME_CMD_PET_ADDED, we could increment this version
            byte yVersion(pkt.ReadByte());
            K2_UNREFERENCED_PARAMETER(yVersion);

            // Read the pet index
            uint uiPetIndex(pkt.ReadInt());

            // Notify our clientcommander of the new pet
            CClientCommander *pCommander(GetClientCommander());
            if (pCommander != nullptr)
                pCommander->OnPetAdded(uiPetIndex);
        }
        break;

    case GAME_CMD_PING_ALL_MESSAGE:
        {
            byte numPlayers(pkt.ReadByte());

            if (numPlayers == 0)
            {
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, GetGameMessage(_T("player_ping_no_players")));
                break;
            }

            for (byte i = 0; i < numPlayers; ++i)
            {
                int iClientNum(pkt.ReadInt());
                ushort uiPing(pkt.ReadShort());

                CPlayer* pPlayer(Game.GetPlayer(iClientNum));
                assert(pPlayer != nullptr);
                if (pPlayer == nullptr)
                    continue;

                tsmapts mapTokens;
                mapTokens[_T("slot_color")] = GetInlineColorString<tstring>(pPlayer->GetColor());
                mapTokens[_T("player")] = pPlayer->GetName();
                mapTokens[_T("ping")] = XtoA(uiPing);
                const tstring &sGameMessage(GetGameMessage(_T("player_ping"), mapTokens));
                Console.Std << sGameMessage << newl;
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sGameMessage);
            }
        }
        break;
        
    case GAME_CMD_RIVAL_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            int iVictim(pkt.ReadInt(-1));
            byte yKillCount(pkt.ReadByte());
            
            uint uiVoice(INVALID_INDEX);

            byte y(pkt.ReadByte());
            uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            
            CPlayer *pKiller(GetPlayer(iKiller));
            CPlayer *pVictim(GetPlayer(iVictim));
            
            if (pKiller == nullptr || pVictim == nullptr)
                break;
                
            const tstring sKillerName(pKiller->GetName());
            const tstring sKillerColor(GetInlineColorString<tstring>(pKiller->GetColor()));

            const tstring sVictimName(pVictim->GetName());
            const tstring sVictimColor(GetInlineColorString<tstring>(pVictim->GetColor()));
            
            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");
            mapTokens[_T("killcount")] = XtoA(yKillCount);
                        
            const tstring &sMessage(GetGameMessage(_T("rival"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                    
            // Only show the Rival arcade text for on the Killer/Victim screen
            if (pKiller == Game.GetPlayerFromClientNumber(iKiller) || pVictim == Game.GetPlayerFromClientNumber(iVictim))
            {
                tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
                assert(!m_vAnnouncerVoices.empty());
                uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

                tsvector vMiniParams(2);
                vMiniParams[0] = XtoA(0);
                vMiniParams[1] = sSet;

                if (!cg_muteAnnouncerVoice)         
                    K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hRival);

                m_pInterfaceManager->Trigger(UITRIGGER_EVENT_RIVAL, vMiniParams);
            }           
        }
        break;
        
    case GAME_CMD_PAYBACK_MESSAGE:
        {
            int iKiller(pkt.ReadInt(-1));
            int iVictim(pkt.ReadInt(-1));
            byte yKillCount(pkt.ReadByte());

            uint uiVoice(INVALID_INDEX);

            byte y(pkt.ReadByte());
            uiVoice = y != 255 ? uint(y) : INVALID_INDEX;
            
            CPlayer *pKiller(GetPlayer(iKiller));
            CPlayer *pVictim(GetPlayer(iVictim));
            
            if (pKiller == nullptr || pVictim == nullptr)
                break;
                
            const tstring sKillerName(pKiller->GetName());
            const tstring sKillerColor(GetInlineColorString<tstring>(pKiller->GetColor()));

            const tstring sVictimName(pVictim->GetName());
            const tstring sVictimColor(GetInlineColorString<tstring>(pVictim->GetColor()));
            
            tsmapts mapTokens;
            mapTokens[_T("killer")] = sKillerColor + sKillerName + _T("^*");
            mapTokens[_T("victim")] = sVictimColor + sVictimName + _T("^*");
            mapTokens[_T("killcount")] = XtoA(yKillCount);
                        
            const tstring &sMessage(GetGameMessage(_T("payback"), mapTokens));
            Console.Std << sMessage << newl;
            if (cg_drawMessages)
                ChatManager.AddGameChatMessage(CHAT_MESSAGE_ADD, sMessage);
                    
            // Only show the Payback arcade text for on the Killer/Victim screen
            if (pKiller == Game.GetPlayerFromClientNumber(iKiller) || pVictim == Game.GetPlayerFromClientNumber(iVictim))
            {
                tstring sSet(uiVoice == INVALID_INDEX ? _T("arcade_text") : Host.GetAnnouncerVoiceArcadeText(uiVoice));
                assert(!m_vAnnouncerVoices.empty());
                uint uiSetIndex(uiVoice == INVALID_INDEX ? uint(m_vAnnouncerVoices.size() - 1) : uiVoice);

                tsvector vMiniParams(2);
                vMiniParams[0] = XtoA(0);
                vMiniParams[1] = sSet;

                if (!cg_muteAnnouncerVoice)         
                    K2SoundManager.Play2DSound(m_vAnnouncerVoices[uiSetIndex].hPayback);

                m_pInterfaceManager->Trigger(UITRIGGER_EVENT_PAYBACK, vMiniParams);
            }           
        }
        break;


        //Client Tracking Trial games.
    case GAME_CMD_TRIAL_INC:
        {
            int iTrialCount(pkt.ReadInt());
            GetClient()->SetTrialGamesCount(iTrialCount);
        }
        break;
    
    default:
        Console.Warn << _T("Unrecognized message") << newl;
        return false;
    }

    return true;
}

/*====================
  CGameClient::IsVisible
  ====================*/
bool    CGameClient::IsVisible(float fX, float fY)
{
    int iVisibilityTileWidth(GetWorldPointer()->GetTileWidth() / m_uiVisibilitySize);
    int iVisibilityTileHeight(GetWorldPointer()->GetTileHeight() / m_uiVisibilitySize);

    uint uiX(uint(CLAMP(INT_FLOOR(fX / m_fVisibilityScale), 0, iVisibilityTileWidth - 1)));
    uint uiY(uint(CLAMP(INT_FLOOR(fY / m_fVisibilityScale), 0, iVisibilityTileHeight - 1)));

    if (!m_cVisibilityMap.IsVisible(uiX, uiY))
        return false;

    return true;
}


/*====================
  CGameClient::SendGameData
  ====================*/
void    CGameClient::SendGameData(const IBuffer &buffer, bool bReliable)
{
    m_pHostClient->SendGameData(buffer, bReliable);
}


/*====================
  CGameClient::MinimapPing
  ====================*/
void    CGameClient::MinimapPing(byte yX, byte yY)
{
    bool bUnitPingFailed(false);

    if (m_uiMinimapHoverUnit != -1 && m_pClientCommander->GetPingEnabled())
    {
        CPlayer* pLocalPlayer(Game.GetLocalPlayer());

        if (!pLocalPlayer)
            bUnitPingFailed = true;

        if (!bUnitPingFailed && pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
            bUnitPingFailed = true;

        IUnitEntity *pHover(GameClient.GetUnitEntity(m_uiMinimapHoverUnit));

        if (!bUnitPingFailed && !(pHover && 
            ((pHover->IsBuilding() && !pHover->GetAsBuilding()->GetNoAltClickPing() && (pHover->GetTeam() == pLocalPlayer->GetTeam() || pLocalPlayer->IsEnemy(pHover)) && !pHover->GetAsBuilding()->GetIsShop() && !pHover->GetAsBuilding()->GetNoAltClickPing()) || 
            (pHover->IsHero() && pLocalPlayer->IsEnemy(pHover)))))
        {
            bUnitPingFailed = true;
        }

        if (!bUnitPingFailed)
        {
            CBufferFixed<5> buffer;
            buffer << GAME_CMD_UNIT_PING
                << m_uiMinimapHoverUnit;
            GameClient.SendGameData(buffer, false);
        }
    }
    else
    {
        bUnitPingFailed = true;
    }

    
    if (bUnitPingFailed && m_pClientCommander->GetPingEnabled())
    {
        CBufferFixed<3> buffer;
        buffer << GAME_CMD_MINIMAP_PING
            << yX
            << yY;
        GameClient.SendGameData(buffer, false);
    }
}


/*====================
  CGameClient::GetHeroStatus
  ====================*/
byte    CGameClient::GetHeroStatus(ushort unHeroTypeID) const
{
    for (uint ui(0); ui < NUM_HERO_LISTS; ++ui)
    {
        const HeroList &vHeroList(m_vHeroes[ui]);
        for (HeroList_cit it(vHeroList.begin()); it != vHeroList.end(); ++it)
        {
            if (it->first == unHeroTypeID)
                return it->second;
        }
    }

    return HERO_LIST_UNKNOWN;
}


/*====================
  CGameClient::TraceCursor
  ====================*/
bool    CGameClient::TraceCursor(STraceInfo &trace, int iIgnoreSurface)
{
    CVec3f v3Dir(m_pCamera->ConstructRay(Input.GetCursorPos() - m_pCamera->GetXY()));
    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), v3Dir, FAR_AWAY));

    return Game.TraceLine(trace, m_pCamera->GetOrigin(), v3End, iIgnoreSurface);
}


/*====================
  CGameClient::Cancel
  ====================*/
void    CGameClient::Cancel()
{
    if (m_bShowMenu)
    {
        m_bShowMenu = false;
        return;
    }

    if (m_uiItemCursorIndex != INVALID_INDEX)
    {
        m_uiItemCursorIndex = INVALID_INDEX;
        return;
    }

    if (m_bLevelup)
    {
        SetLevelup(false);
        return;
    }
    
    if (StopPinging())
        return;

#if 0
    if (ICvar::GetBool(_T("ui_minimapDrawing")))
    {
        ICvar::SetBool(_T("ui_minimapDrawing"), false);
        return;
    }
#endif

    if (ICvar::GetBool(_T("ui_minimapPing")))
    {
        ICvar::SetBool(_T("ui_minimapPing"), false);
        return;
    }

    CClientCommander *pCommander(GetClientCommander());
    if (pCommander != nullptr)
        pCommander->Cancel(Input.GetCursorPos());
}


/*====================
  CGameClient::AddOverlay
  ====================*/
void    CGameClient::AddOverlay(const CVec4f &v4Color, ResHandle hMaterial)
{
    SOverlayInfo overlay;
    overlay.v4Color = v4Color;
    overlay.hMaterial = hMaterial;
    m_vOverlays.push_back(overlay);
}


/*====================
  CGameClient::AddOverlay
  ====================*/
void    CGameClient::AddOverlay(const CVec4f &v4Color)
{
    AddOverlay(v4Color, g_ResourceManager.GetWhiteTexture());
}


/*====================
  CGameClient::StartEffect
  ====================*/
void    CGameClient::StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge)
{
    switch (m_eEventTarget)
    {
    case CG_EVENT_TARGET_ENTITY:
        m_pCurrentEntity->StartEffect(sEffect, iChannel, iTimeNudge);
        break;
    }
}


/*====================
  CGameClient::StopEffect
  ====================*/
void    CGameClient::StopEffect(int iChannel)
{
    switch (m_eEventTarget)
    {
    case CG_EVENT_TARGET_ENTITY:
        m_pCurrentEntity->StopEffect(iChannel);
        break;
    }
}


/*====================
  CGameClient::PlaySound
  ====================*/
void    CGameClient::PlaySound(const tstring &sSound, int iChannel, float fFalloff, float fVolume, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, bool bOverride, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime, float fFalloffEnd)
{
    PROFILE("CGameClient::PlaySound");
    
    ResHandle hSample(g_ResourceManager.LookUpPrecached(sSound));
    if (hSample == INVALID_RESOURCE)
        K2_WITH_GAME_RESOURCE_SCOPE()
            hSample = g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(sSound, iSoundFlags), RES_SAMPLE);

    if (hSample == INVALID_RESOURCE || !m_pCurrentEntity)
        return;

    m_pCurrentEntity->PlaySound
    (
        hSample,
        fVolume,
        fFalloff,
        iChannel,
        128,
        iSoundFlags,
        iFadeIn,
        iFadeOutStartTime,
        iFadeOut,
        bOverride,
        iSpeedUpTime,
        fSpeed1,
        fSpeed2,
        iSlowDownTime,
        fFalloffEnd
    );
}


/*====================
  CGameClient::PlaySoundStationary
  ====================*/
void    CGameClient::PlaySoundStationary(const tstring &sSound, int iChannel, float fFalloff, float fVolume)
{
    ResHandle hSample(g_ResourceManager.LookUpPrecached(sSound));
    if (hSample == INVALID_RESOURCE)
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
            hSample = g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(sSound, 0), RES_SAMPLE);
    }

    if (hSample == INVALID_RESOURCE || !m_pCurrentEntity)
        return;

    K2SoundManager.PlaySFXSound(hSample, &m_pCurrentEntity->GetCurrentEntity()->GetPosition(), nullptr, fVolume, fFalloff);
}


/*====================
  CGameClient::StopSound
  ====================*/
void    CGameClient::StopSound(int iChannel)
{
    m_pCurrentEntity->StopSound(iChannel);
}


/*====================
  CGameClient::PlayClientGameSound
  ====================*/
void    CGameClient::PlayClientGameSound(const tstring &sSound, int iChannel, float fVolume, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, bool bOverride, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime)
{
    PROFILE("CGameClient::PlayClientGameSound");

    ResHandle hSample(g_ResourceManager.LookUpPrecached(sSound));
    if (hSample == INVALID_RESOURCE)
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
            hSample = g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CSample)(sSound, 0), RES_SAMPLE);
    }

    if (hSample == INVALID_RESOURCE || !m_pCurrentEntity)
        return;

    // Search from an unused sound slot
    /*if (iChannel == -1)
    {
        for (int i(NUM_CLIENT_SOUND_HANDLES - 1); i >= 0; --i)
        {
            if (m_ahSoundHandle[i] == INVALID_INDEX)
            {
                iChannel = i;
                break;
            }
        }

        if (iChannel == -1)
            return;
    }*/

    if (iChannel != -1)
    {
        if (m_ahSoundHandle[iChannel] != INVALID_INDEX && K2SoundManager.IsHandleActive(m_ahSoundHandle[iChannel]))
        {
            if (!bOverride)
                return;

            K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
            m_ahSoundHandle[iChannel] = INVALID_INDEX;
        }
        m_ahSoundHandle[iChannel] = K2SoundManager.Play2DSFXSound
        (
            hSample,
            fVolume,
            -1,
            128,
            (iSoundFlags & SND_LOOP) == SND_LOOP,
            iFadeIn,
            iFadeOutStartTime,
            iFadeIn,
            iSpeedUpTime,
            fSpeed1,
            fSpeed2,
            iSlowDownTime
        );
    }
    else
    {
        K2SoundManager.Play2DSFXSound
        (
            hSample,
            fVolume,
            -1,
            128,
            (iSoundFlags & SND_LOOP) == SND_LOOP,
            iFadeIn,
            iFadeOutStartTime,
            iFadeIn,
            iSpeedUpTime,
            fSpeed1,
            fSpeed2,
            iSlowDownTime
        );
    }
}


/*====================
  CGameClient::StopClientGameSound
  ====================*/
void    CGameClient::StopClientGameSound(int iChannel)
{
    if (iChannel == -1 || iChannel >= NUM_CLIENT_SOUND_HANDLES)
        return;
    
    if (m_ahSoundHandle[iChannel] != INVALID_INDEX)
    {
        K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
    }
}


/*====================
  CGameClient::GetClientEntity
  ====================*/
CClientEntity*  CGameClient::GetClientEntity(uint uiIndex) const
{
    return m_pClientEntityDirectory->GetClientEntity(uiIndex);
}


/*====================
  CGameClient::GetClientEntityCurrent
  ====================*/
IVisualEntity*  CGameClient::GetClientEntityCurrent(uint uiIndex) const
{
    return m_pClientEntityDirectory->GetClientEntityCurrent(uiIndex);
}


/*====================
  CGameClient::GetClientEntityPrev
  ====================*/
IVisualEntity*  CGameClient::GetClientEntityPrev(uint uiIndex) const
{
    return m_pClientEntityDirectory->GetClientEntityPrev(uiIndex);
}


/*====================
  CGameClient::GetClientEntityNext
  ====================*/
IVisualEntity*  CGameClient::GetClientEntityNext(uint uiIndex) const
{
    return m_pClientEntityDirectory->GetClientEntityNext(uiIndex);
}


/*====================
  CGameClient::GetPlayerByName
  ====================*/
CPlayer*    CGameClient::GetPlayerByName(const tstring &sName) const
{
    return nullptr;
}


/*====================
  CGameClient::GetNumClients
  ====================*/
uint    CGameClient::GetNumClients() const
{
    uint uiNumPlayers(0);

    if (m_pClientEntityDirectory == nullptr)
        return 0;

    for (PlayerMap::const_iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        if (!it->second->IsDisconnected())
            uiNumPlayers++;

    return uiNumPlayers;
}


/*====================
  CGameClient::UpdateMinimap
  ====================*/
void    CGameClient::UpdateMinimap()
{
    PROFILE("CGameClient::UpdateMinimap");

    if (m_pLocalPlayer == nullptr)
        return;

    // Limit minimap updates to cg_minimapFPS
    uint uiMS(1000 / cg_minimapFPS);
    if (m_uiLastMinimapUpdateTime + uiMS > Host.GetTime())
        return;

    m_uiLastMinimapUpdateTime = Host.GetTime() / uiMS * uiMS;  // Round down to the nearest uiMS (acts like an accumulator)
 
    CBufferFixed<1> buffer;
    Minimap.Execute(_T("clear"), buffer);
    
    {
        CWorld *pWorld(GetWorldPointer());
        CVec4f v4Padding(pWorld ? pWorld->GetMinimapPadding() : CVec4f(0.0f, 0.0f, 0.0f, 0.0f));
        CBufferFixed<16> paddingBuffer;
        paddingBuffer.WriteFloat(v4Padding.x);
        paddingBuffer.WriteFloat(v4Padding.y);
        paddingBuffer.WriteFloat(v4Padding.z);
        paddingBuffer.WriteFloat(v4Padding.w);

        Minimap.Execute(_T("padding"), paddingBuffer);
    }

    // Add icons
    {
        PROFILE("Entities");

        vector<IUnitEntity*>                vDrawLast;
        vector<IUnitEntity*>::iterator      itDraw;
        vDrawLast.clear();

        CClientEntity *pEnd(m_pClientEntityDirectory->GetFirstLocalClientEntity());

        m_uiMinimapHoverUnit = UpdateMinimapHover();

        // Black outline for Buildings / creeps
        if (cg_drawMinimapOutlines)
        {
                // Buildings / creeps
            for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
            {
                CTeamInfo* pTeam(m_pLocalPlayer->GetTeamInfo());
                if (!pClEnt->GetCurrentEntity()->IsCreep() && !pClEnt->GetCurrentEntity()->IsNeutral())
                    continue;

                IUnitEntity *pUnit(pClEnt->GetCurrentEntity()->GetAsUnit());
                if (pUnit == nullptr)
                    continue;
                if (pTeam->IsTeamTarget(pUnit->GetIndex()))
                    continue;
                if (pUnit->GetIndex() == m_uiMinimapHoverUnit)
                    continue;
            
                pUnit->DrawOutlineOnMap(Minimap, m_pLocalPlayer);
            }
        }

        // Neutral camps
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            IVisualEntity *pEntity(pClEnt->GetCurrentEntity());
            if (pEntity == nullptr)
                continue;
            if (pEntity->GetType() != Entity_NeutralCampController &&
                pEntity->GetType() != Entity_BossController)
                continue;
            
            pEntity->DrawOnMap(Minimap, m_pLocalPlayer);
        }

        // Buildings
        IBuildingEntity* pHoverBuilding(0);
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            CTeamInfo* pTeam(m_pLocalPlayer->GetTeamInfo());
            IBuildingEntity *pBuilding(pClEnt->GetCurrentEntity()->GetAsBuilding());
            if (pBuilding == nullptr)
                continue;
            if (pTeam->IsTeamTarget(pBuilding->GetIndex()))
            {
                vDrawLast.push_back(pBuilding->GetAsUnit());
                continue;
            }
            if (pBuilding->GetIndex() == m_uiMinimapHoverUnit)
            {
                pHoverBuilding = pBuilding;
                continue;
            }
            
            pBuilding->DrawOnMap(Minimap, m_pLocalPlayer);
        }

        // Non-Heroes
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            IUnitEntity *pUnit(pClEnt->GetCurrentEntity()->GetAsUnit());
            if (pUnit == nullptr || pUnit->IsHero())
                continue;

            if (pUnit->IsGadget())
            {
                pUnit->GetAsGadget()->DrawOnMap(Minimap, m_pLocalPlayer);
                continue;
            }
            
            pUnit->DrawOnMap(Minimap, m_pLocalPlayer);
        }

        if (pHoverBuilding)
        {
            if (cg_drawMinimapOutlines)
                pHoverBuilding->DrawOutlineOnMap(Minimap, m_pLocalPlayer);
            pHoverBuilding->DrawOnMap(Minimap, m_pLocalPlayer);
            pHoverBuilding = 0;
        }

        // Team mates
        IHeroEntity *pHoverHero(0);
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            IHeroEntity *pHero(pClEnt->GetCurrentEntity()->GetAsHero());
            if (pHero == nullptr || m_pLocalPlayer->IsEnemy(pHero))
                continue;
            if (pHero->GetIndex() == m_uiMinimapHoverUnit)
            {
                pHoverHero = pHero;
                continue;
            }
            
            pHero->DrawOnMap(Minimap, m_pLocalPlayer);
        }

        if (pHoverHero)
        {
            pHoverHero->DrawOnMap(Minimap, m_pLocalPlayer);
            pHoverHero = 0;
        }

        // Enemies
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            CTeamInfo* pTeam(m_pLocalPlayer->GetTeamInfo());
            IHeroEntity *pHero(pClEnt->GetCurrentEntity()->GetAsHero());
            if (pHero == nullptr || !m_pLocalPlayer->IsEnemy(pHero))
                continue;
            if (pTeam->IsTeamTarget(pHero->GetIndex()))
            {
                vDrawLast.push_back(pHero->GetAsUnit());
                continue;
            }
            if (pHero->GetIndex() == m_uiMinimapHoverUnit)
            {
                pHoverHero = pHero;
                continue;
            }
            
            pHero->DrawOnMap(Minimap, m_pLocalPlayer);
        }

        if (pHoverHero)
        {
            pHoverHero->DrawOnMap(Minimap, m_pLocalPlayer);
            pHoverHero = 0;
        }
        
        for (itDraw = vDrawLast.begin(); itDraw != vDrawLast.end(); ++itDraw)
        {
            if ((*itDraw)->IsBuilding())
            {
                IBuildingEntity* pBuilding((*itDraw)->GetAsBuilding());

                if (pBuilding->GetIndex() == m_uiMinimapHoverUnit)
                {
                    pHoverBuilding = pBuilding;
                    continue;
                }

                pBuilding->DrawOnMap(Minimap, m_pLocalPlayer);
            }
            else if ((*itDraw)->IsHero())
            {
                IHeroEntity* pHero((*itDraw)->GetAsHero());

                if (pHero->GetIndex() == m_uiMinimapHoverUnit)
                {
                    pHoverHero = pHero;
                    continue;
                }

                pHero->DrawOnMap(Minimap, m_pLocalPlayer);
            }
        }

        if (pHoverHero)
            pHoverHero->DrawOnMap(Minimap, m_pLocalPlayer);
        
        if (pHoverBuilding)
            pHoverBuilding->DrawOnMap(Minimap, m_pLocalPlayer);
    }

    // Draw view box
    if (m_pLocalPlayer != nullptr && Game.GetGamePhase() >= GAME_PHASE_PRE_MATCH)
        m_pLocalPlayer->DrawViewBox(Minimap, *m_pCamera);

    UpdateMinimapPings();
}


/*====================
  CGameClient::UpdateMinimapHover
  ====================*/
uint    CGameClient::UpdateMinimapHover()
{
    uint uiUnitHoverIndex(UIManager.GetMinimapHoverUnit());

    if (m_uiLastUnitIndex == uiUnitHoverIndex)
        return uiUnitHoverIndex;

    
    IUnitEntity* pUnit(0);
    bool bShow(false);

    if (uiUnitHoverIndex != -1)
    {
        pUnit = Game.GetUnitEntity(uiUnitHoverIndex);
        bShow = true;
    }

    tsvector vParams;

    if (pUnit)
    {
        vParams.push_back(XtoA(bShow));                             //Show

        if (pUnit->IsBuilding())
        {
            IBuildingEntity* pBuilding(pUnit->GetAsBuilding());

            vParams.push_back(XtoA(false));                         // IsHero
            vParams.push_back(XtoA(pBuilding->GetIsShop()));        // IsShop
            vParams.push_back(pBuilding->GetDisplayName());         // Name
            vParams.push_back(pBuilding->GetIconPath());            // Icon
            vParams.push_back(XtoA(pBuilding->GetHealthPercent())); // HP
            vParams.push_back(XtoA(pBuilding->GetManaPercent()));   // MP
            vParams.push_back(TSNULL);                              // Inventory slot 1
            vParams.push_back(TSNULL);                              // Inventory slot 2
            vParams.push_back(TSNULL);                              // Inventory slot 3
            vParams.push_back(TSNULL);                              // Inventory slot 4
            vParams.push_back(TSNULL);                              // Inventory slot 5
            vParams.push_back(TSNULL);                              // Inventory slot 6
            vParams.push_back(XtoA(false));                         // IsGadget
        }
        else if (pUnit->IsHero())
        {
            IHeroEntity* pHero(pUnit->GetAsHero());

            vParams.push_back(XtoA(true));                      // 1 IsHero
            vParams.push_back(XtoA(false));                     // 2 IsShop
            vParams.push_back(pHero->GetDisplayName());         // 3 Name
            vParams.push_back(pHero->GetIconPath());            // 4 Icon
            vParams.push_back(XtoA(pHero->GetHealthPercent())); // 5 HP
            vParams.push_back(XtoA(pHero->GetManaPercent()));   // 6 MP
            for (int iItemSlot(INVENTORY_START_BACKPACK); iItemSlot <= INVENTORY_END_BACKPACK; ++iItemSlot)
            {
                IEntityItem* pItem(pHero->GetItem(iItemSlot));
                if (pItem)
                {
                    vParams.push_back(pItem->GetIconPath());                // 7 - 12 Inventory slot 1 - 6
                }
                else
                {
                    vParams.push_back(TSNULL);  // 7 - 12 Inventory slot 1 - 6      
                }
            }
            vParams.push_back(XtoA(false));                     // 13 IsGadget
        }
        else if (pUnit->IsGadget())
        {
            IGadgetEntity* pGadget(pUnit->GetAsGadget());

            CPlayer* pPlayer(GetLocalPlayer());

            if (pPlayer->IsEnemy(pUnit))
            {
                vParams.pop_back();
                vParams.push_back(XtoA(false));                     // Show
                vParams.push_back(XtoA(false));                     // IsHero
                vParams.push_back(XtoA(false));                     // IsShop
                vParams.push_back(TSNULL);                          // Name
                vParams.push_back(TSNULL);                          // Icon
                vParams.push_back(XtoA(0.0f));                      // HP
                vParams.push_back(XtoA(0.0f));                      // MP
                vParams.push_back(TSNULL);                          // Inventory slot 1
                vParams.push_back(TSNULL);                          // Inventory slot 2
                vParams.push_back(TSNULL);                          // Inventory slot 3
                vParams.push_back(TSNULL);                          // Inventory slot 4
                vParams.push_back(TSNULL);                          // Inventory slot 5
                vParams.push_back(TSNULL);                          // Inventory slot 6
                vParams.push_back(XtoA(false));                     // IsGadget
            }
            else
            {
                vParams.push_back(XtoA(false));                                                 // IsHero
                vParams.push_back(XtoA(false));                                                 // IsShop
                vParams.push_back(pGadget->GetDisplayName());                                   // Name
                vParams.push_back(pGadget->GetIconPath());                                      // Icon
                vParams.push_back(XtoA(pGadget->GetRemainingLifetimePercent()));                // HP
                vParams.push_back(XtoA(0.0f));                                                  // MP
                vParams.push_back(TSNULL);                                                      // Inventory slot 1
                vParams.push_back(TSNULL);                                                      // Inventory slot 2
                vParams.push_back(TSNULL);                                                      // Inventory slot 3
                vParams.push_back(TSNULL);                                                      // Inventory slot 4
                vParams.push_back(TSNULL);                                                      // Inventory slot 5
                vParams.push_back(TSNULL);                                                      // Inventory slot 6
                vParams.push_back(XtoA(true));                                                  // IsGadget
            }
        }
        else
        {
            vParams.pop_back();
            vParams.push_back(XtoA(false));                     // Show
            vParams.push_back(XtoA(false));                     // IsHero
            vParams.push_back(XtoA(false));                     // IsShop
            vParams.push_back(TSNULL);                          // Name
            vParams.push_back(TSNULL);                          // Icon
            vParams.push_back(XtoA(0.0f));                      // HP
            vParams.push_back(XtoA(0.0f));                      // MP
            vParams.push_back(TSNULL);                          // Inventory slot 1
            vParams.push_back(TSNULL);                          // Inventory slot 2
            vParams.push_back(TSNULL);                          // Inventory slot 3
            vParams.push_back(TSNULL);                          // Inventory slot 4
            vParams.push_back(TSNULL);                          // Inventory slot 5
            vParams.push_back(TSNULL);                          // Inventory slot 6
            vParams.push_back(XtoA(false));                     // IsGadget
        }
    }
    else
    {
        vParams.push_back(XtoA(false));                     // Show
        vParams.push_back(XtoA(false));                     // IsHero
        vParams.push_back(XtoA(false));                     // IsShop
        vParams.push_back(TSNULL);                          // Name
        vParams.push_back(TSNULL);                          // Icon
        vParams.push_back(XtoA(0.0f));                      // HP
        vParams.push_back(XtoA(0.0f));                      // MP
        vParams.push_back(TSNULL);                          // Inventory slot 1
        vParams.push_back(TSNULL);                          // Inventory slot 2
        vParams.push_back(TSNULL);                          // Inventory slot 3
        vParams.push_back(TSNULL);                          // Inventory slot 4
        vParams.push_back(TSNULL);                          // Inventory slot 5
        vParams.push_back(TSNULL);                          // Inventory slot 6
        vParams.push_back(XtoA(false));                     // IsGadget
    }

    MinimapHover.Trigger(vParams);
    m_uiLastUnitIndex = uiUnitHoverIndex;
    
    return uiUnitHoverIndex;
}


/*====================
  CGameClient::UpdateMinimapPings
  ====================*/
void    CGameClient::UpdateMinimapPings()
{
    CBufferFixed<40> buffer;

    for (vector<SMinimapPing>::iterator it(m_vMinimapPing.begin()); it != m_vMinimapPing.end(); )
    {
        if (Game.GetGameTime() - it->uiSpawnTime >= cg_mapPingLifetime)
            it = m_vMinimapPing.erase(it);
        else
            ++it;
    }

    for (vector<SMinimapPing>::iterator it(m_vMinimapPing.begin()); it != m_vMinimapPing.end(); ++it)
    {
        float fAlpha(CLAMP(MsToSec(it->uiSpawnTime + cg_mapPingLifetime - Game.GetGameTime()), 0.0f, 1.0f));

        buffer.Clear();
        buffer << it->v2Pos.x;
        buffer << 1.0f - it->v2Pos.y;
        buffer << 32.0f / 256.0f; // Width
        buffer << 32.0f / 256.0f; // Height
        buffer << it->v4Color[R]; // Color R
        buffer << it->v4Color[G]; // Color G
        buffer << it->v4Color[B]; // Color B
        buffer << it->v4Color[A] * fAlpha; // Color A
        buffer << it->hTexture;
        buffer << uint(-1); // Pings dont have Unit's Attached.
        Minimap.Execute(_T("icon"), buffer);
    }
}


/*====================
  CGameClient::UpdateMinimapTexture
  ====================*/
void    CGameClient::UpdateMinimapTexture()
{
    SAFE_DELETE(m_pMinimapBitmap);
    CWorld *pWorld(GetWorldPointer());

#if 0
    if (m_hMinimapTexture != INVALID_RESOURCE)
    {
        g_ResourceManager.Unregister(m_hMinimapTexture, UNREG_RESERVE_HANDLE);
        m_hMinimapTexture = INVALID_RESOURCE;
    }
#endif

    tstring sMinimapFilename(Filename_StripExtension(pWorld->GetPath()) + _T("/minimap.tga"));
    if (Vid.TextureExists(sMinimapFilename, TEX_FULL_QUALITY))
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            m_hMinimapTexture = g_ResourceManager.Register(K2_NEW(ctx_GameClient, CTexture)(sMinimapFilename, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
        }
        g_ResourceManager.UpdateReference(m_hMinimapReference, m_hMinimapTexture);
        return;
    }

    m_pMinimapBitmap = K2_NEW(ctx_GameClient, CBitmap)(pWorld->GetTileWidth(), pWorld->GetTileWidth(), BITMAP_RGB);

    map<ResHandle, CVec4b> mapColors;

    for (int iY(0); iY < m_pMinimapBitmap->GetHeight(); ++iY)
    {
        for (int iX(0); iX < m_pMinimapBitmap->GetWidth(); ++iX)
        {
            CVec3f v3Normal(Normalize(pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT) + pWorld->GetTileNormal(iX, iY, TRIANGLE_RIGHT)));

            CVec4b av4LayerColors[NUM_TERRAIN_LAYERS];

            for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
            {
                ResHandle hTexture(pWorld->GetTileDiffuseTexture(iX, iY, iLayer));

                map<ResHandle, CVec4b>::iterator findit(mapColors.find(hTexture));
                if (findit == mapColors.end())
                {
                    CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));

                    if (pTexture)
                    {
                        CVec4f v4Color(Vid.GetTextureColor(pTexture));

                        av4LayerColors[iLayer] = CVec4b(BYTE_ROUND(v4Color[R] * 255.0f), BYTE_ROUND(v4Color[G] * 255.0f), BYTE_ROUND(v4Color[B] * 255.0f), BYTE_ROUND(v4Color[A] * 255.0f));
                        mapColors[hTexture] = av4LayerColors[iLayer];
                    }
                    else
                    {
                        av4LayerColors[iLayer] = CVec4b(192, 192, 192, 255);
                        mapColors[hTexture] = av4LayerColors[iLayer];
                    }
                }
                else
                {
                    av4LayerColors[iLayer] = findit->second;
                }
            }

            int iTexelDensity(pWorld->GetTexelDensity());
            int iAlphamap(0);

            for (int iAlphaY(0); iAlphaY < iTexelDensity; ++iAlphaY)
                for (int iAlphaX(0); iAlphaX < iTexelDensity; ++iAlphaX)
                    iAlphamap += pWorld->GetTexelAlpha(iX * iTexelDensity + iAlphaX, iY * iTexelDensity + iAlphaY);

            iAlphamap /= (iTexelDensity * iTexelDensity);
            
            int iAlpha(INT_ROUND((float(pWorld->GetGridColor(iX, iY)[A]) +
                pWorld->GetGridColor(iX + 1, iY)[A] +
                pWorld->GetGridColor(iX, iY + 1)[A] +
                pWorld->GetGridColor(iX + 1, iY + 1)[A]) / 2.0f * (iAlphamap / 255.0f)));

            CVec3f v3PaintColor
            (
                float((pWorld->GetGridColor(iX, iY)[R] + pWorld->GetGridColor(iX + 1, iY)[R] + pWorld->GetGridColor(iX, iY + 1)[R] + pWorld->GetGridColor(iX + 1, iY + 1)[R]) / 4.0f) / 255.0f,
                float((pWorld->GetGridColor(iX, iY)[G] + pWorld->GetGridColor(iX + 1, iY)[G] + pWorld->GetGridColor(iX, iY + 1)[G] + pWorld->GetGridColor(iX + 1, iY + 1)[G]) / 4.0f) / 255.0f,
                float((pWorld->GetGridColor(iX, iY)[B] + pWorld->GetGridColor(iX + 1, iY)[B] + pWorld->GetGridColor(iX, iY + 1)[B] + pWorld->GetGridColor(iX + 1, iY + 1)[B]) / 4.0f) / 255.0f
            );

            CVec4b v4Color;
            v4Color[R] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][R], av4LayerColors[1][R]) * v3PaintColor[R]);
            v4Color[G] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][G], av4LayerColors[1][G]) * v3PaintColor[G]);
            v4Color[B] = byte(LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][B], av4LayerColors[1][B]) * v3PaintColor[B]);
            v4Color[A] = LERP(CLAMP(iAlpha / 255.0f, 0.0f, 1.0f), av4LayerColors[0][A], av4LayerColors[1][A]);

            float fShade(DotProduct(v3Normal, CVec3f(0.0f, 0.0f, 1.0f)) * 1.7f);

            m_pMinimapBitmap->SetPixel4b(iX, iY,
                CLAMP(INT_ROUND(v4Color[R] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[G] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[B] * fShade), 0, 255),
                255);
        }
    }

    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        m_hMinimapTexture = g_ResourceManager.Register(K2_NEW(ctx_GameClient, CTexture)(_T("*game_minimap"), m_pMinimapBitmap, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }
    g_ResourceManager.UpdateReference(m_hMinimapReference, m_hMinimapTexture);
}


/*====================
  CGameClient::ForceInterfaceRefresh
  ====================*/
void    CGameClient::ForceInterfaceRefresh()
{
    m_pInterfaceManager->ForceUpdate();
}



/*====================
  CEntityStringTableWatcher::Rebuild
  ====================*/
void    CEntityStringTableWatcher::Rebuild(ResHandle hResource)
{
    CGameMechanics *pGameMechanics(GameClient.GetGameMechanics());
    if (pGameMechanics != nullptr)
        pGameMechanics->PostLoad();

    GameClient.PostProcessEntities();
}


/*====================
  CGameClient::DrawAreaCast
  ====================*/
void    CGameClient::DrawAreaCast()
{
    IUnitEntity *pHoverUnit(Game.GetUnitEntity(m_pClientCommander->GetHoverEntity()));

    // Hover highlight
    if (cg_hoverHighlight)
    {
        if (pHoverUnit != nullptr)
        {
            pHoverUnit->SetHighlightFrame();
            pHoverUnit->SetHighlightColor(pHoverUnit->GetSelectionColor(m_pLocalPlayer) * cg_hoverHighlightStrength + CVec4f(cg_hoverHighlightBase, 1.0f));
        }
    }

    CPlayer* pLocalPlayer(Game.GetLocalPlayer());
    if (pHoverUnit && pLocalPlayer && pLocalPlayer->GetTeam() != TEAM_SPECTATOR && m_pClientCommander->GetPingModifiersDown() && m_pClientCommander->GetCommanderState() == COMSTATE_HOVER && !(pHoverUnit->IsBuilding() && (pHoverUnit->GetAsBuilding()->GetIsShop() || pHoverUnit->GetAsBuilding()->GetNoAltClickPing())))
    {
        if (pLocalPlayer->IsEnemy(pHoverUnit) && (pHoverUnit->IsBuilding() || pHoverUnit->IsHero()))
        {
            pHoverUnit->SetHighlightFrame();
            pHoverUnit->SetHighlightColor(CVec4f(1.0f, 0.05f, 0.1f, 1.0f) * (cg_hoverHighlightStrength + 0.7f) + CVec4f(cg_hoverHighlightBase, 1.0f));
        }
        else if (!pHoverUnit->IsNeutral() && pHoverUnit->IsBuilding())
        {
            pHoverUnit->SetHighlightFrame();
            pHoverUnit->SetHighlightColor(CVec4f(0.05f, 1.0f, 0.1f, 1.0f) * (cg_hoverHighlightStrength + 0.7f) + CVec4f(cg_hoverHighlightBase, 1.0f));
        }
    }

    IUnitEntity *pUnit(m_pClientCommander->GetSelectedControlEntity());
    IEntityTool *pItem(nullptr);
    if (pUnit == nullptr || (pItem = pUnit->GetTool(m_pClientCommander->GetActiveSlot())) == nullptr)
        return;

    //if (!pItem->IsReady())
    //  return;

    CVec3f v3Origin(m_pClientCommander->GetTracePosition());

    if ((pItem->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
        pItem->GetActionType() == TOOL_ACTION_TARGET_CURSOR) &&
        pItem->GetTargetRadius() > 0.0f)
    {
        // Color potential targets
        uivector vResult;
        Game.GetEntitiesInRadius(vResult, v3Origin.xy(), pItem->GetTargetRadius(), 0);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(GameClient.GetUnitEntity(GameClient.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == nullptr)
                continue;

            if (!Game.IsValidTarget(pItem->GetTargetScheme(), pItem->GetCastEffectType(), pItem->GetOwner(), pUnit, false))
                continue;

            pUnit->SetHighlightFrame();
            pUnit->SetHighlightColor(ORANGE * CVec4f(2.0f, 2.0f, 2.0f, 1.0f));
        }
    }

    if (pItem->GetActionType() == TOOL_ACTION_TARGET_VECTOR && pItem->GetTargetRadius() > 0.0f)
    {
        // Color potential targets
        uivector vResult;
        Game.GetEntitiesInRadius(vResult, v3Origin.xy(), pItem->GetTargetRadius(), 0);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(GameClient.GetUnitEntity(GameClient.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == nullptr)
                continue;

            if (!Game.IsValidTarget(pItem->GetTargetScheme(), pItem->GetCastEffectType(), pItem->GetOwner(), pUnit, false))
                continue;

            pUnit->SetHighlightFrame();
            pUnit->SetHighlightColor(ORANGE * CVec4f(2.0f, 2.0f, 2.0f, 1.0f));
        }
    }

    if ((pItem->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
        pItem->GetActionType() == TOOL_ACTION_TARGET_CURSOR) &&
        pItem->GetNoTargetRadius() > 0.0f)
    {
        // Color invalid targets
        bool bInvalidTarget(false);

        uivector vResult;
        Game.GetEntitiesInRadius(vResult, v3Origin.xy(), pItem->GetNoTargetRadius(), 0);

        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(GameClient.GetUnitEntity(GameClient.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == nullptr)
                continue;

            if (!Game.IsValidTarget(pItem->GetNoTargetScheme(), pItem->GetNoCastEffectType(), pItem->GetOwner(), pUnit, false))
                continue;

            pUnit->SetHighlightFrame();
            pUnit->SetHighlightColor(RED * CVec4f(2.0f, 2.0f, 2.0f, 1.0f));
            bInvalidTarget = true;
        }

        ResHandle hMaterial(pItem ? pItem->GetNoTargetMaterial() : INVALID_RESOURCE);

        if (bInvalidTarget &&
            pItem != nullptr &&
            pItem->GetNoTargetRadius() > 0.0f &&
            hMaterial != INVALID_RESOURCE &&
            (pItem->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
            pItem->GetActionType() == TOOL_ACTION_TARGET_CURSOR))
        {
            CSceneEntity scTargetSprite;
            scTargetSprite.Clear();
            scTargetSprite.objtype = OBJTYPE_GROUNDSPRITE;
            scTargetSprite.angle = 0.0f;
            scTargetSprite.hRes = hMaterial;
            scTargetSprite.SetPosition(v3Origin);
            scTargetSprite.width = scTargetSprite.height = pItem->GetNoTargetRadius();
            SceneManager.AddEntity(scTargetSprite);
        }
    }
}


/*====================
  CGameClient::IsEntitySelected
  ====================*/
bool    CGameClient::IsEntitySelected(uint uiIndex)
{
    return m_pClientCommander->IsSelected(uiIndex);
}


/*====================
  CGameClient::IsEntityHoverSelected
  ====================*/
bool    CGameClient::IsEntityHoverSelected(uint uiIndex)
{
    return m_pClientCommander->IsHoverSelected(uiIndex);
}


/*====================
  CGameClient::GetActiveControlEntity
  ====================*/
uint    CGameClient::GetActiveControlEntity()
{
    return m_pClientCommander->GetActiveControlEntity();
}


/*====================
  CGameClient::GetLocalClientNum
  ====================*/
int     CGameClient::GetLocalClientNum()
{
    return m_pHostClient->GetClientNum();
}


/*====================
  CGameClient::DrawVoiceInfo
  ====================*/
void    CGameClient::DrawVoiceInfo()
{
/*  map<uint, uint>::iterator it(m_mapVoiceMarkers.begin());
    IVisualEntity *pPlayer;

    while (it != m_mapVoiceMarkers.end())
    {
        if (GetGameTime() > it->second)
        {
            STL_ERASE(m_mapVoiceMarkers, it);
            continue;
        }

        pPlayer = GetClientEntityCurrent(it->first);

        if (pPlayer == nullptr)
        {
            STL_ERASE(m_mapVoiceMarkers, it);
            continue;
        }

        ResHandle hHandle(g_ResourceManager.Register(_T("/shared/effects/levelup.effect"), RES_EFFECT));

        if (pPlayer->GetEffect(EFFECT_CHANNEL_VOICECOMMAND) == hHandle || hHandle == INVALID_RESOURCE)
        {
            it++;
            continue;
        }

        pPlayer->SetEffect(EFFECT_CHANNEL_VOICECOMMAND, hHandle);
        pPlayer->IncEffectSequence(EFFECT_CHANNEL_VOICECOMMAND);

        it++;
    }

    for (VoiceClientMap_it it(m_vVoiceMap.begin()); it != m_vVoiceMap.end(); it++)
    {
        if (it->second.bTalking)
        {
            pPlayer = GetPlayer(it->first);

            if (pPlayer == nullptr)
                continue;

            pPlayer = GetClientEntityCurrent(pPlayer->GetIndex());

            if (pPlayer == nullptr)
                continue;

            cRect = CRectf(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()));
            v3EntPos = pPlayer->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pPlayer->GetModelHandle()).GetMax().z * pPlayer->GetScale());

            if (m_pCamera->IsPointInScreenRect(v3EntPos, cRect) && m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
            {
                v2ScreenPos.x = floor(v2ScreenPos.x);
                v2ScreenPos.y = floor(v2ScreenPos.y);

                UIManager.Render(m_hVoiceChat, v2ScreenPos);
            }
        }
    }

    if (m_vcLocalClient.bTalking)
    {
        pPlayer = GetPlayer();

        if (pPlayer != nullptr)
            pPlayer = GetClientEntityCurrent(pPlayer->GetIndex());

        if (pPlayer != nullptr)
        {
            cRect = CRectf(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()));
            v3EntPos = pPlayer->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pPlayer->GetModelHandle()).GetMax().z * pPlayer->GetScale());

            if (m_pCamera->IsPointInScreenRect(v3EntPos, cRect) && m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
            {
                v2ScreenPos.x = floor(v2ScreenPos.x);
                v2ScreenPos.y = floor(v2ScreenPos.y);

                UIManager.Render(m_hVoiceChat, v2ScreenPos);
            }
        }
    }*/
}


/*====================
  CGameClient::DrawAltInfo
  ====================*/
void    CGameClient::DrawAltInfo()
{
    PROFILE("CGameClient::DrawAltInfo");

    if (m_pLocalPlayer == nullptr)
        return;

    // Aquire the interfaces
    CInterface *pInterfaceGeneric(UIManager.GetInterface(m_ahResources[CLIENT_RESOURCE_ALT_INFO_INTERFACE]));
    CInterface *pInterfaceCreep(UIManager.GetInterface(m_ahResources[CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE]));
    CInterface *pInterfaceHero(UIManager.GetInterface(m_ahResources[CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE]));
    CInterface *pInterfaceBuilding(UIManager.GetInterface(m_ahResources[CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE]));

    if (pInterfaceGeneric == nullptr ||
        pInterfaceCreep == nullptr ||
        pInterfaceHero == nullptr ||
        pInterfaceBuilding == nullptr)
        return;

    CRectf cRect(m_pCamera->GetX(), m_pCamera->GetY(), m_pCamera->GetX() + m_pCamera->GetWidth(), m_pCamera->GetY() + m_pCamera->GetHeight());

    static set<IUnitEntity*> setEntities;
    setEntities.clear();

    if (cg_alwaysShowHealthBars || m_pClientCommander->GetModifier3() || AltInfoAllies || AltInfoEnemies)
    {
        // Generate list of potential entities
        CClientEntity *pEnd(m_pClientEntityDirectory->GetFirstLocalClientEntity());
        for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
        {
            IUnitEntity *pUnit(pClEnt->GetCurrentEntity() ? pClEnt->GetCurrentEntity()->GetAsUnit() : nullptr);
            if (pUnit == nullptr)
                continue;
            if (pUnit->GetTeam() == TEAM_PASSIVE)
                continue;
            if (pUnit->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;
            if (!pUnit->GetIsSelectable())
                continue;
            if (!m_pLocalPlayer->CanSee(pUnit))
                continue;
            if (!cg_alwaysShowHealthBars && !m_pClientCommander->GetModifier3() && m_pLocalPlayer->IsEnemy(pUnit) && !AltInfoEnemies)
                continue;
            if (!cg_alwaysShowHealthBars && !m_pClientCommander->GetModifier3() && !m_pLocalPlayer->IsEnemy(pUnit) && !AltInfoAllies)
                continue;

            setEntities.insert(pUnit);
        }
    }

    // Add hovered entities to the list of potential entities
    const uiset &setHoverEntities(m_pClientCommander->GetHoveredEntities());
    for (uiset_cit it(setHoverEntities.begin()); it != setHoverEntities.end(); ++it)
    {
        IUnitEntity *pUnit(GetUnitEntity(*it));
        if (pUnit == nullptr)
            continue;
        if (pUnit->GetStatus() != ENTITY_STATUS_ACTIVE)
            continue;
        if (m_pClientEntityDirectory->IsLocalEntity(*it))
            continue;
        if (!pUnit->GetIsSelectable() && !pUnit->IsType(Prop_Tree))
            continue;
        if (!m_pLocalPlayer->CanSee(pUnit) && !pUnit->GetAlwaysTargetable())
            continue;

        setEntities.insert(pUnit);
    }

    bool bSpectator(m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR);

    // Render interfaces
    for (set<IUnitEntity*>::iterator it(setEntities.begin()); it != setEntities.end(); ++it)
    {
        IUnitEntity *pEntity(*it);
        
        if (pEntity == nullptr)
            continue;

        float fOffset;

        if (pEntity->GetInfoHeight() != 0.0f)
            fOffset = pEntity->GetInfoHeight();
        else
            fOffset = g_ResourceManager.GetModelBounds(pEntity->GetModel()).GetMax().z * pEntity->GetBaseScale() * pEntity->GetScale();

        CVec3f v3EntPos(pEntity->GetPosition());
        v3EntPos.z += fOffset + 8.0f;
        if (!m_pCamera->IsPointInScreenRect(v3EntPos, cRect))
            continue;

        CVec2f v2ScreenPos;
        if (!m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
            continue;

        v2ScreenPos += CVec2f(m_pCamera->GetX(), m_pCamera->GetY());

        CPlayer *pPlayer(GetPlayerFromClientNumber(pEntity->GetOwnerClientNumber()));
        CTeamInfo *pTeam(Game.GetTeam(pEntity->GetTeam()));

        v2ScreenPos.x = ROUND(v2ScreenPos.x);
        v2ScreenPos.y = ROUND(v2ScreenPos.y);

        ResHandle hAltInfo;
        CInterface *pInterface;
        EGameUITrigger eTriggerPlayer;
        EGameUITrigger eTriggerName;
        EGameUITrigger eTriggerTeam;
        EGameUITrigger eTriggerColor;
        EGameUITrigger eTriggerHasHealth, eTriggerHealthPercent, eTriggerHealthLerp;
        EGameUITrigger eTriggerHasMana, eTriggerManaPercent;
        EGameUITrigger eTriggerLevel;
        EGameUITrigger eTriggerShield, eTriggerMaxShield, eTriggerShieldPercent;
        EGameUITrigger eTriggerItems;
        EGameUITrigger eTriggerInExpRange;

        if (pEntity->IsHero())
        {
            pInterface = pInterfaceHero;
            hAltInfo = m_ahResources[CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE];
            eTriggerPlayer = UITRIGGER_ALT_INFO_0_PLAYER;
            eTriggerName = UITRIGGER_ALT_INFO_0_NAME;
            eTriggerTeam = UITRIGGER_ALT_INFO_0_TEAM;
            eTriggerColor = UITRIGGER_ALT_INFO_0_COLOR;
            eTriggerHasHealth = UITRIGGER_ALT_INFO_0_HAS_HEALTH;
            eTriggerHealthPercent = UITRIGGER_ALT_INFO_0_HEALTH_PERCENT;
            eTriggerHealthLerp = UITRIGGER_ALT_INFO_0_HEALTH_LERP;
            eTriggerHasMana = UITRIGGER_ALT_INFO_0_HAS_MANA;
            eTriggerManaPercent = UITRIGGER_ALT_INFO_0_MANA_PERCENT;
            eTriggerLevel = UITRIGGER_ALT_INFO_0_LEVEL;
            eTriggerItems = UITRIGGER_ALT_INFO_0_ITEMS;
            eTriggerShield = UITRIGGER_ALT_INFO_0_SHIELD;
            eTriggerMaxShield = UITRIGGER_ALT_INFO_0_MAX_SHIELD;
            eTriggerShieldPercent = UITRIGGER_ALT_INFO_0_SHIELD_PERCENT;
            eTriggerInExpRange = NUM_UITRIGGERS;
        }
        else if (pEntity->IsBuilding() || pEntity->GetLargeUnit())
        {
            pInterface = pInterfaceBuilding;
            hAltInfo = m_ahResources[CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE];
            eTriggerPlayer = UITRIGGER_ALT_INFO_3_PLAYER;
            eTriggerName = UITRIGGER_ALT_INFO_3_NAME;
            eTriggerTeam = UITRIGGER_ALT_INFO_3_TEAM;
            eTriggerColor = UITRIGGER_ALT_INFO_3_COLOR;
            eTriggerHasHealth = UITRIGGER_ALT_INFO_3_HAS_HEALTH;
            eTriggerHealthPercent = UITRIGGER_ALT_INFO_3_HEALTH_PERCENT;
            eTriggerHealthLerp = UITRIGGER_ALT_INFO_3_HEALTH_LERP;
            eTriggerHasMana = UITRIGGER_ALT_INFO_3_HAS_MANA;
            eTriggerManaPercent = UITRIGGER_ALT_INFO_3_MANA_PERCENT;
            eTriggerLevel = UITRIGGER_ALT_INFO_3_LEVEL;
            eTriggerItems = NUM_UITRIGGERS;
            eTriggerShield = UITRIGGER_ALT_INFO_3_SHIELD;
            eTriggerMaxShield = UITRIGGER_ALT_INFO_3_MAX_SHIELD;
            eTriggerShieldPercent = UITRIGGER_ALT_INFO_3_SHIELD_PERCENT;
            eTriggerInExpRange = UITRIGGER_ALT_INFO_3_IS_IN_EXP_RANGE;
        }
        else if (pEntity->GetOwnerClientNumber() != -1)
        {
            pInterface = pInterfaceCreep;
            hAltInfo = m_ahResources[CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE];
            eTriggerPlayer = UITRIGGER_ALT_INFO_1_PLAYER;
            eTriggerName = UITRIGGER_ALT_INFO_1_NAME;
            eTriggerTeam = UITRIGGER_ALT_INFO_1_TEAM;
            eTriggerColor = UITRIGGER_ALT_INFO_1_COLOR;
            eTriggerHasHealth = UITRIGGER_ALT_INFO_1_HAS_HEALTH;
            eTriggerHealthPercent = UITRIGGER_ALT_INFO_1_HEALTH_PERCENT;
            eTriggerHealthLerp = UITRIGGER_ALT_INFO_1_HEALTH_LERP;
            eTriggerHasMana = UITRIGGER_ALT_INFO_1_HAS_MANA;
            eTriggerManaPercent = UITRIGGER_ALT_INFO_1_MANA_PERCENT;
            eTriggerLevel = UITRIGGER_ALT_INFO_1_LEVEL;
            eTriggerItems = NUM_UITRIGGERS;
            eTriggerShield = UITRIGGER_ALT_INFO_1_SHIELD;
            eTriggerMaxShield = UITRIGGER_ALT_INFO_1_MAX_SHIELD;
            eTriggerShieldPercent = UITRIGGER_ALT_INFO_1_SHIELD_PERCENT;
            eTriggerInExpRange = UITRIGGER_ALT_INFO_1_IS_IN_EXP_RANGE;
        }
        else
        {
            pInterface = pInterfaceGeneric;
            hAltInfo = m_ahResources[CLIENT_RESOURCE_ALT_INFO_INTERFACE];
            eTriggerPlayer = UITRIGGER_ALT_INFO_2_PLAYER;
            eTriggerName = UITRIGGER_ALT_INFO_2_NAME;
            eTriggerTeam = UITRIGGER_ALT_INFO_2_TEAM;
            eTriggerColor = UITRIGGER_ALT_INFO_2_COLOR;
            eTriggerHasHealth = UITRIGGER_ALT_INFO_2_HAS_HEALTH;
            eTriggerHealthPercent = UITRIGGER_ALT_INFO_2_HEALTH_PERCENT;
            eTriggerHealthLerp = UITRIGGER_ALT_INFO_2_HEALTH_LERP;
            eTriggerHasMana = UITRIGGER_ALT_INFO_2_HAS_MANA;
            eTriggerManaPercent = UITRIGGER_ALT_INFO_2_MANA_PERCENT;
            eTriggerLevel = UITRIGGER_ALT_INFO_2_LEVEL;
            eTriggerItems = NUM_UITRIGGERS;
            eTriggerShield = UITRIGGER_ALT_INFO_2_SHIELD;
            eTriggerMaxShield = UITRIGGER_ALT_INFO_2_MAX_SHIELD;
            eTriggerShieldPercent = UITRIGGER_ALT_INFO_2_SHIELD_PERCENT;
            eTriggerInExpRange = UITRIGGER_ALT_INFO_2_IS_IN_EXP_RANGE;
        }

        pInterface->SetAlwaysUpdate(true);

        if (pPlayer != nullptr)
            m_pInterfaceManager->Trigger(eTriggerPlayer, pPlayer->GetName());
        else if (pTeam != nullptr)
            m_pInterfaceManager->Trigger(eTriggerPlayer, pTeam->GetName());
        else
            m_pInterfaceManager->Trigger(eTriggerPlayer, TSNULL);

        if (cg_alwaysShowHealthBars && m_pClientCommander->IsDragSelecting() && m_pClientCommander->IsHoverSelected(pEntity->GetIndex()))
            continue;
        else if ((!m_pClientCommander->IsDragSelecting() && m_pClientCommander->IsHoverSelected(pEntity->GetIndex())) || pEntity->IsType(Entity_Chest))
            m_pInterfaceManager->Trigger(eTriggerName, pEntity->GetDisplayName());
        else
            m_pInterfaceManager->Trigger(eTriggerName, TSNULL);

        static tsvector vTeam(2);
        vTeam[0] = XtoA(pEntity->GetTeam());

        if (bSpectator)
            vTeam[1] = XtoA(pEntity->GetTeam() == 1);
        else
            vTeam[1] = XtoA(!m_pLocalPlayer->IsEnemy(pEntity));

        m_pInterfaceManager->Trigger(eTriggerTeam, vTeam);

        static tsvector vColor(2);
        vColor[0] = XtoA(pEntity->GetSelectionColor(m_pLocalPlayer));
        if (pPlayer != nullptr)
            vColor[1] = XtoA(pPlayer->GetColor());
        else if (pTeam != nullptr)
            vColor[1] = XtoA(pTeam->GetColor());
        else
            vColor[1] = XtoA(WHITE);
        m_pInterfaceManager->Trigger(eTriggerColor, vColor);

        bool bVisible(m_pLocalPlayer->CanSee(pEntity));
        
        if (bVisible)
        {
            m_pInterfaceManager->Trigger(eTriggerHasHealth, pEntity->GetMaxHealth() != 0.0f);
            m_pInterfaceManager->Trigger(eTriggerHealthPercent, pEntity->GetHealthPercent());
            if((pInterface == pInterfaceGeneric && cg_showGenericHealthLerp) ||
                (pInterface == pInterfaceCreep && cg_showCreepHealthLerp) ||
                (pInterface == pInterfaceHero && cg_showHeroHealthLerp) ||
                (pInterface == pInterfaceBuilding && cg_showBuildingHealthLerp))
                m_pInterfaceManager->Trigger(eTriggerHealthLerp, pEntity->GetHealthShadow());
            else
                m_pInterfaceManager->Trigger(eTriggerHealthLerp, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerHasMana, pEntity->GetMaxMana() != 0.0f);
            m_pInterfaceManager->Trigger(eTriggerManaPercent, pEntity->GetManaPercent());
            m_pInterfaceManager->Trigger(eTriggerShield, pEntity->GetShield());
            m_pInterfaceManager->Trigger(eTriggerMaxShield, pEntity->GetMaxShield());
            m_pInterfaceManager->Trigger(eTriggerShieldPercent, pEntity->GetShieldPercent());

            if (pEntity->IsHero())
            {
                tsvector sParams;
                IHeroEntity* pHero(pEntity->GetAsHero());
                if (pHero)
                {
                    for (int iItemSlot(INVENTORY_START_BACKPACK); iItemSlot <= INVENTORY_END_BACKPACK; ++iItemSlot)
                    {
                        IEntityItem* pItem(pHero->GetItem(iItemSlot));
                        if (pItem)
                            sParams.push_back(pItem->GetIconPath());
                        else
                            sParams.push_back(TSNULL);
                    }
                }
                else
                {
                    for (int iItemSlot(0); iItemSlot < 6; ++iItemSlot)
                    {
                        sParams.push_back(TSNULL);
                    }
                }

                if (eTriggerItems != NUM_UITRIGGERS)
                    m_pInterfaceManager->Trigger(eTriggerItems, sParams);
            }
            else if (pEntity->IsCreep())
            {
                CPlayer* pLocalPlayer(Game.GetLocalPlayer());

                if (pLocalPlayer && pLocalPlayer->IsEnemy(pEntity))
                {
                    IHeroEntity* pHero(pLocalPlayer->GetHero());
                    if (pHero != nullptr)
                    {
                        CVec3f vCreepPos(pEntity->GetPosition());
                        CWorldEntity* pWorldHeroEntity(Game.GetWorldEntity(pHero->GetWorldIndex()));
                        CVec3f vHeroPos(pWorldHeroEntity->GetWorldBounds().Clamp(vCreepPos));
                        if (cg_showExperienceRangeIndicator && DistanceSq(vHeroPos.xy(), vCreepPos.xy()) < SQR(Game.GetExperienceRange()))
                        {
                            m_pInterfaceManager->Trigger(eTriggerInExpRange, _T("true"));
                        }
                        else
                        {
                            m_pInterfaceManager->Trigger(eTriggerInExpRange, _T("false"));
                        }
                    }
                }
                else
                {
                    m_pInterfaceManager->Trigger(eTriggerInExpRange, _T("false"));
                }
            }
        }
        else
        {
            m_pInterfaceManager->Trigger(eTriggerHasHealth, false);
            m_pInterfaceManager->Trigger(eTriggerHealthPercent, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerHealthLerp, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerHasMana, false);
            m_pInterfaceManager->Trigger(eTriggerManaPercent, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerShield, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerMaxShield, 0.0f);
            m_pInterfaceManager->Trigger(eTriggerShieldPercent, 0.0f);

            if (pEntity->IsHero())
            {
                tsvector sParams;
                for (int iItemSlot(0); iItemSlot < 6; ++iItemSlot)
                {
                    sParams.push_back(TSNULL);
                }
                if (eTriggerItems != NUM_UITRIGGERS)
                    m_pInterfaceManager->Trigger(eTriggerItems, sParams);
            }
            else if (pEntity->IsCreep())
            {
                m_pInterfaceManager->Trigger(eTriggerInExpRange, _T("false"));
            }
        }
        
        if (pEntity->IsGadget())
        {
            IGadgetEntity *pGadget(pEntity->GetAsGadget());
            CGadgetDefinition *pGadDef(pGadget->GetDefinition<CGadgetDefinition>()); 
            if (pGadget && pGadDef->GetShowLifetime())
            {
                m_pInterfaceManager->Trigger(eTriggerLevel, XtoA(pEntity->GetAsGadget()->GetRemainingLifetimePercent()));
            }
            else
            {
                m_pInterfaceManager->Trigger(eTriggerLevel, XtoA(0));
            }
        }
        else
        {
            if (pEntity->IsHero())
            {
                m_pInterfaceManager->Trigger(eTriggerLevel, (pEntity ? XtoA(pEntity->GetLevel()) : XtoA(0)));
            }
            else
            {
                m_pInterfaceManager->Trigger(eTriggerLevel, XtoA(0));
            }

        }

        UIManager.Render(hAltInfo, v2ScreenPos);

        pInterfaceGeneric->SetAlwaysUpdate(false);
        pInterfaceCreep->SetAlwaysUpdate(false);
        pInterfaceHero->SetAlwaysUpdate(false);
    }
}


/*====================
  CGameClient::SendMessage
  ====================*/
void    CGameClient::SendMessage(const tstring &sMsg, int iClientNum)
{
    if (iClientNum == -1 || iClientNum == GetLocalClientNum())
    {
        if (cg_drawMessages)
            GameMessage.Trigger(sMsg);
    }
}


/*====================
  CGameClient::DrawPopupMessages
  ====================*/
void    CGameClient::DrawPopupMessages()
{
    // Update list
    for (vector<SPopupMessage>::iterator it(m_vPopupMessage.begin()); it != m_vPopupMessage.end(); )
    {
        // Delete expired popups
        if (it->uiSpawnTime != INVALID_TIME && (it->uiSpawnTime + it->uiLifeTime < GetGameTime() || it->uiSpawnTime > GetGameTime()))
        {
            it = m_vPopupMessage.erase(it);
            continue;
        }

        // Check for delayed popups that need to get their position
        if (it->uiEntityIndex == INVALID_INDEX || (it->uiServerTime != INVALID_TIME && it->uiServerTime > GetServerTime()))
        {
            ++it;
            continue;
        }

        IUnitEntity *pEntity(GetUnitEntity(it->uiEntityIndex));
        if (pEntity == nullptr)
        {
            it = m_vPopupMessage.erase(it);
            continue;
        }

        CVec3f v3Pos(pEntity->GetPosition() + pEntity->GetBounds().GetMid());
        CVec2f v2ScreenPos0;
        CVec2f v2ScreenPos1;
        float fScale(1.0f);

        if (m_pCamera->WorldToScreen(v3Pos, v2ScreenPos0) &&
            m_pCamera->WorldToScreen(v3Pos + m_pCamera->GetViewAxis(UP), v2ScreenPos1))
        {
            fScale = 1.0f / Distance(v2ScreenPos0, v2ScreenPos1);
        }

        it->v3Pos = v3Pos + (m_pCamera->GetViewAxis(RIGHT) * M_Randnum(-20.0f, 20.0f) + m_pCamera->GetViewAxis(UP) * M_Randnum(-2.5f, 2.5f)) * fScale;
        it->uiEntityIndex = INVALID_INDEX;
        it->uiSpawnTime = GetGameTime();

        ++it;
    }

    CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hPopupFont));

    for (vector<SPopupMessage>::iterator it(m_vPopupMessage.begin()); it != m_vPopupMessage.end(); ++it)
    {
        if (it->uiServerTime != INVALID_TIME && it->uiServerTime > GetServerTime())
            continue;

        CVec2f v2ScreenPos;
        if (m_pCamera->WorldToScreen(it->v3Pos, v2ScreenPos))
        {
            CVec2f v2StringPos(ROUND(v2ScreenPos.x - pFontMap->GetStringWidth(it->sText) / 2.0f), ROUND(v2ScreenPos.y - pFontMap->GetMaxHeight() / 2.0f));

            v2StringPos += (it->v2Start + it->v2Speed * MsToSec(GetGameTime() - it->uiSpawnTime)) * Draw2D.GetScreenH() / 100.0f;

            float fAlpha;
            if (it->uiFadeTime == 0)
                fAlpha = 1.0f;
            else
                fAlpha = 1.0f - CLAMP(float(int(GetGameTime() - it->uiSpawnTime - it->uiLifeTime + it->uiFadeTime)) / it->uiFadeTime, 0.0f, 1.0f);

            Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, fAlpha));
            Draw2D.String(v2StringPos.x + 2, v2StringPos.y + 2, it->sText, m_hPopupFont);

            Draw2D.SetColor(CVec4f(it->v4Color.xyz(), fAlpha));
            Draw2D.String(v2StringPos.x, v2StringPos.y, it->sText, m_hPopupFont);
        }
    }
}


/*====================
  CGameClient::SpawnPopupMessage
  ====================*/
void    CGameClient::SpawnPopupMessage(const tstring &sText, uint uiEntityIndex, const CVec4f &v4Color, float fStartX, float fStartY, float fSpeedX, float fSpeedY, uint uiLifeTime, uint uiFadeTime, uint uiServerTime)
{
    if (uiEntityIndex == INVALID_INDEX)
        return;

    SPopupMessage popup;
    popup.sText = sText;
    popup.uiEntityIndex = uiEntityIndex;
    popup.uiServerTime = uiServerTime;
    popup.uiSpawnTime = uiServerTime == INVALID_TIME ? GetGameTime() : INVALID_TIME;
    popup.uiLifeTime = uiLifeTime;
    popup.uiFadeTime = uiFadeTime;
    popup.v4Color = v4Color;
    popup.v2Start = CVec2f(fStartX, -fStartY);
    popup.v2Speed = CVec2f(fSpeedX, -fSpeedY);

    m_vPopupMessage.push_back(popup);
}


/*====================
  CGameClient::SpawnMinimapPing
  ====================*/
void    CGameClient::SpawnMinimapPing(ResHandle hTexture, const CVec2f &v2Pos, const CVec4f &v4Color, bool bPlaySound)
{
#if 0
    SMinimapPing ping;
    ping.hTexture = hTexture;
    ping.v2Pos = v2Pos;
    ping.v4Color = v4Color;
    ping.uiSpawnTime = Game.GetGameTime();

    m_vMinimapPing.push_back(ping);
#endif

    if (bPlaySound)
        K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_PING_SAMPLE]);
}


/*====================
  CGameClient::PrecacheEntities
  ====================*/
void    CGameClient::PrecacheEntities()
{
    PROFILE_EX("CGameClient::PrecacheEntities", PROFILE_GAME_PRECACHE_ENTITIES);

    K2_WITH_GAME_RESOURCE_SCOPE()
        RegisterGameMechanics(_T("/base.gamemechanics"));

    if (!FetchGameMechanics())
        Console.Err << _T("Missing game mechanics!") << newl;

    // Register dynamic entity definitions
    tsvector vFileList;
    FileManager.GetFileList(_T("/"), _T("*.entity"), true, vFileList);

    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
            g_ResourceManager.Register(*it, RES_ENTITY_DEF);
    }

    //if (!cg_precacheEntities)
        //return;

    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("Game_Rules"), RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, _T("Entity_Chest"), RES_ENTITY_DEF);

    // Shops and items
    vector<ushort> vShops;
    EntityRegistry.GetShopList(vShops);

    for (vector<ushort>::iterator it(vShops.begin()), itEnd(vShops.end()); it != itEnd; ++it)
    {
        AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, EntityRegistry.LookupName(*it), RES_ENTITY_DEF);

        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr)
            continue;

        const tsvector &vsItems(pShop->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
            AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, *cit, RES_ENTITY_DEF);
    }

    // Powerups
    for (uint uiPowerup(0); uiPowerup < g_powerups.GetSize(); ++uiPowerup)
        AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_powerups[uiPowerup], RES_ENTITY_DEF);

    // Critters
    for (uint uiCritter(0); uiCritter < g_critters.GetSize(); ++uiCritter)
        AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_critters[uiCritter], RES_ENTITY_DEF);

    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam1Melee, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam1Ranged, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam1Siege, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam2Melee, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam2Ranged, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_creepTeam2Siege, RES_ENTITY_DEF);
    AddResourceToLoadingQueue(CLIENT_RESOURCE_UNTRACKED, g_waypoint, RES_ENTITY_DEF);
}


/*====================
  CGameClient::DrawFogofWar
  ====================*/
void    CGameClient::DrawFogofWar()
{
    PROFILE("CGameClient::DrawFogofWar");

    if (m_pLocalPlayer == nullptr)
    {
        Vid.RenderFogofWar(1.0f, false, 0.0f);
        return;
    }
    
    const uint FOW_UPDATE_TIME(g_fogofwarUpdateTime);

    // Check for rewind
    if (m_uiLastFogofWarUpdate > Game.GetGameTime())
        m_uiLastFogofWarUpdate = Game.GetGameTime() / FOW_UPDATE_TIME * FOW_UPDATE_TIME; // Round down to the nearest uiMS (acts like an accumulator)

    if (m_uiLastFogofWarUpdate == INVALID_TIME || m_uiLastFogofWarUpdate + FOW_UPDATE_TIME <= Game.GetGameTime())
    {
        if (GetGamePhase() == GAME_PHASE_ENDED || !cg_fogofwar)
        {
            m_cVisibilityMap.Clear(255);
        }
        else
        {
            m_cVisibilityMap.Clear(0);
            int iTeam(m_pLocalPlayer->GetTeam());

            CClientEntity *pEnd(m_pClientEntityDirectory->GetFirstLocalClientEntity());
            for (CClientEntity *pClEnt(m_pClientEntityDirectory->GetFirstClientEntity()); pClEnt != pEnd; pClEnt = pClEnt->GetNextClientEntity())
            {
                IVisualEntity *pCurrent(pClEnt->GetCurrentEntity());

                if (pCurrent == nullptr || !pCurrent->IsUnit())
                    continue;

                IUnitEntity *pCurrentUnit(pCurrent->GetAsUnit());

                if (m_pLocalPlayer->IsIsolated() && pCurrentUnit->GetOwnerClientNumber() != m_pLocalPlayer->GetClientNumber())
                    continue;

                bool bSpectate(iTeam == TEAM_SPECTATOR && pCurrent->GetTeam() >= 1 && pCurrent->GetTeam() <= 2);

                // Show area under the bounds of enemy units that are visible
                if (pCurrent->GetTeam() != iTeam && (iTeam == TEAM_SPECTATOR || pCurrent->HasVisibilityFlags(VIS_VISION(iTeam))) && !bSpectate)
                {
                    float fSightRange(pCurrent->GetBounds().GetDim(X) * 0.5f);

                    if (fSightRange <= 0.0f)
                        continue;

                    float fX(pCurrent->GetPosition().x);
                    float fY(pCurrent->GetPosition().y);

                    int iX(INT_FLOOR(fX / m_fVisibilityScale));
                    int iY(INT_FLOOR(fY / m_fVisibilityScale));
                    int iRadius(INT_FLOOR(fSightRange / m_fVisibilityScale) + 1);

                    iRadius = MIN(iRadius, RASTER_BUFFER_SPAN / 2 - 1);

                    CRecti  recRegion(iX - iRadius, iY - iRadius, iX + iRadius + 1, iY + iRadius + 1);

                    // Un-occlude this unit's bounds
                    m_cVisRaster.Clear(0, recRegion.GetWidth() * recRegion.GetWidth());

                    m_cVisRaster.DrawFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iRadius);

                    CRecti  recBuffer(recRegion);

                    if (GetWorldPointer()->ClipRect(recRegion, TILE_SPACE))
                        m_cVisibilityMap.AddVision(recRegion, m_cVisRaster.GetBuffer() + recBuffer.GetWidth() * (recRegion.top - recBuffer.top) + (recRegion.left - recBuffer.left), recBuffer.GetWidth(), SIGHTED_BIT);

                    continue;
                }

                if ((pCurrent->GetStatus() != ENTITY_STATUS_ACTIVE && pCurrent->GetStatus() != ENTITY_STATUS_DEAD) ||
                    (pCurrent->GetTeam() != iTeam && !bSpectate))
                    continue;

                float fSightRange(pCurrentUnit->GetSightRange());

                if (fSightRange <= 0.0f)
                    continue;

                float fX(pCurrent->GetPosition().x);
                float fY(pCurrent->GetPosition().y);

                int iX(INT_FLOOR(fX / m_fVisibilityScale));
                int iY(INT_FLOOR(fY / m_fVisibilityScale));
                int iRadius(INT_FLOOR(fSightRange / m_fVisibilityScale));

                iRadius = MIN(iRadius, RASTER_BUFFER_SPAN / 2 - 1);

                CRecti  recRegion(iX - iRadius, iY - iRadius, iX + iRadius + 1, iY + iRadius + 1);

                if (pCurrentUnit->GetClearVision())
                {
                    m_cVisRaster.Clear(0, recRegion.GetWidth() * recRegion.GetWidth());
                    m_cVisRaster.DrawFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iRadius);
                }
                else
                {
                    // Get world occluders
                    float fHeight(pCurrentUnit->GetPosition().z + g_occlusionHeight);

                    GetWorldPointer()->GetOcclusion(recRegion, m_cOccRaster.GetBuffer(), fHeight);
                    
                    // Occlude sight range
                    m_cOccRaster.DrawOuterFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iRadius);
                    m_cVisRaster.CalcVisibilty2(recRegion.GetWidth(), m_cOccRaster.GetBuffer());
                }

#if 0
                // Set this unit's bounds as visible
                {
                    int iUnitRadius(INT_FLOOR((pCurrent->GetBounds().GetDim(X) * 0.5f) / m_fVisibilityScale) + 1);

                    iUnitRadius = MIN(iUnitRadius, RASTER_BUFFER_SPAN / 2 - 1);

                    m_cVisRaster.DrawFilledCircle(recRegion.GetWidth(), iRadius, iRadius, iUnitRadius);
                }
#endif

                CRecti  recBuffer(recRegion);

                if (GetWorldPointer()->ClipRect(recRegion, VIS_TILE_SPACE))
                    m_cVisibilityMap.AddVision(recRegion, m_cVisRaster.GetBuffer() + recBuffer.GetWidth() * (recRegion.top - recBuffer.top) + (recRegion.left - recBuffer.left), recBuffer.GetWidth(), SIGHTED_BIT);
            }
        }
        
        byte yDarkness(255 - BYTE_ROUND(cg_fogofwarDarkness * 255.0f));

        m_cVisibilityMap.FillBitmap(*m_pFogofWarBitmap, 255, yDarkness);

        byte *pBuffer(m_pFogofWarBitmap->GetBuffer());

        int iSize(m_pFogofWarBitmap->GetWidth());

        // Borders
        MemManager.Set(pBuffer, 0, iSize);
        MemManager.Set(pBuffer + iSize, 0, iSize);

        MemManager.Set(pBuffer + iSize * (iSize - 2), 0, iSize);
        MemManager.Set(pBuffer + iSize * (iSize - 1), 0, iSize);

        byte *p;

        p = pBuffer;
        for (int i(0); i < iSize; ++i, p += iSize)
            *p = 0;

        p = pBuffer + 1;
        for (int i(0); i < iSize; ++i, p += iSize)
            *p = 0;

        p = pBuffer + iSize - 2;
        for (int i(0); i < iSize; ++i, p += iSize)
            *p = 0;

        p = pBuffer + iSize - 1;
        for (int i(0); i < iSize; ++i, p += iSize)
            *p = 0;

        Vid.UpdateFogofWar(*m_pFogofWarBitmap);

        m_uiLastFogofWarUpdate = Game.GetGameTime() / FOW_UPDATE_TIME * FOW_UPDATE_TIME; // Round down to the nearest uiMS (acts like an accumulator)
    }

    Vid.RenderFogofWar(0.0f, true, CLAMP(float(Game.GetGameTime() - m_uiLastFogofWarUpdate) / FOW_UPDATE_TIME, 0.0f, 1.0f));
}


/*====================
  CGameClient::StartClientGameEffect
  ====================*/
int     CGameClient::StartClientGameEffect(ResHandle hEffect, int iChannel, int iTimeNudge, const CVec3f &v3Pos)
{
    if (hEffect == INVALID_RESOURCE)
        return -1;

    CEffect* pEffect(g_ResourceManager.GetEffect(hEffect));
    assert(pEffect != nullptr);
    if (pEffect == nullptr)
        return -1;

    // Search from an unused effect slot
    if (iChannel == -1)
    {
        for (int i(NUM_CLIENT_GAME_EFFECT_THREADS - 1); i >= NUM_CLIENT_GAME_EFFECT_CHANNELS; --i)
        {
            if (!m_apEffectThreads[i])
            {
                iChannel = i;
                break;
            }
        }

        if (iChannel == -1)
            return -1;
    }

    if (m_apEffectThreads[iChannel])
    {
        K2_DELETE(m_apEffectThreads[iChannel]);
        m_apEffectThreads[iChannel] = nullptr;
    }

    if (pEffect)
    {
        m_apEffectThreads[iChannel] = pEffect->SpawnThread(GameClient.GetGameTime() + iTimeNudge);

        m_apEffectThreads[iChannel]->SetCamera(GameClient.GetCamera());
        m_apEffectThreads[iChannel]->SetWorld(GameClient.GetWorldPointer());

        m_apEffectThreads[iChannel]->SetSourceSkeleton(nullptr);
        m_apEffectThreads[iChannel]->SetSourceModel(nullptr);
        m_apEffectThreads[iChannel]->SetTargetSkeleton(nullptr);
        m_apEffectThreads[iChannel]->SetTargetModel(nullptr);

        m_apEffectThreads[iChannel]->SetActive(true);

        m_apEffectThreads[iChannel]->SetSourcePos(v3Pos);
        m_apEffectThreads[iChannel]->SetSourceAxis(CAxis(0.0f, 0.0f, 0.0f));
        m_apEffectThreads[iChannel]->SetSourceScale(1.0f);
        
        if (m_apEffectThreads[iChannel]->Execute(GameClient.GetGameTime() + iTimeNudge))
        {
            // Effect finished, so delete it
            K2_DELETE(m_apEffectThreads[iChannel]);
            m_apEffectThreads[iChannel] = nullptr;
        }
    }

    return iChannel;
}


/*====================
  CGameClient::StopClientGameEffect
  ====================*/
void    CGameClient::StopClientGameEffect(int iChannel)
{
    if (iChannel < 0 || iChannel >= NUM_CLIENT_GAME_EFFECT_THREADS)
        return;
    
    if (m_apEffectThreads[iChannel])
    {
        K2_DELETE(m_apEffectThreads[iChannel]);
        m_apEffectThreads[iChannel] = nullptr;
    }
}


/*====================
  CGameClient::UpdateClientGameEffect

  Process global client game effect threads
  ====================*/
void    CGameClient::UpdateClientGameEffect(int iChannel, bool bActive, const CVec3f &v3Pos)
{
    if (!m_apEffectThreads[iChannel])
        return;

    m_apEffectThreads[iChannel]->SetActive(bActive);
    m_apEffectThreads[iChannel]->SetSourcePos(v3Pos);
    m_apEffectThreads[iChannel]->SetSourceAxis(CAxis(0.0f, 0.0f, 0.0f));
    m_apEffectThreads[iChannel]->SetSourceScale(1.0f);
}


/*====================
  CGameClient::AddClientGameEffects

  Process global client game effect threads
  ====================*/
void    CGameClient::AddClientGameEffects()
{
    for (int i(0); i < NUM_CLIENT_GAME_EFFECT_THREADS; ++i)
    {
        if (!m_apEffectThreads[i])
            continue;

        // Non-channels automatically follow the camera
        if (i >= NUM_CLIENT_GAME_EFFECT_CHANNELS)
        {
            m_apEffectThreads[i]->SetActive(true);
            m_apEffectThreads[i]->SetSourcePos(m_pCamera->GetOrigin());
            m_apEffectThreads[i]->SetSourceAxis(m_pCamera->GetViewAxis());
            m_apEffectThreads[i]->SetSourceScale(1.0f);
        }

        // Execute effect
        if (m_apEffectThreads[i]->Execute(GetGameTime()))
        {
            // Effect finished, so delete it
            K2_DELETE(m_apEffectThreads[i]);
            m_apEffectThreads[i] = nullptr;
        }
        else
        {
            // Camera movement
            GameClient.AddCameraEffectOffset(m_apEffectThreads[i]->GetCameraOffset());
            GameClient.AddCameraEffectAngleOffset(m_apEffectThreads[i]->GetCameraAngleOffset());

            // Update and render all particles systems associated with this effect thread
            const InstanceMap &mapInstances(m_apEffectThreads[i]->GetInstances());
            for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
            {
                IEffectInstance *pParticleSystem = it->second;

                pParticleSystem->Update(GameClient.GetGameTime());

                if (!pParticleSystem->IsDead() && pParticleSystem->IsParticleSystem())
                    SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
            }

            // Cleanup
            m_apEffectThreads[i]->Cleanup();
        }
    }
}


/*====================
  CGameClient::GetStateString
  ====================*/
CStateString&   CGameClient::GetStateString(uint uiID)
{
    return *m_pHostClient->GetStateString(uiID);
}


/*====================
  CGameClient::GetStateBlock
  ====================*/
CStateBlock&    CGameClient::GetStateBlock(uint uiID)
{
    return *m_pHostClient->GetStateBlock(uiID);
}


/*====================
  CGameClient::GetServerFrame
  ====================*/
uint    CGameClient::GetServerFrame()
{
    return m_pHostClient->GetServerFrame();
}


/*====================
  CGameClient::GetServerTime
  ====================*/
uint    CGameClient::GetServerTime() const
{
    return m_pHostClient->GetServerTime();
}


/*====================
  CGameClient::GetPrevServerTime
  ====================*/
uint    CGameClient::GetPrevServerTime()
{
    return m_pHostClient->GetPrevServerTime();
}


/*====================
  CGameClient::GetServerFrameLength
  ====================*/
uint    CGameClient::GetServerFrameLength()
{
    return SecToMs(1.0f / GetStateString(STATE_STRING_SERVER_INFO).GetInt(_CTS("svr_gameFPS")));
}


/*====================
  CGameClient::GetServerVersion
  ====================*/
tstring CGameClient::GetServerVersion()
{
    return GetStateString(STATE_STRING_SERVER_INFO).GetString(_CTS("svr_version"));
}


/*====================
  CGameClient::SendClientSnapshot
  ====================*/
void    CGameClient::SendClientSnapshot()
{
    // Save a snapshot of this frame
    if (m_vServerSnapshots[m_uiSnapshotBufferPos] == INVALID_POOL_HANDLE ||
        !CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos])->IsValid() ||
        CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos])->GetFrameNumber() != m_CurrentServerSnapshot.GetFrameNumber())
    {
        ++m_uiSnapshotBufferPos;
        if (m_uiSnapshotBufferPos >= m_vServerSnapshots.size())
            m_uiSnapshotBufferPos = 0;

        const CSnapshot *pSnapshot(m_vServerSnapshots[m_uiSnapshotBufferPos] != INVALID_POOL_HANDLE ? CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos]) : nullptr);

        if (pSnapshot && pSnapshot->IsValid() && pSnapshot->GetFrameNumber() >= m_pHostClient->GetLastAckedServerFrame())
        {
            //Console << _T("No more room in server snapshot buffer for frame ") << m_CurrentServerSnapshot.GetFrameNumber() << newl;

            if (m_hServerSnapshotFallback == INVALID_POOL_HANDLE)
            {
                //Console << _T("Saving fallback frame ") << m_vServerSnapshots[m_uiSnapshotBufferPos].GetFrameNumber() << newl;
                m_hServerSnapshotFallback = m_vServerSnapshots[m_uiSnapshotBufferPos];
                CSnapshot::AddRefToHandle(m_vServerSnapshots[m_uiSnapshotBufferPos]);
            }
        }

        SAFE_DELETE_SNAPSHOT(m_vServerSnapshots[m_uiSnapshotBufferPos]);
        m_vServerSnapshots[m_uiSnapshotBufferPos] = CSnapshot::Allocate(m_CurrentServerSnapshot);
    }

    const CSnapshot *pSnapshot(m_vServerSnapshots[m_uiSnapshotBufferPos] != INVALID_POOL_HANDLE ? CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos]) : nullptr);
    
    m_CurrentClientSnapshot.Update(GetGameTime(), GetFrameLength(), pSnapshot ? pSnapshot->GetFrameNumber() : 0);
    m_PendingClientSnapshot.Merge(m_CurrentClientSnapshot);
    
    if (m_pHostClient->IsReadyToSendSnapshot())
    {
        CBufferDynamic bufClientSnapshot;
        m_PendingClientSnapshot.GetUpdate(bufClientSnapshot);
        m_pHostClient->SendClientSnapshot(bufClientSnapshot);
        m_PendingClientSnapshot.Reset();
    }

    if (m_vServerSnapshots.size() != cg_serverSnapshotCacheSize)
        m_vServerSnapshots.resize(cg_serverSnapshotCacheSize, INVALID_POOL_HANDLE);
}


/*====================
  CGameClient::StopWorldSounds
  ====================*/
void    CGameClient::StopWorldSounds()
{
    for (SWorldSoundsMap_it it(m_mapWorldSounds.begin()); it != m_mapWorldSounds.end();)
    {
        if (it->second.hSound != INVALID_INDEX)
            K2SoundManager.StopHandle(it->second.hSound);

        STL_ERASE(m_mapWorldSounds, it);
    }
    if (m_hWorldAmbientSound != INVALID_INDEX)
        K2SoundManager.StopHandle(m_hWorldAmbientSound);

    m_mapWorldSounds.clear();
}


/*====================
  CGameClient::WorldSoundsFrame
  ====================*/
void    CGameClient::WorldSoundsFrame()
{
    float fVolumeMult(0.5f);
    for (SWorldSoundsMap_it it(m_mapWorldSounds.begin()); it != m_mapWorldSounds.end(); ++it)
    {
        if (it->second.hSound == INVALID_INDEX && it->second.uiNextStartTime > Host.GetTime())
            continue;

        CWorldSound *pSound(GameClient.GetWorldSound(it->first));

        if (it->second.hSound == INVALID_INDEX)
            it->second.hSound = K2SoundManager.PlayWorldSFXSound(it->second.hSample, &pSound->GetPosition(), fVolumeMult * pSound->GetVolume(), pSound->GetFalloff(), -1, 150, pSound->GetLoopDelay().Max() == 0 ? true : false);

        if (!K2SoundManager.UpdateHandle(it->second.hSound, pSound->GetPosition(), V3_ZERO))
        {
            it->second.hSound = INVALID_INDEX;
            it->second.uiNextStartTime = Host.GetTime() + pSound->GetLoopDelay();
        }
    }
}


/*====================
  CGameClient::RegisterModel
  ====================*/
ResHandle   CGameClient::RegisterModel(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        ResHandle hModel(g_ResourceManager.Register(sPath, RES_MODEL));

        if (hModel != INVALID_RESOURCE)
            g_ResourceManager.PrecacheSkin(hModel, uint(-1));

        return hModel;
    }
    assert(false);
    return INVALID_RESOURCE;
}


/*====================
  CGameClient::RegisterEffect
  ====================*/
ResHandle   CGameClient::RegisterEffect(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        return g_ResourceManager.Register(sPath, RES_EFFECT);

    assert(false);
    return INVALID_RESOURCE;
}


/*====================
  CGameClient::RegisterIcon
  ====================*/
ResHandle   CGameClient::RegisterIcon(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        return g_ResourceManager.Register(K2_NEW(ctx_GameClient,   CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    assert(false);
    return INVALID_RESOURCE;
}


/*====================
  CGameClient::RegisterSample
  ====================*/
ResHandle   CGameClient::RegisterSample(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        return g_ResourceManager.Register(sPath, RES_SAMPLE);

    assert(false);
    return INVALID_RESOURCE;
}


/*====================
  CGameClient::RegisterMaterial
  ====================*/
ResHandle   CGameClient::RegisterMaterial(const tstring &sPath)
{
    if (sPath.empty())
        return INVALID_RESOURCE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        return g_ResourceManager.Register(sPath, RES_MATERIAL);

    assert(false);
    return INVALID_RESOURCE;
}


/*====================
  CGameClient::RenderSelectedPlayerView
  ====================*/
CVAR_BOOL(pip_show, false);
CVAR_FLOAT(pip_x, 860.f);
CVAR_FLOAT(pip_y, 380.f);
CVAR_FLOAT(pip_w, 160.f);
CVAR_FLOAT(pip_h, 120.f);
CVAR_FLOAT(pip_fov, 90.f);
CVAR_BOOL(pip_repopulate, false);
void    CGameClient::RenderSelectedPlayerView(uint uiPlayerIndex)
{
    PROFILE("CGameClient::RenderSelectedPlayerView");

    if (!pip_show)
        return;

    if (m_pClientCommander == nullptr)
        return;

    IUnitEntity *pUnit(m_pClientCommander->GetSelectedControlEntity());
    if (pUnit == nullptr)
        return;

    CCamera camPip(*m_pCamera);

    // Prepare the scene
    if (pip_repopulate)
    {
        SceneManager.Clear();
        m_pClientEntityDirectory->PopulateScene();
    }
    //SceneManager.ClearBackground();

    camPip.SetOrigin(pUnit->GetPosition() + V_UP * pUnit->GetBoundsHeight());
    camPip.SetAngles(pUnit->GetAngles());
    camPip.SetTime(MsToSec(GetGameTime()));
    camPip.SetX(pip_x);
    camPip.SetY(pip_y);
    camPip.SetWidth(pip_w);
    camPip.SetHeight(pip_h);
    camPip.SetFovXCalc(pip_fov);
    camPip.RemoveFlags(CAM_FOG_OF_WAR | CAM_NO_FOG | CAM_SHADOW_UNIFORM | CAM_SHADOW_NO_FALLOFF);
    camPip.SetShadowBias(0.0f);

    SceneManager.PrepCamera(camPip);
    SceneManager.DrawSky(camPip, MsToSec(m_pHostClient->GetClientFrameLength()));
    SceneManager.Render();
}


/*====================
  CGameClient::DrawSelectedStats
  ====================*/
void    CGameClient::DrawSelectedStats()
{
    if (!cg_drawSelectedStats)
        return;

    IUnitEntity *pUnit(m_pClientCommander->GetSelectedInfoEntity());
    if (pUnit == nullptr)
    {
        pUnit = m_pClientCommander->GetSelectedControlEntity();
        if (pUnit == nullptr)
            return;
    }

    ResHandle hFont(g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
    if (pFontMap == nullptr)
        return;
    
    const float fLines(28.0f);

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 29;
    const float PANEL_HEIGHT = fLines;
    const float START_X = INT_FLOOR(FONT_WIDTH);
    const float START_Y = INT_FLOOR(Draw2D.GetScreenH() - (FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2) - Draw2D.GetScreenH() * 0.333f);
    const int   NAME_LENGTH = 14;

    float fDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - FONT_WIDTH, START_Y - FONT_WIDTH, FONT_WIDTH * PANEL_WIDTH + FONT_WIDTH * 2, FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2);

    Draw2D.SetColor(WHITE);

    Draw2D.String(START_X, fDrawY, XtoA(_T("Entity")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetTypeName()), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Index")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetIndex()), hFont);
    fDrawY += FONT_HEIGHT;
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("X")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetPosition().x), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Y")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetPosition().y), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Z")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetPosition().z), hFont);
    fDrawY += FONT_HEIGHT;
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Level")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetLevel()), hFont);
    fDrawY += FONT_HEIGHT;
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Health")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetHealth()), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Max Health")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetMaxHealth()), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Health Regen")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetHealthRegen()), hFont);
    fDrawY += FONT_HEIGHT;
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Mana")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetMana()), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Max Mana")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetMaxMana()), hFont);
    fDrawY += FONT_HEIGHT;
    Draw2D.String(START_X, fDrawY, XtoA(_T("Mana Regen")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetManaRegen()), hFont);
    fDrawY += FONT_HEIGHT;
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Armor")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetArmor()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Physical Res")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(Game.GetArmorDamageAdjustment(pUnit->GetArmorType(), pUnit->GetArmor())), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Magic Armor")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetMagicArmor()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Magic Res")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(Game.GetArmorDamageAdjustment(pUnit->GetMagicArmorType(), pUnit->GetMagicArmor())), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Move Speed")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetMoveSpeed()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Attack Speed")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetAttackSpeed() * 100.0f), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Attack Time")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(MsToSec(pUnit->GetAdjustedAttackCooldown())), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Lifetime")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(MsToSec(pUnit->GetRemainingLifetime())), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Stealth")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->IsStealth()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Life Steal")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetLifeSteal()), hFont);
    fDrawY += FONT_HEIGHT;
    
    Draw2D.String(START_X, fDrawY, XtoA(_T("Sight Range")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pUnit->GetSightRange()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*====================
  CGameClient::SetReplayClient
  ====================*/
void    CGameClient::SetReplayClient(int iClientNum)
{
    if (!ReplayManager.IsPlaying())
        return;

    if (iClientNum == -1)
    {
        m_pLocalPlayer = m_pReplaySpectator;
    }
    else
    {
        PlayerMap_it itClient(m_mapClients.find(iClientNum));
        if (itClient == m_mapClients.end())
            return;

        m_pLocalPlayer = itClient->second;
    }

    m_pHostClient->SetClientNum(iClientNum);

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_SET_REPLAY_CLIENT << iClientNum;
    SendGameData(buffer, true);

    m_pClientCommander->SetPlayer(m_pLocalPlayer);

    replay_client = iClientNum;
}


/*====================
  CGameClient::NextReplayClient
  ====================*/
void    CGameClient::NextReplayClient()
{
    if (!ReplayManager.IsPlaying() && m_pLocalPlayer)
        return;

    PlayerMap_it itClient(m_mapClients.find(m_pLocalPlayer->GetClientNumber()));
    if (itClient == m_mapClients.end())
        return;

    PlayerMap_it itStartClient(itClient);
    
    do
    {
        ++itClient;
        if (itClient == m_mapClients.end())
            itClient = m_mapClients.begin();
    } while (itClient != itStartClient && itClient->second->IsDisconnected());

    SetReplayClient(itClient->first);
}


/*====================
  CGameClient::PrevReplayClient
  ====================*/
void    CGameClient::PrevReplayClient()
{
    if (!ReplayManager.IsPlaying() && m_pLocalPlayer)
        return;

    PlayerMap_it itClient(m_mapClients.find(m_pLocalPlayer->GetClientNumber()));
    if (itClient == m_mapClients.end())
        return;

    PlayerMap_it itStartClient(itClient);
    
    do
    {
        if (itClient == m_mapClients.begin())
            itClient = m_mapClients.end();
        --itClient;
    } while (itClient != itStartClient && itClient->second->IsDisconnected());

    SetReplayClient(itClient->first);
}


/*====================
  CGameClient::UpdateTeamRosters
  ====================*/
void    CGameClient::UpdateTeamRosters()
{
    // Clear old client lists
    for (int iTeam(0); iTeam <= 2; ++iTeam)
    {
        CTeamInfo *pTeam(GameClient.GetTeam(iTeam));
        if (pTeam == nullptr)
            continue;

        ivector &vClients(pTeam->GetClientList());
        vClients.clear();
    }

    for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CPlayer *pClient(it->second);

        CTeamInfo *pTeam(GameClient.GetTeam(pClient->GetTeam()));
        if (pTeam == nullptr)
            continue;

        if (pClient->GetTeamIndex() < 0)
            continue;

        ivector &vClients(pTeam->GetClientList());

        if (size_t(pClient->GetTeamIndex()) >= vClients.size())
            vClients.resize(pClient->GetTeamIndex() + 1, -1);

        vClients[pClient->GetTeamIndex()] = it->first;
    }
}


/*====================
  CGameClient::GetPropType
  ====================*/
const tstring&  CGameClient::GetPropType(const tstring &sPath) const
{
    CStringTable *StringTable(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_PROP_TYPE_TABLE]));

    if (!StringTable)
    {
        //Console.Warn << _T("Prop type StringTable not found") << newl;
        return TSNULL;
    }
    
    const tstring &sType(StringTable->Get(sPath));
    if (sType.compare(sPath))
        return sType;
    else
        return TSNULL;
}


/*====================
  CGameClient::Connect
  ====================*/
void    CGameClient::Connect(const tstring &sAddr)
{
    m_CurrentServerSnapshot.SetValid(false);

    SAFE_DELETE_SNAPSHOT(m_hServerSnapshotFallback);

    if (m_vServerSnapshots.size() != cg_serverSnapshotCacheSize)
        m_vServerSnapshots.resize(cg_serverSnapshotCacheSize, INVALID_POOL_HANDLE);

    for (vector<PoolHandle>::iterator it(m_vServerSnapshots.begin()); it != m_vServerSnapshots.end(); ++it)
        SAFE_DELETE_SNAPSHOT(*it);

    m_uiSnapshotBufferPos = 0;
    m_bStartedLoadingResources = false;
    m_bStartedLoadingEntityResources = false;
    m_bStartedSpawningEntities = false;
    m_bStartedLoadingHeroes = false;
    m_bWasUsingCustomFiles = false;
    m_bWereCoreFilesModified = false;

    m_vHeroes[0].clear();
    m_vHeroes[1].clear();
}


/*====================
  CGameClient::GetTerrainType
  ====================*/
const tstring&  CGameClient::GetTerrainType()
{
    static tstring s_sGrass(_T("grass"));

    switch (m_eEventTarget)
    {
    case CG_EVENT_TARGET_ENTITY:
        return m_pCurrentEntity->GetCurrentEntity()->GetTerrainType();
    }

    return s_sGrass;
}


/*====================
  CGameClient::GetConnectedClientCount
  ====================*/
int     CGameClient::GetConnectedClientCount(int iTeam)
{
    if (iTeam == -1)
    {
        int iNumClients(0);

        for (PlayerMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
            if (!it->second->IsDisconnected())
                iNumClients++;

        return iNumClients;
    }

    CTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == nullptr)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameClient::RemoveClient
  ====================*/
void    CGameClient::RemoveClient(int iClientNum)
{
    if (m_pLocalPlayer && m_pLocalPlayer->GetClientNumber() == iClientNum)
        m_pLocalPlayer = nullptr;

    m_mapClients.erase(iClientNum);
    UpdateTeamRosters();
}


/*====================
  CGameClient::PrimaryAction
  ====================*/
void    CGameClient::PrimaryAction(int iSlot)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
    if (!pUnit)
        return;

    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return;

    IEntityItem *pItem(pUnit->GetItem(iSlot));
    if (pItem != nullptr)
    {
        if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        {
            ushort unShop(GetShop(pItem->GetTypeName()));
            CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
            if (pShop != nullptr)
            {
                if (m_pInterfaceManager->IsShopVisible() &&
                    GetActiveShop() == EntityRegistry.LookupName(unShop) &&
                    GetActiveRecipe() == pItem->GetTypeName())
                {
                    m_pInterfaceManager->SetShopVisible(false);
                    //SetActiveShop(TSNULL);
                }
                else
                {
                    m_pInterfaceManager->SetShopVisible(true);
                    SetActiveShop(EntityRegistry.LookupName(unShop));
                    SetActiveRecipe(pItem->GetTypeName(), false);
                }
            }
        }
    }
}


/*====================
  CGameClient::SecondaryAction
  ====================*/
void    CGameClient::SecondaryAction(int iSlot)
{
    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return;

    IUnitEntity *pUnit(GetUnitEntity(m_pClientCommander->GetSelectedControlEntityIndex()));
    if (!pUnit)
        return;

    IEntityItem *pItem(pUnit->GetItem(iSlot));
    if (pItem == nullptr || !pItem->CanDrop() || pItem->HasFlag(ENTITY_TOOL_FLAG_LOCKED))
        return;

    if (Input.IsCtrlDown() && !Input.IsShiftDown() && pItem->GetNoStash())
        return;

    m_uiItemCursorIndex = pItem->GetIndex();

    if (Input.IsCtrlDown())
    {
        if (Input.IsShiftDown())
            ItemSell(iSlot);
        else
            ItemPlaceStash(-1);

        m_uiItemCursorIndex = INVALID_INDEX;
    }

    if (m_uiItemCursorIndex != INVALID_INDEX)
        K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_SELECT_ITEM_SAMPLE]);
}


/*====================
  CGameClient::ItemPlace
  ====================*/
void    CGameClient::ItemPlace(int iSlot)
{
    if (m_uiItemCursorIndex == INVALID_INDEX)
        return;

    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
    if (!pUnit)
        return;

    if (iSlot < INVENTORY_START_BACKPACK || iSlot > INVENTORY_END_BACKPACK)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (!pEntity || !pEntity->IsItem())
        return;

    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem->GetOwner() != pUnit &&
        !(pItem->GetSlot() >= INVENTORY_START_STASH && pItem->GetSlot() <= INVENTORY_END_STASH && pItem->GetOwner() == m_pLocalPlayer->GetHero()))
        return;

    IEntityItem *pBlockingItem(pUnit->GetItem(iSlot));

    bool bFromStash(pItem->GetSlot() >= INVENTORY_START_STASH && pItem->GetSlot() <= INVENTORY_END_STASH);
    bool bToStash(iSlot >= INVENTORY_START_STASH && iSlot <= INVENTORY_END_STASH);

    if (pBlockingItem != nullptr)
    {
        if (bToStash && pBlockingItem->GetNoStash())
            return;

        if (!pBlockingItem->CanDrop() && bFromStash != bToStash)
            return;
        if (pBlockingItem->HasFlag(ENTITY_TOOL_FLAG_LOCKED))
            return;
    }

    CBufferFixed<13> buffer;
    buffer << GAME_CMD_MOVE_ITEM << uiUnitIndex << int(pItem->GetSlot()) << int(iSlot);
    GameClient.SendGameData(buffer, true);

    m_uiItemCursorIndex = INVALID_INDEX;

    K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_PLACE_ITEM_SAMPLE]);
}


/*====================
  CGameClient::PrimaryActionStash
  ====================*/
void    CGameClient::PrimaryActionStash(int iSlot)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
    if (pUnit == nullptr)
        return;

    IUnitEntity *pStash(m_pLocalPlayer->GetHero());
    if (pStash == nullptr)
        return;

    if (iSlot < 0 || iSlot > INVENTORY_STASH_SIZE)
        return;

    IEntityItem *pItem(pStash->GetItem(iSlot + INVENTORY_START_STASH));
    if (pItem != nullptr)
    {
        if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        {
            ushort unShop(GetShop(pItem->GetTypeName()));
            CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
            if (pShop != nullptr)
            {
                if (m_pInterfaceManager->IsShopVisible() &&
                    GetActiveShop() == EntityRegistry.LookupName(unShop) &&
                    GetActiveRecipe() == pItem->GetTypeName())
                {
                    m_pInterfaceManager->SetShopVisible(false);
                    //SetActiveShop(TSNULL);
                }
                else
                {
                    m_pInterfaceManager->SetShopVisible(true);
                    SetActiveShop(EntityRegistry.LookupName(unShop));
                    SetActiveRecipe(pItem->GetTypeName(), false);
                }
            }
        }
        else
        {
            ItemDisassemble(pItem->GetSlot());
        }
    }
}


/*====================
  CGameClient::SecondaryActionStash
  ====================*/
void    CGameClient::SecondaryActionStash(int iSlot)
{
    if (m_pClientCommander == nullptr)
        return;

    IUnitEntity *pUnit(GetUnitEntity(m_pClientCommander->GetSelectedControlEntityIndex()));
    if (pUnit == nullptr)
        return;

    IUnitEntity *pStash(m_pLocalPlayer->GetHero());
    if (pStash == nullptr)
        return;

    if (iSlot < 0 || iSlot > INVENTORY_STASH_SIZE)
        return;
    
    IEntityItem *pItem(pStash->GetItem(INVENTORY_START_STASH + iSlot));
    if (pItem == nullptr)
        return;

    if (pUnit->GetStashAccess())
        m_uiItemCursorIndex = pItem->GetIndex();

    if (Input.IsCtrlDown())
    {
        if (Input.IsShiftDown())
            ItemSell(iSlot + INVENTORY_START_STASH);
        else
            ItemTakeFromStash(-1);

        m_uiItemCursorIndex = INVALID_INDEX;
    }
    
    if (m_uiItemCursorIndex != INVALID_INDEX)
        K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_SELECT_ITEM_SAMPLE]);
}


/*====================
  CGameClient::ItemPlaceStash
  ====================*/
void    CGameClient::ItemPlaceStash(int iSlot)
{
    if (m_pClientCommander == nullptr)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (pEntity == nullptr)
        return;
    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem == nullptr)
        return;

    IUnitEntity *pUnit(GetUnitEntity(m_pClientCommander->GetSelectedControlEntityIndex()));
    if (pUnit == nullptr || (!pUnit->GetStashAccess() && pUnit->GetShopAccess().empty()))
        return;

    IUnitEntity *pStash(m_pLocalPlayer->GetHero());
    if (pStash == nullptr)
        return;

    if (pItem->GetOwner() != pUnit && !(IS_STASH_SLOT(pItem->GetSlot()) && pItem->GetOwner() == pStash))
        return;

    if (pItem->GetNoStash())
        return;

    // Find a free slot
    if (iSlot == -1)
    {
        for (int iTry(INVENTORY_START_STASH); iTry <= INVENTORY_STASH_PROVISIONAL; ++iTry)
        {
            if (pStash->GetItem(iTry) == nullptr)
            {
                iSlot = iTry - INVENTORY_START_STASH;
                break;
            }
        }
    }

    if (iSlot < 0 || iSlot > INVENTORY_STASH_SIZE)
        return;

    CBufferFixed<13> buffer;
    buffer << GAME_CMD_MOVE_ITEM << pUnit->GetIndex() << int(pItem->GetSlot()) << int(iSlot + INVENTORY_START_STASH);
    GameClient.SendGameData(buffer, true);

    m_uiItemCursorIndex = INVALID_INDEX;

    K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_PLACE_ITEM_SAMPLE]);
}


/*====================
  CGameClient::ItemTakeFromStash
  ====================*/
void    CGameClient::ItemTakeFromStash(int iSlot)
{
    if (m_pClientCommander == nullptr)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (pEntity == nullptr)
        return;
    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem == nullptr)
        return;

    if (pItem->GetSlot() < INVENTORY_START_STASH || pItem->GetSlot() > INVENTORY_END_STASH)
        return;

    IUnitEntity *pStash(m_pLocalPlayer->GetHero());
    if (pStash == nullptr)
        return;

    if (pItem->GetOwner() != pStash)
        return;

    IUnitEntity *pUnit(GetUnitEntity(m_pClientCommander->GetSelectedControlEntityIndex()));
    if (pUnit == nullptr || !pUnit->GetStashAccess() || !pUnit->GetCanCarryItems())
        return;

    // Find a free slot
    if (iSlot == -1)
    {
        for (int iTry(INVENTORY_START_BACKPACK); iTry <= INVENTORY_BACKPACK_PROVISIONAL; ++iTry)
        {
            if (pUnit->GetItem(iTry) == nullptr)
            {
                iSlot = iTry - INVENTORY_START_BACKPACK;
                break;
            }
        }
    }

    if (iSlot < 0 || iSlot > INVENTORY_BACKPACK_SIZE)
        return;

    CBufferFixed<13> buffer;
    buffer << GAME_CMD_MOVE_ITEM << pUnit->GetIndex() << int(pItem->GetSlot()) << int(iSlot + INVENTORY_START_BACKPACK);
    GameClient.SendGameData(buffer, true);

    m_uiItemCursorIndex = INVALID_INDEX;

    K2SoundManager.Play2DSound(m_ahResources[CLIENT_RESOURCE_PLACE_ITEM_SAMPLE]);
}


/*====================
  CGameClient::ItemPlaceHero
  ====================*/
void    CGameClient::ItemPlaceHero()
{
    if (m_pClientCommander == nullptr)
        return;

    if (m_uiItemCursorIndex == INVALID_INDEX)
        return;

    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
    if (!pUnit)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (!pEntity || !pEntity->IsItem())
        return;

    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem->GetOwner() != pUnit)
        return;

    IHeroEntity *pHero(m_pLocalPlayer->GetHero());
    if (pHero != nullptr && pHero->GetCanCarryItems())
    {
        m_pClientCommander->GiveOrder(CMDR_ORDER_GIVEITEM, pHero->GetIndex(), Input.IsButtonDown(BUTTON_SHIFT) ? Input.IsButtonDown(BUTTON_CTRL) ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, m_uiItemCursorIndex);
        m_uiItemCursorIndex = INVALID_INDEX;
    }
}


/*====================
  CGameClient::ItemPlaceEntity
  ====================*/
void    CGameClient::ItemPlaceEntity(uint uiIndex)
{
    if (m_pClientCommander == nullptr)
        return;

    if (m_uiItemCursorIndex == INVALID_INDEX)
        return;

    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    IUnitEntity *pUnit(GetUnitEntity(uiUnitIndex));
    if (!pUnit)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (!pEntity || !pEntity->IsItem())
        return;

    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem->GetOwner() != pUnit)
        return;

    IUnitEntity *pTargetUnit(Game.GetUnitEntity(uiIndex));
    if (pTargetUnit != nullptr && pTargetUnit->GetCanCarryItems())
    {
        m_pClientCommander->GiveOrder(CMDR_ORDER_GIVEITEM, pTargetUnit->GetIndex(), Input.IsButtonDown(BUTTON_SHIFT) ? Input.IsButtonDown(BUTTON_CTRL) ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, m_uiItemCursorIndex);
        m_uiItemCursorIndex = INVALID_INDEX;
    }
}


/*====================
  CGameClient::ItemPlaceSelected
  ====================*/
void    CGameClient::ItemPlaceSelected(int iSlot)
{
    if (m_pClientCommander == nullptr)
        return;

    IGameEntity *pEntity(GameClient.GetEntity(m_uiItemCursorIndex));
    if (pEntity == nullptr)
        return;
    IEntityItem *pItem(pEntity->GetAsItem());
    if (pItem == nullptr)
        return;

    IUnitEntity *pUnit(GetUnitEntity(m_pClientCommander->GetSelectedControlEntityIndex()));
    if (pUnit == nullptr)
        return;

    if (pItem->GetOwner() != pUnit)
        return;

    const uiset &setSelection(m_pClientCommander->GetSelectedControlEntities().size() > 1 ? m_pClientCommander->GetSelectedControlEntities() : m_pClientCommander->GetSelectedInfoEntities());

    uiset_cit cit(setSelection.begin());
    for (; cit != setSelection.end() && iSlot > 0; ++cit, --iSlot) {}

    if (cit != setSelection.end())
    {
        IUnitEntity *pSelectedUnit(GetUnitEntity(*cit));
        if (pSelectedUnit != nullptr && pSelectedUnit->GetCanCarryItems())
        {
            m_pClientCommander->GiveOrder(CMDR_ORDER_GIVEITEM, pSelectedUnit->GetIndex(), Input.IsButtonDown(BUTTON_SHIFT) ? Input.IsButtonDown(BUTTON_CTRL) ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, m_uiItemCursorIndex);
            m_uiItemCursorIndex = INVALID_INDEX;
        }
    }
}


/*====================
  CGameClient::ItemSell
  ====================*/
void    CGameClient::ItemSell(int iSlot)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    uint uiUnitIndex(pCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    if (iSlot == -1)
    {
        IGameEntity *pEntity(GetEntity(GameClient.GetItemCursorIndex()));
        if (pEntity != nullptr && pEntity->IsItem())
            iSlot = pEntity->GetAsItem()->GetSlot();

        SetItemCursorIndex(INVALID_INDEX);
    }
    
    CBufferFixed<9> buffer;
    buffer << GAME_CMD_SELL << uiUnitIndex << iSlot;
    SendGameData(buffer, true);

    m_uiItemCursorIndex = INVALID_INDEX;
}


/*====================
  CGameClient::ItemDisassemble
  ====================*/
void    CGameClient::ItemDisassemble(int iSlot)
{
    CClientCommander *pCommander(GameClient.GetClientCommander());
    if (pCommander == nullptr)
        return;

    uint uiUnitIndex(pCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return;

    if (!IS_ITEM_SLOT(iSlot))
        return;

    CBufferFixed<9> buffer;
    buffer << GAME_CMD_DISASSEMBLE << uiUnitIndex << iSlot;
    SendGameData(buffer, true);
}


/*====================
  CGameClient::StateStringChanged
  ====================*/
void    CGameClient::StateStringChanged(uint uiID, const CStateString &ss)
{
    if (uiID == STATE_STRING_ENTITIES)
        NetworkResourceManager.ApplyUpdateFromStateString(ss);
}


/*====================
  CGameClient::StateBlockChanged
  ====================*/
void    CGameClient::StateBlockChanged(uint uiID, const CStateBlock &block)
{
    if (uiID >= STATE_BLOCK_HERO_GROUP0 && uiID <= STATE_BLOCK_HERO_GROUP9)
    {
        const IBuffer &bufferHeroes(block.GetBuffer());
        bufferHeroes.Rewind();
        
        uint uiNumHeroes(bufferHeroes.GetLength() / 5);
        
        HeroList &vHeroes(m_vHeroes[uiID - STATE_BLOCK_HERO_GROUP0]);
        vHeroes.resize(uiNumHeroes);

        for (uint ui(0); ui < uiNumHeroes; ++ui)
        {
            ushort unIndex(bufferHeroes.ReadShort());
            ushort unTypeID(bufferHeroes.ReadShort());
            byte yStatus(bufferHeroes.ReadByte());

            if (vHeroes.size() <= unIndex)
                vHeroes.resize(unIndex + 1);
            vHeroes[unIndex] = HeroListEntry(unTypeID, yStatus);
        }
    }
    else if (uiID == STATE_BLOCK_BIT)
    {
        if (IsWorldLoaded() && !IsWorldPreloaded())
        {
            m_pClientEntityDirectory->ClearBitEntities();

            const IBuffer &cBitEntityMap(m_pHostClient->GetStateBlock(STATE_BLOCK_BIT)->GetBuffer());
            cBitEntityMap.Rewind();
            uint uiNumBitEntities(cBitEntityMap.GetLength() / 4);
            for (uint ui(0); ui < uiNumBitEntities; ++ui)
            {
                uint uiGameIndex(cBitEntityMap.ReadShort());
                uint uiWorldIndex(cBitEntityMap.ReadShort());

                CWorldEntity *pWorldEntity(GetWorldEntity(uiWorldIndex));
                if (pWorldEntity == nullptr)
                    continue;

                IGameEntity *pEntity(m_pClientEntityDirectory->Allocate(uiGameIndex, pWorldEntity->GetType()));
                if (pEntity == nullptr)
                    continue;

                pWorldEntity->SetGameIndex(uiGameIndex);

                pEntity->ApplyWorldEntity(*pWorldEntity);
                pEntity->Spawn();
                pEntity->Validate();

                IBitEntity *pBit(pEntity->GetAsBit());
                if (pBit != nullptr)
                {
                    pBit->SetBitIndex(ui);
                    m_pClientEntityDirectory->AddBitEntity(pEntity->GetAsBit());
                }
            }
        }
    }
    else if (uiID == STATE_BLOCK_ENTITY_TYPES)
    {
#if 0
        const IBuffer &cDynamicEntities(block.GetBuffer());
        cDynamicEntities.Rewind();
        uint uiNumDynamicEntities(cDynamicEntities.GetLength() / 6);
        for (uint ui(0); ui < uiNumDynamicEntities; ++ui)
        {
            ushort unTypeID(cDynamicEntities.ReadShort());
            uint uiNetResIndex(cDynamicEntities.ReadInt());

            EntityRegistry.RegisterDynamicEntity(unTypeID, NetworkResourceManager.GetLocalHandle(uiNetResIndex));
        }
#endif
    }
}

#include "../k2/c_hostserver.h" // You don't see anything here, move along

/*====================
  CGameClient::DrawNavGrid
  ====================*/
void    CGameClient::DrawNavGrid()
{
    if (!cg_drawNavGrid || !Host.HasServer())
        return;

    CWorld *pWorld(Host.GetServer()->GetWorld());
    ResHandle hLineMaterial(g_ResourceManager.Register(_T("/core/materials/line.material"), RES_MATERIAL));

    CNavigationMap &cNavigationMap(pWorld->GetNavigationMap());

    uint uiNavFlags(NAVIGATION_ALL);
    uiNavFlags &= ~NAVIGATION_ANTI;

    CNavGridZ *pNavGrid(cNavigationMap.PrepForSearch(uiNavFlags, cg_drawNavGridDownsize));

    uint *pHorizontal(pNavGrid->GetHorizontal());
    uint *pVertical(pNavGrid->GetVertical());

    uint uiNavigationWidth(pNavGrid->GetWidth());
    uint uiNavigationHeight(pNavGrid->GetHeight());
    uint uiIntsPerRow(pNavGrid->GetIntsPerRow());

    STraceInfo trace;

    if (!TraceCursor(trace, TRACE_TERRAIN))
        return;

    float fGateScale(pWorld->GetNavigationScale());
    float fTileScale(float(1 << pWorld->GetNavigationSize() >> pWorld->GetSize() << pNavGrid->GetDownSize()) * pWorld->GetScale());
    int iDisplaySize(cg_pathDisplaySize);
    int iDisplaySize2(int(cg_pathDisplaySize) << pNavGrid->GetDownSize());

    uint uiHorizontalStartX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fGateScale) - iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiHorizontalStartY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fTileScale) - iDisplaySize, 0, int(uiNavigationHeight)));
    uint uiHorizontalEndX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fGateScale) + iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiHorizontalEndY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fTileScale) + iDisplaySize, 0, int(uiNavigationHeight) - 1));

    SSceneFaceVert poly[1024];
    MemManager.Set(poly, 0, sizeof(poly));
    int p(0);

    // Horizontal gates
    for (uint uiY(uiHorizontalStartY); uiY != uiHorizontalEndY; ++uiY)
    {
        float fY((uiY + 1) * fTileScale);

        for (uint uiX(uiHorizontalStartX); uiX != uiHorizontalEndX; ++uiX)
        {
            // Restart batch if we overflow
            if (p >= 1024)
            {
                SceneManager.AddPoly(p, poly, hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                MemManager.Set(poly, 0, sizeof(poly));
                p = 0;
            }

            float fX0((uiX) * fGateScale);
            float fX1((uiX + 1) * fGateScale);
            
            float fZ0(pWorld->GetTerrainHeight(fX0, fY));
            float fZ1(uiX + 1 != uiNavigationWidth ? pWorld->GetTerrainHeight(fX1, fY) : pWorld->GetTerrainHeight(fX1 - 0.001f, fY));

            if (pHorizontal[uiY * uiIntsPerRow + (uiX >> 5)] & BIT(31 - (uiX & 31)))
            {
                poly[p].vtx[0] = fX0;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;

                poly[p].vtx[0] = fX1;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;
            }
            else
            {
                poly[p].vtx[0] = fX0;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;

                poly[p].vtx[0] = fX1;
                poly[p].vtx[1] = fY;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;
            }
        }
    }

    uint uiVerticalStartX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fTileScale) - iDisplaySize, 0, int(uiNavigationWidth)));
    uint uiVerticalStartY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fGateScale) - iDisplaySize2, 0, int(uiIntsPerRow * 32)));
    uint uiVerticalEndX(CLAMP(INT_ROUND(trace.v3EndPos[X] / fTileScale) + iDisplaySize, 0, int(uiNavigationWidth) - 1));
    uint uiVerticalEndY(CLAMP(INT_ROUND(trace.v3EndPos[Y] / fGateScale) + iDisplaySize2, 0, int(uiIntsPerRow * 32)));

    // Vertical gates
    for (uint uiX(uiVerticalStartX); uiX != uiVerticalEndX; ++uiX)
    {
        float fX((uiX + 1) * fTileScale);

        for (uint uiY(uiVerticalStartY); uiY != uiVerticalEndY; ++uiY)
        {
            // Restart batch if we overflow
            if (p >= 1024)
            {
                SceneManager.AddPoly(p, poly, hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
                MemManager.Set(poly, 0, sizeof(poly));
                p = 0;
            }

            float fY0((uiY) * fGateScale);
            float fY1((uiY + 1) * fGateScale);
            
            float fZ0(pWorld->GetTerrainHeight(fX, fY0));
            float fZ1(uiY + 1 != uiNavigationWidth ? pWorld->GetTerrainHeight(fX, fY1) : pWorld->GetTerrainHeight(fX, fY1 - 0.001f));

            if (pVertical[uiX * uiIntsPerRow + (uiY >> 5)] & BIT(31 - (uiY & 31)))
            {
                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY0;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;

                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY1;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 255, 255, 255);
                ++p;
            }
            else
            {
                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY0;
                poly[p].vtx[2] = fZ0;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;

                poly[p].vtx[0] = fX;
                poly[p].vtx[1] = fY1;
                poly[p].vtx[2] = fZ1;
                SET_VEC4(poly[p].col, 255, 0, 0, 255);
                ++p;
            }
        }
    }

    // Draw remaining polys
    if (p > 0)
    {
        SceneManager.AddPoly(p, poly, hLineMaterial, POLY_LINELIST | POLY_NO_DEPTH_TEST);
        p = 0;
    }
}


/*====================
  CGameClient::DrawSelectedPath

  Hax to directly access some server data for debugging purposes
  ====================*/
void    CGameClient::DrawSelectedPath()
{
    if (!cg_drawSelectedPath)
        return;

    IUnitEntity *pUnitClient(m_pClientCommander->GetSelectedInfoEntity());
    if (pUnitClient == nullptr)
    {
        pUnitClient = m_pClientCommander->GetSelectedControlEntity();
        if (pUnitClient == nullptr)
            return;
    }

    if (Host.GetServer() == nullptr)
        return;

    IUnitEntity *pUnitServer(static_cast<IUnitEntity *>(Host.GetServer()->GetEntity(pUnitClient->GetIndex())));

    CWorld *pWorld(Host.GetServer()->GetWorld());

    PoolHandle hPath(pUnitServer->GetActivePath());

    CPath *pPath(pWorld->AccessPath(hPath));

    // Unsmoothed path
    if (pPath != nullptr)
    {
        PathResult &vecInfoPath(pPath->GetSimpleResult());
        uint uiCount(uint(vecInfoPath.size()));

        for (uint i(0); i + 1 < uiCount; ++i)
        {
            CVec2f v2Src(vecInfoPath[i].GetPath());
            CVec2f v2End(vecInfoPath[i + 1].GetPath());

#if 1 // SHOW_GATES
            CVec2f v2PosGate, v2NegGate;
            CVec2f v2Dir(0.0f, 0.0f);

            switch (vecInfoPath[i].Direction())
            {
            case SD_NORTH: v2Dir = CVec2f(-1.0f, 0.0f); break;
            case SD_EAST: v2Dir = CVec2f(0.0f, 1.0f); break;
            case SD_SOUTH: v2Dir = CVec2f(1.0f, 0.0f); break;
            case SD_WEST: v2Dir = CVec2f(0.0f, -1.0f); break;
            default: break;
            }

            v2PosGate = v2Dir * vecInfoPath[i].GetRadiusPositive();
            v2PosGate += v2Src;

            v2NegGate = v2Dir * -vecInfoPath[i].GetRadiusNegative();
            v2NegGate += v2Src;

            if (vecInfoPath[i].Direction() != SD_INVALID)
            {
                Vid.AddLine(
                    CVec3f(v2PosGate.x, v2PosGate.y, pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                    CVec3f(v2NegGate.x, v2NegGate.y, pWorld->GetTerrainHeight(v2Src.x, v2Src.y)), 
                    vecInfoPath[i].GetColor());
            }
#endif
            {
                CVec4f v4PathLineColor(0.0f, 1.0f, 0.0f, 1.0f);

                Vid.AddLine(
                    CVec3f(v2Src.x, v2Src.y, pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                    CVec3f(v2End.x, v2End.y, pWorld->GetTerrainHeight(v2End.x, v2End.y)),
                    v4PathLineColor);
            }
        }
    }

    // Smoothed path
    if (pPath != nullptr)
    {
        PathResult &vecPath(pPath->GetSmoothResult());
        uint uiCount(uint(vecPath.size()));

        for (uint uiIndex(0); uiIndex + 1 < uiCount; ++uiIndex)
        {
            CVec2f v2Src(vecPath[uiIndex].GetPath());
            CVec2f v2Dst(vecPath[uiIndex + 1].GetPath());

            Vid.AddLine(
                CVec3f(v2Src.x, v2Src.y, pWorld->GetTerrainHeight(v2Src.x, v2Src.y)),
                CVec3f(v2Dst.x, v2Dst.y, pWorld->GetTerrainHeight(v2Dst.x, v2Dst.y)),
                CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    // Goal
    if (pPath != nullptr)
    {
        CVec2f v2Goal(pPath->GetGoal());

        Vid.AddLine(
            CVec3f(v2Goal.x - 10.0f, v2Goal.y - 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            CVec3f(v2Goal.x - 10.0f, v2Goal.y + 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            LIME);

        Vid.AddLine(
            CVec3f(v2Goal.x - 10.0f, v2Goal.y + 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            CVec3f(v2Goal.x + 10.0f, v2Goal.y + 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            LIME);

        Vid.AddLine(
            CVec3f(v2Goal.x + 10.0f, v2Goal.y + 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            CVec3f(v2Goal.x + 10.0f, v2Goal.y - 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            LIME);

        Vid.AddLine(
            CVec3f(v2Goal.x + 10.0f, v2Goal.y - 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            CVec3f(v2Goal.x - 10.0f, v2Goal.y - 10.0f, pWorld->GetTerrainHeight(v2Goal.x, v2Goal.y)),
            LIME);
    }
}


/*====================
  CGameClient::AddSelectionRingToScene
  ====================*/
void    CGameClient::AddSelectionRingToScene(uint uiIndex, const CVec3f &v3Pos, float fSize, uint uiOrderTime)
{
    CPlayer *pLocalPlayer(GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    if (fSize <= 0.0f)
        return;

    uint uiGameTime(Game.GetGameTime());

    bool bFlash(uiOrderTime != INVALID_TIME && uiOrderTime <= uiGameTime && uiOrderTime + 1000 > uiGameTime && (uiGameTime - uiOrderTime) / 250 % 2 == 0);

    if (!IsEntitySelected(uiIndex) && !IsEntityHoverSelected(uiIndex) && !bFlash)
        return;

    CSceneEntity sceneEntity;
    sceneEntity.Clear();

    sceneEntity.width = fSize;
    sceneEntity.height = fSize;
    sceneEntity.scale = 1.0f;
    sceneEntity.SetPosition(v3Pos);
    sceneEntity.hRes = m_ahResources[CLIENT_RESOURCE_SELECTION_INDICATOR];
    sceneEntity.flags = SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;
    sceneEntity.objtype = OBJTYPE_GROUNDSPRITE;
    sceneEntity.color = YELLOW;

    if (IsEntityHoverSelected(uiIndex) && !IsEntitySelected(uiIndex))
    {
        sceneEntity.color[R] *= 0.5f;
        sceneEntity.color[G] *= 0.5f;
        sceneEntity.color[B] *= 0.5f;
    }

    SceneManager.AddEntity(sceneEntity);
}


/*====================
  CGameClient::RenderWorldEntities
  ====================*/
void    CGameClient::RenderWorldEntities()
{
    PROFILE("CGameClient::RenderWorldEntities");

    static WorldEntVector vEntities;
    vEntities.clear();

    GetEntityHandlesInRegion(vEntities, SceneManager.GetFrustumBounds(), SURF_STATIC | SURF_DYNAMIC);

    static CSceneEntity cSceneEnt;

    WorldEntVector_cit citEnd(vEntities.end());
    for (WorldEntVector_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        CWorldEntity *pWorldEnt(GetWorldPointer()->GetEntityByHandle(*cit));

        cSceneEnt.Clear();

        cSceneEnt.objtype = OBJTYPE_MODEL;
        cSceneEnt.SetPosition(pWorldEnt->GetPosition());
        cSceneEnt.axis = pWorldEnt->GetAxis();
        cSceneEnt.scale = pWorldEnt->GetScale();
        cSceneEnt.hRes = pWorldEnt->GetModelHandle();
        cSceneEnt.bounds = pWorldEnt->GetRenderBounds();
        cSceneEnt.color = WHITE;
        cSceneEnt.flags = SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS | SCENEENT_USE_BOUNDS | SCENEENT_FOG_OF_WAR;

        if (pWorldEnt->GetSurfFlags() & SURF_CLIFF)
            cSceneEnt.flags |= SCENEENT_TERRAIN_TEXTURES;

        if (pWorldEnt->GetSurfFlags() & SURF_WATER)
        {
            SSceneEntityEntry &cEntry(SceneManager.AddEntity(cSceneEnt));

            if (!cEntry.bCull)
                Game.SetActiveReflection(true);
        }
        else
        {
            SSceneEntityEntry &cEntry(SceneManager.AddEntity(cSceneEnt));

            if (!cEntry.bCull)
            {
                if (pWorldEnt->GetSurfFlags() & SURF_TREE)
                {
                    IVisualEntity *pTree(GetVisualEntity(pWorldEnt->GetGameIndex()));
                    if (pTree != nullptr)
                    {
                        if (pTree->IsHighlighted())
                            cEntry.cEntity.color *= pTree->GetHighlightColor();

                        AddSelectionRingToScene(pWorldEnt->GetGameIndex(), pWorldEnt->GetPosition(), 64.0f, pTree->GetOrderTime());
                    }
                }
            }
        }
    }
}


/*====================
  CGameClient::GetPrecacheList
  ====================*/
void    CGameClient::GetPrecacheList(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
{
    if (sName.empty())
        return;

    IEntityDefinition *pDefinition(EntityRegistry.GetDefinition<IEntityDefinition>(sName));
    if (pDefinition == nullptr)
        return;

    pDefinition->GetPrecacheList(eScheme, sModifier, deqPrecache);
}


/*====================
  CGameClient::Precache
  ====================*/
void    CGameClient::Precache(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier)
{
    EntityRegistry.ClientPrecache(EntityRegistry.LookupID(sName), eScheme, sModifier);
}


/*====================
  CGameClient::Precache
  ====================*/
void    CGameClient::Precache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier)
{
    EntityRegistry.ClientPrecache(unType, eScheme, sModifier);
}


/*====================
  CGameClient::ParticleTrace
  ====================*/
bool    CGameClient::ParticleTrace(const CVec3f &v3Start, const CVec3f &v3End, CVec3f &v3EndPos, CVec3f &v3Normal)
{
    STraceInfo trace;
    if (Game.TraceLine(trace, v3Start, v3End, TRACE_TERRAIN))
    {
        v3EndPos = trace.v3EndPos;
        v3Normal = trace.plPlane.v3Normal;
        return true;
    }
    else
    {
        v3EndPos = v3End;
        v3Normal = CVec3f(0.0f, 0.0f, 1.0f);
        return false;
    }
}


/*====================
  CGameClient::CanAccessShop

  Determines if the current control entity can access a certain shop
  ====================*/
bool    CGameClient::CanAccessShop(const tstring &sShop)
{
    if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
        return true;

    IUnitEntity *pControl(m_pClientCommander->GetSelectedControlEntity());

    if (pControl == nullptr)
        return false;

    return pControl->CanAccessShop(sShop);
}


/*====================
  CGameClient::CanAccessLocalShop

  Determines if the current control entity can access a certain shop
  ====================*/
bool    CGameClient::CanAccessLocalShop(const tstring &sShop)
{
    if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
        return true;

    IUnitEntity *pControl(m_pClientCommander->GetSelectedControlEntity());

    if (pControl == nullptr)
        return false;

    return pControl->CanAccessLocalShop(sShop);
}


/*====================
  CGameClient::CanAccessItem

  Determines if the current control entity can access a certain item
  ====================*/
bool    CGameClient::CanAccessItem(const tstring &sItem)
{
    if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
        return true;

    IUnitEntity *pControl(m_pClientCommander->GetSelectedControlEntity());

    if (pControl == nullptr)
        return false;

    return pControl->CanAccessItem(sItem);
}


/*====================
  CGameClient::CanAccessItemLocal

  Determines if the current control entity can access a certain item
  ====================*/
bool    CGameClient::CanAccessItemLocal(const tstring &sItem)
{
    if (m_pLocalPlayer != nullptr && m_pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
        return true;

    IUnitEntity *pControl(m_pClientCommander->GetSelectedControlEntity());

    if (pControl == nullptr)
        return false;

    return pControl->CanAccessItemLocal(sItem);
}


/*====================
  CGameClient::GetItemRestockTime
  ====================*/
uint    CGameClient::GetItemRestockTime(const tstring &sItem)
{
    CPlayer *pLocalPlayer(GetLocalPlayer());

    if (pLocalPlayer == nullptr)
        return 0;

    CTeamInfo *pTeam(GetTeam(pLocalPlayer->GetTeam()));

    if (pTeam == nullptr)
        return 0;

    CShopInfo *pShop(pTeam->GetShopInfo());

    if (pShop == nullptr)
        return 0;

    return pShop->GetRestockTimeRemaining(sItem);
}


/*====================
  CGameClient::GetItemStock
  ====================*/
uint    CGameClient::GetItemStock(const tstring &sItem)
{
    CPlayer *pLocalPlayer(GetLocalPlayer());

    if (pLocalPlayer == nullptr)
        return -1;

    CTeamInfo *pTeam(GetTeam(pLocalPlayer->GetTeam()));

    if (pTeam == nullptr)
        return -1;

    CShopInfo *pShop(pTeam->GetShopInfo());

    if (pShop == nullptr)
        return -1;

    return pShop->GetStockRemaining(sItem);
}


/*====================
  CGameClient::GetShop
  ====================*/
ushort  CGameClient::GetShop(const tstring &sItem)
{
    if (sItem.empty())
        return INVALID_ENT_TYPE;

    static vector<ushort> vShops;
    vShops.clear();

    ushort unBestShop(INVALID_ENT_TYPE);
    int iBestShopOrder(-1);
    
    EntityRegistry.GetShopList(vShops);

    for (vector<ushort>::iterator it(vShops.begin()), itEnd(vShops.end()); it != itEnd; ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr)
            continue;

        if (pShop->GetOrder() < iBestShopOrder)
            continue;

        const tsvector &vsItems(pShop->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
        {
            if (*cit == sItem)
            {
                unBestShop = *it;
                iBestShopOrder = pShop->GetOrder();
                break;
            }
        }
    }

    return unBestShop;
}


/*====================
  CGameClient::GetUsedIn
  ====================*/
void    CGameClient::GetUsedIn(const tstring &sItem, vector<ushort> &vUsedIn)
{
    if (sItem.empty())
        return;

    static vector<ushort> vItems;
    vItems.clear();
    
    EntityRegistry.GetItemList(vItems);

    for (vector<ushort>::iterator itItem(vItems.begin()), itEnd(vItems.end()); itItem != itEnd; ++itItem)
    {
        CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(*itItem));
        if (pItem == nullptr)
            continue;

        for (uint ui(0); ui < pItem->GetComponentsSize(); ++ui)
        {
            const tsvector &vComponents(pItem->GetComponents(ui));
            for (tsvector_cit itComponent(vComponents.begin()); itComponent != vComponents.end(); ++itComponent)
            {
                if (*itComponent == sItem && find(vUsedIn.begin(), vUsedIn.end(), *itItem) == vUsedIn.end())
                    vUsedIn.push_back(*itItem);
            }
        }
    }
}


/*====================
  CGameClient::SendCreateGameRequest
  ====================*/
void    CGameClient::SendCreateGameRequest(const tstring &sName, const tstring &sSettings)
{
    CBufferDynamic buffer;
    buffer << GAME_CMD_CREATE_GAME << TStringToUTF8(sName) << byte(0) << TStringToUTF8(sSettings) << byte(0);
    GameClient.SendGameData(buffer, true);
    m_pHostClient->UpdateServerTimeout(cg_createGameTimeOut);   // Give the server some time to load the world
}


/*====================
  CGameClient::GetGameMessage
  ====================*/
tstring     CGameClient::GetGameMessage(const tstring &sKey, const tsmapts &mapTokens)
{
    CStringTable *pGameMessages(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_GAME_MESSAGE_TABLE]));
    if (pGameMessages == nullptr)
        return TSNULL;

    tstring sMessage(pGameMessages->Get(sKey));

    if (mapTokens.empty())
        return sMessage;

    size_t zOffset(0);
    while (zOffset != tstring::npos)
    {
        size_t zStart(sMessage.find(_T('{'), zOffset));
        if (zStart == tstring::npos)
            break;
        size_t zEnd(sMessage.find(_T('}'), zStart));
        if (zEnd == tstring::npos)
            break;

        // Default parameter
        size_t zMid(sMessage.find(_T('='), zStart));
        if (zMid < zEnd)
        {
            const tstring &sToken(sMessage.substr(zStart + 1, zMid - zStart - 1));
            tsmapts_cit itFind(mapTokens.find(sToken));

            if (itFind != mapTokens.end())
            {
                const tstring &sValue(itFind->second);
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            else
            {
                const tstring &sValue(sMessage.substr(zMid + 1, zEnd - zMid - 1));
                zOffset = zStart + sValue.length();
                sMessage.replace(zStart, zEnd - zStart + 1, sValue);
            }
            continue;
        }

        const tstring &sToken(sMessage.substr(zStart + 1, zEnd - zStart - 1));

        tsmapts_cit itFind(mapTokens.find(sToken));
        const tstring &sValue(itFind == mapTokens.end() ? TSNULL : itFind->second);
        zOffset = zStart + sValue.length();
        sMessage.replace(zStart, zEnd - zStart + 1, sValue);
    }

    return sMessage;
}


/*====================
  CGameClient::GetEntityString
  ====================*/
const tstring&  CGameClient::GetEntityString(const tstring &sKey) const
{
    CStringTable *pEntityStrings(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_ENTITY_STRING_TABLE]));
    if (pEntityStrings == nullptr)
        return TSNULL;

    return pEntityStrings->Get(sKey);
}

const tstring&  CGameClient::GetEntityString(uint uiIndex) const
{
    CStringTable *pEntityStrings(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_ENTITY_STRING_TABLE]));
    if (pEntityStrings == nullptr)
        return TSNULL;

    return pEntityStrings->Get(uiIndex);
}


/*====================
  CGameClient::GetEntityStringIndex
  ====================*/
uint    CGameClient::GetEntityStringIndex(const tstring &sKey) const
{
    CStringTable *pEntityStrings(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_ENTITY_STRING_TABLE]));
    if (pEntityStrings == nullptr)
        return INVALID_INDEX;

    return pEntityStrings->GetIndex(sKey);
}


/*====================
  CGameClient::PostProcessEntities
  ====================*/
void    CGameClient::PostProcessEntities()
{
    map<ushort, tstring> mapEntities;
    EntityRegistry.GetEntityList(mapEntities);
    for (map<ushort, tstring>::iterator itEntity(mapEntities.begin()); itEntity != mapEntities.end(); ++itEntity)
        EntityRegistry.PostProcess(itEntity->first);
}


/*====================
  CGameClient::LevelupAbility
  ====================*/
bool    CGameClient::LevelupAbility(byte ySlot)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return false;

    IHeroEntity *pHero(GetHeroEntity(uiUnitIndex));
    if (pHero != nullptr)
    {
        if (pHero->GetAvailablePoints() == 1)
            SetLevelup(false);
    }

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_LEVEL_UP << uiUnitIndex << ySlot;
    GameClient.SendGameData(buffer, true);
    
    return true;
}


/*====================
  CGameClient::SetLevelup
  ====================*/
void    CGameClient::SetLevelup(bool bLevelup)
{
    m_bLevelup = bLevelup;

    if (bLevelup)
        m_uiLevelupIndex = m_pClientCommander->GetSelectedControlEntityIndex();
    else
        m_uiLevelupIndex = INVALID_INDEX;
}


/*====================
  CGameClient::ValidateLevelup
  ====================*/
void    CGameClient::ValidateLevelup()
{
    if (!m_bLevelup)
        return;

    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex != m_uiLevelupIndex)
        SetLevelup(false);
}


/*====================
  CGameClient::SetActiveShop
  ====================*/
void    CGameClient::SetActiveShop(const tstring &sShop, bool bForce)
{
    if (m_pInterfaceManager->GetShopLock() && !bForce)
        return;

    m_sActiveShop = sShop;
    SetActiveRecipe(TSNULL, false);
}


/*====================
  CGameClient::SetActiveRecipe
  ====================*/
void    CGameClient::SetActiveRecipe(const tstring &sRecipe, bool bHistory, bool bForce)
{
    if (m_pInterfaceManager->GetShopLock() && !bForce)
        return;

    m_sActiveRecipe = sRecipe;

    if (!m_sActiveRecipe.empty())
        m_sActiveShop = EntityRegistry.LookupName(GetShop(m_sActiveRecipe));
}


/*====================
  CGameClient::Purchase
  ====================*/
bool    CGameClient::Purchase(int iSlot)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());

    ushort unShop(EntityRegistry.LookupID(GetActiveShop()));
    if (unShop == INVALID_ENT_TYPE)
        return false;

    CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
    if (pShop == nullptr)
        return false;

    CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(pShop->GetItem(iSlot)));
    if (pItem == nullptr)
        return false;

    const tstring &sName(pItem->GetName());

    // TODO: Do the modifiers a different way
    if ((pItem->IsRecipe() || Input.IsButtonDown(BUTTON_SHIFT)) && !Input.IsButtonDown(BUTTON_CTRL))
    {
        if (GetActiveRecipe() != pShop->GetItem(iSlot))
            SetActiveRecipe(pShop->GetItem(iSlot), false);
        else
            SetActiveRecipe(TSNULL, false);
    }
    else if (unShop == EntityRegistry.LookupID(_T("Shop_Recommended")))
    {
        if (uiUnitIndex == INVALID_INDEX)
            return false;

        ushort unID(EntityRegistry.LookupID(pShop->GetItem(iSlot)));

        if (unID == INVALID_ENT_TYPE)
            return false;

        bool bAccess(CanAccessItem(sName));
        bool bRecipe(pItem->IsRecipe());

        tsmapts mapTokens;
        mapTokens[_T("item")] = pItem->GetDisplayName();

        if (!bAccess)
        {
            m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_item_unavailable"), mapTokens));
            return false;
        }
        else if (m_pLocalPlayer->GetGold() < pItem->GetCost())
        {
            if (bRecipe)
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_recipe"), mapTokens));
            else
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_item"), mapTokens));

            return false;
        }

        CBufferFixed<7> buffer;
        buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
        SendGameData(buffer, true);
    }
    else
    {
        if (uiUnitIndex == INVALID_INDEX)
            return false;

        bool bAccess(CanAccessItem(sName));
        bool bRecipe(pItem->IsRecipe());

        tsmapts mapTokens;
        mapTokens[_T("item")] = pItem->GetDisplayName();

        if (!bAccess)
        {
            m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_item_unavailable"), mapTokens));
            return false;
        }
        else if (m_pLocalPlayer->GetGold() < pItem->GetCost())
        {
            if (bRecipe)
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_recipe"), mapTokens));
            else
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_item"), mapTokens));

            return false;
        }

        CBufferFixed<8> buffer;
        buffer << GAME_CMD_PURCHASE << uiUnitIndex << unShop << byte(iSlot);
        SendGameData(buffer, true);
    }

    return true;
}


/*====================
  CGameClient::PurchaseComponent
  ====================*/
bool    CGameClient::PurchaseComponent(int iSlot, int iVariation)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());

    ushort unShop(EntityRegistry.LookupID(GetActiveShop()));
    if (unShop == INVALID_ENT_TYPE)
        return false;

    CItemDefinition *pRecipeItem(EntityRegistry.GetDefinition<CItemDefinition>(GetActiveRecipe()));
    if (pRecipeItem == nullptr)
        return false;

    uint uiComponentList(iVariation);
    if (uiComponentList >= pRecipeItem->GetComponentsSize())
        return false;

    const tsvector &vComponents(pRecipeItem->GetComponents(uiComponentList));
    uint uiSlot(iSlot);
    uiSlot = MIN(uiSlot, INT_SIZE(vComponents.size()));

    const tstring &sName(uiSlot == vComponents.size() ? GetActiveRecipe() : vComponents[uiSlot]);

    ushort unID(EntityRegistry.LookupID(sName));
    if (unID == INVALID_ENT_TYPE)
        return false;

    CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(unID));
    if (pItem == nullptr)
        return false;

    // TODO: Do the modifiers a different way
    if (((pItem->IsRecipe() && uiSlot != vComponents.size()) || Input.IsButtonDown(BUTTON_SHIFT)) && !Input.IsButtonDown(BUTTON_CTRL))
    {
        SetActiveRecipe(sName, true);
    }
    else
    {
        if (uiUnitIndex == INVALID_INDEX)
            return false;

        bool bAccess(CanAccessItem(sName));
        bool bRecipe(pItem->IsRecipe());

        tsmapts mapTokens;
        mapTokens[_T("item")] = pItem->GetDisplayName();

        if (!bAccess)
        {
            m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_item_unavailable"), mapTokens));
            return false;
        }
        else if (m_pLocalPlayer->GetGold() < pItem->GetCost())
        {
            if (bRecipe)
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_recipe"), mapTokens));
            else
                m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_item"), mapTokens));

            return false;
        }

        CBufferFixed<8> buffer;
        buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
        SendGameData(buffer, true);
    }

    return true;
}


/*====================
  CGameClient::PurchaseAllComponents
  ====================*/
bool    CGameClient::PurchaseAllComponents(const tstring& sName)
{
    if (sName.empty())
        return false;
        
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    if (uiUnitIndex == INVALID_INDEX)
        return false;
        
    CItemDefinition *pRecipeItem(EntityRegistry.GetDefinition<CItemDefinition>(sName));
    if (pRecipeItem == nullptr)
        return false;
        
    // not sure why this is always 0, but it is associated with iVariation in Shop()
    uint uiComponentList(0);
    const tsvector &vComponents(pRecipeItem->GetComponents(uiComponentList));
    
    ushort unID(EntityRegistry.LookupID(sName));
    if (unID == INVALID_ENT_TYPE)
        return false;
        
    CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(unID));
    if (pItem == nullptr)
        return false;
        
    tsmapts mapTokens;
        
    if (pItem->GetTotalCost() > m_pLocalPlayer->GetGold())
    {
        mapTokens[_T("item")] = pItem->GetDisplayName();        
        m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_low_gold_buy_all"), mapTokens));

        return false;
    }
    
    // if it's a recipe, try to purchase it here
    if (!pItem->GetAutoAssemble() && CanAccessItem(sName))
    {
        CBufferFixed<8> buffer;
        buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
        SendGameData(buffer, true);
    }
                    
    // loop over each component needed for this item
    for (tsvector_cit it(vComponents.begin()); it != vComponents.end(); ++it)
    {
        // get the component name from the vector list of components, note that actual "Recipes" are not included as components
        tstring sComponent(it->c_str());
        
        // get the entity ID from the sub item name
        unID = EntityRegistry.LookupID(sComponent);
        if (unID == INVALID_ENT_TYPE)
            return false;
            
        // take the entity ID and lookup the entity definition of the component
        pItem = EntityRegistry.GetDefinition<CItemDefinition>(unID);
        if (pItem == nullptr)
            return false;                   
            
        mapTokens[_T("item")] = pItem->GetDisplayName();
        
        // recursive call to this function to purchase components that also require recipes
        if (pItem->GetComponentsSize() > 0)
        {
            if (PurchaseAllComponents(sComponent))
                continue;
            else
                return false;
        }
        
        // if the player doesn't have this component up on their shop menu, display error       
        if (!CanAccessItem(sComponent))
        {
            m_pClientCommander->PrintErrorMessage(GetGameMessage(_T("error_item_unavailable"), mapTokens));
            continue;
        }
            
        // they have access to the shop where this component is, and they have enough gold, purchase the item
        CBufferFixed<8> buffer;
        buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
        SendGameData(buffer, true);
    }

    return true;
}


/*====================
  CGameClient::PurchaseUsedIn
  ====================*/
bool    CGameClient::PurchaseUsedIn(int iSlot)
{
    uint uiUnitIndex(m_pClientCommander->GetSelectedControlEntityIndex());
    
    ushort unShop(EntityRegistry.LookupID(GetActiveShop()));
    if (unShop == INVALID_ENT_TYPE)
        return false;

    static vector<ushort> vUsedIn;
    vUsedIn.clear();

    GetUsedIn(GetActiveRecipe(), vUsedIn);
    
    ushort unID(iSlot >= 0 && iSlot < int(vUsedIn.size()) ? vUsedIn[iSlot] : INVALID_ENT_TYPE);
    if (unID == INVALID_ENT_TYPE)
        return false;

    CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(unID));
    if (pItem == nullptr)
        return false;

    // TODO: Do the modifiers a different way
    if ((pItem->IsRecipe() || Input.IsButtonDown(BUTTON_SHIFT)) && !Input.IsButtonDown(BUTTON_CTRL))
    {
        SetActiveRecipe(EntityRegistry.LookupName(unID), true);
    }
    else
    {
        if (uiUnitIndex == INVALID_INDEX)
            return false;

        CBufferFixed<8> buffer;
        buffer << GAME_CMD_PURCHASE2 << uiUnitIndex << unID;
        SendGameData(buffer, true);
    }

    return true;
}


/*====================
  CGameClient::Shop
  ====================*/
void    CGameClient::Shop(int iSlot)
{
    if (iSlot == -1)
    {
        if (!m_sActiveRecipe.empty())
            SetActiveRecipe(TSNULL, false);
        else if (!m_sActiveShop.empty())
            SetActiveShop(TSNULL);
        else
            m_pInterfaceManager->SetShopVisible(false);

        return;
    }

    if (GetActiveShop().empty())
    {
        vector<ushort> vShops;
        EntityRegistry.GetShopList(vShops);

        for (vector<ushort>::iterator it(vShops.begin()), itEnd(vShops.end()); it != itEnd; ++it)
        {
            CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
            if (pShop == nullptr)
                continue;

            if (pShop->GetSlot() == iSlot)
                SetActiveShop(EntityRegistry.LookupName(*it));
        }
    }
    else if (GetActiveRecipe().empty())
    {
        Purchase(iSlot);
    }
    else
    {
        int iVariation(0);

        CItemDefinition *pRecipeItem(EntityRegistry.GetDefinition<CItemDefinition>(GetActiveRecipe()));
        if (pRecipeItem == nullptr)
            return;

        uint uiComponentList(iVariation);
        if (uiComponentList >= pRecipeItem->GetComponentsSize())
            return;

        const tsvector &vComponents(pRecipeItem->GetComponents(uiComponentList));

        int iNumComponents(int(vComponents.size()));

        if (pRecipeItem->IsRecipe() && !pRecipeItem->GetAutoAssemble())
            ++iNumComponents;

        if (iSlot >= iNumComponents)
            PurchaseUsedIn(iSlot - iNumComponents);
        else
            PurchaseComponent(iSlot, iVariation);
    }
}


/*====================
  CGameClient::ToggleLevelup
  ====================*/
void    CGameClient::ToggleLevelup()
{
    m_pInterfaceManager->ToggleLevelupInterface();
}


/*====================
  CGameClient::Ping
  ====================*/
void    CGameClient::Ping(byte yType, float fX, float fY, int iClientNumber)
{
    m_v3LastEvent = Game.GetTerrainPosition(CVec2f(fX, fY));

    const CPing *pPing(GetPing(yType));
    if (pPing == nullptr)
        return;

    CPlayer *pPlayer(GetPlayerFromClientNumber(iClientNumber));

    static tsvector vMapEffect(4);
    vMapEffect[0] = pPing->GetEffectPath();
    vMapEffect[1] = XtoA(fX / Game.GetWorldWidth());
    vMapEffect[2] = XtoA(fY / Game.GetWorldHeight());
    vMapEffect[3] = (pPlayer != nullptr && pPing->GetUsePlayerColor() ? XtoA(pPlayer->GetColor()) : XtoA(pPing->GetColor()));

    MapEffect.Trigger(vMapEffect);
}


/*====================
  CGameClient::UpdateRecommendedItems
  ====================*/
void    CGameClient::UpdateRecommendedItems()
{
    IHeroEntity *pHero(m_pLocalPlayer->GetHero());

    if (pHero == nullptr)
        return;

    if (pHero->GetType() == m_unLastHeroType)
        return;

    // Hero has changed, update items in recommended shops
    CHeroDefinition *pHeroDef(EntityRegistry.GetDefinition<CHeroDefinition>(pHero->GetType()));

    if (pHeroDef == nullptr)
        return;

    m_unLastHeroType = pHero->GetType();

    vector<ushort> vShops;
    EntityRegistry.GetShopList(vShops);

    for (vector<ushort>::iterator it(vShops.begin()); it != vShops.end(); it++)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));

        if (pShop == nullptr || !pShop->GetRecommendedItems())
            continue;

        pShop->ClearItems();
        pShop->SetItems(pHeroDef->GetRecommendedItems());
    }
}


/*====================
  CGameClient::DownloadReplay
  ====================*/
bool    CGameClient::DownloadReplay(const tstring &sUrl)
{
#if 1
    if (m_bDownloadingReplay)
        return false;

    CFile *pFile(FileManager.GetFile(_T("~/replays/") + Filename_StripPath(sUrl), FILE_WRITE | FILE_BINARY));

    if (pFile == nullptr)
        return false;

    if (!pFile->IsOpen())
    {
        SAFE_DELETE(pFile);
        return false;
    }

    m_sLastReplay = sUrl;
    m_pReplayDownload->SetFileTarget(pFile);

    m_bDownloadingReplay = m_pReplayDownload->Open(m_sLastReplay, FILE_HTTP_WRITETOFILE);

    m_uiLastSpeedUpdate = 0;
    m_uiNumSpeedUpdates = 1;
    m_uiFirstSpeedUpdate = 0;
    
    return m_bDownloadingReplay;
#else
    return false;
#endif
}


/*====================
  CGameClient::GetReplayDownloadProgress
  ====================*/
float   CGameClient::GetReplayDownloadProgress()
{
    return m_pReplayDownload->GetProgressPercent();
}


/*====================
  CGameClient::GetReplayDownloadProgress
  ====================*/
bool    CGameClient::ReplayDownloadErrorEncountered()
{
    return m_pReplayDownload->ErrorEncountered();
}


/*====================
  CGameClient::StopReplayDownload
  ====================*/
void    CGameClient::StopReplayDownload()
{
    if (!m_bDownloadingReplay)
        return;

    m_pReplayDownload->StopTransfer(m_sLastReplay);
    m_bDownloadingReplay = false;

    FileManager.Delete(_T("~/replays/") + Filename_StripPath(m_sLastReplay));
}


/*====================
  CGameClient::LoadStringTables
  ====================*/
void    CGameClient::LoadStringTables()
{
    // Register string tables immediately so that other entities can be loaded
    m_ahResources[CLIENT_RESOURCE_GAME_MESSAGE_TABLE] = g_ResourceManager.Register(_T("/stringtables/game_messages.str"), RES_STRINGTABLE);
}


/*====================
  CGameClient::SendScriptMessage
  ====================*/
bool    CGameClient::SendScriptMessage(const tstring &sName, const tstring &sValue)
{
    CBufferDynamic buffer;
    buffer << GAME_CMD_SCRIPT_MESSAGE << TStringToUTF8(sName) << byte(0) << TStringToUTF8(sValue) << byte(0);
    SendGameData(buffer, true);
    return true;
}


/*====================
  CGameClient::GetLocalVoiceSet
  ====================*/
SVoiceSet&  CGameClient::GetLocalVoiceSet()
{
    uint uiAnnouncerVoice(m_pHostClient->GetAnnouncerVoice());
    if (uiAnnouncerVoice == INVALID_INDEX)
    {
        if (m_vAnnouncerVoices.empty())
            return g_NullVoiceSet;

        return m_vAnnouncerVoices.back();
    }

    assert(uiAnnouncerVoice < m_vAnnouncerVoices.size());
    if (uiAnnouncerVoice >= m_vAnnouncerVoices.size())
        return g_NullVoiceSet;

    return m_vAnnouncerVoices[uiAnnouncerVoice];
}


/*====================
  CGameClient::GetLocalVoiceSet
  ====================*/
bool    CGameClient::CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)
{
    return m_pHostClient->CanAccessAltAvatar(sHero, sAltAvatar);
}


/*====================
  CGameClient::ServerRefreshUpgrades
  ====================*/
void    CGameClient::ServerRefreshUpgrades()
{
    if (!IsConnected())
        return;

    CBufferFixed<1> buffer;
    buffer << GAME_CMD_REFRESH_UPGRADES;
    GameClient.SendGameData(buffer, true);
}


/*====================
  CGameClient::CalcAvailableWeatherEffects
  ====================*/
uint    CGameClient::CalcAvailableWeatherEffects(vector<SWeatherInfo>& vWeather)
{
    // If the "game_messages" stringtable hasn't yet been reloaded, abort.
    if (m_ahResources[CLIENT_RESOURCE_GAME_MESSAGE_TABLE] == INVALID_RESOURCE)
    {
        Console.Err << _T("CGameClient::CalcAvailableWeatherEffects() - Failed:  stringtable 'game_messages' has not yet been loaded") << newl;
        assert(!"CalcAvailableWeatherEffects - game_messages.str not yet loaded");
        return 0;
    }

    //Get the "game_messages" stringtable
    CStringTable* pGameMessagesStringtable(g_ResourceManager.GetStringTable(m_ahResources[CLIENT_RESOURCE_GAME_MESSAGE_TABLE]));

    // Add the default weather effect
    tstring sDefaultName;
    tstring sDefaultKey;
    {
        SWeatherInfo cInfo;

        cInfo.sFilePath = _T("");
        cInfo.sKeyName = _T("");
        cInfo.sLocalizedName = _T("");
        // Now localize the name of the default weather effect
        //  Find weather entry in stringtable ("acidrain" => "game_weather_acidrain" => "Chuva ��cida")
        tstring sStringtableKey(_TS("game_weather_"));
        uint uiFindIdx(pGameMessagesStringtable->GetIndex(sStringtableKey));
        if (uiFindIdx != INVALID_INDEX)
            cInfo.sLocalizedName = pGameMessagesStringtable->Get(uiFindIdx);
        sDefaultName = LowerString(cInfo.sLocalizedName);
        sDefaultKey =  LowerString(StringReplace(sDefaultName, _T(" "), _T("")));

        vWeather.push_back(cInfo);
    }

    // Get a list of all weather effect files (for example ":/game/shared/effects/weather/*.effect")
    tsvector vFileList;
    if (!FileManager.GetFileList(_T("/shared/effects/weather/"), _T("*.effect"), false, vFileList, true))
    {
        Console.Err << _T("CGameClient::CalcAvailableWeatherEffects() - Failed:  FileManager.GetFileList(\"/shared/effects/weather/\", \"*.effect\")") << newl;
        assert(!"CalcAvailableWeatherEffects - GetFileList");
        return 0;
    }

    uint uiCount(1);
    for (size_t i(0); i < vFileList.size(); ++i)
    {
        const tstring& sFilename(vFileList[i]);

        SWeatherInfo cInfo;

        // Sanity check: sanitize the filename, resolving it to root
        cInfo.sFilePath = FileManager.SanitizePath(sFilename, true);
        if (cInfo.sFilePath.empty())
            continue;

        // Now generate a key name for the weather effect
        //  "/game/shared/effects/weather/aCiD rAiN.effect" => "aCiD rAiN"
        cInfo.sKeyName = Filename_GetName(cInfo.sFilePath);
        //  "aCiD rAiN" becomes the default localized name of the effect
        cInfo.sLocalizedName = cInfo.sKeyName;
        //  Strip spaces and lowercase
        //  "aCiD rAiN" => "acidrain"
        //  UNICODE-ALERT:  Is this algorithm Unicode-correct?
        cInfo.sKeyName = LowerString(StringReplace(cInfo.sKeyName, _T(" "), _T("")));

        // Now localize the name of the weather effect
        //  Find weather entry in stringtable ("acidrain" => "game_weather_acidrain" => "Chuva ��cida")
        tstring sStringtableKey(_TS("game_weather_") + cInfo.sKeyName);
        uint uiFindIdx(pGameMessagesStringtable->GetIndex(sStringtableKey));
        if (uiFindIdx != INVALID_INDEX)
            cInfo.sLocalizedName = pGameMessagesStringtable->Get(uiFindIdx);
        
        // Don't add the default entry twice
        if (cInfo.sKeyName.empty())
            continue;
        if (!sDefaultName.empty())
            if (cInfo.sKeyName == sDefaultKey || cInfo.sLocalizedName == sDefaultName)
                continue;

        // Finally, add the weather effect
        ++uiCount;
        vWeather.push_back(cInfo);
    }

    // Return the number of weather effects we returned
    return uiCount;
}


/*====================
  CGameClient::IsWeatherEffectName
  ====================*/
bool    CGameClient::IsWeatherEffectName(const tstring& sStartWeather)
{
    if (sStartWeather.empty())
        return false;

    vector<SWeatherInfo> vWeather;
    CalcAvailableWeatherEffects(vWeather);

    tstring sStartWeatherLower(LowerString(sStartWeather));

    for (uint i(0); i < vWeather.size(); ++i)
    {
        SWeatherInfo& cWeatherInfo(vWeather[i]);

        if ((cWeatherInfo.sKeyName == sStartWeatherLower)
            || LowerString(cWeatherInfo.sLocalizedName) == sStartWeatherLower)
        {
            return true;
        }
    }
    return false;
}


/*====================
  CGameClient::StartWeatherByName
  ====================*/
uint    CGameClient::StartWeatherByName(const tstring& sStartWeather)
{
    if (sStartWeather.empty())
        return INVALID_INDEX;

    tstring sStartWeatherLower(LowerString(sStartWeather));

    vector<SWeatherInfo> vWeather;
    CalcAvailableWeatherEffects(vWeather);

    uint uiIdx(INVALID_INDEX);
    for (uint i(0); i < vWeather.size(); ++i)
    {
        SWeatherInfo& cWeatherInfo(vWeather[i]);

        if ((cWeatherInfo.sKeyName != sStartWeatherLower)
            && LowerString(cWeatherInfo.sLocalizedName) != sStartWeatherLower)
        {
            continue;
        }

        // Found the weather effect, start it
        uiIdx = StartWeather(cWeatherInfo);
        if (uiIdx != INVALID_INDEX)
            break;
    }
    return uiIdx;
}


/*====================
CGameClient::StartWeather
====================*/
uint    CGameClient::StartWeather(const SWeatherInfo& cWeatherInfo)
{
    // Default weather effect?
    if (cWeatherInfo.sFilePath.empty())
        return INVALID_INDEX;

    if (cWeatherInfo.sKeyName.empty())
    {
        Console.Err << _T("CGameClient::StartWeather() - Empty key name for: ") << cWeatherInfo.sFilePath << newl;
        return INVALID_INDEX;
    }

    // Load each weather effect into its own resource context such as "client:weather:rain"
    ResHandle hEffect(INVALID_RESOURCE);
    K2_WITH_RESOURCE_SCOPE(_TS("client:weather:") + cWeatherInfo.sKeyName)
    {
        hEffect = g_ResourceManager.Register(cWeatherInfo.sFilePath, RES_EFFECT);
        if (hEffect == INVALID_RESOURCE)
        {
            Console.Err << _T("CGameClient::StartWeather() - Failed to load effect '") << cWeatherInfo.sFilePath << _T("'") << newl;
            return INVALID_INDEX;
        }
    }

    // Start the weather effect
    int iIdx(StartClientGameEffect(hEffect, -1, 0, V3_ZERO));
    if (iIdx < 0 || iIdx >= NUM_CLIENT_GAME_EFFECT_THREADS)
        return INVALID_INDEX;

    // Add the weather effect to the active list
    assert(m_mapActiveWeatherEffects.find(m_uiWeatherEffectCounter) == m_mapActiveWeatherEffects.end());
    pair<WeatherInfoMap::iterator, bool> cAdded(m_mapActiveWeatherEffects.insert(std::make_pair(m_uiWeatherEffectCounter, cWeatherInfo)));;;;;

    assert(cAdded.second);
    if (!cAdded.second)
    {
        Console.Err << _T("CGameClient::StartWeather() - Failed to start '") << cWeatherInfo.sLocalizedName << _T("'") << newl;
        return INVALID_INDEX;
    }

    SWeatherInfo& cAddedInfo((cAdded.first)->second);
    assert(cAddedInfo.iEffectChannel == -1);
    cAddedInfo.iEffectChannel = iIdx;
    assert(cAddedInfo.hEffect == INVALID_RESOURCE);
    cAddedInfo.hEffect = hEffect;
    return m_uiWeatherEffectCounter++;
}


/*====================
CGameClient::StopWeather
====================*/
void    CGameClient::StopWeather(uint uiIdx)
{
    if (uiIdx == INVALID_INDEX)
        return;

    WeatherInfoMap::iterator itFind(m_mapActiveWeatherEffects.find(uiIdx));
    if (itFind == m_mapActiveWeatherEffects.end())
        return;

    const SWeatherInfo& cWeatherInfo(itFind->second);

    // Stop the weather effect
    StopClientGameEffect(cWeatherInfo.iEffectChannel);

    // Unload its resource context
    g_ResourceInfo.ExecCommandLine(_TS("context delete client:weather:") + cWeatherInfo.sKeyName);

    // Remove the weather effect from the active list
    m_mapActiveWeatherEffects.erase(itFind);
}


/*====================
  CGameClient::StopWeatherByName
  ====================*/
uint    CGameClient::StopWeatherByName(const tstring& sFindName)
{
    for (WeatherInfoMap::iterator it(m_mapActiveWeatherEffects.begin()), itEnd(m_mapActiveWeatherEffects.end());
        it != itEnd;
        ++it)
    {
        uint uiIdx(it->first);
        SWeatherInfo& sWeatherInfo(it->second);

        if ((sFindName == sWeatherInfo.sKeyName) ||
            (sFindName == LowerString(sWeatherInfo.sLocalizedName)))
        {
            StopWeather(uiIdx);
            return uiIdx;
        }
    }

    return INVALID_INDEX;
}


/*====================
CGameClient::StopAllWeather
====================*/
void    CGameClient::StopAllWeather()
{
    // Stop each weather effect
    while (!m_mapActiveWeatherEffects.empty())
    {
        WeatherInfoMap::iterator it(m_mapActiveWeatherEffects.begin());
        uint uiEffectIdx(it->first);
        StopWeather(uiEffectIdx);
    }
}
