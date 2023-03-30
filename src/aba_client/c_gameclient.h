// (C)2005 S2 Games
// c_gameclient.h
//
//=============================================================================
#ifndef __C_GAMECLIENT_H__
#define __C_GAMECLIENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_cliententity.h"

#include "../aba_shared/c_player.h"
#include "../aba_shared/c_visibilitymap.h"

#include "../k2/c_clientsnapshot.h"
#include "../k2/c_snapshot.h"
#include "../k2/c_rasterbuffer.h"
#include "../k2/c_hostclient.h"
#include "../k2/i_resourcedependent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameClient;

class CWorld;
class CCamera;
class IGameEntity;
class CPlayer;
class CSnapshot;
class CClientEntityDirectory;
class CGameInterfaceManager;
class CPlayerCommander;
class CClientCommander;
class CGameInfo;
class CEntityEffect;
class CStateBlock;
class CClientAccount;
class CFileHTTP;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GameClient (*CGameClient::GetCurrentClientGamePointer())

EXTERN_CVAR_BOOL(cg_unitPlayerColors);
EXTERN_CVAR_BOOL(cg_heroIndicators);
EXTERN_CVAR_BOOL(cg_dev);
EXTERN_CVAR_BOOL(cg_fogofwar);
EXTERN_CVAR_BOOL(chat_showChatTimestamps);
EXTERN_CVAR_BOOL(cg_lockCamera);

extern bool g_bMouseLook;

enum EClientEventTarget
{
    CG_EVENT_TARGET_ENTITY
};

enum EGameInterface
{
    CG_INTERFACE_INVALID,
    CG_INTERFACE_MAIN,
    CG_INTERFACE_ONLINE,
    CG_INTERFACE_LOADING,
    CG_INTERFACE_LOBBY,
    CG_INTERFACE_HERO_SELECT,
    CG_INTERFACE_HERO_LOADING,
    CG_INTERFACE_GAME,
    CG_INTERFACE_GAME_SPECTATOR,

    NUM_CG_INTERFACES
};

typedef deque<CClientSnapshot>          ClientSnapshotDeque;
typedef ClientSnapshotDeque::iterator   ClientSnapshotDeque_it;

struct SOverlayInfo
{
    ResHandle   hMaterial;
    CVec4f      v4Color;
};

struct SPopupMessage
{
    tstring     sText;
    uint        uiEntityIndex;
    CVec3f      v3Pos;
    uint        uiServerTime;
    uint        uiSpawnTime;
    uint        uiLifeTime;
    uint        uiFadeTime;
    CVec4f      v4Color;
    CVec2f      v2Start;
    CVec2f      v2Speed;
};

struct SMinimapPing
{
    ResHandle   hTexture;
    CVec2f      v2Pos;
    uint        uiSpawnTime;
    CVec4f      v4Color;
};

enum EClientGameEffect
{
    CGFX_PING = 0,
    NUM_CLIENT_GAME_EFFECT_CHANNELS
};

const uint  NUM_CLIENT_GAME_EFFECTS(32);
const uint  NUM_CLIENT_GAME_EFFECT_THREADS(NUM_CLIENT_GAME_EFFECT_CHANNELS + NUM_CLIENT_GAME_EFFECTS);

const uint  VC_MIN_TIME(2000); // min time between using VC's

// Sub-menu voice command options
struct VCSub
{
    tstring sDesc;
    ResHandle hSound;
    EButton eButton;
};

typedef map<tstring, VCSub>             VCSubMap;
typedef pair<tstring, VCSub>            VCSubPair;
typedef VCSubMap::iterator              VCSubMap_it;

// Main menu voice command categories
struct VCCategory
{
    tstring sDesc;
    EButton eButton;
    EVCType eType;
    map<EButton, VCSubMap> mapSubItems;
};

typedef map<EButton, VCCategory>        VCMap;
typedef pair<EButton, VCCategory>       VCPair;
typedef VCMap::iterator                 VCMap_it;

extern CUITrigger Scores;

enum EClientResource
{
    CLIENT_RESOURCE_UNTRACKED,

