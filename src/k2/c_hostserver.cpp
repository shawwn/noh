// (C)2005 S2 Games
// c_hostserver.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_hostserver.h"
#include "c_hostclient.h"
#include "c_world.h"
#include "c_netdriver.h"
#include "c_buffer.h"
#include "c_networkresourcemanager.h"
#include "c_updater.h"
#include "md5.h"
#include "c_voiceserver.h"
#include "c_clientsnapshot.h"
#include "c_date.h"
#include "c_phpdata.h"
#include "c_chatmanager.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_serverchatconnection.h"
#include "c_servermanager.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLF(     svr_broadcast,              false,                      CVAR_SAVECONFIG);
#ifndef NDEBUG // TKTK: svr_debugHeartbeat in debug mode
CVAR_BOOL(      svr_debugHeartbeat,         true);
#else
CVAR_BOOL(      svr_debugHeartbeat,         false);
#endif
CVAR_UINTF(     svr_heartbeatInterval,      60000,                      CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_adminPassword,          "",                         CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_userPassword,           "",                         CVAR_SAVECONFIG);
CVAR_INTR(      svr_gameFPS,                20,                         CVAR_SERVERINFO | CVAR_SAVECONFIG,  1,      60);
CVAR_STRINGF(   svr_version,                "",                         CVAR_SERVERINFO | CVAR_READONLY);
CVAR_INTR(      svr_maxFramesPerHostFrame,  2,                          CVAR_SAVECONFIG,                    1,      5);
CVAR_INTR(      svr_port,                   DEFAULT_SERVER_PORT,        CVAR_SAVECONFIG,                    1024,   65535);
CVAR_STRINGF(   svr_name,                   "Unnamed Server",           CVAR_SERVERINFO | CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_desc,                   "",                         CVAR_SERVERINFO | CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_location,               "",                         CVAR_SERVERINFO | CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_ip,                     "",                         CVAR_SERVERINFO | CVAR_SAVECONFIG);
CVAR_INT(       svr_snapshotCacheSize,      30);
CVAR_INT(       svr_slave,                  -1);

CVAR_FLOATF(    svr_leaverThreshold,        0.08f,                          CVAR_SAVECONFIG);

CVAR_STRINGF(   svr_login,                  "",                             CVAR_SAVECONFIG);
CVAR_STRINGF(   svr_password,               "",                             CVAR_SAVECONFIG);

CVAR_INTF(      svr_maxClients,             -1,                             CVAR_SAVECONFIG);

CVAR_INT(       svr_addServerMS,            0);

CVAR_INT(       svr_sendBuffer,             -1);
CVAR_INT(       svr_recvBuffer,             -1);

CVAR_STRINGF(   svr_managerName,            "",                         CVAR_READONLY);
CVAR_INTF(      svr_managerPort,            0,                          CVAR_READONLY);

CVAR_BOOL(      svr_hitchWarning,           false);
CVAR_BOOLF(     svr_showLongServerFrames,   true,                       CVAR_SAVECONFIG);
CVAR_UINTF(     svr_requestSessionCookieTimeout,    SecToMs(10u),       CVAR_SAVECONFIG);

CVAR_STRINGF(   svr_chatAddress,            "174.36.178.66",            CVAR_SAVECONFIG);
CVAR_INTF(      svr_chatPort,               11031,                      CVAR_SAVECONFIG);

CVAR_UINTF(     svr_connectReqThreshold,    10,                         CVAR_SAVECONFIG);
CVAR_UINTF(     svr_connectReqPeriod,       3000,                       CVAR_SAVECONFIG);

CVAR_UINTF(     svr_maxReminders,               5,                      CVAR_SAVECONFIG);

#ifdef _DEBUG
const tstring SERVER_LIBRARY_NAME(_T("game_debug"));
#else
const tstring SERVER_LIBRARY_NAME(_T("game"));
#endif

CVAR_UINTF(     _svr_ms,                            0,  CVAR_READONLY);
CVAR_UINTF(     _svr_frame,                         0,  CVAR_READONLY);
CVAR_UINTF(     _svr_acked_frame,                   0,  CVAR_READONLY);
CVAR_UINTF(     _svr_paused,                        0,  CVAR_READONLY);
CVAR_UINTF(     _svr_snapshotPoolAllocated,         0,  CVAR_READONLY);
CVAR_UINTF(     _svr_entitySnapshotPoolAllocated,   0,  CVAR_READONLY);

//EXTERN_CVAR_BOOL(svr_requireAuthentication);
//=============================================================================

/*====================
  CHostServer::~CHostServer
  ====================*/
CHostServer::~CHostServer()
{
        Console.Server << _T("Shutting down...") << newl;

#ifndef K2_CLIENT
    m_pHTTPManager->ReleaseRequest(m_pHeartbeat);
#endif

    // Inform manager that we've shutdown
    if (m_bHasManager)
    {
        CPacket pkt;
        pkt.WriteByte(NETCMD_MANAGER_SHUTDOWN);

        m_sockGame.SetSendAddr(svr_managerName, svr_managerPort);
        m_sockGame.SendPacket(pkt);
    }

    // Notify all clients that the server is going down
    SendShutdown();

    m_GameLib.Shutdown();
    m_GameLib.Invalidate();

#ifdef K2_CLIENT
    SAFE_DELETE(m_pClient);
#else
    ClientMap_it it(m_mapClients.begin());
    while (it != m_mapClients.end())
    {
        if (it->second != NULL)
            SAFE_DELETE(it->second);

        STL_ERASE(m_mapClients, it);
    }
#endif

    for (vector<PoolHandle>::iterator it(m_vSnapshots.begin()); it != m_vSnapshots.end(); ++it)
        SAFE_DELETE_SNAPSHOT(*it);

    K2_DELETE(m_pWorld);

#ifndef K2_CLIENT
    K2_DELETE(m_pChatConnection);
#endif

    Console.Execute(_T("ResourceCmdEx context delete hostserver*"));
}


/*====================
  CHostServer::CHostServer
  ====================*/
CHostServer::CHostServer(CHTTPManager *pHTTPManager) :
m_pHTTPManager(pHTTPManager),
m_GameLib(SERVER_LIBRARY_NAME),
m_pWorld(NULL),
m_sMasterServerURL(K2System.GetMasterServerAddress() + "/server_requester.php"),
#ifndef K2_CLIENT
m_pChatConnection(NULL),
#endif

m_uiFrameCount(0),
m_uiLongServerFrameCount(0),
m_uiFrameLength(0),
m_uiFrameAccumulator(0),
m_uiServerTime(0),
m_uiPauseTime(0),

m_vSnapshots(svr_snapshotCacheSize, INVALID_POOL_HANDLE),
m_uiSnapshotBufferPos(-1),

m_uiServerID(-1),
m_uiNextHeartbeat(0),
#ifndef K2_CLIENT
m_pHeartbeat(NULL),
#endif

m_sockGame(_T("SERVER_GAME")),
m_sockManager(_T("SERVER_MANAGER")),

m_bUpdateAvailable(false),
m_bUpdating(false),
m_bUpdateComplete(false),

m_bPaused(false),

m_uiDroppedFrames(0),
m_bInitializeMatchHeartbeat(false),
m_bGameLoading(false),
m_bMatchStarted(false),
m_bPractice(false),
m_bLocal(false),
m_uiMatchupID(INVALID_INDEX),
m_bTournMatch(false),
m_bLeagueMatch(false),
m_uiServerLoad(0),
m_uiLastServerLoad(0),
m_bHasManager(false),
m_eAccess(ACCESS_PUBLIC),
m_bForceInviteOnly(false),
m_iTier(1),
m_unMinPSR(0),
m_unMaxPSR(0),
m_yHostFlags(0),
m_uiLastUpdateCheck(INVALID_TIME),

m_uiLastConnectRequestPeriod(INVALID_TIME),

#ifdef K2_CLIENT
m_pClient(NULL),
#endif

m_uiBytesSent(0),
m_uiPacketsSent(0),
m_uiBytesDropped(0),
m_uiPacketsDropped(0),
m_uiBytesReceived(0),
m_uiPacketsReceived(0),

m_iTournMatchStartTime(INVALID_TIME),
m_iLastCState(-1)
{
    ResetStateData();
}


/*====================
  CHostServer::GetClient
  ====================*/
CClientConnection*  CHostServer::GetClient(int iClientNum)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->GetClientNum() == iClientNum)
        return m_pClient;
    else
        return NULL;
#else
    ClientMap_it findit(m_mapClients.find(iClientNum));
    if (findit == m_mapClients.end())
        return NULL;

    return findit->second;
#endif
}


/*====================
  CHostServer::HasClient
  ====================*/
bool    CHostServer::HasClient(int iClientNum)
{
#ifdef K2_CLIENT
    return m_pClient != NULL && m_pClient->GetClientNum() == iClientNum;
#else
    return m_mapClients.find(iClientNum) != m_mapClients.end();
#endif
}


/*====================
  CHostServer::GetClientNumber
  ====================*/
int     CHostServer::GetClientNumber(int iAccountID)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->IsConnected() && m_pClient->GetAccountID() == iAccountID)
        return m_pClient->GetClientNum();
    else
        return -1;
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second == NULL || !it->second->IsConnected())
            continue;

        if (it->second->GetAccountID() == iAccountID)
            return it->second->GetClientNum();
    }

    return -1;
#endif
}

int     CHostServer::GetClientNumber(const tstring &sName)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->IsConnected() && CompareNoCase(m_pClient->GetName(), sName) == 0)
        return m_pClient->GetClientNum();
    else
        return -1;
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second == NULL || !it->second->IsConnected())
            continue;

        if (CompareNoCase(it->second->GetName(), sName) == 0)
            return it->second->GetClientNum();
    }

    return -1;
#endif
}


/*====================
  CHostServer::GetAccountIDs
  ====================*/
iset    CHostServer::GetAccountIDs()
{
    iset setIDs;

#ifdef K2_CLIENT
    if (m_pClient != NULL)
        setIDs.insert(m_pClient->GetAccountID());
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        setIDs.insert(it->second->GetAccountID());
#endif

    return setIDs;

}


/*====================
  CHostServer::ReleaseClientID
  ====================*/
void    CHostServer::ReleaseClientID(int iClientNumber)
{
#ifndef K2_CLIENT
    for (map<ushort, int>::iterator it(m_mapClientNumbers.begin()); it != m_mapClientNumbers.end(); )
    {
        if (it->second == iClientNumber)
            STL_ERASE(m_mapClientNumbers, it);
        else
            ++it;
    }
#endif
}


/*====================
  CHostServer::GenerateClientID
  ====================*/
int     CHostServer::GenerateClientID(ushort unConnectionID)
{
#ifdef K2_CLIENT
    return 0;
#else
    map<ushort, int>::iterator itFind(m_mapClientNumbers.find(unConnectionID));
    if (itFind != m_mapClientNumbers.end())
        return itFind->second;

    int iClientNumber(0);
    while (true)
    {
        map<ushort, int>::iterator it(m_mapClientNumbers.begin());
        for (; it != m_mapClientNumbers.end(); ++it)
        {
            if (it->second == iClientNumber)
                break;
        }

        if (it == m_mapClientNumbers.end())
            break;

        ++iClientNumber;
    }

    m_mapClientNumbers[unConnectionID] = iClientNumber;
    return m_mapClientNumbers[unConnectionID];
#endif
}


/*====================
  CHostServer::ReauthorizeClient
  ====================*/
void    CHostServer::ReauthorizeClient(CClientConnection *pClient)
{
    if (pClient == NULL)
        return;

#ifndef K2_CLIENT
    for (ClientMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); ++itClient)
    {
        if (itClient->second == pClient)
            continue;

        if (itClient->second->GetAccountID() == pClient->GetAccountID())
            itClient->second->SendAuthRequest();
    }

    for (vector<CClientConnection*>::iterator itClient(m_vProvisionalClients.begin()); itClient != m_vProvisionalClients.end(); ++itClient)
    {
        if ((*itClient)->GetAccountID() == pClient->GetAccountID())
            (*itClient)->SendAuthRequest();
    }
#endif
}


/*====================
  CHostServer::AddClient
  ====================*/
