// (C)2005 S2 Games
// c_hostclient.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_host.h"
#include "c_hostclient.h"
#include "c_world.h"
#include "c_clientgamelib.h"
#include "c_snapshot.h"
#include "c_socket.h"
#include "c_statestring.h"
#include "c_networkresourcemanager.h"
#include "c_chatmanager.h"
#include "c_zip.h"
#include "c_sample.h"
#include "c_netstats.h"
#include "c_updater.h"
#include "c_eventmanager.h"
#include "c_inputstate.h"
#include "c_actionregistry.h"
#include "c_hostserver.h"
#include "c_uicmd.h"
#include "c_voicemanager.h"
#include "c_stringtable.h"
#include "i_widget.h"
#include "i_listwidget.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_INT(net_lagThreshold);
EXTERN_CVAR_INT(net_problemIconTime);
EXTERN_CVAR_BOOL(cc_showGameInvites);
EXTERN_CVAR_BOOL(cc_showNewPatchNotification);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOATR(    cl_simulatePacketLoss,      0.0f,               0,                  0.0f,   1.0f);
CVAR_BOOL(      cl_showUnordered,           false);
CVAR_BOOLF(     cl_ignoreBadPackets,        false,              CVAR_SAVECONFIG);
CVAR_BOOL(      cl_showPackets,             false);
CVAR_STRING(    cl_password,                "");
CVAR_INTR(      cl_packetSendFPS,           30,                 CVAR_SAVECONFIG,    1,      85);
CVAR_BOOLF(     cl_printStateStringChanges, false,              CVAR_SAVECONFIG);
CVAR_BOOL(      cl_debugGameTime,           false);
CVAR_BOOL(      cl_smoothGameTime,          true);
CVAR_UINTF(     cl_timeout,                 15000,              CVAR_SAVECONFIG);
CVAR_UINTF(     cl_connectionID,            0,                  CVAR_SAVECONFIG);
CVAR_FLOATF(    cl_infoRequestRate,         30.0f,              CVAR_SAVECONFIG);
CVAR_BOOL(      cl_debugSnapshot,           false);

CVAR_STRINGF(   cl_reconnectAddress,        "",                 CVAR_SAVECONFIG);
CVAR_UINTF(     cl_reconnectMatchID,        0,                  CVAR_SAVECONFIG);

#ifdef K2_GARENA
CVAR_BOOLF(     cl_GarenaEnable,            true,               CVAR_READONLY);
#else
CVAR_BOOLF(     cl_GarenaEnable,            false,              CVAR_READONLY);
#endif

CVAR_STRING(    cl_filterName,              "");
CVAR_STRING(    cl_inviteAddress,           "");

CVAR_INTF(      net_maxPacketSize,          1300,               CVAR_SAVECONFIG);
CVAR_INTF(      net_maxBPS,                 20000,              CVAR_SAVECONFIG);
CVAR_INTF(      net_FPS,                    20,                 CVAR_SAVECONFIG);
CVAR_STRINGF(   net_name,                   "UnnamedNewbie",    CVAR_SAVECONFIG);

CVAR_INT(       _cl_ms,                     0);
CVAR_STRING(    _cl_state,                  "");
CVAR_INT(       _cl_delta,                  0);
CVAR_FLOAT(     _cl_lerp,                   0.0f);
CVAR_INT(       _cl_nudgegametime,          0);

CVAR_FLOATF (   cam_smoothPositionHalfLife, 0.1f,               CVAR_SAVECONFIG);
CVAR_FLOATF (   cam_smoothAnglesHalfLife,   0.05f,              CVAR_SAVECONFIG);
CVAR_FLOATR (   cam_fov,                    90.0f,              CVAR_SAVECONFIG,    MIN_PLAYER_FOV, MAX_PLAYER_FOV);

CVAR_BOOL(      cl_testFailConnect,         false);
CVAR_BOOL(      cl_testFailLoad,            false);

INPUT_STATE_BOOL(BlockIncoming);
INPUT_STATE_BOOL(BlockOutgoing);

UI_TRIGGER(HostErrorMessage);
UI_TRIGGER(AccountInfo);

UI_TRIGGER(ServerListAdd);
UI_TRIGGER(ServerListShow);
UI_TRIGGER(ServerListHide);
UI_TRIGGER(ServerListUpdate);
UI_TRIGGER(ServerListClear);

UI_TRIGGER(GameListAdd);
UI_TRIGGER(GameListShow);
UI_TRIGGER(GameListHide);
UI_TRIGGER(GameListUpdate);
UI_TRIGGER(GameListClear);

UI_TRIGGER(OngoingGameList);
UI_TRIGGER(OngoingGameListShow);
UI_TRIGGER(OngoingGameListHide);
UI_TRIGGER(OngoingGameListUpdate);
UI_TRIGGER(OngoingGameListClear);

UI_TRIGGER(LocalServerList);
UI_TRIGGER(LocalGameList);
UI_TRIGGER(RemoteLoading);
UI_TRIGGER(GameInvite);
UI_TRIGGER(ReconnectShow);
UI_TRIGGER(ReconnectTimer);
UI_TRIGGER(ReconnectAddress);
//=============================================================================

/*====================
  XtoA
  ====================*/
tstring     XtoA(EClientState eState)
{
    switch (eState)
    {
    case CLIENT_STATE_IDLE:
        return _T("CLIENT_STATE_IDLE");

    case CLIENT_STATE_PRELOADING:
        return _T("CLIENT_STATE_PRELOADING");

    case CLIENT_STATE_CONNECTING:
        return _T("CLIENT_STATE_CONNECTING");

    case CLIENT_STATE_LOADING:
        return _T("CLIENT_STATE_LOADING");

    case CLIENT_STATE_CONNECTED:
        return _T("CLIENT_STATE_CONNECTED");

    case CLIENT_STATE_WAITING_FIRST_SNAPSHOT:
        return _T("CLIENT_STATE_WAITING_FIRST_SNAPSHOT");

    case CLIENT_STATE_IN_GAME:
        return _T("CLIENT_STATE_IN_GAME");
    
    default:
        return _T("");
    }
}


/*====================
  CHostClient::~CHostClient
  ====================*/
CHostClient::~CHostClient()
{
    Disconnect();

    // Clear out any buffers allocated to hold snapshot fragments
    m_uiServerFrame = m_uiPrevServerFrame = UINT_MAX;
    CheckSnapshotFragments();

    if (m_pGameLib != nullptr)
        m_pGameLib->Shutdown();
    SAFE_DELETE(m_pGameLib);

    m_sockGame.Close();
    SAFE_DELETE(m_pWorld);
}


/*====================
  CHostClient::CHostClient
  ====================*/
CHostClient::CHostClient(uint uiIndex, CHTTPManager *pHTTPManager) :
m_ClientAccount(pHTTPManager),
m_ServerList(pHTTPManager),
m_uiIndex(uiIndex),
m_pGameLib(nullptr),

m_hClientMessages(g_ResourceManager.Register(_T("/stringtables/client_messages.str"), RES_STRINGTABLE)),

m_eState(CLIENT_STATE_IDLE),
m_uiFrameCount(0),
m_uiGameTime(0),
m_uiLastGameTime(0),
m_uiClientFrameLength(0),

m_unConnectionID(0),
m_iClientNum(-1),
m_uiLastSendTime(INVALID_TIME),
m_uiLastSnapshotReceiveTime(INVALID_TIME),
m_uiServerTimeout(INVALID_TIME),
m_uiServerFPS(20),
m_bReadyToSendSnapshot(true),

m_uiServerFrame(0),
m_uiServerTimeStamp(0),
m_uiPrevServerFrame(0),
m_uiPrevServerTimeStamp(0),
m_uiLastAckedServerFrame(0),

m_sockGame(_T("LOCAL_CLIENT")),

m_pWorld(nullptr),

m_bStartedLoadingNetworkResources(false),
m_uiNetworkResourceCount(0),

m_yStateSequence(0),
m_sOldCookie(_T("")),

m_bReceivedStateData(false),
m_bReceivedAuth(false),
m_bStartLoading(false),
m_bSentKeepAlive(false),
m_bGameHost(false),
m_bSilentConnect(false),
m_bPractice(false),
m_bAutoStartGame(false),
m_bAutoStartGameLocal(false),
m_uiLastUpdateCheck(INVALID_TIME),
m_yAuthFlags(0),
m_bWasIdle(false),
m_bStartedLoadingWorld(false),

m_sReconnectIP(_T("")),
m_uiReconnectMatchID(-1)
{
    // Create reserved state strings
    m_vStateStrings.resize(STATE_STRING_NUM_RESERVED);

    // Create reserved state blocks
    m_vStateBlocks.resize(STATE_BLOCK_NUM_RESERVED);

    m_ServerList.SetClient(this);

    for (int i(0); i < NUM_CLIENT_STATES; ++i)
        m_auiStateStartTime[i] = INVALID_TIME;
}


/*====================
  CHostClient::GetStateString
  ====================*/
CStateString*   CHostClient::GetStateString(ushort unID)
{
    try
    {
        if (unID >= m_vStateStrings.size())
            EX_WARN(_T("Invalid state string ID: ") + XtoA(unID));

        return &(m_vStateStrings[unID]);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::GetStateString() - "));
        return nullptr;
    }
}


/*====================
  QueueNetworkResourceUpdate
  ====================*/
void    QueueNetworkResourceUpdate(const string &sStateUTF8, const string &sValueUTF8)
{
    tstring sValue(UTF8ToTString(sValueUTF8));

    SNetworkResourceRecord record;
    record.m_uiIndex = AtoI(UTF8ToString(sStateUTF8));
    record.m_eType = EResourceType(AtoI(sValue.substr(0, 3)));
    record.m_sPath = sValue.substr(3);

    CHostClient *pClient(Host.GetCurrentClient());
    if (pClient == nullptr)
        return;

    pClient->QueueNetworkResource(record);
}


/*====================
  UpdateNetworkResource
  ====================*/
void    UpdateNetworkResource(const string &sStateUTF8, const string &sValueUTF8)
{
    uint uiIndex(AtoI(UTF8ToString(sStateUTF8)));

    tstring sValue(UTF8ToTString(sValueUTF8));
    EResourceType eType(EResourceType(AtoI(sValue.substr(0, 3))));
    tstring sPath(sValue.substr(3));

    // at this point, the specified path should have already been precached.
    ResHandle hRes(g_ResourceManager.LookUpPath(sPath));
    if (hRes == INVALID_RESOURCE)
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            if (eType == RES_TEXTURE)
                hRes = g_ResourceManager.Register(K2_NEW(ctx_HostClient,  CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
            else
                hRes = g_ResourceManager.Register(sPath, eType);
        }
    }
    NetworkResourceManager.RegisterNetworkResource(hRes, uiIndex);
}


/*====================
  CHostClient::StartLoadingNetworkResources
  ====================*/
void    CHostClient::StartLoadingNetworkResources()
{
    CStateString *pStateString(GetStateString(STATE_STRING_RESOURCES));
    if (pStateString == nullptr)
        return;

    m_bStartedLoadingNetworkResources = true;
    pStateString->ForEachState(QueueNetworkResourceUpdate, false);
    m_uiNetworkResourceCount = INT_SIZE(m_deqNetworkResources.size());
}


/*====================
  CHostClient::LoadNextNetworkResource
  ====================*/
void    CHostClient::LoadNextNetworkResource()
{
    if (m_deqNetworkResources.empty())
        return;

    uint uiIndex(m_deqNetworkResources.front().m_uiIndex);
    EResourceType eType(m_deqNetworkResources.front().m_eType);
    tstring sPath(m_deqNetworkResources.front().m_sPath);
    m_deqNetworkResources.pop_front();

    // Load the resource
    ResHandle hRes(INVALID_RESOURCE);
    K2_WITH_GAME_RESOURCE_SCOPE()
    {
        if (eType == RES_TEXTURE)
            hRes = g_ResourceManager.Register(K2_NEW(ctx_HostClient,  CTexture)(sPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
        else
            hRes = g_ResourceManager.Register(sPath, eType);
    }
    NetworkResourceManager.RegisterNetworkResource(hRes, uiIndex);
}


/*====================
  CHostClient::SetStateString
  ====================*/
void    CHostClient::SetStateString(ushort unID, const IBuffer &buffer)
{
    try
    {
        // Grow the vector if this state string id is beyond the highest known
        if (m_vStateStrings.size() <= unID)
            m_vStateStrings.resize(unID + 1);

        // If the string exists, just update it
        CStateString *pStateString(GetStateString(unID));
        if (pStateString == nullptr)
            EX_WARN(_T("Failed to retrieve state string: ") + XtoA(unID));

        CStateString ssDiff;
        ssDiff.Set(buffer);

        if (unID == STATE_STRING_CVAR_SETTINGS)
        {
            if (pStateString->IsEmpty())
                ICvar::SetTransmitCvars(&ssDiff);
            else
                ICvar::ModifyTransmitCvars(&ssDiff);

            if (~m_yAuthFlags & BIT(0))
                ICvar::ProtectTransmitCvars();
        }

        // Only update the network resource list right away if a world is already loaded,
        // since world archives can override files
        if (unID == STATE_STRING_RESOURCES && m_pWorld != nullptr && m_pWorld->IsLoaded())
        ssDiff.ForEachState(UpdateNetworkResource, false);

        pStateString->Modify(buffer);

        if (unID == STATE_STRING_SERVER_INFO)
        {
            m_uiServerFPS = pStateString->GetInt(_T("svr_gameFPS"));
            if (m_uiServerFPS == 0)
            {
                m_uiServerFPS = 20;
                Console.Warn << _T("Server did not send it's target FPS, using a default of ") << m_uiServerFPS << newl;
            }
        }

        m_pGameLib->StateStringChanged(unID, *pStateString);

        Console.SetDefaultStream(Console.Dev);
        Console << _T("State string #") << unID << _T(" updated") << newl;
        if (cl_printStateStringChanges)
            ssDiff.Print();
        Console.SetDefaultStream(Console.Client);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::SetStateString() - "));
    }
}


/*====================
  CHostClient::GetStateBlock
  ====================*/
CStateBlock*    CHostClient::GetStateBlock(ushort unID)
{
    try
    {
        if (unID >= m_vStateBlocks.size())
            EX_WARN(_T("Invalid state block ID: ") + XtoA(unID));

        return &(m_vStateBlocks[unID]);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::GetStateBlock() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  CHostClient::SetStateBlock
  ====================*/
void    CHostClient::SetStateBlock(ushort unID, const IBuffer &buffer)
{
    try
    {
        // Grow the vector if this state string id is beyond the highest known
        if (m_vStateBlocks.size() <= unID)
            m_vStateBlocks.resize(unID + 1);

        // If the string exists, just update it
        CStateBlock *pStateBlock(GetStateBlock(unID));
        if (pStateBlock == nullptr)
            EX_WARN(_T("Failed to retrieve state block: ") + XtoA(unID));

        CStateBlock ssDiff;
        ssDiff.Set(buffer);

        pStateBlock->Modify(buffer);

        m_pGameLib->StateBlockChanged(unID, *pStateBlock);

        Console.Dev << _T("State block #") << unID << _T(" updated") << newl;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::SetStateBlock() - "));
    }
}


/*====================
  CHostClient::Init
  ====================*/
void    CHostClient::Init()
{
    PROFILE("CHostClient::Init");

    try
    {
        m_unConnectionID = cl_connectionID;

        // Load the game library
        m_pGameLib = K2_NEW(ctx_HostClient,  CClientGameLib)(CLIENT_LIBRARY_NAME);
        if (m_pGameLib == nullptr || !m_pGameLib->IsValid())
            EX_ERROR(_T("Failed to load client library"));

        // Create a CWorld, in case the client wants to load a world
        m_pWorld = K2_NEW(ctx_World,  CWorld)(WORLDHOST_CLIENT);
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Failed to allocate a CWorld"));
        if (!m_pWorld->IsValid())
            EX_ERROR(_T("Failure constructing CWorld"));

        // Successful start
        Console.Client << _T("Started client ") << QuoteStr(m_pGameLib->GetName()) << _T(" version ")
                << XtoA(m_pGameLib->GetMajorVersion()) << _T(".") << XtoA(m_pGameLib->GetMinorVersion()) << newl;
        m_pGameLib->Init(this);
#ifdef _DEBUG
        K2System.SetTitle(m_pGameLib->GetName() + _T(" [DEBUG]"));
#else
        K2System.SetTitle(m_pGameLib->GetName());
#endif

        IModalDialog::SetNumLoadingJobs(5);
    }
    catch (CException &ex)
    {
        SAFE_DELETE(m_pWorld);
        SAFE_DELETE(m_pGameLib);
        ex.Process(_T("CHostClient::Init() - "));
    }
}


/*====================
  CHostClient::SetGamePointer
  ====================*/
void    CHostClient::SetGamePointer()
{
    if (m_pGameLib != nullptr)
        m_pGameLib->SetGamePointer(m_uiIndex);
}


/*====================
  CHostClient::ResetStateData
  ====================*/
void    CHostClient::ResetStateData()
{
    Console.Client << _T("Clearing all state data...") << newl;

    m_yStateSequence = 0;

    // Dealloc all state strings
    m_vStateStrings.clear();
    m_vStateStrings.resize(STATE_STRING_NUM_RESERVED);
    m_bufferStateStringFragment.Clear();
    m_unStateStringFragmentIndex = 0;

    // Dealloc all state blocks
    m_vStateBlocks.clear();
    m_vStateBlocks.resize(STATE_BLOCK_NUM_RESERVED);
    m_bufferStateBlockFragment.Clear();
    m_unStateBlockFragmentIndex = 0;

    if (!Host.HasServer())
        ICvar::ResetVars(CVAR_TRANSMIT);
}


/*====================
  CHostClient::ReadStateStringUpdate
  ====================*/
bool    CHostClient::ReadStateStringUpdate(CPacket &pkt, bool bCompressed)
{
    try
    {
        ushort unID(pkt.ReadShort());
        ushort unReadSize(pkt.ReadShort());
        if (unReadSize > pkt.GetUnreadLength())
            EX_ERROR(_T("StateString packet is truncated"));

        uint uiDataSize(unReadSize);
        if (bCompressed)
            pkt >> uiDataSize;

        CBufferDynamic bufferStateString(uiDataSize);
        if (bCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, uiDataSize));
            CZip::Decompress((const byte*)pkt.GetBuffer(), unReadSize, pCompressed, uiDataSize);
            bufferStateString.Write(pCompressed, uiDataSize);
            SAFE_DELETE_ARRAY(pCompressed);
        }
        else
        {
            bufferStateString.Write(pkt.GetBuffer(), unReadSize);
        }

        pkt.Advance(unReadSize);

        SetStateString(unID, bufferStateString);
        ++m_yStateSequence;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateStringUpdate() - "));
        return false;
    }
}


/*====================
  CHostClient::ReadStateStringFragment
  ====================*/
bool    CHostClient::ReadStateStringFragment(CPacket &pkt)
{
    try
    {
        ushort unID(pkt.ReadShort());
        if (m_unStateStringFragmentIndex != 0 && m_unStateStringFragmentIndex != unID)
            EX_ERROR(_T("Received a state string fragment with a new ID before terminating the old one"));

        ushort unReadSize(pkt.ReadShort());
        m_unStateStringFragmentIndex = unID;
        m_bufferStateStringFragment.Append(pkt.GetBuffer(), unReadSize);
        pkt.Advance(unReadSize);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateStringFragment() - "));
        return false;
    }
}


/*====================
  CHostClient::ReadStateStringTermination
  ====================*/
bool    CHostClient::ReadStateStringTermination(CPacket &pkt, bool bCompressed)
{
    try
    {
        ushort unID(pkt.ReadShort());
        if (m_unStateStringFragmentIndex == 0)
            EX_ERROR(_T("Received a state string termination without any fragments"));
        if (m_unStateStringFragmentIndex != unID)
            EX_ERROR(_T("Received a state string termination with a new ID before terminating the old one"));

        ushort unReadSize(pkt.ReadShort());
        if (unReadSize > pkt.GetUnreadLength())
            EX_ERROR(_T("StateString packet is truncated"));

        uint uiDataSize(unReadSize);
        if (bCompressed)
            pkt >> uiDataSize;

        m_bufferStateStringFragment.Append(pkt.GetBuffer(), unReadSize);

        CBufferDynamic bufferStateString(uiDataSize);
        if (bCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, uiDataSize));
            CZip::Decompress((const byte*)m_bufferStateStringFragment.Get(), m_bufferStateStringFragment.GetLength(), pCompressed, uiDataSize);
            bufferStateString.Write(pCompressed, uiDataSize);
            SAFE_DELETE_ARRAY(pCompressed);
        }
        else
        {
            bufferStateString.Write(m_bufferStateStringFragment.Get(), m_bufferStateStringFragment.GetLength());
        }

        pkt.Advance(unReadSize);

        SetStateString(unID, bufferStateString);
        m_unStateStringFragmentIndex = 0;
        m_bufferStateStringFragment.Clear();
        ++m_yStateSequence;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateStringTermination() - "));
        return false;
    }
}