    CLIENT_RESOURCE_GAME_MESSAGE_TABLE,
    CLIENT_RESOURCE_ENTITY_STRING_TABLE,
    CLIENT_RESOURCE_GAME_SOUND_TABLE,
    CLIENT_RESOURCE_MOVE_INDICATOR_EFFECT,
    CLIENT_RESOURCE_ATTACK_INDICATOR_EFFECT,
    CLIENT_RESOURCE_CAST_INDICATOR_EFFECT,
    CLIENT_RESOURCE_PLAYER_TALKING_EFFECT,
    CLIENT_RESOURCE_PLAYER_DISCONNECTED_EFFECT,
    CLIENT_RESOURCE_ILLUSION_EFFECT,
    CLIENT_RESOURCE_PING_SAMPLE,
    CLIENT_RESOURCE_START_GAME_SAMPLE,
    CLIENT_RESOURCE_FIRST_BLOOD_SAMPLE,
    CLIENT_RESOURCE_SELECT_ITEM_SAMPLE,
    CLIENT_RESOURCE_PLACE_ITEM_SAMPLE,
    CLIENT_RESOURCE_PICKUP_ITEM_SAMPLE,
    CLIENT_RESOURCE_KILL_STREAK_SAMPLE,
    CLIENT_RESOURCE_MULTIKILL_SAMPLE = CLIENT_RESOURCE_KILL_STREAK_SAMPLE + 8,
    CLIENT_RESOURCE_DENIED_SAMPLE = CLIENT_RESOURCE_MULTIKILL_SAMPLE + 4,
    CLIENT_RESOURCE_TEAM_KILL_STREAK_SAMPLE,
    CLIENT_RESOURCE_TEAM_WIPE_SAMPLE,
    CLIENT_RESOURCE_ALT_INFO_INTERFACE,
    CLIENT_RESOURCE_ALT_INFO_CREEP_INTERFACE,
    CLIENT_RESOURCE_ALT_INFO_HERO_INTERFACE,
    CLIENT_RESOURCE_ALT_INFO_BUILDING_INTERFACE,
    CLIENT_RESOURCE_PROP_TYPE_TABLE,
    CLIENT_RESOURCE_SELECTION_INDICATOR,
    CLIENT_RESOURCE_PING_EFFECT,
    CLIENT_RESOURCE_HERO_SELECT_SAMPLE,
    CLIENT_RESOURCE_HERO_BAN_SAMPLE,

    CLIENT_RESOURCE_VICTORY_SAMPLE,
    CLIENT_RESOURCE_DEFEAT_SAMPLE,
    CLIENT_RESOURCE_LEGION_WIN_SAMPLE,
    CLIENT_RESOURCE_HELLBOURNE_WIN_SAMPLE,

    CLIENT_RESOURCE_VOTE_CALLED_SAMPLE,
    CLIENT_RESOURCE_VOTE_PASSED_SAMPLE,
    CLIENT_RESOURCE_VOTE_FAILED_SAMPLE,

    CLIENT_RESOURCE_SELECTED_SAMPLE,
    CLIENT_RESOURCE_ABILITY_ACTIVATE_ERROR_SAMPLE,
    CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE,

    CLIENT_RESOURCE_SMACKDOWN_SAMPLE,
    CLIENT_RESOURCE_HUMILIATION_SAMPLE,

    CLIENT_RESOURCE_COUNTDOWN_START_SAMPLE,

    CLIENT_RESOURCE_RAGE_QUIT_SAMPLE,
        
    CLIENT_RESOURCE_HELLBOURNE_DESTROY_TOWER_SAMPLE,
    CLIENT_RESOURCE_LEGION_DESTROY_TOWER_SAMPLE,
    CLIENT_RESOURCE_HELLBOURNE_BARRACKS_DESTROYED_SAMPLE,
    CLIENT_RESOURCE_LEGION_BARRACKS_DESTROYED_SAMPLE,
    CLIENT_RESOURCE_BASE_UNDER_ATTACK_SAMPLE,

