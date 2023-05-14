// (C)2005 S2 Games
// c_hostclient.h
//
//=============================================================================
#ifndef __C_HOSTCLIENT_H__
#define __C_HOSTCLIENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_protocol.h"

#include "c_host.h"
#include "c_packet.h"
#include "c_snapshot.h"
#include "c_statestring.h"
#include "c_stateblock.h"
#include "c_clientlogin.h"
#include "c_socket.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHostClient;
class CWorld;
class CClientGameLib;
class CSnapshot;
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef _DEBUG
const tstring CLIENT_LIBRARY_NAME(_T("cgame_debug"));
#else
const tstring CLIENT_LIBRARY_NAME(_T("cgame"));
#endif

enum EClientState
{
    CLIENT_STATE_IDLE = 0,
    CLIENT_STATE_PRELOADING,
    CLIENT_STATE_CONNECTING,
    CLIENT_STATE_CONNECTED,
    CLIENT_STATE_LOADING,
    CLIENT_STATE_WAITING_FIRST_SNAPSHOT,
    CLIENT_STATE_IN_GAME,

    NUM_CLIENT_STATES
};

typedef map<uint, IBuffer*>     BufferMap;
typedef BufferMap::iterator     BufferMap_it;

struct SSnapshotFragmentCollector
{
    BufferMap   mapBuffers;
    byte        yCount;
    bool        bIsCompressed;
    uint        uiUncompressedLength;

    SSnapshotFragmentCollector() :
    yCount(0),
    bIsCompressed(false),
    uiUncompressedLength(0)
    {}
};

struct SNetworkResourceRecord
{
    uint            m_uiIndex;
    EResourceType   m_eType;
    tstring         m_sPath;
};

EXTERN_CVAR_STRING(net_name);

typedef deque<SNetworkResourceRecord>   NetworkResourceList;

typedef map<uint, SSnapshotFragmentCollector>   FragmentIndexMap;
typedef FragmentIndexMap::iterator              FragmentIndexMap_it;

const byte SERVER_OFFICIAL  (BIT(0));

enum EGameListType
{
    LIST_NONE = 0,
    LIST_SERVER,
    LIST_GAME,
    LIST_ONGOING_GAME
};
//=============================================================================

//=============================================================================
// CServerList
//=============================================================================
class CServerList
{
private:
    struct SServerListEntry
    {
        tstring         m_sAddress;
        ushort          m_unPing;
        tstring         m_sServerName;
        tstring         m_sServerNameLower;
        tstring         m_sGameName;
        tstring         m_sGameNameLower;
        tstring         m_sMapName;
        tstring         m_sDescription;
        tstring         m_sLocation;
        byte            m_yNumPlayers;
        byte            m_yMaxPlayers;
        byte            m_yTeamSize;
        tstring         m_sGameMode;
        uint            m_uiGameOptions;
        uint            m_uiRequestTime;
        EGameListType   m_eList;
        bool            m_bInvite;
        tstring         m_sInviter;
        ushort          m_unChallenge;
        EServerAccess   m_eAccess;
        byte            m_yFlags;
        byte            m_yHostFlags;
        int             m_iTier;
        bool            m_bMatchStarted;
        bool            m_bValid;
        tstring         m_sRegion;
        bool            m_bVisible;
        byte            m_yArrangedType;
        ushort          m_unMinPSR;
        ushort          m_unMaxPSR;

        SServerListEntry(
            const tstring &sAddress,
            ushort unPing = USHRT_MAX);
    };

    struct SServerInfoRequest
    {
        tstring         m_sAddress;
    };
    
    typedef map<tstring, SServerListEntry>  ServerMap;
    typedef pair<tstring, SServerListEntry> ServerMapPair;
    typedef ServerMap::iterator             ServerMap_it;

    CHostClient*    m_pClient;
    CHTTPManager*   m_pHTTPManager;
    
    CHTTPRequest*   m_pRequest;
    EGameListType   m_eRequestList;

    ServerMap       m_mapServers;
    ServerMap       m_mapLocalServers;
    CSocket         m_sockBrowser;

    tstring         m_sMatchKey;
    
    ushort          m_unLocalChallenge;
    uint            m_uiLocalRequestTime;
    EGameListType   m_eLocalList;