bool    CHostServer::AddClient(const tstring &sAddress, ushort unPort, CPacket &pkt)
{
    CClientConnection *pNewClient(NULL);

    try
    {
#ifdef K2_CLIENT
        if (m_pClient != NULL)
            EX_ERROR(_T("rejected_invalid_request"));
#endif

        // Count connection attempts from each address
        map<wstring, uint>::iterator itFind(m_mapConnectionRequests.find(sAddress));
        if (itFind == m_mapConnectionRequests.end())
            itFind = m_mapConnectionRequests.insert(pair<wstring, uint>(sAddress, 0)).first;

        if (itFind->second >= svr_connectReqThreshold)
            EX_ERROR(_T("rejected_too_many_attempts"));

        ++itFind->second;
        
        // Validate connection request
        wstring sGame(pkt.ReadWString());
        wstring sVersion(pkt.ReadWString());
        uint uiHostID(pkt.ReadInt());
        ushort unConnectionID(pkt.ReadShort());
        wstring sPassWord(pkt.ReadWString());
        wstring sName(pkt.ReadWString());
        wstring sCookie(pkt.ReadWString());
        wstring sIP(pkt.ReadWString());
        wstring sMatchKey(pkt.ReadWString());
        wstring sInvitationCode(pkt.ReadWString());
        byte yHostRequest(pkt.ReadByte());
        if (pkt.HasFaults())
            EX_ERROR(_T("rejected_invalid_request"));

        Console.Server << _T("Connection request from: ") << sAddress << newl
            << _T("Game: ") << sGame << newl
            << _T("Version: ") << sVersion << newl
            << _T("Host ID: ") << uiHostID << newl
            << _T("Connection ID: ") << unConnectionID << newl
            << _T("Password: ") << sPassWord << newl
            << _T("Name: ") << sName << newl
            << _T("Cookie: ") << sCookie << newl
            << _T("IP: ") << sIP << newl
            << _T("Match Key: ") << sMatchKey << newl
            << _T("Invitation: ") << sInvitationCode << newl;

        if (sAddress.empty())
            EX_ERROR(_T("rejected_empty_address"));

        if (K2System.GetGameName() != sGame)
            EX_ERROR(_T("rejected_wrong_game"));
        
        tsvector vsServerVersion(TokenizeString(K2System.GetVersionString(), _T('.')));
        uint uiServerVersion((AtoI(vsServerVersion[0]) << 24) + (AtoI(vsServerVersion[1]) << 16) + (AtoI(vsServerVersion[2]) << 8));
        
        tsvector vsClientVersion(TokenizeString(sVersion, _T('.')));
        uint uiClientVersion((AtoI(vsClientVersion[0]) << 24) + (AtoI(vsClientVersion[1]) << 16) + (AtoI(vsClientVersion[2]) << 8));
        
        if (uiServerVersion != uiClientVersion)
            EX_ERROR(_T("rejected_wrong_version"));
        
        if (false /* TODO: Check for banned clients */)
            EX_ERROR(_T("rejected_banned"));

        uint uiFlags(0);

#ifdef K2_CLIENT
        uiFlags = CLIENT_CONNECTION_LOCAL | CLIENT_CONNECTION_GAME_HOST;
        unConnectionID = 0;
        if (Host.GetID() != uiHostID || (yHostRequest & BIT(0)))
            EX_ERROR(_T("rejected_invalid_request"));
        if (!m_bPractice)
            EX_ERROR(_T("rejected_invalid_request"));
#else
        if (Host.GetID() == uiHostID && !K2System.IsDedicatedServer())
            uiFlags = CLIENT_CONNECTION_LOCAL;
        else if (!svr_adminPassword.empty() && sPassWord == svr_adminPassword)
            uiFlags = CLIENT_CONNECTION_ADMIN;
        else if (!svr_userPassword.empty() && sPassWord != svr_userPassword)
            EX_ERROR(_T("rejected_invalid_password"));

        if (m_bPractice != ((yHostRequest & BIT(1)) != 0))
            EX_ERROR(_T("rejected_invalid_request"));

        // First connection to an idle server is automatically promoted to host
        if (m_pWorld != NULL && 
            !m_pWorld->IsLoaded() &&
            !svr_requireAuthentication &&
            m_mapClients.empty() &&
            m_vProvisionalClients.empty())
        {
            uiFlags |= CLIENT_CONNECTION_GAME_HOST;
        }

        if (unConnectionID == 0)
            unConnectionID = K2System.GetRandomSeed32() & USHRT_MAX;
        if (unConnectionID == 0)
            unConnectionID = USHRT_MAX;

        // Resolve collisions in connection ID
        bool bNoCollisions(false);
        while (!bNoCollisions)
        {
            bNoCollisions = true;
            for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
            {
                if (it->second->GetConnectionID() == unConnectionID)
                {
                    unConnectionID = K2System.GetRandomSeed32() & USHRT_MAX;
                    bNoCollisions = false;
                    break;
                }
            }
        }

        // No match keys for matches that have already started
        if (m_pWorld != NULL && m_pWorld->IsLoaded())
            sMatchKey.clear();

        // Check for a name conflict
        uint uiConflictCounter(0);
        tstring sBaseName(sName.substr(0, MAX_NAME_LENGTH));
        for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            if (!it->second->GetMatchKey().empty() || it->second->HasFlags(CLIENT_CONNECTION_GAME_HOST))
                sMatchKey.clear();

            const tstring &sCheckName(it->second->GetName());
            if (sName != sCheckName)
                continue;

            sName = sBaseName.substr(0, MAX_NAME_LENGTH - 3) + ParenStr(XtoA(++uiConflictCounter));
            it = m_mapClients.begin();
        }
        for (vector<CClientConnection*>::iterator it(m_vProvisionalClients.begin()); it != m_vProvisionalClients.end(); ++it)
        {
            if (!(*it)->GetMatchKey().empty() || (*it)->HasFlags(CLIENT_CONNECTION_GAME_HOST))
                sMatchKey.clear();

            const tstring &sCheckName((*it)->GetName());
            if (sName != sCheckName)
                continue;

            sName = sBaseName.substr(0, MAX_NAME_LENGTH - 3) + ParenStr(XtoA(++uiConflictCounter));
            it = m_vProvisionalClients.begin();
        }

        if (m_pWorld != NULL && 
            !m_pWorld->IsLoaded() &&
            (!m_mapClients.empty() ||
            !m_vProvisionalClients.empty()))
        {
            if (!(uiFlags & CLIENT_CONNECTION_GAME_HOST) && sMatchKey.empty())
                EX_ERROR(_T("rejected_server_taken"));
        }

        if ((yHostRequest & BIT(0)) && !(uiFlags & CLIENT_CONNECTION_GAME_HOST || !sMatchKey.empty()))
            EX_ERROR(_T("rejected_not_host"));
#endif

        // Create a new client connection
        pNewClient = K2_NEW(ctx_HostServer,  CClientConnection)(this, Host.GetHTTPManager(), sAddress, unPort, m_sockGame);
        if (pNewClient == NULL)
            EX_ERROR(_T("rejected_server_error"));
        
        pNewClient->SetConnectionID(unConnectionID);
        pNewClient->SetName(sName);
        pNewClient->SetCookie(sCookie);
        pNewClient->SetMatchKey(sMatchKey);
        pNewClient->SetInvitationCode(sInvitationCode);
        pNewClient->SetFlags(uiFlags);
        pNewClient->ProcessNetSettings(pkt);
        pNewClient->SetPublicAddress(sIP);

#ifdef K2_CLIENT
        Console.Server << _T("New local connection") << newl;

        m_pClient = pNewClient;
        m_pClient->AuthSuccess(0, sName);
        GetGameLib().AddClient(m_pClient);
#else
        m_vProvisionalClients.push_back(pNewClient);

        Console.Server << _T("New client connection: ") << unConnectionID << _T(", ") << sIP << _T(":") << unPort;

        if ((uiFlags & CLIENT_CONNECTION_LOCAL) == CLIENT_CONNECTION_LOCAL)
            Console.Server << _T(" [Local]");
        else if ((uiFlags & CLIENT_CONNECTION_ADMIN) == CLIENT_CONNECTION_ADMIN)
            Console.Server << _T(" [Admin]");
        else if (uiFlags & CLIENT_CONNECTION_GAME_HOST)
            Console.Server << _T(" [Host]");
        
        Console.Server << newl;


        m_pChatConnection->SendStatusUpdate();
#endif
        return true;
    }
    catch (CException &ex)
    {
        SAFE_DELETE(pNewClient);
        RejectConnection(sAddress, unPort, ex.GetMsg());
        return false;
    }
}


/*====================
  CHostServer::RejectConnection
  ====================*/
void    CHostServer::RejectConnection(const tstring &sAddress, ushort unPort, const tstring &sReason)
{
    CPacket pktDeny;
    pktDeny << NETCMD_KICK << sReason;
    m_sockGame.SetSendAddr(sAddress, unPort);
    m_sockGame.SendPacket(pktDeny);
}


#ifdef K2_CLIENT
/*====================
  CHostServer::RemoveClient
  ====================*/
void    CHostServer::RemoveClient(int iClientNum, const tstring &sReason)
{
    if (m_pClient == NULL || m_pClient->GetClientNum() != iClientNum)
        return;

    Console.Server << _T("Client has been removed.") << newl;

    m_GameLib.RemoveClient(iClientNum, sReason);

    if (m_pClient != NULL)
        SAFE_DELETE(m_pClient);

    Console.AddCmdBuffer(_T("StopServer"));
}
#else
/*====================
  CHostServer::RemoveClient
  ====================*/
ClientMap_it    CHostServer::RemoveClient(ClientMap_it itClient, const tstring &sReason)
{
    if (itClient == m_mapClients.end())
        return itClient;

    if (itClient->second != NULL)
    {
        int iClientNum(itClient->second->GetClientNum());

        Console.Server << _T("Client #") << iClientNum << _T(" has been removed.") << newl;
    
        m_GameLib.RemoveClient(iClientNum, sReason);

        // Re-grab the client iterator incase m_GameLib.RemoveClient deletes something
        itClient = m_mapClients.find(iClientNum);
        if (itClient != m_mapClients.end())
        {
            itClient->second->Disconnect(sReason);
            SAFE_DELETE(itClient->second);
        }

        // Immediately send a new heartbeat if this is an idle server when a client disconnects
        if (GetWorld() == NULL || !GetWorld()->IsLoaded())
            m_uiNextHeartbeat = 0;
    }

    if (itClient != m_mapClients.end())
        STL_ERASE(m_mapClients, itClient);

    return itClient;
}

void    CHostServer::RemoveClient(int iClientNum, const tstring &sReason)
{
    ClientMap_it itClient(m_mapClients.find(iClientNum));
    RemoveClient(itClient, sReason);
}
#endif


/*====================
  CHostServer::GetClientConnection
  ====================*/
CClientConnection*  CHostServer::GetClientConnection(const tstring &sAddress, ushort unConnectionID)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->GetAddress() == sAddress && m_pClient->GetConnectionID() == unConnectionID)
        return m_pClient;
    else
        return NULL;
#else
    // Check connected clients
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (sAddress == it->second->GetAddress() && (unConnectionID == it->second->GetConnectionID()))
            return it->second;
    }

    // Check provisional clients
    for (vector<CClientConnection*>::iterator it(m_vProvisionalClients.begin()); it != m_vProvisionalClients.end(); ++it)
    {
        if (sAddress == (*it)->GetAddress() && unConnectionID == (*it)->GetConnectionID())
            return *it;
    }

    return NULL;
#endif
}


/*====================
  CHostServer::ProcessManagerPacket
  ====================*/
