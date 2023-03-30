// (C)2010 S2 Games
// c_serverchatconnection.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "chatserver_protocol.h"
#include "c_serverchatconnection.h"
#include "c_socket.h"
#include "c_hostserver.h"
#include "c_servermanager.h"
#include "c_world.h"

#ifndef K2_CLIENT
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTF(svr_chatConnectTimeout,      10000,  CVAR_SAVECONFIG);
CVAR_UINTF(svr_chatReconnectDelay,      5000,   CVAR_SAVECONFIG);
CVAR_UINTF(svr_chatConnectedTimeout,    30000,  CVAR_SAVECONFIG);
CVAR_UINTF(svr_clientReminderInterval,  5000,   CVAR_SAVECONFIG);

EXTERN_CVAR_BOOL(svr_broadcast);
//=============================================================================

/*====================
  CServerChatConnection::~CServerChatConnection
  ====================*/
CServerChatConnection::~CServerChatConnection()
{
    Disconnect();
}


/*====================
  CServerChatConnection::CServerChatConnection
  ====================*/
CServerChatConnection::CServerChatConnection(CHostServer *pHostServer) :
m_pHostServer(pHostServer),
m_pSocket(NULL),
m_unPort(0),
m_eState(STATE_IDLE),
m_uiReconnectTime(INVALID_TIME),
m_uiTimeout(INVALID_TIME),
m_uiLastReceiveTime(INVALID_TIME),
m_bSentPing(false),
m_uiNextConnectReminder(INVALID_TIME)
{
}


/*====================
  CServerChatConnection::Connect
  ====================*/
void    CServerChatConnection::Connect(const wstring &sAddress, ushort unPort)
{
    if (!svr_broadcast)
        return;

    Console.Server << L"Attempting to connect to chat server: " << sAddress << L":" << unPort << newl;

    Disconnect();
    m_pSocket = K2_NEW(ctx_Net,  CSocket)(L"ServerChat");
    if (!m_pSocket->Init(K2_SOCKET_TCP2))
        Disconnect();

    m_uiReconnectTime = INVALID_TIME;

    m_sAddress = sAddress;
    m_unPort = unPort;

    if (!m_pSocket->SetSendAddr(m_sAddress, m_unPort))
    {
        Reconnect(svr_chatReconnectDelay, _T("Connect: Couldn't set SendAddr"));
        return;
    }

    m_eState = STATE_CONNECTING;
    m_uiTimeout = K2System.Milliseconds() + svr_chatConnectTimeout;
}


/*====================
  CServerChatConnection::Reconnect
  ====================*/
void    CServerChatConnection::Reconnect(uint uiTimeout, const tstring &sReason)
{
    Disconnect(sReason);

    Console.Server << L"Will attempt to reconnect to chat server in " << uiTimeout << L"ms" << newl;

    m_uiReconnectTime = K2System.Milliseconds() + uiTimeout;
}


/*====================
  CServerChatConnection::Disconnect
  ====================*/
void    CServerChatConnection::Disconnect(const tstring &sReason)
{
    if (m_pSocket == NULL)
        return;

    Console.Server << L"Connection to chat server terminated. " << ParenStr(sReason) << newl;

    m_pktSend.Clear();
    m_pktSend << NET_CHAT_GS_DISCONNECT;
    m_pSocket->SendPacket(m_pktSend);

    m_uiReconnectTime = INVALID_TIME;
    m_eState = STATE_DISCONNECTED;
    K2_DELETE(m_pSocket);
    m_pSocket = NULL;
}


/*====================
  CServerChatConnection::Handshake
  ====================*/
void    CServerChatConnection::Handshake()
{
    if (m_pSocket == NULL)
        return;

    Console.Server << L"Sending handshake to chat server." << newl;

    m_pktSend.Clear();
    m_pktSend << NET_CHAT_GS_CONNECT << m_pHostServer->GetServerID() << m_pHostServer->GetSessionCookie() << CHAT_PROTOCOL_VERSION;
    m_pSocket->SendPacket(m_pktSend);

    m_eState = STATE_AUTHENTICATING;
    m_uiLastReceiveTime = K2System.Milliseconds();
    m_uiTimeout = INVALID_TIME;
}