/*====================
  CHostClient::ReadStateBlockUpdate
  ====================*/
bool    CHostClient::ReadStateBlockUpdate(CPacket &pkt, bool bCompressed)
{
    try
    {
        ushort unID(pkt.ReadShort());
        ushort unReadSize(pkt.ReadShort());
        if (unReadSize > pkt.GetUnreadLength())
            EX_ERROR(_T("StateBlock packet is truncated"));

        uint uiDataSize(unReadSize);
        if (bCompressed)
            pkt >> uiDataSize;

        CBufferDynamic bufferStateBlock(uiDataSize);
        if (bCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, uiDataSize));
            CZip::Decompress((const byte*)pkt.GetBuffer(), unReadSize, pCompressed, uiDataSize);
            bufferStateBlock.Write(pCompressed, uiDataSize);
            SAFE_DELETE_ARRAY(pCompressed);
        }
        else
        {
            bufferStateBlock.Write(pkt.GetBuffer(), unReadSize);
        }

        pkt.Advance(unReadSize);

        SetStateBlock(unID, bufferStateBlock);
        ++m_yStateSequence;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateBlockUpdate() - "));
        return false;
    }
}


/*====================
  CHostClient::ReadStateBlockFragment
  ====================*/
bool    CHostClient::ReadStateBlockFragment(CPacket &pkt)
{
    try
    {
        ushort unID(pkt.ReadShort());
        if (m_unStateBlockFragmentIndex != 0 && m_unStateBlockFragmentIndex != unID)
            EX_ERROR(_T("Received a state string fragment with a new ID before terminating the old one"));

        ushort unReadSize(pkt.ReadShort());
        m_unStateBlockFragmentIndex = unID;
        m_bufferStateBlockFragment.Append(pkt.GetBuffer(), unReadSize);
        pkt.Advance(unReadSize);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateBlockFragment() - "));
        return false;
    }
}


/*====================
  CHostClient::ReadStateBlockTermination
  ====================*/
bool    CHostClient::ReadStateBlockTermination(CPacket &pkt, bool bCompressed)
{
    try
    {
        ushort unID(pkt.ReadShort());
        if (m_unStateBlockFragmentIndex == 0)
            EX_ERROR(_T("Received a state string termination without any fragments"));
        if (m_unStateBlockFragmentIndex != unID)
            EX_ERROR(_T("Received a state string termination with a new ID before terminating the old one"));

        ushort unReadSize(pkt.ReadShort());
        if (unReadSize > pkt.GetUnreadLength())
            EX_ERROR(_T("StateBlock packet is truncated"));

        uint uiDataSize(unReadSize);
        if (bCompressed)
            pkt >> uiDataSize;

        m_bufferStateBlockFragment.Append(pkt.GetBuffer(), unReadSize);

        CBufferDynamic bufferStateBlock(uiDataSize);
        if (bCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, uiDataSize));
            CZip::Decompress((const byte*)m_bufferStateBlockFragment.Get(), m_bufferStateBlockFragment.GetLength(), pCompressed, uiDataSize);
            bufferStateBlock.Write(pCompressed, uiDataSize);
            SAFE_DELETE_ARRAY(pCompressed);
        }
        else
        {
            bufferStateBlock.Write(m_bufferStateBlockFragment.Get(), m_bufferStateBlockFragment.GetLength());
        }

        pkt.Advance(unReadSize);

        SetStateBlock(unID, bufferStateBlock);
        m_unStateBlockFragmentIndex = 0;
        m_bufferStateBlockFragment.Clear();
        ++m_yStateSequence;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadStateBlockTermination() - "));
        return false;
    }
}


/*====================
  CHostClient::AllStateDataReceived
  ====================*/
bool    CHostClient::AllStateDataReceived()
{
    Console.Client << _T("Server finished sending state data") << newl;

    // Server info state string
    CStateString *pServerInfoString(GetStateString(STATE_STRING_SERVER_INFO));
    if (pServerInfoString == nullptr || pServerInfoString->IsEmpty())
    {
        Console << _T("Client does not have a server info state string") << newl;
        return false;
    }

    m_uiServerFPS = pServerInfoString->GetInt(_T("svr_gameFPS"));
    if (m_uiServerFPS == 0)
    {
        m_uiServerFPS = 20;
        Console.Warn << _T("Server did not send it's target FPS, using a default of ") << m_uiServerFPS << newl;
    }

    Console << _T("Server info state string:") << newl;
    pServerInfoString->Print();

    m_pktReliableSend << NETCMD_CLIENT_READY;
    m_bReceivedStateData = true;
    return true;
}


/*====================
  CHostClient::StartLoadingWorld
  ====================*/
bool    CHostClient::StartLoadingWorld(const tstring &sWorldName, bool bPreload)
{
    RemoteLoading.Trigger(XtoA(false));

    m_bSilentConnect = false;

    if (Host.HasServer())
    {
        IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
        IModalDialog::Show(_T("loading"));

        IModalDialog::NextLoadingJob();
    }
    else if (bPreload)
    {
        IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
        IModalDialog::Show(_T("loading_matchmaking_preload"));

        IModalDialog::SetNumLoadingJobs(4);
    }
    else
    {
        IModalDialog::SetDisplay(LOADING_DISPLAY_INTERFACE);
        IModalDialog::Show(m_sLoadingInterface);

        IModalDialog::SetNumLoadingJobs(5);
    }

    IModalDialog::SetProgress(0.0f);
    IModalDialog::Update();

    m_pGameLib->Reinitialize();

    if (bPreload ||
        m_pWorld == nullptr ||
        !m_pWorld->IsPreloaded(sWorldName))
    {
        Console.Client << _T("Client loading world ") << QuoteStr(sWorldName) << newl;

        m_pWorld->ClearPreloaded();

        m_pGameLib->StartLoading(sWorldName);

        if (!m_pWorld->StartLoad(sWorldName, bPreload))
            return false;
    }
    else
    {
        m_pGameLib->StartLoading(sWorldName);
    }

    Console << _T("Setting m_bStartedLoadingWorld true") << newl;
    m_bStartedLoadingWorld = true;

    //IModalDialog::SetLoadingImage(m_pWorld->GetLoadingImagePath());

    return true;
}


/*====================
  CHostClient::ReadCompleteSnapshot
  ====================*/
bool    CHostClient::ReadCompleteSnapshot(CBufferBit &cBuffer)
{
    PROFILE("CHostClient::ReadCompleteSnapshot");

    // Process game events from the old snapshot if we received
    // multiple this frame, so they aren't lost
    if (m_uiLastSnapshotReceiveTime == Host.GetSystemTime() &&
        m_cSnapshotTemp.GetStateSequence() == GetStateSequence() &&
        m_cSnapshotTemp.GetFrameNumber() >= m_uiServerFrame &&
        m_eState == CLIENT_STATE_IN_GAME)
    {
        Console.SetDefaultStream(Console.ClientGame);
        m_pGameLib->ProcessGameEvents(m_cSnapshotTemp);
        Console.SetDefaultStream(Console.Client);
    }

    m_cSnapshotTemp.ReadBuffer(cBuffer);

    m_uiLastSnapshotReceiveTime = Host.GetSystemTime();

    return true;
}


/*====================
  CHostClient::ProcessCurrentSnapshot
  ====================*/
bool    CHostClient::ProcessCurrentSnapshot()
{
    PROFILE("CHostClient::ProcessCurrentSnapshot");

    if (m_cSnapshotTemp.GetStateSequence() != GetStateSequence())
    {
        Console << _T("Dropping desync'd snapshot (Received state sequence ") << m_cSnapshotTemp.GetStateSequence() << _T(", expected ") << GetStateSequence() << newl;
        return true;
    }

    if (m_cSnapshotTemp.GetFrameNumber() < m_uiServerFrame)
    {
        Console << _T("Dropping old snapshot") << newl;
        return true;
    }

    if (m_eState == CLIENT_STATE_WAITING_FIRST_SNAPSHOT)
    {
        m_pktReliableSend << NETCMD_CLIENT_IN_GAME;
        m_eState = CLIENT_STATE_IN_GAME;
        m_auiStateStartTime[CLIENT_STATE_IN_GAME] = K2System.Microseconds();
        m_uiServerTimeout = INVALID_TIME;

        ChatManager.JoinGameLobby(false);
    }

    // Don't read snapshots if we're not in the proper state
    if (m_eState != CLIENT_STATE_IN_GAME)
        return true;

    // On the first frame from the server, synch up the client's time
    if (m_uiServerTimeStamp == 0)
    {
        m_uiLastGameTime = m_uiGameTime = m_cSnapshotTemp.GetTimeStamp();
        Console << _T("Received first server frame, synchronized time to: ") << m_uiGameTime << newl;
    }

#if 0
    // Adjust for server stalls
    int iServerFPS(GetStateString(STATE_STRING_SERVER_INFO)->GetInt(_T("svr_gameFPS")));
    uint uiMSperFrame(SecToMs(1.0f / iServerFPS));
    m_uiGameTime -= snapshot.GetDroppedFrameCount() * uiMSperFrame;
#endif

    m_uiPrevServerFrame = m_uiServerFrame;
    m_uiServerFrame = m_cSnapshotTemp.GetFrameNumber();
    m_uiPrevServerTimeStamp = m_uiServerTimeStamp;
    m_uiServerTimeStamp = m_cSnapshotTemp.GetTimeStamp();

    Console.SetDefaultStream(Console.ClientGame);
    bool bRet(m_pGameLib->ProcessSnapshot(m_cSnapshotTemp));
    Console.SetDefaultStream(Console.Client);

    if (!bRet)
        return true;

    if (m_cSnapshotTemp.GetPrevFrameNumber() != -1)
        m_uiLastAckedServerFrame = m_cSnapshotTemp.GetPrevFrameNumber();

    if (!Host.IsReplay() && cl_smoothGameTime)
    {
        // Snapshots processed this frame will have arrived sometime between
        // the last frame and this frame, so split the difference and use
        // that as the target time

        // Drift local client game time toward server time
        uint uiTargetTimeStamp(MIN(m_uiServerTimeStamp - INT_ROUND(MS_PER_SEC / float(m_uiServerFPS)) + (Host.GetFrameLength() / 2), m_uiServerTimeStamp));

        int iDelta(m_uiGameTime - uiTargetTimeStamp);

        if (ABS(iDelta) >= 500)
        {
            if (cl_debugGameTime)
                Console << _T("Reset     ") << XtoA(iDelta, FMT_SIGN) << newl;

            // Reset game time
            m_uiLastGameTime = m_uiGameTime = uiTargetTimeStamp;
        }
        else if (ABS(iDelta) >= 100)
        {
            if (cl_debugGameTime)
                Console << _T("Fast      ") << XtoA(iDelta, FMT_SIGN) << newl;

            // Reduce delta by 50%
            m_uiGameTime = uiTargetTimeStamp + (iDelta / 2);
        }
        else
        {
            // Slow drift Q3 style (+1/-2)
            if (m_uiGameTime >= uiTargetTimeStamp)
            {
                if (cl_debugGameTime)
                    Console << _T("Slow Near ") << XtoA(iDelta, FMT_SIGN) << newl;

                m_uiGameTime -= 2;
            }
            else
            {
                if (cl_debugGameTime)
                    Console << _T("Slow Far  ") << XtoA(iDelta, FMT_SIGN) << newl;

                m_uiGameTime += 1;
            }
        }
    }

    return true;
}


/*====================
  CHostClient::ReadSnapshot
  ====================*/
bool    CHostClient::ReadSnapshot(CPacket &pkt, bool bCompressed)
{
    PROFILE("CHostClient::ReadSnapshot");

    try
    {
        if (CHANCE(cl_simulatePacketLoss))
        {
            Console.Dev << _T("Snapshot packet dropped due to cl_simulatePacketLoss") << newl;
            pkt.Clear();
            return true;
        }

        uint uiReadSize(pkt.ReadInt());
        if (uiReadSize > pkt.GetUnreadLength())
            EX_ERROR(_T("Snapshot packet is truncated"));

        NetStats.RecordBytes(uiReadSize + 4, NETSAMPLE_SNAPSHOT, NETSOCKET_INCOMING);

        uint uiDataSize(uiReadSize);
        if (bCompressed)
            pkt >> uiDataSize;

        m_cBufferSnapshotTemp.Clear();

        if (bCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, uiDataSize));
            CZip::Decompress((const byte *)pkt.GetBuffer(), uiReadSize, pCompressed, uiDataSize);
            m_cBufferSnapshotTemp.Write(pCompressed, uiDataSize);
            SAFE_DELETE_ARRAY(pCompressed);
        }
        else
        {
            m_cBufferSnapshotTemp.Write(pkt.GetBuffer(), uiReadSize);
        }

        pkt.Advance(uiReadSize);

        return ReadCompleteSnapshot(m_cBufferSnapshotTemp);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadSnapshot() - "), NO_THROW);
        pkt.Clear();
        return false;
    }
}


/*====================
  CHostClient::AssembleSnapshot
  ====================*/
void    CHostClient::AssembleSnapshot(uint uiFrameNumber)
{
    try
    {
        SSnapshotFragmentCollector &fragments(m_mapSnapshotFragments[uiFrameNumber]);

        CBufferBit cBuffer;
        for (byte y(0); y <= fragments.yCount; ++y)
        {
            IBuffer *pFragmentBuffer(fragments.mapBuffers[y]);
            if (pFragmentBuffer == nullptr)
                EX_ERROR(_T("Missing a fragment that should be complete"));

            cBuffer << (*pFragmentBuffer);
        }

        Console.Dev << _T("Assembled snapshot for frame: ") << uiFrameNumber << _T(" from ")
                    << fragments.yCount + 1 << _T(" pieces.") << newl;

        if (fragments.bIsCompressed)
        {
            byte *pCompressed(K2_NEW_ARRAY(ctx_HostClient, byte, fragments.uiUncompressedLength));
            CZip::Decompress((const byte*)cBuffer.Get(), cBuffer.GetLength(), pCompressed, fragments.uiUncompressedLength);
            cBuffer.Write(pCompressed, fragments.uiUncompressedLength);
            SAFE_DELETE_ARRAY(pCompressed);
        }

        ReadCompleteSnapshot(cBuffer);

        DeleteSnapshotFragments(uiFrameNumber);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::AssembleSnapshot() - "), NO_THROW);
    }
}


/*====================
  CHostClient::DeleteSnapshotFragments
  ====================*/
void    CHostClient::DeleteSnapshotFragments(uint uiFrame)
{
    FragmentIndexMap_it it(m_mapSnapshotFragments.begin());
    while (it != m_mapSnapshotFragments.end())
    {
        if (it->first != uiFrame)
        {
            ++it;
            continue;
        }

        for (BufferMap_it itPieces(it->second.mapBuffers.begin()); itPieces != it->second.mapBuffers.end(); ++itPieces)
            SAFE_DELETE(itPieces->second);

        STL_ERASE(m_mapSnapshotFragments, it);
    }
}


/*====================
  CHostClient::CheckSnapshotFragments
  ====================*/
void    CHostClient::CheckSnapshotFragments()
{
    // Check for fragments that should be purged
    FragmentIndexMap_it it(m_mapSnapshotFragments.begin());
    while (it != m_mapSnapshotFragments.end())
    {
        if (it->first > m_uiServerFrame)
        {
            ++it;
            continue;
        }

        if (cl_debugSnapshot)
            Console.Dev << _T("Discarding fragments for frame: ") << it->first << newl;

        for (BufferMap_it itPieces(it->second.mapBuffers.begin()); itPieces != it->second.mapBuffers.end(); ++itPieces)
            SAFE_DELETE(itPieces->second);

        STL_ERASE(m_mapSnapshotFragments, it);
    }

    // Check for complete fragment collections
    for (FragmentIndexMap::iterator it(m_mapSnapshotFragments.begin()); it != m_mapSnapshotFragments.end(); ++it)
    {
        if (it->second.yCount > 0 && it->second.mapBuffers.size() == it->second.yCount + 1)
        {
            AssembleSnapshot(it->first);
            return;
        }
    }
}


/*==========================
  CHostClient::ReadSnapshotFragment
  ==========================*/