    tstring     m_sFilterName;
    tsmapts     m_mapFilterSettings;

    deque<SServerInfoRequest>   m_deqRequests;

    float       m_fRequestAccumulator;

    uint        m_uiReconnectExpireTime;

    uint        m_uiPlayerCount;

    int         m_iNumOnline;
    int         m_iNumInGame;

    bool    ProcessServerResponse(const tstring &sAddress, CPacket &pkt);
    bool    ProcessReconnectInfoResponse(const tstring &sAddress, CPacket &pkt);
    void    AddServerToList(SServerListEntry &cEntry, bool bLocal);
    void    RelistServers();
    bool    IsGameVisible(SServerListEntry &cEntry);
    void    ProcessServerList(const tstring &sResponse);

    CServerList();

public:
    ~CServerList();
    CServerList(CHTTPManager *pHTTPManager) :
    m_pHTTPManager(pHTTPManager),
    m_pRequest(NULL),
    m_sockBrowser(_T("Server list")),
    m_pClient(NULL),
    m_fRequestAccumulator(0.0f),
    m_uiReconnectExpireTime(INVALID_TIME),
    m_uiPlayerCount(0),
    m_iNumOnline(0),
    m_iNumInGame(0)
    {
        m_sockBrowser.Init(K2_SOCKET_GAME);
    }

    const tstring&  GetMatchKey()   { return m_sMatchKey; }
    
    void        RequestLocalServerList(EGameListType eList);
    void        RequestMasterServerList(const tstring &sCookie, const tstring &sQueryType, EGameListType eList, const tstring &sRegion);
    void        RequestServerInfo(const tstring &sAddress, bool bOfficial, EGameListType eList, bool bInvite, const tstring &sInviter);
    void        ServerInvite(const tstring &sInviterName, int iInviterAccountID, const tstring &sAddress);
    void        CheckReconnectStatus(const tstring &sAddress, uint uiMatchID, uint uiAccountID, ushort unConnectionID);
    void        CancelServerList();
    void        ClearServerList();
    uint        GetServerListCount() const;
    
    void        Frame();

    tstring     GetBestServer();
    tstring     GetBestLocalServer();
    tstring     GetBestServerName();
    tstring     GetBestLocalServerName();

    bool        AwaitingResponses() const;
    bool        AwaitingLocalResponses() const;

    void        UpdateFilter(const tstring &sName, const tstring &sSettings);
    void        SpamGameList(int iCount);

    void        SetClient(CHostClient *pClient)     { m_pClient = pClient; }

    K2_API bool GetStatus(bool &bList, uint &uiProcessed, uint &uiTotal, uint &uiResponses, uint &uiVisible) const;

    void        Clear();

    uint        GetPlayerCount() const              { return m_uiPlayerCount; }
    int         GetNumOnline() const                { return m_iNumOnline; }
    int         GetNumInGame() const                { return m_iNumInGame; }
};
//=============================================================================

//=============================================================================
// CHostClient
//=============================================================================
class CHostClient
{
private:
    uint                    m_uiIndex;
    CClientGameLib*         m_pGameLib;

    ResHandle               m_hClientMessages;

    EClientState            m_eState;
    uint                    m_uiFrameCount;
    uint                    m_uiGameTime;
    uint                    m_uiLastGameTime;
    uint                    m_uiClientFrameLength;

    ushort                  m_unConnectionID;
    int                     m_iClientNum;
    uint                    m_uiLastSendTime;
    uint                    m_uiLastSnapshotReceiveTime;
    uint                    m_uiServerTimeout;
    uint                    m_uiServerFPS;
    bool                    m_bReadyToSendSnapshot;

    CClientAccount          m_ClientAccount;
    CServerList             m_ServerList;

    uint                    m_uiServerFrame;
    uint                    m_uiServerTimeStamp;
    uint                    m_uiPrevServerFrame;
    uint                    m_uiPrevServerTimeStamp;
    uint                    m_uiLastAckedServerFrame;

    FragmentIndexMap        m_mapSnapshotFragments;
    map<uint, CSnapshot>    m_mapSnapshots;

    tstring                 m_sReconnectIP;
    uint                    m_uiReconnectMatchID;

    CPacket                 m_pktSend;
    CPacket                 m_pktReliableSend;