/*====================
  CServerChatConnection::SendStatusUpdate
  ====================*/
void    CServerChatConnection::SendStatusUpdate()
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    Console.Server << L"Sending status update." << newl;
    m_pktSend.Clear();
    m_pktSend
        << NET_CHAT_GS_STATUS
        << m_pHostServer->GetServerID()
        << m_pHostServer->GetAddress()
        << m_pHostServer->GetPort()
        << m_pHostServer->GetLocation()
        << m_pHostServer->GetName()
        << m_pHostServer->GetServerStatus();
        
    if (m_pHostServer->IsArrangedMatch())
        m_pktSend << byte(1);
    else if (m_pHostServer->IsTournMatch())
        m_pktSend << byte(2);
    else if (m_pHostServer->IsLeagueMatch())
        m_pktSend << byte(3);
    else
        m_pktSend << byte(0);
            
    if (m_pHostServer->GetServerStatus() == SERVER_STATUS_ACTIVE)
    {                       
        m_pktSend   
            << m_pHostServer->GetOfficial()                                                 // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
            << byte(m_pHostServer->GetNoLeaver())                                           // No Leavers (1), Leavers (0)
            << byte(m_pHostServer->GetServerAccess())                                       // Private (1), Not Private (0)
            << m_pHostServer->GetWorld()->GetName()                                         // Map Name             
            << byte(m_pHostServer->GetTier())                                               // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)          
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetGameName")                // Game Name
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetGameModeName")            // Game Mode Name               
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetTeamSize"))             // Team Size
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetAllHeroes"))            // All Heroes (1), Not All Heroes (0)
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetCasualMode"))           // Casual Mode (1), Not Casual Mode (0)
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetForceRandom"))          // Force Random (1), Not Force Random (0)                           
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetAutoBalanced"))         // Auto Balanced (1), Non Auto Balanced (0)
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetAdvancedOptions"))      // Advanced Options (1), No Advanced Options (0)
            << m_pHostServer->GetMinPSR()                                                   // Min PSR
            << m_pHostServer->GetMaxPSR()                                                   // Max PSR
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetDevHeroes"))            // Dev Heroes (1), Non Dev Heroes (0)
            << byte(m_pHostServer->GetGameLib().GetGameInfoInt(L"GetHardcore"))             // Hardcore (1), Non Hardcore (0)
            << m_pHostServer->GetGameLib().GetGameInfoInt(L"GetCurrentGameTime")            // Current Game Time
            << m_pHostServer->GetGameLib().GetGameInfoInt(L"GetCurrentGamePhase")           // Current Game Phase
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetTeamInfo1")               // Get Legion Team Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetTeamInfo2")               // Get Hellbourne Team Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo0")             // Get Player 0 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo1")             // Get Player 1 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo2")             // Get Player 2 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo3")             // Get Player 3 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo4")             // Get Player 4 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo5")             // Get Player 5 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo6")             // Get Player 6 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo7")             // Get Player 7 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo8")             // Get Player 8 Info
            << m_pHostServer->GetGameLib().GetGameInfoString(L"GetPlayerInfo9");            // Get Player 9 Info
    }
    else
    {
        static const byte yDefault(0);
        static const ushort unDefault(0);
        static const uint uiDefault(0);
        static const ULONGLONG ulDefault(0);
        
        m_pktSend       
            << yDefault                                                     // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
            << yDefault                                                     // No Leavers (1), Leavers (0)
            << yDefault                                                     // Private (1), Not Private (0)
            << L""                                                          // Map Name             
            << yDefault                                                     // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
            << L""                                                          // Game Name
            << L""                                                          // Game Mode Name               
            << yDefault                                                     // Team Size
            << yDefault                                                     // All Heroes (1), Not All Heroes (0)
            << yDefault                                                     // Casual Mode (1), Not Casual Mode (0)
            << yDefault                                                     // Force Random (1), Not Force Random (0)                                       
            << yDefault                                                     // Auto Balanced (1), Non Auto Balanced (0)
            << yDefault                                                     // Advanced Options (1), No Advanced Options (0)
            << unDefault                                                    // Min PSR
            << unDefault                                                    // Max PSR
            << yDefault                                                     // Dev Heroes (1), Non Dev Heroes (0)
            << yDefault                                                     // Hardcore (1), Non Hardcore (0)           
            << uiDefault                                                    // Current Game Time
            << uiDefault                                                    // Current Game Phase
            << L""                                                          // Get Legion Team Info
            << L""                                                          // Get Hellbourne Team Info
            << L""                                                          // Get Player 0 Info
            << L""                                                          // Get Player 1 Info
            << L""                                                          // Get Player 2 Info
            << L""                                                          // Get Player 3 Info
            << L""                                                          // Get Player 4 Info
            << L""                                                          // Get Player 5 Info
            << L""                                                          // Get Player 6 Info
            << L""                                                          // Get Player 7 Info
            << L""                                                          // Get Player 8 Info
            << L"";                                                         // Get Player 9 Info
    }
    
    // Strip off the drive from the RootDir (C:\)
    ULONGLONG ulSpaceFree(K2System.GetDriveFreeSpaceEx(K2System.GetRootDir().substr(0, 3)));
    ULONGLONG ulTotalSpace(K2System.GetDriveSizeEx(K2System.GetRootDir().substr(0, 3)));

    m_pktSend
        << m_pHostServer->GetLastServerLoad()                               // Server Load
        << m_pHostServer->GetLongServerFrameCount();                        // # of Long Server Frames
        
    m_pktSend.WriteInt64(K2System.GetFreePhysicalMemory());                 // Free Memory
    m_pktSend.WriteInt64(K2System.GetTotalPhysicalMemory());                // Total Memory
    m_pktSend.WriteInt64(ulSpaceFree);                                      // Drive Space Free
    m_pktSend.WriteInt64(ulTotalSpace);                                     // Drive Space Total
        
    m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CServerChatConnection::UpdateReminderTime
  ====================*/
