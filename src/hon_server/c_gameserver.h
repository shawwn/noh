// (C)2005 S2 Games
// c_gameserver.h
//
//=============================================================================
#ifndef __C_GAMESERVER_H__
#define __C_GAMESERVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_triggermanager.h"
#include "c_serverentitydirectory.h"
#include "c_gamelog.h"

#include "../hon_shared/i_game.h"
#include "../hon_shared/c_teaminfo.h"
#include "../hon_shared/c_visibilitymap.h"
#include "../hon_shared/c_playeraccountstats.h"

#include "../k2/c_rasterbuffer.h"
#include "../k2/c_date.h"

#include "chatserver_protocol.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameServer;
class IGameEntity;
class CServerEntityDirectory;
class CHostServer;
class CWorld;
class CPacket;
class CSnapshot;
class CStatsTracker;
class CGameInfo;
class CClientConnection;
class CHTTPRequest;
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GameServer (*CGameServer::GetInstance())

#define SERVER_CMD(name) \
    static bool svcmd##name##Fn(const tsvector &vArgList);\
CMD(name)\
{\
    bool bReturn(false);\
\
    IGame *pGame(Game.GetCurrentGamePointer());\
    Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
    bReturn = svcmd##name##Fn(vArgList);\
\
    Game.SetCurrentGamePointer(pGame);\
\
    return bReturn;\
}\
bool    svcmd##name##Fn(const tsvector &vArgList)


#define SERVER_FCN(name) \
    static tstring  svfn##name##Fn(const tsvector &vArgList);\
FUNCTION(name)\
{\
    tstring sReturn(_T(""));\
\
    IGame *pGame(Game.GetCurrentGamePointer());\
    Game.SetCurrentGamePointer(CGameServer::GetInstance());\
\
    sReturn = svfn##name##Fn(vArgList);\
\
    Game.SetCurrentGamePointer(pGame);\
\
    return sReturn;\
}\
tstring svfn##name##Fn(const tsvector &vArgList)

enum EHeroKillCode
{
    HKK_UNKNOWN = -1,
    HKK_HERO,
    HKK_TEAM = 142,
    HKK_NEUTRAL,
    HKK_TOWER,
    HKK_KONGOR
};

struct SMatchKillEntry
{
    EHeroKillCode   m_eKillCode;
    int             m_iKillerClientNumber;
    int             m_iVictimClientNumber;
    uint            m_uiTimeStamp;
    ivector         m_vAssists;
};

struct SGuestEntry
{
    uint    m_uiAccountID;
    uint    m_uiTeam;
};

typedef vector<SGuestEntry>     GuestVector;
typedef GuestVector::iterator   GuestVector_it;

GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterDecrementInterval);
GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterFloodThreshold);
//=============================================================================

//=============================================================================
// CGameServer
//=============================================================================
class CGameServer : public IGame
{
    SINGLETON_DEF(CGameServer)

private:
    CServerEntityDirectory* m_pServerEntityDirectory;
    CHostServer*            m_pHostServer;

    uint                    m_uiLastStatusNotifyTime;
    uint                    m_uiLastChatCounterDecrement;

    uint                    m_uiMatchCreationTime;

    HeroList                m_vHeroLists[NUM_HERO_LISTS];
    uint                    m_uiDraftRound;
    uint                    m_uiFinalDraftRound;
    uint                    m_uiBanningTeam;
    int                     m_iBanRound;
    float                   m_fAverageRating;
    float                   m_fTeamAverageRating[2];

    map<int, CPlayerAccountStats>               m_mapPlayerAccountStats;
    map<int, CPlayerAccountStats>::iterator     m_itPlayerAccountStats;

    uint                    m_uiCreepWaveCount;
    uint                    m_uiNextCreepWaveTime;
    uint                    m_uiLastCreepUpgradeLevel;

    uint                    m_uiNextTreeSpawnTime;
    uint                    m_uiNextPowerupSpawnTime;
    uint                    m_uiNextCritterSpawnTime;

    CVisibilityMap          m_cVisibilityMap[2];
    uint                    m_uiVisibilitySize;
    float                   m_fVisibilityScale;
    CRasterBuffer           m_cVisRaster;
    CRasterBuffer           m_cOccRaster;

    uivector                m_vNeutralCamps;
    uivector                m_vKongors;