    CSocket                 m_sockGame;

    CWorld*                 m_pWorld;

    CBufferBit              m_cBufferSnapshotTemp;
    CSnapshot               m_cSnapshotTemp;

    // State string data
    byte                    m_yStateSequence;

    vector<CStateString>    m_vStateStrings;
    CBufferDynamic          m_bufferStateStringFragment;
    ushort                  m_unStateStringFragmentIndex;

    StateBlockVector        m_vStateBlocks;
    CBufferDynamic          m_bufferStateBlockFragment;
    ushort                  m_unStateBlockFragmentIndex;
    
    bool                    m_bStartedLoadingNetworkResources;
    uint                    m_uiNetworkResourceCount;
    NetworkResourceList     m_deqNetworkResources;

    bool                    m_bReceivedStateData;
    bool                    m_bReceivedAuth;
    bool                    m_bStartLoading;
    bool                    m_bSentKeepAlive;
    bool                    m_bGameHost;
    bool                    m_bSilentConnect;
    bool                    m_bPractice;
    tstring                 m_sLoadingInterface;

    bool                    m_bAutoStartGame;
    bool                    m_bAutoStartGameLocal;
    tstring                 m_sAutoStartName;
    tstring                 m_sAutoStartOptions;

    tstring                 m_sInviteAddress;
    tstring                 m_sInviteGameName;

    tstring                 m_sOldCookie;

    uint                    m_uiLastUpdateCheck;
    byte                    m_yAuthFlags;

    bool                    m_bWasIdle;
    bool                    m_bStartedLoadingWorld;

    uint                    m_auiStateStartTime[NUM_CLIENT_STATES];

    // State string functions
    void            ResetStateData();
    
    bool            ReadStateStringUpdate(CPacket &pkt, bool bCompressed);
    bool            ReadStateStringFragment(CPacket &pkt);
    bool            ReadStateStringTermination(CPacket &pkt, bool bCompressed);
    void            SetStateString(ushort unID, const IBuffer &buffer);

    void            StartLoadingNetworkResources();
    void            LoadNextNetworkResource();
    bool            FinishedLoadingNetworkResources() const                 { return m_deqNetworkResources.empty() && m_bStartedLoadingNetworkResources; }
    float           GetNetworkResourceLoadingProgress() const               { return (m_uiNetworkResourceCount - INT_SIZE(m_deqNetworkResources.size())) / float(m_uiNetworkResourceCount); }

    bool            ReadStateBlockUpdate(CPacket &pkt, bool bCompressed);
    bool            ReadStateBlockFragment(CPacket &pkt);
    bool            ReadStateBlockTermination(CPacket &pkt, bool bCompressed);
    void            SetStateBlock(ushort unID, const IBuffer &buffer);

    // Snapshot functions
    bool            ReadCompleteSnapshot(CBufferBit &cBuffer);
    bool            ReadSnapshot(CPacket &pkt, bool bCompressed);
    bool            ReadSnapshotFragment(CPacket &pkt);
    bool            ReadSnapshotTermination(CPacket &pkt, bool bCompressed);
    void            AssembleSnapshot(uint uiFrameNumber);
    void            CheckSnapshotFragments();
    void            DeleteSnapshotFragments(uint uiFrame);
    bool            ProcessCurrentSnapshot();

    // Other server command functions
    void            ProcessKickPacket(CPacket &pkt);
    bool            StartLoadingWorld(const tstring &sWorldName, bool bPreload);
    void            ProcessRemoteLoading(CPacket &pkt);

    // Client state logic
    bool            AllStateDataReceived();

    bool            ProcessPacket(CPacket &pkt);
    void            ReadPacketsFromServer();
    void            SendPackets();
    void            UpdateNetSettings();

    void            CheckTimeout();
    void            CheckAutoStartGame();

    float           LoadingStep();
    void            LoadingFrame();

    float           PreloadingStep();
    void            PreloadingFrame();

public:
    K2_API ~CHostClient();
    K2_API CHostClient(uint uiIndex, CHTTPManager *pHTTPManager);
    
    uint            GetIndex() const                        { return m_uiIndex; }

