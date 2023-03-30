// (C)2005 S2 Games
// c_clientconnection.h
//
//=============================================================================
#ifndef __C_CLIENTCONNECTION_H__
#define __C_CLIENTCONNECTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_protocol.h"
#include "c_socket.h"
#include "c_snapshot.h"
#include "c_statestring.h"
#include "c_stateblock.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IBuffer;
class CHostServer;
class CHTTPManager;
class CHTTPRequest;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MIN_MTU_SIZE(512);
const uint MAX_MTU_SIZE(1500);

const uint MIN_BPS(2000);
const uint MAX_BPS(20000);

const uint MIN_NET_FPS(10);
const uint MAX_NET_FPS(60);

const uint CLIENT_CONNECTION_GAME_HOST              (BIT(0));
const uint CLIENT_CONNECTION_ADMIN                  (BIT(1) | CLIENT_CONNECTION_GAME_HOST);
const uint CLIENT_CONNECTION_LOCAL                  (BIT(2) | CLIENT_CONNECTION_ADMIN);
const uint CLIENT_CONNECTION_LAN                    (BIT(3));

const uint CLIENT_CONNECTION_IN_GAME                (BIT(4));
const uint CLIENT_CONNECTION_AUTH_REQUESTED         (BIT(5));
const uint CLIENT_CONNECTION_KEY_VALIDATE_REQUESTED (BIT(6));
const uint CLIENT_CONNECTION_PSEUDO                 (BIT(7));
const uint CLIENT_CONNECTION_LEAVER                 (BIT(8));

const uint CLIENT_CONNECTION_STAFF                  (BIT(9));
const uint CLIENT_CONNECTION_PREMIUM                (BIT(10));

const uint CLIENT_CONNECTION_TIMING_OUT             (BIT(11));

const uint CLIENT_CONNECTION_TRIAL                  (BIT(12));

const bool  svr_requireAuthentication(REQUIRE_AUTHENTICATION);

struct SPacketRecord
{
    uint    uiTimeStamp;
    uint    uiSize;

    SPacketRecord(uint _uiTimeStamp, uint _uiSize) :
    uiTimeStamp(_uiTimeStamp),
    uiSize(_uiSize)
    {}
};

struct SSentSnapshots
{
    uint    uiFrame;
    uint    uiStream;

    SSentSnapshots(uint _uiFrame, uint _uiStream) :
    uiFrame(_uiFrame),
    uiStream(_uiStream)
    {}
};

typedef deque<SPacketRecord>        PacketRecordDeque;
typedef PacketRecordDeque::iterator PacketRecordDeque_it;
//=============================================================================

//=============================================================================
// CClientConnection
//=============================================================================
class CClientConnection
{
private:
    CHTTPManager*           m_pHTTPManager;
    CHostServer*            m_pHostServer;

    int                     m_iClientNum;
    int                     m_iAccountID;
    int                     m_iClanID;
    ushort                  m_unConnectionID;
    tstring                 m_sName;
    tstring                 m_sCookie;
    tstring                 m_sGameCookie;
    tstring                 m_sMatchKey;
    tstring                 m_sInvitationCode;
    uint                    m_uiFlags;
    float                   m_fLoadingProgress;

    CSocket                 m_sockGame;
    tstring                 m_sAddress;
    tstring                 m_sPublicAddress;
    ushort                  m_unPort;

    CPacket                 m_pktSend;
    CPacket                 m_pktReliable;

    EClientConnectionState  m_eConnectionState;
    uint                    m_uiLastReceiveTime;
    uint                    m_uiLastAckedServerFrame;
    uint                    m_uiLastReceivedClientTime;
    bool                    m_bFirstAckReceived;
    bool                    m_bAllStateDataSent;
    bool                    m_bWorldLoaded;
    ushort                  m_unPing;

    // Snapshots
    bool                    m_bSnapshotIsCompressed;
    uint                    m_uiUncompressedSnapshotLength;
    CBufferBit              m_cBufferSnapshot;
    CBufferDynamic          m_bufferEndGame;
    PoolHandle              m_hFirstSnapshot;
    PoolHandle              m_hRetryFirstSnapshot;
    bool                    m_bFirstSnapshotSent;
    uint                    m_uiFirstSnapshotSendTime;
    PoolHandle              m_hLastAckedSnapshot;
    uint                    m_uiLastAckedStream;
    uint                    m_uiSnapshotBufferIndex;
    uint                    m_uiSnapshotFragmentFrame;
    byte                    m_ySnapshotFragmentIndex;
    uint                    m_uiPrevButtonStates;