    CLIENT_RESOURCE_ALLY_HERO_UNDER_ATTACK_SAMPLE,
    CLIENT_RESOURCE_OUR_HERO_UNDER_ATTACK_SAMPLE,

    CLIENT_RESOURCE_UNPAUSING_SAMPLE,
    
    CLIENT_RESOURCE_KONGOR_SLAIN_SAMPLE,

    NUM_CLIENT_RESOURCES
};

struct SClientResourceLoadRequest
{
    EClientResource m_eID;
    int             m_iType;
    tstring         m_sName;

    SClientResourceLoadRequest(EClientResource eID, const tstring &sName, int iType) :
    m_eID(eID),
    m_sName(sName),
    m_iType(iType)
    {}

    bool    operator==(const SClientResourceLoadRequest &B) const
    {
        return (m_eID == B.m_eID) && (m_iType == B.m_iType) && (m_sName == B.m_sName);
    };
};

typedef deque<SClientResourceLoadRequest>   ClientResourceList;
typedef ClientResourceList::iterator        ClientResourceList_it;

enum EWorldThing
{
    WORLD_THING_ENTITY,
    WORLD_THING_SOUND,
    WORLD_THING_LIGHT,
    WORLD_THING_BIT_ENTITY
};

struct SWordldThingSpawnRequest
{
    EWorldThing m_eType;
    uint        m_uiWorldIndex;
    uint        m_uiGameIndex;
    uint        m_uiBitIndex;
    tstring     m_sName;

    SWordldThingSpawnRequest(EWorldThing eType, uint uiWorldIndex, uint uiGameIndex, uint uiBitIndex = 0) :
    m_eType(eType),
    m_uiWorldIndex(uiWorldIndex),
    m_uiGameIndex(uiGameIndex),
    m_uiBitIndex(uiBitIndex)
    {}

    SWordldThingSpawnRequest(EWorldThing eType, uint uiWorldIndex, uint uiGameIndex, const tstring &sName) :
    m_eType(eType),
    m_uiWorldIndex(uiWorldIndex),
    m_uiGameIndex(uiGameIndex),
    m_uiBitIndex(0),
    m_sName(sName)
    {}
};

class CEntityStringTableDependent : public IResourceDependent
{
private:
public:
    ~CEntityStringTableDependent()  {}
    CEntityStringTableDependent()   {}

    void    Rebuild(ResHandle hResource);
};

typedef deque<SWordldThingSpawnRequest> WorldThingList;
typedef WorldThingList::iterator        WorldThingList_it;
//=============================================================================

//=============================================================================
// CGameClient
//=============================================================================
class CGameClient : public IGame
{
private:
    tsmapts                 m_mapCensor;

    CHostClient*            m_pHostClient;
    CClientEntityDirectory* m_pClientEntityDirectory;
    CGameInterfaceManager*  m_pInterfaceManager;
    CCamera*                m_pCamera;

    CVec3f                  m_v3CameraPosition;
    CVec3f                  m_v3CameraAngles;
    uint                    m_uiCameraFrame;
    uint                    m_uiCameraIndex;
    CVec3f                  m_v3CameraCenter;

    CSnapshot               m_CurrentServerSnapshot;
    vector<PoolHandle>      m_vServerSnapshots;
    PoolHandle              m_hServerSnapshotFallback;
    uint                    m_uiSnapshotBufferPos;

    HeroList                m_vHeroes[NUM_HERO_LISTS];

    CFileHTTP*              m_pReplayDownload;
    tstring                 m_sLastReplay;
    bool                    m_bDownloadingReplay;
    
    // Interface
    bool                    m_bShowMenu;
    bool                    m_bShowLobby;
    uint                    m_uiLastGamePhase;
    EGameInterface          m_eLastInterface;

    CPlayer*                m_pLocalPlayer;
    CClientEntity*          m_pCurrentEntity;
    CClientCommander*       m_pClientCommander;
    EClientEventTarget      m_eEventTarget;

    vector<SPopupMessage>   m_vPopupMessage;
    ResHandle               m_hPopupFont;