    bool                    m_bStartAnnounced;
    bool                    m_bHasForcedRandom;
    int                     m_iActiveTeam;
    bool                    m_bSentTrialGame;

    CGameLog                m_GameLog;
    tstring                 m_sGameLogPath;

    tstring                 m_sName;
    int                     m_iCreatorClientNum;

    uint                    m_uiPowerupUID;
    uint                    m_uiLastVisibilityUpdate;
    
    uint                    m_uiLongServerFrame;

    CDate                   m_cMatchDate;

    vector<SMatchKillEntry> m_vMatchKillLog;

    bool                    m_bSolo;
    bool                    m_bEmpty;
    bool                    m_bLocal;

    bool                    m_bAllowedUnpause;
    uint                    m_uiPauseToggleTime;
    uint                    m_uiPauseStartTime;

    GuestVector             m_vGuests;

    tstring                 m_sReplayHost;
    tstring                 m_sReplayDir;

#ifndef K2_CLIENT
    CHTTPRequest*           m_pStatSubmitRequest;
    CHTTPRequest*           m_pAccountAuthRequest;
    uint                    m_uiLastAccountAuthRequest;
    uint                    m_uiSentAccountAuthRequest;
    bool                    m_bSentAccountAuthRequest;
#endif

    struct SDelayedMessage
    {
        CBufferDynamic  cBuffer;
        bool            bReliable;
        uint            uiSendTime;
        uint            uiTargetClient;
    };

    vector<SDelayedMessage> m_vDelayedMessages;

    // Message handlers
    bool        ProcessUnitRequest();
    bool        ProcessTeamRequest();

    void        PrecacheEntities(bool bHeroes);
    void        PrecacheEntity(const tstring &sName);

    void        AnalyzeTerrain()                { GetWorldPointer()->AnalyzeTerrain(); }
    void        UpdateNavigation();
    void        UpdateVisibility();

    void        BuildHeroLists();

    void        VoteFrame();
    void        UpdatePauseStatus();
    void        ReplayFrame();
    bool        SetupFrame();
    bool        ActiveFrame();
    bool        EndedFrame();

    bool        IsKickVoteValid(CPlayer *pTarget);

    bool        IsVisible(IUnitEntity* pFrom, float fX, float fY);

    CPlayer*    TryAddClient(CClientConnection *pClientConnection);

    void        StartPause(CPlayer *pPlayer);
    void        StartUnpause(CPlayer *pPlayer);

    uint        GetNumReferees();

    // send a message about several players.
    void        SendGeneralMessage(const tstring &sMsg, int iPlayerID)      { SendGeneralMessage(sMsg, 1, &iPlayerID); }
    void        SendGeneralMessage(const tstring &sMsg, int iNumPlayers, int *iPlayerList);

#ifndef K2_CLIENT
    void        UpdateSubmitStatsRequest();
#endif

public:
    ~CGameServer();

    bool            IsServer() const                        { return true; }
    CHostServer*    GetHostServer() const                   { return m_pHostServer; }

    ResHandle       RegisterGameMechanics(ResHandle hMechanics)     { g_NetworkResourceManager.GetNetIndex(hMechanics); return IGame::RegisterGameMechanics(hMechanics); }
    ResHandle       RegisterGameMechanics(const tstring &sPath)     { return RegisterGameMechanics(IGame::RegisterGameMechanics(sPath)); }

    IGameEntity*    AllocateEntity(const tstring &sName, uint uiMinIndex = INVALID_INDEX)           { return m_pServerEntityDirectory->Allocate(sName, uiMinIndex); }
    IGameEntity*    AllocateEntity(ushort unType, uint uiMinIndex = INVALID_INDEX)                  { return m_pServerEntityDirectory->Allocate(unType, uiMinIndex); }
    IGameEntity*    AllocateDynamicEntity(const tstring &sName, uint uiMinIndex, uint uiBaseType)   { return m_pServerEntityDirectory->AllocateDynamicEntity(sName, uiMinIndex, uiBaseType); }
    IGameEntity*    AllocateDynamicEntity(ushort unTypeID, uint uiMinIndex, uint uiBaseType)        { return m_pServerEntityDirectory->AllocateDynamicEntity(unTypeID, uiMinIndex, uiBaseType); }
    void            DeleteEntity(IGameEntity *pEntity);
    void            DeleteEntity(uint uiIndex);

