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
#include "c_voicemanager.h"
#include "c_voice.h"

#include "../game_shared/c_playercommander.h"
#include "../game_shared/c_teamdefinition.h"
#include "../game_shared/c_teaminfo.h"
#include "../game_shared/c_replaymanager.h"
#include "../game_shared/c_entitygameinfo.h"
#include "../game_shared/c_entitynpccontroller.h"
#include "../game_shared/c_entityclientinfo.h"
#include "../game_shared/c_entityeffect.h"

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
#include "../k2/c_zip.h"
#include "../k2/c_eventmanager.h"
#include "../k2/c_npcdefinition.h"
#include "../k2/c_dbmanager.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_chatmanager.h"
#include "../k2/c_stringtable.h"
#include "../k2/c_statestring.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOL   (cg_debugPrediction,                false);

CVAR_BOOL   (cg_prediction,                     true);
CVAR_BOOLF  (cg_debugClientSnapshot,            false,      CONEL_DEV);
CVAR_INT    (cg_drawMeleeTraces,                0);
CVAR_BOOLF  (cg_meleeHitSounds,                 false,      CVAR_SAVECONFIG);

CVAR_FLOAT  (cg_snapcastLerp,                   10.0f);
CVAR_FLOAT  (cg_snapcastCumulativeDecay,        15.0f);
CVAR_FLOAT  (cg_snapcastCumulativeBreakAngle,   6.0f);
CVAR_INT    (cg_snapcastReacquireTime,          100);
CVAR_FLOAT  (cg_snapcastTracesize,              0.0f);

CVAR_FLOAT  (cg_cmdrSnapBreakDistance,          50.0f);
CVAR_BOOL   (cg_cmdrSnapStickyCursor,           false);

CVAR_UINTF  (cg_dashActivationTime,             250,        CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_showTips,                       true,       CVAR_SAVECONFIG);
CVAR_UINTF  (cg_gameTipDisplayTime,             2000,       CVAR_SAVECONFIG);

CVAR_UINTF  (cg_buildingAttackAlertTime,        5000,       CVAR_SAVECONFIG);
CVAR_UINTF  (cg_hitNotificationInterval,        100,        CVAR_SAVECONFIG);

CVAR_UINTF  (cg_teamTipShowTime,                10000,      CVAR_SAVECONFIG);
CVAR_UINTF  (cg_teamTipReshowTime,              45000,      CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_precacheEntities,               true,       CVAR_SAVECONFIG);

CVAR_STRINGF(cg_gameTime,                       "",         CVAR_READONLY);

CVAR_BOOL   (cg_debugGameEvent,                 false);

CVAR_INT    (cg_serverSnapshotCacheSize,        30);

CVAR_STRINGF(cg_worldAmbientSound,              "",         CVAR_WORLDCONFIG);
CVAR_FLOATF (cg_worldAmbientSoundVolume,        1.0,        CVAR_WORLDCONFIG);

CVAR_BOOL   (cg_drawPlayerStats,                false);
CVAR_BOOL   (cg_drawHoverStats,                 false);

EXTERN_CVAR (bool, cg_debugEntities);

CVAR_BOOL   (cg_debugGameData,                  false);

CVAR_UINTF  (cam_endGameLerpTime,               3000,                           CVAR_GAMECONFIG);
CVAR_FLOATF (cam_endGameMinHeight,              10.0f,                          CVAR_GAMECONFIG);
CVAR_UINTF  (cam_endGameTerrainAngleAmount,     0.5f,                           CVAR_GAMECONFIG);
CVAR_VEC3F  (cam_endGameOffset,                 CVec3f(0.0f, 2000.0f, 1000.0f), CVAR_GAMECONFIG);

CCvarb *sound_voiceDisabled(IMPORT_CVAR_BOOL("sound_voiceDisabled", false));

CVAR_UINTF  (cg_orderNotifyInterval,            2000,       CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_impactFeedback,                 true,       CVAR_SAVECONFIG);

CVAR_STRINGF(cg_victoryMusic,                   _T("/music/victory.ogg"),   CVAR_SAVECONFIG);
CVAR_STRINGF(cg_defeatMusic,                    _T("/music/defeat.ogg"),    CVAR_SAVECONFIG);

CVAR_BOOLF  (cg_firstPersonShadows,             false,      CVAR_SAVECONFIG);

CVAR_BOOL   (cg_replayServerStats,              false);

CVAR_BOOL   (cg_snapshotRateLimiter,            false);

UI_TRIGGER(Minimap);
UI_TRIGGER(CurrentOrder);
UI_TRIGGER(CurrentOfficerOrder);
UI_TRIGGER(GameMessage);
UI_TRIGGER(PetInfo);

UI_TRIGGER(LoadoutCost);
UI_TRIGGER(LoadoutTradeIn);
UI_TRIGGER(LoadoutNetCost);
UI_TRIGGER(LoadoutPurchase);
UI_TRIGGER(LoadoutSpawn);

UI_TRIGGER(LoadoutUnitBuilderAffordable);
UI_TRIGGER(LoadoutUnitScoutAffordable);
UI_TRIGGER(LoadoutUnitSavageAffordable);
UI_TRIGGER(LoadoutUnitLegionnaireAffordable);
UI_TRIGGER(LoadoutUnitChaplainAffordable);
UI_TRIGGER(LoadoutUnitBatteringramAffordable);
UI_TRIGGER(LoadoutUnitSteambuchetAffordable);
UI_TRIGGER(LoadoutUnitBuilderAvailable);
UI_TRIGGER(LoadoutUnitScoutAvailable);
UI_TRIGGER(LoadoutUnitSavageAvailable);
UI_TRIGGER(LoadoutUnitLegionnaireAvailable);
UI_TRIGGER(LoadoutUnitChaplainAvailable);
UI_TRIGGER(LoadoutUnitBatteringramAvailable);
UI_TRIGGER(LoadoutUnitSteambuchetAvailable);
UI_TRIGGER(LoadoutUnitBuilderCost);
UI_TRIGGER(LoadoutUnitScoutCost);
UI_TRIGGER(LoadoutUnitSavageCost);
UI_TRIGGER(LoadoutUnitLegionnaireCost);
UI_TRIGGER(LoadoutUnitChaplainCost);
UI_TRIGGER(LoadoutUnitBatteringramCost);
UI_TRIGGER(LoadoutUnitSteambuchetCost);
UI_TRIGGER(LoadoutUnitBuilderInService);
UI_TRIGGER(LoadoutUnitScoutInService);
UI_TRIGGER(LoadoutUnitSavageInService);
UI_TRIGGER(LoadoutUnitLegionnaireInService);
UI_TRIGGER(LoadoutUnitChaplainInService);
UI_TRIGGER(LoadoutUnitBatteringramInService);
UI_TRIGGER(LoadoutUnitSteambuchetInService);
UI_TRIGGER(LoadoutUnitBuilderInServiceSquad);
UI_TRIGGER(LoadoutUnitScoutInServiceSquad);
UI_TRIGGER(LoadoutUnitSavageInServiceSquad);
UI_TRIGGER(LoadoutUnitLegionnaireInServiceSquad);
UI_TRIGGER(LoadoutUnitChaplainInServiceSquad);
UI_TRIGGER(LoadoutUnitBatteringramInServiceSquad);
UI_TRIGGER(LoadoutUnitSteambuchetInServiceSquad);

UI_TRIGGER(LoadoutUnitConjurerAffordable);
UI_TRIGGER(LoadoutUnitShapeshifterAffordable);
UI_TRIGGER(LoadoutUnitSummonerAffordable);
UI_TRIGGER(LoadoutUnitPredatorAffordable);
UI_TRIGGER(LoadoutUnitShamanAffordable);
UI_TRIGGER(LoadoutUnitBehemothAffordable);
UI_TRIGGER(LoadoutUnitTempestAffordable);
UI_TRIGGER(LoadoutUnitConjurerAvailable);
UI_TRIGGER(LoadoutUnitShapeshifterAvailable);
UI_TRIGGER(LoadoutUnitSummonerAvailable);
UI_TRIGGER(LoadoutUnitPredatorAvailable);
UI_TRIGGER(LoadoutUnitShamanAvailable);
UI_TRIGGER(LoadoutUnitBehemothAvailable);
UI_TRIGGER(LoadoutUnitTempestAvailable);
UI_TRIGGER(LoadoutUnitConjurerCost);
UI_TRIGGER(LoadoutUnitShapeshifterCost);
UI_TRIGGER(LoadoutUnitSummonerCost);
UI_TRIGGER(LoadoutUnitPredatorCost);
UI_TRIGGER(LoadoutUnitShamanCost);
UI_TRIGGER(LoadoutUnitBehemothCost);
UI_TRIGGER(LoadoutUnitTempestCost);
UI_TRIGGER(LoadoutUnitConjurerInService);
UI_TRIGGER(LoadoutUnitShapeshifterInService);
UI_TRIGGER(LoadoutUnitSummonerInService);
UI_TRIGGER(LoadoutUnitPredatorInService);
UI_TRIGGER(LoadoutUnitShamanInService);
UI_TRIGGER(LoadoutUnitBehemothInService);
UI_TRIGGER(LoadoutUnitTempestInService);
UI_TRIGGER(LoadoutUnitConjurerInServiceSquad);
UI_TRIGGER(LoadoutUnitShapeshifterInServiceSquad);
UI_TRIGGER(LoadoutUnitSummonerInServiceSquad);
UI_TRIGGER(LoadoutUnitPredatorInServiceSquad);
UI_TRIGGER(LoadoutUnitShamanInServiceSquad);
UI_TRIGGER(LoadoutUnitBehemothInServiceSquad);
UI_TRIGGER(LoadoutUnitTempestInServiceSquad);

UI_TRIGGER(InfoTabSelect);
UI_TRIGGER(UpkeepEvent);

INPUT_STATE_BOOL(AltInfo);

const CVec4f OFFICER_COLOR(0.5f, 0.75f, 0.95f, 1.00f);
const CVec4f PET_COLOR(1.00f, 0.65f, 0.00f, 1.00f);
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

    SAFE_DELETE(m_pVoiceManager);
    SAFE_DELETE(m_pInterfaceManager);

    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        SAFE_DELETE(m_apFirstPersonEffectThread[i]);

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
    {
        if (m_ahFirstPersonSoundHandle[i] != INVALID_INDEX)
            K2SoundManager.StopHandle(m_ahFirstPersonSoundHandle[i]);
    }
    StopWorldSounds();
    K2SoundManager.StopMusic();

    if (m_hAltInfo != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_hAltInfo);

    for (vector<PoolHandle>::iterator it(m_vServerSnapshots.begin()); it != m_vServerSnapshots.end(); ++it)
        SAFE_DELETE_SNAPSHOT(*it);

    SAFE_DELETE_SNAPSHOT(m_hServerSnapshotFallback);

    if (m_hMinimapTexture != INVALID_RESOURCE)
        g_ResourceManager.Unregister(m_hMinimapTexture);
}


/*====================
  CGameClient::CGameClient
  ====================*/
CGameClient::CGameClient() :
m_pHostClient(NULL),
m_pClientEntityDirectory(NULL),
m_pInterfaceManager(NULL),
m_pCamera(K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CCamera)),

m_v3CameraPosition(V3_ZERO),
m_v3CameraAngles(V3_ZERO),
m_uiCameraFrame(-1),
m_uiCameraIndex(INVALID_INDEX),

m_vServerSnapshots(cg_serverSnapshotCacheSize, INVALID_POOL_HANDLE),
m_hServerSnapshotFallback(INVALID_POOL_HANDLE),
m_uiSnapshotBufferPos(0),

m_bShowInfoScreen(false),
m_bShowMenu(false),
m_bShowPurchase(false),
m_bShowLobby(false),
m_bShowBuildMenu(false),
m_hAltInfo(INVALID_RESOURCE),
m_pPreviewUnit(NULL),
m_eLastGamePhase(EGamePhase(-1)),
m_eLastInterface(CG_INTERFACE_NONE),

m_uiLastTeamTipTime(0),

m_pLocalClient(NULL),
m_pCurrentEntity(NULL),
m_pClientCommander(K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CClientCommander)),

m_iGameMatchID(-1),

m_hMinimapReference(INVALID_RESOURCE),
m_hMinimapTexture(INVALID_RESOURCE),
m_pMinimapBitmap(NULL), // UTTAR
m_hSnapSample(INVALID_RESOURCE),
m_hBuildingPlacedSample(INVALID_RESOURCE),
m_hBuildingCannotPlaceSample(INVALID_RESOURCE),
m_hRangedHitSample(INVALID_RESOURCE),
m_hRangedHitUnitSample(INVALID_RESOURCE),
m_hRangedHitBuildingSample(INVALID_RESOURCE),
m_uiLastHitSoundNotificationTime(0),
m_hMeleeHitEffect(INVALID_RESOURCE),
m_hRangedHitEffect(INVALID_RESOURCE),
m_hMeleeHitUnitEffect(INVALID_RESOURCE),
m_hRangedHitUnitEffect(INVALID_RESOURCE),
m_hMeleeHitBuildingEffect(INVALID_RESOURCE),
m_hRangedHitBuildingEffect(INVALID_RESOURCE),
m_hFogofWarCircle(INVALID_RESOURCE),
m_hKillSample(INVALID_RESOURCE),
m_hAssistSample(INVALID_RESOURCE),
m_hRazedSample(INVALID_RESOURCE),
m_hCommanderChatSample(INVALID_RESOURCE),
m_hItemPickupSample(INVALID_RESOURCE),
m_hSuddenDeathSample(INVALID_RESOURCE),

m_hWaypointCommanderMoveEffect(INVALID_RESOURCE),
m_hWaypointCommanderAttackEffect(INVALID_RESOURCE),
m_hWaypointOfficerMoveEffect(INVALID_RESOURCE),
m_hWaypointOfficerAttackEffect(INVALID_RESOURCE),

m_hBlockedFeedbackEffect(INVALID_RESOURCE),
m_hGotBlockedFeedbackEffect(INVALID_RESOURCE),
m_hInterruptedFeedbackEffect(INVALID_RESOURCE),
m_hGotInterruptedFeedbackEffect(INVALID_RESOURCE),
m_hMeleeHitFeedbackEffect(INVALID_RESOURCE),
m_hGotHitByMeleeFeedbackEffect(INVALID_RESOURCE),
m_hGotHitByMeleeFeedbackEffectNoKick(INVALID_RESOURCE),
m_hMeleeImpactFeedbackEffect(INVALID_RESOURCE),

m_uiLastOrderNotifyTime(0),

m_bForceMouseHidden(false),
m_bMouseHidden(false),
m_bForceMouseCentered(false),
m_bMouseCentered(false),

m_bAllowMouseAim(true),
m_bAllowMovement(true),
m_bAllowAttacks(true),

m_uiHoverEntity(INVALID_INDEX),
m_uiHoverEntityAcquiring(INVALID_INDEX),
m_uiHoverEntityAcquireTime(0),
m_uiHoverEntityDisplayTime(0),
m_v3CameraEffectAngleOffset(V3_ZERO),
m_v3CameraEffectOffset(V3_ZERO),
m_uiOrderEvent(INVALID_INDEX),
m_yOrderUniqueID(-1),

m_bValidPosition(false),
m_uiBuildFoundation(INVALID_INDEX),
m_pPreviewGadget(NULL),
m_bBuildingRotate(false),

m_bPinging(false),
m_bPingEffectActive(false),
m_uiLastPingTime(0),

m_bAutoRun(false),
m_uiLastForwardPress(INVALID_TIME),

m_pVoiceManager(NULL),
m_pSoundVoiceMicMuted(NULL),

m_bVCActive(false),
m_VCSubActive(BUTTON_INVALID),
m_uiNextVCTime(0),
m_uiVCUses(0),

m_iActiveFirstPersonAnim(-1),
m_yActiveFirstPersonAnimSequence(0),
m_hFirstPersonModelHandle(INVALID_RESOURCE),

m_bWasCommander(false),

m_hPropTypeStringTable(INVALID_RESOURCE)
{
    m_pInterfaceManager = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CGameInterfaceManager);
    m_pClientEntityDirectory = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CClientEntityDirectory);

    m_sSnap.uiNextAquire = 0;
    m_sSnap.uiLockedIndex = INVALID_INDEX;
    m_sSnap.v3CumulativeDelta.Clear();
    m_sSnap.v3OldAngles.Clear();

    for (int i(0); i < 8; ++i)
    {
        m_hUpkeepNotification[i] = INVALID_RESOURCE;
        m_hKillStreakNotification[i] = INVALID_RESOURCE;
    }

    for (int i(0); i < NUM_CLIENT_GAME_EFFECT_THREADS; ++i)
        m_apEffectThreads[i] = NULL;

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        m_ahSoundHandle[i] = INVALID_INDEX;

    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        m_apFirstPersonEffectThread[i] = NULL;
    }

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
        m_ahFirstPersonSoundHandle[i] = INVALID_INDEX;

    for (int i(0); i < MAX_PERSISTANT_ITEMS; ++i)
    {
        m_sItemVault.unItemType[i] = PERSISTANT_ITEM_NULL;
        m_sItemVault.uiItemID[i] = -1;
    }

    SeedRand(K2System.GetTicks() & UINT_MAX);
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
        if (m_pHostClient == NULL)
            EX_ERROR(_T("Invalid CHostClient"));

        // Setup IGame members
        SetWorldPointer(m_pHostClient->GetWorld());
        SetEntityDirectory(m_pClientEntityDirectory);
        Validate();

        // Create a camera
        if (m_pCamera == NULL)
            EX_ERROR(_T("Failed to allocate a CCamera"));
        m_pCamera->DefaultCamera(float(Vid.GetScreenW()), float(Vid.GetScreenH()));

        // Default position
        m_pCamera->SetOrigin(GetWorldWidth() / 2.0f, GetWorldHeight() / 2.0f, 250.0f);
        m_pCamera->SetAngles(0.0f, 0.0f, 0.0f);
        

        /*
        ActionRegistry.BindAxis(BINDTABLE_GAME_PLAYER,      AXIS_JOY_Z, BIND_MOD_NONE,          _T("CameraYaw"));
        ActionRegistry.BindAxis(BINDTABLE_GAME_PLAYER,      AXIS_JOY_U, BIND_MOD_NONE,          _T("CameraPitch"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK13, BIND_MOD_NONE,   _T("MoveForward"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK16, BIND_MOD_NONE0,  _T("MoveLeft"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK15, BIND_MOD_NONE0,  _T("MoveBack"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK14, BIND_MOD_NONE,   _T("MoveRight"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK3, BIND_MOD_NONE,    _T("Jump"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_JOYSTICK2, BIND_MOD_NONE,    _T("InvNext"));
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_JOYSTICK1, BIND_MOD_NONE,    _T("InvPrev"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK5, BIND_MOD_NONE,    _T("QuickAttack"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK6, BIND_MOD_NONE,    _T("StrongAttack"));
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_JOYSTICK8, BIND_MOD_NONE,    _T("Block"));
        */

        ActionRegistry.BindAxis(BINDTABLE_GAME_PLAYER,      AXIS_MOUSE_X, BIND_MOD_NONE,        _T("CameraYaw"),        _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindAxis(BINDTABLE_GAME_PLAYER,      AXIS_MOUSE_Y, BIND_MOD_NONE,        _T("CameraPitch"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('W'), BIND_MOD_NONE,        _T("MoveForward"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('A'), BIND_MOD_NONE,        _T("MoveLeft"),         _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('S'), BIND_MOD_NONE,        _T("MoveBack"),         _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('D'), BIND_MOD_NONE,        _T("MoveRight"),        _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_SHIFT, BIND_MOD_NONE,        _T("Sprint"),           _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_SPACE, BIND_MOD_NONE,        _T("Jump"),             _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_WHEELUP, BIND_MOD_NONE,      _T("InvNext"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_WHEELDOWN, BIND_MOD_NONE,    _T("InvPrev"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_MOUSEL, BIND_MOD_NONE,       _T("QuickAttack"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_MOUSER, BIND_MOD_NONE,       _T("StrongAttack"),     _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    BUTTON_MOUSEM, BIND_MOD_NONE,       _T("Block"),            _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('C'), BIND_MOD_NONE,        _T("Block"),            _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('E'), BIND_MOD_NONE,        _T("Use"),              _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_PLAYER,    EButton('R'), BIND_MOD_NONE,        _T("Repair"),           _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_F1, BIND_MOD_NONE,           _T("StartPing"),        _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('P'), BIND_MOD_NONE,        _T("StartPing"),        _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('Q'), BIND_MOD_NONE,        _T("AutoRun"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('G'), BIND_MOD_NONE,        _T("PetCommands"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('F'), BIND_MOD_NONE,        _T("OfficerCommands"),  _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('1'),   BIND_MOD_NONE,      _T("Inventory0"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('2'),   BIND_MOD_NONE,      _T("Inventory1"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('3'),   BIND_MOD_NONE,      _T("Inventory2"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('4'),   BIND_MOD_NONE,      _T("Inventory3"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('5'),   BIND_MOD_NONE,      _T("Inventory4"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('6'),   BIND_MOD_NONE,      _T("Inventory5"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('7'),   BIND_MOD_NONE,      _T("Inventory6"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('8'),   BIND_MOD_NONE,      _T("Inventory7"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('9'),   BIND_MOD_NONE,      _T("Inventory8"),       _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('0'),   BIND_MOD_NONE,      _T("Inventory9"),       _T(""), BIND_CONFIG, 0);

        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('1'),   BIND_MOD_ALT,       _T("Inventory10"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('2'),   BIND_MOD_ALT,       _T("Inventory11"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('3'),   BIND_MOD_ALT,       _T("Inventory12"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('4'),   BIND_MOD_ALT,       _T("Inventory13"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('5'),   BIND_MOD_ALT,       _T("Inventory14"),      _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   EButton('6'),   BIND_MOD_ALT,       _T("Inventory15"),      _T(""), BIND_CONFIG, 0);

        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM1,    BIND_MOD_NONE,      _T("Inventory10"),      _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM2,    BIND_MOD_NONE,      _T("Inventory11"),      _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM3,    BIND_MOD_NONE,      _T("Inventory12"),      _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM4,    BIND_MOD_NONE,      _T("Inventory13"),      _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM5,    BIND_MOD_NONE,      _T("Inventory14"),      _T(""), BIND_CONFIG, 1);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_PLAYER,   BUTTON_NUM6,    BIND_MOD_NONE,      _T("Inventory15"),      _T(""), BIND_CONFIG, 1);

        ActionRegistry.BindAxis(BINDTABLE_GAME_COMMANDER,       AXIS_MOUSE_X, BIND_MOD_NONE,        _T("DragScrollX"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindAxis(BINDTABLE_GAME_COMMANDER,       AXIS_MOUSE_Y, BIND_MOD_NONE,        _T("DragScrollY"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     EButton('W'), BIND_MOD_NONE,        _T("MoveForward"),          _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     EButton('A'), BIND_MOD_NONE,        _T("MoveLeft"),             _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     EButton('S'), BIND_MOD_NONE,        _T("MoveBack"),             _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     EButton('D'), BIND_MOD_NONE,        _T("MoveRight"),            _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    BUTTON_WHEELDOWN, BIND_MOD_NONE,    _T("ZoomOut"),              _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    BUTTON_WHEELUP, BIND_MOD_NONE,      _T("ZoomIn"),               _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_MOUSEL, BIND_MOD_NONE,       _T("CommanderPrimary"),     _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_MOUSER, BIND_MOD_NONE,       _T("CommanderSecondary"),   _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_MOUSEM, BIND_MOD_NONE,       _T("CommanderTertiary"),    _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_SHIFT, BIND_MOD_NONE,        _T("CommanderModifier1"),   _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_CTRL, BIND_MOD_NONE,         _T("CommanderModifier2"),   _T(""), BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('1'), BIND_MOD_NONE,        _T("InvSelect"),            _T("0"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('2'), BIND_MOD_NONE,        _T("InvSelect"),            _T("1"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('3'), BIND_MOD_NONE,        _T("InvSelect"),            _T("2"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('4'), BIND_MOD_NONE,        _T("InvSelect"),            _T("3"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('5'), BIND_MOD_NONE,        _T("InvSelect"),            _T("4"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('6'), BIND_MOD_NONE,        _T("InvSelect"),            _T("5"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('7'), BIND_MOD_NONE,        _T("InvSelect"),            _T("6"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('8'), BIND_MOD_NONE,        _T("InvSelect"),            _T("7"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('9'), BIND_MOD_NONE,        _T("InvSelect"),            _T("8"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME_COMMANDER,    EButton('0'), BIND_MOD_NONE,        _T("InvSelect"),            _T("9"),    BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_MISC5,   BIND_MOD_NONE,      _T("VoiceChat"),            _T(""),     BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME_COMMANDER,     BUTTON_MOUSEL, BIND_MOD_ALT,        _T("CommanderPing"),        _T(""),     BIND_CONFIG, 0);

        ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_ESC,     BIND_MOD_NONE,  _T("Cancel"),           _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  BUTTON_F10,     BIND_MOD_NONE,  _T("ToggleMenu"),       _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  BUTTON_TAB,     BIND_MOD_NONE,  _T("ToggleInfoScreen"), _T("scores"),       BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  EButton('T'),   BIND_MOD_NONE,  _T("ShowInfoScreen"),   _T("chat"),         BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  EButton('B'),   BIND_MOD_NONE,  _T("ShowInfoScreen"),   _T("tech"),         BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  EButton('H'),   BIND_MOD_NONE,  _T("ShowInfoScreen"),   _T("character"),    BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  EButton('L'),   BIND_MOD_NONE,  _T("ToggleLobby"),      _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME,   BUTTON_ALT,     BIND_MOD_NONE,  _T("AltInfo"),          _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  BUTTON_ENTER,   BIND_MOD_NONE,  _T("ChatTeam"),         _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  BUTTON_ENTER,   BIND_MOD_SHIFT, _T("ChatAll"),          _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  BUTTON_ENTER,   BIND_MOD_CTRL,  _T("ChatSquad"),        _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('Z'),   BIND_MOD_NONE,  _T("ShowChat"),         _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindImpulse(BINDTABLE_GAME,  EButton('V'),   BIND_MOD_NONE,  _T("VoiceCommands"),    _T(""),             BIND_CONFIG, 0);
        ActionRegistry.BindButton(BINDTABLE_GAME,   EButton('X'),   BIND_MOD_NONE,  _T("ToggleScoreOverlay"),   _T(""),             BIND_CONFIG, 0);

        m_hMinimapReference = g_ResourceManager.Register(_T("!minimap_texture"), RES_REFERENCE);
        g_ResourceManager.UpdateReference(m_hMinimapReference, g_ResourceManager.GetBlackTexture());

        if (!sound_voiceDisabled->GetValue())
            m_pVoiceManager = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CVoiceManager);
        
        m_bForceMouseHidden = false;
        m_bForceMouseCentered = false;
        m_bAllowMouseAim = true;
        m_bAllowMovement = true;
        m_bAllowAttacks = true;

        m_bShowInfoScreen = false;
        m_bShowLobby = false;
        m_bShowMenu = false;
        m_bShowPurchase = false;
        m_bShowBuildMenu = false;
        
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
    m_v3CameraPosition = V3_ZERO;
    m_v3CameraAngles = V3_ZERO;
    m_uiCameraFrame = -1;
    m_uiCameraIndex = INVALID_INDEX;

    m_bShowInfoScreen = false;
    m_bShowMenu = false;
    m_bShowPurchase = false;
    m_bShowLobby = false;
    m_bShowBuildMenu = false;

    m_bAutoRun = false;
    m_uiLastForwardPress = INVALID_TIME;

    m_pLocalClient = NULL;
    m_pCurrentEntity = NULL;

    m_uiLastHitSoundNotificationTime = 0;

    m_uiHoverEntityAcquireTime = 0;
    m_uiHoverEntityDisplayTime = 0;
    m_uiHoverEntity = INVALID_INDEX;
    m_uiHoverEntityAcquiring = INVALID_INDEX;
    m_v3CameraEffectAngleOffset = V3_ZERO;
    m_v3CameraEffectOffset = V3_ZERO;
    m_uiOrderEvent = INVALID_INDEX;

    m_bValidPosition = false;
    m_uiBuildFoundation = INVALID_INDEX;

    m_pPreviewUnit = NULL;

    m_bPinging = false;
    m_bPingEffectActive = false;

    m_uiLastTeamTipTime = 0;
    m_uiLastPingTime = 0;

    m_deqClientSnapshots.clear();

    m_sSnap.uiNextAquire = 0;
    m_sSnap.uiLockedIndex = INVALID_INDEX;
    m_sSnap.v3CumulativeDelta.Clear();
    m_sSnap.v3OldAngles.Clear();

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

    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        SAFE_DELETE(m_apFirstPersonEffectThread[i]);
    }

    for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
    {
        if (m_ahFirstPersonSoundHandle[i] != INVALID_INDEX)
        {
            K2SoundManager.StopHandle(m_ahFirstPersonSoundHandle[i]);
            m_ahFirstPersonSoundHandle[i] = INVALID_INDEX;
        }
    }

    StopWorldSounds();
    K2SoundManager.StopMusic();

    ClearTeams();

    SAFE_DELETE(m_pVoiceManager);
    if (!sound_voiceDisabled->GetValue())
        m_pVoiceManager = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CVoiceManager);

    m_bWasCommander = false;

    SetWinningTeam(-1);

    m_mapClients.clear();

    m_vFirstPersonEffects.clear();
}


/*====================
  CGameClient::LoadWorld
  ====================*/
bool    CGameClient::LoadWorld()
{
    PROFILE_EX("CGameClient::LoadWorld", PROFILE_GAME_LOAD_WORLD);

    try
    {
        if (!IsWorldLoaded())
            EX_ERROR(_T("Host has not loaded the world yet"));

        // Register global scripts
        m_mapScripts.clear();
        m_mapScripts = GetWorldScriptMap();
        
        m_pCamera->SetWorld(GetWorldPointer());

        class CCreateEntitiesFunctions : public CLoadJob<WorldEntMap, pair<uint, CWorldEntity*> >::IFunctions
        {
        private:
            CClientEntityDirectory* m_pEntityDirectory;

        public:
            CCreateEntitiesFunctions(CClientEntityDirectory *pEntityDirectory) : m_pEntityDirectory(pEntityDirectory)   {}

            void    Advance(WorldEntMap_it &it) {}
            float   Frame(WorldEntMap_it &it, float f) const
            {
                CWorldEntity *pWorldEntity(GameClient.GetWorldEntity(it->first));
                if (!pWorldEntity)
                    EX_ERROR(_T("Failed world entity lookup on #") + XtoA(it->first));

                SetTitle(_T("Creating entities"));
                SetDescription(pWorldEntity->GetType());
                SetProgress(f);

                return 0.0f;
            }

            float   PostFrame(WorldEntMap_it &it, float f) const
            {
                CWorldEntity *pWorldEntity(GameClient.GetWorldEntity(it->first));
                if (!pWorldEntity)
                    EX_ERROR(_T("Failed world entity lookup on #") + XtoA(it->first));

                ushort uiID(EntityRegistry.LookupID(pWorldEntity->GetType()));
                if (uiID == ushort(-1))
                {
                    ++it;
                    return 1.0f;
                }

                IGameEntity* pNew(m_pEntityDirectory->AllocateLocal(uiID));
                if (pNew == NULL)
                    EX_ERROR(_T("Failed to allocate a client-side game entity for world entity #") + XtoA(it->first));

                CClientEntity* pNewClientEnt(m_pEntityDirectory->GetClientEntity(pNew->GetIndex()));
                if (pNewClientEnt == NULL)
                    EX_ERROR(_T("Failed to allocate a client game entity for world entity #") + XtoA(it->first));

                IVisualEntity *pNewEnt(pNewClientEnt->GetNextEntity());
                if (!pNewEnt->IsStatic())
                {
                    STL_ERASE((*m_pList), it);
                    m_pEntityDirectory->Delete(pNewEnt->GetIndex());
                    return 1.0f;
                }
                
                pWorldEntity->SetGameIndex(it->first);
                
                pNewEnt->ApplyWorldEntity(*pWorldEntity);
                pNewEnt->Spawn();

                Console << _T("Spawned new entity #") << pNewEnt->GetIndex() << _T(" (") + EntityRegistry.LookupName(uiID) + _T(") @ ") << pNewEnt->GetAsVisualEnt()->GetPosition()
                            << SPACE << pNewEnt->GetAsVisualEnt()->GetAngles() << newl;

                pNewEnt->Validate();

                // Setup previous and current states
                pNewClientEnt->CopyNextToPrev();
                pNewClientEnt->CopyNextToCurrent();
                pNewClientEnt->Interpolate(1.0f);
                ++it;

                return 1.0f;
            }
        };
        CCreateEntitiesFunctions fnCreateEntities(m_pClientEntityDirectory);
        CLoadJob<WorldEntMap, pair<uint, CWorldEntity*> > jobCreateEntities(GetWorldEntityMap(), &fnCreateEntities, false);
        jobCreateEntities.Execute(GetWorldEntityMap().size());

        // Spawn lights
        WorldLightsMap &mapWorldLights(GetWorldLightsMap());
        for (WorldLightsMap_it it(mapWorldLights.begin()); it != mapWorldLights.end();)
        {
            ushort uiID(Light_Static);
            if (uiID == ushort(-1))
            {
                ++it;
                continue;
            }

            IGameEntity* pNew(m_pClientEntityDirectory->AllocateLocal(uiID));

            CClientEntity* pNewClientEnt(m_pClientEntityDirectory->GetClientEntity(pNew->GetIndex()));
            if (pNewClientEnt == NULL)
                EX_ERROR(_T("Failed to allocate a light for world light #") + XtoA(it->first));

            IGameEntity *pNewEnt(pNewClientEnt->GetNextEntity());

            if (!pNewEnt->IsStatic())
            {
                STL_ERASE(mapWorldLights, it);
                m_pClientEntityDirectory->Delete(pNewEnt->GetIndex());
                continue;
            }

            ILight *pLight(pNewEnt->GetAsLight());
            if (pLight == NULL)
                EX_ERROR(_T("Allocated game entity is not the correct type"));

            pLight->SetWorldIndex(it->second->GetIndex());
            pLight->Spawn();
            Console.ServerGame << _T("Spawned new light #") << pLight->GetIndex() << _T(" @ ") << pLight->GetPosition() << newl;

            pLight->Validate();

            pNewClientEnt->CopyNextToPrev();
            pNewClientEnt->CopyNextToCurrent();
            pNewClientEnt->Interpolate(1.0f);
            ++it;
        }

        // World sounds
        WorldSoundsMap  &mapWorldSounds(GetWorldSoundsMap());
        StopWorldSounds();
        for (WorldSoundsMap_it it(mapWorldSounds.begin()); it != mapWorldSounds.end(); it++)
        {
            ResHandle hSample(g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(it->second->GetSound(), 0), RES_SAMPLE));
            if (hSample == INVALID_INDEX)
                continue;

            m_mapWorldSounds[it->first] = SWorldSound(INVALID_INDEX, hSample);
        }
        ResHandle hSample(g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(cg_worldAmbientSound, SND_2D), RES_SAMPLE));
        if (hSample != INVALID_INDEX)
            m_hWorldAmbientSound = K2SoundManager.Play2DSFXSound(hSample, cg_worldAmbientSoundVolume, -1, 100, true);

        K2SoundManager.MuteSFX(true);

        UpdateMinimapTexture();

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGameClient::LoadWorld() - "), THROW);
        return false;
    }
}


/*====================
  CGameClient::LoadResources
  ====================*/
bool    CGameClient::LoadResources()
{
    PrecacheEntities();

    // Load client game resources
    XMLManager.Process(_T("/game_resources.xml"));

    // client sounds stringtable
    ResHandle hClientSounds(g_ResourceManager.Register(_T("/stringtables/ClientSounds.str"), RES_STRINGTABLE));
    CStringTable *pClientSounds(g_ResourceManager.GetStringTable(hClientSounds));

    if (pClientSounds)
    {
        m_hCommanderChatSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("commander_chat")), SND_2D), RES_SAMPLE);

        m_hSnapSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("snap")), SND_2D), RES_SAMPLE);

        m_hBuildingPlacedSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("building_placed")), SND_2D), RES_SAMPLE);
        m_hBuildingCannotPlaceSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("cannot_place_building")), SND_2D), RES_SAMPLE);

        m_hSpawnportalPlacedSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("spawnportal_placed_human")), SND_2D), RES_SAMPLE);
        m_hSpawnportalPlacedSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("spawnportal_placed_beast")), SND_2D), RES_SAMPLE);

        m_hMeleeHitUnitSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("melee_hit_unit")), SND_2D), RES_SAMPLE);
        m_hMeleeHitBuildingSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("melee_hit_building")), SND_2D), RES_SAMPLE);
        m_hRangedHitSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("ranged_hit")), SND_2D), RES_SAMPLE);
        m_hRangedHitUnitSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("ranged_hit_unit")), SND_2D), RES_SAMPLE);
        m_hRangedHitBuildingSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("ranged_hit_building")), SND_2D), RES_SAMPLE);
    }
    
    m_hMeleeHitEffect = g_ResourceManager.Register(_T("/shared/effects/hit_melee.effect"), RES_EFFECT);
    m_hRangedHitEffect = g_ResourceManager.Register(_T("/shared/effects/hit_ranged.effect"), RES_EFFECT);
    m_hMeleeHitUnitEffect = g_ResourceManager.Register(_T("/shared/effects/hit_melee_unit.effect"), RES_EFFECT);
    m_hRangedHitUnitEffect = g_ResourceManager.Register(_T("/shared/effects/hit_ranged_unit.effect"), RES_EFFECT);
    m_hMeleeHitBuildingEffect = g_ResourceManager.Register(_T("/shared/effects/hit_melee_building.effect"), RES_EFFECT);
    m_hRangedHitBuildingEffect = g_ResourceManager.Register(_T("/shared/effects/hit_ranged_building.effect"), RES_EFFECT);

    m_hWaypointCommanderMoveEffect = g_ResourceManager.Register(_T("/shared/effects/waypoint.effect"), RES_EFFECT);
    m_hWaypointCommanderAttackEffect = g_ResourceManager.Register(_T("/shared/effects/attack_waypoint.effect"), RES_EFFECT);
    m_hWaypointOfficerMoveEffect = g_ResourceManager.Register(_T("/shared/effects/officer_waypoint.effect"), RES_EFFECT);
    m_hWaypointOfficerAttackEffect = g_ResourceManager.Register(_T("/shared/effects/officer_attack_waypoint.effect"), RES_EFFECT);

    if (pClientSounds)
    {
        // 0 = human, 1 = beast
        m_hOrderMoveSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_move")), SND_2D), RES_SAMPLE);
        m_hOrderMoveSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_move")), SND_2D), RES_SAMPLE);
        m_hOrderAttackSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_attack")), SND_2D), RES_SAMPLE);
        m_hOrderAttackSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_attack")), SND_2D), RES_SAMPLE);
        m_hOrderAttackUnitSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_attack_unit")), SND_2D), RES_SAMPLE);
        m_hOrderAttackUnitSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_attack_unit")), SND_2D), RES_SAMPLE);
        m_hOrderAttackBuildingSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_attack_building")), SND_2D), RES_SAMPLE);
        m_hOrderAttackBuildingSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_attack_building")), SND_2D), RES_SAMPLE);
        m_hOrderDefendBuildingSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_defend_building")), SND_2D), RES_SAMPLE);
        m_hOrderDefendBuildingSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_defend_building")), SND_2D), RES_SAMPLE);
        m_hOrderRepairBuildingSample[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_human_repair_building")), SND_2D), RES_SAMPLE);
        m_hOrderRepairBuildingSample[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("order_beast_repair_building")), SND_2D), RES_SAMPLE);
        
        m_hPingSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("minimap_ping")), SND_2D), RES_SAMPLE);
        m_hBuildingAttackedSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("building_under_attack")), SND_2D | SND_STREAM), RES_SAMPLE);
        m_hGoldMineLowSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("gold_mine_low")), SND_2D | SND_STREAM), RES_SAMPLE);
        m_hGoldMineDepletedSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("gold_mine_depleted")), SND_2D | SND_STREAM), RES_SAMPLE);
        m_hItemPickupSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("item_pickup")), SND_2D), RES_SAMPLE);
        m_hSuddenDeathSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("sudden_death")), SND_2D), RES_SAMPLE);
        
        // bit 3 set = global
        m_hKillStreakNotification[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak3")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak5")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[2] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak7")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[3] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak10")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[4] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak3_global")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[5] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak5_global")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[6] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak7_global")), SND_2D), RES_SAMPLE);
        m_hKillStreakNotification[7] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("killstreak10_global")), SND_2D), RES_SAMPLE);
    }

    m_hAltInfo = g_ResourceManager.Register(_T("/ui/alt_info.xml"), RES_INTERFACE);

    if (pClientSounds)
    {
        m_hKillSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("get_kill")), SND_2D), RES_SAMPLE);
        m_hAssistSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("get_assist")), SND_2D), RES_SAMPLE);
        m_hRazedSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("get_raze")), SND_2D), RES_SAMPLE);

        // Interleave these for easy access
        // bit 0: human/beast
        // bit 1: restored/failed
        // bit 2: ally/enemy
        m_hUpkeepNotification[0] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_failed_human")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[1] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_failed_beast")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[2] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_restored_human")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[3] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_restored_beast")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[4] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_enemy_failed_human")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[5] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_enemy_failed_beast")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[6] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_enemy_restored_human")), SND_2D), RES_SAMPLE);
        m_hUpkeepNotification[7] = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(pClientSounds->Get(_T("upkeep_enemy_restored_beast")), SND_2D), RES_SAMPLE);
    }

    m_hBlockedFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_blocked.effect"), RES_EFFECT);
    m_hGotBlockedFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_gotblocked.effect"), RES_EFFECT);
    m_hInterruptedFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_interrupted.effect"), RES_EFFECT);
    m_hGotInterruptedFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_gotinterrupted.effect"), RES_EFFECT);
    m_hMeleeHitFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_meleehit.effect"), RES_EFFECT);
    m_hGotHitByMeleeFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_gothitbymelee.effect"), RES_EFFECT);
    m_hGotHitByMeleeFeedbackEffectNoKick = g_ResourceManager.Register(_T("/shared/effects/feedback_gothitbymelee_nokick.effect"), RES_EFFECT);
    m_hMeleeImpactFeedbackEffect = g_ResourceManager.Register(_T("/shared/effects/feedback_melee_impact.effect"), RES_EFFECT);

    m_hPropTypeStringTable = g_ResourceManager.Register(_T("/world/props/PropTypes.str"), RES_STRINGTABLE);

    m_pClientCommander->LoadResources(hClientSounds);

    m_pInterfaceManager->Init();

    // Loading an interface no longer shows it by default, so we have to set it to visible
    CInterface *pInterface(UIManager.GetInterface(m_hAltInfo));
    if (pInterface != NULL)
        pInterface->Show();

    m_hLittleTextPopupFont = g_ResourceManager.LookUpName(_T("littletextpopup"), RES_FONTMAP);
    m_hLocatorFont = g_ResourceManager.LookUpName(_T("locator"), RES_FONTMAP);

    m_hMinimapTriangle = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/triangle.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_hOfficerLocator = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/officer_locator.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    m_hWaypointLocator = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/waypoint_locator.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    m_hPetLocator = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/officer_locator.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_hFogofWarCircle = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/core/textures/fogofwar.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_hPingTexture = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/ping_alpha.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    return true;
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
  CGameClient::Frame
  ====================*/