void    CHostServer::ProcessManagerPacket(CPacket &pkt)
{
    // Handle packets from non-clients
    bool bProcessed(false);
    byte cmd(pkt.ReadByte());
    switch (cmd)
    {
    case NETCMD_MANAGER_SLEEP:
        Host.SetSleeping(true);
        m_uiNextHeartbeat = 0;
        bProcessed = true;
#ifndef K2_CLIENT
        m_pChatConnection->SendStatusUpdate();
#endif
        break;

    case NETCMD_MANAGER_WAKE:
        Host.SetSleeping(false);
        m_uiNextHeartbeat = 0;
        bProcessed = true;
#ifndef K2_CLIENT
        m_pChatConnection->SendStatusUpdate();
#endif
        break;

    case NETCMD_MANAGER_SHUTDOWN_SLAVE:
        K2System.RestartOnExit(false);
        K2System.Exit(0);
        bProcessed = true;
        break;

    case NETCMD_MANAGER_RESTART_SLAVE:
        K2System.RestartOnExit(true);
        K2System.Exit(0);
        bProcessed = true;
        break;

    case NETCMD_MANAGER_CHAT:
        {
            wstring sMsg(pkt.ReadWString());
            Console.Execute(L"ServerChat " + QuoteStr(sMsg));
            bProcessed = true;
        }
        break;
    }

    if (!bProcessed)
        Console.Net << _T("Invalid packet received from server manager ") << m_sockGame.GetRecvAddrName()
                << _T(":") << m_sockGame.GetRecvPort() << _T(", cmd ") << XtoA(cmd, FMT_PADZERO, 4, 16) << newl;
}


/*====================
  CHostServer::ReadPackets
  ====================*/
void    CHostServer::ReadPackets()
{
    PROFILE("CHostServer::ReadPackets");

    uint uiNumProcessed(0);
    
    CPacket pkt;
    while (m_sockGame.ReceivePacket(pkt) > 0)
    {
        m_uiBytesReceived += pkt.GetLength();
        ++m_uiPacketsReceived;

        if (m_sockGame.GetRecvAddrName() == svr_managerName && m_sockGame.GetRecvPort() == svr_managerPort)
        {
            ProcessManagerPacket(pkt);
            continue;
        }

        // Ignore other packets if we're sleeping
        if (Host.IsSleeping())
            continue;

        // Check if this is from a connected client
        ushort unConnectionID(pkt.GetConnectionID());
        CClientConnection *pClient(GetClientConnection(m_sockGame.GetRecvAddrName(), unConnectionID));
        if (pClient != NULL)
        {
            pClient->CheckPort(m_sockGame.GetRecvPort());
            pClient->ProcessPacket(pkt);

            if (!pClient->IsConnected())
                RemoveClient(pClient->GetClientNum());

            continue;
        }

        // Handle packets from non-clients
        bool bProcessed(false);
        byte cmd(pkt.ReadByte());
        switch (cmd)
        {
        // Conncetion request
        case NETCMD_CONNECT:
            bProcessed = AddClient(m_sockGame.GetRecvAddrName(), m_sockGame.GetRecvPort(), pkt);
            break;

#ifndef K2_CLIENT
        case NETCMD_INFO_REQUEST:
            {
                // Echo the challange to prevent old or bogus reponses from
                // showing up on a client's server list

                ushort unChallange(pkt.ReadShort());

                Console.Net << _T("Sending info to ") << m_sockGame.GetRecvAddrName() << newl;

                CPacket pktInfo;
                pktInfo.WriteByte(NETCMD_SERVER_INFO);
                pktInfo.WriteShort(unChallange);
                pktInfo.WriteString(svr_name);
                pktInfo.WriteByte(GetNumConnectedClients());
                pktInfo.WriteByte(GetMaxPlayers());
                pktInfo.WriteString(m_pWorld == NULL ? TSNULL : m_pWorld->GetName());
                pktInfo.WriteString(K2System.GetVersionString());
                pktInfo.WriteByte(byte(GetServerAccess()));
                pktInfo.WriteByte(byte(m_iTier));               
                pktInfo.WriteByte(m_yHostFlags);
                pktInfo.WriteByte(byte(m_bMatchStarted));
                pktInfo.WriteString(svr_location);
                                
                m_GameLib.GetServerInfo(pktInfo);
                
                // sending m_yArrangedType
                if (IsArrangedMatch())
                    pktInfo.WriteByte(byte(1));
                else if (IsTournMatch())
                    pktInfo.WriteByte(byte(2));
                else if (IsLeagueMatch())
                    pktInfo.WriteByte(byte(3));
                else
                    pktInfo.WriteByte(byte(0));
                    
                pktInfo.WriteShort(m_unMinPSR);
                pktInfo.WriteShort(m_unMaxPSR);                         

                m_sockGame.SetSendAddr(m_sockGame.GetRecvAddrName(), m_sockGame.GetRecvPort());
                m_sockGame.SendPacket(pktInfo);

                bProcessed = true;
            }
            break;

        case NETCMD_RECONNECT_INFO_REQUEST:
            {
                // Echo the challange to prevent old or bogus reponses from
                // showing up on a client's server list

                uint uiMatchID(pkt.ReadInt(-1));
                uint uiAccountID(pkt.ReadInt(-1));
                ushort uiConnectionID(pkt.ReadShort(-1));

                Console.Net << _T("Sending info to ") << m_sockGame.GetRecvAddrName() << newl;

                CPacket pktInfo;
                pktInfo.WriteByte(NETCMD_RECONNECT_INFO_RESPONSE);

                m_GameLib.GetReconnectInfo(pktInfo, uiMatchID, uiAccountID, uiConnectionID);

                m_sockGame.SetSendAddr(m_sockGame.GetRecvAddrName(), m_sockGame.GetRecvPort());
                m_sockGame.SendPacket(pktInfo);

                bProcessed = true;
            }
            break;

        /*
        case NETCMD_CREATE_TOURN_MATCH:
            {
                uint uiMatchID(pkt.ReadInt());
                uint uiChallenge(pkt.ReadInt());
                wstring sKey(pkt.ReadWString());
                wstring sName(pkt.ReadWString());
                wstring sSettings(pkt.ReadWString());
                uint uiMatchStartTime(pkt.ReadInt());

                if (pkt.HasFaults() || sKey.empty() || sName.empty())
                    break;

                if (sKey != _T("JKNGKFVNEOSNUENSVUVJ"))
                    break;

                byte yTeam1Size(pkt.ReadByte(0));
                for (byte yIndex(0); yIndex < yTeam1Size; ++yIndex)
                {
                    uint uiAccountID(pkt.ReadInt(-1));
                    if (uiAccountID != -1)
                        AddToRoster(uiAccountID, 1); // TEAM_1

                    if (pkt.HasFaults())
                        break;
                }

                byte yTeam2Size(pkt.ReadByte(0));
                for (byte yIndex(0); yIndex < yTeam2Size; ++yIndex)
                {
                    uint uiAccountID(pkt.ReadInt(-1));
                    if (uiAccountID != -1)
                        AddToRoster(uiAccountID, 2); // TEAM_2

                    if (pkt.HasFaults())
                        break;
                }
                
                byte yRefereeSize(pkt.ReadByte(0));
                for (byte yIndex(0); yIndex < yRefereeSize; ++yIndex)
                {
                    uint uiAccountID(pkt.ReadInt(-1));
                    if (uiAccountID != -1)
                        AddToRoster(uiAccountID, -3); // TEAM_REFEREE

                    if (pkt.HasFaults())
                        break;
                }
                
                byte ySpectatorSize(pkt.ReadByte(0));
                for (byte yIndex(0); yIndex < ySpectatorSize; ++yIndex)
                {
                    uint uiAccountID(pkt.ReadInt(-1));
                    if (uiAccountID != -1)
                        AddToRoster(uiAccountID, 0); // TEAM_SPECTATOR

                    if (pkt.HasFaults())
                        break;
                }                           

                if (pkt.HasFaults())
                    break;

                bProcessed = true;

                if (m_pWorld != NULL && m_pWorld->IsLoaded() || m_pWorld->IsLoading())
                {
                    Console << L"Ignoring NETCMD_CREATE_TOURN_MATCH because a match is already loaded" << newl;
                    break;
                }

                SetTournMatch(true);
                m_iTournMatchStartTime = SecToMs(uiMatchStartTime);

                if (StartGame(sName, sSettings))
                {
                    CPacket pkt;
                    pkt << NETCMD_TOURN_MATCH_READY << uiMatchID << uiChallenge;

                    m_sockGame.SetSendAddr(m_sockGame.GetRecvAddrName(), m_sockGame.GetRecvPort());
                    m_sockGame.SendPacket(pkt);
                }
            }
            break;
            */
#endif
        }

        if (!bProcessed)
        {
            Console.Net << _T("Packet received from non-client ") << m_sockGame.GetRecvAddrName()
                    << _T(":") << m_sockGame.GetRecvPort() << _T(", cmd ") << XtoA(cmd, FMT_PADZERO, 4, 16) << newl;
        }
        else
        {
            ++uiNumProcessed;
            if (uiNumProcessed >= MAX_INCOMING_PACKETS_PER_FRAME)
            {
                Console.Warn << _T("CHostServer::ReadPackets() - Packet flood") << newl;
                return;
            }
        }
    }

    // Process any newly valid out of sequence reliable packets
#ifdef K2_CLIENT
    if (m_pClient != NULL)
    {
        m_pClient->ProcessOutOfSequencePackets();
        if (!m_pClient->IsConnected())
            RemoveClient(m_pClient->GetClientNum());
    }
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); )
    {
        it->second->ProcessOutOfSequencePackets();
        if (!it->second->IsConnected())
            it = RemoveClient(it);
        else
            ++it;
    }
#endif
}


/*====================
  CHostServer::UpdateClients
  ====================*/
void    CHostServer::UpdateClients()
{
    PROFILE("CHostServer::UpdateClients");

    // Send data back to each client and make sure they haven't timed out
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->IsConnected())
        m_pClient->WriteClientPackets(svr_gameFPS);
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end();)
    {
        if (!it->second->IsConnected())
        {
            ++it;
            continue;
        }

        // notify the game lib if the client is unresponsive for a period of time.
        if (it->second->IsTimingOut())
        {
            if (!it->second->HasFlags(CLIENT_CONNECTION_TIMING_OUT))
            {
                int iClientNum(it->second->GetClientNum());
                Console.Server << _T("Client #") << iClientNum
                    << _T(" timing out")
                    << _T(" @ ") << Host.GetSystemTime() << newl;
                it->second->SetFlags(CLIENT_CONNECTION_TIMING_OUT);

                m_GameLib.ClientTimingOut(iClientNum);
            }
        }
        else
        {
            it->second->RemoveFlags(CLIENT_CONNECTION_TIMING_OUT);
        }

        // notify the game lib that the client has timed out.
        if (it->second->CheckTimeout())
        {
            it = RemoveClient(it, _T("Connection timed out"));
            continue;
        }

        it->second->WriteClientPackets(svr_gameFPS);
        ++it;
    }
#endif
}


/*====================
  CHostServer::AddStateString
  ====================*/
ushort  CHostServer::AddStateString()
{
    if (m_vStateStrings.size() >= USHRT_MAX)
    {
        Console.Err << _T("CHostServer::AddStateString() - Exceeded max state strings") << newl;
        return STATE_STRING_NULL;
    }

    CStateString ss;
    m_vStateStrings.push_back(ss);
    return ushort(m_vStateStrings.size() - 1);
}


/*====================
  CHostServer::GetStateString
  ====================*/
CStateString&   CHostServer::GetStateString(ushort unID)
{
    try
    {
        if (unID >= m_vStateStrings.size())
            EX_ERROR(_T("Invalid state string ID: ") + XtoA(unID));

        return m_vStateStrings[unID];
    }
    catch (CException &ex)
    {
        static CStateString ssInvalid;
        ex.Process(_T("CHostServer::GetStateString() - "));
        return ssInvalid;
    }
}


/*====================
  CHostServer::SendStateString
  ====================*/
void    CHostServer::SendStateString(int iClientNum, ushort unID)
{
    PROFILE("CHostServer::SendStateString");

    try
    {
        if (GetStateString(unID).IsEmpty())
            return;

        CClientConnection *pClient(GetClient(iClientNum));
        if (pClient == NULL)
            return;

        pClient->AddStringToUpdateQueue(unID, GetStateString(unID));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostServer::SendStateString() - "), NO_THROW);
    }
}


/*====================
  CHostServer::UpdateStateStrings
  ====================*/