    // API functions
    void            SetGamePointer()                        { SetCurrentGamePointer(this); }
    bool            Initialize(CHostServer *pHostServer);
    void            Frame();
    bool            LoadWorld(const tstring &sName, const tstring &sGameMode);
    bool            StartReplay(const tstring &sFilename);
    void            StopReplay();
    bool            AddClient(CClientConnection *pClientConnection);
    void            RemoveClient(int iClientNum, const tstring &sReason = _T("Disconnected"));
    void            ClientTimingOut(int iClientNum);
    uint            GetMaxClients() const;
    bool            ProcessClientSnapshot(int iClientNum, CClientSnapshot &snapshot);
    bool            ProcessGameData(int iClientNum, CPacket &pkt);
    void            GetSnapshot(CSnapshot &snapshot);
    void            ReauthClient(CClientConnection *pClientConnection);
    void            Shutdown();
    void            UnloadWorld();
    const tstring&  GetGameName() const                     { return m_sName; }
    void            GetServerInfo(CPacket &pkt);
    void            GetReconnectInfo(CPacket &pkt, uint uiMatchID, uint uiAccountID, ushort unConnectionID);
    bool            IsPlayerReconnecting(int iAccountID);
    bool            IsDuplicateAccountInGame(int iAccountID);
    bool            RemoveDuplicateAccountsInGame(int iAccountID);
    void            GetGameStatus(CPacket &pkt);
#ifndef K2_CLIENT
    void            GetHeartbeatInfo(CHTTPRequest *pHeartbeat);
    void            ProcessAuthData(int iAccountID, const CPHPData *pData);
    void            GetAccountAuth();
    void            ProcessAuxData(int iAccountID, const CPHPData *pData);
    void            SendReconnectData(CPlayer *pClient);
#endif
    void            ClientStateChange(int iClientNum, EClientConnectionState eState);
        
    int             GetClientNumFromAccountID(int iAccountID);
    int             GetClientNumFromName(const tstring &sName);

    // Client requests
    void            RequestChangeTeam(int iClientID, uint uiTeamID, uint uiSlot = INVALID_TEAM_SLOT, bool bBecomingReferee = false);
    void            ChangeTeam(int iClientID, uint uiTeamID, uint uiSlot = INVALID_TEAM_SLOT);
    void            RequestAssignFirstBanTeam(int iClientID, uint uiTeamID);
    void            AssignFirstBanTeam(uint uiTeamID);
    bool            PurchaseItem(int iClientNum, uint uiUnitIndex, ushort unShop, int iSlot);
    void            SellItem(int iClientNum, uint uiUnitIndex, int iSlot);
    void            MoveItem(int iClientNum, uint uiUnitIndex, int iSlot0, int iSlot1);
    bool            BuyBackHero(int iClientNum, uint uiUnitIndex);
    void            SetSelection(int iClientNum, const uiset &setSelection);
    void            ProcessCallVoteRequest(CPlayer *pPlayer, CPacket &pkt);
    
    bool            AllPlayersReady() const;
    bool            AllPlayersFullyConnected() const;
    bool            AllPlayersConnected() const;
    uint            GetClientCount(int iTeam = -1);
    int             GetConnectedClientCount(int iTeam = -1);

    bool            StartReplayRecording();

    void            RequestStartGame(int iClientNum);
    void            StartGame(bool bAllowSolo, bool bAllowEmpty);
    void            StartPreMatch();
    void            StartMatch();
    void            EndMatch(int iLosingTeam);
    void            Remake();
    void            Concede(int iLosingTeam);
    void            Reset(bool bFailed = false, const tstring &sReason = _T("disconnect_game_over"), bool bAborted = false, EMatchAbortedReason eAbortedReason = MATCH_ABORTED_UNKNOWN);
    void            EndFrame(PoolHandle hSnapshot);

    void            BalanceTeams(bool bAutoBalancedMode = false);
    void            ForceSwapPlayerSlots(int iTeamA, uint uiSlotA, int iTeamB, uint uiSlotB);
    void            SwapPlayerSlots(int iTeamA, uint uiSlotA, int iTeamB, uint uiSlotB);
    void            LockSlot(int iTeam, uint uiSlot);
    void            UnlockSlot(int iTeam, uint uiSlot);
    void            ToggleSlotLock(int iTeam, uint uiSlot);