void    CGameClient::Frame()
{
    PROFILE("CGameClient::Frame");

    cg_gameTime =
        XtoA(INT_FLOOR(MsToMin(GetCurrentGameLength())), FMT_PADZERO, 2) +
        _T(":") +
        XtoA(INT_FLOOR(MsToSec(GetCurrentGameLength() % 60000)), FMT_PADZERO, 2);

    Host.AverageFPS(false);

    if (m_pHostClient->GetIndex() != Host.GetActiveClientIndex())
    {
        BackgroundFrame();
        return;
    }

    if (m_pHostClient->GetState() < CLIENT_STATE_IN_GAME)
    {
        BackgroundFrame();
        m_pInterfaceManager->Update();
        return;
    }

    if (K2System.Milliseconds() - m_uiLastUpdateCheck >= 300000)
    {
        m_pHostClient->SilentUpdate();
        m_uiLastUpdateCheck = K2System.Milliseconds();
    }

    CheckVoiceCvars();
    if (m_pVoiceManager != NULL)
        m_pVoiceManager->Frame();

    ClearTransparentEntities();

    switch (GetGamePhase())
    {
    case GAME_PHASE_WAITING_FOR_PLAYERS:
    case GAME_PHASE_SELECTING_COMMANDER:
    case GAME_PHASE_SELECTING_OFFICERS:
    case GAME_PHASE_FORMING_SQUADS:
        SetupFrame();
        break;

    case GAME_PHASE_ACTIVE:
        if (m_eLastGamePhase != GetGamePhase() && !GetWorldPointer()->GetMusicList().empty())
            K2SoundManager.PlayPlaylist(GetWorldPointer()->GetMusicList());

    case GAME_PHASE_WARMUP:
        ActiveFrame();
        break;

    case GAME_PHASE_ENDED:
        EndedFrame();
        break;

    case GAME_PHASE_STANDBY:
        SceneManager.Clear();
        SceneManager.ClearBackground();
        m_pInterfaceManager->Update();
        break;
    }

    m_eLastGamePhase = GetGamePhase();
}


/*====================
  CGameClient::PrecacheAll
  ====================*/
void    CGameClient::PrecacheAll()
{
    bool bPrecache(cg_precacheEntities);
    cg_precacheEntities = true;
    LoadResources();
    cg_precacheEntities = bPrecache;
}


/*====================
  CGameClient::SetupFrame
  ====================*/
void    CGameClient::SetupFrame()
{
    PROFILE("CGameClient::SetupFrame");

    Draw2D.SetColor(BLACK);
    Draw2D.Clear();

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    UpdateVision();

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(m_pHostClient->GetLerpValue());

    m_pInterfaceManager->Update();

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);

        m_bVCActive = false;
        m_VCSubActive = BUTTON_INVALID;
    }

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    // Generate a new client snapshot
    if (m_pHostClient->GetServerFrame() > 1)
        SendClientSnapshot();
    else
        m_pHostClient->SendLoadingHeartbeat();
    
    UpdateMinimap();
    
    Vid.RenderFogofWar(1.0f);
}


/*====================
  CGameClient::InterfaceNeedsUpdate
  ====================*/
bool    CGameClient::InterfaceNeedsUpdate()
{
    bool bUpdate(UIManager.NeedsRefresh());

    UIManager.ResetRefresh();

    return bUpdate;
}


/*====================
  CGameClient::GetCurrentInterface
  ====================*/
EGameInterface  CGameClient::GetCurrentInterface() const
{
    if (IModalDialog::IsActive())
        return CG_INTERFACE_MESSAGE;

    if (m_pHostClient->GetState() == CLIENT_STATE_IDLE)
        return CG_INTERFACE_NONE;

    if (GetGamePhase() == GAME_PHASE_STANDBY)
        return CG_INTERFACE_STANDBY;

    if (m_pHostClient->GetState() < CLIENT_STATE_IN_GAME)
        return CG_INTERFACE_NONE;

    if (GetGamePhase() < GAME_PHASE_ACTIVE)
        return CG_INTERFACE_LOBBY;

    if (m_pLocalClient == NULL)
        return CG_INTERFACE_NONE;

    if (GetGamePhase() == GAME_PHASE_ENDED)
        return CG_INTERFACE_GAME_OVER;

    if (m_bShowLobby)
        return CG_INTERFACE_LOBBY;

    IPlayerEntity *pLocalPlayer(m_pLocalClient->GetPlayerEntity());

    if (m_pLocalClient->GetSquad() == INVALID_SQUAD && !IsCommander() && (pLocalPlayer == NULL || pLocalPlayer->GetTeam() != 0))
        return CG_INTERFACE_LOBBY;

    if (m_pLocalClient->GetTeam() == 0 && (pLocalPlayer == NULL || !pLocalPlayer->IsObserver() || pLocalPlayer->GetStatus() == ENTITY_STATUS_DORMANT))
        return CG_INTERFACE_LOBBY;

    if (m_bShowMenu)
        return CG_INTERFACE_MENU;

    if (m_bShowPurchase)
        return CG_INTERFACE_PURCHASE;

    if (m_pLocalClient->GetTeam() != 0)
    {
        if (pLocalPlayer != NULL && pLocalPlayer->HasNetFlags(ENT_NET_FLAG_SACRIFICE_MENU))
            return CG_INTERFACE_SACRIFICED;

        if (pLocalPlayer == NULL || pLocalPlayer->IsObserver() || pLocalPlayer->GetStatus() == ENTITY_STATUS_DORMANT)
            return CG_INTERFACE_LOADOUT;

        if (pLocalPlayer != NULL && pLocalPlayer->GetStatus() == ENTITY_STATUS_SPAWNING)
            return CG_INTERFACE_SPAWN;

        if (m_bShowInfoScreen)
        {
            if (IsCommander())
                return CG_INTERFACE_COMMANDER_INFO;
            else
                return CG_INTERFACE_PLAYER_INFO;
        }

        if (pLocalPlayer != NULL && pLocalPlayer->GetStatus() == ENTITY_STATUS_DEAD)
            return CG_INTERFACE_DEAD;

        if (pLocalPlayer != NULL && pLocalPlayer->HasNetFlags(ENT_NET_FLAG_BUILD_MODE))
            return CG_INTERFACE_PLAYER_BUILD;

        if (IsCommander())
            return CG_INTERFACE_COMMANDER;

        return CG_INTERFACE_PLAYER;
    }

    if (pLocalPlayer != NULL && pLocalPlayer->IsObserver())
        return CG_INTERFACE_OBSERVER;

    return CG_INTERFACE_LOBBY;
}


/*====================
  CGameClient::ToggleInfoScreen
  ====================*/
bool    CGameClient::ToggleInfoScreen(const tstring &sTab)
{
    EGameInterface eInterface(GetCurrentInterface());

    if (eInterface != CG_INTERFACE_PLAYER &&
        eInterface != CG_INTERFACE_COMMANDER &&
        eInterface != CG_INTERFACE_OBSERVER &&
        eInterface != CG_INTERFACE_COMMANDER_INFO &&
        eInterface != CG_INTERFACE_PLAYER_INFO &&
        eInterface != CG_INTERFACE_DEAD &&
        eInterface != CG_INTERFACE_PLAYER_BUILD)
        return false;

    m_bShowInfoScreen = !m_bShowInfoScreen;
    if (m_bShowInfoScreen && !sTab.empty())
        InfoTabSelect.Trigger(sTab, true);

    return true;
}


/*====================
  CGameClient::ShowInfoScreen
  ====================*/
void    CGameClient::ShowInfoScreen(const tstring &sTab)
{
    m_bShowInfoScreen = true;
    InfoTabSelect.Trigger(sTab, true);
}


/*====================
  CGameClient::DrawMeleeTraces
  ====================*/
void    CGameClient::DrawMeleeTraces()
{
    if (cg_drawMeleeTraces == 0)
        return;

    if (m_pLocalClient == NULL)
        return;
    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;

    bool bStrongAttack(false);
    int iSlot(cg_drawMeleeTraces);
    if (iSlot < 0)
    {
        iSlot = -iSlot;
        bStrongAttack = true;
    }
    iSlot -= 1;

    IInventoryItem *pItem(pPlayer->GetItem(iSlot));
    if (pItem == NULL)
        return;

    CMeleeAttackEvent   attack;

    if (pItem->IsMelee())
    {
#define SETUP_ATTACK(type) \
{ \
    attack.SetPivot(pMelee->Get##type##AttackPivotHeight(), pMelee->Get##type##AttackPivotFactor()); \
    attack.SetMetric(MELEE_HEIGHT, pMelee->Get##type##AttackHeightMin(), pMelee->Get##type##AttackHeightMax(), pMelee->Get##type##AttackHeightStep()); \
    attack.SetMetric(MELEE_ANGLE, pMelee->Get##type##AttackAngleMin(), pMelee->Get##type##AttackAngleMax(), pMelee->Get##type##AttackAngleStep()); \
    attack.SetMetric(MELEE_RANGE, pMelee->Get##type##AttackRangeMin(), pMelee->Get##type##AttackRangeMax(), pMelee->Get##type##AttackRangeStep()); \
}
        IMeleeItem *pMelee(pItem->GetAsMelee());
        if (bStrongAttack)
            SETUP_ATTACK(Strong)
        else
            SETUP_ATTACK(Quick)
#undef SETUP_ATTACK
    }
    else if (pItem->IsSkill())
    {
        ISkillItem *pSkill(pItem->GetAsSkill());
        if (!pSkill->IsMeleeSkill())
            return;
        ISkillMelee *pMelee(static_cast<ISkillMelee*>(pSkill));
        attack.SetMetric(MELEE_HEIGHT, pMelee->GetMinHeight(), pMelee->GetMaxHeight(), pMelee->GetHeightStep());
        attack.SetMetric(MELEE_ANGLE, pMelee->GetMinAngle(), pMelee->GetMaxAngle(), pMelee->GetAngleStep());
        attack.SetMetric(MELEE_RANGE, pMelee->GetMinRange(), pMelee->GetMaxRange(), pMelee->GetRangeStep());
    }
    else
    {
        return;
    }

    CAxis axis(pPlayer->GetAngles());
    int iColor(0);

    // Step through each layer of the attack
    int iTraceCount(0);
    for (float fHeight(attack.GetMin(MELEE_HEIGHT)); fHeight < attack.GetMax(MELEE_HEIGHT); fHeight += attack.GetStep(MELEE_HEIGHT))
    {
        // Find the center point along the "up" axis
        CVec3f v3Center(attack.GetCenter(pPlayer->GetPosition(), axis, fHeight));

        // Step through each slice of the arc
        for (float fAngle(attack.GetMin(MELEE_ANGLE)); fAngle < attack.GetMax(MELEE_ANGLE); fAngle += attack.GetStep(MELEE_ANGLE))
        {
            // Determine next point in the arc
            float fNextAngle(MIN(fAngle + attack.GetStep(MELEE_ANGLE), attack.GetMax(MELEE_ANGLE)));
            
            // Determine the direction of each angle
            CVec3f v3DirA(attack.GetDir(axis, fAngle));
            CVec3f v3DirB(attack.GetDir(axis, fNextAngle));

            // Step to the maximum range of the attack
            for (float fDist(attack.GetMin(MELEE_RANGE)); fDist < attack.GetMax(MELEE_RANGE); fDist += attack.GetStep(MELEE_RANGE))
            {
                CVec3f v3Start(v3Center + v3DirA * fDist);
                CVec3f v3End(v3Center + v3DirB * fDist);
                
                Vid.AddLine(v3Center, v3Start, g_v4Colors[iColor % g_zNumColors]);
                Vid.AddLine(v3Center, v3End, g_v4Colors[iColor % g_zNumColors]);
                Vid.AddLine(v3Start, v3End, g_v4Colors[iColor % g_zNumColors]);
                ++iTraceCount;
            }
        }
        ++iColor;
    }

    Draw2D.SetColor(WHITE);
    Draw2D.String(Draw2D.GetScreenW() - 200.0f, 20.0f, _T("Total traces: ") + XtoA(iTraceCount), m_hLittleTextPopupFont);
}


/*====================
  CGameClient::ActiveFrame
  ====================*/
void    CGameClient::ActiveFrame()
{
    PROFILE("CGameClient::ActiveFrame");

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    if (m_pHostClient->GetState() != CLIENT_STATE_IN_GAME || m_pLocalClient == NULL)
    {
        // This is very bad if it ever happens...
        if (m_pHostClient->GetState() != CLIENT_STATE_IN_GAME)
            Console.Err << _T("Client state invalid! State: ") << XtoA(m_pHostClient->GetState()) << newl;

        if (m_pLocalClient == NULL)
            Console.Err << _T("Local client invalid!") << newl;

        Draw2D.SetColor(BLACK);
        Draw2D.Clear();
        return;
    }

    m_v3CameraEffectAngleOffset.Clear(); 
    m_v3CameraEffectOffset.Clear();

    UpdateVision();

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(m_pHostClient->GetLerpValue());

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());

    // Screen fade
    if (pPlayer != NULL && pPlayer->GetStatus() == ENTITY_STATUS_DEAD)
        GameClient.AddOverlay(CVec4f(0.0f, 0.0f, 0.0f, pPlayer->GetDeathPercent()));

    m_pClientCommander->SetCamera(m_pCamera);
    m_pClientCommander->SetPlayerCommander(GetLocalPlayer() == NULL ? NULL : GetLocalPlayer()->GetAsCommander());
    m_pClientCommander->SetCursorPos(Input.GetCursorPos());

    if (!m_bWasCommander && IsCommander())
    {
        m_pClientCommander->Spawn();
        m_bWasCommander = true;
    }

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);

        m_bVCActive = false;
        m_VCSubActive = BUTTON_INVALID;
    }

    // Voice command binds must be run FIRST so BINDTABLE_GAME does not catch the inventory binds
    if (IsVCSubMenuActive())
        Input.ExecuteBinds(BINDTABLE_GAME_VOICECOMMAND_SUB, 0);
    else if (IsVCMenuActive())
        Input.ExecuteBinds(BINDTABLE_GAME_VOICECOMMAND, 0);

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    bool bMuteSFX(false);
    switch (GetCurrentInterface())
    {
    case CG_INTERFACE_LOBBY:
    case CG_INTERFACE_LOADOUT:
    case CG_INTERFACE_SPAWN:
        bMuteSFX = true;
    case CG_INTERFACE_PLAYER_INFO:
    case CG_INTERFACE_MENU:
    case CG_INTERFACE_SACRIFICED:
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);
        m_CurrentClientSnapshot.SetCursorPosition(V2_ZERO);
        break;

    case CG_INTERFACE_DEAD:
        Input.ExecuteBinds(BINDTABLE_GAME_PLAYER, 0);
        m_CurrentClientSnapshot.AdjustCameraAngles(V_ZERO); // HACK: Reset the roll
        m_CurrentClientSnapshot.SetCursorPosition(V2_ZERO);
        break;

    case CG_INTERFACE_PLAYER_BUILD:
    case CG_INTERFACE_PLAYER_OFFICER:
        {
            Input.ExecuteBinds(BINDTABLE_GAME_COMMANDER, 0);

            CVec2f v2Cursor(Input.GetCursorPos());

            // Only rotate if they are holding down the middle mouse button
            if (m_CurrentClientSnapshot.GetButtonStatus(GAME_CMDR_BUTTON_DRAGSCROLL) == GAME_BUTTON_STATUS_DOWN)
            {
                // Calculate the distance the cursor is from the 90 degree angle in front of the player
                float fRotX(v2Cursor.x - (Vid.GetScreenW() / 2));
                float fRotY(v2Cursor.y - Vid.GetScreenH());

                // Calculate rotation speed based on distance from 90 degree angle and frame length (to keep rotation speed semi-consistant)
                float fRotation((fRotY + fabs(fRotX)) * (GetFrameLength() / 5000.0f));

                // If it's past the 90 degree line, rotate the camera
                if (fRotation > 0)
                    m_CurrentClientSnapshot.AdjustCameraYaw(fRotation * (fRotX < 0 ? -1 : 1));
            }

            v2Cursor.x /= Vid.GetScreenW();
            v2Cursor.y /= Vid.GetScreenH();
            m_CurrentClientSnapshot.SetCursorPosition(v2Cursor);
        }
        break;

    case CG_INTERFACE_PLAYER:
    case CG_INTERFACE_OBSERVER:
        Input.ExecuteBinds(BINDTABLE_GAME_PLAYER, 0);
        m_CurrentClientSnapshot.AdjustCameraAngles(V_ZERO); // HACK: Reset the roll
        m_CurrentClientSnapshot.SetCursorPosition(V2_ZERO);
        Snapcast();
        break;

    case CG_INTERFACE_COMMANDER:
        {
            Input.ExecuteBinds(BINDTABLE_GAME_COMMANDER, 0);
            if (!IsCommander())
                break;

            m_pClientCommander->PrepareClientSnapshot(m_CurrentClientSnapshot);
            CommanderSnapcast();
        }
        break;
    }

    // Sound
    WorldSoundsFrame();
    K2SoundManager.MuteSFX(bMuteSFX);

    // Remove any input we used above but didn't discard
    Input.FlushByTable(BINDTABLE_GAME, 0);

    IPlayerEntity *pPredictedPlayer(NULL);

    IPlayerEntity *pLocalPlayerNext(GetLocalPlayerNext());
    IPlayerEntity *pLocalPlayerCurrent(GetLocalPlayerCurrent());
    if (pLocalPlayerCurrent != NULL)
        pPredictedPlayer = pLocalPlayerCurrent;

    if (pPredictedPlayer != NULL)
    {
        m_CurrentClientSnapshot.SetFov(ICvar::GetFloat(_T("cam_fov")));

        IInventoryItem *pNewItem(pPredictedPlayer->GetItem(m_CurrentClientSnapshot.GetSelectedItem()));
        IInventoryItem *pPrevItem(pPredictedPlayer->GetCurrentItem());

        if (pNewItem != NULL)
        {
            CVec3f v3Angles(m_CurrentClientSnapshot.GetCameraAngles());
            pNewItem->ApplyDrift(v3Angles);
            m_CurrentClientSnapshot.SetCameraAngles(v3Angles[YAW], v3Angles[PITCH]);
        }

        if (pNewItem != NULL && pPrevItem != NULL)
        {
            CVec3f v3Angles(m_CurrentClientSnapshot.GetCameraAngles());

            // Constrain camera
            if (pNewItem->IsMelee() && pPrevItem->IsMelee())
            {
                if (pPredictedPlayer->GetIsVehicle())
                {
                    m_CurrentClientSnapshot.AdjustCameraPitch(v3Angles[PITCH] - CLAMP(v3Angles[PITCH], float(p_vehicleMeleeMinPitch), float(p_vehicleMeleeMaxPitch)));

                    if (v3Angles[YAW] > 180.0f)
                        m_CurrentClientSnapshot.AdjustCameraYaw(v3Angles[YAW] - CLAMP(v3Angles[YAW], 360.0f - float(p_vehicleMeleeYaw), 360.0f));
                    else
                        m_CurrentClientSnapshot.AdjustCameraYaw(v3Angles[YAW] - CLAMP(v3Angles[YAW], 0.0f, float(p_vehicleMeleeYaw)));
                }
                else
                {
                    m_CurrentClientSnapshot.AdjustCameraPitch(v3Angles[PITCH] - CLAMP(v3Angles[PITCH], float(p_meleeMinPitch), float(p_meleeMaxPitch)));
                }
            }
            else if (pNewItem->IsSiege() && pPrevItem->IsSiege())
            {
                m_CurrentClientSnapshot.AdjustCameraPitch(v3Angles[PITCH] - CLAMP(v3Angles[PITCH], float(p_siegeMinPitch), float(p_siegeMaxPitch)));

                if (v3Angles[YAW] > 180.0f)
                    m_CurrentClientSnapshot.AdjustCameraYaw(v3Angles[YAW] - CLAMP(v3Angles[YAW], 360.0f - float(p_siegeYaw), 360.0f));
                else
                    m_CurrentClientSnapshot.AdjustCameraYaw(v3Angles[YAW] - CLAMP(v3Angles[YAW], 0.0f, float(p_siegeYaw)));
            }
        }

        if (pPredictedPlayer->GetTransparentAngle() != 0.0f && m_CurrentClientSnapshot.GetCameraAngles()[PITCH] < pPredictedPlayer->GetTransparentAngle())
        {
            pPredictedPlayer->AddClientRenderFlags(ECRF_HALFTRANSPARENT);
            AddTransparentEntity(pPredictedPlayer->GetIndex());
        }
    }

    // Generate a new client snapshot
    SendClientSnapshot();

    if (GetCurrentInterface() == CG_INTERFACE_LOADOUT)
        UpdateLoadout();

    m_vFirstPersonEffects.clear();

    SceneManager.Clear();
    SceneManager.ClearBackground();

    // Local client
    if (pLocalPlayerNext != NULL && pLocalPlayerCurrent != NULL)
    {
        PROFILE("Local client");

        m_pCurrentEntity = GetClientEntity(pLocalPlayerNext->GetIndex());
        SetEventTarget(CG_EVENT_TARGET_ENTITY);

        pPredictedPlayer = NULL;

        if (cg_prediction && !ReplayManager.IsPlaying())
        {
            // Predict the local player's entity from the latest information from the server
            pLocalPlayerCurrent->Copy(*pLocalPlayerNext);
            pPredictedPlayer = pLocalPlayerCurrent;
            if (pPredictedPlayer == NULL)
                EX_ERROR(_T("Failed creating an entity for the predicted player state"));

            uint uiRealGameTime(GetGameTime());

            // Apply all unacknowledged client snapshots
            for (ClientSnapshotDeque::iterator it(m_deqClientSnapshots.begin()); it != m_deqClientSnapshots.end(); ++it)
            {
                // Set local game time to the point that we generated this client snapshot
                SetGameTime(it->GetTimeStamp());
                SetFrameLength(it->GetFrameLength());
                SeedRand(it->GetTimeStamp() * it->GetServerFrame());

                pPredictedPlayer->ReadClientSnapshot(*it);
            }

            if (m_PendingClientSnapshot.GetFrameLength() > 0)
            {
                SetGameTime(m_PendingClientSnapshot.GetTimeStamp());
                SetFrameLength(m_PendingClientSnapshot.GetFrameLength());
                SeedRand(m_PendingClientSnapshot.GetTimeStamp() * m_PendingClientSnapshot.GetServerFrame());

                pPredictedPlayer->ReadClientSnapshot(m_PendingClientSnapshot);
            }

            // Reset local game time
            SetGameTime(uiRealGameTime);
            SetFrameLength(m_pHostClient->GetClientFrameLength());
            SeedRand(m_CurrentClientSnapshot.GetTimeStamp() * m_CurrentClientSnapshot.GetServerFrame());
            
            pPredictedPlayer->SetBaseFov(m_CurrentClientSnapshot.GetFov());

            if (IsCommander())
                m_pClientCommander->Frame();
        }
        else // No prediction
        {
            m_pCurrentEntity = GetClientEntity(pLocalPlayerNext->GetIndex());

            m_pCurrentEntity->Interpolate(m_pHostClient->GetLerpValue());

            pPredictedPlayer = pLocalPlayerCurrent;
            if (pPredictedPlayer == NULL)
                EX_ERROR(_T("Failed creating an entity for the predicted player state"));

            if (ReplayManager.IsPlaying())
            {
                const CVec3f &v3Angles(pPredictedPlayer->GetAngles());
                m_CurrentClientSnapshot.SetCameraAngles(v3Angles[YAW], v3Angles[PITCH]);
                m_CurrentClientSnapshot.SelectItem(pPredictedPlayer->GetSelectedItem());
            }

            pPredictedPlayer->SetViewAngles(pPredictedPlayer->GetAngles());

            pPredictedPlayer->SetBaseFov(m_CurrentClientSnapshot.GetFov());

            if (IsCommander())
                m_pClientCommander->Frame();
        }

        pPredictedPlayer->UpdateViewBob(m_pHostClient->GetClientFrameLength());

        // Camera
        CVec3f v3CameraAngles;
        CVec3f v3CameraPosition;

        v3CameraPosition = pPredictedPlayer->GetPosition();

        if (pPredictedPlayer->GetIsVehicle())
            v3CameraAngles = pPredictedPlayer->GetAngles() + m_CurrentClientSnapshot.GetCameraAngles();
        else
            v3CameraAngles = m_CurrentClientSnapshot.GetCameraAngles();

        if (pPredictedPlayer->GetStatus() == ENTITY_STATUS_DORMANT ||
            pPredictedPlayer->GetStatus() == ENTITY_STATUS_SPAWNING ||
            pPredictedPlayer->IsFirstPerson())
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
                pPredictedPlayer->GetIndex() == m_uiCameraIndex)
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
            m_uiCameraIndex = pPredictedPlayer->GetIndex();
        }

        v3CameraPosition = m_v3CameraPosition;
        v3CameraAngles = m_v3CameraAngles + m_v3CameraEffectAngleOffset;

        pPredictedPlayer->SetupCamera(*m_pCamera, v3CameraPosition, v3CameraAngles);

        m_pCamera->AddOffset(m_v3CameraEffectOffset);

        // Camera
        m_pCamera->SetTime(MsToSec(GetGameTime()));
        m_pCamera->SetWidth(float(Vid.GetScreenW()));
        m_pCamera->SetHeight(float(Vid.GetScreenH()));
        m_pCamera->SetFovXCalc(pPredictedPlayer->GetFov());

        if (IsCommander())
        {
            m_pCamera->AddFlags(CAM_FOG_OF_WAR | CAM_NO_FOG | CAM_SHADOW_UNIFORM | CAM_SHADOW_NO_FALLOFF);
            m_pCamera->SetShadowBias(m_pClientCommander->GetCameraDistance());
            m_pCamera->SetShadowMaxFov(180.0f);
        }
        else
        {
            m_pCamera->RemoveFlags(CAM_FOG_OF_WAR | CAM_NO_FOG | CAM_SHADOW_UNIFORM | CAM_SHADOW_NO_FALLOFF);
            m_pCamera->SetShadowBias(0.0f);

            if (pPredictedPlayer->GetType() == Player_Observer)
                m_pCamera->SetShadowMaxFov(180.0f);
            else
                m_pCamera->SetShadowMaxFov(110.0f);
        }

        UpdateGameTips();
        UpdateHoverEntity();
        UpdateOrders();

        // Interface updates
        if (IsCommander())
            m_pInterfaceManager->UpdateCommSelectionInfo(m_pClientCommander->GetSelectedEntity(), true);

        m_pInterfaceManager->UpdateHoverInfo(m_uiHoverEntity, true);

        UpdatePetInfo();

        DrawAreaCast();
        UpdatePinging();
    }

    m_pInterfaceManager->Update();

    // Render the fog of war texture
    DrawFogofWar();

    if (GetCurrentInterface() == CG_INTERFACE_SPAWN)
    {
        Draw2D.SetColor(BLACK);
        Draw2D.Clear();
        UpdateMinimap();
        return;
    }

    Host.AverageFPS(true);

    SceneManager.PrepCamera(*m_pCamera);

    // Draw entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->PopulateScene();

    // Draw events 
    EventsFrame();

    //
    AddClientGameEffects();

    if (pLocalPlayerCurrent)
    {
        if (IsCommander())
        {
            CVec3f v3Pos(m_pCamera->GetOrigin());
            CVec3f v3End(M_PointOnLine(v3Pos, m_pCamera->GetViewAxis(FORWARD), FAR_AWAY));
            STraceInfo trace;
            if (GameClient.TraceLine(trace, v3Pos, v3End, TRACE_TERRAIN))
                K2SoundManager.SetListenerPosition((v3Pos + trace.v3EndPos * 4.0f) / 5.0f, V3_ZERO, m_pCamera->GetViewAxis(FORWARD), m_pCamera->GetViewAxis(UP), false);
            else
                K2SoundManager.SetListenerPosition(v3Pos, V3_ZERO, m_pCamera->GetViewAxis(FORWARD), m_pCamera->GetViewAxis(UP), false);
        }
        else
        {
            CAxis axis(pLocalPlayerCurrent->GetAngles());
            K2SoundManager.SetListenerPosition(pLocalPlayerCurrent->GetCameraPosition(pLocalPlayerCurrent->GetPosition(), pLocalPlayerCurrent->GetAngles()), pLocalPlayerCurrent->GetVelocity(), axis.Forward(), axis.Up(), false);
        }
    }

    UpdateFirstPerson();

    // Redo camera (for effect offsets)
    if (pLocalPlayerCurrent && pLocalPlayerCurrent->GetStatus() != ENTITY_STATUS_DORMANT)
    {
        pLocalPlayerCurrent->SetupCamera(*m_pCamera, m_v3CameraPosition, m_v3CameraAngles + m_v3CameraEffectAngleOffset);
        m_pCamera->AddOffset(m_v3CameraEffectOffset);
    }

    SceneManager.DrawSky(*m_pCamera, MsToSec(m_pHostClient->GetClientFrameLength()));

    // Render the main scene
    SceneManager.PrepCamera(*m_pCamera);
    SceneManager.Render();

    // Render the player's first-person weapon and hands
    RenderFirstPerson();

    if (IsCommander())
        RenderSelectedPlayerView(m_pClientCommander->GetSelectedEntity());

    // Commander 2D Drawing
    if (IsCommander())
        m_pClientCommander->Draw();

    DrawLittleTextPopupMessages();
    DrawOrderLocator();
    DrawOfficerLocator();
    DrawPetLocator();

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
    DrawPlayerStats();
    DrawMeleeTraces();
    DrawHoverStats();

    if (m_deqClientSnapshots.size() > 127)
    {
        Draw2D.SetColor(BLACK);
        Draw2D.String(2.0f, 2.0f + Draw2D.GetScreenH() * 0.25f, Draw2D.GetScreenW(), Draw2D.GetScreenH(), _T("Connection Interrupted"), g_ResourceManager.LookUpName(_T("gamelarge"), RES_FONTMAP), DRAW_STRING_CENTER);

        Draw2D.SetColor(WHITE);
        Draw2D.String(0.0f, Draw2D.GetScreenH() * 0.25f, Draw2D.GetScreenW(), Draw2D.GetScreenH(), _T("Connection Interrupted"), g_ResourceManager.LookUpName(_T("gamelarge"), RES_FONTMAP), DRAW_STRING_CENTER);
    }
}


/*====================
  CGameClient::EndedFrame
  ====================*/