bool    CHostClient::ReadSnapshotFragment(CPacket &pkt)
{
    try
    {
        if (CHANCE(cl_simulatePacketLoss))
        {
            Console.Dev << _T("Snapshot packet dropped due to cl_simulatePacketLoss") << newl;
            return true;
        }

        // Read the fragment header info
        uint uiFragmentFrame(pkt.ReadInt());
        byte yFragmentIndex(pkt.ReadByte());

        // Read the fragment data into a buffer
        uint uiSize(pkt.GetUnreadLength());
        IBuffer *pBufferSnapshot(K2_NEW(ctx_HostClient,  CBufferDynamic)(uiSize));
        pBufferSnapshot->Write(pkt.GetBuffer(), uiSize);
        pkt.Advance(uiSize);

        // Store the buffer
        m_mapSnapshotFragments[uiFragmentFrame].mapBuffers[yFragmentIndex] = pBufferSnapshot;

        if (cl_debugSnapshot)
            Console.Dev << _T("Received snapshot fragment #") << yFragmentIndex << _T(" for frame number: ") << uiFragmentFrame << newl;

        CheckSnapshotFragments();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadSnapshotFragment() - "), NO_THROW);
        return false;
    }
}


/*====================
  CHostClient::ReadSnapshotTermination
  ====================*/
bool    CHostClient::ReadSnapshotTermination(CPacket &pkt, bool bCompressed)
{
    try
    {
        if (CHANCE(cl_simulatePacketLoss))
        {
            Console.Dev << _T("Snapshot packet dropped due to cl_simulatePacketLoss") << newl;
            return true;
        }

        // Read the fragment header info
        uint uiFragmentFrame(pkt.ReadInt());
        byte yFragmentIndex(pkt.ReadByte());
        uint uiUncompressedLength(0);
        if (bCompressed)
            pkt >> uiUncompressedLength;

        // Read the fragment data into a buffer
        ushort unSize(pkt.ReadShort());
        if (unSize > pkt.GetUnreadLength())
            EX_ERROR(_T("Snapshot termination packet is truncated"));

        IBuffer *pBufferSnapshot(K2_NEW(ctx_HostClient,  CBufferDynamic)(unSize));
        if (unSize > 0)
            pBufferSnapshot->Write(pkt.GetBuffer(), unSize);
        pkt.Advance(unSize);

        // Store the buffer
        m_mapSnapshotFragments[uiFragmentFrame].mapBuffers[yFragmentIndex] = pBufferSnapshot;
        m_mapSnapshotFragments[uiFragmentFrame].yCount = yFragmentIndex;
        m_mapSnapshotFragments[uiFragmentFrame].bIsCompressed = bCompressed;
        m_mapSnapshotFragments[uiFragmentFrame].uiUncompressedLength = uiUncompressedLength;

        if (cl_debugSnapshot)
            Console.Dev << _T("Received snapshot terminator #") << yFragmentIndex << _T(" for frame number: ") << uiFragmentFrame << newl;

        CheckSnapshotFragments();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadSnapshotTermination() - "), NO_THROW);
        return false;
    }
}


/*====================
  GetNetCmdString
  ====================*/
tstring GetNetCmdString(byte yCmd)
{
    switch(yCmd)
    {
    case NETCMD_START_STATE_DATA:
        return _T("NETCMD_START_STATE_DATA");

    case NETCMD_STATE_STRING:
        return _T("NETCMD_STATE_STRING");

    case NETCMD_COMPRESSED_STATE_STRING:
        return _T("NETCMD_COMPRESSED_STATE_STRING");

    case NETCMD_STATE_STRING_FRAGMENT:
        return _T("NETCMD_STATE_STRING_FRAGMENT");

    case NETCMD_STATE_STRING_TERMINATION:
        return _T("NETCMD_STATE_STRING_TERMINATION");

    case NETCMD_COMPRESSED_STATE_STRING_TERMINATION:
        return _T("NETCMD_COMPRESSED_STATE_STRING_TERMINATION");

    case NETCMD_END_STATE_DATA:
        return _T("NETCMD_END_STATE_DATA");

    case NETCMD_LOAD_WORLD:
        return _T("NETCMD_LOAD_WORLD");

    case NETCMD_SNAPSHOT:
        return _T("NETCMD_SNAPSHOT");

    case NETCMD_COMPRESSED_SNAPSHOT:
        return _T("NETCMD_COMPRESSED_SNAPSHOT");

    case NETCMD_SNAPSHOT_FRAGMENT:
        return _T("NETCMD_SNAPSHOT_FRAGMENT");

    case NETCMD_SNAPSHOT_TERMINATION:
        return _T("NETCMD_SNAPSHOT_TERMINATION");

    case NETCMD_COMPRESSED_SNAPSHOT_TERMINATION:
        return _T("NETCMD_COMPRESSED_SNAPSHOT_TERMINATION");

    case NETCMD_SERVER_GAME_DATA:
        return _T("NETCMD_SERVER_GAME_DATA");

    case NETCMD_KICK:
        return _T("NETCMD_KICK");

    case NETCMD_UPDATE_VOICE_CLIENT:
        return _T("NETCMD_UPDATE_VOICE_CLIENT");

    case NETCMD_NEW_VOICE_CLIENT:
        return _T("NETCMD_NEW_VOICE_CLIENT");

    case NETCMD_REMOVE_VOICE_CLIENT:
        return _T("NETCMD_REMOVE_VOICE_CLIENT");

    default:
        return _T("NETCMD_INVALID");
    }
}


/*====================
  CHostClient::ProcessPacket
  ====================*/
bool    CHostClient::ProcessPacket(CPacket &pkt)
{
    PROFILE("CHostClient::ProcessPacket");

    try
    {
        while (!pkt.DoneReading())
        {
            byte yCmd(pkt.ReadByte());
            switch (yCmd)
            {
            case NETCMD_SERVER_KEEP_ALIVE:
                m_bSentKeepAlive = false;
                break;

            case NETCMD_CONSOLE_MESSAGE:
                Console.Std << pkt.ReadString();
                break;

            case NETCMD_AUTH_OKAY:
                {
                    ushort unConnectionID(pkt.ReadShort());
                    int iClientNum(pkt.ReadInt());
                    byte yAuthFlags(pkt.ReadByte());
                    tstring sCompatVersion(pkt.ReadWStringAsTString());

                    FileManager.SetCompatVersion(sCompatVersion);

                    m_unConnectionID = unConnectionID;
                    m_iClientNum = iClientNum;
                    m_yAuthFlags = yAuthFlags;

                    Console.Client << _T("Connection accepted. Connection ID: ") << m_unConnectionID << _T(" Client number: ") << m_iClientNum << (m_bGameHost ? _T(" [Host]") : TSNULL) << newl;
                    m_sockGame.SetConnectionID(m_unConnectionID);
                    cl_connectionID = m_unConnectionID;
                    Host.SaveConfig();
                    m_bReceivedAuth = true;
                }
                break;

            case NETCMD_GAME_HOST:
                m_bGameHost = pkt.ReadByte(0) != 0;
                break;

            case NETCMD_START_LOADING:
                m_bStartLoading = true;
                break;

            case NETCMD_START_STATE_DATA:
                ResetStateData();
                break;

            case NETCMD_STATE_STRING:
            case NETCMD_COMPRESSED_STATE_STRING:
                if (!ReadStateStringUpdate(pkt, yCmd == NETCMD_COMPRESSED_STATE_STRING))
                    EX_ERROR(_T("Bad state string update"));
                break;

            case NETCMD_STATE_STRING_FRAGMENT:
                if (!ReadStateStringFragment(pkt))
                    EX_ERROR(_T("Bad state string fragment"));
                break;

            case NETCMD_STATE_STRING_TERMINATION:
            case NETCMD_COMPRESSED_STATE_STRING_TERMINATION:
                if (!ReadStateStringTermination(pkt, yCmd == NETCMD_COMPRESSED_STATE_STRING_TERMINATION))
                    EX_ERROR(_T("Bad state string termination"));
                break;

            case NETCMD_STATE_BLOCK:
            case NETCMD_COMPRESSED_STATE_BLOCK:
                if (!ReadStateBlockUpdate(pkt, yCmd == NETCMD_COMPRESSED_STATE_BLOCK))
                    EX_ERROR(_T("Bad state block update"));
                break;

            case NETCMD_STATE_BLOCK_FRAGMENT:
                if (!ReadStateBlockFragment(pkt))
                    EX_ERROR(_T("Bad state block fragment"));
                break;

            case NETCMD_STATE_BLOCK_TERMINATION:
            case NETCMD_COMPRESSED_STATE_BLOCK_TERMINATION:
                if (!ReadStateBlockTermination(pkt, yCmd == NETCMD_COMPRESSED_STATE_BLOCK_TERMINATION))
                    EX_ERROR(_T("Bad state block termination"));
                break;

            case NETCMD_END_STATE_DATA:
                if (!AllStateDataReceived())
                    EX_WARN(_T("Error in NETCMD_END_STATE_DATA packet"));
                break;

            case NETCMD_LOAD_WORLD:
                if (!StartLoadingWorld(pkt.ReadTString(), false))
                    EX_ERROR(_T("Could not process request to load world"));
                break;

            case NETCMD_SNAPSHOT:
            case NETCMD_COMPRESSED_SNAPSHOT:
                if (!ReadSnapshot(pkt, yCmd == NETCMD_COMPRESSED_SNAPSHOT))
                    EX_ERROR(_T("Bad snapshot"));
                break;

            case NETCMD_SNAPSHOT_FRAGMENT:
                if (!ReadSnapshotFragment(pkt))
                    EX_ERROR(_T("Bad snapshot fragment"));
                break;

            case NETCMD_SNAPSHOT_TERMINATION:
            case NETCMD_COMPRESSED_SNAPSHOT_TERMINATION:
                if (!ReadSnapshotTermination(pkt, yCmd == NETCMD_COMPRESSED_SNAPSHOT_TERMINATION))
                    EX_ERROR(_T("Bad snapshot termination"));
                break;

            case NETCMD_SERVER_GAME_DATA:
                {
                    uint uiStart(pkt.GetReadPos());

                    if (!m_pGameLib->ProcessGameData(pkt))
                        EX_ERROR(_T("Game client failed to handle a NETCMD_SERVER_GAME_DATA message"));

                    NetStats.RecordBytes(pkt.GetReadPos() - uiStart, NETSAMPLE_GAMEDATA, NETSOCKET_INCOMING);
                }
                break;

            case NETCMD_KICK:
                ProcessKickPacket(pkt);
                return false;

            case NETCMD_NEW_VOICE_CLIENT:
                {
                    uint uiClientID(pkt.ReadInt());
                    byte yVoiceID(pkt.ReadByte());

                    VoiceManager.AddClient(uiClientID, yVoiceID);
                }
                break;

            case NETCMD_UPDATE_VOICE_CLIENT:
                {
                    byte yVoiceID(pkt.ReadByte());
                    ushort unPort(pkt.ReadShort());
                    byte yNumUsers(pkt.ReadByte());

                    VoiceManager.Connect(yVoiceID, m_sockGame.GetSendAddrName(), unPort, GetClientNum());

                    for (int i(0); i < yNumUsers; i++)
                    {
                        uint uiClientID(pkt.ReadInt());
                        byte yVoiceID(pkt.ReadByte());

                        VoiceManager.AddClient(uiClientID, yVoiceID);
                    }
                }
                break;

            case NETCMD_REMOVE_VOICE_CLIENT:
                {
                    byte yVoiceID(pkt.ReadByte());
                    VoiceManager.RemoveClient(yVoiceID);
                }
                break;

            case NETCMD_REMOTE_START_LOADING:
                ProcessRemoteLoading(pkt);
                break;

            case NETCMD_SERVER_INVITE:
                {
                    tstring sName(pkt.ReadWStringAsTString());
                    ChatManager.SendServerInvite(sName);
                }
                break;

            default:
                EX_ERROR(_T("Invalid command received: ") + XtoA(yCmd, FMT_PADZERO, 4, 16));
                break;
            }
        }

        UpdateServerTimeout();
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::ReadServerPackets() - "), NO_THROW);
        if (cl_ignoreBadPackets)
        {
            Console.Warn << _T("Received a packet with bad data, discarding...") << newl;
            return true;
        }
        else
        {
            EX_ERROR(_T("Bad packet from server"));
        }
    }
}


/*====================
  CHostClient::ReadPacketsFromServer
  ====================*/
void    CHostClient::ReadPacketsFromServer()
{
    PROFILE("CHostClient::ReadPacketsFromServer");

    // Read the packets
    CPacket pkt;
    while (m_sockGame.ReceivePacket(pkt) > 0)
    {
        NetStats.RecordPacket(NETSOCKET_INCOMING, pkt.GetLength());

        if (!m_sockGame.PreProcessPacket(pkt))
            continue;

        if (!ProcessPacket(pkt))
            break;
    }

    if (m_uiLastSnapshotReceiveTime == Host.GetSystemTime())
        ProcessCurrentSnapshot();
}


/*====================
  CHostClient::PreFrame
  ====================*/
void    CHostClient::PreFrame()
{
    PROFILE("CHostClient::PreFrame");

    SetGamePointer();

    Console.SetDefaultStream(Console.ClientGame);
    m_pGameLib->PreFrame();
    Console.SetDefaultStream(Console.Client);
}


/*====================
  CHostClient::Frame
  ====================*/
void    CHostClient::Frame()
{
    PROFILE("CHostClient::Frame");

    SetGamePointer();

    m_uiClientFrameLength = Host.GetFrameLength();

    if (GetState() >= CLIENT_STATE_CONNECTED && m_uiServerTimeout == INVALID_TIME)
        m_uiServerTimeout = Host.GetSystemTime() + cl_timeout;

    // Correct game time for local server hitches
    if (Host.HasServer())
    {
        uint uiCorrection(Host.GetServer()->GetDroppedFrames() * Host.GetServer()->GetFrameLength());

        if (uiCorrection >= m_uiClientFrameLength)
            m_uiClientFrameLength = 1;
        else
            m_uiClientFrameLength -= uiCorrection;

        Host.GetServer()->ResetDroppedFrames();
    }

    // Advance game time when not paused (always advance in replays)
    if ((m_uiServerTimeStamp != m_uiPrevServerTimeStamp || m_uiServerTimeStamp == 0) || Host.IsReplay())
        m_uiGameTime += m_uiClientFrameLength;

    _cl_ms = m_uiGameTime;

    if (_cl_nudgegametime)
    {
        m_uiGameTime += _cl_nudgegametime;
        m_uiLastGameTime += _cl_nudgegametime;
        _cl_nudgegametime = 0;
    }

    if (m_ClientAccount.IsLoggedIn() && GetState() == CLIENT_STATE_IDLE)
    {
        if (m_uiLastUpdateCheck == INVALID_TIME || K2System.Milliseconds() - m_uiLastUpdateCheck >= 300000)
        {
            SilentUpdate();
            m_uiLastUpdateCheck = K2System.Milliseconds();
        }
    }

    m_ClientAccount.Frame();
    UpdateNetSettings();

    UpdateCookie();

    m_ServerList.Frame();

    CheckAutoStartGame();

    m_sockGame.SetBlockIncoming(BlockIncoming);
    m_sockGame.SetBlockOutgoing(BlockOutgoing);

    if (GetState() > CLIENT_STATE_IDLE)
        ReadPacketsFromServer();

    switch (GetState())
    {
    case CLIENT_STATE_IDLE:
        break;

    case CLIENT_STATE_PRELOADING:
        PreloadingFrame();
        break;

    case CLIENT_STATE_CONNECTING:
        if (m_bReceivedAuth && m_bReceivedStateData)
        {
            m_eState = CLIENT_STATE_CONNECTED;
            m_auiStateStartTime[CLIENT_STATE_CONNECTED] = K2System.Microseconds();
        }
        break;

    case CLIENT_STATE_CONNECTED:
        if (!m_bSentKeepAlive)
        {
            m_bSentKeepAlive = true;
            m_pktReliableSend << NETCMD_CLIENT_KEEP_ALIVE;
        }

        if (m_bStartLoading && !cl_testFailLoad)
        {
            m_eState = CLIENT_STATE_LOADING;
            m_auiStateStartTime[CLIENT_STATE_LOADING] = K2System.Microseconds();

            ChatManager.LeaveMatchChannels();

            if (!m_bPractice)
                ChatManager.JoiningGame(Host.GetConnectedAddress());
        }
        break;

    case CLIENT_STATE_LOADING:
        LoadingFrame();
        break;

    case CLIENT_STATE_WAITING_FIRST_SNAPSHOT:
        break;

    case CLIENT_STATE_IN_GAME:
        _cl_delta = m_uiGameTime - m_uiPrevServerTimeStamp;
        _cl_lerp = GetLerpValue();
        break;

    case NUM_CLIENT_STATES:
        K2_UNREACHABLE();
        break;
    }

    _cl_state = XtoA(m_eState);

    if (m_uiServerTimeStamp && m_eState == CLIENT_STATE_IN_GAME)
    {
        if (Host.IsReplay() || !cl_smoothGameTime)
        {
            if (m_uiGameTime < m_uiPrevServerTimeStamp)
            {
                //Console.Warn << "HostClient GameTime < PrevServerTimeStamp " << ParenStr(XtoA(int(m_uiGameTime - m_uiPrevServerTimeStamp))) << newl;
                m_uiGameTime = m_uiPrevServerTimeStamp;
            }
            else if (m_uiGameTime >= m_uiServerTimeStamp)
            {
                //Console.Warn << "HostClient GameTime > ServerTimeStamp " << ParenStr(XtoA(int(m_uiServerTimeStamp - m_uiGameTime))) << newl;
                m_uiGameTime = m_uiServerTimeStamp - 1;
            }

            m_uiLastGameTime = m_uiGameTime;
            m_uiClientFrameLength = Host.GetFrameLength();
        }
        else
        {
            // Don't let game time go backwards in regular play
            if (m_uiGameTime < m_uiLastGameTime)
                m_uiGameTime = m_uiLastGameTime;

            m_uiClientFrameLength = m_uiGameTime - m_uiLastGameTime;
            m_uiLastGameTime = m_uiGameTime;
        }
    }

    Console.SetDefaultStream(Console.ClientGame);
    m_pGameLib->Frame();
    Console.SetDefaultStream(Console.Client);

    if (!m_bWasIdle)
        SendPackets();

    CheckTimeout();

    ++m_uiFrameCount;
}


/*====================
  CHostClient::Connect
  ====================*/
