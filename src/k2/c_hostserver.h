// (C)2005 S2 Games
// c_hostserver.h
//
//=============================================================================
#ifndef __C_HOSTSERVER_H__
#define __C_HOSTSERVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
#include "c_socket.h"
#include "c_statestring.h"
#include "c_stateblock.h"
#include "c_clientconnection.h"
#include "c_snapshot.h"
#include "c_servergamelib.h"
#include "chatserver_protocol.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHostServer;

class CWorld;
class CSocket;
class CClientSnapshot;
class CDate;
class CHTTPManager;
class CServerChatConnection;

K2_API EXTERN_CVAR_STRING(svr_name);
EXTERN_CVAR_INT(svr_slave);
K2_API EXTERN_CVAR_INT(svr_port);
EXTERN_CVAR_STRING(svr_location);
K2_API EXTERN_CVAR_STRING(svr_ip);
EXTERN_CVAR_STRING(svr_version);
#if 1
EXTERN_CVAR_STRING(svr_chatAddress);
EXTERN_CVAR_INT(svr_chatPort);
#else
extern const tstring svr_chatAddress;
extern const int svr_chatPort;
#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MAX_INCOMING_PACKETS_PER_FRAME(512);

enum EVoiceTargets
{
    VOICE_TARGET_ALL,
    VOICE_TARGET_TEAM,
    VOICE_TARGET_SQUAD,

    NUM_VOICE_TARGETS
};

typedef map<int, CClientConnection*>    ClientMap;
typedef pair<int, CClientConnection*>   ClientPair;
typedef ClientMap::iterator ClientMap_it;

typedef pair<int, int>  iipair;
typedef vector<iipair>  TierVector;
//=============================================================================

//=============================================================================
// CHostServer
//=============================================================================
class CHostServer
{
private:
    struct SRosterEntty
    {
        uint    uiTeam;
        uint    uiSlot;
        uint    uiReminders;

        SRosterEntty() :
        uiTeam(-1),
        uiSlot(-1),
        uiReminders(0)
        {}

        SRosterEntty(uint _uiTeam, uint _uiSlot) :
        uiTeam(_uiTeam),
        uiSlot(_uiSlot),
        uiReminders(0)
        {}
    };

    typedef map<uint, SRosterEntty> Roster;
    typedef Roster::iterator        Roster_it;
    typedef Roster::const_iterator  Roster_cit;

    CHTTPManager*           m_pHTTPManager;
#ifndef K2_CLIENT
    CServerChatConnection*  m_pChatConnection;
#endif

    CServerGameLib          m_GameLib;
    CWorld*                 m_pWorld;

    string                  m_sMasterServerURL;

    uint                    m_uiFrameCount;
    uint                    m_uiLongServerFrameCount;
    uint                    m_uiFrameLength;
    uint                    m_uiFrameAccumulator;
    uint                    m_uiServerTime;
    uint                    m_uiPauseTime;
    uint                    m_uiDroppedFrames;

    vector<PoolHandle>      m_vSnapshots;
    uint                    m_uiSnapshotBufferPos;

    uint                    m_uiServerID;
    tstring                 m_sSessionCookie;
    uint                    m_uiNextHeartbeat;

    map<ushort, int>        m_mapClientNumbers;

    uint                    m_uiLastConnectRequestPeriod;
    map<wstring, uint>      m_mapConnectionRequests;

#ifdef K2_CLIENT
    CClientConnection*      m_pClient;
#else
    ClientMap                   m_mapClients;
    vector<CClientConnection*>  m_vProvisionalClients;
#endif

    CSocket                 m_sockGame;

    vector<CStateString>    m_vStateStrings;
    StateBlockVector        m_vStateBlocks;
    uivector                m_vStateBlockModCounts;

    bool                    m_bUpdateAvailable;
    bool                    m_bUpdating;
    bool                    m_bUpdateComplete;
    tstring                 m_sUpdateVersion;

    bool                    m_bPaused;
    byte                    m_yPrivateValue;
    map<int, tstring>       m_mapInvitations;
    map<tstring, tstring>   m_mapInvitations2;

#ifndef K2_CLIENT
    CHTTPRequest*           m_pHeartbeat;
#endif
    bool                    m_bInitializeMatchHeartbeat;
    bool                    m_bGameLoading;
    bool                    m_bMatchStarted;
    bool                    m_bPractice;
    uint                    m_uiMatchupID;
    bool                    m_bTournMatch;
    bool                    m_bLeagueMatch;
    bool                    m_bLocal;