    void            ResetPlayerVotes();

    void            UpdateVoIP();

    void            ParseGameLog(const tstring &sPath);
    void            GetMatchIDFromMasterServer();
    void            SendStats();
    void            SendAdditionalStats();

    // MikeG Trial Account game count increased
    void            SendTrialGameIncrease();

    void            BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient = -1, uint uiDelay = 0);
    void            BroadcastGameDataToTeam(int iTeam, const IBuffer &buffer, bool bReliable, int iExcludeClient = -1);

    void            SendPersistantDataToClient(int iClientNum);

    void            SendMessage(const tstring &sMsg, int iClientNum);
    void            SendGameData(int iClient, const IBuffer &buffer, bool bReliable);
    void            SendGameData(int iClient, const IBuffer &buffer, bool bReliable, uint uiDelay);
    void            SendReliablePacket(int iClient, const IBuffer &buffer);

    void            StateStringChanged(uint uiID, const CStateString &ss);
    void            StateBlockChanged(uint uiID, const IBuffer &buffer);

    CStateString&   GetStateString(uint uiID);
    CStateBlock&    GetStateBlock(uint uiID);

    uint            GetServerFrame();
    uint            GetServerTime() const;
    uint            GetPrevServerTime();
    uint            GetServerFrameLength();

    CTeamInfo*      AddTeam(const tstring &sName, const CVec4f &v4Color, uint uiTeamID);
    
    // Resources
    ResHandle   RegisterModel(const tstring &sPath);
    ResHandle   RegisterEffect(const tstring &sPath);
    ResHandle   RegisterIcon(const tstring &sPath);

    void    Precache(const tstring &sName, EPrecacheScheme eScheme, const tstring &sModifier);
    void    Precache(ushort unType, EPrecacheScheme eScheme, const tstring &sModifier);

    void    SendPopup(const CPopup *pPopup, IUnitEntity *pSource, IUnitEntity *pTarget, ushort unValue);
    void    SendPopup(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget = nullptr, ushort unValue = 0);
    void    SendPopup(EPopup eType, IUnitEntity *pSource, IUnitEntity *pTarget = nullptr, ushort unValue = 0);

    void    SendPing(const CPing *pPing, IUnitEntity *pSource, IUnitEntity *pTarget, byte yX, byte yY);
    void    SendPing(const CPing *pPing, IUnitEntity *pSource, IUnitEntity *pTarget, const CVec2f &v2Pos);
    void    SendPing(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget = nullptr, const CVec2f &v2Pos = V2_ZERO);
    void    SendPing(EPing eType, IUnitEntity *pSource, IUnitEntity *pTarget = nullptr, const CVec2f &v2Pos = V2_ZERO);

    void    SendUnitPing(const CPing *pPing, IUnitEntity *pSource, IUnitEntity *pTarget);
    void    SendUnitPing(byte yType, IUnitEntity *pSource, IUnitEntity *pTarget);
    void    SendUnitPing(EPing eType, IUnitEntity *pSource, IUnitEntity *pTarget);

    tstring         GetServerStatus();

    void            UpdateHeroList();
    void            GetAvailableHeroList(CPlayer *pPlayer, vector<ushort> &vHeroes);
    bool            SelectHero(int iClientNumber, ushort unHero, bool bPotentialHero, bool bAllowAvatar);
    bool            SelectAvatar(const int iClientNumber, const tstring &sAltAvatar);
    void            SelectRandomHero(int iClientNumber);
    void            SelectPotentialHeroOrRandomHero(int iClientNumber);
    void            ClearPotentialHero(CPlayer *pPlayer, ushort unHero);
    void            SetHeroStatus(ushort unHeroTypeID, byte yStatus);
    byte            GetHeroStatus(ushort unHeroTypeID);
    void            GetBannedHeroes(vector<ushort>& vHeroes);
    tstring         GetBannedHeroesStr();
    bool            RemoveHero(int iClientNumber);
    void            ResetPicks(int iClientNumber);
    ushort          GetRandomHeroFromPool();

    bool            HasMegaCreeps(uint uiTeam);
    void            SpawnCreeps();

    void            KillTrees();
    void            SpawnTrees();

    void            SpawnPowerup();

    void            SpawnCritters();

    void            RegisterShopInfo();

    PoolHandle  FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal, float fGoalRange, vector<PoolHandle> *pBlockers = nullptr) const   { return GetWorldPointer()->FindPath(v2Src, fEntityWidth, uiNavigationFlags, v2Goal, fGoalRange, pBlockers); }
    CPath*      AccessPath(PoolHandle hPath) const                                              { return GetWorldPointer()->AccessPath(hPath); }
    PoolHandle  ClonePath(PoolHandle hPath) const                                               { return GetWorldPointer()->ClonePath(hPath); }
    void        FreePath(PoolHandle hPath) const                                                { return GetWorldPointer()->FreePath(hPath); }
    PoolHandle  BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)  { return GetWorldPointer()->BlockPath(uiFlags, v2Position, fWidth, fHeight); }
    void        BlockPath(vector<PoolHandle> &vecBlockers, uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight)     { GetWorldPointer()->BlockPath(vecBlockers, uiFlags, cSurf, fStepHeight); }
    void        ClearPath(PoolHandle hBlockerID)                                                { GetWorldPointer()->ClearPath(hBlockerID); }

    ushort      GetVision(float fX, float fY) const;

    void        UpdateUnitVisibility(IUnitEntity *pUnit);

    void    UnitRespawned(uint uiIndex);
    void    UnitKilled(uint uiIndex);

    void    LogPlayer(EGameLogEvent eEvent, CPlayer *pPlayer)                                                                   { m_GameLog.WritePlayer(eEvent, pPlayer); }
    void    LogKill(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor = nullptr, ivector *pAssists = nullptr)     { m_GameLog.WriteKill(pTarget, pAttacker, pInflictor, pAssists); }
    void    LogAssist(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, CPlayer *pPlayer)                  { m_GameLog.WriteAssist(pTarget, pAttacker, pInflictor, pPlayer); }
    void    LogDamage(IUnitEntity *pTarget, int iPlayer, ushort unAttackerType, ushort unInflictorType, float fDamage)          { m_GameLog.WriteDamage(pTarget, iPlayer, unAttackerType, unInflictorType, fDamage); }
    void    LogDeny(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, float fExperience, ushort unGold)    { m_GameLog.WriteDeny(pTarget, pAttacker, pInflictor, fExperience, unGold); }
    void    LogExperience(EGameLogEvent eEvent, IUnitEntity *pTarget, IUnitEntity *pSource, float fExperience)                  { m_GameLog.WriteExperience(eEvent, pTarget, pSource, fExperience); }
    void    LogGold(EGameLogEvent eEvent, CPlayer *pPlayer, IUnitEntity *pSource, ushort unGold)                                { m_GameLog.WriteGold(eEvent, pPlayer, pSource, unGold); }
    void    LogHero(EGameLogEvent eEvent, IHeroEntity *pHero, const tstring &sParamA = TSNULL)                                  { m_GameLog.WriteHero(eEvent, pHero, sParamA); }
    void    LogAward(EGameLogEvent eEvent, IUnitEntity *pAttacker, IUnitEntity *pTarget, ushort unGold = 0)                     { m_GameLog.WriteAward(eEvent, pAttacker, pTarget, unGold); }
    void    LogItem(EGameLogEvent eEvent, IEntityItem *pItem, IUnitEntity *pTarget = nullptr)                                      { m_GameLog.WriteItem(eEvent, pItem, pTarget); }
    void    LogAbility(EGameLogEvent eEvent, IEntityAbility *pAbility, IUnitEntity *pTarget = nullptr)                             { m_GameLog.WriteAbility(eEvent, pAbility, pTarget); }

    void            LongServerFrame(uint uiLength);

    CPlayer*        AddFakePlayer(int iTeam);

    void            UpdateUpgrades(int iClientNum);

    EStatsStatus    GetStatsStatus() const
    {
        CGameInfo *pGameInfo(GetGameInfo());
        return pGameInfo != nullptr ? pGameInfo->GetStatsStatus() : STATS_NULL;
    }

    void    SelectBaseAvatar(int iClientNumber);
};
//=============================================================================

#endif //__C_GAMESERVER_H__