void    CHostClient::Connect(const tstring &sAddr, bool bSilent, bool bPractice, const tstring &sLoadingInterface)
{
    PROFILE("CHostClient::Connect");

    try
    {
        // Close any existing connection
        Disconnect(_T("CHostClient::Connect"));

        if (cl_testFailConnect)
            return;

        // Create a new connection
        if (!m_sockGame.Init(K2_SOCKET_GAME))
            EX_ERROR(_T("Couldn't initialize socket"));

        tstring sFullAddr(sAddr);
        if (sFullAddr.find(_T(':')) == tstring::npos)
            sFullAddr += _T(":") + XtoA(DEFAULT_SERVER_PORT);
        if (!m_sockGame.SetSendAddr(sFullAddr))
            EX_ERROR(_T("Failed to set the send address"));

        if (!bSilent)
            IModalDialog::Show(sLoadingInterface);

        m_bSilentConnect = bSilent;
        m_bPractice = bPractice;
        m_sLoadingInterface = sLoadingInterface;
        m_bStartedLoadingWorld = false;

        for (int i(0); i < NUM_CLIENT_STATES; ++i)
            m_auiStateStartTime[i] = INVALID_TIME;

        // Start connecting to server
        m_eState = CLIENT_STATE_CONNECTING;
        m_auiStateStartTime[CLIENT_STATE_CONNECTING] = K2System.Microseconds();

        byte yFlags(0);
        if (bSilent)
            yFlags |= BIT(0);
        if (bPractice)
            yFlags |= BIT(1);

        CPacket pktConnect;
        pktConnect <<
            NETCMD_CONNECT <<               // Connect command
            K2System.GetGameName() <<       // Game module name
            K2System.GetVersionString() <<  // Game version
            Host.GetID() <<                 // Host ID (to verify local clients)
            GetConnectionID() <<            // Connection ID
            cl_password <<                  // Server password (can grant user, referee or admin access)
            GetNetName() <<                 // Name (account name will override this on official servers)
            m_ClientAccount.GetCookie() <<  // Cookie (from master server, allows server to authenticate client)
            m_ClientAccount.GetIP() <<      // IP as reported by master server
            m_ServerList.GetMatchKey() <<   // Match Key (can grant a user host priveledge on an official server)
            "" <<                           // Invitation code (allows access to private servers)
            yFlags <<
            net_maxPacketSize <<            // Network settings
            net_maxBPS <<
            net_FPS;

        m_sockGame.SendPacket(pktConnect);

        m_sockGame.ResetSeq();
        m_sockGame.ClearReliablePackets();

        m_yStateSequence = 0;
        m_uiServerTimeout = Host.GetSystemTime() + cl_timeout;
        m_uiServerFPS = 20;

        SetInviteAddress(TSNULL);
        SetInviteGameName(TSNULL);

        m_pGameLib->Connect(sFullAddr);

        ChatManager.AddGameChatMessage(CHAT_MESSAGE_CLEAR);

        Console.Client << _T("Attempting connection to ") << sAddr << _T("...") << newl
            << _T("Game: ") << K2System.GetGameName() << newl
            << _T("Version: ") << K2System.GetVersionString() << newl
            << _T("Host ID: ") << Host.GetID() << newl
            << _T("Connection ID: ") << m_unConnectionID << newl
            << _T("Password: ") << cl_password << newl
            << _T("Name: ") << net_name << newl
            << _T("Cookie: ") << m_ClientAccount.GetCookie() << newl
            << _T("IP: ") << m_ClientAccount.GetIP() << newl
            << _T("Match Key: ") << m_ServerList.GetMatchKey() << newl
            << _T("Invitation: ") << "" << newl
            << _T("Max packet size: ") << net_maxPacketSize << newl
            << _T("Max bps: ") << net_maxBPS << newl
            << _T("Net FPS: ") << net_FPS << newl;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::Connect() - "), NO_THROW);
    }
}


/*====================
  CHostClient::Disconnect
  ====================*/
void    CHostClient::Disconnect(const tstring &sReason)
{
    if (!sReason.empty())
        Console << _T("Disconnect - ") << sReason << newl;

    try
    {
        IModalDialog::Hide();

        // If there is an active connection, notify the server of the disconnection
        if (GetState() != CLIENT_STATE_IDLE)
        {
            CPacket pkt;
            while (m_sockGame.ReceivePacket(pkt) > 0);

            Console.Client << _T("Sending disconnect notice...") << newl;
            pkt.Clear();
            pkt << NETCMD_CLIENT_DISCONNECT;
            m_sockGame.SendPacket(pkt);

            // Notify IRC we're no longer in a game
            ChatManager.LeftGame();

            ChatManager.LeaveGameLobby();
        }

        if (m_pGameLib != nullptr)
            m_pGameLib->Reinitialize();

        m_eState = CLIENT_STATE_IDLE;
        m_uiLastGameTime = 0;
        m_uiGameTime = 0;

        m_iClientNum = -1;
        m_uiLastSendTime = INVALID_TIME;
        m_uiServerTimeout = INVALID_TIME;
        m_uiServerFPS = 20;

        m_uiServerFrame = 0;
        m_uiServerTimeStamp = 0;
        m_uiPrevServerTimeStamp = 0;
        m_bReadyToSendSnapshot = false;

        m_bReceivedStateData = false;
        m_bReceivedAuth = false;
        m_bStartLoading = false;
        m_bSentKeepAlive = false;
        m_bGameHost = false;
        m_yAuthFlags = 0;

        m_pktSend.Clear();
        m_pktReliableSend.Clear();

        m_sockGame.Close();

        if (m_pWorld != nullptr && !m_pWorld->IsPreloaded())
            m_pWorld->Free();

        UIManager.UnloadTempInterfaces();

        ResetStateData();

        m_bStartedLoadingNetworkResources = false;
        m_deqNetworkResources.clear();
        m_uiNetworkResourceCount = 0;

        VoiceManager.Disconnect();

        m_bAutoStartGame = false;

        RemoteLoading.Trigger(XtoA(false));

        ChatManager.ResetGame();

        m_bStartedLoadingWorld = false;

        // Move prev back to current if loading was canceled
        if (!g_ResourceInfo.WasGameContextCategoryLoaded(_T("world")))
        {
            if (g_ResourceInfo.LookupContext(_T("client:prevgame_world"), false) != nullptr)
                g_ResourceInfo.ExecCommandLine(_T("context move client:prevgame_world client:curgame_world"));
            else if (g_ResourceInfo.LookupContext(_T("client:curgame_world"), false) != nullptr)
                g_ResourceInfo.ExecCommandLine(_T("context delete client:curgame_world"));

            g_ResourceInfo.SetGameContextCategoryLoaded(_T("world"), false);
        }

        if (!g_ResourceInfo.WasGameContextCategoryLoaded(_T("heroes")))
        {
            if (g_ResourceInfo.LookupContext(_T("client:prevgame_heroes"), false) != nullptr)
                g_ResourceInfo.ExecCommandLine(_T("context move client:prevgame_heroes client:curgame_heroes"));
            else if (g_ResourceInfo.LookupContext(_T("client:curgame_heroes"), false) != nullptr)
                g_ResourceInfo.ExecCommandLine(_T("context delete client:curgame_heroes"));

            g_ResourceInfo.SetGameContextCategoryLoaded(_T("heroes"), false);
        }

        g_ResourceInfo.ExecCommandLine(_T("orphans unregister"));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostClient::Disconnect() - "));
    }
}


/*====================
  CHostClient::LoadAllResources
  ====================*/
void    CHostClient::LoadAllResources()
{
    m_pGameLib->LoadAllResources();
}


/*====================
  CHostClient::LoadWorldResources
  ====================*/
void    CHostClient::LoadWorldResources(const tstring &sWorldName)
{
    if (m_eState == CLIENT_STATE_IDLE)
    {
        m_bWasIdle = true;
    }
    else
    {
        m_bWasIdle = false;
    }

    StartLoadingWorld(sWorldName, false);
    m_eState = CLIENT_STATE_LOADING;

    Console << _T("CLIENT_STATE_LOADING") << newl;

    if (m_auiStateStartTime[CLIENT_STATE_LOADING] == INVALID_TIME)
        m_auiStateStartTime[CLIENT_STATE_LOADING] = K2System.Microseconds();
}


/*====================
  CHostClient::PreloadWorldResources
  ====================*/
void    CHostClient::PreloadWorldResources(const tstring &sWorldName)
{
    StartLoadingWorld(sWorldName, true);
    m_eState = CLIENT_STATE_PRELOADING;

    if (m_auiStateStartTime[CLIENT_STATE_PRELOADING] == INVALID_TIME)
        m_auiStateStartTime[CLIENT_STATE_PRELOADING] = K2System.Microseconds();
}


/*====================
  CHostClient::ProcessKickPacket
  ====================*/
void    CHostClient::ProcessKickPacket(CPacket &pkt)
{
    static tsvector vMessage(3);
    tstring sReason(pkt.ReadWStringAsTString());

    // MikeG check for trial dc and pop up the gui to purchase
    if (sReason == _T("disconnect_trial_expired"))
    {
        m_ClientAccount.SetTrialExpired();
        Console << _T("Disconnected: ") << sRed << sReason << newl;
        static tsvector vAccountInfo(12);
        vAccountInfo[0] = XtoA(m_ClientAccount.GetAccountID());
        vAccountInfo[1] = m_ClientAccount.GetNickname();
        vAccountInfo[2] = XtoA(m_ClientAccount.GetAccountType());
        vAccountInfo[3] = XtoA(m_ClientAccount.GetLevel());
        vAccountInfo[4] = XtoA(m_ClientAccount.GetGames());
        vAccountInfo[5] = XtoA(m_ClientAccount.GetDisconnects());
        vAccountInfo[6] = XtoA(m_ClientAccount.IsLeaver());
        vAccountInfo[7] = XtoA(m_ClientAccount.WillBeLeaver());
        vAccountInfo[8] = XtoA(m_ClientAccount.GetLeaverPercent());
        vAccountInfo[9] = XtoA(m_ClientAccount.GetNextLeaverPercent());
        vAccountInfo[10] = XtoA(m_ClientAccount.GetNextLeaverThreshold());
        vAccountInfo[11] = XtoA(m_ClientAccount.IsTrialExpired());
        AccountInfo.Trigger(vAccountInfo, true);
    }
    else
    {
        vMessage[0] = _T("disconnect_title");
        vMessage[1] = sReason.empty() ? _T("disconnect_unknown") : sReason;
        
        if (sReason == _T("disconnect_duplicate_login"))
            vMessage[2] = XtoA(true);
        else
            vMessage[2] = XtoA(false);

        if (sReason == _T("disconnect_duplicate_login"))
            vMessage[2] = XtoA(true);
        else
            vMessage[2] = XtoA(false);

        CStringTable *pClientMessages(g_ResourceManager.GetStringTable(m_hClientMessages));
        if (pClientMessages != nullptr)
        {
            vMessage[0] = pClientMessages->Get(vMessage[0]);
            vMessage[1] = pClientMessages->Get(vMessage[1]);
        }

        Console << _T("Disconnected: ") << sRed << sReason << newl;
        HostErrorMessage.Trigger(vMessage);
    }

    Disconnect(_T("Kicked"));
}


/*====================
  CHostClient::ProcessRemoteLoading
  ====================*/
void    CHostClient::ProcessRemoteLoading(CPacket &pkt)
{
    RemoteLoading.Trigger(XtoA(true));
}


/*====================
  CHostClient::GameError
  ====================*/
void    CHostClient::GameError(const tstring &sError)
{
    static tsvector vMessage(2);

    vMessage[0] = _T("error_title");
    vMessage[1] = sError.empty() ? _T("error_unknown") : sError;

    CStringTable *pClientMessages(g_ResourceManager.GetStringTable(m_hClientMessages));
    if (pClientMessages != nullptr)
    {
        vMessage[0] = pClientMessages->Get(vMessage[0]);
        vMessage[1] = pClientMessages->Get(vMessage[1]);
    }

    Console << _T("Error: ") << sRed << vMessage[1] << newl;
    HostErrorMessage.Trigger(vMessage);

    Disconnect(_T("CHostClient::GameError"));
}


/*====================
  CHostClient::SendClientSnapshot
  ====================*/
void    CHostClient::SendClientSnapshot(const IBuffer &buffer)
{
    m_bReadyToSendSnapshot = false;

    PROFILE("CHostClient::SendClientSnapshot");
    m_pktSend << NETCMD_CLIENT_SNAPSHOT << buffer;
}


/*====================
  CHostClient::SendGameData
  ====================*/
void    CHostClient::SendGameData(const IBuffer &buffer, bool bReliable)
{
    if (bReliable)
        m_pktReliableSend << NETCMD_CLIENT_GAME_DATA << buffer;
    else
        m_pktSend << NETCMD_CLIENT_GAME_DATA << buffer;
}


/*====================
  CHostClient::SendServerPrivateValue
  ====================*/
void    CHostClient::SendServerPrivateValue(byte yValue)
{
    m_pktReliableSend << NETCMD_SET_PRIVATE << yValue;
}


/*====================
  CHostClient::SendRemoteCommand
  ====================*/
void    CHostClient::SendRemoteCommand(const tstring &sCommand)
{
    if (!sCommand.empty())
        m_pktReliableSend << NETCMD_CLIENT_REMOTE_COMMAND << sCommand;
}


/*====================
  CHostClient::SendPackets
  ====================*/
void    CHostClient::SendPackets()
{
    PROFILE("CHostClient::SendPackets");

    if (GetState() == CLIENT_STATE_IDLE || GetState() == CLIENT_STATE_PRELOADING)
        return;

    // Rate limiter
    uint uiPacketMS(SecToMs(1.0f / cl_packetSendFPS));
    if (m_uiLastSendTime != INVALID_TIME && Host.GetSystemTime() - m_uiLastSendTime < uiPacketMS)
        return;

    m_bReadyToSendSnapshot = true;

    m_sockGame.CheckPacketTimeouts();

    if (m_pktSend.GetLength() == 0)
        m_pktSend << NETCMD_CLIENT_HEARTBEAT;

    if (m_pktSend.GetLength() > 0)
    {
        if (cl_showPackets)
            Console.Net << _T("PACKET: ") << m_pktSend.GetLength() << _T(" bytes") << newl;

        NetStats.RecordPacket(NETSOCKET_OUTGOING, m_pktSend.GetLength());

        m_sockGame.SendPacket(m_pktSend);

        m_pktSend.Clear();
    }

    if (m_pktReliableSend.GetLength() > 0)
    {
        if (cl_showPackets)
            Console.Net << _T("RELIABLE: ") << ParenStr(XtoA(m_pktReliableSend.GetLength())) << _T(" bytes") << newl;

        NetStats.RecordPacket(NETSOCKET_OUTGOING, m_pktReliableSend.GetLength());

        m_sockGame.SendReliablePacket(m_pktReliableSend, false);

        m_pktReliableSend.Clear();
    }

    m_uiLastSendTime = Host.GetSystemTime() / uiPacketMS * uiPacketMS; // Round down to the nearest uiPacketMS (acts like an accumulator)
}


/*====================
  CHostClient::UpdateCookie
  ====================*/
void    CHostClient::UpdateCookie()
{
    if (GetState() < CLIENT_STATE_LOADING)
        return;

    if (GetCookie() != m_sOldCookie && GetCookie() != TSNULL)
    {
        m_sOldCookie = GetCookie();
        m_pktReliableSend << NETCMD_CLIENT_COOKIE << m_ClientAccount.GetCookie();   // Send New Cookie
    }
}


/*====================
  CHostClient::UpdateNetSettings
  ====================*/
void    CHostClient::UpdateNetSettings()
{
    if (GetState() < CLIENT_STATE_CONNECTED)
        return;

    if (net_maxPacketSize.IsModified() || net_maxBPS.IsModified() || net_FPS.IsModified())
    {
        net_maxPacketSize.SetModified(false);
        net_maxBPS.SetModified(false);
        net_FPS.SetModified(false);
        m_pktSend << NETCMD_CLIENT_NET_SETTINGS << net_maxPacketSize << net_maxBPS << net_FPS;
    }
}


/*====================
  CHostClient::FileDropNotify
  ====================*/
void    CHostClient::FileDropNotify(const tsvector &vsFiles)
{
    if (m_pGameLib != nullptr)
        m_pGameLib->FileDropNotify(vsFiles);
}


/*====================
  CHostClient::UpdateServerTimeout
  ====================*/
void    CHostClient::UpdateServerTimeout(uint uiLength)
{
    if (uiLength == INVALID_TIME)
        uiLength = cl_timeout;

    m_uiServerTimeout = MAX(Host.GetSystemTime() + uiLength, m_uiServerTimeout);
}


/*====================
  CHostClient::GetConnectedAddress
  ====================*/
tstring CHostClient::GetConnectedAddress()
{
    if (GetState() == CLIENT_STATE_IDLE)
        return _T("");

    return (m_sockGame.GetSendAddrName() + _T(":") + XtoA(m_sockGame.GetSendPort()));
}


/*====================
  CHostClient::CheckReconnect
  ====================*/
void    CHostClient::CheckReconnect(const tstring &sAddress, uint uiMatchID)
{
    m_ServerList.CheckReconnectStatus(sAddress, uiMatchID, GetAccountID(), GetConnectionID());
}

void    CHostClient::CheckReconnect()
{
    if (m_sReconnectIP.empty() || m_uiReconnectMatchID == -1 || GetAccountID() == -1)
        return;

    m_ServerList.CheckReconnectStatus(m_sReconnectIP, m_uiReconnectMatchID, GetAccountID(), GetConnectionID());
}


/*====================
  CHostClient::UpdateAvailable
  ====================*/
void    CHostClient::UpdateAvailable(const tstring &sVersion)
{
    if (GetState() != CLIENT_STATE_IDLE)
        return;

    if (cc_showNewPatchNotification)
        ChatManager.PushNotification(NOTIFY_TYPE_UPDATE, sVersion);
}


/*====================
  CHostClient::SilentUpdate
  ====================*/
void    CHostClient::SilentUpdate()
{
    K2Updater.SilentUpdate();
}


/*====================
  CHostClient::CheckTimeout
  ====================*/
void    CHostClient::CheckTimeout()
{
    if (GetState() == CLIENT_STATE_IDLE)
        return;

    if (m_uiServerTimeout == INVALID_TIME)
        return;

    if (Host.GetSystemTime() < m_uiServerTimeout)
        return;

    Console.Client << _T("Server timed out") << newl;

    GameError(_T("disconnect_timeout"));
}


/*====================
  CHostClient::RequestLocalServerList
  ====================*/
void    CHostClient::RequestLocalServerList(EGameListType eList)
{
    m_ServerList.RequestLocalServerList(eList);
}


/*====================
  CHostClient::RequestMasterServerList
  ====================*/
void    CHostClient::RequestMasterServerList(const tstring &sQueryType, EGameListType eList, const tstring &sRegion)
{
    m_ServerList.RequestMasterServerList(m_ClientAccount.GetCookie(), sQueryType, eList, sRegion);
}


/*====================
  CHostClient::CancelServerList
  ====================*/
void    CHostClient::CancelServerList()
{
    m_ServerList.CancelServerList();
}

/*====================
  CHostClient::ClearServerList
  ====================*/
void    CHostClient::ClearServerList()
{
    m_ServerList.ClearServerList();
}

/*====================
  CHostClient::GetServerListCount
  ====================*/
uint    CHostClient::GetServerListCount() const
{
    return m_ServerList.GetServerListCount();
}


/*====================
  CHostClient::RequestMasterServerList
  ====================*/