void    CGameClient::EndedFrame()
{
    PROFILE("CGameClient::EndedFrame");

    SetGameTime(m_pHostClient->GetTime());
    SetFrameLength(m_pHostClient->GetClientFrameLength());

    if (m_pHostClient->GetState() == CLIENT_STATE_CONNECTED)
    {
        SetGamePhase(GAME_PHASE_STANDBY);
        return;
    }

    UpdateVision();

    // Update client entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->Frame(m_pHostClient->GetLerpValue());

    // Input
    if (m_eLastInterface != GetCurrentInterface())
    {
        m_eLastInterface = GetCurrentInterface();
        m_CurrentClientSnapshot.ClearButton(GAME_BUTTON_ALL);

        m_bVCActive = false;
        m_VCSubActive = BUTTON_INVALID;
    }

    // Voice command binds must be run FIRST so BINDTABLE_GAME does not catch the inventory binds
    if (IsVCSubMenuActive())
        Input.ExecuteBinds(BINDTABLE_GAME_VOICECOMMAND_SUB, 0);
    else if (IsVCMenuActive())
        Input.ExecuteBinds(BINDTABLE_GAME_VOICECOMMAND, 0);

    Input.ExecuteBinds(BINDTABLE_GAME, 0);

    // Sound
    WorldSoundsFrame();

    // Remove any input we used above but didn't discard
    Input.FlushByTable(BINDTABLE_GAME, 0);

    // Generate a new client snapshot
    SendClientSnapshot();

    IPlayerEntity *pPredictedPlayer(GetLocalPlayerCurrent());

    SceneManager.Clear();
    SceneManager.ClearBackground();

    // Camera
    CVec3f v3CameraAngles;

    if (pPredictedPlayer != NULL)
    {
        if (pPredictedPlayer->GetIsVehicle())
            v3CameraAngles = pPredictedPlayer->GetAngles() + m_CurrentClientSnapshot.GetCameraAngles() + m_v3CameraEffectAngleOffset;
        else
            v3CameraAngles = m_CurrentClientSnapshot.GetCameraAngles() + m_v3CameraEffectAngleOffset;

        pPredictedPlayer->SetupCamera(*m_pCamera, pPredictedPlayer->GetPosition(), v3CameraAngles);
    }

    CVec3f v3CamStart(m_pCamera->GetOrigin());
    CAxis axisStart(m_pCamera->GetViewAxis());

    CVec3f v3CamTarget(V_ZERO);
    CAxis axisTarget(V_ZERO);
    int iWinningTeam(GetWinningTeam());
    CEntityTeamInfo *pTeam(GetTeam(iWinningTeam ^ 3));
    if (pTeam != NULL)
    {
        IBuildingEntity *pTargetBuilding(GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
        if (pTargetBuilding != NULL)
        {
            v3CamTarget = pTargetBuilding->GetPosition();
            axisTarget = pTargetBuilding->GetAngles();
        }
    }
    CVec3f v3CamEnd(v3CamTarget + cam_endGameOffset * axisTarget);
    
    float fLerp(CLAMP((Game.GetGameTime() - Game.GetPhaseStartTime()) / float(cam_endGameLerpTime), 0.0f, 1.0f));
    CVec3f v3Pos(LERP(fLerp, v3CamStart, v3CamEnd));

    float fHeight(Game.GetTerrainHeight(v3Pos.x, v3Pos.y));
    v3Pos.z = MAX(fHeight + cam_endGameMinHeight, v3Pos.z);
    m_pCamera->SetOrigin(v3Pos);
    m_pCamera->SetTarget(v3CamTarget);

    m_pCamera->SetViewAxis(M_QuatToAxis(M_LerpQuat(fLerp, M_AxisToQuat(axisStart), M_AxisToQuat(m_pCamera->GetViewAxis()))));

    m_pCamera->AddOffset(m_v3CameraEffectOffset);

    // Camera
    m_pCamera->SetTime(MsToSec(GetGameTime()));
    m_pCamera->SetWidth(float(Vid.GetScreenW()));
    m_pCamera->SetHeight(float(Vid.GetScreenH()));
    m_pCamera->SetFovXCalc(pPredictedPlayer == NULL ? ICvar::GetFloat(_T("cam_fov")) : pPredictedPlayer->GetFov());
    m_pCamera->RemoveFlags(CAM_FOG_OF_WAR | CAM_NO_FOG);

    // Interface updates
    m_pInterfaceManager->Update();

    Host.AverageFPS(true);

    // Draw the rest of the entities
    SetEventTarget(CG_EVENT_TARGET_ENTITY);
    m_pClientEntityDirectory->PopulateScene();

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

    SceneManager.DrawSky(*m_pCamera, MsToSec(Game.GetFrameLength()));

    // Render the main scene
    SceneManager.PrepCamera(*m_pCamera);
    SceneManager.Render();

    DrawLittleTextPopupMessages();

    DrawVoiceInfo();

    // Screen effects
    for (vector<SOverlayInfo>::iterator itOverlay(m_vOverlays.begin()); itOverlay != m_vOverlays.end(); ++itOverlay)
    {
        Draw2D.SetColor(itOverlay->v4Color);
        Draw2D.Rect(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()), itOverlay->hMaterial);
    }
    m_vOverlays.clear();

    uint uiGameTime(GetGameTime());
    uint uiPhaseEndTime(GetPhaseEndTime());
    if (uiGameTime >= uiPhaseEndTime)
        SetGamePhase(GAME_PHASE_STANDBY);
}


/*====================
  CGameClient::BackgroundFrame
  ====================*/
void    CGameClient::BackgroundFrame()
{
    PROFILE("CGameClient::BackgroundFrame");

    // Generate a new client snapshot
    if (m_pHostClient->GetServerFrame() > 1)
        SendClientSnapshot();
}


/*====================
  CGameClient::Shutdown
  ====================*/
uint    CGameClient::Shutdown()
{
    Console << _T("Shutting down client...") << newl;
    Input.SetCursorRecenter(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorHidden(CURSOR_GAME, BOOL_NOT_SET);
    Input.SetCursorConstrained(CURSOR_GAME, BOOL_NOT_SET);
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
  CGameClient::ProcessSnapshot
  ====================*/
bool    CGameClient::ProcessSnapshot(CSnapshot &snapshot)
{
    PROFILE_EX("CGameClient::ProcessSnapshot", PROFILE_GAME_CLIENT_PROCESS_SNAPSHOT);

    uint uiRealGameTime(GetGameTime());
    int iClient(m_pHostClient->GetClientNum());

    try
    {
        // Clean out any client snapshots that are older than this frame
        while (!m_deqClientSnapshots.empty() && m_deqClientSnapshots.front().GetTimeStamp() <= snapshot.GetLastReceivedClientTime())
        {
            m_deqClientSnapshots.pop_front();
        }

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
#if 1
                    EX_ERROR(_T("Invalid previous frame number: ") + XtoA(snapshot.GetPrevFrameNumber()) + _T(" (current: ") + XtoA(snapshot.GetFrameNumber()) + _T(")"));
#else
                    Console << _T("Invalid previous frame number: ") << XtoA(snapshot.GetPrevFrameNumber()) << _T(" (current: ") << XtoA(snapshot.GetFrameNumber()) << _T(")") << newl;
                    return false;
#endif
                }
                else
                {
                    m_CurrentServerSnapshot = *CSnapshot::GetByHandle(*it);
                }
            }
        }

        m_CurrentServerSnapshot.SetValid(true);
        m_CurrentServerSnapshot.SetFrameNumber(snapshot.GetFrameNumber());
        m_CurrentServerSnapshot.SetPrevFrameNumber(-1);
        m_CurrentServerSnapshot.SetLastReceivedClientTime(snapshot.GetLastReceivedClientTime());
        m_CurrentServerSnapshot.SetTimeStamp(snapshot.GetTimeStamp());

        // Read events
        for (int i(0); i < snapshot.GetNumEvents(); ++i)
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

        SnapshotVector &vBaseEntities(m_CurrentServerSnapshot.GetEntities());
        SnapshotVector_it citBase(vBaseEntities.begin());

        static CEntitySnapshot entSnapshot;

        START_PROFILE("Translate Entities")

        // Translate entities
        for (;;)
        {
            // Grab a "shell" entity snapshot from the the frame snapshot.
            // The data will be filled in once we know the type.
            entSnapshot.Clear();
            if (!snapshot.GetNextEntity(entSnapshot, iClient))
                break;

            while (citBase != vBaseEntities.end() && citBase->first < entSnapshot.GetIndex())
                ++citBase;

            if (citBase == vBaseEntities.end() || citBase->first > entSnapshot.GetIndex())
            {
                //
                // New entity, read from baseline
                //

                ushort unType(entSnapshot.GetType());

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                    continue;

                const vector<SDataField>* pTypeVector(EntityRegistry.GetTypeVector(unType));
                if (pTypeVector == NULL)
                    EX_ERROR(_T("Unknown new entity type, bad snapshot"));

                entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
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

                // If the type is NULL, the entity is dead and should be removed
                if (unType == 0)
                {
                    CEntitySnapshot::DeleteByHandle(citBase->second);
                    citBase = vBaseEntities.erase(citBase);
                    continue;
                }               

                const vector<SDataField>* pTypeVector(EntityRegistry.GetTypeVector(unType));
                if (pTypeVector == NULL)
                    EX_ERROR(_T("Unknown updated entity type, bad snapshot"));

                if (entSnapshot.GetTypeChange())
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector, EntityRegistry.GetBaseline(unType));
                    *pBaseSnapshot = entSnapshot;
                }
                else
                {
                    entSnapshot.ReadBody(snapshot.GetReceivedBuffer(), *pTypeVector);
                    pBaseSnapshot->ApplyDiff(entSnapshot);
                }
                pBaseSnapshot->SetApplyToFrame(snapshot.GetFrameNumber());
                ++citBase;
            }
        }

        END_PROFILE // Translate Entities

        // Copy next states into previous states, etc
        m_pClientEntityDirectory->PrepForSnapshot();

        START_PROFILE("Process")

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

            // Detach state incase owner changed but type didn't
            if (pEntity)
            {
                if (pEntity->IsState())
                {
                    IEntityState *pState(pEntity->GetAsState());

                    CClientEntity *pOwner(GetClientEntity(pState->GetOwner()));
                    if (pOwner)
                    {
                        pOwner->GetNextEntity()->ClearState(pState);
                        pOwner->GetPrevEntity()->ClearState(pState);
                        pOwner->GetCurrentEntity()->ClearState(pState);
                    }
                }
                else if (pEntity->IsInventoryItem())
                {
                    IInventoryItem *pItem(pEntity->GetAsInventoryItem());

                    CClientEntity *pOwner(GetClientEntity(pItem->GetOwner()));
                    if (pOwner)
                    {
                        pOwner->GetNextEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
                        pOwner->GetPrevEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
                        pOwner->GetCurrentEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
                    }
                }
                else if (pEntity->GetType() == Entity_ClientInfo)
                {
                    if (pEntity == m_pLocalClient)
                        m_pLocalClient = NULL;
                }
            }

            // If the client does not have an entry for this entity, allocate a new one
            if (pEntity == NULL)
            {
                pEntity = m_pClientEntityDirectory->Allocate(pEntitySnapshot->GetIndex(), pEntitySnapshot->GetType());

                // Apply the update
                pEntity->ReadSnapshot(*pEntitySnapshot);

                pEntity->Spawn();

                // Reapply the update to undo things spawn changed
                pEntitySnapshot->RewindRead();
                pEntity->ReadSnapshot(*pEntitySnapshot);
            }
            else if (pEntity->GetType() != pEntitySnapshot->GetType())
            {
                if (cg_debugEntities)
                    Console << _T("Entity #") << pEntitySnapshot->GetIndex() << _T(" type change from ")
                            << SHORT_HEX_STR(pEntity->GetType()) << _T(" to ")
                            << SHORT_HEX_STR(pEntitySnapshot->GetType()) << newl;

                // Make sure we clear out the client entry
                if (pEntity->GetType() == Entity_ClientInfo)
                    m_mapClients.erase(static_cast<CEntityClientInfo*>(pEntity)->GetClientNumber());
                
                m_pClientEntityDirectory->Delete(pEntitySnapshot->GetIndex());
                pEntity = m_pClientEntityDirectory->Allocate(pEntitySnapshot->GetIndex(), pEntitySnapshot->GetType());

                // Apply the update
                pEntity->ReadSnapshot(*pEntitySnapshot);
                
                pEntity->Spawn();

                // Reapply the update to fix things spawn changed
                pEntitySnapshot->RewindRead();
                pEntity->ReadSnapshot(*pEntitySnapshot);
            }
            else
            {
                // Apply the update
                pEntity->ReadSnapshot(*pEntitySnapshot);
            }

            pEntity->SetFrame(m_CurrentServerSnapshot.GetFrameNumber());
            pEntity->Validate();

            // Check for a game info update
            if (pEntity->GetType() == Entity_GameInfo)
            {
                CEntityGameInfo *pGameInfo(static_cast<CEntityGameInfo *>(pEntity));

                if (pGameInfo->GetGamePhase() >= NUM_GAME_PHASES)
                    EX_ERROR(_T("Invalid game phase"));

                if (pGameInfo->GetGamePhase() != GetGamePhase() ||
                    pGameInfo->GetPhaseStartTime() != GetPhaseStartTime() ||
                    pGameInfo->GetPhaseDuration() != GetPhaseDuration())
                {
                    SetGamePhase(EGamePhase(pGameInfo->GetGamePhase()), pGameInfo->GetPhaseDuration(), pGameInfo->GetPhaseStartTime());
                    Console << _T("Game phase change - Phase: ") << GetGamePhase() << _T(" Start: ") << GetPhaseStartTime() << _T(" End: ") << GetPhaseEndTime() << newl;
                }

                SetGameMatchID(pGameInfo->GetGameMatchID());
                SetSuddenDeath(pGameInfo->GetSuddenDeath());

                continue;
            }

            // Check for a team info update
            if (pEntity->GetType() == Entity_TeamInfo)
            {
                CEntityTeamInfo *pTeamInfo(static_cast<CEntityTeamInfo *>(pEntity));
                CEntityTeamInfo *pOldTeamInfo(Game.GetTeam(pTeamInfo->GetTeamID()));

                Game.SetTeam(pTeamInfo->GetTeamID(), pTeamInfo);

                if (pOldTeamInfo)
                {
                    if (pOldTeamInfo != pTeamInfo)
                    {
                        K2_DELETE(pOldTeamInfo);

                        // Initialize team list with any clients we've already recieved
                        for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                        {
                            if (it->second->GetTeam() == pTeamInfo->GetTeamID())
                                UpdateTeamRosters(it->second->GetClientNumber(), it->second->GetTeam(), it->second->GetSquad(), it->second->HasFlags(CLIENT_INFO_IS_OFFICER), it->second->HasFlags(CLIENT_INFO_IS_COMMANDER));
                        }
                    }
                }

                continue;
            }

            // Store client info and track local client
            if (pEntity->GetType() == Entity_ClientInfo)
            {
                CEntityClientInfo *pClient(static_cast<CEntityClientInfo*>(pEntity));
                ClientInfoMap_it itClient(m_mapClients.find(pClient->GetClientNumber()));
                if (itClient != m_mapClients.end() && itClient->second != pClient)
                    Console.Warn << _T("Client number changed: ") << itClient->second->GetClientNumber() << _T(" -> ") << pClient->GetClientNumber() << newl;
                m_mapClients[pClient->GetClientNumber()] = pClient;

                if (pClient->GetClientNumber() == m_pHostClient->GetClientNum())
                    m_pLocalClient = pClient;

                UpdateTeamRosters(pClient->GetClientNumber(), pClient->GetTeam(), pClient->GetSquad(), pClient->HasFlags(CLIENT_INFO_IS_OFFICER), pClient->HasFlags(CLIENT_INFO_IS_COMMANDER));
            }

            // Update attached entity states
            if (pEntity->IsState())
            {
                IEntityState *pState(pEntity->GetAsState());

                CClientEntity *pOwner(GetClientEntity(pState->GetOwner()));
                if (pOwner)
                {
                    pOwner->GetNextEntity()->AddState(pState);
                    pOwner->GetPrevEntity()->AddState(pState);
                    pOwner->GetCurrentEntity()->AddState(pState);
                }
                continue;
            }
            else if (pEntity->IsInventoryItem())
            {
                IInventoryItem *pItem(pEntity->GetAsInventoryItem());

                CClientEntity *pOwner(GetClientEntity(pItem->GetOwner()));
                if (pOwner)
                {
                    pOwner->GetNextEntity()->SetInventorySlot(pItem->GetSlot(), pItem);
                    pOwner->GetPrevEntity()->SetInventorySlot(pItem->GetSlot(), pItem);
                    pOwner->GetCurrentEntity()->SetInventorySlot(pItem->GetSlot(), pItem);
                }
                continue;
            }

            // Use the new state as a base for interpolations
            CClientEntity *pClientEntity(GetClientEntity(pEntity->GetIndex()));
            if (pClientEntity)
                pClientEntity->GetCurrentEntity()->Copy(*pEntity);
        }

        m_pClientEntityDirectory->CleanupEntities();

        END_PROFILE // Process

        // Give events that just spawned a chance to synch with entities that arrived in the same frame
        SynchNewEvents();

        if (ReplayManager.IsPlaying())
        {
            if (m_pLocalClient == NULL && !m_mapClients.empty())
            {
                ClientInfoMap_it itClient(m_mapClients.begin());

                for (; itClient != m_mapClients.end() && itClient->second->IsDisconnected(); ++itClient) {}

                if (itClient != m_mapClients.end())
                    SetReplayClient(itClient->first);
            }
        }
                
        SetGameTime(uiRealGameTime);

        return true;
    }
    catch (CException &ex)
    {
        m_pHostClient->GameError(ex.GetMsg());
        ex.Process(_T("CGameClient::ProcessSnapshot() - "));
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

        case GAME_CMD_CHAT_TEAM:
            sCmd = _T("GAME_CMD_CHAT_TEAM");
            break;

        case GAME_CMD_CHAT_SQUAD:
            sCmd = _T("GAME_CMD_CHAT_SQUAD");
            break;

        case GAME_CMD_SERVERCHAT_ALL:
            sCmd = _T("GAME_CMD_SERVERCHAT_ALL");
            break;

        case GAME_CMD_MESSAGE:
            sCmd = _T("GAME_CMD_MESSAGE");
            break;

        case GAME_CMD_REWARD:
            sCmd = _T("GAME_CMD_REWARD");
            break;

        case GAME_CMD_HITFEEDBACK:
            sCmd = _T("GAME_CMD_HITFEEDBACK");
            break;

        case GAME_CMD_MINIMAP_DRAW:
            sCmd = _T("GAME_CMD_MINIMAP_DRAW");
            break;

        case GAME_CMD_MINIMAP_PING:
            sCmd = _T("GAME_CMD_MINIMAP_PING");
            break;

        case GAME_CMD_BUILDING_ATTACK_ALERT:
            sCmd = _T("GAME_CMD_BUILDING_ATTACK_ALERT");
            break;

        case GAME_CMD_END_GAME:
            sCmd = _T("GAME_CMD_END_GAME");
            break;

        case GAME_CMD_END_GAME_FRAGMENT:
            sCmd = _T("GAME_CMD_END_GAME_FRAGMENT");
            break;

        case GAME_CMD_END_GAME_TERMINATION:
            sCmd = _T("GAME_CMD_END_GAME_TERMINATION");
            break;

        case GAME_CMD_HELLSHRINE_BUILDING:
            sCmd = _T("GAME_CMD_HELLSHRINE_BUILDING");
            break;

        case GAME_CMD_MALPHAS_SPAWN:
            sCmd = _T("GAME_CMD_MALPHAS_SPAWN");
            break;

        case GAME_CMD_CONSTRUCTION_COMPLETE:
            sCmd = _T("GAME_CMD_CONSTRUCTION_COMPLETE");
            break;

        case GAME_CMD_CONSTRUCTION_STARTED:
            sCmd = _T("GAME_CMD_CONSTRUCTION_STARTED");
            break;

        case GAME_CMD_BUILDING_DESTROYED:
            sCmd = _T("GAME_CMD_BUILDING_DESTROYED");
            break;
            
        case GAME_CMD_BLD_HEALTH_LOW:
            sCmd = _T("GAME_CMD_BLD_HEALTH_LOW");
            break;

        case GAME_CMD_SPAWNFLAG_PLACED:
            sCmd = _T("GAME_CMD_SPAWNFLAG_PLACED");
            break;

        case GAME_CMD_GOLD_MINE_LOW:
            sCmd = _T("GAME_CMD_GOLD_MINE_LOW");
            break;

        case GAME_CMD_GOLD_MINE_DEPLETED:
            sCmd = _T("GAME_CMD_GOLD_MINE_DEPLETED");
            break;

        case GAME_CMD_PERSISTANT_ITEMS:
            sCmd = _T("GAME_CMD_PERSISTANT_ITEMS");
            break;

        case GAME_CMD_VOICE_DATA:
            sCmd = _T("GAME_CMD_VOICE_DATA");
            break;

        case GAME_CMD_VOICE_STOPTALKING:
            sCmd = _T("GAME_CMD_VOICE_STOPTALKING");
            break;

        case GAME_CMD_VOICE_STARTTALKING:
            sCmd = _T("GAME_CMD_VOICE_STARTTALKING");
            break;

        case GAME_CMD_VOICE_REMOVECLIENT:
            sCmd = _T("GAME_CMD_VOICE_REMOVECLIENT");
            break;

        case GAME_CMD_VOICECOMMAND:
            sCmd = _T("GAME_CMD_VOICECOMMAND");
            break;

        case GAME_CMD_EXEC_SCRIPT:
            sCmd = _T("GAME_CMD_EXEC_SCRIPT");
            break;

        case GAME_CMD_PETCMD_ORDERCONFIRMED:
            sCmd = _T("GAME_CMD_PETCMD_ORDERCONFIRMED");
            break;

        case GAME_CMD_KILL_NOTIFICATION:
            sCmd = _T("GAME_CMD_KILL_NOTIFICATION");
            break;

        case GAME_CMD_ASSIST_NOTIFICATION:
            sCmd = _T("GAME_CMD_ASSIST_NOTIFICATION");
            break;

        case GAME_CMD_RAZED_NOTIFICATION:
            sCmd = _T("GAME_CMD_RAZED_NOTIFICATION");
            break;

        case GAME_CMD_GADGET_ACCESSED:
            sCmd = _T("GAME_CMD_GADGET_ACCESSED");
            break;
        case GAME_CMD_KILLSTREAK:
            sCmd = _T("GAME_CMD_KILLSTREAK");
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
    case GAME_CMD_CONSOLE_MESSAGE:
        {
            Console.Std << pkt.ReadString();
        }
        break;
    
    case GAME_CMD_CHAT_ALL:
        {
            int iSender(pkt.ReadInt());
            tstring sMsg(pkt.ReadString());

            CEntityClientInfo *pSender(GetClientInfo(iSender));
            if (pSender == NULL || Host.IsIgnored(pSender->GetName()))
                break;

            Console << _T("^269[ALL] ^r") << pSender->GetName() << _T(": ^*") << sMsg << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ^269[ALL] ^r") + pSender->GetName() + _T(": ^*") + sMsg);
        }
        break;

    case GAME_CMD_CHAT_TEAM:
        {
            int iSender(pkt.ReadInt());
            tstring sMsg(pkt.ReadString());

            CEntityClientInfo *pSender(GetClientInfo(iSender));
            if (pSender == NULL || Host.IsIgnored(pSender->GetName()))
                break;

            tstring sPrefix(_T("^y[TEAM] ^r"));
            tstring sColor(_T("^*"));
            CEntityTeamInfo *pTeam(GetTeam(m_pLocalClient->GetTeam()));
            if (pTeam != NULL && pTeam->GetCommanderClientID() == iSender)
            {
                sColor = _T("^293");
                sPrefix = _T("^972[COMMANDER] ^r");
                if (iSender != m_pLocalClient->GetClientNumber())
                    K2SoundManager.Play2DSound(m_hCommanderChatSample);
            }

            Console << sPrefix << pSender->GetName() << _T(": ") << sColor << sMsg << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ") + sPrefix + pSender->GetName() + _T(": ") + sColor + sMsg);
        }
        break;

    case GAME_CMD_CHAT_SQUAD:
        {
            int iSender(pkt.ReadInt());
            tstring sMsg(pkt.ReadString());

            CEntityClientInfo *pSender(GetClientInfo(iSender));
            if (pSender == NULL || Host.IsIgnored(pSender->GetName()))
                break;

            tstring sPrefix(_T("^090[SQUAD] ^r"));
            tstring sColor(_T("^*"));
            CEntityTeamInfo *pTeam(GetTeam(m_pLocalClient->GetTeam()));
            if (pTeam != NULL && pTeam->GetCommanderClientID() == iSender)
            {
                sColor = _T("^293");
                sPrefix = _T("^972[COMMANDER] ^r");
                if (iSender != m_pLocalClient->GetClientNumber())
                    K2SoundManager.Play2DSound(m_hCommanderChatSample);
            }

            Console << sPrefix << pSender->GetName() << _T(": ^*") << sMsg << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ") + sPrefix + pSender->GetName() + _T(": ") + sColor + sMsg);
        }
        break;

    case GAME_CMD_SERVERCHAT_ALL:
        {
            tstring sMsg(pkt.ReadString());
            Console << _T("^yServer Message: ") << sMsg << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ^188[SERVER] ^yServer Message: ") + sMsg);
        }
        break;

    case GAME_CMD_SCRIPT_MESSAGE:
        {
            tstring sMsg(pkt.ReadString());
            Console << _T("^yScript Message: ") << sMsg << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ") + sMsg);
        }
        break;

    case GAME_CMD_MESSAGE:
        {
            tstring sMsg(pkt.ReadString());

            Console << sMsg << newl;
            GameMessage.Trigger(sMsg);
        }
        break;

    case GAME_CMD_REWARD:
        {
            byte yType(pkt.ReadByte());
            CVec3f v3Pos(pkt.ReadV3f());
            ushort unAmount(pkt.ReadShort());

            switch (yType)
            {
            case 0: // Gold
                SpawnLittleTextPopupMessage(_T("+") + XtoA(unAmount), v3Pos, YELLOW);
                break;
            case 1: // EXP
                SpawnLittleTextPopupMessage(_T("+") + XtoA(unAmount), v3Pos, CVec4f(0.75f, 0.00f, 0.75f, 1.00f));
                break;
            }
        }
        break;

    case GAME_CMD_HITFEEDBACK:
        {
            EHitFeedbackType eType(EHitFeedbackType(pkt.ReadByte()));
            

            if (eType == HIT_MELEE_IMPACT)
            {
                CVec3f v3Pos(pkt.ReadV3f());
                HitFeedback(eType, v3Pos);
            }
            else
            {
                uint uiIndex(pkt.ReadShort());
                HitFeedback(eType, uiIndex);
            }
        }
        break;

    case GAME_CMD_MINIMAP_DRAW:
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());

            CBufferFixed<8> buffer;
            buffer << fX << fY;

            Minimap.Execute(_T("draw"), buffer);
        }
        break;

    case GAME_CMD_MINIMAP_PING:
        {
            float fX(pkt.ReadFloat());
            float fY(pkt.ReadFloat());

            SpawnMinimapPing(m_hPingTexture, CVec2f(fX, fY), WHITE);
        }
        break;

    case GAME_CMD_BUILDING_ATTACK_ALERT:
        {
            byte yCount(pkt.ReadByte());

            for (byte y(0); y < yCount; ++y)
            {
                uint uiIndex(pkt.ReadInt());
                IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
                if (pBuilding == NULL)
                    continue;

                pBuilding->MinimapFlash(RED, cg_buildingAttackAlertTime);
                m_pInterfaceManager->BuildingAttackAlert(pBuilding->GetEntityName());
            }

            if (yCount > 0)
                K2SoundManager.Play2DSound(m_hBuildingAttackedSample);
        }
        break;

    case GAME_CMD_END_GAME:
        {
            int iTeam(pkt.ReadInt());
            uint uiEndTime(pkt.ReadInt());
            tstring sNextMap(pkt.ReadString());

            SetWinningTeam(iTeam);
            SetGamePhase(GAME_PHASE_ENDED, uiEndTime);
            Console << _T("Winning team: ") << iTeam << newl;
            Console << _T("Next map: ") << sNextMap << newl;

            m_pHostClient->AcknowledgeEndGame();
            
            if (m_pLocalClient != NULL)
            {
                if (iTeam == m_pLocalClient->GetTeam())
                    K2SoundManager.PlayMusic(cg_victoryMusic, false, true);
                else
                    K2SoundManager.PlayMusic(cg_defeatMusic, false, true);
            }

            Host.AverageFPS(false);
        }
        break;

    case GAME_CMD_END_GAME_TIME:
        {
            uint uiStartTime(pkt.ReadInt());
            uint uiDuration(pkt.ReadInt());

            if (GetGamePhase() == GAME_PHASE_ENDED && (uiStartTime != GetPhaseStartTime() || uiDuration != GetPhaseDuration()))
                SetGamePhase(GAME_PHASE_ENDED, uiDuration, uiStartTime);
        }
        break;

    case GAME_CMD_END_GAME_FRAGMENT:
        {
            uint uiPacketLength(pkt.ReadInt());
            static char pOut[MAX_PACKET_SIZE + HEADER_SIZE + 1];

            pkt.Read(pOut, uiPacketLength);
            m_bufEndGameData.Append(pOut, uiPacketLength);
        }
        break;

    case GAME_CMD_END_GAME_TERMINATION:
        {
            byte *pDecompressed(NULL);
            char *pFragment(NULL);

            try
            {
                Console.ClientGame << _T("Recieved end game statistics") << newl;
            
                // Append stat data to buffer
                uint uiFragmentSize(pkt.ReadInt());
                if (pkt.HasFaults())
                    EX_ERROR(_T("Could not read fragment size"));
                char *pFragment(K2_NEW_ARRAY(MemManager.GetHeap(HEAP_CLIENT_GAME), char, uiFragmentSize));
                if (pFragment == NULL)
                    EX_ERROR(_T("Failed allocating buffer for fragment"));
                pkt.Read(pFragment, uiFragmentSize);
                if (pkt.HasFaults())
                    EX_ERROR(_T("Failure reading fragment"));
                m_bufEndGameData.Append(pFragment, uiFragmentSize);
                SAFE_DELETE_ARRAY(pFragment);

                // Decompress
                m_bufEndGameData.Rewind();
                uint uiCompressedSize(m_bufEndGameData.ReadInt());
                uint uiOriginalSize(m_bufEndGameData.ReadInt());
                byte *pDecompressed(K2_NEW_ARRAY(MemManager.GetHeap(HEAP_CLIENT_GAME), byte, uiOriginalSize));
                if (pDecompressed == NULL)
                    EX_ERROR(_T("Failed to allocate decompression buffer"));
                CZip::Decompress((const byte*)m_bufEndGameData.Get(m_bufEndGameData.GetReadPos()), uiCompressedSize, pDecompressed, uiOriginalSize);
                m_bufEndGameData.Write(pDecompressed, uiOriginalSize);
                SAFE_DELETE_ARRAY(pDecompressed);

                // Read each players data from the buffer
                uint uiNumPlayers(m_bufEndGameData.ReadByte());
                for (uint uiPlayer(0); uiPlayer < uiNumPlayers; ++uiPlayer)
                {
                    int iClientID(m_bufEndGameData.ReadInt());
                    if (iClientID == -1)
                        continue;
                    CEntityClientInfo *pClient(Game.GetClientInfo(iClientID));
                    if (pClient == NULL)
                    {
                        Console.Err << _T("GAME_CMD_END_GAME_TERMINATION - Could not find client: ") << iClientID << newl;
                        break;
                    }

                    pClient->ReadMatchStatBuffer(m_bufEndGameData);
                }
                m_bufEndGameData.Clear();

#if 0
                // Report client data
                CDBManager *pDB = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CDBManager)(_T("masterserver.savage2.s2games.com"), _T("/irc_updater/client_requester.php"));
                CEntityClientInfo *pClient(Game.GetClientInfo(GameClient.GetLocalClientNum()));

                if (pClient != NULL && pClient->GetAccountID() != -1 && GetGameMatchID() != -1)
                {
                    uint uiAverageFPS = Host.GetAverageFPS();
                    SSysInfo structSysInfo = K2System.GetSystemInfo();

                    pDB->AddRequestVariable(_T("f"), _T("sys_vars"));
                    pDB->AddRequestVariable(_T("account_id"), XtoA(pClient->GetAccountID()));
                    pDB->AddRequestVariable(_T("match_id"), XtoA(GetGameMatchID()));
                    pDB->AddRequestVariable(_T("mac"), structSysInfo.sMAC);
                    pDB->AddRequestVariable(_T("proc"), structSysInfo.sProcessor);
                    pDB->AddRequestVariable(_T("ram"), structSysInfo.sRAM);
                    pDB->AddRequestVariable(_T("os"), structSysInfo.sOS);
                    pDB->AddRequestVariable(_T("vc"), structSysInfo.sVideo);

                    if (uiAverageFPS != -1)
                        pDB->AddRequestVariable(_T("fps"), XtoA(uiAverageFPS));
                    else
                        pDB->AddRequestVariable(_T("fps"), _T("?"));

                    pDB->AddRequestVariable(_T("model"), ICvar::GetString(_T("options_slider_model_value")));
                    pDB->AddRequestVariable(_T("texture"), ICvar::GetString(_T("options_slider_texture_value")));
                    pDB->AddRequestVariable(_T("shader"), ICvar::GetString(_T("options_slider_shader_value")));
                    pDB->AddRequestVariable(_T("shadow"), ICvar::GetString(_T("options_slider_shadow_value")));
                    pDB->AddRequestVariable(_T("wdd"), ICvar::GetString(_T("options_slider_terrain_value")));
                    pDB->AddRequestVariable(_T("odd"), ICvar::GetString(_T("options_slider_object_value")));
                    pDB->AddRequestVariable(_T("fdd"), ICvar::GetString(_T("options_slider_foliage_value")));
                    pDB->AddRequestVariable(_T("tc"), XtoA(ICvar::GetBool(_T("vid_textureCompression"))));
                    pDB->AddRequestVariable(_T("res"), ICvar::GetString(_T("vid_currentMode")));

                    pDB->SendRequest(_T("FPS Information"), false);

                    while (pDB->RequestWaiting())
                    {
                        pDB->Frame();
                        K2System.Sleep(10);
                    }
                }
                SAFE_DELETE(pDB);
#endif
                Host.ResetAverageFPS();
            }
            catch (CException &ex)
            {
                SAFE_DELETE_ARRAY(pFragment);
                SAFE_DELETE_ARRAY(pDecompressed);
                ex.Process(_T("CGameClient::ProcessGameData(GAME_CMD_END_GAME_TERMINATION) - "), NO_THROW);
                return false;
            }
        }
        break;

    case GAME_CMD_HELLSHRINE_BUILDING:
        {
            K2SoundManager.OverridePlayMusic(_T("/music/hellshrine_under_construction.ogg"));
        }
        break;

    case GAME_CMD_MALPHAS_SPAWN:
        {
            //K2SoundManager.OverridePlayMusic(_T("/music/malphas_spawn.ogg"));
        }
        break;

    case GAME_CMD_CONSTRUCTION_COMPLETE:
        {
            uint uiType(pkt.ReadShort());
            ICvar *pPath(EntityRegistry.GetGameSetting(uiType, _T("ConstructionCompleteSoundPath")));

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

    case GAME_CMD_CONSTRUCTION_STARTED:
        {
            uint uiType(pkt.ReadShort());
            ICvar *pPath(EntityRegistry.GetGameSetting(uiType, _T("ConstructionStartedSoundPath")));

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

    case GAME_CMD_BUILDING_DESTROYED:
        {
            uint uiType(pkt.ReadShort());
            uint uiTeam(pkt.ReadByte());
            
            IPlayerEntity *pPlayer(GetLocalPlayer());

            if (pPlayer == NULL)
                break;
            
            ICvar *pPath;
            
            if(pPlayer->GetTeam() == uiTeam)
            {
                pPath = EntityRegistry.GetGameSetting(uiType, _T("DestroyedSoundPath"));
            }
            else
            {
                CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

                if (pTeam == NULL)
                    break;
                
                tstring sSetting(_T("DestroyedSoundPath") + pTeam->GetDefinition()->GetName());
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

    case GAME_CMD_SPAWNFLAG_PLACED:
        {
            IPlayerEntity *pPlayer(GetLocalPlayer());

            if (pPlayer == NULL)
                break;

            CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

            if (pTeam == NULL)
                break;

            int iRace(LowerString(pTeam->GetDefinition()->GetName()) == _T("beast") ? 1 : 0);

            K2SoundManager.Play2DSound(m_hSpawnportalPlacedSample[iRace]);
        }
        break;

    case GAME_CMD_GOLD_MINE_LOW:
        {
            uint uiIndex(pkt.ReadInt());
            IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
            if (pBuilding == NULL)
                break;

            pBuilding->MinimapFlash(YELLOW, cg_buildingAttackAlertTime);

            K2SoundManager.Play2DSound(m_hGoldMineLowSample);
        }
        break;

    case GAME_CMD_GOLD_MINE_DEPLETED:
        {
            uint uiIndex(pkt.ReadInt());
            IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
            if (pBuilding == NULL)
                break;

            pBuilding->MinimapFlash(YELLOW, cg_buildingAttackAlertTime);

            K2SoundManager.Play2DSound(m_hGoldMineDepletedSample);
        }
        break;

    case GAME_CMD_PERSISTANT_ITEMS:
        {
            ushort unType[MAX_PERSISTANT_ITEMS];
            uint uiID[MAX_PERSISTANT_ITEMS];

            for (int i(0); i < MAX_PERSISTANT_ITEMS; ++i)
            {
                unType[i] = pkt.ReadShort();
                uiID[i] = pkt.ReadInt();
            }

            Console.Net << _T("Recieved persistant item list") << newl;

            for (int i(0); i < MAX_PERSISTANT_ITEMS; ++i)
                AddPersistantItem(i, unType[i], uiID[i]);
        }
        break;

    case GAME_CMD_VOICE_DATA:
        {
            ProcessVoicePacket(pkt);
        }
        break;

    case GAME_CMD_VOICE_STOPTALKING:
        {
            int iTargetNum = pkt.ReadInt();
            StoppedTalking(iTargetNum);
        }
        break;

    case GAME_CMD_VOICE_STARTTALKING:
        {
            int iTargetNum = pkt.ReadInt();
            StartedTalking(iTargetNum);
        }
        break;

    case GAME_CMD_VOICE_REMOVECLIENT:
        {
            int iTargetNum = pkt.ReadInt();
            if (m_pVoiceManager != NULL)
                m_pVoiceManager->RemoveClient(iTargetNum);
        }
        break;

    case GAME_CMD_VOICECOMMAND:
        {
            int iSource(pkt.ReadInt());
            tstring sRace(pkt.ReadString());
            tstring sCategory(pkt.ReadString());
            tstring sSubItem(pkt.ReadString());

            IPlayerEntity *pPlayer(GetPlayer(iSource));

            if (pPlayer == NULL)
                break;

            if (Host.IsIgnored(pPlayer->GetClientName()))
                break;

            CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

            if (pTeam == NULL)
                break;

            VCCategory *pCategory(GetVCCategory(Input.MakeEButton(sCategory)));

            if (pCategory == NULL)
                break;

            map<EButton, VCSubMap>::iterator findit;

            findit = pCategory->mapSubItems.find(Input.MakeEButton(sSubItem));

            if (findit == pCategory->mapSubItems.end())
                break;

            VCSubMap_it it(findit->second.find(sRace));

            if (it == findit->second.end())
                break;

            K2SoundManager.Play2DSound(it->second.hSound);

            tstring sPrefix(_T("^696"));

            if (m_pLocalClient != NULL && m_pLocalClient->GetTeam() != pPlayer->GetTeam())
                sPrefix = _T("^966");

            tstring sHeading;

            if (pCategory->eType == VC_ALL)
                sHeading = _T("[VC ALL]");
            else if (pCategory->eType == VC_SQUAD)
                sHeading = _T("[VC SQUAD]");
            else if (pCategory->eType == VC_COMMANDER)
                sHeading = _T("[VC COMMANDER]");
            else
                sHeading = _T("[VC TEAM]");

            Console << sPrefix << sHeading << _T("^* ") << pPlayer->GetClientName() << _T(": ") << it->second.sDesc << newl;
            m_pHostClient->AddGameChatMessage(_T("Add ") + sPrefix + sHeading + _T("^* ") + pPlayer->GetClientName() + _T(": ") + it->second.sDesc);

            CVec4f v4Color(GREEN);
            float fX(0.0f);
            float fY(0.0f);

            if (m_pLocalClient != NULL && m_pLocalClient->GetTeam() != pPlayer->GetTeam())
                v4Color = RED;

            if (GetWorldPointer() != NULL)
            {
                if (GetWorldPointer()->GetWorldWidth() > 0)
                    fX = pPlayer->GetPosition()[X] / GetWorldPointer()->GetWorldWidth();
        
                if (GetWorldPointer()->GetWorldHeight() > 0)
                    fY = pPlayer->GetPosition()[Y] / GetWorldPointer()->GetWorldHeight();
            }

            SpawnMinimapPing(m_hPingTexture, CVec2f(fX, fY), v4Color, false);

            map<uint, uint>::iterator markerit(m_mapVoiceMarkers.find(pPlayer->GetIndex()));
            if (markerit != m_mapVoiceMarkers.end())
                markerit->second = GetGameTime() + 3000;
            else
                m_mapVoiceMarkers.insert(pair<uint, uint>(pPlayer->GetIndex(), GetGameTime() + 3000));
        }
        break;

    case GAME_CMD_EXEC_SCRIPT:
        {
            tstring sScript(pkt.ReadString());
            short nNumArgs(pkt.ReadShort());

            m_mapScriptParams.clear();

            for (int i(0); i < nNumArgs; i++)
            {
                tstring sParam(pkt.ReadString());
                tstring sData(pkt.ReadString());

                m_mapScriptParams.insert(pair<tstring, tstring>(sParam, sData));
            }

            TriggerScript(sScript);
        }
        break;

    case GAME_CMD_PETCMD_ORDERCONFIRMED:
        {
            if (m_pLocalClient == NULL)
                break;
            IPlayerEntity *pLocalPlayer(m_pLocalClient->GetPlayerEntity());
            if (pLocalPlayer == NULL)
                break;
            IPetEntity *pPetEntity(GetPetEntity(pLocalPlayer->GetPetIndex()));
            if (pPetEntity == NULL)
                break;

            tstring sSound(pPetEntity->GetOrderConfirmedSoundPath());

            if (!sSound.empty())
            {
                ResHandle hSample(g_ResourceManager.LookUpPath(sSound));

                if (hSample == INVALID_RESOURCE)
                    hSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(sSound, SND_2D), RES_SAMPLE);

                if (hSample == INVALID_RESOURCE)
                    break;

                K2SoundManager.Play2DSound(hSample);
            }
        }
        break;

    case GAME_CMD_DEATH_MESSAGE:
        {
            uint uiKiller(pkt.ReadInt(-1));
            uint uiVictim(pkt.ReadInt(-1));
            ushort unMethod(pkt.ReadShort(INVALID_ENT_TYPE));

            IVisualEntity *pKiller(GetVisualEntity(uiKiller));
            IVisualEntity *pVictim(GetVisualEntity(uiVictim));
            ICvar *pMethod(EntityRegistry.GetGameSetting(unMethod, _T("Name")));
            if (pKiller == NULL || pVictim == NULL)
                break;

            tstring sKillerName(_T("A ") + pKiller->GetEntityName());
            tstring sVictimName(pVictim->GetEntityName());

            if (pKiller->IsPlayer())
            {
                CEntityClientInfo *pClient(GetClientInfo(pKiller->GetAsPlayerEnt()->GetClientID()));

                if (pClient != NULL)
                    sKillerName = pClient->GetName();
            }

            if (pVictim->IsPlayer())
            {
                CEntityClientInfo *pClient(GetClientInfo(pVictim->GetAsPlayerEnt()->GetClientID()));

                if (pClient != NULL)
                    sVictimName = pClient->GetName();
            }

            tstring sKillerColor(pKiller->GetTeam() == m_pLocalClient->GetTeam() ? _T("^292") : _T("^922"));
            tstring sVictimColor(pVictim->GetTeam() == m_pLocalClient->GetTeam() ? _T("^292") : _T("^922"));

            m_pInterfaceManager->Trigger(UITRIGGER_KILL_NOTIFICATION,
                sKillerColor + sKillerName +
                _T("^* killed ") + sVictimColor + sVictimName +
                ((pMethod != NULL && !pMethod->GetString().empty()) ? (_T("^* with a ^069") + pMethod->GetString()) : SNULL));
        }
        break;

    case GAME_CMD_KILL_NOTIFICATION:
        if (m_hKillSample != INVALID_RESOURCE)
            K2SoundManager.Play2DSound(m_hKillSample);
        break;

    case GAME_CMD_ASSIST_NOTIFICATION:
        if (m_hAssistSample != INVALID_RESOURCE)
            K2SoundManager.Play2DSound(m_hAssistSample);
        break;

    case GAME_CMD_RAZED_NOTIFICATION:
        if (m_hRazedSample != INVALID_RESOURCE)
            K2SoundManager.Play2DSound(m_hRazedSample);
        break;

    case GAME_CMD_PICKUP_ITEM:
        {
            pkt.ReadByte(); // byte ySlot
            pkt.ReadShort(); // ushort unItem

            if (m_hItemPickupSample != INVALID_RESOURCE)
                K2SoundManager.Play2DSound(m_hItemPickupSample);
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

    case GAME_CMD_GADGET_ACCESSED:
        {
            uint uiIndex(pkt.ReadInt());
            uint uiSpawnTime(pkt.ReadInt());
            CClientEntity *pClientEntity(GetClientEntity(uiIndex));
            if (pClientEntity == NULL)
                break;
            IGadgetEntity *pGadget(pClientEntity->GetNextEntity()->GetAsGadget());
            if (pGadget == NULL)
                break;
            if (pGadget->GetSpawnTime() != uiSpawnTime)
                break;

            pGadget->Accessed();
        }
        break;

    case GAME_CMD_CONSOLE_EXECUTE:
        {
            tstring sMsg(pkt.ReadString());
            Console.Execute(sMsg);
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

            IPlayerEntity *pPlayer(GetLocalPlayerCurrent());
            if (pPlayer)
                SelectItem(pPlayer->GetDefaultInventorySlot());
        }
        break;
    
    case GAME_CMD_KILLSTREAK:
        {
            byte yKillStreak = pkt.ReadByte();
            int iClientNum = pkt.ReadInt();
            
            IPlayerEntity *pPlayer(GetLocalPlayer());
            if (pPlayer)
            {
                byte yIndex(0);
                switch (yKillStreak)
                {
                case 3:
                    yIndex = 0;
                    break;
                case 5:
                    yIndex = 1;
                    break;
                case 7:
                    yIndex = 2;
                    break;
                case 10:
                    yIndex = 3;
                    break;
                }
                
                if (pPlayer->GetClientID() != iClientNum)
                    yIndex |= 4;
                
                if (m_hKillStreakNotification[yIndex] != INVALID_RESOURCE)
                    K2SoundManager.Play2DSound(m_hKillStreakNotification[yIndex]);
            }
        }
        break;

    default:
        Console.Warn << _T("Unrecognized message") << newl;
        return false;
    }

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
  CGameClient::TraceCursor
  ====================*/
bool    CGameClient::TraceCursor(STraceInfo &trace, int iIgnoreSurface)
{
    CVec3f v3Dir(m_pCamera->ConstructRay(Input.GetCursorPos()));
    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), v3Dir, FAR_AWAY));

    return Game.TraceLine(trace, m_pCamera->GetOrigin(), v3End, iIgnoreSurface);
}