void    CServerChatConnection::UpdateReminderTime()
{
    m_uiNextConnectReminder = K2System.Milliseconds() + svr_clientReminderInterval;
}


/*====================
  CServerChatConnection::SendConnectionReminder
  ====================*/
void    CServerChatConnection::SendConnectionReminder(uint uiAccountID)
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    m_pktSend.Clear();
    m_pktSend
        << NET_CHAT_GS_REMIND_PLAYER
        << uiAccountID;
    m_pSocket->SendPacket(m_pktSend);

    Console.Server << L"Reminding player #" << uiAccountID << L" to connect." << newl;
}


/*====================
  CServerChatConnection::ReplacePlayer
  ====================*/
void    CServerChatConnection::ReplacePlayer(uint uiAccountID)
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    m_pktSend.Clear();
    m_pktSend
        << NET_CHAT_GS_REPLACE_PLAYER
        << uiAccountID;
    m_pSocket->SendPacket(m_pktSend);

    Console.Server << L"Requesting replacement of player #" << uiAccountID << L"." << newl;
}


/*====================
  CServerChatConnection::SendAbandonMatch
  ====================*/
void    CServerChatConnection::SendAbandonMatch(bool bFailed)
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    m_pktSend.Clear();
    m_pktSend << NET_CHAT_GS_ABANDON_MATCH << byte(bFailed);
    m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CServerChatConnection::SendMatchStarted
  ====================*/
void    CServerChatConnection::SendMatchStarted()
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    m_pktSend.Clear();
    m_pktSend << NET_CHAT_GS_MATCH_STARTED << m_pHostServer->GetMatchupID();
    m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CServerChatConnection::SendMatchAborted
  ====================*/