bool    CHostClient::IsLoading() const
{
    return m_eState == CLIENT_STATE_LOADING || (m_pWorld != nullptr && m_pWorld->IsLoading());
}


/*====================
  CHostClient::StartAutoLocalGame
  ====================*/
void    CHostClient::StartAutoLocalGame(const tstring &sName, const tstring &sOptions)
{
    m_bAutoStartGame = true;
    m_bAutoStartGameLocal = true;
    m_sAutoStartName = sName;
    m_sAutoStartOptions = sOptions;

    m_ServerList.Clear();
    RequestLocalServerList(LIST_SERVER);
}


/*====================
  CHostClient::StartAutoRemoteGame
  ====================*/
void    CHostClient::StartAutoRemoteGame(const tstring &sName, const tstring &sOptions)
{
    m_bAutoStartGame = true;
    m_bAutoStartGameLocal = false;
    m_sAutoStartName = sName;
    m_sAutoStartOptions = sOptions;

    tstring sRegion(_T(""));

    // if they selected "Automatic" server selection, attempt to parse their preferred region settings
    if (m_sAutoStartOptions.find(_T("region:")) != tstring::npos)
    {
        sRegion = sOptions.substr(sOptions.find(_T("region:")), 11);
        const size_t zColon(sRegion.find(_T(":")));
        const size_t zSpace(sRegion.find(_T(" ")));

        // check to see if the region sent was null, if so skip setting the region
        if (zSpace == zColon + 1)
            sRegion = _T("");
        else
            sRegion = sRegion.substr(zColon + 1, zSpace - zColon - 1);
    }

    m_ServerList.Clear();
    RequestMasterServerList(_T("90"), LIST_SERVER, sRegion);
}


/*====================
  CHostClient::CheckAutoStartGame
  ====================*/
void    CHostClient::CheckAutoStartGame()
{
    if (!m_bAutoStartGame)
        return;

    if (GetState() == CLIENT_STATE_IDLE)
    {
        const tstring &sBestAddress(m_bAutoStartGameLocal ? m_ServerList.GetBestLocalServer() : m_ServerList.GetBestServer());
        if (!sBestAddress.empty())
        {
            Host.Connect(sBestAddress, true);

            m_bAutoStartGame = true; // Connect/Disconnect clear this
        }
        else if (!(m_bAutoStartGameLocal ? m_ServerList.AwaitingLocalResponses() : m_ServerList.AwaitingResponses()))
        {
            // Cancel this request
            m_bAutoStartGame = false;
        }
    }
    else if (GetState() == CLIENT_STATE_CONNECTED)
    {
        m_pGameLib->SendCreateGameRequest(m_sAutoStartName, m_sAutoStartOptions);
        m_bAutoStartGame = false;
    }
}


/*====================
  CHostClient::LoadingStep
  ====================*/