/*====================
  CGameClient::Cancel
  ====================*/
void    CGameClient::Cancel()
{
    if (m_bShowInfoScreen && ToggleInfoScreen())
        return;
    
    if (StopBuildingPlacement())
    {
        return;
    }
    
    if (m_bShowBuildMenu)
    {
        HideBuildMenu();
        return;
    }
    
    if (StopPinging())
        return;

    if (ICvar::GetBool(_T("ui_minimapDrawing")))
    {
        ICvar::SetBool(_T("ui_minimapDrawing"), false);
        return;
    }

    if (ICvar::GetBool(_T("ui_minimapPing")))
    {
        ICvar::SetBool(_T("ui_minimapPing"), false);
        return;
    }

    if (GameClient.IsVCMenuActive() || GameClient.IsVCSubMenuActive())
    {
        VCMenuActive(false);
        VCSubMenuActive(BUTTON_INVALID);
        return;
    }

    CClientCommander *pCommander(GetClientCommander());
    if (pCommander != NULL && pCommander->Cancel())
        return;

    IPlayerEntity *pPlayer(GetLocalPlayer());
    if (pPlayer != NULL && pPlayer->GetCurrentItem() != NULL && pPlayer->GetCurrentItem()->Cancel(GAME_BUTTON_STATUS_PRESSED))
        return;

    if (pPlayer != NULL && pPlayer->GetStatus() == ENTITY_STATUS_SPAWNING)
    {
        CBufferDynamic buffer;
        buffer << GAME_CMD_CANCEL_SPAWN;
        SendGameData(buffer, true);
        return;
    }   

    ToggleMenu();
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
  CGameClient::StartEffect
  ====================*/
void    CGameClient::StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge)
{
    switch (m_eEventTarget)
    {
    case CG_EVENT_TARGET_ENTITY:
        m_pCurrentEntity->StartEffect(sEffect, iChannel, iTimeNudge);
        break;

    case CG_EVENT_TARGET_HANDS:
        StartFirstPersonEffect(sEffect, iChannel, iTimeNudge);
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

    case CG_EVENT_TARGET_HANDS:
        StopFirstPersonEffect(iChannel);
        break;
    }
}


/*====================
  CGameClient::PlaySound
  ====================*/
void    CGameClient::PlaySound(const tstring &sSound, int iChannel, float fFalloff, float fVolume, int iSoundFlags, int iFadeIn, int iFadeOutStartTime, int iFadeOut, bool bOverride, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime)
{
    PROFILE("CGameClient::PlaySound");

    ResHandle hSample(g_ResourceManager.LookUpPath(sSound));

    if (hSample == INVALID_RESOURCE)
        hSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(sSound, iSoundFlags), RES_SAMPLE);

    if (hSample == INVALID_RESOURCE || !m_pCurrentEntity)
        return;

    m_pCurrentEntity->PlaySound(hSample, fVolume, fFalloff, iChannel, 128, (iSoundFlags & SND_LOOP) == SND_LOOP, iFadeIn, iFadeOutStartTime, iFadeOut, bOverride, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime);
}


/*====================
  CGameClient::PlaySoundStationary
  ====================*/
void    CGameClient::PlaySoundStationary(const tstring &sSound, int iChannel, float fFalloff, float fVolume)
{
    ResHandle hSample(g_ResourceManager.LookUpPath(sSound));

    if (hSample == INVALID_RESOURCE)
        hSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(sSound, 0), RES_SAMPLE);

    if (hSample == INVALID_RESOURCE || !m_pCurrentEntity)
        return;

    K2SoundManager.PlaySFXSound(hSample, &m_pCurrentEntity->GetCurrentEntity()->GetPosition(), NULL, fVolume, fFalloff);
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

    ResHandle hSample(g_ResourceManager.LookUpPath(sSound));

    if (hSample == INVALID_RESOURCE)
        hSample = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(sSound, 0), RES_SAMPLE);

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
        if (m_ahSoundHandle[iChannel] != INVALID_INDEX && K2SoundManager.UpdateHandle(m_ahSoundHandle[iChannel], V3_ZERO, V3_ZERO, false))
        {
            if (!bOverride)
                return;
            K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
            m_ahSoundHandle[iChannel] = INVALID_INDEX;
        }
        m_ahSoundHandle[iChannel] = K2SoundManager.Play2DSFXSound(hSample, fVolume, -1, 128, (iSoundFlags & SND_LOOP) == SND_LOOP, iFadeIn, iFadeOutStartTime, iFadeIn, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime);
    }
    else
    {
        K2SoundManager.Play2DSFXSound(hSample, fVolume, -1, 128, (iSoundFlags & SND_LOOP) == SND_LOOP, iFadeIn, iFadeOutStartTime, iFadeIn, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime);
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
  CGameClient::IsCommander
  ====================*/
bool    CGameClient::IsCommander() const
{
    if (m_pLocalClient == NULL || !m_pLocalClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
        return false;
    return true;
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
  CGameClient::GetPlayer
  ====================*/
IPlayerEntity*  CGameClient::GetPlayer(int iClientNum) const
{
    return m_pClientEntityDirectory->GetPlayerEntityFromClientID(iClientNum);
}


/*====================
  CGameClient::GetPlayerByName
  ====================*/
IPlayerEntity*  CGameClient::GetPlayerByName(const tstring &sName) const
{
    CClientEntityDirectory::ClientEntMap &mapEntMap(m_pClientEntityDirectory->GetEntMap());

    for (CClientEntityDirectory::ClientEntMap_it it(mapEntMap.begin()); it != mapEntMap.end(); ++it)
    {
        if (it->second->GetCurrentEntity() != NULL)
            if (it->second->GetCurrentEntity()->GetAsPlayerEnt() != NULL)
                if (it->second->GetCurrentEntity()->GetAsPlayerEnt()->GetClientName() == sName)
                    return it->second->GetCurrentEntity()->GetAsPlayerEnt();
    }

    return NULL;
}


/*====================
  CGameClient::GetNumClients
  ====================*/
uint    CGameClient::GetNumClients() const
{
    uint uiNumPlayers(0);

    if (m_pClientEntityDirectory == NULL)
        return 0;

    for (ClientInfoMap::const_iterator it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        if (!it->second->IsDisconnected())
            uiNumPlayers++;

    return uiNumPlayers;
}


/*====================
  CGameClient::UpdateMinimapPlayerWaypoint
  ====================*/
void    CGameClient::UpdateMinimapPlayerWaypoint(IPlayerEntity *pPlayer)
{
    if (pPlayer == NULL)
        return;

    CBufferFixed<36> buffer;

    if (pPlayer->GetCurrentOrder() != CMDR_ORDER_CLEAR)
    {
        CVec3f v3OrderPos;

        if (pPlayer->GetCurrentOrderEntIndex() != INVALID_INDEX)
        {
            IVisualEntity *pEnt(GetClientEntityCurrent(pPlayer->GetCurrentOrderEntIndex()));

            if (pEnt)
                v3OrderPos = pEnt->GetPosition();
            else
                v3OrderPos = pPlayer->GetCurrentOrderPos();
        }
        else
            v3OrderPos = pPlayer->GetCurrentOrderPos();

        switch (pPlayer->GetCurrentOrder())
        {
        case CMDR_ORDER_ATTACK:
            buffer.Clear();
            buffer << v3OrderPos.x / GetWorldWidth();
            buffer << 1.0f - (v3OrderPos.y / GetWorldHeight());
            buffer << 8.0f; // Width
            buffer << 8.0f; // Height
            buffer << 1.0f; // Color R
            buffer << 0.0f; // Color G
            buffer << 0.0f; // Color B
            buffer << 1.0f; // Color A
            buffer << m_hMinimapTriangle;
            Minimap.Execute(_T("icon"), buffer);
            break;
        case CMDR_ORDER_MOVE:
            buffer.Clear();
            buffer << v3OrderPos.x / GetWorldWidth();
            buffer << 1.0f - (v3OrderPos.y / GetWorldHeight());
            buffer << 8.0f; // Width
            buffer << 8.0f; // Height
            buffer << 1.0f; // Color R
            buffer << 1.0f; // Color G
            buffer << 0.0f; // Color B
            buffer << 1.0f; // Color A
            buffer << m_hMinimapTriangle;
            Minimap.Execute(_T("icon"), buffer);
            break;
        }
    }

    if (pPlayer->GetOfficerOrder() != OFFICERCMD_INVALID)
    {
        CVec3f v3OrderPos;

        if (pPlayer->GetOfficerOrderEntIndex() != INVALID_INDEX)
        {
            IVisualEntity *pEnt(GetClientEntityCurrent(pPlayer->GetOfficerOrderEntIndex()));

            if (pEnt)
                v3OrderPos = pEnt->GetPosition();
            else
                v3OrderPos = pPlayer->GetOfficerOrderPos();
        }
        else
            v3OrderPos = pPlayer->GetOfficerOrderPos();

        switch (pPlayer->GetOfficerOrder())
        {
        case OFFICERCMD_ATTACK:
            buffer.Clear();
            buffer << v3OrderPos.x / GetWorldWidth();
            buffer << 1.0f - (v3OrderPos.y / GetWorldHeight());
            buffer << 8.0f; // Width
            buffer << 8.0f; // Height
            buffer << 1.0f; // Color R
            buffer << 0.0f; // Color G
            buffer << 0.0f; // Color B
            buffer << 1.0f; // Color A
            buffer << m_hMinimapTriangle;
            Minimap.Execute(_T("icon"), buffer);
            break;
        default:
            buffer.Clear();
            buffer << v3OrderPos.x / GetWorldWidth();
            buffer << 1.0f - (v3OrderPos.y / GetWorldHeight());
            buffer << 8.0f; // Width
            buffer << 8.0f; // Height
            buffer << 1.0f; // Color R
            buffer << 1.0f; // Color G
            buffer << 0.0f; // Color B
            buffer << 1.0f; // Color A
            buffer << m_hMinimapTriangle;
            Minimap.Execute(_T("icon"), buffer);
            break;
        }
    }
}


/*====================
  CGameClient::UpdateVision
  ====================*/
void    CGameClient::UpdateVision()
{
    PROFILE("CGameClient::UpdateVision");

    IPlayerEntity *pPlayer(GetLocalPlayer());

    int iTeam(pPlayer == NULL ? -1 : pPlayer->GetTeam());

    m_vVision.clear();

    CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
    for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        IVisualEntity *pCurrent(it->second->GetCurrentEntity());
        if (pCurrent == NULL)
            continue;

        if (pCurrent->GetTeam() != iTeam ||
            pCurrent->GetStatus() != ENTITY_STATUS_ACTIVE ||
            pCurrent->GetSightRange() <= 0.0f)
            continue;

        m_vVision.push_back(pCurrent);
    }
}


/*====================
  CGameClient::UpdateMinimap
  ====================*/
void    CGameClient::UpdateMinimap()
{
    PROFILE("CGameClient::UpdateMinimap");

    if (m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(GetLocalPlayer());

    bool bLargeMap(false);
    if (pPlayer != NULL && pPlayer->GetStatus() == ENTITY_STATUS_SPAWNING)
        bLargeMap = true;

    CBufferFixed<1> buffer;
    Minimap.Execute(_T("clear"), buffer);
    
    {
        CWorld *pWorld(GetWorldPointer());
        CVec4f v4Padding(pWorld?pWorld->GetMinimapPadding():CVec4f(0.f, 0.f, 0.f, 0.f));
        CBufferFixed<16> paddingBuffer;
        paddingBuffer.WriteFloat(v4Padding.x);
        paddingBuffer.WriteFloat(v4Padding.y);
        paddingBuffer.WriteFloat(v4Padding.z);
        paddingBuffer.WriteFloat(v4Padding.w);

        Minimap.Execute(_T("padding"), paddingBuffer);
    }

    // Draw player's location
    if (pPlayer != NULL && (pPlayer->GetStatus() == ENTITY_STATUS_ACTIVE || pPlayer->GetStatus() == ENTITY_STATUS_DEAD || IsCommander()))
    {
        if (pPlayer->GetMinimapIcon() != INVALID_RESOURCE)
        {
            CBufferFixed<36> buffer;
            buffer << pPlayer->GetPosition().x / GetWorldWidth();
            buffer << 1.0f - (pPlayer->GetPosition().y / GetWorldHeight());
            buffer << float(pPlayer->GetMinimapIconSize()); // Width
            buffer << float(pPlayer->GetMinimapIconSize()); // Height
            buffer << 1.0f; // Color R
            buffer << 1.0f; // Color G
            buffer << 1.0f; // Color B
            buffer << 1.0f; // Color A
            buffer << pPlayer->GetMinimapIcon();
            Minimap.Execute(_T("icon"), buffer);
        }
    }

    // Add icons
    {
        PROFILE("Entities");

        CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());

        // Buildings
        for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
        {
            IVisualEntity *pCurrent(it->second->GetCurrentEntity());
            if (pCurrent == NULL)
                continue;
            if (!pCurrent->IsBuilding())
                continue;
            
            pCurrent->DrawOnMap(Minimap, pPlayer, bLargeMap);
        }

        // Non-Players
        for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
        {
            IVisualEntity *pCurrent(it->second->GetCurrentEntity());
            if (pCurrent == NULL || pCurrent->IsBuilding() || pCurrent->IsPlayer())
                continue;
            
            pCurrent->DrawOnMap(Minimap, pPlayer, bLargeMap);
        }

        if (pPlayer == NULL)
        {
            // All players
            for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
            {
                IVisualEntity *pCurrent(it->second->GetCurrentEntity());
                if (pCurrent == NULL || !pCurrent->IsPlayer())
                    continue;
                
                pCurrent->DrawOnMap(Minimap, pPlayer, bLargeMap);
            }
        }
        else
        {
            // Team mates
            for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
            {
                IVisualEntity *pCurrent(it->second->GetCurrentEntity());
                if (pCurrent == NULL || !pCurrent->IsPlayer())
                    continue;
                if (pPlayer->LooksLikeEnemy(pCurrent))
                    continue;
                
                pCurrent->DrawOnMap(Minimap, pPlayer, bLargeMap);
            }

            // Enemies
            for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
            {
                IVisualEntity *pCurrent(it->second->GetCurrentEntity());
                if (pCurrent == NULL || !pCurrent->IsPlayer())
                    continue;
                if (!pPlayer->LooksLikeEnemy(pCurrent))
                    continue;
                
                pCurrent->DrawOnMap(Minimap, pPlayer, bLargeMap);
            }
        }
    }

    UpdateMinimapPlayerWaypoint(pPlayer);

    // Selected player waypoints
    CPlayerCommander *pCommander(GetLocalPlayer() == NULL ? NULL : GetLocalPlayer()->GetAsCommander());
    if (pCommander != NULL)
    {
        set<SOrders>    setPreviousOrders;

        const uiset &setSelected(m_pClientCommander->GetSelectedEntities());
        for (uiset::const_iterator it(setSelected.begin()); it != setSelected.end(); ++it)
        {
            IVisualEntity *pEnt(GetClientEntityCurrent(*it));
            if (pEnt == NULL)
                continue;

            if (pEnt && pEnt->IsPlayer() && pEnt->GetTeam() == m_pLocalClient->GetTeam())
            {
                IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

                CVec3f v3OrderPos;

                if (pPlayer->GetCurrentOrderEntIndex() != INVALID_INDEX)
                {
                    IVisualEntity *pEnt(GetClientEntityCurrent(pPlayer->GetCurrentOrderEntIndex()));

                    if (pEnt)
                        v3OrderPos = pEnt->GetPosition();
                    else
                        v3OrderPos = pPlayer->GetCurrentOrderPos();
                }
                else
                    v3OrderPos = pPlayer->GetCurrentOrderPos();

                SOrders sOrders;
                sOrders.yOrder = pPlayer->GetCurrentOrder();
                sOrders.v3OrderPos = v3OrderPos;

                if (setPreviousOrders.find(sOrders) == setPreviousOrders.end())
                {
                    setPreviousOrders.insert(sOrders);
                    UpdateMinimapPlayerWaypoint(pPlayer);
                }
            }
        }
    }
    
    // Draw view box
    if (pPlayer != NULL && pPlayer->GetStatus() == ENTITY_STATUS_ACTIVE)
        pPlayer->DrawViewBox(Minimap, *m_pCamera);

    UpdateMinimapPings();
}


/*====================
  CGameClient::UpdateMinimapPings
  ====================*/
void    CGameClient::UpdateMinimapPings()
{
    const uint LIFETIME(5000);
    CBufferFixed<36> buffer;

    for (vector<SMinimapPing>::iterator it(m_vMinimapPing.begin()); it != m_vMinimapPing.end(); )
    {
        if (it->uiSpawnTime + LIFETIME < Game.GetGameTime())
            it = m_vMinimapPing.erase(it);
        else
            ++it;
    }

    for (vector<SMinimapPing>::iterator it(m_vMinimapPing.begin()); it != m_vMinimapPing.end(); ++it)
    {
        float fAlpha(0.0f);
        int iAge(Game.GetGameTime() - it->uiSpawnTime);

        if (iAge < LIFETIME - 1000)
            fAlpha = 1.0f;
        else
            fAlpha = 1.0f - (float(iAge - (LIFETIME - 1000)) / 1000);

        buffer.Clear();
        buffer << it->v2Pos.x;
        buffer << 1.0f - it->v2Pos.y;
        buffer << 32.0f; // Width
        buffer << 32.0f; // Height
        buffer << it->v4Color[R]; // Color R
        buffer << it->v4Color[G]; // Color G
        buffer << it->v4Color[B]; // Color B
        buffer << it->v4Color[A] * fAlpha; // Color A
        buffer << it->hTexture;
        Minimap.Execute(_T("icon"), buffer);
    }
}


/*====================
  CGameClient::UpdateMinimapTexture
  ====================*/
void    CGameClient::UpdateMinimapTexture()
{
    if (m_hMinimapTexture != INVALID_RESOURCE)
    {
        g_ResourceManager.Unregister(m_hMinimapTexture);
        m_hMinimapTexture = INVALID_RESOURCE;
    }

    // UTTAR
    SAFE_DELETE(m_pMinimapBitmap);
    CWorld *pWorld(GetWorldPointer());
    m_pMinimapBitmap = K2_NEW(g_heapResources,   CBitmap)(pWorld->GetTileWidth(), pWorld->GetTileWidth(), BITMAP_RGB);

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

            float fShade(DotProduct(v3Normal, CVec3f(0.0f, 0.0f, 1.0f)) * 0.90f);

            m_pMinimapBitmap->SetPixel4b(iX, iY,
                CLAMP(INT_ROUND(v4Color[R] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[G] * fShade), 0, 255),
                CLAMP(INT_ROUND(v4Color[B] * fShade), 0, 255),
                255);
        }
    }

    m_hMinimapTexture = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("*game_minimap"), m_pMinimapBitmap, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
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
  CGameClient::DrawAreaCast
  ====================*/
void    CGameClient::DrawAreaCast()
{
    IPlayerEntity *pPlayer(GetLocalPlayerCurrent());
    IInventoryItem *pItem(NULL);
    if (pPlayer == NULL || (pItem = pPlayer->GetCurrentItem()) == NULL)
    {
        m_pClientEntityDirectory->Delete(m_pPreviewGadget);
        m_pPreviewGadget = NULL;
        return;
    }

    if (pItem->IsSpell() && pItem->IsReady())
    {
        ISpellItem *pSpell(pItem->GetAsSpell());
        if (pSpell->IsSnapcast())
        {
            m_pClientEntityDirectory->Delete(m_pPreviewGadget);
            m_pPreviewGadget = NULL;
            return;
        }

        // Color potential targets
        CVec3f v3Origin(pSpell->GetTargetLocation());
        CSphere bbSphere(v3Origin, pSpell->GetTargetRadius());

        uivector    vResult;
        Game.GetEntitiesInRadius(vResult, bbSphere, 0);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IVisualEntity *pEnt(GameClient.GetClientEntityCurrent(GameClient.GetGameIndexFromWorldIndex(*it)));
            if (!pEnt)
                continue;

            if (!pSpell->IsValidTarget(pEnt, false))
                continue;

            pEnt->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pEnt->SetSelectColor(CVec4f(pSpell->GetSnapcastSelectColor(), 1.0f));
        }

        // Draw ground sprite
        ResHandle hMaterial(g_ResourceManager.Register(pSpell->GetTargetMaterialPath(), RES_MATERIAL));
        if (hMaterial != INVALID_RESOURCE)
        {
            CSceneEntity scTarget;
            scTarget.Clear();
            scTarget.objtype = OBJTYPE_GROUNDSPRITE;
            scTarget.angle[YAW] = pPlayer->GetAngles()[YAW];
            scTarget.hMaterial = hMaterial;
            scTarget.SetPosition(v3Origin);
            scTarget.width = scTarget.height = pSpell->GetTargetRadius();
            SceneManager.AddEntity(scTarget);
        }

        // Draw gadget preview
        if (!pSpell->GetGadgetName().empty())
        {
            if (m_pPreviewGadget == NULL ||
                m_pPreviewGadget->GetType() != EntityRegistry.LookupID(pSpell->GetGadgetName()))
            {
                if (m_pPreviewGadget != NULL)
                    m_pClientEntityDirectory->Delete(m_pPreviewGadget);

                m_pPreviewGadget = NULL;

                IGameEntity *pNew(m_pClientEntityDirectory->AllocateLocal(pSpell->GetGadgetName()));

                CClientEntity *pNewEnt(pNew ? m_pClientEntityDirectory->GetClientEntity(pNew->GetIndex()) : NULL);
                if (pNewEnt != NULL)
                {
                    if (!pNewEnt->GetCurrentEntity()->IsGadget())
                    {
                        m_pClientEntityDirectory->Delete(pNewEnt);
                    }
                    else
                    {
                        pNewEnt->SetClientEntity(true);
                        pNewEnt->GetPrevEntity()->Validate();
                        pNewEnt->GetNextEntity()->Validate();
                        pNewEnt->GetCurrentEntity()->Validate();

                        m_pPreviewGadget = pNewEnt->GetCurrentEntity()->GetAsGadget();

                        // Set initial angles and position
                        CAxis axis(pPlayer->GetAngles());
                        CVec3f v3Normal(Game.GetTerrainNormal(v3Origin.x, v3Origin.y));
                        CVec3f v3Angles(
                            90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Forward2d(), 0.0f)))),
                            90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Right2d(), 0.0f)))),
                            pPlayer->GetAngles()[YAW]);
                        m_pPreviewGadget->SetAngles(v3Angles);
                        m_pPreviewGadget->SetPosition(v3Origin);
                        m_pPreviewGadget->AddClientRenderFlags(ECRF_HALFTRANSPARENT);

                        m_pPreviewGadget->SpawnPreview();
                    }
                }
            }

            if (m_pPreviewGadget != NULL)
            {
                m_pPreviewGadget->SetPosition(v3Origin);
                CAxis axis(pPlayer->GetAngles());
                CVec3f v3Normal(Game.GetTerrainNormal(v3Origin.x, v3Origin.y));
                CVec3f v3Angles(
                    90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Forward2d(), 0.0f)))),
                    90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Right2d(), 0.0f)))),
                    pPlayer->GetAngles()[YAW]);
                m_pPreviewGadget->SetAngles(LERP(Game.GetFrameLength() / 200.0f, m_pPreviewGadget->GetAngles(), v3Angles));
                m_pPreviewGadget->AddClientRenderFlags(ECRF_SNAPSELECTED);

                if (m_pPreviewGadget->CanSpawn())
                    m_pPreviewGadget->SetSelectColor(CVec4f(0.0f, 2.0f, 0.0f, 1.0f));
                else
                    m_pPreviewGadget->SetSelectColor(CVec4f(2.0f, 0.0f, 0.0f, 1.0f));
            }
        }
        return;
    }

    m_pClientEntityDirectory->Delete(m_pPreviewGadget);
    m_pPreviewGadget = NULL;

    if (pItem->IsSiege())
    {
        ISiegeItem *pSiege(pItem->GetAsSiege());
        if (!pSiege->IsAreaofEffect())
            return;

        CVec3f v3Origin;
        v3Origin = pSiege->GetTargetLocation();

        CSphere bbSphere(v3Origin, pSiege->GetTargetRadius());

        ResHandle hMaterial(g_ResourceManager.Register(pSiege->GetTargetMaterialPath(), RES_MATERIAL));
        if (hMaterial == INVALID_RESOURCE)
            return;

        CSceneEntity scTarget;
        scTarget.Clear();
        scTarget.objtype = OBJTYPE_GROUNDSPRITE;
        scTarget.angle[YAW] = pPlayer->GetAngles()[YAW];
        scTarget.hMaterial = hMaterial;
        scTarget.SetPosition(v3Origin);
        scTarget.width = scTarget.height = pSiege->GetTargetRadius();
        SceneManager.AddEntity(scTarget);
    }
}


/*====================
  CGameClient::CommanderSnapcast
  ====================*/
void    CGameClient::CommanderSnapcast()
{
    // Validate local player
    if (m_pLocalClient == NULL)
        return;
    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;
    CPlayerCommander *pCommander(pPlayer->GetAsCommander());
    if (pCommander == NULL)
        return;

    if (GetClientCommander() == NULL || GetClientCommander()->GetOverideSnapscast())
        return;

    // Check that a snapcasting spell is in use
    IInventoryItem *pItem(pPlayer->GetCurrentItem());
    if (pItem == NULL)
        return;
    ISpellItem *pSpell(pItem->GetAsSpell());
    if (pSpell == NULL)
        return;

    // Check to see if an existing target is valid
    IGameEntity *pOldTarget(GetClientEntityCurrent(m_sSnap.uiLockedIndex));
    if (m_sSnap.uiLockedIndex != INVALID_INDEX && pOldTarget == NULL)
        m_sSnap.uiLockedIndex = INVALID_INDEX;
    
    // Set the snapcast parameters
    m_sSnapcast.bActive = pSpell->IsSnapcast() && pSpell->IsReady();
    m_sSnapcast.fTraceSize = cg_snapcastTracesize;
    m_sSnapcast.fDist = pSpell->GetRange();
    m_sSnapcast.fCumulativeDecay = cg_snapcastCumulativeDecay;
    m_sSnapcast.fCumulativeBreakAngle = cg_snapcastCumulativeBreakAngle;
    m_sSnapcast.uiRequireTime = cg_snapcastReacquireTime;
    m_sSnapcast.fLerp = cg_snapcastLerp;
    m_sSnapcast.fBreakAngle = pSpell->GetSnapcastBreakAngle();
    m_sSnapcast.v3SelectColor = pSpell->GetSnapcastSelectColor();

    // Stop here if snapcasting is inactive
    if (!m_sSnapcast.bActive)
    {
        m_sSnap.uiLockedIndex = INVALID_INDEX;
        m_CurrentClientSnapshot.SetSelectedEntity(INVALID_INDEX);
        return;
    }

    // Check acquisition time
    if (m_sSnap.uiNextAquire > Host.GetTime())
        return;

    // Perform a trace
    // FIXME: Unify this so that there is only one cursor trace per frame for the commander
    STraceInfo trace;
    STraceInfo traceTrans;
    CVec3f v3Dir(m_pCamera->ConstructRay(Input.GetCursorPos()));
    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), v3Dir, FAR_AWAY));

    bool bSound(false);
    uiset setIgnored;
    CVec3f v3TransStart(m_pCamera->GetOrigin());

    TraceLine(trace, v3TransStart, v3End, TRACE_COMMANDER_SNAPCAST, pPlayer->GetWorldIndex());
    traceTrans.uiEntityIndex = 0;

    while (traceTrans.uiEntityIndex != INVALID_INDEX)
    {
        TraceLine(traceTrans, v3TransStart, v3End, SURF_IGNORE | SURF_BLOCKER);

        if (traceTrans.uiEntityIndex == trace.uiEntityIndex)
            break;

        v3TransStart = traceTrans.v3EndPos;

        CWorldEntity *pWorld(GameClient.GetWorldEntity(traceTrans.uiEntityIndex));

        if (pWorld == NULL)
            break;

        setIgnored.insert(traceTrans.uiEntityIndex);
        pWorld->SetSurfFlags(pWorld->GetSurfFlags() | SURF_IGNORE);
            
        IVisualEntity *pEnt(Game.GetVisualEntity(pWorld->GetGameIndex()));

        if (pEnt != NULL)
        {
            pEnt->AddClientRenderFlags(ECRF_HALFTRANSPARENT);
            m_setTransparent.insert(pEnt->GetIndex());
        }
    }

    for (uiset::iterator it(setIgnored.begin()); it != setIgnored.end(); it++)
    {
        CWorldEntity *pWorld(GameClient.GetWorldEntity(*it));

        if (pWorld != NULL)
            pWorld->SetSurfFlags(pWorld->GetSurfFlags() & ~SURF_IGNORE);
    }

    // If CTRL key is pressed, disable locking
    if (GetCurrentSnapshot()->IsButtonDown(GAME_BUTTON_CTRL))
    {
        if (m_sSnap.uiLockedIndex != INVALID_INDEX)
        {
            IVisualEntity *pEnt(GetClientEntityCurrent(m_sSnap.uiLockedIndex));
            if (pEnt)
                pEnt->RemoveClientRenderFlags(ECRF_SNAPSELECTED);
        }

        m_sSnap.uiLockedIndex = INVALID_INDEX;
    }
    // Acquire a new target
    else if (m_sSnap.uiLockedIndex == INVALID_INDEX && trace.uiEntityIndex != INVALID_INDEX)
    {
        uint uiTargetIndex(GetGameIndexFromWorldIndex(trace.uiEntityIndex));
        IGameEntity *pNewTarget(GetClientEntityCurrent(uiTargetIndex));
        if (pSpell->IsValidTarget(pNewTarget, false))
        {
            m_sSnap.uiLockedIndex = uiTargetIndex;
            bSound = true;

        }
    }

    // Validate target
    IVisualEntity *pTarget(GetClientEntityCurrent(m_sSnap.uiLockedIndex));
    if (!pSpell->IsValidTarget(pTarget, false))
    {
        m_sSnap.uiLockedIndex = INVALID_INDEX;
        m_CurrentClientSnapshot.SetSelectedEntity(INVALID_INDEX);
        return;
    }
    else if (pTarget)
    {
        // Check for breaking the lock
        CVec2f v2TargetPos;
#if 0
        if (!m_pCamera->WorldToScreen(pTarget->GetPosition(), v2TargetPos) ||
            CVec2f(v2TargetPos - Input.GetCursorPos()).Length() > cg_cmdrSnapBreakDistance)
#else
        if (m_sSnap.uiLockedIndex != GetGameIndexFromWorldIndex(trace.uiEntityIndex))
#endif
        {
            m_sSnap.uiLockedIndex = INVALID_INDEX;
            m_CurrentClientSnapshot.SetSelectedEntity(INVALID_INDEX);
            return;
        }
        else

        {
            // Track the current target
            pTarget->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pTarget->SetSelectColor(CVec4f(m_sSnapcast.v3SelectColor, 1.0f));

            // Reposition cursor
            if (cg_cmdrSnapStickyCursor)
            {
                Input.SetCursorPos(v2TargetPos.x, v2TargetPos.y);
                m_CurrentClientSnapshot.SetCursorPosition(v2TargetPos);
                K2System.SetMousePos(v2TargetPos.x, v2TargetPos.y);
            }
        }
    }

    if (bSound)
        K2SoundManager.Play2DSound(m_hSnapSample);

    m_CurrentClientSnapshot.SetSelectedEntity(m_sSnap.uiLockedIndex);
}