void    CHostServer::UpdateStateStrings()
{
    PROFILE("CHostServer::UpdateStateStrings");

    try
    {
        if (Host.IsReplay())
            return;

        if (ICvar::IsTransmitModified())
        {
            PROFILE("Transmit Cvars");

            CStateString &ssLastTransmitCvars(GetStateString(STATE_STRING_CVAR_SETTINGS));
            CStateString ssTransmitCvarsDiff;
            ICvar::SetTransmitModified(false);

            ICvar::GetTransmitCvars(ssTransmitCvarsDiff);
            m_GameLib.SetGamePointer();
            m_GameLib.StateStringChanged(STATE_STRING_CVAR_SETTINGS, ssTransmitCvarsDiff);
            ssLastTransmitCvars.GetDifference(ssTransmitCvarsDiff);

#ifdef K2_CLIENT
            if (m_pClient != NULL)
                m_pClient->AddStringToUpdateQueue(STATE_STRING_CVAR_SETTINGS, ssTransmitCvarsDiff);
#else
            for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                it->second->AddStringToUpdateQueue(STATE_STRING_CVAR_SETTINGS, ssTransmitCvarsDiff);
#endif

            ICvar::GetTransmitCvars(ssLastTransmitCvars);
        }

        if (ICvar::IsServerInfoModified())
        {
            PROFILE("Server Info");

            CStateString &ssLastServerInfo(GetStateString(STATE_STRING_SERVER_INFO));
            CStateString ssServerInfoDiff;
            ICvar::SetServerInfoModified(false);

            ICvar::GetServerInfo(ssServerInfoDiff);
            m_GameLib.SetGamePointer();
            m_GameLib.StateStringChanged(STATE_STRING_SERVER_INFO, ssServerInfoDiff);
            ssLastServerInfo.GetDifference(ssServerInfoDiff);
            
#ifdef K2_CLIENT
            if (m_pClient != NULL)
                m_pClient->AddStringToUpdateQueue(STATE_STRING_CVAR_SETTINGS, ssServerInfoDiff);
#else
            for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                it->second->AddStringToUpdateQueue(STATE_STRING_CVAR_SETTINGS, ssServerInfoDiff);
#endif

            ICvar::GetServerInfo(ssLastServerInfo);
        }

        if (NetworkResourceManager.IsModified())
        {
            PROFILE("Network Resources");

            CStateString &ssLastNetworkResources(GetStateString(STATE_STRING_RESOURCES));
            CStateString ssNetworkResourcesDiff;
            NetworkResourceManager.SetModified(false);

            NetworkResourceManager.GetStateString(ssNetworkResourcesDiff);
            m_GameLib.SetGamePointer();
            m_GameLib.StateStringChanged(STATE_STRING_RESOURCES, ssNetworkResourcesDiff);
            ssLastNetworkResources.GetDifference(ssNetworkResourcesDiff);
            
#ifdef K2_CLIENT
            if (m_pClient != NULL)
                m_pClient->AddStringToUpdateQueue(STATE_STRING_RESOURCES, ssNetworkResourcesDiff);
#else
            for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                it->second->AddStringToUpdateQueue(STATE_STRING_RESOURCES, ssNetworkResourcesDiff);
#endif

            NetworkResourceManager.GetStateString(ssLastNetworkResources);
        }

        if (NetworkResourceManager.IsStringListModified())
        {
            PROFILE("Network Strings");

            CStateString &ssNetworkStrings(GetStateString(STATE_STRING_ENTITIES));
            CStateString ssNetworkStringsDiff;
            NetworkResourceManager.SetStringListModified(false);

            NetworkResourceManager.UpdateEntityStateString(ssNetworkStringsDiff);
            m_GameLib.SetGamePointer();
            m_GameLib.StateStringChanged(STATE_STRING_ENTITIES, ssNetworkStringsDiff);
            ssNetworkStrings.GetDifference(ssNetworkStringsDiff);
            
#ifdef K2_CLIENT
            if (m_pClient != NULL)
                m_pClient->AddStringToUpdateQueue(STATE_STRING_ENTITIES, ssNetworkStringsDiff);
#else
            for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
                it->second->AddStringToUpdateQueue(STATE_STRING_ENTITIES, ssNetworkStringsDiff);
#endif

            NetworkResourceManager.UpdateEntityStateString(ssNetworkStrings);
        }

        // Check each state block
        for (StateBlockVector_it itBlock(m_vStateBlocks.begin()); itBlock != m_vStateBlocks.end(); ++itBlock)
        {
            ushort unID(itBlock - m_vStateBlocks.begin());

            if (m_vStateBlockModCounts[unID] != itBlock->GetModifiedCount())
            {
                m_GameLib.StateBlockChanged(unID, itBlock->GetBuffer());
            }

            m_vStateBlockModCounts[unID] = itBlock->GetModifiedCount();

#ifdef K2_CLIENT
            if (m_pClient != NULL)
                if (itBlock->GetModifiedCount() != m_pClient->GetStateBlockModCount(unID))
                {
                    m_pClient->AddBlockToUpdateQueue(unID, *itBlock);
                    m_GameLib.StateBlockChanged(unID, itBlock->GetBuffer());
                }
#else
            for (ClientMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); ++itClient)
            {
                if (itBlock->GetModifiedCount() != itClient->second->GetStateBlockModCount(unID))
                    itClient->second->AddBlockToUpdateQueue(unID, *itBlock);
            }
#endif
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostServer::UpdateStateStrings() - "), NO_THROW);
    }
}


/*====================
  CHostServer::AddStateBlock
  ====================*/
void    CHostServer::AddStateBlock(ushort unID)
{
    if (m_vStateBlocks.size() <= unID)
        m_vStateBlocks.resize(unID + 1);

    m_vStateBlockModCounts.resize(m_vStateBlocks.size());
}


/*====================
  CHostServer::GetStateBlock
  ====================*/
CStateBlock&    CHostServer::GetStateBlock(ushort unID)
{
    AddStateBlock(unID);

    if (unID >= m_vStateBlocks.size())
    {
        Console.Err << L"Invalid state block ID: " << unID << newl;
        static CStateBlock blockInvalid;
        return blockInvalid;
    }

    return m_vStateBlocks[unID];
}


/*====================
  CHostServer::SendStateBlock
  ====================*/
void    CHostServer::SendStateBlock(int iClientNum, ushort unID)
{
    PROFILE("CHostServer::SendStateBlock");

    try
    {
        if (GetStateBlock(unID).IsEmpty())
            return;

        CClientConnection *pClient(GetClient(iClientNum));
        if (pClient == NULL)
            return;

        pClient->AddBlockToUpdateQueue(unID, GetStateBlock(unID));
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostServer::SendStateBlock() - "), NO_THROW);
    }
}


/*====================
  CHostServer::RequestSessionCookie
  ====================*/
#ifndef K2_CLIENT
bool    CHostServer::RequestSessionCookie(bool bNew)
{
    if (GetPractice())
        return false;

    if (svr_login.empty() || svr_password.empty())
        return false;

    if (bNew)
        m_sSessionCookie.clear();

    if (!m_sSessionCookie.empty())
        return true;

    Console << _T("Requesting new session cookie") << newl;

    CHTTPRequest *pNewSessionRequest(Host.GetHTTPManager()->SpawnRequest());
    if (pNewSessionRequest == NULL)
        return false;

    pNewSessionRequest->SetTargetURL(m_sMasterServerURL);
    pNewSessionRequest->AddVariable(L"f", L"new_session");
    pNewSessionRequest->AddVariable(L"login", svr_login);
    pNewSessionRequest->AddVariable(L"pass", TStringToUTF8(svr_password));
    pNewSessionRequest->AddVariable(L"port", svr_port.GetString());
    pNewSessionRequest->AddVariable(L"name", svr_name);
    pNewSessionRequest->AddVariable(L"desc", svr_desc);
    pNewSessionRequest->AddVariable(L"location", svr_location);
    pNewSessionRequest->AddVariable(L"ip", svr_ip);
    pNewSessionRequest->SetTimeout(MsToSec(svr_requestSessionCookieTimeout));
    pNewSessionRequest->SendPostRequest();
    pNewSessionRequest->Wait();

    const CPHPData phpResponse(pNewSessionRequest->GetResponse());

    if (!pNewSessionRequest->WasSuccessful() || !phpResponse.IsValid())
    {
        Console << _T("Session cookie request failed!") << newl;
        Host.GetHTTPManager()->ReleaseRequest(pNewSessionRequest);
        m_sSessionCookie.clear();
        return false;
    }

    const CPHPData *pError(phpResponse.GetVar(_T("error")));
    if (pError != NULL)
    {
        const CPHPData *pErrorCode(pError->GetVar(0));
        Console << _T("Session cookie request failed: ") << (pErrorCode == NULL ? _T("Unknown error") : pErrorCode->GetString()) << newl;
        Host.GetHTTPManager()->ReleaseRequest(pNewSessionRequest);
        m_sSessionCookie.clear();
        return false;
    }

    Host.GetHTTPManager()->ReleaseRequest(pNewSessionRequest);

    m_uiNextHeartbeat = 0;
    m_iLastCState = -1;

    m_uiServerID = phpResponse.GetInteger(_T("server_id"));
    m_sSessionCookie = phpResponse.GetString(_T("session"));

    if (m_sSessionCookie.empty())
        Console << _T("No session cookie returned!") << newl;
    else
        Console << _T("New session cookie [") << m_sSessionCookie << _T("]") << newl;

    m_fLeaverThreshold = phpResponse.GetFloat(_T("leaverthreshold"));
    
    return !m_sSessionCookie.empty();
}
#endif


/*====================
  CHostServer::BreakSessionCookie
  ====================*/
#ifndef K2_CLIENT
void    CHostServer::BreakSessionCookie()
{
    m_sSessionCookie.clear();

    for (int i(0); i < 32; ++i)
        m_sSessionCookie += XtoA(M_Randnum(0, 15), FMT_NOPREFIX, 0, 16);
}
#endif

#ifndef K2_CLIENT
CVAR_INT(_svr_forcecstate, -1);

/*====================
  CHostServer::SendHeartbeat
  ====================*/