float   CHostClient::LoadingStep()
{
    float fProgress(0.0f);
    bool bFinishedLoading(false);

    int iCurrentLoadingStep(0);
    const int LOADING_STEPS(5);

    if (m_bStartedLoadingWorld)
    {
        if (m_pWorld->IsPreloaded())
        {
            m_pWorld->ClearPreloaded();

            m_uiServerTimeout = INVALID_TIME;

            StartLoadingNetworkResources();
            IModalDialog::NextLoadingJob();

            if (FinishedLoadingNetworkResources())
            {
                m_pGameLib->StartLoadingResources();
                IModalDialog::NextLoadingJob();
                return float(iCurrentLoadingStep + 2.0f) / LOADING_STEPS;
            }

            return float(iCurrentLoadingStep + 1.0f) / LOADING_STEPS;
        }

        if (m_pWorld->IsLoading())
        {
            IModalDialog::SetProgress(m_pWorld->GetLoadProgress());
            IModalDialog::Update();

            m_uiServerTimeout = INVALID_TIME;

            if (!m_pWorld->LoadNextComponent())
            {
                m_pWorld->Free();
                IModalDialog::Hide();
                return 0.0f;
            }

            if (m_pWorld->IsLoaded())
            {
                StartLoadingNetworkResources();
                IModalDialog::NextLoadingJob();

                if (FinishedLoadingNetworkResources())
                {
                    m_pGameLib->StartLoadingResources();
                    IModalDialog::NextLoadingJob();
                    return float(iCurrentLoadingStep + 2.0f) / LOADING_STEPS;
                }

                return float(iCurrentLoadingStep + 1.0f) / LOADING_STEPS;
            }

            fProgress = float(iCurrentLoadingStep + m_pWorld->GetLoadProgress()) / LOADING_STEPS;
        }
    }

    ++iCurrentLoadingStep;

    // Load client resources after the world has finished loading
    if (m_bStartedLoadingNetworkResources && !FinishedLoadingNetworkResources())
    {
        // Load resources now that the world is fully loaded, so that any resources
        // contained in the world archive will override the base resources
        LoadNextNetworkResource();

        IModalDialog::SetProgress(GetNetworkResourceLoadingProgress());
        IModalDialog::Update();

        if (FinishedLoadingNetworkResources())
        {
            m_pGameLib->StartLoadingResources();
            IModalDialog::NextLoadingJob();
        }

        fProgress = float(iCurrentLoadingStep + GetNetworkResourceLoadingProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (FinishedLoadingNetworkResources() && !m_pGameLib->IsFinishedLoadingResources())
    {
        m_pGameLib->LoadNextResource();

        IModalDialog::SetProgress(m_pGameLib->GetResourceLoadingProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedLoadingResources())
        {
            if (m_bWasIdle)
                bFinishedLoading = true;
            else
                m_pGameLib->StartLoadingWorld();

            IModalDialog::NextLoadingJob();
        }

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetResourceLoadingProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (m_pGameLib->IsFinishedLoadingResources() && m_pGameLib->IsSpawningEntities() && !m_pGameLib->IsFinishedSpawningEntities())
    {
        // Let the game process the fully loaded world so it can do things like spawn entities
        m_pGameLib->SpawnNextWorldEntity();

        IModalDialog::SetProgress(m_pGameLib->GetEntitySpawningProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedSpawningEntities())
        {
            m_pGameLib->StartLoadingEntityResources();
            IModalDialog::NextLoadingJob();
        }

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetEntitySpawningProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (m_pGameLib->IsFinishedSpawningEntities() && !m_pGameLib->IsFinishedLoadingEntityResources())
    {
        m_pGameLib->LoadNextEntityResource();

        IModalDialog::SetProgress(m_pGameLib->GetEntityResourceLoadingProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedLoadingEntityResources())
            bFinishedLoading = true;

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetEntityResourceLoadingProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (bFinishedLoading)
    {
        // If the client is connected to a server, notify it that we are ready to start receiving snapshots
        if (m_eState > CLIENT_STATE_IDLE)
        {
            if (m_bWasIdle)
            {
                m_eState = CLIENT_STATE_IDLE;
            }
            else
            {
                m_pktReliableSend << NETCMD_FINISHED_LOADING_WORLD;
                m_eState = CLIENT_STATE_WAITING_FIRST_SNAPSHOT;
                m_auiStateStartTime[CLIENT_STATE_WAITING_FIRST_SNAPSHOT] = K2System.Microseconds();
            }
            m_bWasIdle = false;
        }

        m_mapSnapshotFragments.clear();

        Console.Client << _T("Client loaded world ") << QuoteStr(m_pWorld->GetName()) << newl;

        m_pGameLib->FinishedLoadingWorld();

        IModalDialog::Hide();
        fProgress = 1.0f;
    }
    else
    {
        fProgress = MIN(fProgress, 0.99f);
    }

    return fProgress;
}


/*====================
  CHostClient::LoadingFrame
  ====================*/
void    CHostClient::LoadingFrame()
{
    assert(m_pGameLib != nullptr);

    // Load the next world component if any are pending
    if (m_pWorld == nullptr)
        return;

    m_uiServerTimeout = INVALID_TIME;

    float fProgress(0.0f);

    uint uiEndTime(K2System.Milliseconds() + 50);
    do
    {
        fProgress = LoadingStep();
    }
    while (K2System.Milliseconds() < uiEndTime && fProgress != 1.0f);

    m_pktSend << NETCMD_LOADING_PROGRESS << fProgress;
}


/*====================
  CHostClient::PreloadingStep
  ====================*/
float   CHostClient::PreloadingStep()
{
    float fProgress(0.0f);
    bool bFinishedPreloading(false);

    int iCurrentLoadingStep(0);
    const int LOADING_STEPS(4);

    if (m_pWorld->IsLoading())
    {
        IModalDialog::SetProgress(m_pWorld->GetLoadProgress());
        IModalDialog::Update();

        if (!m_pWorld->LoadNextComponent())
        {
            m_pWorld->Free();
            IModalDialog::Hide();
            return 0.0f;
        }

        if (m_pWorld->IsLoaded())
        {
            m_pGameLib->StartLoadingResources();
            IModalDialog::NextLoadingJob();
            return float(iCurrentLoadingStep + 1.0f) / LOADING_STEPS;
        }

        fProgress = float(iCurrentLoadingStep + m_pWorld->GetLoadProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;
    
    if (m_pWorld->IsLoaded() && !m_pGameLib->IsFinishedLoadingResources())
    {
        m_pGameLib->LoadNextResource();

        IModalDialog::SetProgress(m_pGameLib->GetResourceLoadingProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedLoadingResources())
        {
            m_pGameLib->StartPreloadingWorld();
            IModalDialog::NextLoadingJob();
        }

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetResourceLoadingProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (m_pGameLib->IsFinishedLoadingResources() && m_pGameLib->IsSpawningEntities() && !m_pGameLib->IsFinishedSpawningEntities())
    {
        // Let the game process the fully loaded world so it can do things like spawn entities
        m_pGameLib->PrecacheNextWorldEntity();

        IModalDialog::SetProgress(m_pGameLib->GetEntitySpawningProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedSpawningEntities())
        {
            m_pGameLib->StartLoadingEntityResources();
            IModalDialog::NextLoadingJob();
        }

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetEntitySpawningProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (m_pGameLib->IsFinishedSpawningEntities() && !m_pGameLib->IsFinishedLoadingEntityResources())
    {
        m_pGameLib->LoadNextEntityResource();

        IModalDialog::SetProgress(m_pGameLib->GetEntityResourceLoadingProgress());
        IModalDialog::Update();

        if (m_pGameLib->IsFinishedLoadingEntityResources())
            bFinishedPreloading = true;

        fProgress = float(iCurrentLoadingStep + m_pGameLib->GetEntityResourceLoadingProgress()) / LOADING_STEPS;
    }

    ++iCurrentLoadingStep;

    if (bFinishedPreloading)
    {
        Console.Client << _T("Client preloaded world ") << QuoteStr(m_pWorld->GetName()) << newl;

        m_eState = CLIENT_STATE_IDLE;

        m_pGameLib->FinishedLoadingWorld();

        IModalDialog::Hide();
        fProgress = 1.0f;
    }
    else
    {
        fProgress = MIN(fProgress, 0.99f);
    }

    return fProgress;
}


/*====================
  CHostClient::PreloadingFrame
  ====================*/
void    CHostClient::PreloadingFrame()
{
    assert(m_pGameLib != nullptr);

    // Load the next world component if any are pending
    if (m_pWorld == nullptr)
        return;

    float fProgress(0.0f);

    uint uiEndTime(K2System.Milliseconds() + 50);
    do
    {
        fProgress = PreloadingStep();
    }
    while (K2System.Milliseconds() < uiEndTime && fProgress != 1.0f);

    ChatManager.SendTMMPlayerLoadingUpdate(BYTE_ROUND(fProgress * 100.0f));
}


/*====================
  CHostClient::FilterGameList
  ====================*/
void    CHostClient::FilterGameList(const tstring &sName, const tstring &sSettings)
{
    m_ServerList.UpdateFilter(sName, sSettings);
}


/*====================
  CHostClient::SpamGameList
  ====================*/
void    CHostClient::SpamGameList(int iCount)
{
    m_ServerList.SpamGameList(iCount);
}


/*====================
  CHostClient::GetGameModeName
  ====================*/
tstring CHostClient::GetGameModeName(uint uiMode) const
{
    return m_pGameLib->GetGameModeName(uiMode);
}


/*====================
  CHostClient::GetGameModeFromString
  ====================*/
uint    CHostClient::GetGameModeFromString(const tstring &sMode) const
{
    return m_pGameLib->GetGameModeFromString(sMode);
}



/*====================
  CHostClient::GetGameOptionName
  ====================*/
tstring CHostClient::GetGameOptionName(uint uiOption) const
{
    return m_pGameLib->GetGameOptionName(uiOption);
}


/*====================
  CHostClient::GetGameOptionFromString
  ====================*/
uint    CHostClient::GetGameOptionFromString(const tstring &sOption) const
{
    return m_pGameLib->GetGameOptionFromString(sOption);
}


/*====================
  CHostClient::GetGameOptionsString
  ====================*/
tstring CHostClient::GetGameOptionsString(const uint uiOption) const
{
    return m_pGameLib->GetGameOptionsString(uiOption);
}

/*====================
  CHostClient::InviteUser
  ====================*/
void    CHostClient::InviteUser(const tstring &sName)
{
    m_pktReliableSend << NETCMD_CLIENT_INVITE << sName;
}


/*====================
  CHostClient::ServerInvite

  TODO: Maybe allow invites in game lobbies too...
  ====================*/
bool    CHostClient::ServerInvite(const tstring &sInviterName, int iInviterAccountID, const tstring &sAddress)
{
    if (m_eState != CLIENT_STATE_IDLE || !m_sInviteAddress.empty())
        return false;

    m_ServerList.ServerInvite(sInviterName, iInviterAccountID, sAddress);
    return true;
}


/*====================
  CHostClient::SetInviteAddress
  ====================*/
void    CHostClient::SetInviteAddress(const tstring &sAddress)
{
    m_sInviteAddress = sAddress;
    cl_inviteAddress = sAddress;
}


/*====================
  CHostClient::IsValidTier
  ====================*/
bool    CHostClient::IsValidTier(int iTier)
{
    return m_ClientAccount.IsValidTier(iTier);
}


/*====================
  CHostClient::IsLeaver
  ====================*/
bool    CHostClient::IsLeaver()
{
    return m_ClientAccount.IsLeaver();
}


/*====================
  CHostClient::IsValidPSR
  ====================*/
bool    CHostClient::IsValidPSR(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR)
{
    return m_ClientAccount.IsValidPSR(iRank, unMinPSR, unMaxPSR, unServerMinPSR, unServerMaxPSR);
}


/*====================
  CHostClient::IsValidPSRForGameList
  ====================*/
bool    CHostClient::IsValidPSRForGameList(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR, const bool bFilter)
{
    return m_ClientAccount.IsValidPSRForGameList(iRank, unMinPSR, unMaxPSR, unServerMinPSR, unServerMaxPSR, bFilter);
}


/*====================
  CHostClient::Translate
  ====================*/
tstring CHostClient::Translate(const tstring &sKey, const tsmapts &mapTokens)
{
    CStringTable *pStringTable(g_ResourceManager.GetStringTable(m_hClientMessages));
    if (pStringTable == nullptr)
        return TSNULL;

    tstring sMessage(pStringTable->Get(sKey));

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
  CHostClient::GetOldestReliable
  ====================*/
uint    CHostClient::GetOldestReliable() const
{
    return m_sockGame.GetOldestReliable();
}


/*====================
  CHostClient::SelectUpgrade
  ====================*/
void    CHostClient::SelectUpgrade(const tstring &sProductCode)
{
    m_ClientAccount.SelectUpgrade(sProductCode);
}


/*====================
  CHostClient::ClearUpgrade
  ====================*/
void    CHostClient::ClearUpgrade(const tstring &sType)
{
    m_ClientAccount.ClearUpgrade(sType);
}


/*====================
  CHostClient::CanAccessAltAvatar
  ====================*/
bool    CHostClient::CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)
{
    return m_ClientAccount.CanAccessAltAvatar(sHero, sAltAvatar);
}


/*====================
  CHostClient::PreloadWorld
  ====================*/
void    CHostClient::PreloadWorld(const tstring &sWorldName)
{
    ICvar::UnprotectTransmitCvars();

    Console.ExecuteScript(_T("/game_settings.cfg"));

    PreloadWorldResources(sWorldName);
}


/*--------------------
  Remote
  --------------------*/
CMD(Remote)
{
    if (vArgList.empty())
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->SendRemoteCommand(ConcatinateArgs(vArgList));
    return true;
}


#if 0
/*--------------------
  QuickConnect
  --------------------*/
CMD(QuickConnect)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->RequestMasterServerList(_T("10"), LIST_GAME);
    return true;
}

UI_VOID_CMD(QuickConnect, 0)
{
    cmdQuickConnect();
}
#endif


/*--------------------
  GetGameList
  --------------------*/
CMD(GetGameList)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    if (vArgList.size() > 0)
        pClient->RequestMasterServerList(_T("10"), LIST_GAME, vArgList[0]);
    else
        pClient->RequestMasterServerList(_T("10"), LIST_GAME);

    return true;
}

UI_VOID_CMD(GetGameList, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    if (vArgList.size() > 0)
        pClient->RequestMasterServerList(_T("10"), LIST_GAME, vArgList[0]->Evaluate());
    else
        pClient->RequestMasterServerList(_T("10"), LIST_GAME);
}

/*--------------------
  GetGameListCount
  --------------------*/
UI_CMD(GetGameListCount, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return _T("0");

    return XtoA(pClient->GetServerListCount());
}


/*--------------------
  GetOngoingGameList
  --------------------*/
CMD(GetOngoingGameList)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->RequestMasterServerList(_T("32"), LIST_ONGOING_GAME);
    return true;
}

UI_VOID_CMD(GetOngoingGameList, 0)
{
    cmdGetOngoingGameList();
}


/*--------------------
  GetServerList
  --------------------*/
CMD(GetServerList)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    if (vArgList.size() > 0)
        pClient->RequestMasterServerList(_T("90"), LIST_SERVER, vArgList[0]);
    else
        pClient->RequestMasterServerList(_T("90"), LIST_SERVER);

    return true;
}

UI_VOID_CMD(GetServerList, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    if (vArgList.size() > 0)
        pClient->RequestMasterServerList(_T("90"), LIST_SERVER, vArgList[0]->Evaluate());
    else
        pClient->RequestMasterServerList(_T("90"), LIST_SERVER);
}


/*--------------------
  GetLocalGameList
  --------------------*/
CMD(GetLocalGameList)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->RequestLocalServerList(LIST_GAME);
    return true;
}

UI_VOID_CMD(GetLocalGameList, 0)
{
    cmdGetLocalGameList();
}


/*--------------------
  GetLocalServerList
  --------------------*/
CMD(GetLocalServerList)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->RequestLocalServerList(LIST_SERVER);
    return true;
}

UI_VOID_CMD(GetLocalServerList, 0)
{
    cmdGetLocalServerList();
}


/*--------------------
  GetInviteAddress
  --------------------*/
UI_CMD(GetInviteAddress, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return pClient->GetInviteAddress();
}

/*--------------------
  GetInviteGameName
  --------------------*/
UI_CMD(GetInviteGameName, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return pClient->GetInviteGameName();
}


/*--------------------
  ClearInviteAddress
  --------------------*/
UI_VOID_CMD(ClearInviteAddress, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    pClient->SetInviteAddress(TSNULL);
    pClient->SetInviteGameName(TSNULL);
}


/*====================
  CServerList::SServerListEntry::SServerListEntry
  ====================*/
CServerList::SServerListEntry::SServerListEntry(
    const tstring &sAddress,
    ushort unPing) :
m_sAddress(sAddress),
m_unPing(unPing),
m_uiRequestTime(INVALID_TIME),
m_eList(LIST_NONE),
m_yNumPlayers(0xff),
m_yMaxPlayers(0xff),
m_bValid(false)
{
}


/*====================
  CServerList::~CServerList
  ====================*/
CServerList::~CServerList()
{
    m_pHTTPManager->ReleaseRequest(m_pRequest);
}


/*====================
  CServerList::RequestLocalServerList
  ====================*/
void    CServerList::RequestLocalServerList(EGameListType eList)
{
    m_mapLocalServers.clear();

    m_unLocalChallenge = M_Randnum(1, USHRT_MAX);
    m_eLocalList = eList;
    m_uiLocalRequestTime = K2System.Milliseconds();

    CPacket pkt;
    pkt.WriteByte(NETCMD_INFO_REQUEST);
    pkt.WriteShort(m_unLocalChallenge);

    m_sockBrowser.AllowBroadcast(true);

    tstring sBroadcastAddr(m_sockBrowser.GetBroadcastAddress());

    for (ushort unPort(DEFAULT_SERVER_PORT); unPort <= DEFAULT_SERVER_PORT + 4; ++unPort)
    {
        m_sockBrowser.SetSendAddr(sBroadcastAddr, unPort);
        m_sockBrowser.SendPacket(pkt);
    }

    m_sockBrowser.AllowBroadcast(false);

    RelistServers();
}


/*====================
  CServerList::RequestMasterServerList

  First digit:
  0 = Untracked
  1 = Quick Match
  2 = Ranked Match

  Second digit:
  0 = Unofficial
  1 = Official
  2 = Both

  Third digit:
  0 = Public
  1 = Private
  2 = Both
  ====================*/
void    CServerList::RequestMasterServerList(const tstring &sCookie, const tstring &sQueryType, EGameListType eList, const tstring &sRegion)
{
    PROFILE("CServerList::RequestMasterServerList");

    Console << _T("Requesting master server list") << newl;

    m_eRequestList = eList;

    m_pHTTPManager->ReleaseRequest(m_pRequest);
    m_pRequest = m_pHTTPManager->SpawnRequest();
    if (m_pRequest == nullptr)
        return;

    m_pRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pRequest->AddVariable(_T("f"), _T("server_list"));

    if (!sRegion.empty())
        m_pRequest->AddVariable(_T("region"), sRegion);

    m_pRequest->AddVariable(_T("cookie"), sCookie);

#ifdef K2_GARENA
    m_pRequest->AddVariable(_T("token"), Host.GetGarenaToken());
#endif

    if (!sQueryType.empty())
        m_pRequest->AddVariable(_T("gametype"), sQueryType);

    m_pRequest->SendPostRequest();

    // Reset pending server info requests
    m_deqRequests.clear();
    m_fRequestAccumulator = 0.0f;

    m_uiPlayerCount = 0;
}


/*====================
  CServerList::CancelServerList
  ====================*/
void    CServerList::CancelServerList()
{
    if (m_pRequest != nullptr)
    {
        m_pHTTPManager->ReleaseRequest(m_pRequest);
        m_pRequest = nullptr;
    }

    m_deqRequests.clear();
}

/*====================
  CServerList::ClearServerList
  ====================*/
void    CServerList::ClearServerList()
{
    m_mapServers.clear();
    GameListClear.Trigger(TSNULL);
}

/*====================
  CServerList::GetServerListCount
  ====================*/
uint    CServerList::GetServerListCount() const
{
    return (uint)m_mapServers.size();
}


/*====================
  CServerList::RequestServerInfo
  ====================*/
void    CServerList::RequestServerInfo(const tstring &sAddress, bool bOfficial, EGameListType eList, bool bInvite, const tstring &sInviter)
{
    ServerMap_it itFind(m_mapServers.find(sAddress));
    if (itFind == m_mapServers.end())
        itFind = m_mapServers.insert(ServerMapPair(sAddress, SServerListEntry(sAddress))).first;

    itFind->second.m_eList = eList;
    itFind->second.m_bInvite = bInvite;
    itFind->second.m_sInviter = sInviter;
    //itFind->second.m_uiRequestTime = K2System.Milliseconds();
    itFind->second.m_uiRequestTime = INVALID_TIME;
    itFind->second.m_unChallenge = M_Randnum(1, USHRT_MAX);
    itFind->second.m_yNumPlayers = 0xff;
    itFind->second.m_yMaxPlayers = 0xff;
    itFind->second.m_bValid = false;
    itFind->second.m_bVisible = false;
    itFind->second.m_yHostFlags = 0;
    itFind->second.m_yArrangedType = 0;

    if (bOfficial)
        itFind->second.m_yFlags = SERVER_OFFICIAL;
    else
        itFind->second.m_yFlags = 0;

    SServerInfoRequest cRequest;
    cRequest.m_sAddress = sAddress;

    if (bInvite)
        m_deqRequests.push_front(cRequest);
    else
        m_deqRequests.push_back(cRequest);
}


/*====================
  CServerList::CheckReconnectStatus
  ====================*/
void    CServerList::CheckReconnectStatus(const tstring &sAddress, uint uiMatchID, uint uiAccountID, ushort unConnectionID)
{
    m_sockBrowser.SetSendAddr(sAddress);

    CPacket pkt;
    pkt.WriteByte(NETCMD_RECONNECT_INFO_REQUEST);
    pkt.WriteInt(uiMatchID);
    pkt.WriteInt(uiAccountID);
    pkt.WriteShort(unConnectionID);
    m_sockBrowser.SendPacket(pkt);
}


/*====================
  CServerList::ProcessServerResponse
  ====================*/
bool    CServerList::ProcessServerResponse(const tstring &sAddress, CPacket &pkt)
{
    ushort unChallenge(pkt.ReadShort());
    tstring sServerName(pkt.ReadWStringAsTString());
    byte yNumPlayers(pkt.ReadByte());
    byte yMaxPlayers(pkt.ReadByte());
    tstring sMap(pkt.ReadWStringAsTString());
    tstring sVersion(pkt.ReadWStringAsTString());
    EServerAccess eAccess(EServerAccess(pkt.ReadByte()));
    int iTier(pkt.ReadByte());
    byte yHostFlags(pkt.ReadByte());
    bool bMatchStarted(pkt.ReadByte() != 0);
    tstring sRegion(pkt.ReadWStringAsTString());
    tstring sGameName(pkt.ReadWStringAsTString());
    tstring sGameMode(pkt.ReadWStringAsTString());
    byte yTeamSize(pkt.ReadByte());
    uint uiGameOptions(pkt.ReadInt());
    byte yArrangedType(pkt.ReadByte());
    ushort unMinPSR(pkt.ReadShort());
    ushort unMaxPSR(pkt.ReadShort());

    static tsvector vParams(5);

    tsvector vsClientVersion(TokenizeString(K2System.GetVersionString(), L'.'));
    uint uiClientVersion((AtoI(vsClientVersion[0]) << 24) + (AtoI(vsClientVersion[1]) << 16) + (AtoI(vsClientVersion[2]) << 8));

    ServerMap_it itFind(m_mapServers.find(sAddress));
    if (itFind != m_mapServers.end() && itFind->second.m_unChallenge == unChallenge)
    {
        itFind->second.m_unPing = K2System.Milliseconds() - itFind->second.m_uiRequestTime;

        if ((itFind->second.m_eList != LIST_SERVER && sMap.empty()) || (itFind->second.m_eList == LIST_SERVER && !sMap.empty()))
        {
            return true;
        }

        tsvector vsServerVersion(TokenizeString(sVersion, _T('.')));
        uint uiServerVersion((AtoI(vsServerVersion[0]) << 24) + (AtoI(vsServerVersion[1]) << 16) + (AtoI(vsServerVersion[2]) << 8));

        if (uiServerVersion != uiClientVersion ||
            (yNumPlayers > 0 && yMaxPlayers == 0) ||
            (bMatchStarted && itFind->second.m_eList != LIST_ONGOING_GAME))
        {
            if (itFind->second.m_eList == LIST_SERVER)
                Console.Client << _T("^y") << sServerName << SPACE << sAddress << SPACE << XtoA(itFind->second.m_unPing) << _T("ms") << _T(" ") << ParenStr(XtoA(yNumPlayers)) << newl;
            else
                Console.Client << _T("^y") << sServerName << SPACE << sAddress << SPACE << XtoA(itFind->second.m_unPing) << _T("ms") << _T(" - ") << sGameName << _T(" ") << ParenStr(XtoA(yNumPlayers)) << newl;

            return true;
        }

        tstring sServerNameLower(LowerString(sServerName));
        tstring sGameNameLower(LowerString(sGameName));

        tstring::iterator itGameName(sGameNameLower.begin()), itGameNameEnd(sGameNameLower.end());
        while (itGameName != itGameNameEnd)
        {
            if (*itGameName == _T(','))
                *itGameName = _T(' ');

            ++itGameName;
        }

        tstring::iterator itServerName(sServerNameLower.begin()), itServerNameEnd(sServerNameLower.end());
        while (itServerName != itServerNameEnd)
        {
            if (*itServerName == _T(','))
                *itServerName = _T(' ');

            ++itServerName;
        }

        itFind->second.m_bValid = true;
        itFind->second.m_sAddress = sAddress;
        itFind->second.m_sServerName = sServerName;
        itFind->second.m_sServerNameLower = sServerNameLower;
        itFind->second.m_sGameName = sGameName;
        itFind->second.m_sGameNameLower = sGameNameLower;
        itFind->second.m_sMapName = sMap;
        itFind->second.m_yTeamSize = yTeamSize;
        itFind->second.m_sGameMode = sGameMode;
        itFind->second.m_uiGameOptions = uiGameOptions;
        itFind->second.m_yNumPlayers = yNumPlayers;
        itFind->second.m_yMaxPlayers = yMaxPlayers;
        itFind->second.m_eAccess = eAccess;
        itFind->second.m_iTier = iTier;
        itFind->second.m_bMatchStarted = bMatchStarted;
        itFind->second.m_sRegion = sRegion;
        itFind->second.m_yArrangedType = yArrangedType;
        itFind->second.m_unMinPSR = unMinPSR;
        itFind->second.m_unMaxPSR = unMaxPSR;

        m_uiPlayerCount += yNumPlayers;

        itFind->second.m_yHostFlags = yHostFlags;

        if (itFind->second.m_bInvite)
        {
            Console.Client << _T("Invite Response: ") << sServerName << SPACE << sAddress << SPACE << XtoA(itFind->second.m_unPing) << _T("ms") << _T(" - ") << sGameName << newl;

            ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, _T("^900* You've been invited to the game '") + sGameName + _T("' at '") + sAddress + _T("' by ") + itFind->second.m_sInviter + _T("."));

            m_pClient->SetInviteAddress(sAddress);
            m_pClient->SetInviteGameName(sGameName);

            static tsvector vInvite(20);
            vInvite[0] = sAddress;
            vInvite[1] = StringReplace(sGameName, _T("'"), _T("`"));    // Replace ' with ` to avoid UI errors
            vInvite[2] = itFind->second.m_sInviter;                     // Inviter Name
            vInvite[3] = itFind->second.m_sRegion;                      // Server Region
            vInvite[4] = itFind->second.m_sGameMode;                    // Game Mode
            vInvite[5] = XtoA(itFind->second.m_yTeamSize);              // Team Size
            vInvite[6] = itFind->second.m_sMapName;                     // Map Name
            vInvite[7] = XtoA(itFind->second.m_iTier);                  // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)

            if ((itFind->second.m_yHostFlags & SERVER_OFFICIAL) && !(itFind->second.m_yHostFlags & HOST_SERVER_NO_STATS))
                vInvite[8] = _T("1");   // Official w/ stats
            else if ((itFind->second.m_yHostFlags & SERVER_OFFICIAL) && (itFind->second.m_yHostFlags & HOST_SERVER_NO_STATS))
                vInvite[8] = _T("2");   // Official w/o stats
            else if (!(itFind->second.m_yHostFlags & SERVER_OFFICIAL))
                vInvite[8] = _T("0");   // Unofficial

            vInvite[9] = XtoA(itFind->second.m_yHostFlags & HOST_SERVER_NO_LEAVER);     // No Leavers (1), Leavers (0)
            vInvite[10] = XtoA(EServerAccess(itFind->second.m_eAccess));                // Private (1), Not Private (0)

            vInvite[11] = _T("0");  // All Heroes (1), Not All Heroes (0)
            vInvite[12] = _T("0");  // Casual Mode (1), Not Casual Mode (0)
            vInvite[13] = _T("0");  // Force Random (1), Not Force Random (0)
            vInvite[14] = _T("0");  // Auto Balanced (1), Non Auto Balanced (0)
            vInvite[18] = _T("0");  // Dev Heroes (1), Non Dev Heroes (0)
            vInvite[19] = _T("0");  // Hardcore (1), Non Hardcore (0)

            bool bAdvancedOptions(false);

            for (uint ui(0); ui < 32; ++ui)
            {
                if (itFind->second.m_uiGameOptions & BIT(ui))
                {
                    const tstring &sGameOption(m_pClient->GetGameOptionName(BIT(ui)));

                    if (TStringCompare(sGameOption, _T("allheroes")) == 0)
                        vInvite[11] = _T("1");                                      // All Heroes (1), Not All Heroes (0)
                    else if (TStringCompare(sGameOption, _T("casual")) == 0)
                        vInvite[12] = _T("1");                                      // Casual Mode (1), Not Casual Mode (0)
                    else if (TStringCompare(sGameOption, _T("forcerandom")) == 0)
                        vInvite[13] = _T("1");                                      // Force Random (1), Not Force Random (0)
                    else if (TStringCompare(sGameOption, _T("autobalance")) == 0)
                        vInvite[14] = _T("1");                                      // Auto Balanced (1), Non Auto Balanced (0)
                    else if (TStringCompare(sGameOption, _T("devheroes")) == 0)
                        vInvite[18] = _T("1");                                      // Dev Heroes (1), Non Dev Heroes (0)
                    else if (TStringCompare(sGameOption, _T("hardcore")) == 0)
                        vInvite[19] = _T("1");                                      // Hardcore (1), Non Hardcore (0)
                    else if (TStringCompare(sGameOption, _T("alternatepicks")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("norepick")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("noswap")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("noagility")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("nointelligence")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("nostrength")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("norespawntimer")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("dropitems")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("nopowerups")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("supercreeps")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("allowduplicate")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("reverseselect")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("notop")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("nomiddle")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("nobottom")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("allowveto")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("shuffleteams")) == 0)
                        bAdvancedOptions = true;
                    else if (TStringCompare(sGameOption, _T("tournamentrules")) == 0)
                        bAdvancedOptions = true;
                }
            }

            // Advanced Options (1), No Advanced Options (0)
            if (bAdvancedOptions)
                vInvite[15] = _T("1");
            else
                vInvite[15] = _T("0");

            vInvite[16] = XtoA(itFind->second.m_unMinPSR);
            vInvite[17] = XtoA(itFind->second.m_unMaxPSR);

            GameInvite.Trigger(vInvite);
            if (cc_showGameInvites)
                ChatManager.PushNotification(NOTIFY_TYPE_GAME_INVITE, itFind->second.m_sInviter, sGameName, TSNULL, vInvite);
        }
        else
        {
            if (itFind->second.m_eList == LIST_SERVER)
                Console.Client << sServerName << SPACE << sAddress << SPACE << XtoA(itFind->second.m_unPing) << _T("ms") << _T(" ") << ParenStr(XtoA(yNumPlayers)) << newl;
            else
                Console.Client << sServerName << SPACE << sAddress << SPACE << XtoA(itFind->second.m_unPing) << _T("ms") << _T(" - ") << sGameName << _T(" ") << ParenStr(XtoA(yNumPlayers)) << newl;

            AddServerToList(itFind->second, false);
        }

        return true;
    }
    else if (unChallenge == m_unLocalChallenge)
    {
        if ((m_eLocalList != LIST_SERVER && sMap.empty()) || (m_eLocalList == LIST_SERVER && !sMap.empty()))
        {
            return true;
        }

        tsvector vsServerVersion(TokenizeString(sVersion, _T('.')));
        uint uiServerVersion((AtoI(vsServerVersion[0]) << 24) + (AtoI(vsServerVersion[1]) << 16) + (AtoI(vsServerVersion[2]) << 8));

        if (uiServerVersion != uiClientVersion ||
            (yNumPlayers > 0 && yMaxPlayers == 0) ||
            (bMatchStarted && m_eLocalList != LIST_ONGOING_GAME))
        {
            return true;
        }

        ushort unPing(K2System.Milliseconds() - m_uiLocalRequestTime);

        ServerMap_it itFind(m_mapLocalServers.find(sAddress));
        if (itFind == m_mapLocalServers.end())
            itFind = m_mapLocalServers.insert(ServerMapPair(sAddress, SServerListEntry(sAddress))).first;

        itFind->second.m_sAddress = sAddress;

        itFind->second.m_sServerName.clear();
        for (tstring::const_iterator it(sServerName.begin()), itEnd(sServerName.end()); it != itEnd; ++it)
        {
            if (*it == _T('\''))
                itFind->second.m_sServerName.push_back(_T('\\'));

            itFind->second.m_sServerName.push_back(*it);
        }
        itFind->second.m_sServerNameLower = LowerString(itFind->second.m_sServerName);

        itFind->second.m_sGameName.clear();
        for (tstring::const_iterator it(sGameName.begin()), itEnd(sGameName.end()); it != itEnd; ++it)
        {
            if (*it == _T('\''))
                itFind->second.m_sGameName.push_back(_T('\\'));

            itFind->second.m_sGameName.push_back(*it);
        }

        itFind->second.m_bValid = true;
        itFind->second.m_sGameNameLower = LowerString(sGameName);
        itFind->second.m_sMapName = sMap;
        itFind->second.m_yTeamSize = yTeamSize;
        itFind->second.m_sGameMode = sGameMode;
        itFind->second.m_uiGameOptions = uiGameOptions;
        itFind->second.m_eList = m_eLocalList;
        itFind->second.m_uiRequestTime = m_uiLocalRequestTime;
        itFind->second.m_unPing = unPing;
        itFind->second.m_unChallenge = m_unLocalChallenge;
        itFind->second.m_yNumPlayers = yNumPlayers;
        itFind->second.m_yMaxPlayers = yMaxPlayers;
        itFind->second.m_eAccess = eAccess;
        itFind->second.m_iTier = iTier;
        itFind->second.m_bMatchStarted = bMatchStarted;
        itFind->second.m_yHostFlags = yHostFlags;
        itFind->second.m_yArrangedType = yArrangedType;
        itFind->second.m_unMinPSR = unMinPSR;
        itFind->second.m_unMaxPSR = unMaxPSR;

        if (m_eLocalList == LIST_SERVER)
            Console.Client << sServerName << SPACE << sAddress << SPACE << XtoA(unPing) << _T("ms") << newl;
        else
            Console.Client << sServerName << SPACE << sAddress << SPACE << XtoA(unPing) << _T("ms") << _T(" - ") << sGameName << newl;

        if (!itFind->second.m_bMatchStarted)
            AddServerToList(itFind->second, true);

        return true;
    }
    else
    {
        Console.Client << _T("Unexpected server info response from ") << sAddress << newl;
        return false;
    }
}


/*====================
  CServerList::ProcessReconnectInfoResponse
  ====================*/
bool    CServerList::ProcessReconnectInfoResponse(const tstring &sAddress, CPacket &pkt)
{
    int iRemaingTime(pkt.ReadInt());
    if (pkt.HasFaults())
        return false;

    m_uiReconnectExpireTime = Host.GetTime() + iRemaingTime;
    ReconnectShow.Trigger(iRemaingTime > 0 ? _CTS("true"): _CTS("false"));
    ReconnectTimer.Trigger(XtoA(iRemaingTime));
    ReconnectAddress.Trigger(sAddress);
    return true;
}


/*====================
  CServerList::IsGameVisible
  ====================*/
bool    CServerList::IsGameVisible(SServerListEntry &cEntry)
{
    // don't show this, it's an arranged match of some sort (0 = None, 1 = MatchMaking, 2 = Tournament, 3 = League)
    if (cEntry.m_yArrangedType != 0)
        return false;

    if (!m_sFilterName.empty())
    {
        // "|" separates each search phrase, "-" excludes the search phrase, "\" can escape "-" or "|"
        tstring sFilterName(LowerString(StripColorCodes(m_sFilterName)));

        // escape the filter name and add funky escape character sequence that nobody would actually search on
        sFilterName = StringReplace(sFilterName, SEARCH_PIPES, REPLACE_PIPES);
        sFilterName = StringReplace(sFilterName, SEARCH_HYPHENS, REPLACE_HYPHENS);

        const tsvector vPhrases(TokenizeString(sFilterName, _T('|')));
        tstring sPhrase(_T(""));
        tstring sFirstChar(_T(""));

        bool bFound(false);
        uint uiExcludeCount(0);

        for (tsvector_cit itPhrase(vPhrases.begin()), itEnd(vPhrases.end()); itPhrase != itEnd; ++itPhrase)
        {
            sPhrase = itPhrase->c_str();
            sFirstChar = sPhrase.substr(0, 1);

            // replace all occurences of the search string (reverse sReplace and sSearch)
            sPhrase = StringReplace(sPhrase, REPLACE_PIPES, SEARCH_PIPES);
            sPhrase = StringReplace(sPhrase, REPLACE_HYPHENS, SEARCH_HYPHENS);

            // if this is an escaped phrase, remove the slash
            if (sPhrase.substr(0, 2) == SEARCH_PIPES || sPhrase.substr(0, 2) == SEARCH_HYPHENS)
                sPhrase = sPhrase.substr(1);

            // exclude this search phrase
            if (sFirstChar == _T("-"))
            {
                uiExcludeCount++;
                sPhrase = sPhrase.substr(1);

                if (cEntry.m_sGameNameLower.find(sPhrase) != tstring::npos)
                    return false;
            }
            else
            {
                if (cEntry.m_sGameNameLower.find(sPhrase) != tstring::npos)
                    bFound = true;
            }
        }

        // if all they typed were exclusions, the rest of the games should still be shown
        if (!bFound && !(uiExcludeCount == vPhrases.size()))
            return false;
    }

    for (tsmapts::iterator it(m_mapFilterSettings.begin()); it != m_mapFilterSettings.end(); ++it)
    {
        if (TStringCompare(it->first, _T("mode")) == 0)
        {
            if (it->second.empty())
                continue;
            else if (it->second.find_first_of(_T(",")) != tstring::npos)
            {
                // the user chose the 'Advanced' option for the Game Mode filter so the interface sent
                // multiple game modes in csv format (singledraft,banningdraft,banningpick,)
                tstring sFilterString(it->second);
                bool bFound(false);
                size_t zPos(sFilterString.find(_T(",")));
                while (zPos != tstring::npos)
                {
                    if (CompareNoCase(sFilterString.substr(0, zPos), cEntry.m_sGameMode) == 0)
                        bFound = true;

                    sFilterString = sFilterString.substr(zPos + 1);
                    zPos = sFilterString.find(_T(","));
                }

                if (bFound)
                    continue;
                else
                    return false;
            }
            else if (CompareNoCase(it->second, cEntry.m_sGameMode) != 0)
                return false;

            continue;
        }
        else if (TStringCompare(it->first, _T("region")) == 0)
        {
            if (it->second.empty())
                continue;
            // USA - All Region Filter
            else if (CompareNoCase(it->second, _T("US")) == 0)
            {
                if (CompareNoCase(cEntry.m_sRegion, _T("USC")) != 0 && CompareNoCase(cEntry.m_sRegion, _T("USE")) != 0 &&
                    CompareNoCase(cEntry.m_sRegion, _T("USS")) != 0 && CompareNoCase(cEntry.m_sRegion, _T("USW")) != 0)
                    return false;
            }
            // Europe - All Region Filter
            else if (CompareNoCase(it->second, _T("EU")) == 0)
            {
                if (CompareNoCase(cEntry.m_sRegion, _T("DE")) != 0 && CompareNoCase(cEntry.m_sRegion, _T("UK")) != 0 &&
                    CompareNoCase(cEntry.m_sRegion, _T("NL")) != 0 && CompareNoCase(cEntry.m_sRegion, _T("EU")) != 0)
                    return false;
            }
            // All Other Region Filters
            else if (CompareNoCase(it->second, _T("USC")) == 0 || CompareNoCase(it->second, _T("USE")) == 0 ||
                     CompareNoCase(it->second, _T("USS")) == 0 || CompareNoCase(it->second, _T("USW")) == 0 ||
                     CompareNoCase(it->second, _T("DE")) == 0 || CompareNoCase(it->second, _T("UK")) == 0 ||
                     CompareNoCase(it->second, _T("NL")) == 0 || CompareNoCase(it->second, _T("JPN")) == 0 ||
                     CompareNoCase(it->second, _T("ZAF")) == 0 || CompareNoCase(it->second, _T("AU")) == 0 ||
                     CompareNoCase(it->second, _T("ID")) == 0 || CompareNoCase(it->second, _T("SEA")) == 0 ||
                     CompareNoCase(it->second, _T("GREG")) == 0 || CompareNoCase(it->second, _T("GVIP")) == 0)
            {
                if (CompareNoCase(it->second, cEntry.m_sRegion) != 0)
                    return false;
            }

            continue;
        }
        else if (TStringCompare(it->first, _T("ping")) == 0)
        {
            const int iPing(AtoI(it->second));

            if (iPing == 0 || cEntry.m_unPing < iPing)
                continue;
            else
                return false;

            continue;
        }
        else if (TStringCompare(it->first, _T("teamsize")) == 0)
        {
            const int iTeamSize(AtoI(it->second));

            if (iTeamSize == -1)
                continue;
            else if (iTeamSize != int(cEntry.m_yTeamSize))
                return false;

            continue;
        }
        else if (TStringCompare(it->first, _T("map")) == 0)
        {
            if (it->second.empty())
                continue;
            else if (CompareNoCase(it->second, cEntry.m_sMapName) != 0)
                return false;

            continue;
        }
        else if (TStringCompare(it->first, _T("servertype")) == 0)
        {
            const int iServerType(AtoI(it->second));

            if (iServerType == -1)
                continue;
            else if (iServerType == 1) // Official w/ stats
            {
                if (!(cEntry.m_yFlags & SERVER_OFFICIAL) || (cEntry.m_yHostFlags & HOST_SERVER_NO_STATS))
                    return false;
            }
            else if (iServerType == 2) // Official w/o stats
            {
                if (!(cEntry.m_yFlags & SERVER_OFFICIAL) || !(cEntry.m_yHostFlags & HOST_SERVER_NO_STATS))
                    return false;
            }
            else if (iServerType == 0) // Unofficial
            {
                if (cEntry.m_yFlags & SERVER_OFFICIAL)
                    return false;
            }

        }
        else if (TStringCompare(it->first, _T("access")) == 0)
        {
            const int iAccess(AtoI(it->second));

            if (iAccess == -1)
                continue;
            else if (iAccess != int(cEntry.m_eAccess))
                return false;
        }
        /*
        else if (TStringCompare(it->first, _T("tier")) == 0)
        {
            const int iTier(AtoI(it->second));

            if (iTier == -1)
                continue;
            else if (iTier != int(cEntry.m_iTier))
                return false;
        }
        */
        else if (TStringCompare(it->first, _T("minpsr")) == 0)
        {
            const ushort unMinPSR(AtoI(it->second));

            if (unMinPSR)
            {
                if (m_pClient->IsValidPSRForGameList(0, unMinPSR, 0, cEntry.m_unMinPSR, cEntry.m_unMaxPSR, true))
                    continue;
                else
                    return false;
            }

            continue;
        }
        else if (TStringCompare(it->first, _T("maxpsr")) == 0)
        {
            const ushort unMaxPSR(AtoI(it->second));

            if (unMaxPSR)
            {
                if (m_pClient->IsValidPSRForGameList(0, 0, unMaxPSR, cEntry.m_unMinPSR, cEntry.m_unMaxPSR, true))
                    continue;
                else
                    return false;
            }

            continue;
        }
        else if (TStringCompare(it->first, _T("noleaver")) == 0)
        {
            if (TStringCompare(it->second, _T("required")) == 0)
            {
                if ((cEntry.m_yHostFlags & HOST_SERVER_NO_LEAVER) == 0)
                    return false;
            }
            else if (TStringCompare(it->second, _T("excluded")) == 0)
            {
                if ((cEntry.m_yHostFlags & HOST_SERVER_NO_LEAVER) != 0)
                    return false;
            }
        }
        else if (TStringCompare(it->first, _T("full")) == 0)
        {
            if (TStringCompare(it->second, _T("required")) == 0)
            {
                if (cEntry.m_yNumPlayers != cEntry.m_yMaxPlayers)
                    return false;
            }
            else if (TStringCompare(it->second, _T("excluded")) == 0)
            {
                if (cEntry.m_yNumPlayers == cEntry.m_yMaxPlayers)
                    return false;
            }
        }

        const uint uiOption(m_pClient->GetGameOptionFromString(it->first));
        if (uiOption != 0)
        {
            if (TStringCompare(it->second, _T("required")) == 0)
            {
                if (~cEntry.m_uiGameOptions & uiOption)
                    return false;
            }
            else if (TStringCompare(it->second, _T("excluded")) == 0)
            {
                if (cEntry.m_uiGameOptions & uiOption)
                    return false;
            }
        }
    }

    return true;
}


/*====================
  CServerList::AddServerToList
  ====================*/
void    CServerList::AddServerToList(SServerListEntry &cEntry, bool bLocal)
{
    if (cEntry.m_unPing == USHRT_MAX || !cEntry.m_bValid)
        return;

    if (cEntry.m_eList == LIST_SERVER)
    {
        static tsvector vsParams(19);

        vsParams[0] = cEntry.m_sAddress;
        vsParams[1] = cEntry.m_sServerNameLower + _T(",") +
            XtoA(cEntry.m_unPing, FMT_PADZERO, 5) + _T(",") +
            XtoA((cEntry.m_yFlags & SERVER_OFFICIAL) != 0) + _T(",") +
            cEntry.m_sRegion;

        vsParams[2] = cEntry.m_sServerName;
        vsParams[3] = XtoA(cEntry.m_unPing);
        vsParams[4] = XtoA(int(cEntry.m_eAccess));
        vsParams[5] = XtoA(true); //XtoA((cEntry.m_yFlags & SERVER_OFFICIAL) != 0);
        vsParams[6] = cEntry.m_sRegion;
        vsParams[7] = XtoA(true);

        ServerListAdd.Trigger(vsParams);

        if (false)
            ServerListHide.Trigger(cEntry.m_sAddress);
        else
            ServerListUpdate.Trigger(TSNULL);
    }
    else
    {
        static tsvector vsParams(23);

        vsParams[0] = cEntry.m_sAddress;
        vsParams[1] = cEntry.m_sGameName;
        vsParams[2] = XtoA(cEntry.m_unPing);
        vsParams[3] = XtoA(cEntry.m_yTeamSize) + _T("v") + XtoA(cEntry.m_yTeamSize);
        vsParams[4] = cEntry.m_sGameMode;
        vsParams[5] = XtoA(int(cEntry.m_eAccess));

        tstring sId;
        int iServerType(0);
        if ((cEntry.m_yFlags & SERVER_OFFICIAL) && !(cEntry.m_yHostFlags & HOST_SERVER_NO_STATS))
        {
            sId = _CTS("official");
            iServerType = 1;
        }
        else if ((cEntry.m_yFlags & SERVER_OFFICIAL) && (cEntry.m_yHostFlags & HOST_SERVER_NO_STATS))
        {
            sId = _CTS("official_2");
            iServerType = 2;
        }
        else if (!(cEntry.m_yFlags & SERVER_OFFICIAL))
        {
            sId = _CTS("invis");
            iServerType = 0;
        }

        vsParams[6] = sId;

        {
            // FIXME: there is only space for 5 icons (4 of these options + 1 for advanced) both on the interface and in the 
            // param list, we need to rearrange the interface and the paramlist to add more so that all can appear properly 
            // without breaking existing stuff.
            const uint uiMaxOptionIcons(5);

            uint uiGameOptions(cEntry.m_uiGameOptions);

            // first, clear each of the 5 game option icons to "invis".
            for (uint ui(0); ui < uiMaxOptionIcons; ++ui)
                vsParams[7 + ui] = _T("invis");

            uint uiIconIdx(0);
            bool bShowAllOptionsIcon(false);
            // now iterate (in reverse, so that the icons line up) over all possible game options.
            for (int i(31); i >= 0; --i)
            {
                uint ui((uint)i);

                // skip options that haven't been set.
                if ((uiGameOptions & BIT(ui)) == 0)
                    continue;
                    
                // get the name of the option.
                const tstring &sGameOption(m_pClient->GetGameOptionName(BIT(ui)));

                if (TStringCompare(sGameOption, _T("casual")) == 0 ||
                    TStringCompare(sGameOption, _T("forcerandom")) == 0 ||
                    TStringCompare(sGameOption, _T("autobalance")) == 0 ||
                    TStringCompare(sGameOption, _T("devheroes")) == 0 ||
                    TStringCompare(sGameOption, _T("hardcore")) == 0)
                {
                    // if we've ran out of icon display spots, then show the "all options" icon.
                    if (uiIconIdx >= uiMaxOptionIcons)
                    {
                        bShowAllOptionsIcon = true;
                        break;
                    }

                    // display the icon of the basic game option.
                    vsParams[7 + uiIconIdx] = sGameOption;
                    ++uiIconIdx;
                }
                else
                {
                    // the game is using advanced game options, so show the "all options" icon.
                    bShowAllOptionsIcon = true;
                }
            }

            // show the "All Options" icon (the "+" icon), overwriting the last game option icon if necessary.
            if (bShowAllOptionsIcon)
            {
                vsParams[7 + CLAMP<uint>(uiIconIdx, 0, uiMaxOptionIcons - 1)] = _T("more");
                vsParams[21] = m_pClient->GetGameOptionsString(uiGameOptions);
            }
            else
            {
                vsParams[21] = _T("0");     
            }
        }
                    
        // all info on the game browser table needs to be here, each csv value is related to each column the data appears on
        // and needs to be here so that SortListboxSortIndex() on the interface can sort properly
        vsParams[12] = cEntry.m_sGameNameLower + _T(",") +
            XtoA(cEntry.m_unPing, FMT_PADZERO, 5) + _T(",") +
            XtoA(cEntry.m_yTeamSize) + _T(",") +
            XtoA(m_pClient->GetGameModeFromString(cEntry.m_sGameMode)) + _T(",") +
            XtoA(cEntry.m_uiGameOptions, FMT_PADZERO, 8) + _T(",") +
            XtoA(iServerType) + _T(",") +
            XtoA(cEntry.m_iTier) + _T(",") +    // (Depreciated)
            XtoA((cEntry.m_yHostFlags & HOST_SERVER_NO_LEAVER) != 0) + _T(",") +
            XtoA(cEntry.m_yMaxPlayers - cEntry.m_yNumPlayers) + _T(",") +
            XtoA(cEntry.m_sMapName) + _T(",") +
            XtoA(cEntry.m_unMinPSR) + _T(",") +
            XtoA(cEntry.m_unMaxPSR);

        // (Depreciated)
        if (cEntry.m_iTier == 0)
            vsParams[13] = _T("beginner");
        else if (cEntry.m_iTier == 1)
            vsParams[13] = _T("general");
        else
            vsParams[13] = _T("veteran");

        vsParams[14] = XtoA((cEntry.m_yHostFlags & HOST_SERVER_NO_LEAVER) != 0);

        // Can the player join this match?
        bool bCanJoin(true);

        // Check leaver status
        if (m_pClient->IsLeaver() && (cEntry.m_yHostFlags & HOST_SERVER_NO_LEAVER))
            bCanJoin = false;

        // Check PSR requirements
        if (!m_pClient->IsValidPSRForGameList(m_pClient->GetAccount().GetPSR(), 0, 0, cEntry.m_unMinPSR, cEntry.m_unMaxPSR))
            bCanJoin = false;

        vsParams[15] = XtoA(bCanJoin);

        vsParams[16] = XtoA(cEntry.m_yNumPlayers);
        vsParams[17] = XtoA(cEntry.m_yMaxPlayers);
        vsParams[18] = XtoA(cEntry.m_sMapName);
        vsParams[19] = XtoA(cEntry.m_unMinPSR);
        vsParams[20] = XtoA(cEntry.m_unMaxPSR);
        vsParams[22] = XtoA((cEntry.m_yHostFlags & HOST_SERVER_NO_STATS) != 0);

        GameListAdd.Trigger(vsParams);

        cEntry.m_bVisible = IsGameVisible(cEntry);

        if (!cEntry.m_bVisible)
            GameListHide.Trigger(cEntry.m_sAddress);
        else
            GameListUpdate.Trigger(TSNULL);
    }
}


/*====================
  CServerList::RelistServers
  ====================*/
void    CServerList::RelistServers()
{
    ServerListClear.Trigger(TSNULL);
    GameListClear.Trigger(TSNULL);

    for (ServerMap_it it(m_mapServers.begin()), itEnd(m_mapServers.end()); it != itEnd; ++it)
        AddServerToList(it->second, false);

    for (ServerMap_it it(m_mapLocalServers.begin()), itEnd(m_mapLocalServers.end()); it != itEnd; ++it)
        AddServerToList(it->second, true);
}


/*====================
  CServerList::ProcessServerList
  ====================*/
void    CServerList::ProcessServerList(const tstring &sResponse)
{
    CPHPData phpResponse(sResponse);

    const CPHPData *pKey(phpResponse.GetVar(_T("acc_key")));
    if (pKey != nullptr)
    {
        m_sMatchKey = pKey->GetString();
        Console << _T("Received key: ") << m_sMatchKey << newl;
    }

    const CPHPData *pStats(phpResponse.GetVar(_T("svr_stats")));
    if (pStats != nullptr)
    {
        m_iNumOnline = pStats->GetInteger(_T("online"));
        m_iNumInGame = pStats->GetInteger(_T("in_game"));
    }

    m_mapServers.clear();

    const CPHPData *pServerList(phpResponse.GetVar(_T("server_list")));
    if (pServerList == nullptr)
        return;

    for (size_t z(0); z < pServerList->GetSize(); ++z)
    {
        const CPHPData *pEntry(pServerList->GetVar(z));
        if (pEntry == nullptr || pEntry->GetType() != PHP_ARRAY)
            continue;

        RequestServerInfo(pEntry->GetString(_T("ip")) + _T(":") + pEntry->GetString(_T("port")), pEntry->GetInteger(_T("class")) > 0, m_eRequestList, false, TSNULL);
    }

    K2::random_shuffle(m_deqRequests.begin(), m_deqRequests.end());

    RelistServers();

    m_fRequestAccumulator = 0.0f;
}


/*====================
  CServerList::Frame
  ====================*/
void    CServerList::Frame()
{
    if (m_uiReconnectExpireTime != INVALID_TIME)
    {
        if (Host.GetTime() >= m_uiReconnectExpireTime)
            m_uiReconnectExpireTime = INVALID_TIME;
        else
            ReconnectTimer.Trigger(XtoA(m_uiReconnectExpireTime - Host.GetTime()));
    }

    if (!m_deqRequests.empty())
    {
        m_fRequestAccumulator += cl_infoRequestRate * MsToSec(MIN(Host.GetFrameLength(), 100u));

        while (m_fRequestAccumulator >= 1.0f && !m_deqRequests.empty())
        {
            tstring sAddress(m_deqRequests.front().m_sAddress);
            m_deqRequests.pop_front();

            ServerMap_it itFind(m_mapServers.find(sAddress));
            if (itFind == m_mapServers.end())
                continue;
            if (itFind->second.m_uiRequestTime != INVALID_TIME)
                continue;

            itFind->second.m_uiRequestTime = K2System.Milliseconds();

            m_sockBrowser.SetSendAddr(sAddress);

            CPacket pkt;
            pkt.WriteByte(NETCMD_INFO_REQUEST);
            pkt.WriteShort(itFind->second.m_unChallenge);
            m_sockBrowser.SendPacket(pkt);

            m_fRequestAccumulator -= 1.0f;
        }
    }
    else
    {
        m_fRequestAccumulator = 0.0f;
    }

    if (m_pRequest != nullptr && !m_pRequest->IsActive())
    {
        if (m_pRequest->WasSuccessful())
            ProcessServerList(m_pRequest->GetResponse());

        m_pHTTPManager->ReleaseRequest(m_pRequest);
        m_pRequest = nullptr;
    }

    CPacket pkt;
    while (m_sockBrowser.ReceivePacket(pkt) > 0)
    {
        //NetStats.RecordPacket(NETSOCKET_INCOMING, pkt.GetLength());

        if (!m_sockBrowser.PreProcessPacket(pkt))
            continue;

        while (!pkt.DoneReading())
        {
            byte yCmd(pkt.ReadByte());

            switch (yCmd)
            {
            case NETCMD_SERVER_INFO:
                ProcessServerResponse(m_sockBrowser.GetRecvAddrName() + _T(":") + XtoA(m_sockBrowser.GetRecvPort()), pkt);
                break;

            case NETCMD_RECONNECT_INFO_RESPONSE:
                ProcessReconnectInfoResponse(m_sockBrowser.GetRecvAddrName() + _T(":") + XtoA(m_sockBrowser.GetRecvPort()), pkt);
                break;

            default:
                Console.Warn << _T("Server list received unknown command: ") << BYTE_HEX_STR(yCmd) << newl;
                break;
            }
        }
    }
}


/*====================
  CServerList::GetBestServer
  ====================*/
tstring     CServerList::GetBestServer()
{
    for (ServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.m_unPing != USHRT_MAX && it->second.m_bValid)
            return it->second.m_sAddress;
    }

    return TSNULL;
}


/*====================
  CServerList::GetBestLocalServer
  ====================*/
tstring     CServerList::GetBestLocalServer()
{
    if (!m_mapLocalServers.empty())
        return m_mapLocalServers.begin()->second.m_sAddress;
    else
        return TSNULL;
}


/*====================
  CServerList::GetBestServerName
  ====================*/
tstring     CServerList::GetBestServerName()
{
    for (ServerMap::iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.m_unPing != USHRT_MAX && it->second.m_bValid)
            return it->second.m_sServerName;
    }

    return TSNULL;
}