void    CServerChatConnection::SendMatchAborted(EMatchAbortedReason eReason)
{
    if (m_pSocket == NULL || m_eState != STATE_CONNECTED)
        return;

    m_pktSend.Clear();
    m_pktSend << NET_CHAT_GS_MATCH_ABORTED << m_pHostServer->GetMatchupID() << byte(eReason);
    m_pSocket->SendPacket(m_pktSend);
}


/*====================
  CServerChatConnection::HandleAccept
  ====================*/
bool    CServerChatConnection::HandleAccept(CPacket &pkt)
{
    Console.Server << L"Connected to chat server." << newl;
    m_eState = STATE_CONNECTED;
    m_bSentPing = false;
    SendStatusUpdate();
    return true;
}


/*====================
  CServerChatConnection::HandleReject
  ====================*/
bool    CServerChatConnection::HandleReject(CPacket &pkt)
{
    byte yReason(pkt.ReadByte(SERVER_REJECT_UNKNOWN));
    
    Console.Server << L"Chat server rejected connection: ";
    switch (yReason)
    {
    case SERVER_REJECT_UNKNOWN:
        Console.Net << L"Unknown reason" << newl;
        break;

    case SERVER_REJECT_BAD_VERSION:
        Console.Net << L"Version mismatch" << newl;
        break;

    case SERVER_REJECT_AUTH_FAILED:
        Console.Net << L"Auth failed" << newl;
        break;
    }

    return false;
}


/*====================
  CServerChatConnection::HandleCreateMatch
  ====================*/
bool    CServerChatConnection::HandleCreateMatch(CPacket &pkt)
{
    const uint uiMatchupID(pkt.ReadInt());
    const uint uiChallenge(pkt.ReadInt());
    const wstring sName(pkt.ReadWString());
    const wstring sSettings(pkt.ReadWString());

    if (pkt.HasFaults() || sName.empty() || sSettings.empty())
        return false;

    if (m_pHostServer->GetServerStatus() != SERVER_STATUS_IDLE)
    {
        // Read players from packet
        uint uiPlayerCount(pkt.ReadInt());
        for (uint uiIndex(0); uiIndex < uiPlayerCount; ++uiIndex)
        {
            pkt.ReadInt(); // uiAccountID
            pkt.ReadByte(); // uiTeam
            pkt.ReadByte(); // uiSlot

            if (pkt.HasFaults())
                return false;
        }

        // Get the number of groups that makeup this match and their respective group numbers
        const uint uiGroupCount(pkt.ReadInt());
        uivector vGroupIDs;

        // Read each GroupID
        for (uint i(0); i < uiGroupCount; i++)
            vGroupIDs.push_back(pkt.ReadInt());

        if (pkt.HasFaults())
            return false;

        m_pktSend.Clear();
        m_pktSend << NET_CHAT_GS_NOT_IDLE << uiMatchupID << uiChallenge << uiGroupCount;
        
        // Loop over each GroupID that was sent to us and echo them back
        for (uivector_cit citGroup(vGroupIDs.begin()), citGroupEnd(vGroupIDs.end()); citGroup != citGroupEnd; ++citGroup)
            m_pktSend << *citGroup;

        m_pSocket->SendPacket(m_pktSend);

        Console << L"Ignoring request to create match because server is not idle!" << newl;
        return true;
    }

    m_pHostServer->ClearRoster();

    uint uiPlayerCount(pkt.ReadInt());
    for (uint uiIndex(0); uiIndex < uiPlayerCount; ++uiIndex)
    {
        const uint uiAccountID(pkt.ReadInt(INVALID_ACCOUNT));
        const uint uiTeam(pkt.ReadByte());
        const uint uiSlot(pkt.ReadByte());

        if (pkt.HasFaults())
            return false;

        if (uiAccountID != INVALID_ACCOUNT)
        {
            Console << L"Adding AccountID#" << uiAccountID << L", Slot#" << uiSlot << L" to the roster..." << newl;
            m_pHostServer->AddToRoster(uiAccountID, uiTeam, uiSlot);
        }
    }

    // Get the number of groups that makeup this match and their respective group numbers
    const uint uiGroupCount(pkt.ReadInt());
    uivector vGroupIDs;

    // Read each GroupID
    for (uint i(0); i < uiGroupCount; i++)
        vGroupIDs.push_back(pkt.ReadInt());

    if (pkt.HasFaults())
        return false;
    
    m_pHostServer->SetMatchupID(uiMatchupID);
    
    if (m_pHostServer->StartGame(sName, sSettings))
    {
        m_uiNextConnectReminder = K2System.Milliseconds() + svr_clientReminderInterval;

        m_pktSend.Clear();
        m_pktSend << NET_CHAT_GS_ANNOUNCE_MATCH << uiMatchupID << uiChallenge << uiGroupCount << m_pHostServer->GetGameLib().GetGameInfoInt(L"GetMatchID");
        
        // Loop over each GroupID that was sent to us and echo them back
        for (uivector_cit citGroup(vGroupIDs.begin()), citGroupEnd(vGroupIDs.end()); citGroup != citGroupEnd; ++citGroup)
            m_pktSend << *citGroup;

        m_pSocket->SendPacket(m_pktSend);
    }

    return true;
}