    vector<SMinimapPing>    m_vMinimapPing;

    // Input
    CClientSnapshot         m_CurrentClientSnapshot;
    CClientSnapshot         m_PendingClientSnapshot;

    ClientResourceList      m_deqClientResources;
    bool                    m_bStartedLoadingResources;
    uint                    m_uiTotalResourcesToLoad;
    ResHandle               m_ahResources[NUM_CLIENT_RESOURCES];

    HeroPrecacheList        m_deqHeroesToLoad;
    size_t                  m_zTotalHeroesToLoad;
    bool                    m_bStartedLoadingHeroes;

    WorldThingList          m_deqWorldThings;
    bool                    m_bStartedSpawningEntities;
    uint                    m_uiTotalWorldThings;

    ResHandle               m_hMinimapReference;
    ResHandle               m_hMinimapTexture;
    class CBitmap*          m_pMinimapBitmap;

    ResHandle               m_hLoadingTexture;
    
    // Visual
    vector<SOverlayInfo>    m_vOverlays;
    CVec4f                  m_v4ScreenEffectColor;
    CVec3f                  m_v3CameraEffectAngleOffset;
    CVec3f                  m_v3CameraEffectOffset;
    uint                    m_uiOrderEvent;
    byte                    m_yOrderUniqueID;
    CEffectThread*          m_apEffectThreads[NUM_CLIENT_GAME_EFFECT_THREADS];

    // Sounds
    SoundHandle             m_ahSoundHandle[NUM_CLIENT_SOUND_HANDLES];
    uint                    m_uiLastConfirmAttackSoundTime;
    uint                    m_uiLastConfirmMoveSoundTime;
    uint                    m_uiLastSelectedSoundTime;

    deque<ResHandle>        m_deqHeroAnnouncements;
    SoundHandle             m_hHeroAnnounce;

    bool                    m_bLevelup;
    uint                    m_uiLevelupIndex;

    ushort                  m_unLastHeroType;
    bool                    m_bProcessedFirstSnapshot;

    CPlayer*                m_pReplaySpectator;

    uint                    m_uiLastSpeedUpdate;
    uint                    m_uiNumSpeedUpdates;
    uint                    m_uiFirstSpeedUpdate;

    uint                    m_uiDelayHeroLoading;

    uint                    m_uiLastCheckedCustomFiles;

    bool                    m_bJustStartedGame;
    bool                    m_bWasUsingCustomFiles;
    bool                    m_bWereCoreFilesModified;

    // World Sounds
    struct  SWorldSound
    {
        SoundHandle hSound;
        ResHandle   hSample;
        uint        uiNextStartTime;
        SWorldSound(SoundHandle sound = INVALID_INDEX, ResHandle sample = INVALID_INDEX, uint start = 0) : hSound(sound), hSample(sample), uiNextStartTime(start) {}
    };
    typedef map<int, SWorldSound>           SWorldSoundsMap;
    typedef SWorldSoundsMap::iterator       SWorldSoundsMap_it;
    SWorldSoundsMap                         m_mapWorldSounds;
    SoundHandle                             m_hWorldAmbientSound;

    // Pinging
    bool                    m_bPinging;
    bool                    m_bPingEffectActive;
    
    struct SOrders
    {
        byte    yOrder;
        CVec3f  v3OrderPos;

        bool operator<(const SOrders &B) const
        {
            if (yOrder < B.yOrder)
                return true;
            else if (yOrder > B.yOrder)
                return false;

            if (v3OrderPos.x < B.v3OrderPos.x)
                return true;
            else if (v3OrderPos.x > B.v3OrderPos.x)
                return false;

            if (v3OrderPos.y < B.v3OrderPos.y)
                return true;
            else if (v3OrderPos.y > B.v3OrderPos.y)
                return false;
            
            if (v3OrderPos.z < B.v3OrderPos.z)
                return true;
            else if (v3OrderPos.z > B.v3OrderPos.z)
                return false;

            return false;
        }
    };

    // Interface
    vector<IVisualEntity *> m_vVision;