    EClientState    GetState() const                        { return m_eState; }
    CWorld*         GetWorld() const                        { return m_pWorld; }
    int             GetClientNum() const                    { return m_iClientNum; }
    uint            GetFrameCount() const                   { return m_uiFrameCount; }
    uint            GetTime() const                         { return m_uiGameTime; }
    uint            GetClientFrameLength() const            { return m_uiClientFrameLength; }
    uint            GetServerFrame() const                  { return m_uiServerFrame; }
    uint            GetPrevServerFrame() const              { return m_uiPrevServerFrame; }
    uint            GetLastAckedServerFrame() const         { return m_uiLastAckedServerFrame; }
    uint            GetServerTime() const                   { return m_uiServerTimeStamp; }
    uint            GetPrevServerTime() const               { return m_uiPrevServerTimeStamp; }
    float           GetLerpValue() const                    { return (m_uiServerTimeStamp > m_uiPrevServerTimeStamp) ? CLAMP(int(m_uiGameTime - m_uiPrevServerTimeStamp) / float(m_uiServerTimeStamp - m_uiPrevServerTimeStamp), 0.0f, 1.0f) : 1.0f; }
    byte            GetStateSequence() const                { return m_yStateSequence; }


    K2_API void     UpdateServerTimeout(uint uiLength = INVALID_TIME);
    void            SetClientNum(int iClientNum)            { m_iClientNum = iClientNum; }

    K2_API tstring  GetConnectedAddress();
    ushort          GetConnectionID() const                 { return m_unConnectionID; }
    void            RegenerateConnectionID()                { m_unConnectionID = K2System.GetRandomSeed32() & USHRT_MAX; m_sockGame.SetConnectionID(m_unConnectionID); }

    K2_API CStateString*    GetStateString(ushort unID);
    K2_API CStateBlock*     GetStateBlock(ushort unID);

    void            FileDropNotify(const tsvector &vsFiles);
    
    void                    ChangePassword(const tstring &sUser, const tstring &sOldPassword, const tstring &sNewPassword, const tstring &sConfirmPassword) { m_ClientAccount.ChangePassword(sUser, sOldPassword, sNewPassword, sConfirmPassword); }

#ifdef K2_GARENA
    void                    Login(const tstring &sToken)                            { m_ClientAccount.Connect(sToken); }
#else
    void                    Login(const tstring &sUser, const tstring &sPassword)   { m_ClientAccount.Connect(sUser, sPassword); }
#endif
    void                    CancelLogin()                                           { m_ClientAccount.Cancel(); }
    void                    Logout()                                                { m_ClientAccount.Logout(); }
    const CClientAccount&   GetAccount() const                                      { return m_ClientAccount; }
    bool                    IsLoggedIn() const                                      { return m_ClientAccount.IsLoggedIn(); }
    int                     GetAccountID() const                                    { return m_ClientAccount.GetAccountID(); }
    const tstring&          GetCookie() const                                       { return m_ClientAccount.GetCookie(); }

    void                    SetNickname(const tstring &sName)                       { m_ClientAccount.SetNickname(sName); }
    void                    SetTrialGamesCount(uint uiTrial)                        { m_ClientAccount.SetTrialGames(uiTrial); }

    K2_API void     Init();
    K2_API void     SetGamePointer();
    K2_API void     PreFrame();
    K2_API void     Frame();
    K2_API void     Connect(const tstring &sAddr, bool bSilent = false, bool bPractice = false, const tstring &sLoadingInterface = _T("loading"));
    void            UpdateCookie();
    K2_API void     Disconnect(const tstring &sReason = TSNULL);
    K2_API void     LoadAllResources();
    K2_API void     LoadWorldResources(const tstring &sWorldName);
    K2_API void     PreloadWorldResources(const tstring &sWorldName);
    
    K2_API void     AcknkowledgeStatsReceived();
    
    K2_API void     SendGameData(const IBuffer &buffer, bool bReliable);
    void            SendServerPrivateValue(byte yValue);
    K2_API void     SendRemoteCommand(const tstring &sCommand);
    K2_API void     SendClientSnapshot(const IBuffer &buffer);
    bool            IsReadyToSendSnapshot() const               { return m_bReadyToSendSnapshot; }