    bool                    m_bHasManager;
    CSocket                 m_sockManager;
    uint                    m_uiServerLoad;
    uint                    m_uiLastServerLoad;
    CPacket                 m_pktManager;

    EServerAccess           m_eAccess;
    bool                    m_bForceInviteOnly;
    int                     m_iTier;    
    byte                    m_yHostFlags;

    TierVector              m_vTiers;
    
    ushort                  m_unMinPSR;
    ushort                  m_unMaxPSR;
    
    float                   m_fLeaverThreshold;

    uint                    m_uiLastUpdateCheck;

    map<int, uint>          m_mapKickedClients;

    Roster                  m_mapRoster;
    uint                    m_iTournMatchStartTime;

    // Basic server profiling stats
    uint                    m_uiBytesSent;
    uint                    m_uiPacketsSent;
    uint                    m_uiBytesDropped;
    uint                    m_uiPacketsDropped;
    uint                    m_uiBytesReceived;
    uint                    m_uiPacketsReceived;

    int                     m_iLastCState;

    CClientConnection*  GetClientConnection(const tstring &sAddress, ushort unConnectionID);

    void            ReadPackets();
    void            UpdateClients();
    bool            ValidateKey(const string &sKey);
    bool            AddClient(const tstring &sAddress, ushort unPort, CPacket &pkt);
    void            RejectConnection(const tstring &sAddress, ushort unPort, const tstring &sReason);
    void            SendShutdown();
    void            SendInfo(CPacket &pkt, bool extended);
#ifndef K2_CLIENT
    void            SendHeartbeat();
#endif
    void            SendNotification();
    void            ResetStateData();

#ifdef K2_CLIENT
    void            RemoveClient(int iClientNum, const tstring &sReason = _T("Unknown"));
#else
    ClientMap_it    RemoveClient(ClientMap_it itClient, const tstring &sReason = _T("Unknown"));
    void            RemoveClient(int iClientNum, const tstring &sReason = _T("Unknown"));
#endif

    void            SendGameStatus(uint uiServerLoad);
    void            ProcessManagerPacket(CPacket &pkt);

public:
    ~CHostServer();
    CHostServer(CHTTPManager *pHTTPManager);

    bool                        Init(bool bPractice, bool bLocal);
    void                        SetGamePointer() const                  { m_GameLib.SetGamePointer(); }
    K2_API bool                 StartGame(const tstring &sName, const tstring &sGameSettings);
    K2_API bool                 StartReplay(const tstring &sFilename);
    K2_API void                 StopReplay();

    const string&               GetMasterServerURL() const              { return m_sMasterServerURL; }

    byte                        GetServerStatus() const;
    byte                        GetOfficial() const;
    
    const uint                  GetLastServerLoad()                     { return m_uiLastServerLoad; }

    EServerAccess               GetServerAccess() const                 { return m_bForceInviteOnly ? ACCESS_INVITEONLY : m_eAccess; }
    void                        SetServerAccess(EServerAccess eAccess)  { if (m_eAccess != eAccess) { m_eAccess = eAccess; m_uiNextHeartbeat = 0; } }
    bool                        GetForceInviteOnly() const              { return m_bForceInviteOnly; }
    void                        SetForceInviteOnly(bool bSet)           { m_bForceInviteOnly = bSet; }

    inline byte                 GetHostFlags() const                    { return m_yHostFlags; }
    inline uint                 GetServerID() const                     { return m_uiServerID; }
    const wstring&              GetLocation() const                     { return svr_location.GetValue(); }
    const wstring&              GetName() const                         { return svr_name.GetValue(); }
    const wstring&              GetAddress() const                      { return svr_ip.GetValue(); }
    const ushort                GetPort() const                         { return svr_port; }
    const wstring&              GetVersion() const                      { return svr_version.GetValue(); }

    int                         GetTier() const                         { return m_iTier; }
    void                        SetTier(int iTier)                      { m_iTier = iTier; }
    
    ushort                      GetMinPSR() const                       { return m_unMinPSR; }
    void                        SetMinPSR(ushort unMinPSR)              { if (unMinPSR < 0) m_unMinPSR = 0; else m_unMinPSR = unMinPSR; }
    ushort                      GetMaxPSR() const                       { return m_unMaxPSR; }
    void                        SetMaxPSR(ushort unMaxPSR)              { if (unMaxPSR < 0) m_unMaxPSR = 0; else m_unMaxPSR = unMaxPSR; }   
    