    uint                    m_uiItemCursorIndex;

    CVisibilityMap          m_cVisibilityMap;
    uint                    m_uiVisibilitySize;
    float                   m_fVisibilityScale;
    CBitmap*                m_pFogofWarBitmap;
    uint                    m_uiLastFogofWarUpdate;
    CRasterBuffer           m_cVisRaster;
    CRasterBuffer           m_cOccRaster;
    uint                    m_uiLastMinimapUpdateTime;

    tstring                 m_sActiveShop;
    tstring                 m_sActiveRecipe;

    CVec3f                  m_v3LastEvent;

    void        DrawAreaCast();
    void        DrawAltInfo();
    void        DrawVoiceInfo();

    void        DrawPopupMessages();
    
    void        DrawFogofWar();

    void        UpdateClientGameEffect(int iChannel, bool bActive, const CVec3f &v3Pos);
    void        AddClientGameEffects();

    void        SendClientSnapshot();

    void        StopWorldSounds();
    void        WorldSoundsFrame();

    void        LoadHeroesFrame();
    void        SetupFrame();
    void        ActiveFrame();
    void        EndedFrame();
    void        BackgroundFrame(bool bProcessBinds);
    void        GamePhaseChanged();

    void        WriteConnectionInfo();

    void        PrecacheEntities();

    void        DrawSelectedStats();
    
    void        RenderSelectedPlayerView(uint uiPlayerIndex);

    void        DrawNavGrid();
    void        DrawSelectedPath();

    void        RenderWorldEntities();
    void        AddSelectionRingToScene(uint uiIndex, const CVec3f &v3Pos, float fSize, uint uiOrderTime);

    void        DumpSnapshot(CSnapshot &snapshot);

    void        ValidateLevelup();

    void        FrameDownloadingReplay();
    void        UpdateCamera();

    void        PrecacheClientResources();

    void        InitCensor();

    void        UpdateNotifyFlags();

public:
    ~CGameClient();
    CGameClient();

    bool                IsClient()                                      { return true; }
    virtual int         GetLocalClientNum();
    virtual CPlayer*    GetLocalPlayer()                                { return m_pLocalPlayer; }

    static CGameClient* GetCurrentClientGamePointer()                   { return static_cast<CGameClient*>(GetCurrentGamePointer()); }

    // API Functions
    void                SetGamePointer()                                { IGame::SetCurrentGamePointer(this); }
    bool                Initialize(CHostClient *pHostClient);
    void                Reinitialize();
    void                StartLoadingWorld();
    void                SpawnNextWorldEntity();
    bool                IsSpawningEntities()                            { return m_bStartedSpawningEntities; }
    bool                IsFinishedSpawningEntities()                    { return m_bStartedSpawningEntities && m_deqWorldThings.empty(); }
    float               GetEntitySpawningProgress()                     { return (m_uiTotalWorldThings - INT_SIZE(m_deqWorldThings.size())) / float(m_uiTotalWorldThings); }
    void                StartLoadingResources();
    void                LoadNextResource();
    bool                IsFinishedLoadingResources()                    { return m_bStartedLoadingResources && m_deqClientResources.empty(); }
    float               GetResourceLoadingProgress()                    { return (m_uiTotalResourcesToLoad - INT_SIZE(m_deqClientResources.size())) / float(m_uiTotalResourcesToLoad); }
    void                PreFrame();
    void                Frame();
    uint                Shutdown();
    bool                ProcessGameEvents(CSnapshot &snapshot);
    bool                ProcessSnapshot(CSnapshot &snapshot);
    bool                ProcessGameData(CPacket &pkt);
    void                PrecacheAll();
    void                PrecacheWorld(const tstring &sWorldName);
    void                Connect(const tstring &sAddr);
    
    bool                IsEntitySelected(uint uiIndex);
    bool                IsEntityHoverSelected(uint uiIndex);
    uint                GetActiveControlEntity();

    bool                IsVisible(float fX, float fY);