    deque<SSentSnapshots>   m_deqSentSnapshots;

    byte                    m_yStateSequence;

    // State strings
    uideque                 m_deqStateStringUpdatesID;
    deque<CStateString>     m_deqStateStringUpdates;
    ushort                  m_unStateStringFragmentIndex;
    bool                    m_bStateStringIsCompressed;
    uint                    m_uiUncompressedStateStringLength;
    CBufferDynamic          m_bufferStateString;
    uint                    m_uiStateStringBufferIndex;

    // State blocks
    uideque                 m_deqStateBlockUpdatesID;
    deque<CStateBlock>      m_deqStateBlockUpdates;
    ushort                  m_unStateBlockFragmentIndex;
    bool                    m_bStateBlockIsCompressed;
    uint                    m_uiUncompressedStateBlockLength;
    CBufferDynamic          m_bufferStateBlock;
    uint                    m_uiStateBlockBufferIndex;
    ivector                 m_vStateBlockModCounts;

    // Net settings
    zdeque                  m_deqSendSize;                  // Bandwidth estimation
    size_t                  m_zSentLength;                  // Data length that we've sent this frame
    uint                    m_uiMaxPacketSize;              // The max packet size we can send per server frame to the client
    uint                    m_uiMaxBPS;                     // Max bytes per second the client wants to receive
    uint                    m_uiNetFPS;
    
    bool                    m_bSnapshotSent;

    uint                    m_uiLastBandwidthWarn;
    bool                    m_bBehind;

    uint                    m_uiLastLoadingProgressUpdate;
    uint                    m_uiLongestLoadingProgressInterval;

    uint                    m_uiLastAuthSuccess;

#ifndef K2_CLIENT
    CHTTPRequest*           m_pAuthenticateRequest;
    CHTTPRequest*           m_pValidateMatchKeyRequest;
    CHTTPRequest*           m_pRefreshUpgradesRequest;
    CHTTPRequest*           m_pRecentMatchStatsRequest;
    bool                    m_bForceTermination;
#endif

    uint                    m_uiAuthRequestTime;

    PacketRecordDeque       m_deqIncomingPackets;

    uint                    m_uiStream;

    sset                    m_setAvailableUpgrades;
    tsmapts                 m_mapSelectedUpgrades;

    bool                    m_bRefreshUpgrades;
    uint                    m_uiLastRefreshUpgradesTime;

    CClientConnection();

#ifndef K2_CLIENT
    void    CheckAuthResult();
    void    Authenticate();
    
    void    ValidateMatchKey();
#endif

public:
    ~CClientConnection();
    CClientConnection(CHostServer* pHostServer, CHTTPManager *pHTTPManager, const tstring &sAddress, ushort unPort, CSocket &sockGame);

    void            SetClientNumber(int iClientNumber)                      { m_iClientNum = iClientNumber; }
    int             GetClientNum() const                                    { return m_iClientNum; }
    
    int             GetAccountID() const                                    { return m_iAccountID; }
    int             GetClanID() const                                       { return m_iClanID; }

    uint            GetMaxPacketSize() const                                { return m_uiMaxPacketSize; }

    void            SetConnectionID(ushort unID)                            { m_unConnectionID = unID; }
    ushort          GetConnectionID() const                                 { return m_unConnectionID; }

    void            SetPublicAddress(const tstring &sAddress)               { m_sPublicAddress = sAddress; }
    const tstring&  GetPublicAddress() const                                { return m_sPublicAddress; }
    const tstring&  GetAddress() const                                      { return m_sAddress; }
    ushort          GetPort() const                                         { return m_unPort; }

    void            SetName(const tstring &sName)                           { m_sName = sName; }
    const tstring&  GetName() const                                         { return m_sName; }