/*====================
  CServerList::GetBestLocalServerName
  ====================*/
tstring     CServerList::GetBestLocalServerName()
{
    if (!m_mapLocalServers.empty())
        return m_mapLocalServers.begin()->second.m_sServerName;
    else
        return TSNULL;
}


/*====================
  CServerList::AwaitingResponses
  ====================*/
bool    CServerList::AwaitingResponses() const
{
    if (m_pRequest != nullptr && m_pRequest->IsActive())
        return true;

    uint uiTime(K2System.Milliseconds());

    for (ServerMap::const_iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        if (it->second.m_unPing != USHRT_MAX)
            continue;
        if (it->second.m_uiRequestTime == INVALID_TIME || (uiTime - it->second.m_uiRequestTime) < 3000)
            return true;
    }

    return false;
}


/*====================
  CServerList::AwaitingLocalResponses
  ====================*/
bool    CServerList::AwaitingLocalResponses() const
{
    uint uiTime(K2System.Milliseconds());

    if (m_uiLocalRequestTime == INVALID_TIME || (uiTime - m_uiLocalRequestTime) < 3000)
        return true;

    return false;
}


/*====================
  CServerList::UpdateFilter
  ====================*/
void    CServerList::UpdateFilter(const tstring &sName, const tstring &sSettings)
{
    m_sFilterName = LowerString(sName);

    tsvector vSettingPairs(TokenizeString(sSettings, _T(' ')));
    m_mapFilterSettings.clear();
    for (tsvector_it it(vSettingPairs.begin()); it != vSettingPairs.end(); ++it)
    {
        size_t zDiv(it->find(_T(':')));
        if (zDiv == tstring::npos)
            continue;

        m_mapFilterSettings[it->substr(0, zDiv)] = it->substr(zDiv + 1);
    }

    for (ServerMap_it it(m_mapServers.begin()), itEnd(m_mapServers.end()); it != itEnd; ++it)
    {
        if (!it->second.m_bValid)
            continue;

        it->second.m_bVisible = IsGameVisible(it->second);

        if (it->second.m_bVisible)
            GameListShow.Trigger(it->second.m_sAddress);
        else
            GameListHide.Trigger(it->second.m_sAddress);
    }

    GameListUpdate.Trigger(TSNULL);
}