    bool                        GetNoLeaver() const                     { return (m_yHostFlags & HOST_SERVER_NO_LEAVER) != 0; }
    void                        SetNoLeaver(bool bNoLeaver)             { if (bNoLeaver) m_yHostFlags |= HOST_SERVER_NO_LEAVER; else m_yHostFlags &= ~HOST_SERVER_NO_LEAVER; }

    bool                        GetNoStats() const                      { return (m_yHostFlags & HOST_SERVER_NO_STATS) != 0; }
    void                        SetNoStats(bool bNoStats)               { if (bNoStats) m_yHostFlags |= HOST_SERVER_NO_STATS; else m_yHostFlags &= ~HOST_SERVER_NO_STATS; }

    bool                        GetPractice() const                     { return m_bPractice; }
    bool                        GetLocal() const                        { return m_bLocal; }

    bool                        IsArrangedMatch() const                 { return m_uiMatchupID != INVALID_INDEX; }
    void                        SetMatchupID(uint uiMatchupID)          { m_uiMatchupID = uiMatchupID; }
    uint                        GetMatchupID() const                    { return m_uiMatchupID; }

    K2_API bool                 IsTournMatch() const                    { return m_bTournMatch; }
    K2_API void                 SetTournMatch(bool bTourn)              { m_bTournMatch = bTourn; }
    K2_API uint                 GetTournMatchStartTime()                { return m_iTournMatchStartTime; }
    
    bool                        IsLeagueMatch() const                   { return m_bLeagueMatch; }
    void                        SetLeagueMatch(bool bLeague)            { m_bLeagueMatch = bLeague; }
    
    inline void                 ClearRoster()                                           { m_mapRoster.clear(); }
    inline void                 AddToRoster(uint uiAccountID, uint uiTeam, uint uiSlot) { m_mapRoster[uiAccountID] = SRosterEntty(uiTeam, uiSlot); }
    inline bool                 IsOnRoster(uint uiAccountID) const                      { return m_mapRoster.find(uiAccountID) != m_mapRoster.end(); }
    inline uint                 GetTeamFromRoster(uint uiAccountID, uint uiFail) const  { Roster_cit it(m_mapRoster.find(uiAccountID)); return (it == m_mapRoster.end()) ? uiFail : it->second.uiTeam; }
    inline uint                 GetSlotFromRoster(uint uiAccountID) const               { Roster_cit it(m_mapRoster.find(uiAccountID)); return (it == m_mapRoster.end()) ? -1 : it->second.uiSlot; }
    K2_API void                 SendConnectReminders();
    K2_API void                 SubstituteRoster(uint uiOldAccountID, uint uiNewAccountID);

    bool                        IsUpdating() const                      { return m_bUpdating; }

    const tstring&              GetSessionCookie() const                { return m_sSessionCookie; }
    bool                        HasManager() const                      { return m_bHasManager; }

    K2_API void                 GenerateInvitation(int iAccountID);
    K2_API void                 GenerateInvitation(const tstring &sUser);
    bool                        ValidateInvitation(int iAccountID);
    bool                        ValidateInvitation(const tstring &sUser);

#ifndef K2_CLIENT
    void                        GetAccountAuth();
#endif

    void                        Frame(uint uiHostFrameLength, bool bClientReady);
    void                        GetSnapshot(CSnapshot &snapshot)    { m_GameLib.GetSnapshot(snapshot); }
    const vector<PoolHandle>&   GetSnapshotBuffer() const           { return m_vSnapshots; }
    uint                        GetSnapshotBufferPos() const        { return m_uiSnapshotBufferPos; }
    PoolHandle                  GetCurrentSnapshot() const          { return m_vSnapshots[m_uiSnapshotBufferPos]; }

    K2_API void                 SendGameData(int iClient, const IBuffer &buffer, bool bReliable);
    K2_API void                 BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient = -1);

    ushort                      AddStateString();
    K2_API CStateString&        GetStateString(ushort unID);
    const vector<CStateString>& GetStateStringVector() const        { return m_vStateStrings; }
    K2_API void                 SetStateString(ushort unID, const string &sStr);
    void                        SendStateString(int iClientNum, ushort unID);
    
    K2_API void                 AddStateBlock(ushort unID);
    K2_API CStateBlock&         GetStateBlock(ushort unID);
    const StateBlockVector&     GetStateBlockVector() const         { return m_vStateBlocks; }
    K2_API void                 SetStateBlock(ushort unID, const IBuffer &buffer);
    K2_API void                 SendStateBlock(int iClientNum, ushort unID);

    K2_API void                 UpdateStateStrings();
    
    CWorld*                     GetWorld() const                    { return m_pWorld; }
    CServerGameLib&             GetGameLib()                        { return m_GameLib; }
    K2_API CClientConnection*   GetClient(int iClientNum);
    K2_API bool                 HasClient(int iClientNum);
    K2_API int                  GetClientNumber(int iAccountID);
    K2_API int                  GetClientNumber(const tstring &sName);

    K2_API int                  GetNumActiveClients();