/*====================
  CGameClient::Snapcast
  ====================*/
void    CGameClient::Snapcast()
{
    if (m_pLocalClient == NULL || m_pLocalClient->GetPlayerEntity() == NULL)
        return;

    // Stop tracking a deleted entity
    if (m_sSnap.uiLockedIndex != INVALID_INDEX)
    {
        if (!GetClientEntityCurrent(m_sSnap.uiLockedIndex))
            m_sSnap.uiLockedIndex = INVALID_INDEX;
    }
    
    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    IInventoryItem *pItem(pPlayer->GetCurrentItem());

    ISpellItem *pSpell(NULL);

    if (pItem == NULL || !pItem->IsSpell())
    {
        m_sSnapcast.bActive = false;
    }
    else
    {
        pSpell = pItem->GetAsSpell();

        m_sSnapcast.bActive = pSpell->IsSnapcast() && pSpell->IsReady();
        m_sSnapcast.fTraceSize = cg_snapcastTracesize;
        m_sSnapcast.fDist = pSpell->GetRange();
        m_sSnapcast.fCumulativeDecay = cg_snapcastCumulativeDecay;
        m_sSnapcast.fCumulativeBreakAngle = cg_snapcastCumulativeBreakAngle;
        m_sSnapcast.uiRequireTime = cg_snapcastReacquireTime;
        m_sSnapcast.fLerp = cg_snapcastLerp;
        m_sSnapcast.fBreakAngle = pSpell->GetSnapcastBreakAngle();
        m_sSnapcast.v3SelectColor = pSpell->GetSnapcastSelectColor();
    }

    if (!m_sSnapcast.bActive)
    {
        if (m_sSnap.uiLockedIndex != INVALID_INDEX)
        {
            m_sSnap.uiLockedIndex = INVALID_INDEX;
            m_CurrentClientSnapshot.SetSelectedEntity(INVALID_INDEX);
        }

        return;
    }

    if (m_sSnap.uiNextAquire > Host.GetTime())
        return;

    float fFrameTime(MsToSec(GetFrameLength()));

    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), m_pCamera->GetViewAxis(FORWARD), m_sSnapcast.fDist));

    bool bSound(false);

    STraceInfo trace;
    STraceInfo traceTrans;
    uiset setIgnored;
    CVec3f v3TransStart(m_pCamera->GetOrigin());

    TraceBox(trace, v3TransStart, v3End, CBBoxf(-m_sSnapcast.fTraceSize, m_sSnapcast.fTraceSize), TRACE_SNAPCAST, pPlayer->GetWorldIndex());
    traceTrans.uiEntityIndex = 0;

    while (traceTrans.uiEntityIndex != INVALID_INDEX)
    {
        TraceLine(traceTrans, v3TransStart, v3End, SURF_IGNORE | SURF_BLOCKER);

        if (traceTrans.uiEntityIndex == trace.uiEntityIndex)
            break;

        v3TransStart = traceTrans.v3EndPos;

        CWorldEntity *pWorld(GameClient.GetWorldEntity(traceTrans.uiEntityIndex));

        if (pWorld == NULL)
            break;

        setIgnored.insert(traceTrans.uiEntityIndex);
        pWorld->SetSurfFlags(pWorld->GetSurfFlags() | SURF_IGNORE);
            
        IVisualEntity *pEnt(Game.GetVisualEntity(pWorld->GetGameIndex()));

        if (pEnt != NULL)
        {
            pEnt->AddClientRenderFlags(ECRF_HALFTRANSPARENT);
            m_setTransparent.insert(pEnt->GetIndex());
        }
    }

    for (uiset::iterator it(setIgnored.begin()); it != setIgnored.end(); it++)
    {
        CWorldEntity *pWorld(GameClient.GetWorldEntity(*it));

        if (pWorld != NULL)
            pWorld->SetSurfFlags(pWorld->GetSurfFlags() & ~SURF_IGNORE);
    }

    // if we hit something, save the index and fade in the name, otherwise fade it out
    if (GetCurrentSnapshot()->IsButtonDown(GAME_BUTTON_CTRL))
    {
        if (m_sSnap.uiLockedIndex != INVALID_INDEX)
        {
            IVisualEntity *pEnt(GameClient.GetClientEntityCurrent(m_sSnap.uiLockedIndex));
            if (pEnt)
                pEnt->RemoveClientRenderFlags(ECRF_SNAPSELECTED);
        }

        m_sSnap.uiLockedIndex = INVALID_INDEX;
    }
    else if (m_sSnap.uiLockedIndex == INVALID_INDEX && trace.uiEntityIndex != INVALID_INDEX)
    {
        IGameEntity *pEntity(GameClient.GetClientEntityCurrent(GetGameIndexFromWorldIndex(trace.uiEntityIndex)));
        if (pEntity && pSpell && pSpell->IsValidTarget(pEntity, false))
        {
            m_sSnap.uiLockedIndex = GetGameIndexFromWorldIndex(trace.uiEntityIndex);
            m_sSnap.v3OldAngles = m_CurrentClientSnapshot.GetCameraAngles();
            bSound = true;
        }
    }

    IGameEntity *pEntity(GameClient.GetClientEntityCurrent(m_sSnap.uiLockedIndex));
    if (!pEntity || !pSpell->IsValidTarget(pEntity, false))
    {
        if (m_sSnap.uiLockedIndex != INVALID_INDEX)
        {
            m_sSnap.uiLockedIndex = INVALID_INDEX;
            m_CurrentClientSnapshot.SetSelectedEntity(INVALID_INDEX);
            bSound = false;
        }
    }

    if (m_sSnap.uiLockedIndex != INVALID_INDEX)
    {
        // Break the current target lock if the cumulative angle has increased beyond the threshold
        CVec3f  vAngleDelta(m_CurrentClientSnapshot.GetCameraAngles() - m_sSnap.v3OldAngles);

        if (vAngleDelta[YAW] < -180.0f)
            vAngleDelta[YAW] += 360.0f;
        if (vAngleDelta[YAW] >= 180.0f)
            vAngleDelta[YAW] -= 360.0f;

        m_CurrentClientSnapshot.AdjustCameraPitch(vAngleDelta[PITCH]);
        m_CurrentClientSnapshot.AdjustCameraYaw(vAngleDelta[YAW]);

        m_sSnap.v3CumulativeDelta += vAngleDelta;

        // Decay cumulative angles
        if (m_sSnap.v3CumulativeDelta[PITCH] > 0.0f)
            m_sSnap.v3CumulativeDelta[PITCH] -= MIN(m_sSnapcast.fCumulativeDecay * fFrameTime, m_sSnap.v3CumulativeDelta[PITCH]);
        else
            m_sSnap.v3CumulativeDelta[PITCH] -= MAX(-m_sSnapcast.fCumulativeDecay * fFrameTime, m_sSnap.v3CumulativeDelta[PITCH]);

        if (m_sSnap.v3CumulativeDelta[YAW] > 0.0f)
            m_sSnap.v3CumulativeDelta[YAW] -= MIN(m_sSnapcast.fCumulativeDecay * fFrameTime, m_sSnap.v3CumulativeDelta[YAW]);
        else
            m_sSnap.v3CumulativeDelta[YAW] -= MAX(-m_sSnapcast.fCumulativeDecay * fFrameTime, m_sSnap.v3CumulativeDelta[YAW]);

        IVisualEntity *pEntity(GetClientEntityCurrent(m_sSnap.uiLockedIndex));
                
        if (ABS(m_sSnap.v3CumulativeDelta[PITCH]) > m_sSnapcast.fCumulativeBreakAngle ||
            ABS(m_sSnap.v3CumulativeDelta[YAW]) > m_sSnapcast.fCumulativeBreakAngle ||
            !pEntity)
        {
            m_CurrentClientSnapshot.AdjustCameraPitch(-m_sSnap.v3CumulativeDelta[PITCH]);
            m_CurrentClientSnapshot.AdjustCameraYaw(-m_sSnap.v3CumulativeDelta[YAW]);
            m_sSnap.uiLockedIndex = INVALID_INDEX;
            m_sSnap.uiNextAquire = Host.GetTime() + m_sSnapcast.uiRequireTime;
            m_sSnap.v3CumulativeDelta.Clear();
            bSound = false;
        }
        else if (pEntity->IsBuilding() || pEntity->IsProp())
        {
            // Track the current target
            pEntity->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pEntity->SetSelectColor(CVec4f(m_sSnapcast.v3SelectColor, 1.0f));

            m_CurrentClientSnapshot.AdjustCameraPitch(-vAngleDelta[PITCH]);
            m_CurrentClientSnapshot.AdjustCameraYaw(-vAngleDelta[YAW]);
            m_sSnap.v3CumulativeDelta.Clear();

            if (GetGameIndexFromWorldIndex(trace.uiEntityIndex) != m_sSnap.uiLockedIndex)
            {
                m_CurrentClientSnapshot.AdjustCameraPitch(-m_sSnap.v3CumulativeDelta[PITCH]);
                m_CurrentClientSnapshot.AdjustCameraYaw(-m_sSnap.v3CumulativeDelta[YAW]);
                m_sSnap.uiLockedIndex = INVALID_INDEX;
                m_sSnap.uiNextAquire = Host.GetTime() + m_sSnapcast.uiRequireTime;
                m_sSnap.v3CumulativeDelta.Clear();
            }
        }
        else
        {
            // Track the current target
            pEntity->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pEntity->SetSelectColor(CVec4f(m_sSnapcast.v3SelectColor, 1.0f));

            CVec3f  v3Entity(pEntity->GetPosition() + pEntity->GetBounds().GetMid());
            CVec3f  v3Camera(m_pCamera->GetOrigin());
            CVec3f  v3Dir(Normalize(v3Entity - v3Camera));
            CVec3f  v3CameraDir(m_pCamera->GetViewAxis(FORWARD));

            CVec3f  v3Angles;
            M_GetAnglesFromForwardVec(v3Dir, v3Angles);

            CVec3f  v3CameraAngles;
            M_GetAnglesFromForwardVec(v3CameraDir, v3CameraAngles);

            float fLerp(CLAMP(fFrameTime * m_sSnapcast.fLerp, 0.0f, 1.0f));
            float fDeltaYaw((v3Angles[YAW] - v3CameraAngles[YAW]));
            float fDeltaPitch((v3Angles[PITCH] - v3CameraAngles[PITCH]));

            if (fDeltaYaw >= 180.0f)
                fDeltaYaw -= 360.0f;

            if (fDeltaYaw < -180.0f)
                fDeltaYaw += 360.0f;

            if (ABS(fDeltaYaw) > m_sSnapcast.fBreakAngle || ABS(fDeltaPitch) > m_sSnapcast.fBreakAngle)
            {
                m_CurrentClientSnapshot.AdjustCameraPitch(-m_sSnap.v3CumulativeDelta[PITCH]);
                m_CurrentClientSnapshot.AdjustCameraYaw(-m_sSnap.v3CumulativeDelta[YAW]);
                m_sSnap.uiLockedIndex = INVALID_INDEX;
                m_sSnap.uiNextAquire = Host.GetTime() + m_sSnapcast.uiRequireTime;
                m_sSnap.v3CumulativeDelta.Clear();
            }
            else
            {
                m_CurrentClientSnapshot.AdjustCameraPitch(-fDeltaPitch * fLerp);
                m_CurrentClientSnapshot.AdjustCameraYaw(-fDeltaYaw * fLerp);
            }
        }
    }

    if (bSound)
        K2SoundManager.Play2DSound(m_hSnapSample);

    m_sSnap.v3OldAngles = m_CurrentClientSnapshot.GetCameraAngles();
    m_CurrentClientSnapshot.SetSelectedEntity(m_sSnap.uiLockedIndex);
}


/*====================
  CGameClient::UpdateGameTips
  ====================*/
void    CGameClient::UpdateGameTips()
{
    if (m_pLocalClient == NULL || m_pLocalClient->GetPlayerEntity() == NULL || !cg_showTips)
    {
        m_uiGameTipExpireTime = 0;
        m_pInterfaceManager->HideGameTip();
        return;
    }
    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    
    // Gametip for when recall is cast on a player. IF we need to set this up for multiple states, a more generic method will be used.
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        IEntityState *pState(pPlayer->GetState(i));
        if (pState == NULL)
            continue;
        
        if (pState->GetType() == State_CommanderRecall)
        {
            m_uiGameTipExpireTime = Host.GetTime() + cg_gameTipDisplayTime;
            m_pInterfaceManager->ShowGameTip(_T("Recall has been cast on you. To be recalled to a spawn point, press the use (E) key within " + XtoA(INT_ROUND((pState->GetExpireTime() - Game.GetGameTime()) * SEC_PER_MS))  + " Seconds."));
        }

    }

    IVisualEntity *pTarget(GameClient.GetClientEntityCurrent(GameClient.GetGameIndexFromWorldIndex(pPlayer->GetGroundEntityIndex())));
    if (pTarget == NULL)
    {
        CAxis axis(m_CurrentClientSnapshot.GetCameraAngles());
        CVec3f v3Forward(axis.Forward());
        CVec3f v3Start(pPlayer->GetPosition() + V_UP * pPlayer->GetViewHeight() + v3Forward * 30.0f);
        CVec3f v3End(v3Start + v3Forward * 100.0f);

        STraceInfo result;
        TraceLine(result, v3Start, v3End, 0, pPlayer->GetWorldIndex());
        if (result.uiEntityIndex != INVALID_INDEX)
            pTarget = GameClient.GetClientEntityCurrent(GameClient.GetGameIndexFromWorldIndex(result.uiEntityIndex));
    }

    if (m_uiGameTipExpireTime <= Host.GetTime())
        m_pInterfaceManager->HideGameTip();

    if (pTarget != NULL && !Game.IsCommander())
    {
        tstring sMessage(_T(""));
        if (pTarget->GetGameTip(pPlayer, sMessage))
        {
            m_uiGameTipExpireTime = Host.GetTime() + cg_gameTipDisplayTime;
            m_pInterfaceManager->ShowGameTip(sMessage);
        }
    }

    if (GetGameTime() - m_uiLastTeamTipTime > cg_teamTipReshowTime)
    {
        int iTeam(m_pLocalClient->GetTeam());
        int iEnemyTeam(iTeam ^ 3);
        CEntityTeamInfo *pTeam(GetTeam(iTeam));
        CEntityTeamInfo *pEnemyTeam(GetTeam(iEnemyTeam));


        if (pTeam != NULL && pEnemyTeam != NULL)
        {
            CStateString *pState(Host.GetStateString(1));   //STATE_STRING_SERVER_INFO

            if (pState != NULL)
            {
                if (pTeam->GetNumClients() > pEnemyTeam->GetNumClients() + pState->GetInt(_T("sv_maxTeamDifference")))
                {
                    m_uiLastTeamTipTime = GetGameTime();
                    m_uiGameTipExpireTime = Host.GetTime() + cg_teamTipShowTime;
                    m_pInterfaceManager->ShowGameTip(_T("Teams are currently unbalanced. Your team will have a spawn queue imposed upon it until teams are even again."));
                }
                else if (pEnemyTeam->GetNumClients() > pTeam->GetNumClients() + pState->GetInt(_T("sv_maxTeamDifference")))
                {
                    m_uiLastTeamTipTime = GetGameTime();
                    m_uiGameTipExpireTime = Host.GetTime() + cg_teamTipShowTime;
                    m_pInterfaceManager->ShowGameTip(_T("Teams are currently unbalanced. The opposing team will have a spawn queue imposed upon it until teams are even again."));
                }
            }
        }
    }

    // Display tool tips for why the player cannot build
    IBuildingEntity *pPreviewBuilding(GetBuildingEntity(pPlayer->GetPreviewBuildingIndex()));
    if (pPreviewBuilding != NULL && !m_sBuildFailure.empty())
    {
        m_uiGameTipExpireTime = Host.GetTime() + cg_gameTipDisplayTime;
        m_pInterfaceManager->ShowGameTip(m_sBuildFailure);
        m_sBuildFailure.clear();
    } 
}


/*====================
  CGameClient::UpdateHoverEntity
  ====================*/
void    CGameClient::UpdateHoverEntity()
{
    IPlayerEntity *pLocalPlayer(GetLocalPlayerCurrent());
    if (pLocalPlayer == NULL)
    {
        m_uiHoverEntity = INVALID_INDEX;
        return;
    }

    if (IsCommander())
        return;

    // Stop tracking a deleted entity
    if (m_uiHoverEntity != INVALID_INDEX)
    {
        if (!GetClientEntityCurrent(m_uiHoverEntity))
            m_uiHoverEntity = INVALID_INDEX;
    }

    // Snapcast overrides hover entity
    if (m_sSnap.uiLockedIndex != INVALID_INDEX)
    {
        m_uiHoverEntity = m_sSnap.uiLockedIndex;
        return;
    }

    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), m_pCamera->GetViewAxis(FORWARD), g_hoverEntityRange));

    STraceInfo trace;
    STraceInfo traceTrans;
    CVec3f v3TransStart(m_pCamera->GetOrigin());

    TraceLine(trace, v3TransStart, v3End, SURF_HULL | SURF_SHIELD | SURF_BLOCKER, pLocalPlayer->GetWorldIndex());
    TraceLine(traceTrans, v3TransStart, v3End, SURF_HULL | SURF_SHIELD | SURF_BLOCKER);

    // If our "transparent" trace hit our player entity, make us 50% transparent
    if (traceTrans.uiEntityIndex != trace.uiEntityIndex)
    {
        IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(traceTrans.uiEntityIndex));

        if (pEnt != NULL)
        {
            pEnt->AddClientRenderFlags(ECRF_HALFTRANSPARENT);
            m_setTransparent.insert(pEnt->GetIndex());
        }
    }

    uint uiTraceEntity(GetGameIndexFromWorldIndex(trace.uiEntityIndex));

    if (uiTraceEntity != INVALID_INDEX)
    {
        if (m_uiHoverEntity == uiTraceEntity)
        {
             // Hovering over the displayed entity extends the display time
            m_uiHoverEntityDisplayTime = Host.GetTime() + g_hoverEntityDisplayTime;
        }
        else if (m_uiHoverEntityAcquiring == uiTraceEntity)
        {
            // Hover over the acquiring to finish the lock
            if (m_uiHoverEntityAcquireTime <= Host.GetTime())
                m_uiHoverEntity = uiTraceEntity;
        }
        else
        {
            // A new aquiring entity, so reset acquire time
            m_uiHoverEntityAcquiring = uiTraceEntity;
            m_uiHoverEntityAcquireTime = Host.GetTime() + g_hoverEntityAcquireTime;
        }
    }
    else
    {
        // Didn't hit anything, reset anything we didn't finish acquiring
        m_uiHoverEntityAcquiring = INVALID_INDEX;

        if (m_uiHoverEntityDisplayTime <= Host.GetTime())
            m_uiHoverEntity = INVALID_INDEX;
    }
}


/*====================
  CGameClient::IsEntityHoverSelected
  ====================*/
bool    CGameClient::IsEntityHoverSelected(uint uiIndex)
{
    if (IsCommander())
        return m_pClientCommander->IsHoverSelected(uiIndex);
    else
        return false;
}


/*====================
  CGameClient::LooksLikeEnemy
  ====================*/
bool    CGameClient::LooksLikeEnemy(uint uiIndex)
{
    if (m_pLocalClient == NULL)
        return false;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    IVisualEntity *pEntity(GetVisualEntity(uiIndex));

    if (pPlayer != NULL && pEntity != NULL)
        return pPlayer->LooksLikeEnemy(pEntity);
    else
        return false;
}


/*====================
  CGameClient::GetLocalClientNum
  ====================*/
int     CGameClient::GetLocalClientNum()
{
    return m_pHostClient->GetClientNum();
}


/*====================
  CGameClient::UpdateOrders
  ====================*/
void    CGameClient::UpdateOrders()
{
    if (m_pLocalClient == NULL || m_pLocalClient->GetPlayerEntity() == NULL)
        return;

    CEntityTeamInfo *pTeam(GetTeam(m_pLocalClient->GetTeam()));
    int iRace(0);

    if (pTeam != NULL && LowerString(pTeam->GetDefinition()->GetName()) == _T("beast"))
        iRace = 1;

    //
    // Commander Orders
    //

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    
    if (pPlayer->GetOrderUniqueID() != m_yOrderUniqueID)
    {
        ResHandle hOrderSample(INVALID_INDEX);
        IVisualEntity *pEnt(NULL);
        if (pPlayer->GetCurrentOrderEntIndex() != INVALID_INDEX)
            pEnt = GetClientEntityCurrent(pPlayer->GetCurrentOrderEntIndex());


        // Delete any old orders
        if (m_uiOrderEvent != INVALID_INDEX)
        {
            Game.DeleteEvent(m_uiOrderEvent);
            m_uiOrderEvent = INVALID_INDEX;
        }

        m_yOrderUniqueID = pPlayer->GetOrderUniqueID();

        switch (pPlayer->GetCurrentOrder())
        {
        case CMDR_ORDER_CLEAR:
            break;

        case CMDR_ORDER_MOVE:
            {
                if (pEnt && pEnt->IsBuilding())
                {
                    if (pEnt->GetHealth() < pEnt->GetMaxHealth())
                        hOrderSample = m_hOrderRepairBuildingSample[iRace];
                    else
                        hOrderSample = m_hOrderDefendBuildingSample[iRace];
                }
                else
                {
                    hOrderSample = m_hOrderMoveSample[iRace];
                }

                CGameEvent ev;
                ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
                ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
                ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/waypoint.effect"), RES_EFFECT));
                ev.Spawn();
                
                m_uiOrderEvent = Game.AddEvent(ev);
            }
            break;
        case CMDR_ORDER_ATTACK:
            {
                if (pEnt && pEnt->IsBuilding())
                {
                    hOrderSample = m_hOrderAttackBuildingSample[iRace];
                }
                if (pEnt && (pEnt->IsPlayer() || pEnt->IsNpc() || pEnt->IsPet()))
                {
                    hOrderSample = m_hOrderAttackUnitSample[iRace];
                }
                else
                {
                    hOrderSample = m_hOrderAttackSample[iRace];
                }
                
                CGameEvent ev;
                ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
                ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
                ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/attack_waypoint.effect"), RES_EFFECT));
                ev.Spawn();
                
                m_uiOrderEvent = Game.AddEvent(ev);
            }
            break;
        }

        if (hOrderSample != INVALID_INDEX)
            K2SoundManager.Play2DSound(hOrderSample);
    }

    switch (pPlayer->GetCurrentOrder())
    {
    case CMDR_ORDER_MOVE:
        CurrentOrder.Trigger(_T("Move here"));
        break;
    case CMDR_ORDER_ATTACK:
        CurrentOrder.Trigger(_T("Attack here"));
        break;
    }

    //
    // Officer Orders
    //
    if (pPlayer->GetOfficerOrderChanged())
    {
        pPlayer->AcknowledgeOfficerOrder();

        // Delete any old events
        if (m_uiOfficerOrderEvent != INVALID_INDEX)
        {
            Game.DeleteEvent(m_uiOfficerOrderEvent);
            m_uiOfficerOrderEvent = INVALID_INDEX;
        }

        if (pPlayer->GetOfficerOrder() != OFFICERCMD_INVALID)
        {
            // Audio cue
            if (Host.GetTime() - m_uiLastOrderNotifyTime > cg_orderNotifyInterval)
            {
                K2SoundManager.Play2DSound(pPlayer->GetOfficerOrder() == OFFICERCMD_ATTACK ? m_hOrderAttackSample[iRace] : m_hOrderMoveSample[iRace]);
                m_uiLastOrderNotifyTime = Host.GetTime();
            }

            // Waypoint beam
            CGameEvent ev;
            ev.SetSourcePosition(pPlayer->GetOfficerOrderPos());
            ev.SetSourceEntity(pPlayer->GetOfficerOrderEntIndex());
            ev.SetEffect(pPlayer->GetOfficerOrder() == OFFICERCMD_ATTACK ? m_hWaypointOfficerAttackEffect : m_hWaypointOfficerMoveEffect);
            ev.Spawn();
            m_uiOfficerOrderEvent = Game.AddEvent(ev);
        }

        switch (pPlayer->GetOfficerOrder())
        {
        case OFFICERCMD_ATTACK:
            CurrentOfficerOrder.Trigger(_T("Attack here"));
            break;
        case OFFICERCMD_MOVE:
            CurrentOfficerOrder.Trigger(_T("Move here"));
            break;
        case OFFICERCMD_FOLLOW:
            CurrentOfficerOrder.Trigger(_T("Follow this unit"));
            break;
        case OFFICERCMD_DEFEND:
            CurrentOfficerOrder.Trigger(_T("Defend here"));
            break;
        case OFFICERCMD_RALLY:
            CurrentOfficerOrder.Trigger(_T("Rally"));
            break;
        }
    }
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

        if (pPlayer == NULL)
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

            if (pPlayer == NULL)
                continue;

            pPlayer = GetClientEntityCurrent(pPlayer->GetIndex());

            if (pPlayer == NULL)
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

        if (pPlayer != NULL)
            pPlayer = GetClientEntityCurrent(pPlayer->GetIndex());

        if (pPlayer != NULL)
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
    PROFILE(_T("CGameClient::DrawAltInfo"));

    if (m_pLocalClient == NULL || m_pLocalClient->GetPlayerEntity() == NULL)
        return;

    // Aquire the interface
    if (m_hAltInfo == INVALID_RESOURCE)
        return;
    CInterface *pInterface(UIManager.GetInterface(m_hAltInfo));
    if (pInterface == NULL)
        return;
    pInterface->SetAlwaysUpdate(true);

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    bool bCommander(IsCommander());

    CVec3f v3ViewPos(m_pCamera->GetOrigin());
    CRectf cRect(0.0f, 0.0f, float(Vid.GetScreenW()), float(Vid.GetScreenH()));

    static set<IVisualEntity *> setEntities;
    setEntities.clear();

    if (!AltInfo)
    {
        if (bCommander)
        {
            CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
            for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
            {
                IVisualEntity *pEntity(it->second->GetCurrentEntity());

                if (!pEntity || !pEntity->HasAltInfo() || !m_pClientCommander->IsHoverSelected(pEntity->GetIndex()))
                    continue;

                if (!pEntity->GetSighted())
                    continue;

                if ((pEntity->GetStatus() != ENTITY_STATUS_ACTIVE && pEntity->GetStatus() != ENTITY_STATUS_DEAD) && 
                    !(pEntity->IsBuilding() && pEntity->GetStatus() == ENTITY_STATUS_SPAWNING))
                    continue;

                if (pEntity->IsPlayer())
                {
                    IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());

                    if (pPlayer->GetClientID() == m_pHostClient->GetClientNum())
                        continue;
                }

                if (pEntity->IsStealthed() && pEntity->GetTeam() != pPlayer->GetTeam())
                    continue;

                CVec3f v3EntPos(pEntity->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pEntity->GetModelHandle()).GetMax().z * pEntity->GetScale() * pEntity->GetScale2()));
                CVec2f v2ScreenPos;

                if (m_pCamera->IsPointInScreenRect(v3EntPos, cRect) && m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
                    setEntities.insert(pEntity);
            }
        }
        else
        {
            IVisualEntity *pEntity(GetClientEntityCurrent(m_uiHoverEntity));

            if (!pEntity || !pEntity->HasAltInfo())
                return;

            if ((pEntity->GetStatus() != ENTITY_STATUS_ACTIVE && pEntity->GetStatus() != ENTITY_STATUS_DEAD) && 
                !(pEntity->IsBuilding() && pEntity->GetStatus() == ENTITY_STATUS_SPAWNING))
                return;

            if (pEntity->IsPlayer())
            {
                IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());

                if (pPlayer->GetClientID() == m_pHostClient->GetClientNum())
                    return;
            }

            if (pEntity->IsStealthed() && pEntity->GetTeam() != pPlayer->GetTeam())
                return;

            CVec3f v3EntPos(pEntity->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pEntity->GetModelHandle()).GetMax().z * pEntity->GetScale() * pEntity->GetScale2()));
            CVec2f v2ScreenPos;

            if (m_pCamera->IsPointInScreenRect(v3EntPos, cRect) && m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
                setEntities.insert(pEntity);
        }
    }
    else
    {
        CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
        for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
        {
            IVisualEntity *pEntity(it->second->GetCurrentEntity());

            if (!pEntity || !pEntity->HasAltInfo())
                continue;

            if ((pEntity->GetStatus() != ENTITY_STATUS_ACTIVE && pEntity->GetStatus() != ENTITY_STATUS_DEAD) && 
                !(pEntity->IsBuilding() && pEntity->GetStatus() == ENTITY_STATUS_SPAWNING))
                continue;

            // Don't show alt-info for local entities
            if (m_pClientEntityDirectory->IsLocalEntity(pEntity->GetIndex()))
                continue;

            if (pEntity->IsPlayer())
            {
                IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());

                if (pPlayer->GetClientID() == m_pHostClient->GetClientNum())
                    continue;
            }

            if (pEntity->IsStealthed() && pEntity->GetTeam() != pPlayer->GetTeam())
                continue;
            if (pEntity->GetIsHidden() && pEntity->GetTeam() != pPlayer->GetTeam())
                continue;

            if (bCommander && !pEntity->GetSighted())
                continue;

            CVec3f v3EntPos(pEntity->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pEntity->GetModelHandle()).GetMax().z * pEntity->GetScale() * pEntity->GetScale2()));
            CVec2f v2ScreenPos;

            if (!bCommander)
            {
                if (DistanceSq(v3EntPos, v3ViewPos) > SQR(float(g_altInfoRange)))
                    continue;

                STraceInfo trace;
                if (Game.TraceLine(trace, v3ViewPos, v3EntPos, SURF_BOUNDS | SURF_HULL | SURF_SHIELD | SURF_BLOCKER, pEntity->GetWorldIndex()))
                    continue;
            }       

            if (m_pCamera->IsPointInScreenRect(v3EntPos, cRect) && m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
                setEntities.insert(pEntity);
        }
    }

    for (set<IVisualEntity*>::iterator it(setEntities.begin()); it != setEntities.end(); ++it)
    {
        IVisualEntity *pEntity(*it);

        CVec3f v3EntPos(pEntity->GetPosition() + CVec3f(0.0f, 0.0f, g_ResourceManager.GetModelBounds(pEntity->GetModelHandle()).GetMax().z * pEntity->GetScale() * pEntity->GetScale2()));
        CVec2f v2ScreenPos;

        if (!m_pCamera->WorldToScreen(v3EntPos, v2ScreenPos))
            continue;

        v2ScreenPos.x = floor(v2ScreenPos.x);
        v2ScreenPos.y = floor(v2ScreenPos.y);

        m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_ALLY, !pPlayer->LooksLikeEnemy(pEntity));
        m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_NAME, pEntity->GetAltInfoName());
        m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_HEALTH_PERCENT, pEntity->GetHealthPercent());
        if (IsCommander())
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DISTANCE, -1);
        else
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DISTANCE, INT_ROUND(Distance(pPlayer->GetPosition(), pEntity->GetPosition()) / 39.37f));

        IBuildingEntity *pBuilding(pEntity->GetAsBuilding());
        IGadgetEntity *pGadget(pEntity->GetAsGadget());
        if (pBuilding != NULL)
        {
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUILD_PERCENT, pBuilding->GetBuildPercent());
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_UPKEEP, pBuilding->HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED));
        }
        else if (pGadget != NULL)
        {
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUILD_PERCENT, pGadget->GetRemainingLifetimePercent());
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_UPKEEP, false);
        }
        else
        {
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUILD_PERCENT, -1.0f);
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_UPKEEP, false);
        }

        int iBuffCount(0);
        int iDebuffCount(0);

        ICombatEntity *pTarget(pEntity->GetAsCombatEnt());
        if (IsCommander() && pTarget != NULL)
        {
            for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
            {
                IEntityState *pState(pTarget->GetState(i));
                if (pState == NULL)
                    continue;
                if (!pState->GetDisplayState())
                    continue;

                if (pState->IsBuff())
                {
                    m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUFF_ACTIVE, true, iBuffCount);
                    m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUFF_ICON, pState->GetIconPath(), iBuffCount);
                    ++iBuffCount;
                }
                else if (pState->IsDebuff())
                {
                    m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DEBUFF_ACTIVE, true, iDebuffCount);
                    m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DEBUFF_ICON, pState->GetIconPath(), iDebuffCount);
                    ++iDebuffCount;
                }
            }
        }

        for (int i(iBuffCount); i < MAX_DISPLAY_BUFFS; ++i)
        {
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUFF_ACTIVE, false, i);
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_BUFF_ICON, _T(""), i);
        }

        for (int i(iDebuffCount); i < MAX_DISPLAY_DEBUFFS; ++i)
        {
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DEBUFF_ACTIVE, false, i);
            m_pInterfaceManager->Trigger(UITRIGGER_ALT_INFO_DEBUFF_ICON, _T(""), i);
        }

        UIManager.Render(m_hAltInfo, v2ScreenPos);
    }

    pInterface->SetAlwaysUpdate(false);
}


/*====================
  CGameClient::UpdatePetInfo
  ====================*/
void    CGameClient::UpdatePetInfo()
{
    if (m_pLocalClient == NULL || m_pLocalClient->GetPlayerEntity() == NULL)
    {
        PetInfo.Hide();
        return;
    }

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());

    uint uiPetEntity(pPlayer->GetPetIndex());
    if (uiPetEntity == INVALID_INDEX || !GetClientEntityCurrent(uiPetEntity))
    {
        PetInfo.Hide();
        return;
    }
    
    IGameEntity *pEntity(GetClientEntityCurrent(uiPetEntity));

    if (!pEntity->IsPet())
    {
        PetInfo.Hide();
        return;
    }

    PetInfo.Show();

    IPetEntity *pPet(pEntity->GetAsPet());

    m_pInterfaceManager->Trigger(UITRIGGER_PET_NAME, pPet->GetEntityName());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_ICON, pPet->GetEntityIconPath());

    m_pInterfaceManager->Trigger(UITRIGGER_PET_HEALTH_VALUE, pPet->GetHealth());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_MANA_VALUE, pPet->GetMana());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_HEALTH_PERCENT, pPet->GetHealthPercent());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_MANA_PERCENT, pPet->GetManaPercent());
    
    m_pInterfaceManager->Trigger(UITRIGGER_PET_MAX_HEALTH, pPet->GetMaxHealth());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_MAX_MANA, pPet->GetMaxMana());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_MAX_STAMINA, pPet->GetMaxStamina());

    m_pInterfaceManager->Trigger(UITRIGGER_PET_ARMOR, pPet->GetArmor());
    m_pInterfaceManager->Trigger(UITRIGGER_PET_ARMOR_REDUCTION, pPet->GetArmorDamageReduction(pPet->GetArmor()));
}