/*====================
  RandomString
  ====================*/
static
tstring     RandomString(uint uiLength)
{
    tstring sRet;

    for (uint ui(0); ui < uiLength; ++ui)
        sRet += char(M_Randnum(int('A'), int('Z')));

    return sRet;
}


/*====================
  CServerList::SpamGameList
  ====================*/
void    CServerList::SpamGameList(int iCount)
{
    for (int i(0); i < iCount; ++i)
    {
        tstring sAddress(XtoA(M_Randnum(1, 255)) + _T(".") + XtoA(M_Randnum(1, 255)) + _T(".") + XtoA(M_Randnum(1, 255)) + _T(".") + XtoA(M_Randnum(1, 255)) + _T(":") + XtoA(M_Randnum(1, 65535)));

        ServerMap_it itFind(m_mapServers.find(sAddress));
        if (itFind != m_mapServers.end())
            continue;

        itFind = m_mapServers.insert(ServerMapPair(sAddress, SServerListEntry(sAddress))).first;

        itFind->second.m_sAddress = sAddress;

        itFind->second.m_sServerName = RandomString(M_Randnum(4, 16));
        itFind->second.m_sGameName = RandomString(M_Randnum(4, 16));

        itFind->second.m_sGameNameLower = LowerString(itFind->second.m_sGameName);
        itFind->second.m_sMapName = M_Randnum(0, 1) ? _T("caldavar") : _T("watchtower");
        itFind->second.m_yTeamSize = M_Randnum(1, 5);
        itFind->second.m_yMaxPlayers = itFind->second.m_yTeamSize * 2;
        itFind->second.m_yNumPlayers = M_Randnum(0, itFind->second.m_yMaxPlayers);
        itFind->second.m_sGameMode = m_pClient->GetGameModeName(M_Randnum(0, 3));
        itFind->second.m_uiGameOptions = (M_Randnum(0, 1) ? BIT(7) : 0) | (M_Randnum(0, 1) ? BIT(0) : 0) | (M_Randnum(0, 1) ? BIT(12) : 0) | (M_Randnum(0, 1) ? BIT(19) : 0);
        itFind->second.m_eList = LIST_GAME;
        itFind->second.m_uiRequestTime = INVALID_TIME;
        itFind->second.m_unPing = M_Randnum(1, 999);
        itFind->second.m_unChallenge = 0;
        itFind->second.m_eAccess = EServerAccess(M_Randnum(0, 1));
        itFind->second.m_yFlags = M_Randnum(0, 1);
        itFind->second.m_yHostFlags = M_Randnum(0, 3);
        itFind->second.m_iTier = M_Randnum(0, 2);
        itFind->second.m_bMatchStarted = false;
        itFind->second.m_bValid = true;
        itFind->second.m_bVisible = false;
        itFind->second.m_unMinPSR = M_Randnum(1200, 1600);
        itFind->second.m_unMaxPSR = itFind->second.m_unMinPSR + M_Randnum(50, 200);

        AddServerToList(itFind->second, false);
    }
}


/*====================
  CServerList::ServerInvite
  ====================*/
void    CServerList::ServerInvite(const tstring &sInviterName, int iInviterAccountID, const tstring &sAddress)
{
    RequestServerInfo(sAddress, false, LIST_NONE, true, sInviterName);
}


/*====================
  CServerList::GetStatus
  ====================*/
bool    CServerList::GetStatus(bool &bList, uint &uiProcessed, uint &uiTotal, uint &uiResponses, uint &uiVisible) const
{
    if (m_pRequest != nullptr && m_pRequest->IsActive())
    {
        bList = false;
        uiProcessed = 0;
        uiTotal = 0;
        uiResponses = 0;
        uiVisible = 0;
        return true;
    }

    uint uiTime(K2System.Milliseconds());

    bList = true;
    uiProcessed = 0;
    uiTotal = 0;
    uiResponses = 0;
    uiVisible = 0;

    for (ServerMap::const_iterator it(m_mapServers.begin()); it != m_mapServers.end(); ++it)
    {
        ++uiTotal;

        if (it->second.m_unPing != USHRT_MAX)
        {
            ++uiResponses;
            ++uiProcessed;
        }
        else if (it->second.m_uiRequestTime != INVALID_TIME && (uiTime - it->second.m_uiRequestTime) >= 3000)
        {
            ++uiProcessed;
        }

        if (it->second.m_bVisible)
            ++uiVisible;
    }

    return uiProcessed != uiTotal;
}


/*====================
  CServerList::Clear
  ====================*/
void    CServerList::Clear()
{
    m_mapServers.clear();
    RelistServers();
}


/*--------------------
  PrintSystemInfo
  --------------------*/
CMD(PrintSystemInfo)
{
    SSysInfo info(K2System.GetSystemInfo());
    Console
        << _T("OS: ") << info.sOS << newl
        << _T("Processor: ") << info.sProcessor << newl
        << _T("MAC: ") << info.sMAC << newl
        << _T("Video: ") << info.sVideo << newl
        << _T("RAM: ") << GetByteString(K2System.GetTotalPhysicalMemory()) << newl;

    return true;
}


/*--------------------
  RegenerateConnectionID
  --------------------*/
CMD(RegenerateConnectionID)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient != nullptr)
        pClient->RegenerateConnectionID();

    return true;
}


/*--------------------
  CheckReconnect
  --------------------*/
CMD(CheckReconnect)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    if (vArgList.size() < 2)
        pClient->CheckReconnect();
    else 
        pClient->CheckReconnect(vArgList[0], AtoUI(vArgList[1]));

    return true;
}


/*--------------------
  SetServerPrivateValue
  --------------------*/
CMD(SetServerPrivateValue)
{
    if (vArgList.empty())
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    byte yValue(AtoI(vArgList[0]));
    pClient->SendServerPrivateValue(yValue);

    return true;
}

UI_VOID_CMD(SetServerPrivateValue, 1)
{
    cmdSetServerPrivateValue(vArgList[0]->Evaluate());
}


/*--------------------
  FilterGameList
  --------------------*/
CMD(FilterGameList)
{
    if (vArgList.size() < 2)
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->FilterGameList(vArgList[0], ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
    return true;
}

UI_VOID_CMD(FilterGameList, 2)
{
    tsvector vArgs(2);
    vArgs[0] = vArgList[0]->Evaluate();
    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
        vArgs[1] += (*it)->Evaluate() + SPACE;

    cmdFilterGameList(vArgs);
}


/*--------------------
  SpamGameList
  --------------------*/
CMD(SpamGameList)
{
    if (vArgList.size() < 1)
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->SpamGameList(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  AddTiers
  --------------------*/
UI_VOID_CMD(AddTiers, 1)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    tstring sWidth, sHeight;
    if (pList)
    {
        sWidth = pList->GetListItemWidth();
        sHeight = pList->GetListItemHeight();
    }
    else
    {
        sWidth = _T("0");
        sHeight = _T("0");
    }

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 1); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    {
        mapParams[_T("label")] = _T("Noobs Only");
        mapParams[_T("texture")] = _T("/ui/icons/beginner.tga");

        if (pClient->IsValidTier(0))
        {
            mapParams[_T("color")] = _T("white");
            mapParams[_T("select")] = _T("true");
        }
        else
        {
            mapParams[_T("color")] = _T("gray");
            mapParams[_T("select")] = _T("false");
        }

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(0), mapParams);
    }

    {
        mapParams[_T("label")] = _T("Noobs Allowed");
        mapParams[_T("texture")] = _T("/ui/icons/general.tga");

        if (pClient->IsValidTier(1))
        {
            mapParams[_T("color")] = _T("white");
            mapParams[_T("select")] = _T("true");
        }
        else
        {
            mapParams[_T("color")] = _T("gray");
            mapParams[_T("select")] = _T("false");
        }

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(1), mapParams);
    }

    {
        mapParams[_T("label")] = _T("Pro");
        mapParams[_T("texture")] = _T("/ui/icons/veteran.tga");

        if (pClient->IsValidTier(2))
        {
            mapParams[_T("color")] = _T("white");
            mapParams[_T("select")] = _T("true");
        }
        else
        {
            mapParams[_T("color")] = _T("gray");
            mapParams[_T("select")] = _T("false");
        }

        pList->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), XtoA(2), mapParams);
    }
}


/*--------------------
  GetNetName
  --------------------*/
UI_CMD(GetNetName, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return net_name;

    return pClient->GetNetName();
}


/*--------------------
  PlayerCount
  --------------------*/
CMD(PlayerCount)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    Console << pClient->GetPlayerCount() << newl;
    return true;
}


/*--------------------
  GetServerCount
  --------------------*/
CMD(GetServerCount)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->RequestMasterServerList(_T(""), LIST_SERVER);
    return true;
}


/*--------------------
  CancelServerList
  --------------------*/
UI_VOID_CMD(CancelServerList, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    pClient->CancelServerList();
    return;
}


/*--------------------
  ClearServerList
  --------------------*/
UI_VOID_CMD(ClearServerList, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    pClient->ClearServerList();
    return;
}


/*--------------------
  GetGarenaToken
  --------------------*/
#ifdef K2_GARENA
UI_CMD(GetGarenaToken, 0)
{
    return Host.GetGarenaToken();
}
#endif


/*--------------------
  IsValidLeaverGame
  --------------------*/
UI_CMD(IsValidLeaverGame, 0)
{
    // it's not a replay and it's not a practice game, so it's considered a leaver game
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return _T("0");

    if (!Host.IsReplay() && !pClient->GetPractice())
        return _T("1");
    else
        return _T("0");
}


/*--------------------
  PrintLoadTimes
  --------------------*/
CMD(PrintLoadTimes)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    Console << _T("Connected: ") << (pClient->GetStateStartTime(CLIENT_STATE_CONNECTED) - pClient->GetStateStartTime(CLIENT_STATE_CONNECTING)) / 1000.0f << _T(" ms") << newl;
    Console << _T("Loading: ") << (pClient->GetStateStartTime(CLIENT_STATE_LOADING) - pClient->GetStateStartTime(CLIENT_STATE_CONNECTED)) / 1000.0f << _T(" ms") << newl;
    Console << _T("First snapshot: ") << (pClient->GetStateStartTime(CLIENT_STATE_WAITING_FIRST_SNAPSHOT) - pClient->GetStateStartTime(CLIENT_STATE_LOADING)) / 1000.0f << _T(" ms") << newl;
    Console << _T("In-game: ") << (pClient->GetStateStartTime(CLIENT_STATE_IN_GAME) - pClient->GetStateStartTime(CLIENT_STATE_WAITING_FIRST_SNAPSHOT)) / 1000.0f << _T(" ms") << newl;
    Console << _T("Total: ") << (pClient->GetStateStartTime(CLIENT_STATE_IN_GAME) - pClient->GetStateStartTime(CLIENT_STATE_CONNECTING)) / 1000.0f << _T(" ms") << newl;

    return true;
}