void    CHostServer::SendHeartbeat()
{
    if (GetPractice())
        return;

    if (svr_broadcast.IsModified())
    {
        svr_broadcast.SetModified(false);
        m_uiNextHeartbeat = 0;
        if (svr_broadcast)
            RequestSessionCookie(false);
    }

    if (!svr_broadcast || m_bUpdating)
        return;

    if (m_pHeartbeat != NULL && !m_pHeartbeat->IsActive())
    {
        if (m_pHeartbeat->WasSuccessful())
        {
            // Finished request
            CPHPData phpResponse(m_pHeartbeat->GetResponse());

            if (svr_debugHeartbeat)
            {
                Console << _T("Sent heartbeat: ") << newl;
                Console << _T("Reply:" ) << m_pHeartbeat->GetResponse() << newl;
                phpResponse.Print();
            }
            else
            {
                Console << _T("Sent heartbeat") << newl;
            }

            if (phpResponse.GetString(_T("authenticate")) == _T("Failed to authenticate B1"))
            {
                Console << _T("Heartbeat: ") << _T("Failed to authenticate B1") << newl;

                // Reinitialize match heartbeat if required
                if (Host.IsSleeping() || GetWorld() == NULL || !GetWorld()->IsLoaded())
                {
                    Console << _T("Resetting session cookie...") << newl;

                    m_sSessionCookie.clear();
                }
                else if (!m_bMatchStarted)
                {
                    Console << _T("Killing unstarted match...") << newl;

                    m_sSessionCookie.clear();

                    for (ClientMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); )
                    {
                        itClient->second->Disconnect(_T("disconnect_invalid_server_session"));
                        SAFE_DELETE(itClient->second);
                        STL_ERASE(m_mapClients, itClient);
                    }

                    m_GameLib.Reset();

                    if (GetWorld() != NULL)
                        GetWorld()->Free();
                }
            }
        }

        m_pHTTPManager->ReleaseRequest(m_pHeartbeat);
        m_pHeartbeat = NULL;
    }

    if (m_uiServerTime + m_uiPauseTime < m_uiNextHeartbeat)
        return;

    if (m_sSessionCookie.empty())
        return;

    //bool bInGame(m_GameLib.GetMatchTime() != 0);

    // If we aren't forcing to a certain IP, use "NA" as our
    // IP, which will let the master server detect it.
    tstring sIP(ICvar::GetString(_T("net_forceIP")));
    if (sIP.empty())
        sIP = _T("NA");

    tstring sGameTime(_T("00:00:00"));
    uint uiTime(m_GameLib.GetMatchTime());
    if (uiTime != 0)
    {
        uint uiHours(uiTime / MS_PER_HR);
        uint uiMinutes((uiTime % MS_PER_HR) / MS_PER_MIN);
        uint uiSeconds((uiTime % MS_PER_MIN) / MS_PER_SEC);
        sGameTime = XtoA(uiHours, FMT_PADZERO, 2) + _T(":") + XtoA(uiMinutes, FMT_PADZERO, 2) + _T(":") + XtoA(uiSeconds, FMT_PADZERO, 2);
    }

    tstring sMap;
    if (GetWorld() != NULL)
        sMap = GetWorld()->GetName();

    if (m_pHeartbeat != NULL)
    {
        switch (m_pHeartbeat->GetStatus())
        {
        case HTTP_REQUEST_ERROR:    Console << _T("Reset failed heartbeat") << newl; break;
        case HTTP_REQUEST_IDLE:     Console << _T("Reset idle heartbeat") << newl; break;
        case HTTP_REQUEST_SENDING:  Console << _T("Reset active heartbeat") << newl; break;
        case HTTP_REQUEST_SUCCESS:  Console << _T("Reset completed heartbeat") << newl; break;
        }
        
        m_pHTTPManager->ReleaseRequest(m_pHeartbeat);
        m_pHeartbeat = NULL;
    }

    m_pHeartbeat = m_pHTTPManager->SpawnRequest();
    if (m_pHeartbeat == NULL)
        return;

    m_pHeartbeat->SetTargetURL(m_sMasterServerURL);
    m_pHeartbeat->ClearVariables();
    m_pHeartbeat->AddVariable(L"f", L"set_online");
    m_pHeartbeat->AddVariable(L"session", m_sSessionCookie);
    m_pHeartbeat->AddVariable(L"num_conn", int(GetNumConnectedClients()));
    m_pHeartbeat->AddVariable(L"cgt", sGameTime);

    if (IsArrangedMatch() || m_bForceInviteOnly)
        m_pHeartbeat->AddVariable(L"private", int(ACCESS_INVITEONLY));
    else
        m_pHeartbeat->AddVariable(L"private", int(GetServerAccess()));

    int iNewCState(-1);

    if (Host.IsSleeping())
    {
        iNewCState = 0;
        m_pHeartbeat->AddVariable(L"c_state", iNewCState);
    }
    else if (GetWorld() == NULL || !GetWorld()->IsLoaded())
    {
        iNewCState = 1;
        m_pHeartbeat->AddVariable(L"c_state", iNewCState);
    }
    else
    {
        iNewCState = m_bMatchStarted ? 3 : 2;
        m_pHeartbeat->AddVariable(L"c_state", iNewCState);

        if (m_bInitializeMatchHeartbeat)
        {
            m_GameLib.GetHeartbeatInfo(m_pHeartbeat);
            m_pHeartbeat->AddVariable(L"league", 0);
            m_pHeartbeat->AddVariable(L"tier", 0);  // Depreciated
            if (GetNoStats())
                m_pHeartbeat->AddVariable(L"option[no_stats]", 1);
            if (GetNoLeaver())
                m_pHeartbeat->AddVariable(L"option[nl]", 1);

            m_bInitializeMatchHeartbeat = false;
        }
    }

    m_pHeartbeat->AddVariable(L"prev_c_state", m_iLastCState);
    m_iLastCState = iNewCState;

    if (svr_debugHeartbeat)
        Console << _T("Sending heartbeat...") << newl;

    // Report the heartbeat info to master here
    m_pHeartbeat->SendPostRequest();
    
    // Report server info here for status updates, in game score tracking and server monitoring
    m_pChatConnection->SendStatusUpdate();  

    m_uiNextHeartbeat = m_uiServerTime + m_uiPauseTime + svr_heartbeatInterval;
}
#endif


/*====================
  CHostServer::SendNotification
  ====================*/
void    CHostServer::SendNotification()
{
}


/*====================
  CHostServer::GetServerStatus
  ====================*/
byte    CHostServer::GetServerStatus() const
{
    if (Host.IsSleeping())
        return SERVER_STATUS_SLEEPING;
    else if (m_bGameLoading)
        return SERVER_STATUS_LOADING;
#ifndef K2_CLIENT
    else if ((GetWorld() == NULL || !GetWorld()->IsLoaded()) && GetNumConnectedClients() == 0 && m_vProvisionalClients.empty())
#else
    else if ((GetWorld() == NULL || !GetWorld()->IsLoaded()) && GetNumConnectedClients() == 0)
#endif
        return SERVER_STATUS_IDLE;
    else
        return SERVER_STATUS_ACTIVE;
}


/*====================
  CHostServer::GetOfficial
  ====================*/
byte    CHostServer::GetOfficial() const
{
    if ((m_yHostFlags & SERVER_OFFICIAL) && !(m_yHostFlags & HOST_SERVER_NO_STATS))
        return byte(1); // Official w/ stats
    else if ((m_yHostFlags & SERVER_OFFICIAL) && (m_yHostFlags & HOST_SERVER_NO_STATS))
        return byte(2); // Official w/o stats
    else if (!(m_yHostFlags & SERVER_OFFICIAL))
        return byte(0); // Unofficial
        
    return byte(1);
}


/*====================
  CHostServer::SendGameStatus
  ====================*/
void    CHostServer::SendGameStatus(uint uiServerLoad)
{
    PROFILE("CHostServer::SendGameStatus");

#ifndef K2_CLIENT
    // Inform manager that we've finished initialization
    if (!m_bHasManager)
        return;

    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        CSocket &cSocket(it->second->GetSocket());

        m_uiBytesSent += cSocket.GetBytesSent();
        m_uiPacketsSent += cSocket.GetPacketsSent();
        m_uiBytesDropped += cSocket.GetBytesDropped();
        m_uiPacketsDropped += cSocket.GetPacketsDropped();

        cSocket.ClearProfileStats();
    }

    VoiceServer.GetProfileStats(m_uiBytesSent, m_uiPacketsSent, m_uiBytesDropped, m_uiPacketsDropped, m_uiBytesReceived, m_uiPacketsReceived); 

    m_pktManager.WriteByte(NETCMD_MANAGER_STATUS);
    m_pktManager.WriteByte(GetServerStatus());
    m_pktManager.WriteInt(Host.GetSystemTime());
    m_pktManager.WriteInt(uiServerLoad);
    m_pktManager.WriteByte(GetNumConnectedClients());
    m_pktManager.WriteByte(byte(m_bMatchStarted));
    m_pktManager.WriteInt(m_uiBytesSent);
    m_pktManager.WriteInt(m_uiPacketsSent);
    m_pktManager.WriteInt(m_uiBytesDropped);
    m_pktManager.WriteInt(m_uiPacketsDropped);
    m_pktManager.WriteInt(m_uiBytesReceived);
    m_pktManager.WriteInt(m_uiPacketsReceived);
    m_pktManager.WriteInt(uint(K2System.GetProcessVirtualMemoryUsage()));

    m_GameLib.SetGamePointer();
    m_GameLib.GetGameStatus(m_pktManager);

    m_sockManager.SendPacket(m_pktManager);
    m_pktManager.Clear();

    m_uiBytesSent = 0;
    m_uiPacketsSent = 0;
    m_uiBytesDropped = 0;
    m_uiPacketsDropped = 0;
    m_uiBytesReceived = 0;
    m_uiPacketsReceived = 0;
#endif
}


/*====================
  CHostServer::ResetStateData
  ====================*/
void    CHostServer::ResetStateData()
{
    // Create server reserved state strings
    m_vStateStrings.clear();
    for (ushort unIndex(STATE_STRING_NULL); unIndex < STATE_STRING_NUM_RESERVED; ++unIndex)
        AddStateString();

    // Create server reserved state blocks
    m_vStateBlocks.clear();
    m_vStateBlockModCounts.clear();
    for (ushort unIndex(STATE_BLOCK_NULL); unIndex < STATE_BLOCK_NUM_RESERVED; ++unIndex)
        AddStateBlock(unIndex);
}


/*====================
  CHostServer::SendShutdown

  tell the master server we're shutting down
  ====================*/
void    CHostServer::SendShutdown()
{
    if (!svr_broadcast || m_bUpdating)
        return;

    CHTTPRequest *pShutdowRequest(m_pHTTPManager->SpawnRequest());
    if (pShutdowRequest == NULL)
        return;

    pShutdowRequest->SetTargetURL(m_sMasterServerURL);
    pShutdowRequest->AddVariable(L"f", L"shutdown");
    pShutdowRequest->AddVariable(L"session", m_sSessionCookie);
    pShutdowRequest->SendPostRequest();
    pShutdowRequest->Wait();
    m_pHTTPManager->ReleaseRequest(pShutdowRequest);
}


/*====================
  CHostServer::SendGameData
  ====================*/
void    CHostServer::SendGameData(int iClient, const IBuffer &buffer, bool bReliable)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->GetClientNum() == iClient)
    {
        buffer.Rewind();
        m_pClient->SendGameData(buffer, bReliable);
    }
#else
    ClientMap_it it(m_mapClients.find(iClient));
    if (it == m_mapClients.end())
    {
        //Console.Warn << _T("CHostServer::SendGameData() - Invalid client: ") << iClient << newl;
        return;
    }

    buffer.Rewind();
    it->second->SendGameData(buffer, bReliable);
#endif
}


/*====================
  CHostServer::BroadcastGameData
  ====================*/
void    CHostServer::BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient)
{
#ifdef K2_CLIENT
    if (m_pClient != NULL)
    {
        buffer.Rewind();
        m_pClient->SendGameData(buffer, bReliable);
    }
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->first == iExcludeClient)
            continue;
        if (!it->second->IsConnected())
            continue;

        buffer.Rewind();
        it->second->SendGameData(buffer, bReliable);
    }
#endif
}


#ifndef K2_CLIENT
/*====================
  CHostServer::ListClients
  ====================*/
void    CHostServer::ListClients()
{
    Console
        << _T(" #") << SPACE
        << _T("           Name") << SPACE
        << _T("Connection") << SPACE
        << _T("Account") << SPACE
        << _T("        Address") << SPACE
        << _T("      Status") << SPACE
        << _T("Cookie") << newl;

    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        Console
            << XtoA(it->first, FMT_NONE, 2) << SPACE
            << XtoA(it->second->GetName(), FMT_NONE, 15) << SPACE
            << XtoA(it->second->GetConnectionID(), FMT_NONE, 10) << SPACE
            << XtoA(it->second->GetAccountID(), FMT_NONE, 7) << SPACE
            << XtoA(it->second->GetAddress(), FMT_NONE, 15) << SPACE;
        switch (it->second->GetState())
        {
        case CLIENT_CONNECTION_STATE_DISCONNECTED:  Console << _T("Disconnected") << SPACE; break;
        case CLIENT_CONNECTION_STATE_CONNECTING:    Console << _T("  Connecting") << SPACE; break;
        case CLIENT_CONNECTION_STATE_READY:         Console << _T("       Ready") << SPACE; break;
        case CLIENT_CONNECTION_STATE_STANDBY:       Console << _T("     Standby") << SPACE; break;
        case CLIENT_CONNECTION_STATE_IN_GAME:       Console << _T("     In Game") << SPACE; break;
        }

        Console << it->second->GetCookie() << newl;
    }
}
#endif


#ifndef K2_CLIENT
/*====================
  CHostServer::ListProvisionalClients
  ====================*/