#ifdef K2_CLIENT
    uint                        GetNumConnectedClients() const      { return m_pClient != NULL ? 1 : 0; }
#else
    uint                        GetNumConnectedClients() const      { return INT_SIZE(m_mapClients.size()); }
#endif

    uint                        GetTime() const                     { return m_uiServerTime; }
    uint                        GetRealTime() const                 { return m_uiServerTime + m_uiFrameAccumulator; }
    uint                        GetFrameNumber() const              { return m_uiFrameCount; }
    uint                        GetLongServerFrameCount() const     { return m_uiLongServerFrameCount; }
    uint                        GetFrameLength() const              { return m_uiFrameLength; }
    uint                        GetPauseTime() const                { return m_uiPauseTime; }

    // Client management
    void                        ListClients();
    void                        ListProvisionalClients();
    K2_API void                 KickClient(int iClientNum, const tstring &sReason);
    void                        BanClient(int iClientNum, int iTime, const tstring &sReason);
    K2_API iset                 GetAccountIDs();
    int                         GenerateClientID(ushort unConnectionID);
    K2_API void                 ReleaseClientID(int iClientNumber);
    void                        ReauthorizeClient(CClientConnection *pClient);

    tstring                     GetServerAddress()                  { return m_sockGame.GetLocalAddr(); }
    word                        GetServerPort()                     { return m_sockGame.GetLocalPort(); }

#ifdef K2_CLIENT
    CClientConnection*          GetClient()                         { return m_pClient; }
#else
    ClientMap&                  GetClientMap()                      { return m_mapClients; }
#endif

    void                        UpdateAvailable(const tstring &sVersion);
    void                        UpdateComplete();
    void                        PerformUpdate();
    K2_API void                 SilentUpdate();

    K2_API void                 StartGame(uint uiMatchID);
    K2_API void                 StartMatch();
    K2_API void                 RestartMatch();
    K2_API void                 EndGame(const tstring &sReplayHost, const tstring &sReplayDir, const tstring &sReplayFilename, const tstring &sLogFilename, bool bFailed, const tstring &sReason = _T("disconnect_game_over"), bool bAborted = false, EMatchAbortedReason eAbortedReason = MATCH_ABORTED_UNKNOWN);

    K2_API uint                 GetMaxPlayers();

    void                        SetSendBuffer(uint uiSendBuffer)    { m_sockGame.SetSendBuffer(uiSendBuffer); }
    void                        SetRecvBuffer(uint uiRecvBuffer)    { m_sockGame.SetRecvBuffer(uiRecvBuffer); }

    uint                        GetDroppedFrames() const            { return m_uiDroppedFrames; }
    void                        ResetDroppedFrames()                { m_uiDroppedFrames = 0; }
    void*                       GetEntity(uint uiIndex) const       { return m_GameLib.GetEntity(uiIndex); }

    void                        AddPseudoClient();

    K2_API void                 SetPaused(bool bPaused)             { m_bPaused = bPaused; }
    K2_API bool                 GetPaused() const                   { return m_bPaused; }

    void                        InviteUser(const tstring &sName);

    uint                        GetClientKickCount(int iAccountID);

    bool                        IsValidTier(int iLevel) const;
    bool                        IsLeaver(float fLeavePercentage, int iNumGames) const;
    bool                        IsValidPSR(const int iRank) const;

    K2_API static float         GetLeaverThreshold(int iNumGames);

#ifndef K2_CLIENT
    bool                        RequestSessionCookie(bool bNew);
    void                        BreakSessionCookie();
    void                        ProcessAuthData(int iAccountID, const CPHPData *pData)  { m_GameLib.ProcessAuthData(iAccountID, pData); }
    void                        ProcessAuxData(int iClientNum, const CPHPData *pData)   { m_GameLib.ProcessAuxData(iClientNum, pData); }
#endif

    void                        KillServer();
};
//=============================================================================

#endif //__C_HOSTSERVER_H__