    void            SetCookie(const tstring &sCookie)                       { m_sCookie = sCookie; }
    const tstring&  GetCookie() const                                       { return m_sCookie; }
    void            SetGameCookie(const tstring &sGameCookie)               { m_sGameCookie = sGameCookie; }
    const tstring&  GetGameCookie() const                                   { return m_sGameCookie; }

    void            SetMatchKey(const tstring &sMatchKey)                   { m_sMatchKey = sMatchKey; }
    const tstring&  GetMatchKey() const                                     { return m_sMatchKey; }
    void            ClearMatchKey()                                         { m_sMatchKey.clear(); }

    void            SetInvitationCode(const tstring &sInvite)               { m_sInvitationCode = sInvite; }
    const tstring&  GetInvitationCode() const                               { return m_sInvitationCode; }
    void            ClearInvitationCode()                                   { m_sInvitationCode.clear(); }

    float           GetLoadingProgress() const                              { return m_fLoadingProgress; }

    ushort          GetPing() const                                         { return m_unPing; }

    EClientConnectionState  GetState() const                                { return m_eConnectionState; }
    bool                    IsConnected() const                             { return m_eConnectionState != CLIENT_CONNECTION_STATE_DISCONNECTED; }
    void                    ResynchStateData();

    void            SetAccountID(int iAccountID)                            { m_iAccountID = iAccountID; }
    
    void            SetFlags(uint uiFlags)                                  { m_uiFlags |= uiFlags; }
    bool            HasFlags(uint uiFlags) const                            { return (m_uiFlags & uiFlags) == uiFlags; }
    uint            RemoveFlags(uint uiFlags)                               { return m_uiFlags &= ~uiFlags; }
    void            ClearFlags()                                            { m_uiFlags = 0; }

    bool            ReadRemoteCommandPacket(CPacket &packet);
    bool            ReadClientSnapshot(CPacket &pkt);

    void            ProcessPacket(CPacket &pkt);
    K2_API void     SendGameData(const IBuffer &buffer, bool bReliable);
    K2_API bool     SendReliablePacket(CPacket &pkt);

    void            CheckPort(ushort unPort);

    bool            BandwidthOK();
#ifndef K2_CLIENT
    void            ConverseWithMasterServer();
#endif
    void            WriteClientPackets(uint uiFPS);
    void            SendPackets(uint uiFPS);
    void            WriteSnapshot(PoolHandle hSnapshot);
    void            WriteSnapshotFragments();
    K2_API void     Disconnect(const tstring &sReason);
    bool            CheckTimeout();
    bool            IsTimingOut();
    void            SendAllStateData();
    void            AddStringToUpdateQueue(ushort unID, const CStateString &ss);
    void            AddBlockToUpdateQueue(ushort unID, const CStateBlock &block);
    int             GetStateBlockModCount(ushort unID)                      { return (unID >= m_vStateBlockModCounts.size()) ? -1 : m_vStateBlockModCounts[unID]; }
    void            SendStateStrings();
    void            SendStateBlocks();
    bool            ProcessNetSettings(CPacket &pkt);

    void            SendLoadWorldRequest();
    void            SendAuthRequest();

    EClientConnectionState  GetConnectionState()                            { return m_eConnectionState; }

    void            NewGameStarted();

    void            ProcessOutOfSequencePackets();

    K2_API  bool    SendPacket(CPacket &pkt);
    CSocket&        GetSocket()                     { return m_sockGame; }

    void            SoftReconnect()                 { m_bFirstAckReceived = false; m_hLastAckedSnapshot = INVALID_POOL_HANDLE; m_bFirstSnapshotSent = false; m_uiFirstSnapshotSendTime = INVALID_TIME; m_eConnectionState = CLIENT_CONNECTION_STATE_READY; }

    void            SetStream(uint uiStream)        { m_uiStream = uiStream; }

    void            AuthSuccess(int iAccountID, const tstring &sName);

    const sset&     GetAvailableUpgrades() const    { return m_setAvailableUpgrades; }
    const tsmapts&  GetSelectedUpgrades() const     { return m_mapSelectedUpgrades; }

#ifndef K2_CLIENT
    K2_API void     RefreshUpgrades();
#endif
};
//=============================================================================

#endif //__C_CLIENTCONNECTION_H__