void    CHostServer::ListProvisionalClients()
{
    Console
        << _T(" #") << SPACE
        << _T("           Name") << SPACE
        << _T("Connection") << SPACE
        << _T("Account") << SPACE
        << _T("        Address") << SPACE
        << _T("      Status") << SPACE
        << _T("Cookie") << newl;

    for (vector<CClientConnection*>::iterator it(m_vProvisionalClients.begin()); it != m_vProvisionalClients.end(); ++it)
    {
        Console
            << XtoA(-1, FMT_NONE, 2) << SPACE
            << XtoA((*it)->GetName(), FMT_NONE, 15) << SPACE
            << XtoA((*it)->GetConnectionID(), FMT_NONE, 10) << SPACE
            << XtoA((*it)->GetAccountID(), FMT_NONE, 7) << SPACE
            << XtoA((*it)->GetAddress(), FMT_NONE, 15) << SPACE;
        switch ((*it)->GetState())
        {
        case CLIENT_CONNECTION_STATE_DISCONNECTED:  Console << _T("Disconnected") << SPACE; break;
        case CLIENT_CONNECTION_STATE_CONNECTING:    Console << _T("  Connecting") << SPACE; break;
        case CLIENT_CONNECTION_STATE_READY:         Console << _T("       Ready") << SPACE; break;
        case CLIENT_CONNECTION_STATE_STANDBY:       Console << _T("     Standby") << SPACE; break;
        case CLIENT_CONNECTION_STATE_IN_GAME:       Console << _T("     In Game") << SPACE; break;
        }

        Console << (*it)->GetCookie() << newl;
    }
}
#endif


/*====================
  CHostServer::KickClient
  ====================*/
void    CHostServer::KickClient(int iClientNum, const tstring &sReason)
{
#ifndef K2_CLIENT
    ClientMap_it itClient(m_mapClients.find(iClientNum));
    if (itClient == m_mapClients.end())
        return;

    uint uiAccountID(itClient->second->GetAccountID());
    map<int, uint>::iterator itKickCount(m_mapKickedClients.find(uiAccountID));
    if (itKickCount == m_mapKickedClients.end())
        m_mapKickedClients[uiAccountID] = 1;
    else
        itKickCount->second += 1;

    RemoveClient(iClientNum, sReason);
#endif
}


/*====================
  CHostServer::BanClient
  ====================*/
void    CHostServer::BanClient(int iClientNum, int iTime, const tstring &sReason)
{
    // TODO: Implement proper banning
    RemoveClient(iClientNum, sReason);
}


/*====================
  CHostServer::Init
  ====================*/
bool    CHostServer::Init(bool bPractice, bool bLocal)
{
    PROFILE("CHostServer::Init");

    try
    {
        svr_version = K2_Version(K2System.GetVersionString());

        // Allocate a CWorld
        m_pWorld = K2_NEW(ctx_HostServer,  CWorld)(WORLDHOST_SERVER);
        if (m_pWorld == NULL)
            EX_ERROR(_T("Failed to allocate a CWorld"));
        if (!m_pWorld->IsValid())
            EX_ERROR(_T("Failure constructing CWorld"));

        // Open up a socket to listen for clients
        if (!m_sockGame.Init(K2_SOCKET_GAME, svr_port, false, svr_sendBuffer, svr_recvBuffer))
            EX_ERROR(_TS("Couldn't open port ") + svr_port);

        // Create a chat server connection object
#ifndef K2_CLIENT
        m_pChatConnection = K2_NEW(ctx_HostServer,  CServerChatConnection)(this);
#endif

        // Successful start
        Console.Server << L"Started server " << QuoteStr(m_GameLib.GetName()) << L" version "
            << XtoA(m_GameLib.GetMajorVersion()) << L"." << XtoA(m_GameLib.GetMinorVersion()) << newl;

        m_GameLib.SetGamePointer();
        if (!m_GameLib.Init(this))
            EX_ERROR(L"Failed to initialize server library");

        NetworkResourceManager.Clear();

        ResetStateData();
        ICvar::SetTransmitModified(true);
        ICvar::SetServerInfoModified(true);
        NetworkResourceManager.SetModified(true);
        NetworkResourceManager.SetStringListModified(true);
        UpdateStateStrings();

        m_mapInvitations.clear();

#ifndef K2_CLIENT
        VoiceServer.Init();

        if (bPractice)
        {
            BreakSessionCookie();
            m_sSessionCookie.clear();
        }
        else
        {
            RequestSessionCookie(true);
        }

        svr_managerName = Host.GetRegisterName();
        svr_managerPort = Host.GetRegisterPort();

        // Inform manager that we've finished initialization
        if (!svr_managerName.empty())
        {
            m_bHasManager = true;

            m_sockManager.Init(m_sockGame);
            m_sockManager.SetSendAddr(svr_managerName, svr_managerPort);

            CPacket pkt;
            pkt.WriteByte(NETCMD_MANAGER_INITIALIZED);
            
            m_sockManager.SendPacket(pkt);
        }

        m_uiBytesSent = 0;
        m_uiPacketsSent = 0;
        m_uiBytesDropped = 0;
        m_uiPacketsDropped = 0;
        m_uiBytesReceived = 0;
        m_uiPacketsReceived = 0;

        // Connect to chat server
        m_pChatConnection->Connect(svr_chatAddress, svr_chatPort);
#endif

        m_bLocal = bLocal;
        m_bPractice = bPractice;

        return true;
    }
    catch (CException &ex)
    {
        m_sockGame.Close();
        SAFE_DELETE(m_pWorld);

        m_GameLib.Invalidate();

        ex.Process(_T("CHostServer::Init() - "), NO_THROW);
        return false;
    }
}


/*====================
  CHostServer::StartGame
  ====================*/
bool    CHostServer::StartGame(const tstring &sName, const tstring &sGameSettings)
{
    m_bGameLoading = true;

#ifndef K2_CLIENT
    m_pChatConnection->SendStatusUpdate();
#endif

    tstring sNewGameSettings(sGameSettings);

    tstring sCleanName(sName);
    for (tstring::iterator it(sCleanName.begin()); it != sCleanName.end(); ++it)
    {
        if (*it == _T('%'))
            *it = _T(' ');
    }

    if (m_bPractice)
        sNewGameSettings += _CWS(" solo:true");
    else
        sNewGameSettings += _CWS(" solo:false");

    if (m_bLocal)
        sNewGameSettings += _CWS(" local:true");
    else
        sNewGameSettings += _CWS(" local:false");

    if (IsArrangedMatch())
        sNewGameSettings += _CWS(" arranged:true");
    else
        sNewGameSettings += _CWS(" arranged:false");
        
    if (m_bTournMatch)
        sNewGameSettings += _CWS(" tourn:true");
    else
        sNewGameSettings += _CWS(" tourn:false");

    if (m_bLeagueMatch)
        sNewGameSettings += _CWS(" league:true");
    else
        sNewGameSettings += _CWS(" league:false");
    
    m_GameLib.SetGamePointer();
    if (!m_GameLib.LoadWorld(sCleanName, sNewGameSettings))
    {
        Console << _T("Failed to start game") << newl;
        return false;
    }

    UpdateStateStrings();

    // Drop the clients out of the "ready" state so that they won't receive the world
    // load request until they have accked any state changes
#ifdef K2_CLIENT
    if (m_pClient != NULL)
        m_pClient->ResynchStateData();
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        it->second->ResynchStateData();
#endif

    m_uiNextHeartbeat = 0;
    m_bInitializeMatchHeartbeat = true;
    m_bMatchStarted = false;
    m_bGameLoading = false;

    if (svr_slave != -1)
        K2System.SetTitle(XtoA(svr_slave) + _T(" - ") + sCleanName);
    else
        K2System.SetTitle(sCleanName);

#ifndef K2_CLIENT
    m_pChatConnection->SendStatusUpdate();
#endif
    return true;
}


/*====================
  CHostServer::StartReplay
  ====================*/