/*====================
  CGameClient::UpdateBuildingPlacement
  ====================*/
void    CGameClient::UpdateBuildingPlacement(IBuildingEntity *pBuilding)
{
    if (pBuilding == NULL)
        return;

    if (m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL || !pPlayer->GetCanBuild())
        return;

    if (pPlayer->GetPreviewBuildingIndex() != pBuilding->GetIndex())
        return; 

    // Highlight other buildings in radius that would get affected by state
    if (!pBuilding->GetStateName().empty())
    {
        uivector vResult;
        Game.GetEntitiesInRadius(vResult, CSphere(pBuilding->GetPosition(), pBuilding->GetStateRadius()), SURF_MODEL);

        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IVisualEntity *pTestEntity(Game.GetEntityFromWorldIndex(*it));
            if (pTestEntity == NULL)
                continue;
            if (!pTestEntity->IsBuilding())
                continue;
            if (pTestEntity->GetStatus() != ENTITY_STATUS_ACTIVE && pTestEntity->GetStatus() != ENTITY_STATUS_SPAWNING)
                continue;
            if (pTestEntity->GetTeam() != m_pLocalClient->GetTeam())
                continue;
            if (pTestEntity->GetType() == pBuilding->GetType() && !pBuilding->GetStateselfapply())
                continue;

            pTestEntity->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pTestEntity->SetSelectColor(pBuilding->GetStatePreviewColor());
        }
    }

    // changes res handle to something other than INVALID_RESOURCE, then applies the effect
    CEntityTeamInfo *pTeam(Game.GetTeam(m_pLocalClient->GetTeam()));
    if (pTeam != NULL)
    {
        for (uiset_cit cit(pTeam->GetBuildingSet().begin()); cit != pTeam->GetBuildingSet().end(); ++cit)
        {
            IBuildingEntity *pTestBuilding(Game.GetBuildingEntity(*cit));
            if (pTestBuilding == NULL)
                continue;
            if (pTestBuilding->GetStatus() != ENTITY_STATUS_ACTIVE && pTestBuilding->GetStatus() != ENTITY_STATUS_SPAWNING)
                continue;
            if (pTestBuilding == pBuilding)
                continue;

            CClientEntity *pClientEnt(GetClientEntity(*cit));
            if (pClientEnt == NULL)
                continue;

            if (pTestBuilding->GetType() != pBuilding->GetType() && !pTestBuilding->GetStateEffectPreviewPath().empty())
                pClientEnt->SetStatePrevEffect(pTestBuilding->GetStateEffectPreviewPath());

            if (pTestBuilding->GetType() == pBuilding->GetType() && !pTestBuilding->GetUniqueRadiusEffectPath().empty())
                pClientEnt->SetNoBuildEffect(pTestBuilding->GetUniqueRadiusEffectPath());

            if (pTestBuilding->GetIsCommandStructure() && (pBuilding->GetMinimumDistanceFromCommandStructure() > 0) && pTestBuilding->GetTeam() == pPlayer->GetTeam() && !pTestBuilding->GetUniqueRadiusEffectPath().empty())
                pClientEnt->SetNoBuildEffect(pTestBuilding->GetUniqueRadiusEffectPath());
        }
    }

    // Cursor trace
    STraceInfo trace;
    CVec3f v3Dir(Game.GetCamera()->ConstructRay(Input.GetCursorPos()));
    CVec3f v3End(M_PointOnLine(Game.GetCamera()->GetOrigin(), v3Dir, FAR_AWAY));
    Game.TraceLine(trace, Game.GetCamera()->GetOrigin(), v3End, TRACE_TERRAIN | SURF_SHIELD);

    if (m_bBuildingRotate)
        trace.v3EndPos = m_v3BuildPosition;

    const tstring &sFoundation(pBuilding->GetFoundationType());

    if (!sFoundation.empty())
    {
        ushort usType(EntityRegistry.LookupID(sFoundation));

        float fBestDist(FAR_AWAY);
        IVisualEntity *pBest(NULL);

        CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
        for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
        {
            IVisualEntity *pEnt(it->second->GetCurrentEntity());
            if (pEnt == NULL)
                continue;

            if (pEnt->GetType() != usType || pEnt->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;

            float fDist(Length(pEnt->GetPosition() - trace.v3EndPos));
            if (fDist < fBestDist && fDist < 250.0f)
            {
                fBestDist = fDist;
                pBest = pEnt;
            }
        }

        if (pBest)
        {
            pBuilding->SetPosition(pBest->GetPosition());
            pBuilding->SetAngles(pBest->GetAngles());
            pBuilding->SetScale(pBest->GetScale() * pBuilding->GetBuildingScale());
            pBuilding->SetFoundation(pBest->GetIndex());
            pBuilding->AddClientRenderFlags(ECRF_SNAPSELECTED);

            m_bValidPosition = pBuilding->CanBuild(pPlayer, m_sBuildFailure);
            if (m_bValidPosition)
                pBuilding->SetSelectColor(CVec4f(0.0f, 2.0f, 0.0f, 1.0f));
            else
                pBuilding->SetSelectColor(CVec4f(2.0f, 0.0f, 0.0f, 1.0f));

            m_v3BuildPosition = pBest->GetPosition();
            m_v3BuildAngles = pBest->GetAngles();
            m_uiBuildFoundation = pBest->GetIndex();

            GameClient.GetCurrentSnapshot()->SetAngles(m_v3BuildAngles);
        }
        else
        {
            pBuilding->SetPosition(trace.v3EndPos);
            pBuilding->SetAngles(m_v3BuildAngles);
            pBuilding->SetScale(pBuilding->GetBuildingScale());
            pBuilding->AddClientRenderFlags(ECRF_SNAPSELECTED);
            pBuilding->SetSelectColor(CVec4f(2.0f, 0.0f, 0.0f, 1.0f));

            m_bValidPosition = false;
            m_v3BuildPosition = trace.v3EndPos;
            m_uiBuildFoundation = INVALID_INDEX;
        }
    }
    else
    {
        pBuilding->SetPosition(trace.v3EndPos);
        pBuilding->SetAngles(m_v3BuildAngles);
        //pBuilding->SetScale(pBuilding->GetBuildingScale());
        pBuilding->AddClientRenderFlags(ECRF_SNAPSELECTED);

        m_bValidPosition = pBuilding->CanBuild(pPlayer, m_sBuildFailure);
        if (m_bValidPosition)
            pBuilding->SetSelectColor(CVec4f(0.0f, 2.0f, 0.0f, 1.0f));
        else
            pBuilding->SetSelectColor(CVec4f(2.0f, 0.0f, 0.0f, 1.0f));

        m_v3BuildPosition = trace.v3EndPos;
        m_uiBuildFoundation = INVALID_INDEX;
    }
}


/*====================
  CGameClient::RotateBuildingPlacement
  ====================*/
void    CGameClient::RotateBuildingPlacement(float fValue)
{
    if (m_uiBuildFoundation != INVALID_INDEX)
        return;

    m_v3BuildAngles[YAW] += fValue * 0.2f;
    GameClient.GetCurrentSnapshot()->SetAngles(m_v3BuildAngles);
}


/*====================
  CGameClient::IsBuildModeActive
  ====================*/
bool    CGameClient::IsBuildModeActive()
{
    if (m_pLocalClient == NULL)
        return false;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL || !pPlayer->GetCanBuild())
        return false;

    IBuildingEntity *pBuilding(GetBuildingEntity(pPlayer->GetPreviewBuildingIndex()));
    if (pBuilding == NULL)
        return false;

    return true;
}


/*====================
  CGameClient::StartBuildingPlacement
  ====================*/
void    CGameClient::StartBuildingPlacement(ushort unType)
{
    if (m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL || !pPlayer->GetCanBuild())
        return;

    // Send the build request
    CBufferFixed<3> buffer;
    buffer << GAME_CMD_START_BUILDING << unType;
    GameClient.SendGameData(buffer, true);

    if (GetClientCommander() != NULL)
        GetClientCommander()->StartBuilding();
    m_v3BuildAngles = V3_ZERO;
    m_bBuildingRotate = false;

    GameClient.GetCurrentSnapshot()->SetAngles(m_v3BuildAngles);
}


/*====================
  CGameClient::TryBuildingPlacement
  ====================*/
bool    CGameClient::TryBuildingPlacement()
{
    if (m_pLocalClient == NULL)
        return false;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL || !pPlayer->GetCanBuild())
        return false;

    IBuildingEntity *pBuilding(GetBuildingEntity(pPlayer->GetPreviewBuildingIndex()));
    if (pBuilding == NULL)
        return false;

    // Test for an invalid placement
    if (!m_bValidPosition)
    {
        // Indicate failure
        if (m_hBuildingCannotPlaceSample != INVALID_RESOURCE)
            K2SoundManager.Play2DSound(m_hBuildingCannotPlaceSample);
        return false;
    }

    if (m_hBuildingPlacedSample != INVALID_RESOURCE)
        K2SoundManager.Play2DSound(m_hBuildingPlacedSample);

    if (GetClientCommander() != NULL)
        GetClientCommander()->StopBuilding();

    HideBuildMenu();

    // Send the build request
    CBufferFixed<31> buffer;
    buffer << GAME_CMD_PLACE_BUILDING << pBuilding->GetType() << m_v3BuildPosition << m_v3BuildAngles << m_uiBuildFoundation;
    GameClient.SendGameData(buffer, true);
    return true;
}


/*====================
  CGameClient::StopBuildingPlacement
  ====================*/
bool    CGameClient::StopBuildingPlacement()
{
    bool bWasBuilding(false);

    if (m_pLocalClient == NULL)
        return false;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return false;

    if (pPlayer->GetPreviewBuildingIndex() != INVALID_INDEX || pPlayer->HasNetFlags(ENT_NET_FLAG_BUILD_MODE))
    {
        bWasBuilding = true;
        m_sBuildFailure.clear();
        if (GetClientCommander() != NULL)
            GetClientCommander()->StopBuilding();

        CBufferFixed<1> buffer;
        buffer << GAME_CMD_STOP_BUILDING;
        GameClient.SendGameData(buffer, true);
    }

    m_CurrentClientSnapshot.SetAngles(V3_ZERO);

    return bWasBuilding;
}


/*====================
  CGameClient::SetBuildingUpkeepLevel
  ====================*/
void    CGameClient::SetBuildingUpkeepLevel(uint uiIndex, float fLevel)
{
    IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
    if (pBuilding == NULL)
        return;

    fLevel = CLAMP(fLevel, 0.0f, 1.0f);
    CBufferFixed<10>    buffer;
    buffer << GAME_CMD_SET_UPKEEP << uiIndex << fLevel;
    SendGameData(buffer, true);
}


/*====================
  CGameClient::ToggleBuildingUpkeep
  ====================*/
void    CGameClient::ToggleBuildingUpkeep(uint uiIndex)
{
    IBuildingEntity *pBuilding(GetBuildingEntity(uiIndex));
    if (pBuilding == NULL)
        return;

    CBufferFixed<10>    buffer;
    if (pBuilding->GetUpkeepLevel() > 0.0f)
        buffer << GAME_CMD_SET_UPKEEP << uiIndex << 0.0f;
    else
        buffer << GAME_CMD_SET_UPKEEP << uiIndex << 1.0f;

    SendGameData(buffer, true);
}


/*====================
  CGameClient::PromotePlayer
  ====================*/
void    CGameClient::PromotePlayer(int iClientID)
{
    if (!IsCommander())
        return;

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_PROMOTE_OFFICER << iClientID;
    SendGameData(buffer, true);
}


/*====================
  CGameClient::DemotePlayer
  ====================*/
void    CGameClient::DemotePlayer(byte ySquad)
{
    if (!IsCommander())
        return;

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_DEMOTE_OFFICER << ySquad;
    SendGameData(buffer, true);
}


/*====================
  CGameClient::SetVote
  ====================*/
void    CGameClient::SetVote(int iVote)
{
    CEntityClientInfo *pTargetClient(GetClientInfo(iVote));
    
    if (m_pLocalClient == NULL || pTargetClient == NULL)
        return;

    if (m_pLocalClient->GetTeam() != pTargetClient->GetTeam())
        return;

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_VOTE << iVote;
    SendGameData(buffer, true);
}


/*====================
  CGameClient::InvNext
  ====================*/
void    CGameClient::InvNext()
{
    IPlayerEntity *pPlayer(GameClient.GetLocalPlayer());
    if (!pPlayer)
        return;

    int iStart(CLAMP(m_CurrentClientSnapshot.GetSelectedItem(), 0, int(INVENTORY_START_BACKPACK)));
    for (int i(iStart + 1); i != iStart; ++i)
    {
        if (i >= INVENTORY_START_BACKPACK)
        {
            i = -1;
            continue;
        }

        if (pPlayer->GetItem(i) == NULL)
            continue;
        if (pPlayer->GetItem(i)->IsConsumable())
            continue;
        if (pPlayer->GetItem(i)->IsDisabled())
            continue;

        SelectItem(i);
        break;
    }
}


/*====================
  CGameClient::InvPrev
  ====================*/
void    CGameClient::InvPrev()
{
    IPlayerEntity *pPlayer(GameClient.GetLocalPlayer());
    if (!pPlayer)
        return;

    int iStart(CLAMP(m_CurrentClientSnapshot.GetSelectedItem(), 0, INVENTORY_START_BACKPACK - 1));
    for (int i(iStart - 1); i != iStart; --i)
    {
        if (i < 0)
        {
            i = INVENTORY_START_BACKPACK;
            continue;
        }
        
        if (pPlayer->GetItem(i) == NULL)
            continue;
        if (pPlayer->GetItem(i)->IsConsumable())
            continue;
        if (pPlayer->GetItem(i)->IsDisabled())
            continue;

        SelectItem(i);
        break;
    }
}


/*====================
  CGameClient::TeamHasBuilding
  ====================*/
bool    CGameClient::TeamHasBuilding(int iTeam, const tstring &sBuilding)
{
    if (sBuilding.empty())
        return true;

    ushort unType(EntityRegistry.LookupID(sBuilding));

    CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
    for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        IVisualEntity *pEnt(it->second->GetCurrentEntity());
        if (pEnt == NULL)
            continue;

        if (pEnt->GetTeam() == iTeam && pEnt->GetType() == unType && pEnt->GetStatus() == ENTITY_STATUS_ACTIVE)
            return true;
    }

    return false;
}


/*====================
  CGameClient::IsUnitAffordable
  ====================*/
bool    CGameClient::IsUnitAffordable(const tstring &sName)
{
    if (ICvar::GetBool(_T("sv_unitH4x")) || Game.GetGamePhase() == GAME_PHASE_WARMUP)
        return true;

    IPlayerEntity *pPlayer(GetLocalPlayer());

    if (pPlayer != NULL && sName == pPlayer->GetTypeName())
        return true;

    int iAvailableGold(m_pLocalClient->GetGold());
    if (pPlayer != NULL)
        iAvailableGold += INT_CEIL(pPlayer->GetCost() * ICvar::GetFloat(_T("g_unitTradeInRefund")));

    int iCost(ICvar::GetUnsignedInteger(sName + _T("_Cost")));

    return iAvailableGold >= iCost;
}


/*====================
  CGameClient::IsUnitAvailable
  ====================*/
bool    CGameClient::IsUnitAvailable(const tstring &sName)
{
    return (TeamHasBuilding(m_pLocalClient->GetTeam(), ICvar::GetString(sName + _T("_Prerequisite"))) || Game.GetGamePhase() == GAME_PHASE_WARMUP || ICvar::GetBool(_T("sv_unitH4x")) || ICvar::GetBool(_T("sv_allUnitsAvailable")));
}


/*====================
  CGameClient::GetUnitCost
  ====================*/
tstring CGameClient::GetUnitCost(const tstring &sName)
{
    if (ICvar::GetBool(_T("sv_unitH4x")) || Game.GetGamePhase() == GAME_PHASE_WARMUP)
        return _T("Free");

    if (!IsUnitAvailable(sName))
        return _T("N/A");

    IPlayerEntity *pPlayer(GetLocalPlayer());
    int iTradeIn(0);
    if (pPlayer != NULL)
        iTradeIn = INT_CEIL(pPlayer->GetCost() * ICvar::GetFloat(_T("g_unitTradeInRefund")));

    int iCost(ICvar::GetUnsignedInteger(sName + _T("_Cost")) - iTradeIn);

    if (IsUnitAffordable(sName))
    {
        if (iCost == 0 || (pPlayer != NULL && pPlayer->GetTypeName() == sName))
            return _T("Free");
        else if (iCost > 0)
            return XtoA(iCost);
        else
            return _T("^090") + XtoA(iCost);
    }
    else
    {
        return _TS("^r") + XtoA(iCost) + _T("^*");
    }
}


/*====================
  CGameClient::GetUnitsInService
  ====================*/
int     CGameClient::GetUnitsInService(const tstring &sName)
{
    ushort unType(EntityRegistry.LookupID(sName));
    int iTeam(m_pLocalClient->GetTeam());

    int iCount(0);

    CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
    for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        IVisualEntity *pEnt(it->second->GetCurrentEntity());
        if (pEnt == NULL)
            continue;

        if (pEnt->GetTeam() == iTeam && pEnt->GetType() == unType && pEnt->GetStatus() != ENTITY_STATUS_CORPSE)
            ++iCount;
    }

    return iCount;
}


/*====================
  CGameClient::GetUnitsInServiceSquad
  ====================*/
int     CGameClient::GetUnitsInServiceSquad(const tstring &sName)
{
    ushort unType(EntityRegistry.LookupID(sName));
    int iTeam(m_pLocalClient->GetTeam());
    int iSquad(m_pLocalClient->GetSquad());

    int iCount(0);

    CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
    for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
    {
        IVisualEntity *pEnt(it->second->GetCurrentEntity());
        if (pEnt == NULL)
            continue;

        if (pEnt->GetTeam() == iTeam && pEnt->GetSquad() == iSquad && pEnt->GetType() == unType && pEnt->GetStatus() != ENTITY_STATUS_CORPSE)
            ++iCount;
    }

    return iCount;
}


/*====================
  CGameClient::UpdateLoadout
  ====================*/
void    CGameClient::UpdateLoadout()
{
    if (m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());

    int iTradeIn(0);
    if (pPlayer != NULL)
        iTradeIn = INT_CEIL(pPlayer->GetCost() * ICvar::GetFloat(_T("g_unitTradeInRefund")));

    if (m_pPreviewUnit != NULL && pPlayer != NULL)
    {
        int iCost(m_pPreviewUnit->GetCost());
        
        int iNetCost;
        
        if (pPlayer->GetType() != m_pPreviewUnit->GetType())
            iNetCost = (iCost - iTradeIn);
        else
            iNetCost = 0;

        LoadoutCost.Trigger(XtoA(iCost));
        LoadoutNetCost.Trigger(XtoA(iNetCost));
        LoadoutPurchase.Trigger(XtoA(m_pLocalClient->GetGold() >= iNetCost, true));
    }
    else
    {
        LoadoutCost.Trigger(_T("-"));
        LoadoutNetCost.Trigger(_T("-"));
        LoadoutPurchase.Trigger(XtoA(false));
    }

    LoadoutTradeIn.Trigger(XtoA(iTradeIn));

    //
    // Humans
    //

    LoadoutUnitBuilderAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Engineer"))));
    LoadoutUnitScoutAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Marksman"))));
    LoadoutUnitSavageAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Savage"))));
    LoadoutUnitLegionnaireAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Legionnaire"))));
    LoadoutUnitChaplainAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Chaplain"))));
    LoadoutUnitBatteringramAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Batteringram"))));
    LoadoutUnitSteambuchetAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Steambuchet"))));

    LoadoutUnitBuilderAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Engineer"))));
    LoadoutUnitScoutAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Marksman"))));
    LoadoutUnitSavageAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Savage"))));
    LoadoutUnitLegionnaireAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Legionnaire"))));
    LoadoutUnitChaplainAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Chaplain"))));
    LoadoutUnitBatteringramAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Batteringram"))));
    LoadoutUnitSteambuchetAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Steambuchet"))));

    LoadoutUnitBuilderCost.Trigger(XtoA(GetUnitCost(_T("Player_Engineer"))));
    LoadoutUnitScoutCost.Trigger(XtoA(GetUnitCost(_T("Player_Marksman"))));
    LoadoutUnitSavageCost.Trigger(XtoA(GetUnitCost(_T("Player_Savage"))));
    LoadoutUnitLegionnaireCost.Trigger(XtoA(GetUnitCost(_T("Player_Legionnaire"))));
    LoadoutUnitChaplainCost.Trigger(XtoA(GetUnitCost(_T("Player_Chaplain"))));
    LoadoutUnitBatteringramCost.Trigger(XtoA(GetUnitCost(_T("Player_Batteringram"))));
    LoadoutUnitSteambuchetCost.Trigger(XtoA(GetUnitCost(_T("Player_Steambuchet"))));

    LoadoutUnitBuilderInService.Trigger(XtoA(GetUnitsInService(_T("Player_Engineer"))));
    LoadoutUnitScoutInService.Trigger(XtoA(GetUnitsInService(_T("Player_Marksman"))));
    LoadoutUnitSavageInService.Trigger(XtoA(GetUnitsInService(_T("Player_Savage"))));
    LoadoutUnitLegionnaireInService.Trigger(XtoA(GetUnitsInService(_T("Player_Legionnaire"))));
    LoadoutUnitChaplainInService.Trigger(XtoA(GetUnitsInService(_T("Player_Chaplain"))));
    LoadoutUnitBatteringramInService.Trigger(XtoA(GetUnitsInService(_T("Player_Batteringram"))));
    LoadoutUnitSteambuchetInService.Trigger(XtoA(GetUnitsInService(_T("Player_Steambuchet"))));

    LoadoutUnitBuilderInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Engineer"))));
    LoadoutUnitScoutInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Marksman"))));
    LoadoutUnitSavageInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Savage"))));
    LoadoutUnitLegionnaireInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Legionnaire"))));
    LoadoutUnitChaplainInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Chaplain"))));
    LoadoutUnitBatteringramInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Batteringram"))));
    LoadoutUnitSteambuchetInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Steambuchet"))));

    //
    // Beasts
    //

    LoadoutUnitConjurerAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Conjurer"))));
    LoadoutUnitShapeshifterAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Shapeshifter"))));
    LoadoutUnitSummonerAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Summoner"))));
    LoadoutUnitPredatorAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Predator"))));
    LoadoutUnitShamanAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Shaman"))));
    LoadoutUnitBehemothAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Behemoth"))));
    LoadoutUnitSteambuchetAffordable.Trigger(XtoA(IsUnitAffordable(_T("Player_Steambuchet"))));

    LoadoutUnitConjurerAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Conjurer"))));
    LoadoutUnitShapeshifterAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Shapeshifter"))));
    LoadoutUnitSummonerAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Summoner"))));
    LoadoutUnitPredatorAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Predator"))));
    LoadoutUnitShamanAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Shaman"))));
    LoadoutUnitBehemothAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Behemoth"))));
    LoadoutUnitTempestAvailable.Trigger(XtoA(IsUnitAvailable(_T("Player_Tempest"))));

    LoadoutUnitConjurerCost.Trigger(XtoA(GetUnitCost(_T("Player_Conjurer"))));
    LoadoutUnitShapeshifterCost.Trigger(XtoA(GetUnitCost(_T("Player_Shapeshifter"))));
    LoadoutUnitSummonerCost.Trigger(XtoA(GetUnitCost(_T("Player_Summoner"))));
    LoadoutUnitPredatorCost.Trigger(XtoA(GetUnitCost(_T("Player_Predator"))));
    LoadoutUnitShamanCost.Trigger(XtoA(GetUnitCost(_T("Player_Shaman"))));
    LoadoutUnitBehemothCost.Trigger(XtoA(GetUnitCost(_T("Player_Behemoth"))));
    LoadoutUnitTempestCost.Trigger(XtoA(GetUnitCost(_T("Player_Tempest"))));

    LoadoutUnitConjurerInService.Trigger(XtoA(GetUnitsInService(_T("Player_Conjurer"))));
    LoadoutUnitShapeshifterInService.Trigger(XtoA(GetUnitsInService(_T("Player_Shapeshifter"))));
    LoadoutUnitSummonerInService.Trigger(XtoA(GetUnitsInService(_T("Player_Summoner"))));
    LoadoutUnitPredatorInService.Trigger(XtoA(GetUnitsInService(_T("Player_Predator"))));
    LoadoutUnitShamanInService.Trigger(XtoA(GetUnitsInService(_T("Player_Shaman"))));
    LoadoutUnitBehemothInService.Trigger(XtoA(GetUnitsInService(_T("Player_Behemoth"))));
    LoadoutUnitTempestInService.Trigger(XtoA(GetUnitsInService(_T("Player_Tempest"))));

    LoadoutUnitConjurerInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Conjurer"))));
    LoadoutUnitShapeshifterInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Shapeshifter"))));
    LoadoutUnitSummonerInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Summoner"))));
    LoadoutUnitPredatorInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Predator"))));
    LoadoutUnitShamanInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Shaman"))));
    LoadoutUnitBehemothInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Behemoth"))));
    LoadoutUnitTempestInServiceSquad.Trigger(XtoA(GetUnitsInServiceSquad(_T("Player_Tempest"))));

    LoadoutSpawn.Trigger(XtoA(pPlayer != NULL && !pPlayer->IsObserver(), true));
}


/*====================
  CGameClient::SetPreviewUnit
  ====================*/
void    CGameClient::SetPreviewUnit(ushort unType)
{   
    if (m_pPreviewUnit != NULL)
    {
        m_pClientEntityDirectory->Delete(m_pPreviewUnit->GetIndex());
        m_pPreviewUnit = NULL;
    }

    IGameEntity *pNew(m_pClientEntityDirectory->AllocateLocal(unType));

    CClientEntity *pNewUnit(pNew ? m_pClientEntityDirectory->GetClientEntity(pNew->GetIndex()) : NULL);

    if (pNewUnit == NULL)
        return;
    if (!pNewUnit->GetNextEntity()->IsPlayer())
        m_pClientEntityDirectory->Delete(pNewUnit->GetNextEntity()->GetIndex());

    m_pPreviewUnit = pNewUnit->GetCurrentEntity()->GetAsPlayerEnt();

    pNewUnit->SetClientEntity(true);
    pNewUnit->GetPrevEntity()->Validate();
    pNewUnit->GetNextEntity()->Validate();
    pNewUnit->GetCurrentEntity()->Validate();
}


/*====================
  CGameClient::SendMessage
  ====================*/
void    CGameClient::SendMessage(const tstring &sMsg, int iClientNum)
{
    if (iClientNum == -1 || iClientNum == GetLocalClientNum())
        GameMessage.Trigger(sMsg);
}


/*====================
  CGameClient::DrawLittleTextPopupMessages
  ====================*/
void    CGameClient::DrawLittleTextPopupMessages()
{
    const uint LIFETIME(3000);

    for (vector<SLittleTextPopupMessage>::iterator it(m_vLittleTextPopupMessage.begin()); it != m_vLittleTextPopupMessage.end(); )
    {
        if (it->uiSpawnTime + LIFETIME < Game.GetGameTime())
            it = m_vLittleTextPopupMessage.erase(it);
        else
            ++it;
    }

    CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hLittleTextPopupFont));

    for (vector<SLittleTextPopupMessage>::iterator it(m_vLittleTextPopupMessage.begin()); it != m_vLittleTextPopupMessage.end(); ++it)
    {
        CVec2f v2ScreenPos;
        float fLerp(float(Game.GetGameTime() - it->uiSpawnTime) / LIFETIME);
        
        if (m_pCamera->WorldToScreen(it->V3Pos + CVec3f(0.0f, 0.0f, 30.0f) * fLerp, v2ScreenPos))
        {
            CVec2f v2StringPos(ROUND(v2ScreenPos.x - pFontMap->GetStringWidth(it->sText) / 2.0f), ROUND(v2ScreenPos.y - pFontMap->GetMaxHeight() / 2.0f));
            float fAlpha(LERP(fLerp, 1.0f, 0.0f));

            Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, fAlpha));
            Draw2D.String(v2StringPos.x + 2, v2StringPos.y + 2, it->sText, m_hLittleTextPopupFont);

            Draw2D.SetColor(CVec4f(it->v4Color.xyz(), fAlpha));
            Draw2D.String(v2StringPos.x, v2StringPos.y, it->sText, m_hLittleTextPopupFont);
        }
    }
}


/*====================
  CGameClient::SpawnLittleTextPopupMessage
  ====================*/
void    CGameClient::SpawnLittleTextPopupMessage(const tstring &sText, const CVec3f &v3Pos, const CVec4f &v4Color)
{
    SLittleTextPopupMessage ltpm;
    ltpm.sText = sText;
    ltpm.uiSpawnTime = Game.GetGameTime();

    CVec2f v2ScreenPos0;
    CVec2f v2ScreenPos1;
    float fScale(1.0f);

    if (m_pCamera->WorldToScreen(v3Pos, v2ScreenPos0) &&
        m_pCamera->WorldToScreen(v3Pos + m_pCamera->GetViewAxis(UP), v2ScreenPos1))
    {
        fScale = 1.0f / Distance(v2ScreenPos0, v2ScreenPos1);
    }

    ltpm.V3Pos = v3Pos + (m_pCamera->GetViewAxis(RIGHT) * M_Randnum(-20.0f, 20.0f) + m_pCamera->GetViewAxis(UP) * M_Randnum(-20.0f, 20.0f)) * fScale;
    ltpm.v4Color = v4Color;

    m_vLittleTextPopupMessage.push_back(ltpm);
}


/*====================
  CGameClient::SpawnMinimapPing
  ====================*/
void    CGameClient::SpawnMinimapPing(ResHandle hTexture, const CVec2f &v2Pos, const CVec4f &v4Color, bool bPlaySound)
{
    SMinimapPing ping;
    ping.hTexture = hTexture;
    ping.v2Pos = v2Pos;
    ping.v4Color = v4Color;
    ping.uiSpawnTime = Game.GetGameTime();

    m_vMinimapPing.push_back(ping);

    if (bPlaySound)
        K2SoundManager.Play2DSound(m_hPingSample);
}


/*====================
  CGameClient::HitFeedback
  ====================*/
void    CGameClient::HitFeedback(EHitFeedbackType eType, uint uiIndex)
{
    ResHandle hSample(INVALID_RESOURCE);
    ResHandle hEffect(INVALID_RESOURCE);

    switch (eType)
    {
    case HIT_GENERIC:
        hSample = m_hMeleeHitUnitSample;
        hEffect = m_hMeleeHitEffect;
        break;

    case HIT_MELEE:
        hSample = m_hMeleeHitUnitSample;
        hEffect = m_hMeleeHitEffect;
        break;

    case HIT_RANGED:
        hSample = m_hRangedHitSample;
        hEffect = m_hRangedHitEffect;
        break;

    case HIT_UNIT_RANGED:
        hSample = m_hRangedHitUnitSample;
        hEffect = m_hRangedHitUnitEffect;
        break;

    case HIT_BUILDING_MELEE:
        hSample = m_hMeleeHitBuildingSample;
        hEffect = m_hMeleeHitBuildingEffect;
        break;

    case HIT_BUILDING_RANGED:
        hSample = m_hRangedHitBuildingSample;
        hEffect = m_hRangedHitBuildingEffect;
        break;
    
    case HIT_BLOCKED:
        hEffect = m_hBlockedFeedbackEffect;
        break;

    case HIT_GOT_BLOCKED:
        hEffect = m_hGotBlockedFeedbackEffect;
        break;

    case HIT_INTERRUPTED:
        hEffect = m_hInterruptedFeedbackEffect;
        break;

    case HIT_GOT_INTERRUPTED:
        hEffect = m_hGotInterruptedFeedbackEffect;
        break;

    case HIT_MELEEHIT:
        if (cg_meleeHitSounds)
            hSample = m_hMeleeHitUnitSample;
        hEffect = m_hMeleeHitFeedbackEffect;
        break;

    case HIT_GOT_HITBYMELEE:
        hEffect = m_hGotHitByMeleeFeedbackEffect;
        break;

    case HIT_GOT_HITBYMELEE_NOKICK:
        hEffect = m_hGotHitByMeleeFeedbackEffectNoKick;
        break;
    }


    if (hSample != INVALID_RESOURCE && Game.GetGameTime() - m_uiLastHitSoundNotificationTime > cg_hitNotificationInterval)
    {
        K2SoundManager.Play2DSound(hSample);
        m_uiLastHitSoundNotificationTime = Game.GetGameTime();
    }

    if (hEffect != INVALID_RESOURCE)
    {
        CClientEntity *pEntity(m_pClientEntityDirectory->GetClientEntity(uiIndex));

        if (pEntity)
            pEntity->StartEffect(hEffect, -1, 0);
    }
}


/*====================
  CGameClient::HitFeedback
  ====================*/
void    CGameClient::HitFeedback(EHitFeedbackType eType, const CVec3f &v3Pos)
{
    ResHandle hEffect(m_hMeleeImpactFeedbackEffect);

    if (hEffect != INVALID_RESOURCE && cg_impactFeedback)
    {
        CGameEvent ev;
        ev.SetEffect(hEffect);
        ev.SetSourcePosition(v3Pos);
        ev.Spawn();
        Game.AddEvent(ev);
    }
}


/*====================
  CGameClient::PrecacheEntities
  ====================*/