    void                SendGameData(const IBuffer &buffer, bool bReliable);
    void                SendGameData(int iClientNum, const IBuffer &buffer, bool bReliable) { SendGameData(buffer, bReliable); }

    void                SetCurrentEntity(CClientEntity *pCurrentEntity) { m_pCurrentEntity = pCurrentEntity; }
    void                SetEventTarget(EClientEventTarget eTarget)      { m_eEventTarget = eTarget; }

    const HeroList&     GetHeroList(uint uiIndex) const                 { assert(uiIndex < NUM_HERO_LISTS); return m_vHeroes[uiIndex]; }

    bool                    InterfaceNeedsUpdate();
    EGameInterface          GetCurrentInterface() const;
    
    void                    ToggleMenu()                                    { m_bShowMenu = !m_bShowMenu; }
    void                    HideMenu()                                      { m_bShowMenu = false; }
    bool                    GetShowMenu()                                   { return m_bShowMenu; }

    //void                  ToggleLobby()                                   { m_bShowLobby = !m_bShowLobby; }
    //void                  HideLobby()                                     { m_bShowLobby = false; }
    //bool                  GetShowLobby()                                  { return m_bShowLobby; }

    CGameInterfaceManager*  GetInterfaceManager()                           { return m_pInterfaceManager; }

    bool                    IsFinishedLoadingHeroes() const                 { return m_deqHeroesToLoad.empty(); }

    // Input
    CClientSnapshot*    GetCurrentSnapshot()                            { return &m_CurrentClientSnapshot; }
    bool                TraceCursor(STraceInfo &trace, int iIgnoreSurface);
    void                Cancel();

    // Resources
    ResHandle           RegisterModel(const tstring &sPath);
    ResHandle           RegisterEffect(const tstring &sPath);
    ResHandle           RegisterIcon(const tstring &sPath);
    ResHandle           RegisterSample(const tstring &sPath);
    ResHandle           RegisterMaterial(const tstring &sPath);

    void                GetPrecacheList(const tstring &sName, EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache);
    void                Precache(const tstring &sName, EPrecacheScheme eScheme);
    void                Precache(ushort unType, EPrecacheScheme eScheme);

    // Visual
    void                AddOverlay(const CVec4f &v4Color, ResHandle hMaterial);
    void                AddOverlay(const CVec4f &v4Color)               { AddOverlay(v4Color, g_ResourceManager.GetWhiteTexture()); }