bool    CHostServer::StartReplay(const tstring &sFilename)
{
    try
    {
        Console << _T("Starting replay ") << QuoteStr(sFilename) << newl;

        m_GameLib.SetGamePointer();
        if (!m_GameLib.StartReplay(sFilename))
            EX_ERROR(_T("Game failed to start replay"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostServer::StartReplay() - "), NO_THROW);
        return false;
    }
}


/*====================
  CHostServer::StopReplay
  ====================*/
void    CHostServer::StopReplay()
{
    Console << _T("Stopping replay...") << newl;
    m_GameLib.StopReplay();
}


/*====================
  CHostServer::SendConnectReminders
  ====================*/
void    CHostServer::SendConnectReminders()
{
#ifndef K2_CLIENT
    if (Host.GetTime() < m_pChatConnection->GetNextReminderTime())
        return;

    m_pChatConnection->UpdateReminderTime();

    for (Roster_it itClient(m_mapRoster.begin()), itEnd(m_mapRoster.end()); itClient != itEnd; ++itClient)
    {
        int iClientNumber(GetClientNumber(itClient->first));
        if (iClientNumber != -1)
            continue;

        if (itClient->second.uiReminders < svr_maxReminders)
        {
            //m_pChatConnection->ReplacePlayer(itClient->first);
            //continue;

            //m_pChatConnection->SendConnectionReminder(itClient->first);
            ++itClient->second.uiReminders;
        }
    }
#endif
}


/*====================
  CHostServer::SubstituteRoster
  ====================*/
void    CHostServer::SubstituteRoster(uint uiOldAccountID, uint uiNewAccountID)
{
    Roster_it itPlayer(m_mapRoster.find(uiOldAccountID));
    if (itPlayer == m_mapRoster.end())
        return;

    SRosterEntty entry(itPlayer->second.uiTeam, itPlayer->second.uiSlot);
    m_mapRoster.erase(itPlayer);
    m_mapRoster[uiNewAccountID] = entry;
}


/*====================
  CHostServer::GenerateInvitation
  ====================*/
void    CHostServer::GenerateInvitation(int iAccountID)
{
    tstring sInviteCode;
    for (int i(0); i < 12; ++i)
        sInviteCode += TCHAR(M_Randnum(32, 127));
    m_mapInvitations[iAccountID] = sInviteCode;
}


/*====================
  CHostServer::GenerateInvitation
  ====================*/
void    CHostServer::GenerateInvitation(const tstring &sUser)
{
    tstring sInviteCode;
    for (int i(0); i < 12; ++i)
        sInviteCode += TCHAR(M_Randnum(32, 127));
    m_mapInvitations2[ChatManager.RemoveClanTag(LowerString(sUser))] = sInviteCode;
}


/*====================
  CHostServer::ValidateInvitation
  ====================*/
bool    CHostServer::ValidateInvitation(int iAccountID)
{
    return m_mapInvitations.find(iAccountID) != m_mapInvitations.end();
}


/*====================
  CHostServer::ValidateInvitation
  ====================*/
bool    CHostServer::ValidateInvitation(const tstring &sUser)
{
    return m_mapInvitations2.find(ChatManager.RemoveClanTag(LowerString(sUser))) != m_mapInvitations2.end();
}


/*====================
  CHostServer::Frame
  ====================*/
void    CHostServer::Frame(uint uiHostFrameLength, bool bClientReady)
{
    PROFILE("CHostServer::Frame");

    try
    {
        uint uiFrameStartTime(K2System.Microseconds());

        // Decrement connection request counts
        if (m_uiLastConnectRequestPeriod == INVALID_TIME)
            m_uiLastConnectRequestPeriod = Host.GetTime();

        if (Host.GetTime() - m_uiLastConnectRequestPeriod >= svr_connectReqPeriod)
        {
            m_uiLastConnectRequestPeriod = Host.GetTime();

            for (map<wstring, uint>::iterator it(m_mapConnectionRequests.begin()), itEnd(m_mapConnectionRequests.end()); it != itEnd; )
            {
                --it->second;
                if (it->second == 0)
                    STL_ERASE(m_mapConnectionRequests, it);
                else
                    ++it;               
            }
        }

        m_uiFrameLength = SecToMs(1.0f / svr_gameFPS);
        uint uiDroppedFrames(0);

#ifndef K2_CLIENT
        if (svr_broadcast && m_sSessionCookie.empty() && !GetPractice())
            RequestSessionCookie(true);
#endif

        // Check for updates... Servers check every 2 minutes
        if (K2System.IsDedicatedServer() && !HasManager() &&
            (m_uiLastUpdateCheck == INVALID_TIME || K2System.Milliseconds() >= m_uiLastUpdateCheck + 120000))
        {
            SilentUpdate();
            m_uiLastUpdateCheck = K2System.Milliseconds();
        }

        if (svr_name.IsModified())
        {
            if (!m_pWorld->IsLoaded())
            {
                if (svr_slave != -1)
                    K2System.SetTitle(XtoA(svr_slave) + _T(" - ") + svr_name);
                else
                    K2System.SetTitle(svr_name);
            }

            svr_name.SetModified(false);
#ifndef K2_CLIENT
            m_pChatConnection->SendStatusUpdate();
#endif
        }

        if (svr_location.IsModified())
        {
            svr_location.SetModified(false);
#ifndef K2_CLIENT
            m_pChatConnection->SendStatusUpdate();
#endif
        }

        // Issue a console warning if the frame time is too long
        if (uiHostFrameLength > m_uiFrameLength && m_uiFrameCount > 0 && !(m_bHasManager && (m_pWorld == NULL || !m_pWorld->IsLoaded())))
        {
            if (K2System.IsDedicatedServer() || svr_hitchWarning)
            {
                if (svr_showLongServerFrames)
                {
                    Console.Warn << _T("Long server frame (") << uiHostFrameLength << _T(" msec)") << newl;
                    ++m_uiLongServerFrameCount;

                    m_GameLib.LongServerFrame(uiHostFrameLength);
                }
            }

            if (m_bHasManager)
            {
                m_pktManager.WriteByte(NETCMD_MANAGER_LONG_FRAME);
                m_pktManager.WriteInt(uiHostFrameLength);
            }
        }

        Host.ResetLocalClientTimeouts();

#ifndef K2_CLIENT
        m_pChatConnection->Frame();
#endif
        // Update voice server
        VoiceServer.Frame();

        // Process incoming data
        ReadPackets();

#ifndef K2_CLIENT
        // Update pseudo-clients
        for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        {
            if (!it->second->HasFlags(CLIENT_CONNECTION_PSEUDO))
                continue;

            CClientSnapshot cSnapshot;
            cSnapshot.Update(0, 0, m_uiFrameCount - 1);

            CBufferDynamic cBuffer;
            cSnapshot.GetUpdate(cBuffer);

            CPacket cPkt;
            cPkt << cBuffer;

            it->second->ReadClientSnapshot(cPkt);
        }
#endif

#ifndef K2_CLIENT
        for (vector<CClientConnection *>::iterator it(m_vProvisionalClients.begin()); it != m_vProvisionalClients.end(); )
        {
            CClientConnection *pClient(*it);
            pClient->ConverseWithMasterServer();
            int iClientNumber(pClient->GetClientNum());
            if (iClientNumber != -1)
            {
                m_mapClients[iClientNumber] = pClient;
                it = m_vProvisionalClients.erase(it);
            }
            else if (pClient->GetConnectionState() == CLIENT_CONNECTION_STATE_DISCONNECTED)
            {
                K2_DELETE(*it);
                it = m_vProvisionalClients.erase(it);
            }
            else
                ++it;
        }
#endif

        // Try to send fragmented updates, even if it's not time for a new server frame
#ifdef K2_CLIENT
        if (m_pClient != NULL)
        {
            m_pClient->WriteSnapshotFragments();
            m_pClient->SendPackets(svr_gameFPS);
            if (!m_pClient->IsConnected())
                RemoveClient(m_pClient->GetClientNum());
        }
#else

        for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); )
        {
            it->second->ConverseWithMasterServer();
            it->second->WriteSnapshotFragments();
            it->second->SendPackets(svr_gameFPS);

            if (!it->second->IsConnected())
                it = RemoveClient(it);
            else
                ++it;
        }
#endif

        m_uiServerLoad += K2System.Microseconds() - uiFrameStartTime;

        // Advance server time  
        m_uiFrameAccumulator += uiHostFrameLength;  

        if (m_uiFrameAccumulator > m_uiFrameLength * (svr_maxFramesPerHostFrame + 1))
        {
            uiDroppedFrames = (m_uiFrameAccumulator - (m_uiFrameLength * svr_maxFramesPerHostFrame)) / m_uiFrameLength;
            
            if (K2System.IsDedicatedServer())
            {
                if (svr_showLongServerFrames)
                        Console.Warn << _T("Server skipped ") << uiDroppedFrames << _T(" frames") << newl;
            }
            m_uiFrameAccumulator -= uiDroppedFrames * m_uiFrameLength;
            m_uiDroppedFrames = uiDroppedFrames;
        }

        // Don't let the server run faster than svr_gameFPS
        while (m_uiFrameAccumulator >= m_uiFrameLength)
        {
            GAME_PROFILE_EX(_T("Server Game Frame"), PROFILE_GAME_SERVER_FRAME);

            uint uiGameFrameStartTime(K2System.Microseconds());

            if (m_bPaused)
            {
                m_uiPauseTime += m_uiFrameLength;

                _svr_paused = m_uiPauseTime;
            }
            else
            {
                m_uiServerTime += m_uiFrameLength;

                _svr_ms = m_uiServerTime;
            }
            
            _svr_snapshotPoolAllocated = CSnapshot::GetSnapshotPool()->GetNumAllocated();
            _svr_entitySnapshotPoolAllocated = CEntitySnapshot::GetEntitySnapshotPool()->GetNumAllocated();

#ifndef K2_CLIENT
            SendHeartbeat();
#endif
            
            // Run the game code
            if (bClientReady && !m_bUpdating)
            {
                Console.SetDefaultStream(Console.ServerGame);
                m_GameLib.SetGamePointer();
                m_GameLib.Frame();
                Console.SetDefaultStream(Console.Server);

                // Save a snapshot of this frame
                if (m_vSnapshots.size() != svr_snapshotCacheSize)
                    m_vSnapshots.resize(svr_snapshotCacheSize);

                uint uiOldSnapshotBufferPos(m_uiSnapshotBufferPos);

                ++m_uiSnapshotBufferPos;

                if (m_uiSnapshotBufferPos >= m_vSnapshots.size())
                    m_uiSnapshotBufferPos = 0;

                if (m_vSnapshots[m_uiSnapshotBufferPos] != INVALID_POOL_HANDLE)
                    CSnapshot::DeleteByHandle(m_vSnapshots[m_uiSnapshotBufferPos]);
                
                m_vSnapshots[m_uiSnapshotBufferPos] = CSnapshot::Allocate(CSnapshot(m_uiFrameCount, m_uiServerTime));
                m_GameLib.SetGamePointer();
                m_GameLib.GetSnapshot(*CSnapshot::GetByHandle(m_vSnapshots[m_uiSnapshotBufferPos]));

                if (uiOldSnapshotBufferPos != uint(-1) && m_vSnapshots[uiOldSnapshotBufferPos] != INVALID_POOL_HANDLE)
                    CSnapshot::GetByHandle(m_vSnapshots[m_uiSnapshotBufferPos])->CalcSequence(*CSnapshot::GetByHandle(m_vSnapshots[uiOldSnapshotBufferPos]));
                else
                    CSnapshot::GetByHandle(m_vSnapshots[m_uiSnapshotBufferPos])->CalcSequence(CSnapshot());

                m_GameLib.EndFrame(m_vSnapshots[m_uiSnapshotBufferPos]);
            }

            // TODO: Only send the lastest snapshot from a multi-game frame host frame
            // This issue with this is game events in the first frame will get dropped
            // The snapshots need to be merged instead
            
            // Update STATE_STRING_SERVER_INFO and STATE_STRING_CVAR_SETTINGS state strings if necessary
            UpdateStateStrings();

            // Send updates to the clients
            UpdateClients();

            // Tidy up
            uiDroppedFrames = 0;
            ++m_uiFrameCount;
            m_uiFrameAccumulator -= m_uiFrameLength;

            if (svr_addServerMS)
                K2System.Sleep(svr_addServerMS);

            m_uiServerLoad += K2System.Microseconds() - uiGameFrameStartTime;
            
            // Store the last server load value for sending status updates to the Chat Server
            m_uiLastServerLoad = m_uiServerLoad;

            SendGameStatus(m_uiServerLoad);         

            m_uiServerLoad = 0;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CHostServer::Frame() - "));
    }
}


/*====================
  CHostServer::GetNumActiveClients
  ====================*/
int     CHostServer::GetNumActiveClients()
{
#ifdef K2_CLIENT
    if (m_pClient != NULL && m_pClient->GetConnectionState() == CLIENT_CONNECTION_STATE_IN_GAME && m_pClient->GetAccountID() != -1)
        return 1;
    else
        return 0;
#else
    int iNumClients(0);
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
    {
        if (it->second->GetConnectionState() != CLIENT_CONNECTION_STATE_IN_GAME)
            continue;
        if (it->second->GetAccountID() == -1)
            continue;

        iNumClients++;
    }

    return iNumClients;
#endif
}


/*====================
  CHostServer::UpdateAvailable
  ====================*/
void    CHostServer::UpdateAvailable(const tstring &sVersion)
{
    if (m_bUpdateAvailable || m_bUpdating || m_bUpdateComplete)
        return;

    m_bUpdateAvailable = true;

    if (GetNumConnectedClients() == 0)
        PerformUpdate();
}


/*====================
  CHostServer::StartGame
  ====================*/
void    CHostServer::StartGame(uint uiMatchID)
{
#ifndef K2_CLIENT
    Console << _T("StartGame - MatchID [") << uiMatchID << _T("]") <<  newl;

    if (m_bHasManager)
    {
        CPacket pkt;
        pkt.WriteByte(NETCMD_MANAGER_MATCH_START);
        pkt.WriteInt(uiMatchID);
        pkt.WriteString(m_pWorld != NULL && m_pWorld->IsLoaded() ? m_pWorld->GetName() : TSNULL);
        
        m_GameLib.GetServerInfo(pkt);

        m_sockManager.SendPacket(pkt);
    }

    m_uiNextHeartbeat = 0;

    m_bPaused = false;
#endif
}


/*====================
  CHostServer::StartMatch
  ====================*/
void    CHostServer::StartMatch()
{
    Console << _T("StartMatch") << newl;

    m_bMatchStarted = true;
    m_uiNextHeartbeat = 0;

    m_bPaused = false;

#ifndef K2_CLIENT
    if (IsArrangedMatch())
        m_pChatConnection->SendMatchStarted();
#endif
}


/*====================
  CHostServer::RestartMatch
  ====================*/
void    CHostServer::RestartMatch()
{
    Console << _T("RestartMatch") << newl;

    m_bMatchStarted = false;
    m_uiNextHeartbeat = 0;

    m_bPaused = false;
}


/*====================
  CHostServer::EndGame
  ====================*/
void    CHostServer::EndGame(const tstring &sReplayHost, const tstring &sReplayDir, const tstring &sReplayFilename, const tstring &sLogFilename, bool bFailed, const tstring &sReason, bool bAborted, EMatchAbortedReason eAbortedReason)
{
    if (!sReason.empty())
        Console << _T("EndGame - ") << sReason << newl;
    else
        Console << _T("EndGame") << newl;

    m_pWorld->Free();

    for (vector<PoolHandle>::iterator it(m_vSnapshots.begin()); it != m_vSnapshots.end(); ++it)
        SAFE_DELETE_SNAPSHOT(*it);

    m_uiSnapshotBufferPos = 0;

    m_mapClientNumbers.clear();

#ifdef K2_CLIENT
    if (m_pClient != NULL)
    {
        m_pClient->Disconnect(sReason);
        SAFE_DELETE(m_pClient);
    }
#else
    for (ClientMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); )
    {
        itClient->second->Disconnect(sReason);
        SAFE_DELETE(itClient->second);
        STL_ERASE(m_mapClients, itClient);
    }
#endif

    NetworkResourceManager.ClearStrings();
    NetworkResourceManager.Clear();

    ResetStateData();
    ICvar::SetTransmitModified(true);
    ICvar::SetServerInfoModified(true);
    NetworkResourceManager.SetModified(true);
    NetworkResourceManager.SetStringListModified(true);
    UpdateStateStrings();

    m_uiServerTime = 0;
    m_uiPauseTime = 0;

#ifndef K2_CLIENT
    m_uiNextHeartbeat = 0;

    if (m_bHasManager)
    {
        CPacket pkt;
        pkt.WriteByte(NETCMD_MANAGER_MATCH_END);
        pkt.WriteString(sReplayHost);
        pkt.WriteString(sReplayDir);
        pkt.WriteString(sReplayFilename);
        pkt.WriteString(sLogFilename);

        m_sockManager.SendPacket(pkt);
    }

    // reset some of the match variables here so players don't get errors when trying to choose 
    // servers to create a game the next time this server is used
    SetNoLeaver(false);
    m_unMinPSR = 0;
    m_unMaxPSR = 0;

    // Request a new session cookie
    RequestSessionCookie(true);

    m_mapInvitations.clear();
    m_mapInvitations2.clear();
    m_mapKickedClients.clear();
#endif

    if (svr_slave != -1)
        K2System.SetTitle(XtoA(svr_slave) + _T(" - ") + svr_name);
    else
        K2System.SetTitle(svr_name);

    m_bMatchStarted = false;
    m_bGameLoading = false;

    m_bPaused = false;

    GetStateString(STATE_STRING_RESOURCES).Validate(); // DEBUG

#ifndef K2_CLIENT
    m_pChatConnection->SendAbandonMatch(bFailed);
    m_pChatConnection->SendStatusUpdate();
#endif

#ifndef K2_CLIENT
    if (bAborted && IsArrangedMatch())
        m_pChatConnection->SendMatchAborted(eAbortedReason);
#endif

    SetMatchupID(INVALID_INDEX);
}