void    CGameClient::PrecacheEntities()
{
    PROFILE_EX("CGameClient::PrecacheEntities", PROFILE_GAME_PRECACHE_ENTITIES);

    if (!cg_precacheEntities)
        return;

    // Only load the game_settings script if we're not connecting locally,
    // since if we're connecting locally the server will have already set these
    if (!Host.HasServer())
        Console.ExecuteScript(_T("/game_settings.cfg"));

    svector vPrecacheList;

    // Human buildings
    vPrecacheList.push_back(_T("Building_Academy"));
    vPrecacheList.push_back(_T("Building_Armory"));
    vPrecacheList.push_back(_T("Building_ArrowTower"));
    vPrecacheList.push_back(_T("Building_CannonTower"));
    vPrecacheList.push_back(_T("Building_Garrison"));
    vPrecacheList.push_back(_T("Building_HumanHellShrine"));
    vPrecacheList.push_back(_T("Building_Monastery"));
    vPrecacheList.push_back(_T("Building_ShieldTower"));
    vPrecacheList.push_back(_T("Building_SiegeWorkshop"));
    vPrecacheList.push_back(_T("Building_SteamMine"));
    vPrecacheList.push_back(_T("Building_Stronghold"));

    // Human units
    vPrecacheList.push_back(_T("Player_BatteringRam"));
    vPrecacheList.push_back(_T("Player_Chaplain"));
    vPrecacheList.push_back(_T("Player_Engineer"));
    vPrecacheList.push_back(_T("Player_Legionnaire"));
    vPrecacheList.push_back(_T("Player_Marksman"));
    vPrecacheList.push_back(_T("Player_Savage"));
    vPrecacheList.push_back(_T("Player_Steambuchet"));

    vPrecacheList.push_back(_T("Pet_HumanWorker"));

    vPrecacheList.push_back(_T("Gadget_HumanOfficerSpawnFlag"));

    // Beast buildings
    vPrecacheList.push_back(_T("Building_PredatorDen"));
    vPrecacheList.push_back(_T("Building_Nexus"));
    vPrecacheList.push_back(_T("Building_StrataSpire"));
    vPrecacheList.push_back(_T("Building_EntangleSpire"));
    vPrecacheList.push_back(_T("Building_Sublair"));
    vPrecacheList.push_back(_T("Building_HumanHellShrine"));
    vPrecacheList.push_back(_T("Building_Sanctuary"));
    vPrecacheList.push_back(_T("Building_ChlorophilicSpire"));
    vPrecacheList.push_back(_T("Building_CharmShrine"));
    vPrecacheList.push_back(_T("Building_GroveMine"));
    vPrecacheList.push_back(_T("Building_Lair"));

    // Beast units
    vPrecacheList.push_back(_T("Player_Conjurer"));
    vPrecacheList.push_back(_T("Player_Shapeshifter"));
    vPrecacheList.push_back(_T("Player_Summoner"));
    vPrecacheList.push_back(_T("Player_Shaman"));
    vPrecacheList.push_back(_T("Player_Predator"));
    vPrecacheList.push_back(_T("Player_Behemoth"));
    vPrecacheList.push_back(_T("Player_Tempest"));

    vPrecacheList.push_back(_T("Pet_BeastWorker"));

    vPrecacheList.push_back(_T("Gadget_BeastSpawnPortal"));

    // Hellbourne units
    vPrecacheList.push_back(_T("Player_Revenant"));
    vPrecacheList.push_back(_T("Player_Malphas"));

    // Observer
    vPrecacheList.push_back(_T("Player_Observer"));
    
    // Commander
    vPrecacheList.push_back(_T("Player_Commander"));

    // Officer
    vPrecacheList.push_back(_T("State_Officer"));
    vPrecacheList.push_back(_T("State_OfficerAura"));

    // Persistant items
    vPrecacheList.push_back(_T("Persistant_Item"));

    // Consumable items
    vPrecacheList.push_back(_T("Consumable_AmmoPack"));
    vPrecacheList.push_back(_T("Consumable_AmmoSatchel"));
    vPrecacheList.push_back(_T("Consumable_Chainmail"));
    vPrecacheList.push_back(_T("Consumable_HealthMajor"));
    vPrecacheList.push_back(_T("Consumable_HealthMinor"));
    vPrecacheList.push_back(_T("Consumable_HealthReplenish"));
    vPrecacheList.push_back(_T("Consumable_HealthShrine"));
    vPrecacheList.push_back(_T("Consumable_ManaClarity"));
    vPrecacheList.push_back(_T("Consumable_ManaMajor"));
    vPrecacheList.push_back(_T("Consumable_ManaMinor"));
    vPrecacheList.push_back(_T("Consumable_ManaShrine"));
    vPrecacheList.push_back(_T("Consumable_Platemail"));
    vPrecacheList.push_back(_T("Consumable_SpeedBoost"));
    vPrecacheList.push_back(_T("Consumable_StaminaMajor"));
    vPrecacheList.push_back(_T("Consumable_StaminaMinor"));
    vPrecacheList.push_back(_T("Consumable_ToughSkin"));
    vPrecacheList.push_back(_T("Consumable_StoneHide"));
    vPrecacheList.push_back(_T("Consumable_ManaCrystal"));
    vPrecacheList.push_back(_T("Consumable_ManaStone"));
    vPrecacheList.push_back(_T("Consumable_LynxFeet"));

    vPrecacheList.push_back(_T("Entity_Soul"));
    vPrecacheList.push_back(_T("Entity_Chest"));
    vPrecacheList.push_back(_T("Prop_Mine"));
    vPrecacheList.push_back(_T("Prop_Scar"));
    vPrecacheList.push_back(_T("State_Dash"));

    class CLoadEntityFunctions : public CLoadJob<svector, tstring>::IFunctions
    {
    public:
        float   Frame(svector_it &it, float f) const
        {
            UIManager.SetActiveInterface(_T("host_connecting"));
            SetTitle(_T("Loading entity resources"));
            SetDescription(*it);
            SetProgress(f);
            return 0.0f;
        }

        float   PostFrame(svector_it &it, float f) const    { EntityRegistry.ClientPrecache(EntityRegistry.LookupID(*it)); ++it; return 1.0f; }
    };
    CLoadEntityFunctions fnLoadEntity;
    CLoadJob<svector, tstring> jobLoadEntities(vPrecacheList, &fnLoadEntity, false);
    jobLoadEntities.Execute(vPrecacheList.size());


    class CLoadWorldFunctions : public CLoadJob<svector, tstring>::IFunctions
    {
    private:
        tstring         m_sTitle;
        EResourceType   m_eType;

    public:
        CLoadWorldFunctions(const tstring &sTitle, EResourceType e) : m_sTitle(sTitle), m_eType(e)  {}

        float   Frame(svector_it &it, float f) const        { SetTitle(m_sTitle); SetDescription(*it); SetProgress(f); return 0.0f; }
        float   PostFrame(svector_it &it, float f) const
        {
            ResHandle h(g_ResourceManager.Register(*it, m_eType));
            if (m_eType == RES_MODEL)
                g_ResourceManager.PrecacheSkin(h, -1);
            else if (m_eType == RES_NPC)
                g_ResourceManager.PrecacheNPC(h);
            ++it;
            return 1.0f;
        }
    };


    // Precache misc. effects not directly attached to an entity
    vPrecacheList.clear();

    vPrecacheList.push_back(_T("/shared/effects/levelup.effect"));
    vPrecacheList.push_back(_T("/shared/effects/bloodimpact_small.effect"));
    vPrecacheList.push_back(_T("/shared/effects/bloodspray_small.effect"));
    vPrecacheList.push_back(_T("/shared/effects/blocked.effect"));
    vPrecacheList.push_back(_T("/shared/effects/ping_target.effect"));
    vPrecacheList.push_back(_T("/shared/effects/attack_waypoint.effect"));
    vPrecacheList.push_back(_T("/shared/effects/waypoint.effect"));
    vPrecacheList.push_back(_T("/shared/effects/move_indicator.effect"));
    vPrecacheList.push_back(_T("/shared/effects/attack_indicator.effect"));
    vPrecacheList.push_back(_T("/shared/effects/team_player.effect"));
    vPrecacheList.push_back(_T("/shared/effects/team_building.effect"));
    vPrecacheList.push_back(_T("/shared/effects/enemy_player.effect"));
    vPrecacheList.push_back(_T("/shared/effects/enemy_building.effect"));
    vPrecacheList.push_back(_T("/shared/effects/eye.effect"));
    vPrecacheList.push_back(_T("/shared/effects/rain2.effect"));
    vPrecacheList.push_back(_T("/shared/effects/rain_splash.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_melee.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_ranged.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_melee_unit.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_ranged_unit.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_melee_building.effect"));
    vPrecacheList.push_back(_T("/shared/effects/hit_ranged_building.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_blocked.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_gotblocked.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_interrupted.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_gotinterrupted.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_meleehit.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_gothitbymelee.effect"));
    vPrecacheList.push_back(_T("/shared/effects/feedback_melee_impact.effect"));
    vPrecacheList.push_back(_T("/shared/effects/charge.effect"));
    
    FileManager.GetFileList(_T("/shared/effects/"), _T("footstep_l_small*.effect"), false, vPrecacheList);
    FileManager.GetFileList(_T("/shared/effects/"), _T("footstep_r_small*.effect"), false, vPrecacheList);

    CLoadWorldFunctions fnLoadMisc(_T("Loading Effects"), RES_EFFECT);
    CLoadJob<svector, tstring>  jobMisc(vPrecacheList, &fnLoadMisc, false);
    jobMisc.Execute(vPrecacheList.size());


    // Precache terrain footstep sounds
    vPrecacheList.clear();

    FileManager.GetFileList(_T("/shared/sounds/footsteps/"), _T("*.mp3"), false, vPrecacheList);
    
    CLoadWorldFunctions fnLoadSounds(_T("Loading Terrain Sounds"), RES_SAMPLE);
    CLoadJob<svector, tstring>  jobSounds(vPrecacheList, &fnLoadSounds, false);
    jobSounds.Execute(vPrecacheList.size());

    // Precache environmental effects
    vPrecacheList.clear();

    int iLoop(1);
    while (FileManager.Exists(_T("/world/sky/lightning") + XtoA(iLoop) + _T(".mdf")))
        vPrecacheList.push_back(_T("/world/sky/lightning") + XtoA(iLoop++) + _T(".mdf"));

    iLoop = 1;
    while (FileManager.Exists(_T("/world/sky/lightning_top") + XtoA(iLoop) + _T(".mdf")))
        vPrecacheList.push_back(_T("/world/sky/lightning_top") + XtoA(iLoop++) + _T(".mdf"));

    vPrecacheList.push_back(_T("/world/sky/sky.mdf"));
    vPrecacheList.push_back(_T("/world/sky/sky_top.mdf"));

    CLoadWorldFunctions fnLoadEnvironmental(_T("Loading Environmental Effects"), RES_MODEL);
    CLoadJob<svector, tstring>  jobEnvironmental(vPrecacheList, &fnLoadEnvironmental, false);
    jobEnvironmental.Execute(vPrecacheList.size());
}


/*====================
  CGameClient::DrawLocator
  ====================*/
void    CGameClient::DrawLocator(const CVec3f &v3Pos, ResHandle hLocator, const CVec4f &v4LocatorColor, float fLocatorSize, const tstring &sText, const CVec4f &v4TextColor)
{
    if (!m_pCamera)
        return;

    CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hLocatorFont));

    CVec2f v2ScreenPos(0.0f, 0.0f);
    bool bGoodPos(m_pCamera->WorldToScreen(v3Pos, v2ScreenPos));

    CRectf recLocator(ROUND(v2ScreenPos.x - fLocatorSize / 2.0f),
        ROUND(v2ScreenPos.y - fLocatorSize / 2.0f),
        ROUND(v2ScreenPos.x - fLocatorSize / 2.0f) + ROUND(fLocatorSize),
        ROUND(v2ScreenPos.y - fLocatorSize / 2.0f) + ROUND(fLocatorSize));

    CRectf recBounds(recLocator);
    if (recBounds.GetWidth() < pFontMap->GetStringWidth(sText))
    {
        recBounds.SetSizeX(pFontMap->GetStringWidth(sText) + 4.0f);
        recBounds.Center(v2ScreenPos.x, v2ScreenPos.y);
    }
    recBounds.StretchY(pFontMap->GetMaxHeight() + 5.0f);

    CRectf recScreen(recBounds.GetWidth() / 2.0f,
        recLocator.GetHeight() / 2.0f,
        Draw2D.GetScreenW() - recBounds.GetWidth() / 2.0f,
        Draw2D.GetScreenH() - recLocator.GetHeight() / 2.0f - (pFontMap->GetMaxHeight() + 5.0f)
    );

    if (!bGoodPos || recBounds.left < 0.0f || recBounds.top < 0.0f || recBounds.right > Draw2D.GetScreenW() || recBounds.bottom > Draw2D.GetScreenH())
    {
        CVec2f v2ViewPos;
        CVec3f v3Local(v3Pos - m_pCamera->GetOrigin());
        CVec3f v3ViewAngles(M_GetAnglesFromForwardVec(m_pCamera->GetViewAxis(FORWARD)));

        {
            CVec3f v3Angles(M_GetAnglesFromForwardVec(v3Local));
            v3Angles[YAW] = v3ViewAngles[YAW];

            CAxis aAxis(v3Angles);

            CVec3f v3Transformed
            (
                DotProduct(v3Local, aAxis[RIGHT]),
                DotProduct(v3Local, aAxis[UP]),
                DotProduct(v3Local, aAxis[FORWARD])
            );

            v3Transformed.z = MAX(1.0f / FAR_AWAY, v3Transformed.z);

            v2ViewPos.x = v3Transformed.x / tan(DEG2RAD(m_pCamera->GetFovX() * 0.5f)) / v3Transformed.z;

            v2ViewPos.x *= 0.5f;
            v2ViewPos.x += 0.5f;
            v2ViewPos.x *= Draw2D.GetScreenW();
            v2ViewPos.x = MAX(recScreen.left, v2ViewPos.x);
            v2ViewPos.x = MIN(recScreen.right, v2ViewPos.x);
        }

        {
            CVec3f v3Angles(M_GetAnglesFromForwardVec(v3Local));
            v3Angles[PITCH] = v3ViewAngles[PITCH];

            CAxis aAxis(v3Angles);

            CVec3f v3Transformed
            (
                DotProduct(v3Local, aAxis[RIGHT]),
                DotProduct(v3Local, aAxis[UP]),
                DotProduct(v3Local, aAxis[FORWARD])
            );

            v3Transformed.z = MAX(1.0f / FAR_AWAY, v3Transformed.z);

            v2ViewPos.y = v3Transformed.y / tan(DEG2RAD(m_pCamera->GetFovY() * 0.5f)) / v3Transformed.z;

            v2ViewPos.y *= -0.5f;
            v2ViewPos.y += 0.5f;
            v2ViewPos.y *= Draw2D.GetScreenH();
            v2ViewPos.y = MAX(recScreen.top, v2ViewPos.y);
            v2ViewPos.y = MIN(recScreen.bottom, v2ViewPos.y);
        }

#if 0
        // Resnap to the edge of the screen if required
        if (v2ViewPos.x != recScreen.left && v2ViewPos.x != recScreen.right && 
            v2ViewPos.y != recScreen.top && v2ViewPos.y != recScreen.bottom)
        {
            if (bGoodPos)
            {
                v2ViewPos = v2ScreenPos;

                v2ViewPos.x = MAX(recScreen.left, v2ViewPos.x);
                v2ViewPos.x = MIN(recScreen.right, v2ViewPos.x);
                v2ViewPos.y = MAX(recScreen.top, v2ViewPos.y);
                v2ViewPos.y = MIN(recScreen.bottom, v2ViewPos.y);
            }
        }
#endif

        // Blend between our good WorldToScreen position and
        // the edge clamped position to smooth out the
        // transition
        if (bGoodPos)
        {
            float fMaxError(0.0f);
            fMaxError = MAX(recScreen.left - v2ScreenPos.x, fMaxError);
            fMaxError = MAX(v2ScreenPos.x - recScreen.right , fMaxError);
            fMaxError = MAX(recScreen.top - v2ScreenPos.y, fMaxError);
            fMaxError = MAX(v2ScreenPos.y - recScreen.bottom, fMaxError);

            float fLerp(fMaxError / 64.0f);

            if (fLerp < 1.0f)
            {
                v2ScreenPos.x = MAX(recScreen.left, v2ScreenPos.x);
                v2ScreenPos.x = MIN(recScreen.right, v2ScreenPos.x);
                v2ScreenPos.y = MAX(recScreen.top, v2ScreenPos.y);
                v2ScreenPos.y = MIN(recScreen.bottom, v2ScreenPos.y);

                v2ViewPos = LERP(fLerp, v2ScreenPos, v2ViewPos);
            }
        }

        recLocator.MoveTo(v2ViewPos - CVec2f(ROUND(fLocatorSize), ROUND(fLocatorSize)) / 2.0f);
    }

    recLocator.left = ROUND(recLocator.left);
    recLocator.top = ROUND(recLocator.top);
    recLocator.right = ROUND(recLocator.right);
    recLocator.bottom = ROUND(recLocator.bottom);

    Draw2D.SetColor(v4LocatorColor);
    Draw2D.Rect(recLocator, hLocator);

    if (pFontMap)
    {
        CVec2f v2StringPos(ROUND((recLocator.left + recLocator.right) / 2.0f - pFontMap->GetStringWidth(sText) / 2.0f), ROUND((recLocator.top + recLocator.bottom) / 2.0f + fLocatorSize / 2.0f + 3.0f));
        
        Draw2D.SetColor(BLACK);
        Draw2D.String(v2StringPos.x + 1, v2StringPos.y + 1, sText, m_hLocatorFont);

        Draw2D.SetColor(v4TextColor);
        Draw2D.String(v2StringPos.x, v2StringPos.y, sText, m_hLocatorFont);
    }
}


/*====================
  CGameClient::DrawPetLocator
  ====================*/
void    CGameClient::DrawPetLocator()
{
    IPlayerEntity *pLocalPlayer(GetLocalPlayer());
    if (pLocalPlayer == NULL)
        return;
    if (pLocalPlayer->GetPetIndex() == INVALID_INDEX)
        return;

    IVisualEntity *pPet(GetClientEntityCurrent(pLocalPlayer->GetPetIndex()));
    if (pPet != NULL && pPet->GetStatus() == ENTITY_STATUS_ACTIVE)
        DrawLocator(pPet->GetPosition(), m_hPetLocator, PET_COLOR, 16.0f, XtoA(INT_ROUND(Distance(pLocalPlayer->GetPosition(), pPet->GetPosition()) / 39.37f)) + _T(" m"), WHITE);
}


/*====================
  CGameClient::DrawOfficerLocator
  ====================*/
void    CGameClient::DrawOfficerLocator()
{
    CEntityTeamInfo *pTeamInfo(GetTeam(m_pLocalClient->GetTeam()));
    IPlayerEntity *pLocalPlayer(GetLocalPlayer());
    if (pTeamInfo == NULL || pLocalPlayer == NULL)
        return;

    uint uiOfficerIndex(pTeamInfo->GetOfficerGameIndex(m_pLocalClient->GetSquad()));
    if (uiOfficerIndex == INVALID_INDEX || uiOfficerIndex == m_pLocalClient->GetPlayerEntityIndex())
        return;

    IVisualEntity *pOfficer(GetClientEntityCurrent(uiOfficerIndex));
    if (pOfficer == NULL || pOfficer->GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    tstring sLabel(XtoA(INT_ROUND(Distance(pLocalPlayer->GetPosition(), pOfficer->GetPosition()) / 39.37f)) + _T(" m"));
    DrawLocator(pOfficer->GetPosition(), m_hOfficerLocator, OFFICER_COLOR, 16.0f, sLabel, WHITE);
}


/*====================
  CGameClient::DrawOrderLocator
  ====================*/
void    CGameClient::DrawOrderLocator()
{
    IPlayerEntity *pPlayer(GetLocalPlayer());
    if (pPlayer == NULL)
        return;

    uint uiTargetEntity(INVALID_INDEX);
    CVec3f v3OrderPos(V_ZERO);
    CVec4f v4Color(YELLOW);

    if (pPlayer->GetCurrentOrder() != CMDR_ORDER_CLEAR)
    {
        // Commander waypoint
        uiTargetEntity = pPlayer->GetCurrentOrderEntIndex();
        v3OrderPos = pPlayer->GetCurrentOrderPos();
        if (pPlayer->GetCurrentOrder() == CMDR_ORDER_ATTACK)
            v4Color = RED;
    }
    else if (pPlayer->GetOfficerOrder() != OFFICERCMD_INVALID) 
    {
        // Officer waypoint
        uiTargetEntity = pPlayer->GetOfficerOrderEntIndex();
        v3OrderPos = pPlayer->GetOfficerOrderPos();

        if (pPlayer->GetOfficerOrder() == OFFICERCMD_ATTACK)
            v4Color = RED;
    }
    else
    {
        return;
    }

    if (uiTargetEntity != INVALID_INDEX)
    {
        IVisualEntity *pTarget(GetVisualEntity(uiTargetEntity));
        if (pTarget != NULL)
            v3OrderPos = pTarget->GetPosition();
    }

    DrawLocator(v3OrderPos, m_hWaypointLocator, v4Color, 16.0f, XtoA(INT_ROUND(Distance(pPlayer->GetPosition(), v3OrderPos) / 39.37f)) + _T(" m"), WHITE);
}


/*====================
  CGameClient::DrawFogofWar
  ====================*/
void    CGameClient::DrawFogofWar()
{
    if (m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (pPlayer == NULL)
        return;

    if (GetGamePhase() != GAME_PHASE_WARMUP)
    {
        CClientEntityDirectory::ClientEntMap &mapEntities(m_pClientEntityDirectory->GetEntMap());
        for (CClientEntityDirectory::ClientEntMap_it it(mapEntities.begin()); it != mapEntities.end(); ++it)
        {
            IVisualEntity *pCurrent(it->second->GetCurrentEntity());

            if (!pCurrent || pCurrent->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;

            if (pCurrent->GetSightRange() <= 0.0f || pCurrent->GetTeam() != pPlayer->GetTeam())
                continue;

            float fX(pCurrent->GetPosition().x);
            float fY(pCurrent->GetPosition().y);
            float fRange(pCurrent->GetSightRange());

            Vid.AddFowRect(fX - fRange, fY - fRange, fRange * 2.0f, fRange * 2.0f, 0.0f, 0.0f, 1.0f, 1.0f, m_hFogofWarCircle);
        }
    }

    Vid.RenderFogofWar(GetGamePhase() == GAME_PHASE_WARMUP ? 1.0f : 0.0f);
}


/*====================
  LittleTextPopupMessage
  ====================*/
CMD(LittleTextPopupMessage)
{
    if (vArgList.size() < 1)
        return false;

    IPlayerEntity *pPlayer(GameClient.GetLocalPlayerCurrent());
    if (pPlayer != NULL)
        GameClient.SpawnLittleTextPopupMessage(ConcatinateArgs(vArgList.begin(), vArgList.end()), pPlayer->GetPosition() + pPlayer->GetBounds().GetMid(), WHITE);
    return true;
}


/*====================
  CGameClient::StartClientGameEffect
  ====================*/
int     CGameClient::StartClientGameEffect(const tstring &sEffect, int iChannel, int iTimeNudge, const CVec3f &v3Pos)
{
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
        m_apEffectThreads[iChannel] = NULL;
    }

    CEffect *pEffect(g_ResourceManager.GetEffect(g_ResourceManager.Register(sEffect, RES_EFFECT)));

    if (pEffect)
    {
        m_apEffectThreads[iChannel] = pEffect->SpawnThread(GameClient.GetGameTime() + iTimeNudge);

        m_apEffectThreads[iChannel]->SetCamera(GameClient.GetCamera());
        m_apEffectThreads[iChannel]->SetWorld(GameClient.GetWorldPointer());

        m_apEffectThreads[iChannel]->SetSourceSkeleton(NULL);
        m_apEffectThreads[iChannel]->SetSourceModel(NULL);
        m_apEffectThreads[iChannel]->SetTargetSkeleton(NULL);
        m_apEffectThreads[iChannel]->SetTargetModel(NULL);

        m_apEffectThreads[iChannel]->SetActive(true);

        m_apEffectThreads[iChannel]->SetSourcePos(v3Pos);
        m_apEffectThreads[iChannel]->SetSourceAxis(CAxis(0.0f, 0.0f, 0.0f));
        m_apEffectThreads[iChannel]->SetSourceScale(1.0f);
        
        if (m_apEffectThreads[iChannel]->Execute(GameClient.GetGameTime() + iTimeNudge))
        {
            // Effect finished, so delete it
            K2_DELETE(m_apEffectThreads[iChannel]);
            m_apEffectThreads[iChannel] = NULL;
        }
    }

    return iChannel;
}


/*====================
  CGameClient::StopClientGameEffect
  ====================*/
void    CGameClient::StopClientGameEffect(int iChannel)
{
    if (m_apEffectThreads[iChannel])
    {
        K2_DELETE(m_apEffectThreads[iChannel]);
        m_apEffectThreads[iChannel] = NULL;
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
            m_apEffectThreads[i] = NULL;
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
  CGameClient::UpdatePinging
  ====================*/
void    CGameClient::UpdatePinging()
{
    IPlayerEntity *pLocalPlayer(GetLocalPlayerCurrent());
    if (pLocalPlayer == NULL)
        return;

    if (m_bPinging && !m_bPingEffectActive)
    {
        STraceInfo trace;
        if (!GameClient.TraceCursor(trace, TRACE_TERRAIN))
            trace.v3EndPos = pLocalPlayer->GetPosition();

        StartClientGameEffect(_T("/shared/effects/ping_target.effect"), CGFX_PING, 0, trace.v3EndPos);

        m_bPingEffectActive = true;
    }
    else if (!m_bPinging && m_bPingEffectActive)
    {
        StopClientGameEffect(CGFX_PING);
        
        m_bPingEffectActive = false;
    }

    CVec3f v3End(M_PointOnLine(m_pCamera->GetOrigin(), m_pCamera->GetViewAxis(FORWARD), FAR_AWAY));

    STraceInfo trace;
    if (!TraceLine(trace, m_pCamera->GetOrigin(), v3End, TRACE_TERRAIN, pLocalPlayer->GetWorldIndex()))
        trace.v3EndPos = GetLocalPlayerCurrent()->GetPosition();

    UpdateClientGameEffect(CGFX_PING, true, trace.v3EndPos);

    if (m_bPinging && m_CurrentClientSnapshot.ButtonPressed(GAME_BUTTON_ACTIVATE_PRIMARY))
    {
        if (Host.GetTime() - GetLastPingTime() > MIN_PING_TIME)
        {
            CBufferFixed<11> buffer;
            buffer << GAME_CMD_MINIMAP_PING << trace.v3EndPos.x / GetWorldPointer()->GetWorldWidth() << trace.v3EndPos.y / GetWorldPointer()->GetWorldHeight() << m_yPingSquad;
            GameClient.SendGameData(buffer, false);
            SetLastPingTime(Host.GetTime());
        }

        m_bPinging = false;
        m_CurrentClientSnapshot.SetButton(GAME_BUTTON_ACTIVATE_PRIMARY, false);
    }

    if (m_CurrentClientSnapshot.ButtonPressed(GAME_BUTTON_ACTIVATE_PRIMARY) && TryBuildingPlacement())
        m_CurrentClientSnapshot.SetButton(GAME_BUTTON_ACTIVATE_PRIMARY, false);
}


/*====================
  CGameClient::ToggleAutoRun
  ====================*/
void    CGameClient::ToggleAutoRun()
{
    if (m_bAutoRun)
    {
        m_CurrentClientSnapshot.SetButton(GAME_BUTTON_FORWARD, 0.0f);
        m_bAutoRun = false;
    }
    else
    {
        m_CurrentClientSnapshot.SetButton(GAME_BUTTON_FORWARD, 1.0f);
        m_bAutoRun = true;
    }
}


/*====================
  CGameClient::BreakAutoRun
  ====================*/
void    CGameClient::BreakAutoRun()
{
    if (m_bAutoRun)
    {
        m_CurrentClientSnapshot.SetButton(GAME_BUTTON_FORWARD, 0.0f);
        m_bAutoRun = false;
    }
}


/*====================
  CGameClient::CheckDash
  ====================*/
bool    CGameClient::CheckDash()
{
    uint uiLastTime(m_uiLastForwardPress);
    m_uiLastForwardPress = Game.GetGameTime();

    if (uiLastTime == INVALID_TIME)
        return false;

    if (GetGameTime() - uiLastTime <= cg_dashActivationTime)
        return true;

    return false;
}


/*====================
  CGameClient::GetStateString
  ====================*/
CStateString&   CGameClient::GetStateString(uint uiID)
{
    return *m_pHostClient->GetStateString(uiID);
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
uint    CGameClient::GetServerTime()
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
    return SecToMs(1.0f / GetStateString(STATE_STRING_SERVER_INFO).GetInt(_T("svr_gameFPS")));
}


/*====================
  CGameClient::PetCommand
  ====================*/
void    CGameClient::PetCommand(EPetCommand ePetCmd)
{
    switch (ePetCmd)
    {
    case PETCMD_ATTACK:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_PETCMD_ATTACK);
        break;
    case PETCMD_FOLLOW:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_PETCMD_FOLLOW);
        break;
    case PETCMD_MOVE:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_PETCMD_MOVE);
        break;
    default:
        {
            CBufferFixed<3> buffer;
            buffer << GAME_CMD_PETCMD << byte(ePetCmd);
            GameClient.SendGameData(buffer, true);
        }
        break;
    }
}


/*====================
  CGameClient::OfficerCommand
  ====================*/
void    CGameClient::OfficerCommand(EOfficerCommand eOfficerCmd)
{
    if (m_pLocalClient == NULL)
        return;
    IPlayerEntity *pLocalPlayer(GetLocalPlayer());
    if (pLocalPlayer == NULL)
        return;

    switch (eOfficerCmd)
    {
    case OFFICERCMD_ATTACK:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_OFFICERCMD_ATTACK);
        break;

    case OFFICERCMD_FOLLOW:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_OFFICERCMD_FOLLOW);
        break;

    case OFFICERCMD_MOVE:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_OFFICERCMD_MOVE);
        break;

    case OFFICERCMD_DEFEND:
        m_CurrentClientSnapshot.SelectItem(INVENTORY_OFFICERCMD_DEFEND);
        break;

    case OFFICERCMD_PING:
        StartPinging(m_pLocalClient->GetSquad());
        break;

    case OFFICERCMD_RALLY:
        {
            CBufferFixed<6> buffer;
            buffer << GAME_CMD_OFFICERCMD_ENT << byte(eOfficerCmd) << pLocalPlayer->GetIndex();
            GameClient.SendGameData(buffer, true);
        }
        break;

    case OFFICERCMD_INVALID:
        {
            CBufferFixed<2> buffer;
            buffer << GAME_CMD_OFFICERCMD << byte(OFFICERCMD_INVALID);
            GameClient.SendGameData(buffer, true);
        }

    default:
        {
            CBufferFixed<2> buffer;
            buffer << GAME_CMD_OFFICERCMD << byte(eOfficerCmd);
            GameClient.SendGameData(buffer, true);
        }
        break;
    }
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

        const CSnapshot *pSnapshot(m_vServerSnapshots[m_uiSnapshotBufferPos] != INVALID_POOL_HANDLE ? CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos]) : NULL);

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

    const CSnapshot *pSnapshot(m_vServerSnapshots[m_uiSnapshotBufferPos] != INVALID_POOL_HANDLE ? CSnapshot::GetByHandle(m_vServerSnapshots[m_uiSnapshotBufferPos]) : NULL);
    
    m_CurrentClientSnapshot.Update(GetGameTime(), GetFrameLength(), pSnapshot ? pSnapshot->GetFrameNumber() : 0);
    m_PendingClientSnapshot.Merge(m_CurrentClientSnapshot);
    
    if (m_pHostClient->IsReadyToSendSnapshot() || (!cg_snapshotRateLimiter && Host.GetFrameLength() > 5))
    {
        CBufferDynamic bufClientSnapshot;
        m_PendingClientSnapshot.GetUpdate(bufClientSnapshot);
        m_pHostClient->SendClientSnapshot(bufClientSnapshot);
        m_deqClientSnapshots.push_back(m_PendingClientSnapshot);
        m_PendingClientSnapshot.Reset();

        while (m_deqClientSnapshots.size() > 128)
            m_deqClientSnapshots.pop_front();
    }

    if (m_vServerSnapshots.size() != cg_serverSnapshotCacheSize)
        m_vServerSnapshots.resize(cg_serverSnapshotCacheSize, INVALID_POOL_HANDLE);
}


/*====================
  CGameClient::StopWorldSounds
  ====================*/
void    CGameClient::StopWorldSounds()
{
    for(SWorldSoundsMap_it it = m_mapWorldSounds.begin(); it != m_mapWorldSounds.end();)
    {
        if (it->second.hSound != INVALID_INDEX)
            K2SoundManager.StopHandle(it->second.hSound);

        STL_ERASE(m_mapWorldSounds, it);
    }
    if (m_hWorldAmbientSound != INVALID_INDEX)
        K2SoundManager.StopHandle(m_hWorldAmbientSound);
}


/*====================
  CGameClient::WorldSoundsFrame
  ====================*/
void    CGameClient::WorldSoundsFrame()
{
    float fVolumeMult(IsCommander() ? 0.5f : 1.0f);
    for(SWorldSoundsMap_it it = m_mapWorldSounds.begin(); it != m_mapWorldSounds.end(); it++)
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
    return g_ResourceManager.Register(sPath, RES_MODEL);
}


/*====================
  CGameClient::RegisterEffect
  ====================*/
ResHandle   CGameClient::RegisterEffect(const tstring &sPath)
{
    return g_ResourceManager.Register(sPath, RES_EFFECT);
}


/*====================
  CGameClient::ProcessVoicePacket
  ====================*/
void    CGameClient::ProcessVoicePacket(CPacket &pkt)
{
    int iClientNum(pkt.ReadInt());
    uint uiLength(pkt.ReadInt());
    
    if (uiLength == 0)
        return;

    if (m_pVoiceManager == NULL)
    {
        pkt.Seek(pkt.GetReadPos() + uiLength);
        return;
    }
    
    char *pData(K2_NEW_ARRAY(MemManager.GetHeap(HEAP_CLIENT_GAME), char, uiLength+1));
    pkt.Read(pData, uiLength);
    m_pVoiceManager->ReadData(iClientNum, uiLength, pData);
    SAFE_DELETE(pData);
}


/*====================
  CGameClient::CheckVoiceCvars
  ====================*/
void    CGameClient::CheckVoiceCvars()
{
    if (sound_voiceDisabled->IsModified())
    {
        sound_voiceDisabled->SetModified(false);
        
        SAFE_DELETE(m_pVoiceManager);
        if (!sound_voiceDisabled->GetValue())
            m_pVoiceManager = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CVoiceManager);
    }

    if (m_pSoundVoiceMicMuted == NULL)
        m_pSoundVoiceMicMuted = ICvar::Find(_T("sound_voiceMicMuted"));
    if (m_pSoundVoiceMicMuted != NULL && m_pSoundVoiceMicMuted->IsModified())
    {
        m_pSoundVoiceMicMuted->SetModified(false);

        if (m_pSoundVoiceMicMuted->GetBool())
            StopVoiceRecording();
    }
}


/*====================
  CGameClient::StartVoiceRecording
  ====================*/
void    CGameClient::StartVoiceRecording()
{
    if (m_pSoundVoiceMicMuted != NULL && m_pSoundVoiceMicMuted->GetBool())
        return;

    if (m_pVoiceManager != NULL)
        m_pVoiceManager->StartRecording();
}


/*====================
  CGameClient::StopVoiceRecording
  ====================*/
void    CGameClient::StopVoiceRecording()
{
    if (m_pVoiceManager != NULL)
        m_pVoiceManager->StopRecording();
}


/*====================
  CGameClient::StoppedTalking
  ====================*/
void    CGameClient::StoppedTalking(int iClientNum)
{
    if (m_pVoiceManager == NULL)
        return;

    CVoice *pVoice(m_pVoiceManager->GetClientVoice(iClientNum));
    if (pVoice != NULL)
        pVoice->StopTalking();
}


/*====================
  CGameClient::StartedTalking
  ====================*/
void    CGameClient::StartedTalking(int iClientNum)
{
    if (m_pVoiceManager == NULL)
        return;

    m_pVoiceManager->StartTalking(iClientNum);
}


/*====================
  CGameClient::IsTalking
  ====================*/
bool    CGameClient::IsTalking(int iClientNum)
{
    if (m_pVoiceManager == NULL)
        return false;

    CVoice *pVoice(m_pVoiceManager->GetClientVoice(iClientNum));
    if (pVoice == NULL)
        return false;
    
    return pVoice->IsTalking();
}


/*====================
  CGameClient::IsMuted
  ====================*/
bool    CGameClient::IsMuted(int iClientNum)
{
    if (m_pVoiceManager == NULL)
        return false;

    CVoice *pVoice(m_pVoiceManager->GetClientVoice(iClientNum));
    if (pVoice == NULL)
        return false;
    
    return pVoice->IsMute();
}


/*====================
  CGameClient::SetVoiceMute
  ====================*/
void    CGameClient::SetVoiceMute(int iClientNum, bool bValue)
{
    if (m_pVoiceManager == NULL)
        return;

    CVoice *pVoice(m_pVoiceManager->GetClientVoice(iClientNum));
    if (pVoice == NULL)
        return;
    
    pVoice->SetMute(bValue);
}


/*====================
  CGameClient::SetFirstPersonModelHandle()
  ====================*/
void    CGameClient::SetFirstPersonModelHandle(ResHandle hFirstPersonModelHandle)
{
    if (m_hFirstPersonModelHandle == hFirstPersonModelHandle)
        return;

    m_hFirstPersonModelHandle = hFirstPersonModelHandle;

    // Clear any old effect threads
    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        SAFE_DELETE(m_apFirstPersonEffectThread[i]);
    }
}


/*====================
  CGameClient::UpdateFirstPerson
  ====================*/
void    CGameClient::UpdateFirstPerson()
{
    PROFILE("CGameClient::RenderFirstPerson");

    if (m_pLocalClient == NULL || !m_pCamera->HasFlags(CAM_FIRST_PERSON))
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (!pPlayer)
        return;

    pPlayer->LocalClientFrame();

    SetFirstPersonModelHandle(pPlayer->GetFirstPersonModelHandle());

    if (m_hFirstPersonModelHandle == INVALID_RESOURCE)
        return;

    m_pCurrentEntity = GetClientEntity(pPlayer->GetIndex());
    SetEventTarget(CG_EVENT_TARGET_HANDS);

    CSkeleton *pSkeleton(pPlayer->GetFirstPersonSkeleton());

    if (!pSkeleton)
    {
        pSkeleton = K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CSkeleton)();
        pPlayer->SetFirstPersonSkeleton(pSkeleton);
        
        // Clear any old effect threads
        for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
        {
            SAFE_DELETE(m_apFirstPersonEffectThread[i]);
        }
    }

    // Update skeleton model
    if (pSkeleton)
        pSkeleton->SetModel(m_hFirstPersonModelHandle);

    // Start any new animations on the skeleton
    if (pSkeleton && pSkeleton->GetModel() != INVALID_RESOURCE)
    {
        //pSkeleton->SetDefaultAnim(g_ResourceManager.GetAnimName(m_hFirstPersonModelHandle, m_pCurrentState->GetDefaultAnim()));

        // Process StopAnim
        if (pPlayer->GetFirstPersonAnim() == ENTITY_STOP_ANIM &&
            (pPlayer->GetFirstPersonAnim() != m_iActiveFirstPersonAnim || 
            pPlayer->GetFirstPersonAnimSequence() != m_yActiveFirstPersonAnimSequence))
        {
            m_iActiveFirstPersonAnim = pPlayer->GetFirstPersonAnim();
            m_yActiveFirstPersonAnimSequence = pPlayer->GetFirstPersonAnimSequence();

            pSkeleton->StopAnim(0, Game.GetGameTime());
        }

        // Start new animation
        if (pPlayer->GetFirstPersonAnim() != m_iActiveFirstPersonAnim || 
            pPlayer->GetFirstPersonAnimSequence() != m_yActiveFirstPersonAnimSequence)
        {
            m_iActiveFirstPersonAnim = pPlayer->GetFirstPersonAnim();
            m_yActiveFirstPersonAnimSequence = pPlayer->GetFirstPersonAnimSequence();

            pSkeleton->StartAnim(
                g_ResourceManager.GetAnimName(m_hFirstPersonModelHandle, pPlayer->GetFirstPersonAnim()),
                Game.GetGameTime(), 0, -1, pPlayer->GetFirstPersonAnimSpeed(), 0);
        }
    }

    // Update skeleton
    pPlayer->UpdateFirstPersonSkeleton();

    // Process effect threads for this entity
    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        if (!m_apFirstPersonEffectThread[i])
            continue;

        // Setup effect parameters
        m_apFirstPersonEffectThread[i]->SetActive(true);

        m_apFirstPersonEffectThread[i]->SetSourceSkeleton(pSkeleton);
        m_apFirstPersonEffectThread[i]->SetSourceModel(g_ResourceManager.GetModel(pPlayer->GetFirstPersonModelHandle()));
        m_apFirstPersonEffectThread[i]->SetTargetSkeleton(NULL);
        m_apFirstPersonEffectThread[i]->SetTargetModel(NULL);

        m_apFirstPersonEffectThread[i]->SetSourcePos(m_pCamera->GetOrigin() + m_pCamera->GetViewAxis() * pPlayer->GetFirstPersonModelOffset());
        m_apFirstPersonEffectThread[i]->SetSourceScale(1.0f);
        m_apFirstPersonEffectThread[i]->SetSourceAxis(m_pCamera->GetViewAxis());

        // Execute effect
        if (m_apFirstPersonEffectThread[i]->Execute(GameClient.GetGameTime()))
        {
            // Effect finished, so delete it
            K2_DELETE(m_apFirstPersonEffectThread[i]);
            m_apFirstPersonEffectThread[i] = NULL;
        }
        else
        {
            // Camera movement
            AddCameraEffectOffset(m_apFirstPersonEffectThread[i]->GetCameraOffset());
            AddCameraEffectAngleOffset(m_apFirstPersonEffectThread[i]->GetCameraAngleOffset());

            // Overlays
            if (m_apFirstPersonEffectThread[i]->HasActiveOverlay())
                AddOverlay(m_apFirstPersonEffectThread[i]->GetOverlayColor(), m_apFirstPersonEffectThread[i]->GetOverlayMaterial());
        }
    }

    // Process first person effect passed from a Entity_Effect
    for (vector<CEffectThread *>::iterator it(m_vFirstPersonEffects.begin()); it != m_vFirstPersonEffects.end(); ++it)
    {
        CEffectThread *pEffectThread(*it);

        // Setup effect parameters
        pEffectThread->SetSourceSkeleton(pSkeleton);
        pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pPlayer->GetFirstPersonModelHandle()));

        pEffectThread->SetSourcePos(m_pCamera->GetOrigin() + m_pCamera->GetViewAxis() * pPlayer->GetFirstPersonModelOffset());
        pEffectThread->SetSourceScale(1.0f);
        pEffectThread->SetSourceAxis(m_pCamera->GetViewAxis());

        // Execute effect
        if (pEffectThread->Execute(GameClient.GetGameTime()))
        {
            // Effect finished, so delete it
            K2_DELETE(pEffectThread);
            pEffectThread = NULL;
        }
        else
        {
            // Camera movement
            AddCameraEffectOffset((*it)->GetCameraOffset());
            AddCameraEffectAngleOffset((*it)->GetCameraAngleOffset());

            // Overlays
            if ((*it)->HasActiveOverlay())
                AddOverlay((*it)->GetOverlayColor(), (*it)->GetOverlayMaterial());
        }
    }
}