/*====================
  CServerChatConnection::HandleRosterSubstitute
  ====================*/
bool    CServerChatConnection::HandleRosterSubstitute(CPacket &pkt)
{
    uint uiOldPlayerID(pkt.ReadInt());
    uint uiNewPlayerID(pkt.ReadInt());
    if (pkt.HasFaults())
        return false;

    m_pHostServer->SubstituteRoster(uiOldPlayerID, uiNewPlayerID);
    return true;
}


/*====================
  CServerChatConnection::HandleRemoteCommand
  ====================*/
bool    CServerChatConnection::HandleRemoteCommand(CPacket &pkt)
{
    wstring sSession(pkt.ReadWString());
    wstring sCommand(pkt.ReadWString());
    if (pkt.HasFaults())
        return false;

    if (sSession != m_pHostServer->GetSessionCookie())
    {
        Console << L"HandleRemoteCommand() - invalid session cookie." << newl;
        return true;
    }

    Console.Execute(sCommand);
    return true;
}


/*====================
  CServerChatConnection::ReadSocket
  ====================*/
void    CServerChatConnection::ReadSocket()
{
    if (m_pSocket == NULL)
        return;

    if (!m_pSocket->IsConnected())
    {
        Reconnect(svr_chatReconnectDelay, _T("ReadSocket: Not connected"));
        return;
    }

    bool bStayConnected(true);
    while (bStayConnected)
    {
        CPacket pktRecv;
        int iRecvLength(m_pSocket->ReceivePacket(pktRecv));
        if (iRecvLength < 1)
        {
            if (iRecvLength < 0)
            {
                Console.Server << L"Error reading chat socket." << newl;
                bStayConnected = false;
            }
            break;
        }

        m_uiLastReceiveTime = K2System.Milliseconds();

        while (!pktRecv.DoneReading())
        {
            ushort unCmd(pktRecv.ReadShort(NET_CHAT_INVALID));
            switch (unCmd)
            {
            case NET_CHAT_PING:
                m_pktSend.Clear();
                m_pktSend << NET_CHAT_PONG;
                m_pSocket->SendPacket(m_pktSend);
                break;

            case NET_CHAT_PONG:
                //Console.Server << L"Chat server PONG @ " << m_uiLastReceiveTime << newl;

                m_bSentPing = false;
                break;

            case NET_CHAT_GS_ACCEPT:
                bStayConnected = HandleAccept(pktRecv);

                if (!bStayConnected)
                    Console.Server << L"Bad NET_CHAT_GS_ACCEPT from chat server" << newl;
                break;

            case NET_CHAT_GS_REJECT:
                bStayConnected = HandleReject(pktRecv);

                if (!bStayConnected)
                    Console.Server << L"Bad NET_CHAT_GS_REJECT from chat server" << newl;
                break;

            case NET_CHAT_GS_CREATE_MATCH:
                bStayConnected = HandleCreateMatch(pktRecv);

                if (!bStayConnected)
                    Console.Server << L"Bad NET_CHAT_GS_CREATE_MATCH from chat server" << newl;
                break;

            case NET_CHAT_GS_ROSTER_SUBSTITUTE:
                bStayConnected = HandleRosterSubstitute(pktRecv);

                if (!bStayConnected)
                    Console.Server << L"Bad NET_CHAT_GS_ROSTER_SUBSTITUTE from chat server" << newl;
                break;

            case NET_CHAT_GS_REMOTE_COMMAND:
                bStayConnected = HandleRemoteCommand(pktRecv);

                if (!bStayConnected)
                    Console.Server << L"Bad NET_CHAT_GS_REMOTE_COMMAND from chat server" << newl;
                break;

            default:
            case NET_CHAT_INVALID:
                Console.Server << L"Game server received bad data from chat server." << newl;
                bStayConnected = false;
                break;
            }

            if (pktRecv.HasFaults())
            {
                Console.Server << L"Game server received bad data from chat server." << newl;
                bStayConnected = false;
            }

            if (!bStayConnected)
                break;
        }

        if (!bStayConnected)
            break;
    }

    if (!bStayConnected)
        Reconnect(svr_chatReconnectDelay, _T("ReadSocket: Reconnect"));
}