    void                StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge);
    void                StopEffect(int iChannel);
    void                PlaySound(const tstring &sSound, int iChannel, float fFalloff, float fVolume, int iSoundFlags, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0, float fFalloffEnd = 0.0f);
    void                PlaySoundStationary(const tstring &sSound, int iChannel, float fFalloff, float fVolume);
    void                StopSound(int iChannel);

    void                PlayClientGameSound(const tstring &sSound, int iChannel, float fVolume, int iSoundFlags, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
    void                StopClientGameSound(int iChannel);

    int                 StartClientGameEffect(const tstring &sEffect, int iChannel, int iTimeNudge, const CVec3f &v3Pos);
    void                StopClientGameEffect(int iChannel);

    CPlayer*            GetLocalPlayer() const                          { return m_pLocalPlayer; }
    uint                GetLocalPlayerIndex() const                     { return (m_pLocalPlayer == NULL) ? INVALID_INDEX : m_pLocalPlayer->GetIndex(); }
    CCamera*            GetCamera() const                               { return m_pCamera; }
    CClientCommander*   GetClientCommander() const                      { return m_pClientCommander; }
    CClientEntity*      GetClientEntity(uint uiIndex) const;
    IVisualEntity*      GetClientEntityCurrent(uint uiIndex) const;
    IVisualEntity*      GetClientEntityNext(uint uiIndex) const;
    IVisualEntity*      GetClientEntityPrev(uint uiIndex) const;
    CPlayer*            GetPlayerByName(const tstring &sName) const;
    uint                GetNumClients() const;

    // Interface
    void                UpdateMinimap();
    void                UpdateMinimapPings();
    void                UpdateMinimapTexture();

    void                ForceInterfaceRefresh();

    // Camera
    void                AddCameraEffectAngleOffset(const CVec3f &v3Angles)  { m_v3CameraEffectAngleOffset += v3Angles; } 
    void                AddCameraEffectOffset(const CVec3f &v3Position)     { m_v3CameraEffectOffset += v3Position; }

    virtual void        SendMessage(const tstring &sMsg, int iClientNum);

    void                SpawnPopupMessage(const tstring &sText, uint uiEntityIndex, const CVec4f &v4Color, float fStartX, float fStartY, float fSpeedX, float fSpeedY, uint uiLifeTime, uint uiFadeTime, uint uiServerTime = INVALID_TIME);
    void                SpawnMinimapPing(ResHandle hTexture, const CVec2f &v2Pos, const CVec4f &v4Color, bool bPlaySound = true);

    void                StartPinging()                      { m_bPinging = true; }
    bool                StopPinging()                       { if (!m_bPinging) return false; m_bPinging = false; return true; }

    virtual CStateString&   GetStateString(uint uiID);
    virtual CStateBlock&    GetStateBlock(uint uiID);

    virtual uint        GetServerFrame();
    virtual uint        GetServerTime() const;
    virtual uint        GetPrevServerTime();
    virtual uint        GetServerFrameLength();
    virtual tstring     GetServerVersion();

    void                SetReplayClient(int iClientNum);

    void                UpdateTeamRosters();

    void                NextReplayClient();
    void                PrevReplayClient();

    const tstring&      GetPropType(const tstring &sPath) const;

    const tstring&      GetTerrainType();

    const vector<IVisualEntity *>&  GetVision() const                       { return m_vVision; }

    int                 GetConnectedClientCount(int iTeam = -1);

    void                RemoveClient(int iClientNum);

    uint                GetItemCursorIndex()                        { return m_uiItemCursorIndex; }
    void                SetItemCursorIndex(uint uiIndex)            { m_uiItemCursorIndex = uiIndex; }
    void                PrimaryAction(int iSlot);
    void                SecondaryAction(int iSlot);
    void                ItemPlace(int iSlot);
    void                PrimaryActionStash(int iSlot);
    void                SecondaryActionStash(int iSlot);
    void                ItemPlaceStash(int iSlot);
    void                ItemTakeFromStash(int iSlot);
    void                ItemPlaceHero();
    void                ItemPlaceSelected(int iSlot);
    void                ItemPlaceEntity(uint uiIndex);
    void                ItemSell(int iSlot);
    void                ItemDisassemble(int iSlot);

    void                StateStringChanged(uint uiID, const CStateString &ss);
    void                StateBlockChanged(uint uiID, const CStateBlock &block);

    static bool         ParticleTrace(const CVec3f &v3Start, const CVec3f &v3End, CVec3f &v3EndPos, CVec3f &v3Normal);

    const CVec3f&       GetCameraCenter() const                 { return m_v3CameraCenter; }

    void                SetActiveShop(const tstring &sShop, bool bForce = false);
    const tstring&      GetActiveShop() const                   { return m_sActiveShop; }

    void                SetActiveRecipe(const tstring &sRecipe, bool bHistory, bool bForce = false);
    const tstring&      GetActiveRecipe() const                 { return m_sActiveRecipe; }

    bool                CanAccessShop(const tstring &sShop);
    bool                CanAccessLocalShop(const tstring &sShop);
    bool                CanAccessItem(const tstring &sItem);
    bool                CanAccessItemLocal(const tstring &sItem);

    uint                GetItemStock(const tstring &sItem);
    uint                GetItemRestockTime(const tstring &sItem);

    ushort              GetShop(const tstring &sItem);

    void                GetUsedIn(const tstring &sItem, vector<ushort> &vUsedIn);

    void                SendCreateGameRequest(const tstring &sName, const tstring &sSettings);

    bool                    IsConnected() const     { return m_pHostClient->GetState() >= CLIENT_STATE_CONNECTED; }
    const CClientAccount&   GetAccount() const      { return m_pHostClient->GetAccount(); }

    CSnapshot&          GetCurrentServerSnapshot()  { return m_CurrentServerSnapshot; }

    tstring             GetGameMessage(const tstring &sKey, const tsmapts &sTokens = SMAPS_EMPTY);
    const tstring&      GetEntityString(const tstring &sKey) const;
    const tstring&      GetEntityString(uint uiIndex) const;
    uint                GetEntityStringIndex(const tstring &sKey) const;
    
    void                PostProcessEntities();

    void                AddResourceToLoadingQueue(EClientResource eResource, const tstring &sName, int iType)   { m_deqClientResources.push_back(SClientResourceLoadRequest(eResource, sName, iType)); }
    ResHandle           GetResource(EClientResource eResource)                                                  { return m_ahResources[eResource]; }

    uint                GetLastConfirmAttackSoundTime() const                               { return m_uiLastConfirmAttackSoundTime; }
    void                SetLastConfirmAttackSoundTime(uint uiLastConfirmAttackSoundTime)    { m_uiLastConfirmAttackSoundTime = uiLastConfirmAttackSoundTime; }
    uint                GetLastConfirmMoveSoundTime() const                                 { return m_uiLastConfirmMoveSoundTime; }
    void                SetLastConfirmMoveSoundTime(uint uiLastConfirmMoveSoundTime)        { m_uiLastConfirmMoveSoundTime = uiLastConfirmMoveSoundTime; }
    uint                GetLastSelectedSoundTime() const                                    { return m_uiLastSelectedSoundTime; }
    void                SetLastSelectedSoundTime(uint uiLastSelectedSoundTime)              { m_uiLastSelectedSoundTime = uiLastSelectedSoundTime; }

    bool                LevelupAbility(byte ySlot);
    void                SetLevelup(bool bLevelup);
    bool                GetLevelup() const          { return m_bLevelup; }
    void                ToggleLevelup();

    void                SendRemoteCommand(const tstring &sCmd)      { m_pHostClient->SendRemoteCommand(sCmd); }

    bool                Purchase(int iSlot);
    bool                PurchaseComponent(int iSlot, int iVariation);
    bool                PurchaseAllComponents(const tstring sName);
    bool                PurchaseUsedIn(int iSlot);
    void                Shop(int iSlot);

    bool                GetGameListStatus(bool &bList, uint &uiProcessed, uint &uiTotal, uint &uiResponses, uint &uiVisible) const  { return m_pHostClient->GetGameListStatus(bList, uiProcessed, uiTotal, uiResponses, uiVisible); }

    CClientSnapshot&    GetCurrentClientSnapshot()      { return m_CurrentClientSnapshot; }

    ResHandle           GetLoadingTexture() const       { return m_hLoadingTexture; }

    virtual bool        UsePlayerColors()               { return cg_unitPlayerColors; }
    virtual bool        UseHeroIndicators()             { return cg_heroIndicators; }

    int                 GetNumPlayersOnline() const     { return m_pHostClient->GetNumPlayersOnline(); }
    int                 GetNumPlayersInGame() const     { return m_pHostClient->GetNumPlayersInGame(); }

    void                Ping(byte yType, float fX, float fY, int iClientNumber);
    CVec3f&             GetLastEventPos()               { return m_v3LastEvent; }

    void                UpdateRecommendedItems();
    
    bool                DownloadReplay(const tstring &sUrl);
    float               GetReplayDownloadProgress();
    bool                ReplayDownloadErrorEncountered();
    bool                ReplayDownloadInProgress()      { return m_bDownloadingReplay; }
    void                StopReplayDownload();

    void                LoadStringTables();
    void                ProcessFirstSnapshot();

    bool                SendScriptMessage(const tstring &sName, const tstring &sValue);

    bool                CensorChat(tstring &sMessage);

    void                DelayHeroLoading(uint uiDuration)       { m_uiDelayHeroLoading = uiDuration; }

    CHostClient*        GetClient() const               { return m_pHostClient; }
};
//=============================================================================

#endif //__C_GAMECLIENT_H__