    void            CheckReconnect(const tstring &sAddress, uint uiMatchID);
    void            CheckReconnect();
    void            SetReconnect(tstring sIp, uint uiMatchID)   { m_sReconnectIP = sIp; m_uiReconnectMatchID = uiMatchID; };

    void            UpdateAvailable(const tstring &sVersion);
    K2_API void     SilentUpdate();

    K2_API void     GameError(const tstring &sError);

    void            RequestLocalServerList(EGameListType eList);
    void            RequestMasterServerList(const tstring &sQueryType, EGameListType eList, const tstring &sRegion = _T(""));
    void            CancelServerList();
    K2_API void     ClearServerList();
    uint            GetServerListCount() const;

    bool            IsGameHost() const                      { return m_bGameHost; }
    K2_API bool     IsLoading() const;

    bool            IsSilentConnect() const                 { return m_bSilentConnect; }

    void            StartAutoLocalGame(const tstring &sName, const tstring &sOptions);
    void            StartAutoRemoteGame(const tstring &sName, const tstring &sOptions);

    void            QueueNetworkResource(SNetworkResourceRecord &record)    { m_deqNetworkResources.push_back(record); }

    void            FilterGameList(const tstring &sName, const tstring &sSettings);
    void            SpamGameList(int iCount);

    tstring         GetGameModeName(uint uiMode) const;
    uint            GetGameModeFromString(const tstring &sMode) const;

    tstring         GetGameOptionName(uint uiOption) const;
    uint            GetGameOptionFromString(const tstring &sOption) const;
    tstring         GetGameOptionsString(const uint uiOption) const;

    void            InviteUser(const tstring &sName);
    bool            ServerInvite(const tstring &sInviterName, int iInviterAccountID, const tstring &sAddress);

    void            SetInviteAddress(const tstring &sAddress);
    const tstring&  GetInviteAddress() const                { return m_sInviteAddress; }

    void            SetInviteGameName(const tstring &sGameName)     { m_sInviteGameName = sGameName; }
    const tstring&  GetInviteGameName() const                       { return m_sInviteGameName; }

    bool            GetGameListStatus(bool &bList, uint &uiProcessed, uint &uiTotal, uint &uiResponses, uint &uiVisible) const      { return m_ServerList.GetStatus(bList, uiProcessed, uiTotal, uiResponses, uiVisible); }

    K2_API bool     IsValidTier(int iTier);
    K2_API bool     IsLeaver();
    K2_API bool     IsValidPSR(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR);
    K2_API bool     IsValidPSRForGameList(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR, const bool bFilter = false);

    tstring         Translate(const tstring &sKey, const tsmapts &mapTokens);

    tstring         GetNetName() const                      { return IsLoggedIn() ? m_ClientAccount.GetNickname() : net_name; }
    uint            GetPlayerCount() const                  { return m_ServerList.GetPlayerCount(); }
    int             GetNumPlayersOnline() const             { return m_ServerList.GetNumOnline(); }
    int             GetNumPlayersInGame() const             { return m_ServerList.GetNumInGame(); }

    void            SoftReconnect()                         { m_eState = CLIENT_STATE_WAITING_FIRST_SNAPSHOT; m_uiPrevServerFrame = 0; m_uiPrevServerTimeStamp = 0; m_uiLastAckedServerFrame = 0; }
    bool            GetPractice() const                     { return m_bPractice; }

    uint            GetStateStartTime(EClientState eState)  { return m_auiStateStartTime[eState]; }

    K2_API uint     GetOldestReliable() const;

    K2_API void     SelectUpgrade(const tstring &sProductCode);
    K2_API void     ClearUpgrade(const tstring &sType);
    uint            GetAnnouncerVoice() const               { return m_ClientAccount.GetAnnouncerVoice(); }
    K2_API bool     CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar);

    uint            GetCoins() const                                { return m_ClientAccount.GetCoins(); }
    const SAvatarInfo*  GetAvatarInfo(const tstring &sName) const   { return m_ClientAccount.GetAvatarInfo(sName); }

    void            RefreshUpgrades()                       { return m_ClientAccount.RefreshUpgrades(); }
    void            RefreshInfos()                          { return m_ClientAccount.RefreshInfos(); }

    K2_API void     PreloadWorld(const tstring &sWorldName);
};
//=============================================================================

#endif //__C_HOSTCLIENT_H__