CVAR_BOOL(_debugTimeout, false);

/*====================
  CServerChatConnection::Frame
  ====================*/
void    CServerChatConnection::Frame()
{
    switch (m_eState)
    {
    case STATE_IDLE:
        break;

    case STATE_DISCONNECTED:
        if (K2System.Milliseconds() >= m_uiReconnectTime)
            Connect(m_sAddress, m_unPort);
        break;

    case STATE_CONNECTING:
        if (m_pSocket != NULL && m_pSocket->IsConnected())
            Handshake();
        else if (m_pSocket == NULL || K2System.Milliseconds() >= m_uiTimeout)
            Reconnect(svr_chatReconnectDelay, _T("CONNECTING: Timeout"));
        break;

    case STATE_AUTHENTICATING:
        if (m_pSocket == NULL || m_pSocket->HasError() || K2System.Milliseconds() - m_uiLastReceiveTime >= svr_chatConnectedTimeout)
        {
            tstring sReason(_T("AUTHENTICATING: "));

            if (m_pSocket == NULL)
                sReason += _T("NULL socket");
            else if (m_pSocket->HasError())
                sReason += _T("Socket error");
            else
                sReason += _T("Timeout");

            Reconnect(svr_chatReconnectDelay, sReason);
        }
        else
            ReadSocket();
        break;

    case STATE_CONNECTED:
        if (m_pSocket == NULL || m_pSocket->HasError() || K2System.Milliseconds() - m_uiLastReceiveTime >= svr_chatConnectedTimeout)
        {
            tstring sReason(_T("CONNECTED: "));

            if (m_pSocket == NULL)
                sReason += _T("NULL socket");
            else if (m_pSocket->HasError())
                sReason += _T("Socket error");
            else
                sReason += _T("Timeout");

            Reconnect(svr_chatReconnectDelay, sReason);
            break;
        }
        
        if (!m_bSentPing && K2System.Milliseconds() - m_uiLastReceiveTime >= svr_chatConnectedTimeout / 2)
        {
            if (_debugTimeout)
                Console.Server << L"Game server PING @ " << K2System.Milliseconds() << newl;

            m_pktSend.Clear();
            m_pktSend << NET_CHAT_PING;

            uint uiByteSent(m_pSocket->GetBytesSent());

            m_pSocket->SendPacket(m_pktSend);

            if (_debugTimeout)
                Console << m_pSocket->GetBytesSent() - uiByteSent << _T(" bytes sent") << newl;
            
            m_bSentPing = true;
        }

        ReadSocket();
        break;
    }
}

#endif //K2_CLIENT