/*====================
  CHostServer::UpdateComplete
  ====================*/
void    CHostServer::UpdateComplete()
{
#ifndef K2_CLIENT
    if (m_bUpdateAvailable && m_bUpdating)
        m_bUpdateComplete = true;

    K2System.RestartOnExit(true);
    K2System.Exit(0);
#endif
}


/*====================
  CHostServer::PerformUpdate
  ====================*/
void    CHostServer::PerformUpdate()
{
#ifndef K2_CLIENT
    if (m_bUpdating || !m_bUpdateAvailable)
        return;

    uiset setProcesses;
    uint uiLocalProcess;
    bool bLowestPID(true);

    SendShutdown();

    m_bUpdating = true;

    m_GameLib.SetGamePointer();
    m_GameLib.UnloadWorld();
    if (m_pWorld != NULL)
        m_pWorld->Free();

    // NOTE: GetRunningProcesses() gets all running PIDs
    // for k2 instances, NOT ALL processes.
    setProcesses = K2System.GetRunningProcesses();
    uiLocalProcess = K2System.GetProcessID();

    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); )
        it = RemoveClient(it, _T("Server is downloading version " + m_sUpdateVersion));

    // Check if we're the lowest PID of all instances
    for (uiset::iterator it(setProcesses.begin()); it != setProcesses.end(); ++it)
    {
        if (*it < uiLocalProcess)
        {
            bLowestPID = false;
            break;
        }
    }

    if ((bLowestPID || K2System.IsServerManager()) && !FileManager.Exists(_T(":/Update/verify")))
    {
        m_bUpdating = true;
        K2Updater.CheckForUpdates(false);
    }
    else
    {
        while (!bLowestPID)
        {
            K2System.Sleep(5000);
            bLowestPID = true;
            setProcesses = K2System.GetRunningProcesses();

            // Check if we're the lowest PID of all instances
            for (uiset::iterator it(setProcesses.begin()); it != setProcesses.end(); ++it)
            {
                if (*it < uiLocalProcess)
                {
                    bLowestPID = false;
                    break;
                }
            }
        }

        K2System.RestartOnExit(true);
        K2System.Exit(0);
    }
#endif
}


/*====================
  CHostServer::SilentUpdate
  ====================*/
void    CHostServer::SilentUpdate()
{
#ifndef K2_CLIENT
    K2Updater.SilentUpdate();
#endif
}


/*====================
  CHostServer::SetStateString
  ====================*/
void    CHostServer::SetStateString(ushort unID, const string &sStr)
{
    if (unID >= m_vStateStrings.size())
        return;

    CStateString &ssLast(GetStateString(unID));
    CStateString ssNew;
    ssNew.Set(sStr);

    CStateString ssDiff(ssNew);
    ssLast.GetDifference(ssDiff);

    m_GameLib.SetGamePointer();
    m_GameLib.StateStringChanged(unID, ssNew);
#ifdef K2_CLIENT
    if (m_pClient != NULL)
        m_pClient->AddStringToUpdateQueue(unID, ssDiff);
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        it->second->AddStringToUpdateQueue(unID, ssDiff);
#endif

    ssLast = ssNew;
}


/*====================
  CHostServer::SetStateBlock
  ====================*/
void    CHostServer::SetStateBlock(ushort unID, const IBuffer &buffer)
{
    if (unID >= m_vStateBlocks.size())
        return;

    CStateBlock &block(GetStateBlock(unID));
    block.Set(buffer);

    m_GameLib.SetGamePointer();
    m_GameLib.StateBlockChanged(unID, buffer);
#ifdef K2_CLIENT
    if (m_pClient != NULL)
        m_pClient->AddBlockToUpdateQueue(unID, block);
#else
    for (ClientMap_it it(m_mapClients.begin()); it != m_mapClients.end(); ++it)
        it->second->AddBlockToUpdateQueue(unID, block);
#endif
}


/*====================
  CHostServer::GetMaxPlayers
  ====================*/
uint    CHostServer::GetMaxPlayers()
{
    if (svr_maxClients == -1)
        return m_GameLib.GetMaxClients();

    return MIN(svr_maxClients.GetUnsignedInteger(), m_GameLib.GetMaxClients());
}


/*====================
  CHostServer::AddPseudoClient
  ====================*/
void    CHostServer::AddPseudoClient()
{
#ifndef K2_CLIENT
    // Create a new client connection
    CClientConnection *pNewClient(K2_NEW(ctx_HostServer,  CClientConnection)(this, m_pHTTPManager, TSNULL, 0, m_sockGame));
    if (pNewClient == NULL)
        EX_ERROR(_T("Failed to allocate a new CClientConnection"));

    m_mapClients[pNewClient->GetClientNum()] = pNewClient;
#endif
}


/*====================
  CHostServer::InviteUser
  ====================*/
void    CHostServer::InviteUser(const tstring &sName)
{
    GenerateInvitation(sName);
}


/*====================
  CHostServer::GetClientKickCount
  ====================*/
uint    CHostServer::GetClientKickCount(int iAccountID)
{
    map<int, uint>::iterator itClient(m_mapKickedClients.find(iAccountID));
    if (itClient == m_mapKickedClients.end())
        return 0;

    return itClient->second;
}


/*====================
  CHostServer::IsValidTier
  ====================*/
bool    CHostServer::IsValidTier(int iLevel) const
{
    return true;

    if (m_iTier < 0 || m_iTier >= int(m_vTiers.size()))
        return true;

    if (iLevel < m_vTiers[m_iTier].first ||
        iLevel > m_vTiers[m_iTier].second)
        return false;

    return true;
}


/*====================
  CHostServer::GetLeaverThreshold
  ====================*/
float   CHostServer::GetLeaverThreshold(int iNumGames)
{
    if (iNumGames == 0)
        return 1.0f;
    if (iNumGames >= 100)
        return 0.05f;

    float fStartGame(0.0f);
    float fStartLeaves(3.0f);

    float fEndGame(80.0f);
    float fEndLeaves(5.0f);

    float fLerp(CLAMP(ILERP<float>(iNumGames, fStartGame, fEndGame), 0.0f, 1.0f));

    return CLAMP(LERP(fLerp, fStartLeaves, fEndLeaves) / iNumGames, 0.0f, 1.0f);
}


/*====================
  CHostServer::IsLeaver
  ====================*/
bool    CHostServer::IsLeaver(float fLeavePercentage, int iNumGames) const
{
    return fLeavePercentage > GetLeaverThreshold(iNumGames) + 0.001f;
}


/*====================
  CHostServer::IsValidPSR
  ====================*/
bool    CHostServer::IsValidPSR(const int iRank) const
{
    if (GetMinPSR())
    {   
        const ushort unMinTolerance(GetMinPSR() / 1.02f);
        
        if (iRank < unMinTolerance)
            return false;
    }

    if (GetMaxPSR())
    {
        const ushort unMaxTolerance(GetMaxPSR() * 1.02f);

        if (iRank > unMaxTolerance)
            return false;
    }
    
    return true;
}


/*====================
  CHostServer::KillServer
  ====================*/
void    CHostServer::KillServer()
{
    m_sSessionCookie.clear();

#ifdef K2_CLIENT
    m_pClient->Disconnect(_T("disconnect_console"));
    SAFE_DELETE(m_pClient);
#else
    for (ClientMap_it itClient(m_mapClients.begin()); itClient != m_mapClients.end(); )
    {
        itClient->second->Disconnect(_T("disconnect_console"));
        SAFE_DELETE(itClient->second);
        STL_ERASE(m_mapClients, itClient);
    }
#endif

    m_GameLib.Reset();
    if (GetWorld() != NULL)
        GetWorld()->Free();
}


/*--------------------
  SetServerSendBuffer
  --------------------*/
CMD(SetServerSendBuffer)
{
    if (vArgList.size() < 1)
        return false;

    if (Host.GetServer())
        Host.GetServer()->SetSendBuffer(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  SetServerRecvBuffer
  --------------------*/
CMD(SetServerRecvBuffer)
{
    if (vArgList.size() < 1)
        return false;

    if (Host.GetServer())
        Host.GetServer()->SetRecvBuffer(AtoI(vArgList[0]));

    return true;
}


#ifndef K2_CLIENT
/*--------------------
  ListClients
  --------------------*/
CMD(ListClients)
{
    if (Host.GetServer() == NULL)
        return false;

    Host.GetServer()->ListClients();
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  ListProvisionalClients
  --------------------*/
CMD(ListProvisionalClients)
{
    if (Host.GetServer() == NULL)
        return false;

    Host.GetServer()->ListProvisionalClients();
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  CreateGame
  --------------------*/
CMD(CreateGame)
{
    if (Host.GetServer() == NULL)
    {
        Console.Err << _T("No active server") << newl;
        return false;
    }

    if (vArgList.empty())
        return false;

    Host.GetServer()->StartGame(vArgList[0], ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  Kick
  --------------------*/
CMD(Kick)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;

    if (vArgList.empty())
    {
        Console << _T("Must specify client to be kicked") << newl;
        return false;
    }

    int iClientNum(pServer->GetClientNumber(vArgList[0]));
    if (iClientNum == -1)
        iClientNum = AtoI(vArgList[0]);
    if (iClientNum == -1)
    {
        Console << _T("Invalid client: ") << vArgList[0] << newl;
        return false;
    }

    pServer->KickClient(iClientNum, (vArgList.size() > 1) ? vArgList[1] : TSNULL);
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  Ban
  --------------------*/
CMD(Ban)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;

    if (vArgList.empty())
    {
        Console << _T("Must specify client to be kicked") << newl;
        return false;
    }

    int iClientNum(pServer->GetClientNumber(vArgList[0]));
    if (iClientNum == -1)
        iClientNum = AtoI(vArgList[0]);
    if (iClientNum == -1)
    {
        Console << _T("Invalid client: ") << vArgList[0] << newl;
        return false;
    }

    pServer->BanClient(iClientNum, -1, (vArgList.size() > 1) ? vArgList[1] : TSNULL);
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  AddPseudoClient
  --------------------*/
CMD(AddPseudoClient)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;

    pServer->AddPseudoClient();
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  PrintServerSessionCookie
  --------------------*/
CMD(PrintServerSessionCookie)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;

    Console << pServer->GetSessionCookie() << newl;
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  ResetServerSessionCookie
  --------------------*/
CMD(ResetServerSessionCookie)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;
    
    pServer->RequestSessionCookie(true);
    return true;
}
#endif


#ifndef K2_CLIENT
/*--------------------
  BreakServerSessionCookie
  --------------------*/
CMD(BreakServerSessionCookie)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;
    
    pServer->BreakSessionCookie();
    return true;
}
#endif


/*--------------------
  ServerUnpause
  --------------------*/
CMD(ServerUnpause)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;
    
    pServer->SetPaused(false);
    return true;
}


/*--------------------
  ServerPause
  --------------------*/
CMD(ServerPause)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;
    
    pServer->SetPaused(true);
    return true;
}


/*--------------------
  KillServer
  --------------------*/
CMD(KillServer)
{
    CHostServer *pServer(Host.GetServer());
    if (pServer == NULL)
        return false;
    
    pServer->KillServer();
    return true;
}