/*====================
  CGameClient::RenderFirstPerson
  ====================*/
void    CGameClient::RenderFirstPerson()
{
    PROFILE("CGameClient::RenderFirstPerson");

    if (m_pLocalClient == NULL || !m_pCamera->HasFlags(CAM_FIRST_PERSON))
        return;

    IPlayerEntity *pPlayer(m_pLocalClient->GetPlayerEntity());
    if (!pPlayer)
        return;

    if (m_hFirstPersonModelHandle == INVALID_RESOURCE)
        return;

    CCamera cGunCamera(*m_pCamera);

    // Prepare the scene
    SceneManager.Clear();
    cGunCamera.SetFovXCalc(pPlayer->GetFirstPersonModelFov());

    uint uiOldSize(uint(SceneManager.GetEntityList().size()));

    // AddToScene
    pPlayer->AddFirstPersonToScene(cGunCamera);

    SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
    SceneEntityList::iterator itBegin(lSceneEntities.begin() + uiOldSize);

    // Process effect threads for this entity
    for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
    {
        if (!m_apFirstPersonEffectThread[i])
            continue;

        // Update and render all particles systems associated with this effect thread
        const InstanceMap &mapInstances(m_apFirstPersonEffectThread[i]->GetInstances());
        for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
        {
            IEffectInstance *pParticleSystem(it->second);

            pParticleSystem->Update(GameClient.GetGameTime());

            if (!pParticleSystem->IsDead())
            {
                if (pParticleSystem->IsParticleSystem())
                    SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
                else if (pParticleSystem->IsModifier())
                {
                    for (SceneEntityList::iterator itEnts(itBegin); itEnts != lSceneEntities.end(); ++itEnts)
                        static_cast<CSceneEntityModifier *>(pParticleSystem)->Modify((*itEnts)->cEntity);
                }
            }
        }

        // Cleanup
        m_apFirstPersonEffectThread[i]->Cleanup();
    }

    // Process first person effect passed from a Entity_Effect
    for (vector<CEffectThread *>::iterator it(m_vFirstPersonEffects.begin()); it != m_vFirstPersonEffects.end(); ++it)
    {
        CEffectThread *pEffectThread(*it);

        // Update and render all particles systems associated with this effect thread
        const InstanceMap &mapInstances(pEffectThread->GetInstances());
        for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
        {
            IEffectInstance *pParticleSystem(it->second);

            pParticleSystem->Update(GameClient.GetGameTime());

            if (!pParticleSystem->IsDead())
            {
                if (pParticleSystem->IsParticleSystem())
                    SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
                else if (pParticleSystem->IsModifier())
                {
                    for (SceneEntityList::iterator itEnts(itBegin); itEnts != lSceneEntities.end(); ++itEnts)
                        static_cast<CSceneEntityModifier *>(pParticleSystem)->Modify((*itEnts)->cEntity);
                }
            }
        }

        // Cleanup
        pEffectThread->Cleanup();
    }
        
    // Render this scene
    cGunCamera.AddFlags(CAM_NO_TERRAIN | CAM_NO_SKY | CAM_SHADOW_PARALLEL | CAM_SHADOW_NOSLIDEBACK);

    if (!cg_firstPersonShadows)
        cGunCamera.AddFlags(CAM_NO_SHADOWS);

    SceneManager.PrepCamera(cGunCamera);
    SceneManager.Render();
}


/*====================
  CGameClient::RenderSelectedPlayerView
  ====================*/
CVAR_BOOL(pip_show, false);
CVAR_FLOAT(pip_x, 860.f);
CVAR_FLOAT(pip_y, 380.f);
CVAR_FLOAT(pip_w, 160.f);
CVAR_FLOAT(pip_h, 120.f);
CVAR_BOOL(pip_repopulate, false);
void    CGameClient::RenderSelectedPlayerView(uint uiPlayerIndex)
{
    PROFILE("CGameClient::RenderSelectedPlayerView");

    if (!pip_show)
        return;

    IPlayerEntity *pPlayer(GetPlayerEntity(uiPlayerIndex));
    if (pPlayer == NULL)
        return;

    CCamera camPip(*m_pCamera);

    // Prepare the scene
    if (pip_repopulate)
    {
        SceneManager.Clear();
        m_pClientEntityDirectory->PopulateScene();
    }
    //SceneManager.ClearBackground();

    pPlayer->SetupCamera(camPip, pPlayer->GetPosition(), pPlayer->GetAngles());
    camPip.SetTime(MsToSec(GetGameTime()));
    camPip.SetX(pip_x);
    camPip.SetY(pip_y);
    camPip.SetWidth(pip_w);
    camPip.SetHeight(pip_h);
    camPip.SetFovXCalc(pPlayer->GetFov());
    camPip.RemoveFlags(CAM_FOG_OF_WAR | CAM_NO_FOG | CAM_SHADOW_UNIFORM | CAM_SHADOW_NO_FALLOFF);
    camPip.SetShadowBias(0.0f);

    SceneManager.PrepCamera(camPip);
    SceneManager.DrawSky(camPip, MsToSec(m_pHostClient->GetClientFrameLength()));
    SceneManager.Render();
}


/*====================
  CGameClient::StartFirstPersonEffect
  ====================*/
int     CGameClient::StartFirstPersonEffect(ResHandle hEffect, int iChannel, int iTimeNudge)
{
    IPlayerEntity *pPlayer(GetLocalPlayerCurrent());
    if (!pPlayer)
        return -1;

    if (pPlayer->GetFirstPersonModelHandle() == INVALID_RESOURCE)
        return -1;

    CSkeleton *pSkeleton(pPlayer->GetFirstPersonSkeleton());

    // Search from an unused effect slot
    if (iChannel == -1)
    {
        for (int i(NUM_CLIENT_EFFECT_THREADS - 1); i >= NUM_EFFECT_CHANNELS; --i)
        {
            if (!m_apFirstPersonEffectThread[i])
            {
                iChannel = i;
                break;
            }
        }

        if (iChannel == -1)
            return -1;
    }
    else
    {
        iChannel += NUM_EFFECT_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS
    }

    if (m_apFirstPersonEffectThread[iChannel])
    {
        K2_DELETE(m_apFirstPersonEffectThread[iChannel]);
        m_apFirstPersonEffectThread[iChannel] = NULL;
    }

    CEffect *pEffect(g_ResourceManager.GetEffect(hEffect));

    if (pEffect)
    {
        m_apFirstPersonEffectThread[iChannel] = pEffect->SpawnThread(GameClient.GetGameTime() + iTimeNudge);

        if (m_apFirstPersonEffectThread[iChannel] == NULL)
            return -1;

        m_apFirstPersonEffectThread[iChannel]->SetCamera(GetCamera());
        m_apFirstPersonEffectThread[iChannel]->SetWorld(GetWorldPointer());

        m_apFirstPersonEffectThread[iChannel]->SetSourceSkeleton(pSkeleton);
        m_apFirstPersonEffectThread[iChannel]->SetSourceModel(g_ResourceManager.GetModel(pPlayer->GetFirstPersonModelHandle()));
        m_apFirstPersonEffectThread[iChannel]->SetTargetSkeleton(NULL);
        m_apFirstPersonEffectThread[iChannel]->SetTargetModel(NULL);

        m_apFirstPersonEffectThread[iChannel]->SetActive(true);
        
        // TODO: we should use a timenudged lerped position instead of the previous frame's CurrentState
        m_apFirstPersonEffectThread[iChannel]->SetSourcePos(m_pCamera->GetOrigin() + m_pCamera->GetViewAxis() * pPlayer->GetFirstPersonModelOffset());
        m_apFirstPersonEffectThread[iChannel]->SetSourceAxis(m_pCamera->GetViewAxis());
        m_apFirstPersonEffectThread[iChannel]->SetSourceScale(1.0f);
        
        if (m_apFirstPersonEffectThread[iChannel]->Execute(GameClient.GetGameTime() + iTimeNudge))
        {
            // Effect finished, so delete it
            K2_DELETE(m_apFirstPersonEffectThread[iChannel]);
            m_apFirstPersonEffectThread[iChannel] = NULL;
        }
    }

    return iChannel;
}


/*====================
  CGameClient::StartFirstPersonEffect
  ====================*/
int     CGameClient::StartFirstPersonEffect(const tstring &sEffect, int iChannel, int iTimeNudge)
{   
    return StartFirstPersonEffect(g_ResourceManager.Register(sEffect, RES_EFFECT), iChannel, iTimeNudge);
}


/*====================
  CGameClient::StopFirstPersonEffect
  ====================*/
void    CGameClient::StopFirstPersonEffect(int iChannel)
{
    if (m_apFirstPersonEffectThread[iChannel])
    {
        K2_DELETE(m_apFirstPersonEffectThread[iChannel]);
        m_apFirstPersonEffectThread[iChannel] = NULL;
    }
}


/*====================
  CGameClient::PushFirstPersonEffect
  ====================*/
void    CGameClient::PushFirstPersonEffect(CEffectThread *pEffectThread)
{
    if (m_vFirstPersonEffects.size() < 16)
        m_vFirstPersonEffects.push_back(pEffectThread);
}


/*====================
  CGameClient::AddVCCategory
  ====================*/
bool    CGameClient::AddVCCategory(EButton button, const tstring &sType, const tstring &sDesc)
{
    if (button == BUTTON_INVALID || button == BUTTON_ESC)
        return false;

    VCMap_it findit(m_mapVC.find(button));

    if (findit != m_mapVC.end())
        return false;

    VCCategory structVC;
    structVC.sDesc = sDesc;
    structVC.eButton = button;

    if (LowerString(sType) == _T("commander"))
        structVC.eType = VC_COMMANDER;
    else if (LowerString(sType) == _T("squad"))
        structVC.eType = VC_SQUAD;
    else if (LowerString(sType) == _T("global"))
        structVC.eType = VC_ALL;
    else
        structVC.eType = VC_TEAM;

    m_mapVC.insert(VCPair(button, structVC));

    ActionRegistry.BindImpulse(BINDTABLE_GAME_VOICECOMMAND, button, BIND_MOD_NONE,  _T("VCMain"), Input.ToString(button));

    return true;
}

/*====================
  CGameClient::GetVCCategory
  ====================*/
VCCategory* CGameClient::GetVCCategory(EButton button)
{
    if (button == BUTTON_INVALID || button == BUTTON_ESC)
        return NULL;

    VCMap_it findit(m_mapVC.find(button));

    if (findit != m_mapVC.end())
        return &findit->second;

    return NULL;
}


/*====================
  CGameClient::GetActiveVCCategory
  ====================*/
VCCategory* CGameClient::GetActiveVCCategory()
{
    if (m_VCSubActive == BUTTON_INVALID)
        return NULL;

    VCMap_it findit(m_mapVC.find(m_VCSubActive));

    if (findit != m_mapVC.end())
        return &findit->second;

    return NULL;
}


/*====================
  CGameClient::AddVCSubItem
  ====================*/
bool    CGameClient::AddVCSubItem(VCCategory *pCategory, EButton button, const tstring &sRace, const tstring &sDesc, const tstring &sPath)
{
    if (pCategory == NULL || button == BUTTON_ESC)
        return false;

    if (button == BUTTON_INVALID)
        return false;

    VCSub structVCSub;

    structVCSub.sDesc = sDesc;
    structVCSub.hSound = g_ResourceManager.Register(K2_NEW(g_heapResources,   CSample)(sPath, SND_2D), RES_SAMPLE);
    structVCSub.eButton = button;

    map<EButton, VCSubMap>::iterator findit(pCategory->mapSubItems.find(button));

    if (findit != pCategory->mapSubItems.end())
        findit->second[LowerString(sRace)] = structVCSub;
    else
    {
        VCSubMap newSubMap;
        newSubMap[LowerString(sRace)] = structVCSub;

        pCategory->mapSubItems.insert(pair<EButton, VCSubMap>(button, newSubMap));
    }

    ActionRegistry.BindImpulse(BINDTABLE_GAME_VOICECOMMAND_SUB, button, BIND_MOD_NONE,  _T("VCSub"), Input.ToString(button));

    return true;
}


/*====================
  CGameClient::VCSubMenuActive
  ====================*/
void    CGameClient::VCSubMenuActive(EButton button)
{
    if (m_mapVC.find(button) != m_mapVC.end())
        m_VCSubActive = button;
    else
        m_VCSubActive = BUTTON_INVALID;
}


/*====================
  CGameClient::DoVoiceCommand
  ====================*/
void    CGameClient::DoVoiceCommand(EButton button)
{
    VCCategory *pCategory(GetActiveVCCategory());
    IPlayerEntity *pPlayer(GetLocalPlayer());

    if (pCategory == NULL)
        return;

    if (button == BUTTON_INVALID)
        return;

    if (pPlayer == NULL)
        return;

    CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));

    if (pTeam == NULL)
        return;

    map<EButton, VCSubMap>::iterator findit;

    findit = pCategory->mapSubItems.find(button);

    if (findit == pCategory->mapSubItems.end())
        return;

    tstring sRace(LowerString(pTeam->GetDefinition()->GetName()));
    VCSubMap_it it(findit->second.find(sRace));

    if (it == findit->second.end())
        return;

    if (m_uiNextVCTime > Host.GetTime())
    {
        Console << _T("^yUsing Voice Commands too frequently. Please wait ") << XtoA(INT_CEIL(float(m_uiNextVCTime-Host.GetTime())/1000.0f)) << _T(" seconds.") << newl;
        m_pHostClient->AddGameChatMessage(_T("Add ^yUsing Voice Commands too frequently. Please wait ") + XtoA(INT_CEIL(float(m_uiNextVCTime-Host.GetTime())/1000.0f)) + _T(" seconds."));

        VCSubMenuActive(BUTTON_INVALID);
        VCMenuActive(false);

        return;
    }

    if (m_uiVCUses > 0 && m_uiNextVCTime + 10000 < Host.GetTime())
        m_uiVCUses = 0;

    m_uiNextVCTime = Host.GetTime() + (VC_MIN_TIME * pow(2.0f, float(m_uiVCUses)));
    m_uiVCUses++;

    CBufferDynamic buffer;

    buffer << GAME_CMD_VOICECOMMAND << byte(pCategory->eType) << sRace << byte(0) << Input.ToString(pCategory->eButton) << byte(0) << Input.ToString(button) << byte(0);
    SendGameData(buffer, true);

    K2SoundManager.Play2DSound(it->second.hSound);

    tstring sHeading;

    if (pCategory->eType == VC_ALL)
        sHeading = _T("[VC ALL]");
    else if (pCategory->eType == VC_SQUAD)
        sHeading = _T("[VC SQUAD]");
    else if (pCategory->eType == VC_COMMANDER)
        sHeading = _T("[VC COMMANDER]");
    else
        sHeading = _T("[VC TEAM]");

    Console << _T("^696") << sHeading << _T("^* ") << m_pLocalClient->GetName() << _T(": ") << it->second.sDesc << newl;
    m_pHostClient->AddGameChatMessage(_T("Add ^696") + sHeading + _T("^r ") + m_pLocalClient->GetName() + _T(": ^*") + it->second.sDesc);

    VCSubMenuActive(BUTTON_INVALID);
    VCMenuActive(false);

    map<uint, uint>::iterator markerit(m_mapVoiceMarkers.find(pPlayer->GetIndex()));
    if (markerit != m_mapVoiceMarkers.end())
        markerit->second = GetGameTime() + 3000;
    else if (pPlayer != NULL)
        m_mapVoiceMarkers.insert(pair<uint, uint>(pPlayer->GetIndex(), GetGameTime() + 3000));
}


/*====================
  CGameClient::DrawPlayerStats
  ====================*/
void    CGameClient::DrawPlayerStats()
{
    if (!cg_drawPlayerStats || m_pLocalClient == NULL)
        return;

    IPlayerEntity *pPlayer(GetLocalPlayerCurrent());

    ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
    if (pFontMap == NULL)
        return;
    
    float fLines = 14.0f;

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 23;
    const float PANEL_HEIGHT = fLines;
    const float START_X = INT_FLOOR(FONT_WIDTH);
    const float START_Y = INT_FLOOR(Draw2D.GetScreenH() - (FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2) - 148.0f);
    const int   NAME_LENGTH = 13;

    float fDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - FONT_WIDTH, START_Y - FONT_WIDTH, FONT_WIDTH * PANEL_WIDTH + FONT_WIDTH * 2, FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2);

    Draw2D.SetColor(WHITE);

    Draw2D.String(START_X, fDrawY, XtoA(_T("Health")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetHealth()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Mana")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetMana()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Stamina")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetStamina()), hFont);
    fDrawY += FONT_HEIGHT;

    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("MaxHealth")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetMaxHealth()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("MaxMana")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetMaxMana()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("MaxStamina")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetMaxStamina()), hFont);
    fDrawY += FONT_HEIGHT;

    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("HealthRegen")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetHealthRegen()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("ManaRegen")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetManaRegen()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("StaminaRegen")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetStaminaRegen()), hFont);
    fDrawY += FONT_HEIGHT;

    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Armor")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetArmor()), hFont);
    fDrawY += FONT_HEIGHT;

    Draw2D.String(START_X, fDrawY, XtoA(_T("Movespeed")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pPlayer->GetSpeed()), hFont);
    fDrawY += FONT_HEIGHT;
    
    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*====================
  CGameClient::TriggerScript
  ====================*/
bool    CGameClient::TriggerScript(const tstring &sName)
{
    smaps::iterator findit(m_mapScripts.find(sName));

    if (findit != m_mapScripts.end() && !findit->second.empty())
    {
        bool bWasStoring(ICvar::StoreCvars());
        ICvar::StoreCvars(false);

        Console.ExecuteScript(findit->second, false, &m_mapScriptParams);

        ICvar::StoreCvars(bWasStoring);
    }

    m_mapScriptParams.clear();

    return true;
}

/*====================
  CGameClient::CanBuild
  ====================*/
bool    CGameClient::CanBuild(const tstring &sBuilding, bool bSetReason)
{
    if (sBuilding.empty())
        return false;

    // Get local player
    IPlayerEntity *pPlayer(GetLocalPlayer());
    if (pPlayer == NULL)
        return false;

    // Get team info
    CEntityTeamInfo *pTeam(GetTeam(pPlayer->GetTeam()));
    if (pTeam == NULL)
        return false;

    // Lookup building ID
    ushort unID(EntityRegistry.LookupID(sBuilding));
    if (unID == INVALID_ENT_TYPE)
        return false;

    // Check cost
    ICvar* pCost(EntityRegistry.GetGameSetting(unID, _T("Cost")));

    if (pCost != NULL && pCost->GetInteger() > int(pTeam->GetGold()))
    {
        if (bSetReason)
            m_pInterfaceManager->Trigger(UITRIGGER_BUILD_FAIL_REASON, _T("Not enough gold"));
        return false;
    }

    // Check single instance
    ICvar* pMajor(EntityRegistry.GetGameSetting(unID, _T("MajorBuilding")));
    if (pMajor != NULL && pMajor->GetBool() && pTeam->HasBuilding(sBuilding))
    {
        if (bSetReason)
            m_pInterfaceManager->Trigger(UITRIGGER_BUILD_FAIL_REASON, _T("Only one building of this type can be built"));
        return false;
    }

    // Check prerequisites
    ICvar* pPrerequisites(EntityRegistry.GetGameSetting(unID, _T("Prerequisite")));
    if (pPrerequisites != NULL)
    {
        svector vPrerequisites(TokenizeString(pPrerequisites->GetString(), _T(' ')));
        for (svector_it it(vPrerequisites.begin()); it != vPrerequisites.end(); ++it)
        {
            if (!pTeam->HasBuilding(*it))
            {
                if (bSetReason)
                {
                    ICvar *pCvar(g_EntityRegistry.GetGameSetting(g_EntityRegistry.LookupID(*it), _T("Name")));

                    if (pCvar != NULL)
                        m_pInterfaceManager->Trigger(UITRIGGER_BUILD_FAIL_REASON, _T("This building requires a ") + pCvar->GetString());
                }
                return false;
            }
        }
    }



    return true;
}


/*====================
  CGameClient::DrawHoverStats
  ====================*/
void    CGameClient::DrawHoverStats()
{
    if (!cg_drawHoverStats)
        return;

    IVisualEntity *pEntity(GetVisualEntity(m_uiHoverEntity));
    if (!pEntity)
        return;

    ResHandle hFont(g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));
    if (pFontMap == NULL)
        return;
    
    float fLines = 1.0f;

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const int   PANEL_WIDTH = 23;
    const float PANEL_HEIGHT = fLines;
    const float START_X = INT_FLOOR(FONT_WIDTH);
    const float START_Y = INT_FLOOR(Draw2D.GetScreenH() - (FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2) - 148.0f);
    const int   NAME_LENGTH = 13;

    float fDrawY = START_Y;
    tstring sStr;

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - FONT_WIDTH, START_Y - FONT_WIDTH, FONT_WIDTH * PANEL_WIDTH + FONT_WIDTH * 2, FONT_HEIGHT * PANEL_HEIGHT + FONT_WIDTH * 2);

    Draw2D.SetColor(WHITE);

    Draw2D.String(START_X, fDrawY, XtoA(_T("Entity")_T(":"), FMT_ALIGNLEFT, NAME_LENGTH) + XtoA(pEntity->GetTypeName()), hFont);
    fDrawY += FONT_HEIGHT;
    
    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*====================
  CGameClient::AddPersistantItem
  ====================*/
void    CGameClient::AddPersistantItem(int iVaultNum, ushort unType, uint uiID)
{
    tstring sName(_T("Empty Vault"));
    tstring sDescription(_T("You do not have an item in this vault space."));

    if (iVaultNum < 0 || iVaultNum >= MAX_PERSISTANT_ITEMS)
        return;

    if (m_sItemVault.unItemType[iVaultNum] == unType && m_sItemVault.uiItemID[iVaultNum] == uiID)
        return;

    m_sItemVault.unItemType[iVaultNum] = unType;
    m_sItemVault.uiItemID[iVaultNum] = uiID;

    if (unType != PERSISTANT_ITEM_NULL)
    {
        int iReplenishNum(unType % 10);
        int iIncreaseNum((unType / 10) % 10);
        int iRegenNum((unType / 100) % 10);
        int iTypeNum((unType / 1000) % 10);
        
        // Set up the item's name and description...
        sName = g_sPersistantRegenColors[iRegenNum];
        sName += _T(" ");
        sName += g_sPersistantItemTypes[iTypeNum];
        sName += _T(" of ");
        sName += g_sPersistantIncreaseNames[iIncreaseNum];
        sName += _T("'s ");
        sName += g_sPersistantReplenishNames[iReplenishNum];

        sDescription = _T("This ") + LowerString(g_sPersistantRegenColors[iRegenNum]) + _T(" ") + LowerString(g_sPersistantItemTypes[iTypeNum]);
        sDescription += _T(" holds the spirit of the ") + g_sPersistantIncreaseNames[iIncreaseNum] + _T(", providing a ");
        sDescription += XtoA((g_fPersistantItemTypeMultipliers[iTypeNum] - 1.0f) * 100, 0, 0, 0) + _T(" percent increase to your maximum ");
        sDescription += LowerString(g_sPersistantIncreaseMods[iIncreaseNum]) + _T(" and ") + LowerString(g_sPersistantRegenModifiers[iRegenNum]);
        sDescription += _T(", as well as allowing you to instantly increase your ") + LowerString(g_sPersistantReplenishMods[iReplenishNum]) + _T(".");
    }

    ICvar::CreateString(_T("Persistant_Vault") + XtoA(iVaultNum) + _T("_Name"), sName, CVAR_READONLY);
    ICvar::CreateString(_T("Persistant_Vault") + XtoA(iVaultNum) + _T("_Description"), sDescription, CVAR_READONLY);
}


/*====================
  CGameClient::SetReplayClient
  ====================*/
void    CGameClient::SetReplayClient(int iClientNum)
{
    if (!ReplayManager.IsPlaying())
        return;

    ClientInfoMap_it itClient(m_mapClients.find(iClientNum));
    if (itClient == m_mapClients.end())
        return;

    m_pLocalClient = itClient->second;

    m_pHostClient->SetClientNum(iClientNum);

    CBufferFixed<6> buffer;
    buffer << GAME_CMD_SET_REPLAY_CLIENT << iClientNum;
    SendGameData(buffer, true);

    m_pClientCommander->SetPlayerCommander(GetLocalPlayer() == NULL ? NULL : GetLocalPlayer()->GetAsCommander());
}


/*====================
  CGameClient::NextReplayClient
  ====================*/
void    CGameClient::NextReplayClient()
{
    if (!ReplayManager.IsPlaying() && m_pLocalClient)
        return;

    ClientInfoMap_it itClient(m_mapClients.find(m_pLocalClient->GetClientNumber()));
    if (itClient == m_mapClients.end())
        return;

    ClientInfoMap_it itStartClient(itClient);
    
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
    if (!ReplayManager.IsPlaying() && m_pLocalClient)
        return;

    ClientInfoMap_it itClient(m_mapClients.find(m_pLocalClient->GetClientNumber()));
    if (itClient == m_mapClients.end())
        return;

    ClientInfoMap_it itStartClient(itClient);
    
    do
    {
        if (itClient == m_mapClients.begin())
            itClient = m_mapClients.end();
        --itClient;
    } while (itClient != itStartClient && itClient->second->IsDisconnected());

    SetReplayClient(itClient->first);
}


/*====================
  CGameClient::AllowMovement
  ====================*/
void    CGameClient::AllowMovement(bool bValue)
{
    if (bValue != m_bAllowMovement) 
    { 
        BreakAutoRun();
        GetCurrentSnapshot()->SetButton(GAME_BUTTON_ALL, 0.0f);
    }
    
    m_bAllowMovement = bValue; 
}


/*====================
  CGameClient::UpdateTeamRosters
  ====================*/
void    CGameClient::UpdateTeamRosters(int iClientNum, int iNewTeam, byte yNewSquad, bool bOfficer, bool bCommander)
{
    CEntityClientInfo *pClient(GetClientInfo(iClientNum));

    for (int iTeam(0); iTeam <= 2; ++iTeam)
    {
        CEntityTeamInfo *pTeam(GameClient.GetTeam(iTeam));
        if (pTeam == NULL)
            continue;

        if (iTeam == iNewTeam && pClient != NULL && !pClient->IsDisconnected())
        {
            if (pTeam->GetTeamIndexFromClientID(iClientNum) == -1)
            {
                ivector &vClients(pTeam->GetClientList());

                // Add this client to the client list
                vClients.push_back(iClientNum);

                pTeam->SortClientList();
            }
            else if (pTeam->GetSquadFromClientID(iClientNum) != yNewSquad ||
                (pTeam->GetCommanderClientID() == iClientNum) != bCommander ||
                (pTeam->GetOfficerClientID(yNewSquad) == iClientNum) != bOfficer)
            {
                pTeam->SortClientList();
            }
        }
        else
        {
            if (pTeam->GetTeamIndexFromClientID(iClientNum) != -1)
            {
                ivector &vClients(pTeam->GetClientList());
    
                // Delete this client from the client list
                ivector_it it(vClients.begin());
                while (it != vClients.end())
                {
                    if (*it == iClientNum)
                    {
                        it = vClients.erase(it);
                        continue;
                    }
                    ++it;
                }

                pTeam->SortClientList();
            }
        }
    }
}


/*====================
  CGameClient::DoUpkeepEvent
  ====================*/
void    CGameClient::DoUpkeepEvent(bool bFailed, int iTeam)
{
    if (m_pLocalClient == NULL)
        return;

    CEntityTeamInfo *pTeam(GetTeam(m_pLocalClient->GetTeam()));
    if (pTeam == NULL)
        return;

    int iNotification(0);
    if (LowerString(pTeam->GetDefinition()->GetName()) == _T("beast"))
        iNotification |= BIT(0);
    if (pTeam->GetTeamID() != iTeam)
        iNotification |= BIT(1);
    if (!bFailed)
        iNotification |= BIT(2);

    UpkeepEvent.Trigger(XtoA(iNotification & ~BIT(0)));

    K2SoundManager.Play2DSound(m_hUpkeepNotification[iNotification]);

    /*
    Console << _T("UPKEEP EVENT: ")
            << ((iNotification & BIT(0)) ? _T("beast") : _T("human")) << SPACE
            << ((iNotification & BIT(1)) ? _T("enemy") : _T("ally")) << SPACE
            << ((iNotification & BIT(2)) ? _T("restored") : _T("failed")) << newl;
    */
}


/*====================
  CGameClient::SuddenDeathAlert
  ====================*/
void    CGameClient::SuddenDeathAlert()
{
    m_pInterfaceManager->Trigger(UITRIGGER_SUDDEN_DEATH_ALERT, Game.GetGameTime());
    K2SoundManager.Play2DSound(m_hSuddenDeathSample);
}


/*====================
  CGameClient::GetPropType
  ====================*/
tstring CGameClient::GetPropType(const tstring &sPath) const
{
    CStringTable *StringTable(g_ResourceManager.GetStringTable(m_hPropTypeStringTable));

    if (!StringTable)
    {
        Console.Warn << _T("Prop type StringTable not found") << newl;

        return _T("");
    }
    
    tstring sType(StringTable->Get(sPath));
    if (sType.compare(sPath))
        return sType;
    else
        return _T("");
}


/*====================
  CGameClient::CanJoinTeam
  ====================*/
bool    CGameClient::CanJoinTeam(int iTeam)
{
    if (m_pLocalClient == NULL)
    {
        Console.Warn << _T("Could not find local client") << newl;
        return false;
    }
    
    if (m_pLocalClient->GetTeam() == iTeam || iTeam < 0 || iTeam >= Game.GetNumTeams())
        return false;

    int iOldTeam(m_pLocalClient->GetTeam());

    CEntityTeamInfo *pNewTeam(GetTeam(iTeam));

    if (pNewTeam == NULL || iTeam == iOldTeam)
        return false;

    int iNewSF(pNewTeam->GetAverageSF());

    CStateString *pState = Host.GetStateString(1);  //STATE_STRING_SERVER_INFO

    if (pState != NULL && pNewTeam->GetNumClients() >= (pState->GetInt(_T("svr_maxPlayers")) + 1) / 2)
        return false;

    if (iTeam != 0)
    {
        for (int i(1); i < Game.GetNumTeams(); ++i)
        {
            CEntityTeamInfo *pTeam(Game.GetTeam(i));
            int iTeamCount;
            int iSF;

            if (pTeam == NULL || i == iTeam)
                continue;

            if (i == iOldTeam)
            {
                iTeamCount = pTeam->GetNumClients() - 1;
                iSF = ((pTeam->GetAverageSF() * pTeam->GetNumClients()) - m_pLocalClient->GetPersistantStat(PLAYER_PERSISTANT_SKILLFACTOR)) * (pTeam->GetNumClients() - 1);
            }
            else
            {
                iTeamCount = pTeam->GetNumClients();
                iSF = pTeam->GetAverageSF();
            }

            if (pState != NULL && iTeamCount < (pNewTeam->GetNumClients() + 1) - pState->GetInt(_T("sv_maxTeamDifference")))
                return false;

            if (iTeamCount == pNewTeam->GetNumClients() && iSF < iNewSF)
                return false;
        }
    }

    return true;
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
}


/*====================
  CGameClient::GetTerrainType
  ====================*/
tstring CGameClient::GetTerrainType()
{
    switch (m_eEventTarget)
    {
    case CG_EVENT_TARGET_ENTITY:
        return m_pCurrentEntity->GetCurrentEntity()->GetTerrainType();

    case CG_EVENT_TARGET_HANDS:
        {
            IPlayerEntity *pLocalPlayer(GetLocalPlayerCurrent());
            if (pLocalPlayer)
                return pLocalPlayer->GetTerrainType();
            else
                return _T("grass");
        }
    }

    return _T("grass");
}

/*====================
  CGameClient::AllowMouseAim
  ====================*/
bool    CGameClient::AllowMouseAim()
{
    if (GameClient.GetCurrentInterface() == CG_INTERFACE_PLAYER_BUILD)
        return false;

    CEntityClientInfo *pClient(GetLocalClient());

    if (pClient == NULL)
        return false;

    if (pClient->IsDemoAccount() && GetCurrentInterface() == CG_INTERFACE_DEAD)
        return false;

    if (pClient->IsDemoAccount() && pClient->GetDemoTimeRemaining() < GetCurrentGameLength())
        return false;

    return m_bAllowMouseAim;
}


/*====================
  CGameClient::AllowMovement
  ====================*/
bool    CGameClient::AllowMovement()
{
    CEntityClientInfo *pClient(GetLocalClient());

    if (pClient == NULL)
        return false;

    if (pClient->IsDemoAccount() && pClient->GetDemoTimeRemaining() < GetCurrentGameLength())
        return false;

    return m_bAllowMovement;
}


/*====================
  CGameClient::GetConnectedClientCount
  ====================*/
int     CGameClient::GetConnectedClientCount(int iTeam)
{
    if (iTeam == -1)
    {
        int iNumClients(0);

        for (ClientInfoMap_it it(m_mapClients.begin()); it != m_mapClients.end(); it++)
            if (!it->second->IsDisconnected())
                iNumClients++;

        return iNumClients;
    }

    CEntityTeamInfo *pTeam(GetTeam(iTeam));
    if (pTeam == NULL)
        return 0;

    return pTeam->GetNumClients();
}


/*====================
  CGameClient::SelectItem
  ====================*/
void    CGameClient::SelectItem(int iSlot)
{
    if (IsCommander() && iSlot != -1)
    {
        IPlayerEntity *pPlayer(GetLocalPlayerCurrent());
        if (pPlayer)
        {
            IInventoryItem *pItem(pPlayer->GetItem(iSlot));
            if (pItem && pItem->IsReady() && pPlayer->GetMana() >= pItem->GetManaCost())
                m_CurrentClientSnapshot.SelectItem(iSlot);

        }
    }
    else
    {
        m_CurrentClientSnapshot.SelectItem(iSlot);
    }
}


/*====================
  CGameClient::RemoveClient
  ====================*/
void    CGameClient::RemoveClient(int iClientNum)
{
    if (m_pLocalClient && m_pLocalClient->GetClientNumber() == iClientNum)
        m_pLocalClient = NULL;

    UpdateTeamRosters(iClientNum, -1, -1, false, false);
    m_mapClients.erase(iClientNum);
}

/*====================
  CGameClient::ClearTransparentEntities
  ====================*/
void    CGameClient::ClearTransparentEntities()
{
    for (uiset::iterator it(m_setTransparent.begin()); it != m_setTransparent.end(); it++)
    {
        IVisualEntity *pEnt(GameClient.GetVisualEntity(*it));

        if (pEnt != NULL)
            pEnt->RemoveClientRenderFlags(ECRF_HALFTRANSPARENT);
    }

    m_setTransparent.clear();
}
