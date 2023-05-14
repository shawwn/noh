// (C)2007 S2 Games
// c_chatmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmldoc.h"
#include "c_chatmanager.h"
#include "c_uicmd.h"
#include "c_uitrigger.h"
#include "c_clientlogin.h"
#include "c_uimanager.h"
#include "c_soundmanager.h"
#include "c_stringtable.h"
#include "c_hostclient.h"
#include "c_date.h"
#include "c_uicmdregistry.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CChatManager    *pChatManager(CChatManager::GetInstance());
SINGLETON_INIT(CChatManager)

UI_TRIGGER(ChatStatus);
UI_TRIGGER(ChatUpdateName);

UI_TRIGGER(ChatChanEvent);
UI_TRIGGER(ChatChanNumUsers);
UI_TRIGGER(ChatChanTopic);
UI_TRIGGER(ChatUserNames);
UI_TRIGGER(ChatUserEvent);

UI_TRIGGER(ChatBuddyOnline);
UI_TRIGGER(ChatBuddyOffline);
UI_TRIGGER(ChatBuddyGame);
UI_TRIGGER(ChatBuddyEvent);

UI_TRIGGER(ChatClanOnline);
UI_TRIGGER(ChatClanOffline);
UI_TRIGGER(ChatClanGame);
UI_TRIGGER(ChatClanEvent);

UI_TRIGGER(ChatBanEvent);
UI_TRIGGER(ChatInviteEvent);

UI_TRIGGER(ChatNewChannel);
UI_TRIGGER(ChatChannelList);
UI_TRIGGER(ChatLeftChannel);

UI_TRIGGER(ChatNewGame);
UI_TRIGGER(ChatLeftGame);

UI_TRIGGER(ChatNotificationBuddy);
UI_TRIGGER(ChatNotificationClan);
UI_TRIGGER(ChatNotificationMessage);
UI_TRIGGER(ChatNotificationInvite);
UI_TRIGGER(ChatNotificationGroupInvite);
UI_TRIGGER(ChatNotificationHistoryPerformCMD);

UI_TRIGGER(ChatWhisperUpdate);
UI_TRIGGER(ChatClanWhisperUpdate);

UI_TRIGGER(ChatCloseIM);

UI_TRIGGER(ChatMessageTrigger);
UI_TRIGGER(ChatClanMessageTrigger);

UI_TRIGGER(ChatLookingForClanEvent);
UI_TRIGGER(ChatRecentlyPlayedClan);
UI_TRIGGER(ChatRecentlyPlayedBuddy);

UI_TRIGGER(ChatHoverName);
UI_TRIGGER(ChatHoverClan);
UI_TRIGGER(ChatHoverServer);
UI_TRIGGER(ChatHoverGameTime);
UI_TRIGGER(ChatHoverAvatar);

UI_TRIGGER(ChatRecentlyPlayedPlayer);
UI_TRIGGER(ChatRecentlyPlayedHeader);
UI_TRIGGER(ChatRecentlyPlayedEvent);

UI_TRIGGER(ChatAutoCompleteClear);
UI_TRIGGER(ChatAutoCompleteAdd);

UI_TRIGGER(ChatCloseNotifications);

UI_TRIGGER(ChatPasswordRequired);
UI_TRIGGER(ChatClanInvite);

UI_TRIGGER(ChatClanCreateFail);
UI_TRIGGER(ChatClanCreateSuccess);
UI_TRIGGER(ChatClanCreateAccept);
UI_TRIGGER(ChatClanCreateTime);

UI_TRIGGER(ChatClanCreateTip);

UI_TRIGGER(ChatShowPostGameStats);

UI_TRIGGER(ChatTotalFriends);
UI_TRIGGER(ChatOnlineFriends);

UI_TRIGGER(ChatTotalClanMembers);
UI_TRIGGER(ChatOnlineClanMembers);

UI_TRIGGER(ChatUsersOnline);

UI_TRIGGER(ChatNumUnreadChannels);
UI_TRIGGER(ChatUnreadChannel);

UI_TRIGGER(ChatSetFocusChannel);
UI_TRIGGER(ChatFocusedIM);
UI_TRIGGER(ChatUnreadIM);

UI_TRIGGER(ChatReceivedIMCount);
UI_TRIGGER(ChatReadIMCount);
UI_TRIGGER(ChatUnreadIMCount);
UI_TRIGGER(ChatSentIMCount);
UI_TRIGGER(ChatOpenIMCount);
UI_TRIGGER(ChatNotificationCount);

UI_TRIGGER(ChatUserStatus);

UI_TRIGGER(ChatRecievedChannelMessage);

UI_TRIGGER(TMMDebugInfo);
UI_TRIGGER(TMMTime);
UI_TRIGGER(TMMDisplay);
UI_TRIGGER(TMMDisplayPopularity);

UI_TRIGGER(TMMReset);
UI_TRIGGER(TMMFoundMatch);
UI_TRIGGER(TMMFoundServer);
UI_TRIGGER(TMMOptionsAvailable);
UI_TRIGGER(TMMAvailable);
UI_TRIGGER(TMMReadyStatus);
UI_TRIGGER(TMMJoinGroup);
UI_TRIGGER(TMMLeaveGroup);
UI_TRIGGER(TMMJoinQueue);
UI_TRIGGER(TMMLeaveQueue);
UI_TRIGGER(TMMJoinMatch);
UI_TRIGGER(TMMServerNotIdle);
UI_TRIGGER(TMMNoMatchesFound);
UI_TRIGGER(TMMNoServersFound);

UI_TRIGGER(TMMPlayerStatus0);
UI_TRIGGER(TMMPlayerStatus1);
UI_TRIGGER(TMMPlayerStatus2);
UI_TRIGGER(TMMPlayerStatus3);
UI_TRIGGER(TMMPlayerStatus4);

CUITrigger *TMMPlayerStatus[] =
{
    &TMMPlayerStatus0,
    &TMMPlayerStatus1,
    &TMMPlayerStatus2,
    &TMMPlayerStatus3,
    &TMMPlayerStatus4
};

UI_TRIGGER(ChatRequestGameInfo);


CVAR_BOOLF(     cc_showBuddyConnectionNotification,     true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanConnectionNotification,      true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyDisconnectionNotification,  true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanDisconnectionNotification,   true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyJoinGameNotification,       true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanJoinGameNotification,        true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyLeaveGameNotification,      true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanLeaveGameNotification,       true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyRequestNotification,    true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyAddNotification,        true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showBuddyRemovedNotification,    true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanRankNotification,        true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanAddNotification,         true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanRemoveNotification,      true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showClanMessageNotification,     true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showIMNotification,              true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showGameInvites,                 true,           CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_showNewPatchNotification,        true,           CVAR_SAVECONFIG);

CVAR_UINTF(     cc_notificationDuration,            10,             CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_DisableNotifications,            false,          CVAR_SAVECONFIG);
CVAR_BOOLF(     cc_DisableNotificationsInGame,      false,          CVAR_SAVECONFIG);

CVAR_UINTF(     chat_gameLobbyChatToggle,           274,            CVAR_SAVECONFIG);   // default to the value of the alt key when pressed (274)

CVAR_BOOLF(     cg_censorChat,                      true,           CVAR_SAVECONFIG);

CVAR_UINTF(     chat_connectTimeout,                3,              CVAR_SAVECONFIG);

CVAR_BOOLF(     cc_lookingForClan,                  false,          CVAR_SAVECONFIG);

CVAR_STRINGF(   cc_curGameChannel,                  "",             CVAR_SAVECONFIG);
CVAR_UINT(      cc_curGameChannelID,                -1);

CVAR_BOOL(      cc_forceTMMInterfaceUpdate,                 false);

#if 0
CVAR_STRING(    chat_serverAddrOverride,            "64.20.203.130");
//CVAR_STRING(  chat_serverAddrOverride,            "127.0.0.1");
#else
CVAR_STRING(    chat_serverAddrOverride,            ""); // TKTK NOTE: chat server is now chat.kongor.online
#endif

CVAR_BOOL(      chat_serverTCP2,                    true);
CVAR_UINTF(     chat_serverPort,                    11031,          CVAR_SAVECONFIG);
CVAR_UINTF(     chat_serverTestPort,                11032,          CVAR_SAVECONFIG);

CVAR_BOOL(      chat_profile,                       false);
CVAR_BOOL(      chat_debugInterface,                false);

CVAR_UINTF(     chat_maxReconnectAttempts,          5,              CVAR_SAVECONFIG);

CVAR_BOOLF(     chat_showChatTimestamps,            false,          CVAR_SAVECONFIG);

static const tstring LOCALHOST(_T("127.0.0.1"));
//=============================================================================

/*====================
  CChatManager::CChatManager
  ====================*/
CChatManager::CChatManager() :
m_pHTTPManager(NULL),

m_pNamesRequest(NULL),

m_sockChat(_T("ChatSocket")),
m_cDate(CDate(true)),
m_eStatus(CHAT_STATUS_DISCONNECTED),
m_uiConnectTimeout(INVALID_TIME),
m_uiLastRecvTime(INVALID_TIME),
m_bBuddyUpdateRequired(false),
m_bClanUpdateRequired(false),
m_uiIgnoreChat(CHAT_IGNORE_NONE),
m_uiConnectRetries(0),
m_uiAccountID(INVALID_ACCOUNT),
m_hStringTable(INVALID_RESOURCE),
m_bPrivateGame(false),
m_bHost(false),
m_uiNextReconnectTime(INVALID_TIME),
m_pRecentlyPlayed(NULL),
m_uiMatchID(-1),
m_bRetrievingStats(false),
m_uiHistoryPos(0),
m_uiCreateTimeSent(INVALID_TIME),
m_bMatchStarted(false),
m_bWaitingToShowStats(false),
m_uiShowStatsMatchID(INVALID_INDEX),
m_uiFocusedChannel(-1),
m_uiFocusCount(0),
m_uiLastIMNotificationTime(0),
m_uiReceivedIMCount(0),
m_uiReadIMCount(0),
m_uiSentIMCount(0),
m_uiNotificationIndex(0),

m_uiTMMStartTime(INVALID_TIME),
m_uiTMMAverageQueueTime(INVALID_TIME),
m_uiTMMStdDevQueueTime(INVALID_TIME),
m_bInGroup(false),
m_bTMMEnabled(false),

m_bFinishedList(false),
m_yListStartSequence(0),
m_yProcessingListSequence(0xff),
m_yFinishedListSequence(0xff),
m_bInGameLobby(false),

m_bWhisperMode(false),
m_uiTabNumber(0),

m_bFollow(false),
m_sFollowName(_T("")),

m_uiTMMGroupLeaderID(INVALID_INDEX),
m_bTMMOtherPlayersReady(false),
m_bTMMAllPlayersReady(false),
m_uiTMMSelfGroupIndex(0)
{
}


/*====================
  CChatManager::~CChatManager
  ====================*/
CChatManager::~CChatManager()
{
    if (m_uiAccountID == INVALID_ACCOUNT || m_eStatus > CHAT_STATUS_CONNECTED)
        LeftGame();

    Disconnect();

    K2_DELETE(m_pRecentlyPlayed);
}


/*====================
  CChatManager::AddGameChatMessage
  ====================*/
void    CChatManager::AddGameChatMessage(EChatMessageType eType, const tstring &sMessage)
{
    PROFILE("AddGameChatMessage");

    tstring sMsg(sMessage);
    
    if (cg_censorChat)
        CensorChat(sMsg, false);
        
    if (chat_showChatTimestamps)
        m_cDate = CDate(true);      

    for (ChatWidgetReference_it it(m_vWidgets.begin()); it != m_vWidgets.end(); it++)
    {
        if (!it->bGameChat || !it->refWidget.IsValid())
            continue;
        
        switch (eType)
        {
            case CHAT_MESSAGE_CLEAR:
                static_cast<CTextBuffer*>(it->refWidget.GetTarget())->ClearText();
                break;
                
            case CHAT_MESSAGE_ROLL:
                if (chat_showChatTimestamps)
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(_T("[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ^190") + sMsg + _T("^*"), Host.GetTime());               
                else
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(_T("^190") + sMsg + _T("^*"), Host.GetTime());                
                break;

            case CHAT_MESSAGE_EMOTE:
                if (chat_showChatTimestamps)
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(_T("[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ^839") + sMsg + _T("^*"), Host.GetTime());
                else
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(_T("^839") + sMsg + _T("^*"), Host.GetTime());
                break;
                
            case CHAT_MESSAGE_ADD:
            default:
                if (chat_showChatTimestamps)
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(_T("[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + sMsg, Host.GetTime());
                else
                    static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText(sMsg, Host.GetTime());
                break;
        }
    }
}


/*====================
  CChatManager::AddIRCChatMessage
  ====================*/
void    CChatManager::AddIRCChatMessage(EChatMessageType eType, const tstring &sMessage, const tstring &sChannel, bool bTimeStamp)
{
    tstring sMsg(sMessage);
    
    if (cg_censorChat)
        CensorChat(sMsg, false);
        
    m_cDate = CDate(true);      

    for (ChatWidgetReference_it it(m_vWidgets.begin()); it != m_vWidgets.end(); it++)
    {
        if ((!sChannel.empty() && CompareNoCase(sChannel, it->sChannel) != 0) || (!sChannel.empty() && it->bGameChat) || !it->refWidget.IsValid())
            continue;
        
        switch (eType)
        {
            case CHAT_MESSAGE_CLEAR:
                static_cast<CTextBuffer*>(it->refWidget.GetTarget())->ClearText();
                break;

            case CHAT_MESSAGE_ROLL:
                static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText((bTimeStamp ? _T("^190[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + sMsg + _T("^*") : sMsg), Host.GetTime());             
                break;

            case CHAT_MESSAGE_EMOTE:
                static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText((bTimeStamp ? _T("^839[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + sMsg + _T("^*") : sMsg), Host.GetTime());
                break;

            case CHAT_MESSAGE_ADD:
            default:
            {
                static_cast<CTextBuffer*>(it->refWidget.GetTarget())->AddText((bTimeStamp ? _T("^770[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + sMsg : sMsg), Host.GetTime());
                break;
            }
        }
    }
}

/*====================
  CChatManager::SetCurrentChatMessage
  ====================*/
tstring     CChatManager::SetCurrentChatMessage(const tstring &sMessage)
{
    // always reply to a /w buddy msg, /b m msg or /c m msg directly to the sender, not to the group
    if (m_lLastWhispers.size() > 0 && (CompareNoCase(sMessage, Translate(_T("chat_command_reply")) + _T(" ")) == 0 || CompareNoCase(sMessage, Translate(_T("chat_command_reply_short")) + _T(" ")) == 0))
        m_sCurrentMessage = Translate(_T("chat_command_whisper_short")) + _T(" ") + m_lLastWhispers.front() + _T(" ");
    else
        m_sCurrentMessage = sMessage;

    tstring sTempString = LowerString(m_sCurrentMessage);
    
    //Set WhisperMode
    if (m_lLastWhispers.size() > 0 && sTempString.find(Translate(_T("chat_command_whisper_short"))) == 0)
    {
        m_bWhisperMode = true;
    }
    else
    {
        m_bWhisperMode = false;
        m_uiTabNumber = 0;
    }

    return m_sCurrentMessage;
}


/*====================
  CChatManager::Connect
  ====================*/
void    CChatManager::Connect(bool bInvisible)
{
    if (m_uiAccountID == INVALID_ACCOUNT || m_eStatus != CHAT_STATUS_DISCONNECTED)
    {
        Console << _T("Invalid account or not disconnected: ") << m_uiAccountID << SPACE << m_eStatus << newl;
        return;
    }

    if (m_sCookie.empty())
    {
        Console << _T("Cookie empty: ") << m_sCookie << newl;
        return;
    }

    if (!m_sockChat.IsInitialized())
        m_sockChat.Init(chat_serverTCP2 ? K2_SOCKET_TCP2 : K2_SOCKET_TCP);

    if (m_sockChat.IsConnected())
        HandleDisconnect();

    if (m_uiConnectRetries == 0 || m_uiConnectRetries == -1)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_connecting")));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_reconnecting"), _T("attempt"), XtoA(m_uiConnectRetries), _T("maxattempts"), XtoA(chat_maxReconnectAttempts)));
        
    // set the clients chat mode type to be 'available' upon connecting, or set it to be 'invisible'
    if (bInvisible)
        SetChatModeType(CHAT_MODE_INVISIBLE, _T(""), true);
    else
        SetChatModeType(CHAT_MODE_AVAILABLE, _T(""), true);
    
    LeaveTMMGroup(true);

    // Always reset the TMM screen so it isn't stuck looking like we were in a group
    TMMReset.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

    m_mapChannels.clear();

    m_mapIMs.clear();
    m_mapIMUnreadCount.clear();
    
    m_uiNotificationIndex = 0;
    
    m_uiReceivedIMCount= 0;
    m_uiReadIMCount = 0;
    m_uiSentIMCount = 0;        

#ifdef K2_TEST
    if (!m_sockChat.SetSendAddr(chat_serverAddrOverride.empty() ? m_sChatAddress : chat_serverAddrOverride, chat_serverTestPort))
#else
    if (!m_sockChat.SetSendAddr(chat_serverAddrOverride.empty() ? m_sChatAddress : chat_serverAddrOverride, chat_serverPort))
#endif
    {
        Console << _T("SetSendAddr failed") << newl;
        return;
    }

    m_eStatus = CHAT_STATUS_CONNECTING;
    m_uiConnectTimeout = Host.GetTimeSeconds() + chat_connectTimeout;
    m_uiLastRecvTime = K2System.Milliseconds();
    m_uiNextReconnectTime = INVALID_TIME;

    if (m_uiConnectRetries == -1)
        m_uiConnectRetries = 0;

    tsvector vMiniParams(2);

    vMiniParams[0] = _T("irc_status_chan");
    vMiniParams[1] = K2System.GetGameName() + _T(" - v") + K2_Version(K2System.GetVersionString());
    ChatChanTopic.Trigger(vMiniParams);

    SetFocusedChannel(-1);

    m_uiTMMGroupLeaderID = INVALID_INDEX;
    m_bTMMOtherPlayersReady = false;
    m_bTMMAllPlayersReady = false;
}


/*====================
  CChatManager::SetInfo
  ====================*/
void    CChatManager::SetInfo(int iAccountID, const tstring &sCookie, const tstring &sNickname, const tstring &sClan, const tstring &sClanTag, int iClanID, EClanRank eClanRank, byte yFlags, uint uiChatSymbol, uint uiChatNameColor, uint uiAccountIcon)
{
    if (m_uiAccountID != INVALID_ACCOUNT)
    {
        m_setRecentlyPlayed.clear();
        m_pRecentlyPlayed->EndNode();
    }

    bool bTraversed = m_pRecentlyPlayed->TraverseChildren();
    bool bFound = false;

    if (bTraversed)
    {
        bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(iAccountID));

        while (!bFound && m_pRecentlyPlayed->TraverseNextChild())
            bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(iAccountID));
    }

    if (!bFound)
    {
        if (bTraversed)
            m_pRecentlyPlayed->EndNode();

        m_pRecentlyPlayed->NewNode(_T("user"));
        m_pRecentlyPlayed->AddProperty(_T("id"), XtoA(iAccountID));
        m_pRecentlyPlayed->AddProperty(_T("name"), sNickname);
    }

    m_uiAccountID = iAccountID;
    m_sCookie = sCookie;

    ChatClientMap_it findit(m_mapUserList.find(m_uiAccountID));

    if (findit == m_mapUserList.end())
    {
        SChatClient structClient;
        structClient.uiAccountID = iAccountID;
        structClient.uiMatchID = -1;
        structClient.yFlags = 0;

        m_mapUserList.insert(ChatClientPair(iAccountID, structClient));
        findit = m_mapUserList.find(iAccountID);
    }

    findit->second.yStatus = CHAT_STATUS_DISCONNECTED;
    findit->second.sName = sNickname;
    findit->second.iClanID = iClanID;
    findit->second.sClan = sClan;
    findit->second.sClanTag = sClanTag;
    findit->second.yFlags |= yFlags;
    findit->second.uiChatSymbol = uiChatSymbol;
    findit->second.uiChatNameColor = uiChatNameColor;
    findit->second.uiAccountIcon = uiAccountIcon;

    uint uiChatNameColor2(uiChatNameColor);

    if (yFlags & CHAT_CLIENT_IS_STAFF && uiChatNameColor2 == INVALID_INDEX)
    {
        uint uiDevChatNameColor(Host.LookupChatNameColor(_CTS("s2logo")));
        if (uiDevChatNameColor != INVALID_INDEX)
            uiChatNameColor2 = uiDevChatNameColor;
    }
    if (yFlags & CHAT_CLIENT_IS_PREMIUM && uiChatNameColor2 == INVALID_INDEX)
    {
        uint uiGoldChatNameColor(Host.LookupChatNameColor(_CTS("goldshield")));
        if (uiGoldChatNameColor != INVALID_INDEX)
            uiChatNameColor2 = uiGoldChatNameColor;
    }

    if (uiChatNameColor2 != INVALID_INDEX)
        findit->second.uiSortIndex = Host.GetChatNameColorSortIndex(uiChatNameColor2);
    else
        findit->second.uiSortIndex = 9;

    UpdateRecentlyPlayed();
}


/*====================
  CChatManager::Init
  ====================*/
void    CChatManager::Init(CHTTPManager *pHTTPManager)
{
    m_pHTTPManager = pHTTPManager;
    m_sMasterServerURL = K2System.GetMasterServerAddress() + "/client_requester.php";

    m_pNamesRequest = m_pHTTPManager->SpawnRequest();
    m_pNamesRequest->SetTargetURL(m_sMasterServerURL);

    // Dedicated servers don't play sounds or add to the recentlyplayed/notes so skip this and save some memory
    if (K2System.IsDedicatedServer() || K2System.IsServerManager())
        return;

    m_hStringTable = g_ResourceManager.Register(_T("/stringtables/chat_sounds.str"), RES_STRINGTABLE);

    m_pRecentlyPlayed = K2_NEW(ctx_Net,  CXMLDoc)();

    CFile *pFile(FileManager.GetFile(_T("~/recentlyplayed.xml"), FILE_READ | FILE_TEXT | FILE_ALLOW_CUSTOM));
    if (pFile != NULL)
    {
        uint uiFileSize(0);
        const char *pBuffer(pFile->GetBuffer(uiFileSize));

        if (pBuffer != NULL && uiFileSize > 0)
            m_pRecentlyPlayed->ReadBuffer(pBuffer, uiFileSize);
        else
            m_pRecentlyPlayed->NewNode(_T("recentlyplayed"));

        pFile->Close();
        SAFE_DELETE(pFile);
    }

    pFile = FileManager.GetFile(_T("~/notes.txt"), FILE_READ | FILE_TEXT | FILE_ALLOW_CUSTOM);
    if (pFile != NULL)
    {
        while (!pFile->IsEOF())
        {
            tstring sLine(pFile->ReadLine());
            tsvector vsTokens(TokenizeString(sLine, _T('|')));

            if (vsTokens.size() >= 3)
            {
                m_vNoteTimes.push_back(vsTokens[1]);
                m_vNotes.push_back(vsTokens[2]);
            }
        }

        pFile->Close();
        SAFE_DELETE(pFile);
    }
}


/*====================
  CChatManager::Frame
  ====================*/
void    CChatManager::Frame()
{
    DatabaseFrame();

    ChatStatus.Trigger(XtoA(IsConnected()));

    if (m_uiAccountID != INVALID_ACCOUNT)
    {
        switch (m_eStatus)
        {
        case CHAT_STATUS_CONNECTING:
            ConnectingFrame();
            break;

        case CHAT_STATUS_WAITING_FOR_AUTH:
            AuthFrame();
            break;

        case CHAT_STATUS_CONNECTED:
        case CHAT_STATUS_JOINING_GAME:
        case CHAT_STATUS_IN_GAME:
            ConnectedFrame();
            break;

        default:
            break;
        }
    }

    if (m_uiNextReconnectTime != INVALID_TIME && m_uiNextReconnectTime < K2System.Milliseconds() && !IsConnected())
    {
        m_uiNextReconnectTime = INVALID_TIME;
        Connect();
    }

    if (m_bBuddyUpdateRequired)
    {
        m_bBuddyUpdateRequired = false;
        UpdateBuddyList();
    }

    if (m_bClanUpdateRequired)
    {
        m_bClanUpdateRequired = false;
        UpdateClanList();
    }

    // TMM UI updates
    if (m_uiTMMStartTime != INVALID_TIME)
    {
        static tsvector vMiniParams(3);

        if (m_uiTMMAverageQueueTime == INVALID_TIME || m_uiTMMStdDevQueueTime == INVALID_TIME)
        {
            vMiniParams[0] = XtoA(0);
            vMiniParams[1] = XtoA(0);
            vMiniParams[2] = XtoA(0);
        }
        else
        {
            const int iLower(m_uiTMMAverageQueueTime - m_uiTMMStdDevQueueTime);
            //const int iHigher(m_uiTMMAverageQueueTime + m_uiTMMStdDevQueueTime);
        
            vMiniParams[0] = XtoA(Host.GetTime() - m_uiTMMStartTime);

            if (iLower < 0)
            {
                vMiniParams[1] = XtoA(m_uiTMMAverageQueueTime);
                vMiniParams[2] = XtoA(m_uiTMMAverageQueueTime);         
            }
            else
            {
                //vMiniParams[1] = XtoA(iLower);
                //vMiniParams[2] = XtoA(iHigher);
                vMiniParams[1] = XtoA(m_uiTMMAverageQueueTime);
                vMiniParams[2] = XtoA(m_uiTMMAverageQueueTime);         
            }
        }

        TMMTime.Trigger(vMiniParams, cc_forceTMMInterfaceUpdate);
    }
}


/*====================
  CChatManager::ProcessFailedRequest
  ====================*/
void    CChatManager::ProcessFailedRequest(SChatDBRequest *pRequest)
{
    switch (pRequest->eType)
    {
    case REQUEST_ADD_BUDDY_NICK2ID:
    case REQUEST_ADD_BUDDY:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_DELETE_BUDDY:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_CLAN_PROMOTE:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_promote"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_CLAN_DEMOTE:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_demote"), _T("target"), pRequest->sTarget));
        break;
        
    case REQUEST_CLAN_REMOVE:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_UPDATE_CLAN:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_update"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_ADD_BANNED_NICK2ID:
    case REQUEST_ADD_BANNED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_REMOVE_BANNED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_GET_BANNED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_get"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_ADD_IGNORED_NICK2ID:
    case REQUEST_ADD_IGNORED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_REMOVE_IGNORED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_SAVE_CHANNEL:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_channel_save"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_REMOVE_CHANNEL:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_channel_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_SAVE_NOTIFICATION:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_notification_save"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_REMOVE_NOTIFICATION:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_notification_remove"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_REMOVE_ALL_NOTIFICATIONS:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_notification_remove_all")));
        break;

    case REQUEST_COMPLETE_NICK:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_complete_nick"), _T("target"), pRequest->sTarget));
        break;

    case REQUEST_CHECK_CLAN_NAME:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_name")));
        ChatClanCreateTip.Trigger(Translate(_T("chat_failed_clan_name_tip")));
        break;
    }
}


/*====================
  CChatManager::ProcessAddBuddyLookupIDSuccess
  ====================*/
void    CChatManager::ProcessAddBuddyLookupIDSuccess(SChatDBRequest *pRequest)
{
/*
    CPHPData phpResponse(pRequest->pRequest->GetResponse());

    const CPHPData *pAccountID(phpResponse.GetVar(1));
    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL && pAccountID != NULL)
    {
        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("new_buddy"));
        pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
        pHTTPRequest->AddVariable(_T("buddy_id"), pAccountID->GetInteger());
        pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net) SChatDBRequest(pHTTPRequest, REQUEST_ADD_BUDDY, pAccountID->GetInteger()));
        pNewRequest->sTarget = pRequest->sTarget;

        m_lHTTPRequests.push_back(pNewRequest);
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add"), _T("target"), pRequest->sTarget));
    }
*/
}


/*====================
  CChatManager::ProcessAddBuddySuccess
  ====================*/
void    CChatManager::ProcessAddBuddySuccess(SChatDBRequest *pRequest)
{
/*
    CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        AddBuddy(pRequest->uiTarget, pRequest->sTarget);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_buddy_add"), _T("target"), pRequest->sTarget));

        const CPHPData *pNotify(phpResponse.GetVar(_T("notification")));
        if (pNotify != NULL && IsConnected())
        {
            uint uiNotify1(pNotify->GetInteger(_T("1")));
            uint uiNotify2(pNotify->GetInteger(_T("2")));

            CPacket pktSend;
            pktSend << CHAT_CMD_BUDDY_ADD << pRequest->uiTarget << uiNotify1 << uiNotify2;
            m_sockChat.SendPacket(pktSend);
        }
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add"), _T("target"), pRequest->sTarget));
    }
*/
}


/*====================
  CChatManager::ProcessRemoveBuddySuccess
  ====================*/
void    CChatManager::ProcessRemoveBuddySuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("remove_buddy")), _T("OK")) == 0 && phpResponse.GetVar(_T("error")) == NULL)
    {
        // Update our buddy list...
        RemoveBuddy(pRequest->uiTarget);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_buddy_remove"), _T("target"), RemoveClanTag(pRequest->sTarget)));

        const CPHPData *pNotify(phpResponse.GetVar(_T("notification")));
        if (pNotify != NULL && IsConnected())
        {
            const uint uiNotify1(pNotify->GetInteger(_T("1")));
            const uint uiNotify2(pNotify->GetInteger(_T("2")));

            CPacket pktSend;
            pktSend << CHAT_CMD_REQUEST_BUDDY_REMOVE << pRequest->uiTarget << uiNotify1 << uiNotify2;
            m_sockChat.SendPacket(pktSend);
        }
    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_remove"), _T("target"), RemoveClanTag(pRequest->sTarget)));
    }
}


/*====================
  CChatManager::ProcessClanPromoteSuccess
  ====================*/
void    CChatManager::ProcessClanPromoteSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("set_rank")), _T("Member updated.")) == 0 && phpResponse.GetVar(_T("error")) == NULL)
    {
        // Update our clan list...
        ChatClientMap_it itFind(m_mapUserList.find(pRequest->uiTarget));
        if (itFind != m_mapUserList.end())
        {
            itFind->second.yFlags |= CHAT_CLIENT_IS_OFFICER;
            RefreshClanList();
        }

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_clan_promote"), _T("target"), RemoveClanTag(pRequest->sTarget)));

        CPacket pktSend;
        pktSend << CHAT_CMD_CLAN_PROMOTE_NOTIFY << pRequest->uiTarget;
        m_sockChat.SendPacket(pktSend);

    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_promote_generic")));
    }
}


/*====================
  CChatManager::ProcessClanDemoteSuccess
  ====================*/
void    CChatManager::ProcessClanDemoteSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("set_rank")), _T("Member updated.")) == 0 && phpResponse.GetVar(_T("error")) == NULL)
    {
        // Update our clan list...
        ChatClientMap_it itFind(m_mapUserList.find(pRequest->uiTarget));
        if (itFind != m_mapUserList.end())
        {
            itFind->second.yFlags &= ~CHAT_CLIENT_IS_OFFICER;
            RefreshClanList();
        }

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_clan_demote"), _T("target"), RemoveClanTag(pRequest->sTarget)));

        CPacket pktSend;
        pktSend << CHAT_CMD_CLAN_DEMOTE_NOTIFY << pRequest->uiTarget;
        m_sockChat.SendPacket(pktSend);
    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_demote_generic")));
    }
}


/*====================
  CChatManager::ProcessClanRemoveSuccess
  ====================*/
void    CChatManager::ProcessClanRemoveSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("set_rank")), _T("Member updated.")) == 0 && phpResponse.GetVar(_T("error")) == NULL)
    {
        CPacket pktSend;
        pktSend << CHAT_CMD_CLAN_REMOVE_NOTIFY << pRequest->uiTarget;
        m_sockChat.SendPacket(pktSend);

        if (pRequest->uiTarget != m_uiAccountID)
        {
            // Update our clan list...
            RemoveClanMember(pRequest->uiTarget);
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_clan_remove"), _T("target"), RemoveClanTag(pRequest->sTarget)));
        }
    }
    else
    {
        if (pRequest->uiTarget != m_uiAccountID)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_remove"), _T("target"),RemoveClanTag(pRequest->sTarget)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_remove_self")));
    }
}


/*====================
  CChatManager::ProcessClanUpdateSuccess
  ====================*/
void    CChatManager::ProcessClanUpdateSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        m_setClanList.clear();

        const CPHPData *pClan(phpResponse.GetVar(_T("clan_roster")));
        if (pClan != NULL && pClan->GetVar(_T("error")) == NULL)
        {
            uint uiNum(0);
            const CPHPData *pClanItem(pClan->GetVar(uiNum++));

            while (pClanItem != NULL)
            {
                tstring sRank(pClanItem->GetString(_T("rank")));
                tstring sName(pClanItem->GetString(_T("nickname")));
                int iAccountID(pClanItem->GetInteger(_T("account_id")));

                if (!pRequest->sText.empty())
                    sName = _T("[") + pRequest->sText + _T("]") + sName;
                
                byte yFlags(0);

                if (CompareNoCase((sRank), _T("Leader")) == 0)
                    yFlags |= CHAT_CLIENT_IS_CLAN_LEADER;
                else if (CompareNoCase((sRank), _T("Officer")) == 0)
                    yFlags |= CHAT_CLIENT_IS_OFFICER;

                AddClanMember(iAccountID, sName, yFlags);
                pClanItem = pClan->GetVar(uiNum++);
            }
        }
    }
}


/*====================
  CChatManager::ProcessClanNameCheckSuccess
  ====================*/
void    CChatManager::ProcessClanNameCheckSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    tsmapts mapParams;
    mapParams[_T("name")] = pRequest->sTarget;
    mapParams[_T("tag")] = pRequest->sText;

    if (CompareNoCase(phpResponse.GetString(_T("clan_check")), _T("OK")) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_clan_name_tip"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_success_clan_name_tip"), mapParams));
    }
    else if (CompareNoCase(phpResponse.GetString(_T("clan_name")), _T("Clan name already taken.")) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_clan_name_used"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_invalid_clan_name_used_tip"), mapParams));
    }
    else if (CompareNoCase(phpResponse.GetString(_T("clan_tag")), _T("Clan tag already taken.")) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_clan_tag_used"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_invalid_clan_tag_used_tip"), mapParams));
    }
    else if (CompareNoCase(phpResponse.GetString(_T("clan_name")), _T("Invalid clan name format. 1-32 alphanum or spaces chars.")) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_clan_name_format"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_invalid_clan_name_format_tip"), mapParams));
    }
    else if (CompareNoCase(phpResponse.GetString(_T("clan_tag")), _T("Invalid clan tag format. 1-4 alphanum chars.")) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_clan_tag_format"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_invalid_clan_tag_format_tip"), mapParams));
    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_clan_name"), mapParams));
        ChatClanCreateTip.Trigger(Translate(_T("chat_invalid_clan_name_tip"), mapParams));
    }
}


/*====================
  CChatManager::ProcessBanLookupIDSuccess
  ====================*/
void    CChatManager::ProcessBanLookupIDSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    const CPHPData *pAccountID(phpResponse.GetVar(1));
    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL && pAccountID != NULL)
    {
        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("new_banned"));
        pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
        pHTTPRequest->AddVariable(_T("banned_id"), pAccountID->GetInteger());
        pHTTPRequest->AddVariable(_T("reason"), pRequest->sText);
        pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_ADD_BANNED, pAccountID->GetInteger(), pRequest->sText));
        pNewRequest->sTarget = pRequest->sTarget;

        m_lHTTPRequests.push_back(pNewRequest);
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add"), _T("target"), pRequest->sTarget));
    }
}


/*====================
  CChatManager::ProcessAddBanSuccess
  ====================*/
void    CChatManager::ProcessAddBanSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        AddBan(pRequest->uiTarget, pRequest->sTarget, pRequest->sText);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_banlist_add"), _T("target"), pRequest->sTarget));
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
    }
}


/*====================
  CChatManager::ProcessRemoveBanSuccess
  ====================*/
void    CChatManager::ProcessRemoveBanSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        //Update our banlist...
        RemoveBan(pRequest->uiTarget);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_banlist_remove"), _T("target"), pRequest->sTarget));
    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_remove"), _T("target"), pRequest->sTarget));
    }
}


/*====================
  CChatManager::ProcessIgnoreLookupIDSuccess
  ====================*/
void    CChatManager::ProcessIgnoreLookupIDSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    const CPHPData *pAccountID(phpResponse.GetVar(1));
    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL && pAccountID != NULL)
    {
        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("new_ignored"));
        pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
        pHTTPRequest->AddVariable(_T("ignored_id"), pAccountID->GetInteger());
        pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_ADD_IGNORED, pAccountID->GetInteger()));
        pNewRequest->sTarget = pRequest->sTarget;

        m_lHTTPRequests.push_back(pNewRequest);
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add"), _T("target"), pRequest->sTarget));
    }

}


/*====================
  CChatManager::ProcessIgnoreAddSuccess
  ====================*/
void    CChatManager::ProcessIgnoreAddSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        AddIgnore(pRequest->uiTarget, pRequest->sTarget);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_ignore_add"), _T("target"), pRequest->sTarget));
    }
    else
    {
        if (!phpResponse.GetString(_T("error")).empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetString(_T("error"))));
        else if (phpResponse.GetVar(_T("error")) != NULL)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_reason"), _T("target"), pRequest->sTarget, _T("reason"), phpResponse.GetVar(_T("error"))->GetString(0)));
    }
}


/*====================
  CChatManager::ProcessIgnoreRemoveSuccess
  ====================*/
void    CChatManager::ProcessIgnoreRemoveSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (phpResponse.GetString(_T("error")).empty() && phpResponse.GetVar(_T("error")) == NULL)
    {
        // Update our ignore list...
        RemoveIgnore(pRequest->uiTarget);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_success_ignore_remove"), _T("target"), pRequest->sTarget));
    }
    else
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_remove"), _T("target"), pRequest->sTarget));
    }
}


/*====================
  CChatManager::ProcessCompleteNickSuccess
  ====================*/
void    CChatManager::ProcessCompleteNickSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    const CPHPData *pArray(phpResponse.GetVar(_T("nicks")));
    if (pArray == NULL)
        return;

    uint uiPos(0);
    const CPHPData *pName(pArray->GetVar(uiPos));

    while (pName != NULL)
    {
        ChatAutoCompleteAdd.Trigger(pName->GetString());
        
        ++uiPos;
        pName = pArray->GetVar(uiPos);
    }
}


/*====================
  CChatManager::ProcessSaveChannelSuccess
  ====================*/
void    CChatManager::ProcessSaveChannelSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("add_room")), _T("OK")) != 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_saving_channel"), _T("channel"), pRequest->sTarget));
    else
        SaveChannelLocal(pRequest->sTarget);
}


/*====================
  CChatManager::ProcessRemoveChannelSuccess
  ====================*/
void    CChatManager::ProcessRemoveChannelSuccess(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());

    if (CompareNoCase(phpResponse.GetString(_T("remove_room")), _T("OK")) != 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_removing_channel"), _T("channel"), pRequest->sTarget));
    else
        RemoveChannelLocal(pRequest->sTarget);  
}


/*====================
  CChatManager::ProcessSaveNotificationResponse
  ====================*/
void    CChatManager::ProcessSaveNotificationResponse(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());
    
    if (CompareNoCase(phpResponse.GetString(_T("status")), _T("OK")) != 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_saving_notification")));
    else
    {       
        const tstring sNotification(phpResponse.GetString(_T("notification")));
        const uint uiExternalNotificationID(phpResponse.GetInteger(_T("notify_id")));
        ParseNotification(sNotification, uiExternalNotificationID);         
    }
}


/*====================
  CChatManager::ProcessRemoveNotificationResponse
  ====================*/
void    CChatManager::ProcessRemoveNotificationResponse(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());
    
    const uint uiExternalNotificationID(phpResponse.GetInteger(_T("notify_id")));   

    if (CompareNoCase(phpResponse.GetString(_T("status")), _T("OK")) != 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_removing_notification"), _T("id"), XtoA(uiExternalNotificationID)));
    else
    {
        const uint uiInternalNotificationID(phpResponse.GetInteger(_T("internal_id")));             
        RemoveExternalNotification(uiInternalNotificationID);
    }       
}


/*====================
  CChatManager::ProcessRemoveAllNotificationsResponse
  ====================*/
void    CChatManager::ProcessRemoveAllNotificationsResponse(SChatDBRequest *pRequest)
{
    const CPHPData phpResponse(pRequest->pRequest->GetResponse());
    
    if (CompareNoCase(phpResponse.GetString(_T("status")), _T("OK")) != 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_removing_notifications")));
    else
        ClearNotifications();
}


/*====================
  CChatManager::DatabaseFrame
  ====================*/
void    CChatManager::DatabaseFrame()
{
    ChatRequestList_it itRequest(m_lHTTPRequests.begin());
    ChatRequestList_it itEnd(m_lHTTPRequests.end());
    while (itRequest != itEnd)
    {
        SChatDBRequest *pRequest(*itRequest);

        switch (pRequest->pRequest->GetStatus())
        {
        case HTTP_REQUEST_SENDING:
            ++itRequest;
            break;

        default:
        case HTTP_REQUEST_IDLE:
        case HTTP_REQUEST_ERROR:
            ProcessFailedRequest(pRequest);
            
            m_pHTTPManager->ReleaseRequest(pRequest->pRequest);
            K2_DELETE(pRequest);
            STL_ERASE(m_lHTTPRequests, itRequest);
            itEnd = m_lHTTPRequests.end();
            break;

        case HTTP_REQUEST_SUCCESS:
            switch (pRequest->eType)
            {
            case REQUEST_ADD_BUDDY_NICK2ID:     ProcessAddBuddyLookupIDSuccess(pRequest); break;
            case REQUEST_ADD_BUDDY:             ProcessAddBuddySuccess(pRequest); break;
            case REQUEST_DELETE_BUDDY:          ProcessRemoveBuddySuccess(pRequest); break;
            case REQUEST_CLAN_PROMOTE:          ProcessClanPromoteSuccess(pRequest); break;
            case REQUEST_CLAN_DEMOTE:           ProcessClanDemoteSuccess(pRequest); break;
            case REQUEST_CLAN_REMOVE:           ProcessClanRemoveSuccess(pRequest); break;
            case REQUEST_ADD_BANNED_NICK2ID:    ProcessBanLookupIDSuccess(pRequest); break;
            case REQUEST_ADD_BANNED:            ProcessAddBanSuccess(pRequest); break;
            case REQUEST_REMOVE_BANNED:         ProcessRemoveBanSuccess(pRequest); break;
            case REQUEST_GET_BANNED:            break;
            case REQUEST_ADD_IGNORED_NICK2ID:   ProcessIgnoreLookupIDSuccess(pRequest); break;
            case REQUEST_ADD_IGNORED:           ProcessIgnoreAddSuccess(pRequest); break;
            case REQUEST_REMOVE_IGNORED:        ProcessIgnoreRemoveSuccess(pRequest); break;
            case REQUEST_UPDATE_CLAN:           ProcessClanUpdateSuccess(pRequest); break;
            case REQUEST_CHECK_CLAN_NAME:       ProcessClanNameCheckSuccess(pRequest); break;
            case REQUEST_COMPLETE_NICK:         ProcessCompleteNickSuccess(pRequest); break;
            case REQUEST_SAVE_CHANNEL:          ProcessSaveChannelSuccess(pRequest); break;
            case REQUEST_REMOVE_CHANNEL:        ProcessRemoveChannelSuccess(pRequest); break;
            case REQUEST_SAVE_NOTIFICATION:     ProcessSaveNotificationResponse(pRequest); break;
            case REQUEST_REMOVE_NOTIFICATION:   ProcessRemoveNotificationResponse(pRequest); break;
            case REQUEST_REMOVE_ALL_NOTIFICATIONS:  ProcessRemoveAllNotificationsResponse(pRequest); break;
            }

            m_pHTTPManager->ReleaseRequest(pRequest->pRequest);
            K2_DELETE(pRequest);
            STL_ERASE(m_lHTTPRequests, itRequest);
            itEnd = m_lHTTPRequests.end();
            break;
        }
    }
}


/*====================
  CChatManager::Disconnect
  ====================*/
void    CChatManager::Disconnect()
{
    m_uiConnectRetries = -1;
    m_uiNextReconnectTime = INVALID_TIME;

    m_setChannelsIn.clear();
    ChatLeftChannel.Trigger(_T("-1"));
    
    // These need to be done on logout and on login, to make sure the UI stays in sync with the various routes they can 
    // disconnect or logout by (manual disconnect, net drop, chat connect retries exceeded, etc)
    ClearNotifications();

    HandleDisconnect();
}


/*====================
  CChatManager::ConnectingFrame
  ====================*/
void    CChatManager::ConnectingFrame()
{
    if (!m_sockChat.IsConnected())
    {
        if (Host.GetTimeSeconds() > m_uiConnectTimeout)
            HandleDisconnect();

        return;
    }

    CPacket pktSend;
    pktSend << NET_CHAT_CL_CONNECT << m_uiAccountID << m_sCookie << Host.GetGarenaToken() << CHAT_PROTOCOL_VERSION << GetChatModeType();
    
    m_sockChat.SendPacket(pktSend);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_waiting_verification")));

    m_eStatus = CHAT_STATUS_WAITING_FOR_AUTH;
}


/*====================
  CChatManager::AuthFrame
  ====================*/
void    CChatManager::AuthFrame()
{
    if (m_sockChat.DataWaiting())
    {
        CPacket pktRecv;

        m_sockChat.ReceivePacket(pktRecv);

        if (pktRecv.GetLength() < 1)
        {
            // Socket closed or socket error
            HandleDisconnect();
            return;
        }

        ProcessData(pktRecv);
    }

    if (!m_sockChat.IsConnected())
    {
        // Connection dropped
        HandleDisconnect();
        return;
    }
}


/*====================
  CChatManager::ConnectedFrame
  ====================*/
void    CChatManager::ConnectedFrame()
{
    if (!m_sockChat.IsConnected())
    {
        // Connection dropped
        HandleDisconnect();
        return;
    }

    /*
    // Looking for clan stuff doesn't work properly or is not intended to be part of HoN
    if (cc_lookingForClan.IsModified() && ClientLogin.GetClan().empty())
    {
        cc_lookingForClan.SetModified(false);

        CPacket pktSend;

        if (cc_lookingForClan)
            pktSend << CHAT_CMD_LOOKING_FOR_CLAN;
        else
            pktSend << CHAT_CMD_NOT_LOOKING_FOR_CLAN;

        m_sockChat.SendPacket(pktSend);
    }
    */

    while (m_sockChat.DataWaiting())
    {
        CPacket pktRecv;
        int iRecvLength;

        iRecvLength = m_sockChat.ReceivePacket(pktRecv);

        if (iRecvLength < 1)
        {
            // Socket error or socket closed, drop the connection...
            HandleDisconnect();
            return;
        }

        ProcessData(pktRecv);
    }

    // Update clan creation timer
    if (m_uiCreateTimeSent != INVALID_TIME)
    {
        if (m_uiCreateTimeSent + 120000 < K2System.Milliseconds())
        {
            m_uiCreateTimeSent = INVALID_TIME;
            ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_time")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_time")));
        }
        else
        {
            ChatClanCreateTime.Trigger(XtoA((m_uiCreateTimeSent + 120000) - K2System.Milliseconds()));
        }
    }
}


/*====================
  CChatManager::ProcessData
  ====================*/
bool    CChatManager::ProcessData(CPacket &pkt)
{
    m_uiLastRecvTime = Host.GetTime();

    ushort unPrevCmd(NET_CHAT_INVALID);

    while (!pkt.DoneReading())
    {
        ushort unCmd(pkt.ReadShort(NET_CHAT_INVALID));

        //Console << _T("Chat server: ") << SHORT_HEX_STR(unCmd) << newl;
        
        switch (unCmd)
        {
        case NET_CHAT_CL_ACCEPT:    HandleAuthAccepted(); break;
        case NET_CHAT_CL_REJECT:    HandleRejected(pkt); break;

        case NET_CHAT_PING:
            HandlePing();
            break;
            
        case NET_CHAT_CL_CHANNEL_INFO:
            HandleChannelInfo(pkt);
            break;

        case NET_CHAT_CL_CHANNEL_LIST_SYN:
            {
                CPacket pktSend;
                pktSend << NET_CHAT_CL_CHANNEL_LIST_ACK;
                m_sockChat.SendPacket(pktSend);
                ChatChannelList.Execute(_T("SortByCol(0);"));
            }
            break;

        case NET_CHAT_CL_CHANNEL_INFO_SUB:
            HandleChannelInfoSub(pkt);
            break;

        case NET_CHAT_CL_CHANNEL_SUBLIST_SYN:
            {
                byte ySequence(pkt.ReadByte());

                CPacket pktSend;
                pktSend << NET_CHAT_CL_CHANNEL_SUBLIST_ACK << ySequence;
                m_sockChat.SendPacket(pktSend);
            }
            break;

        case NET_CHAT_CL_CHANNEL_SUBLIST_START:
            {
                byte ySequence(pkt.ReadByte());
                tstring sHead(pkt.ReadWStringAsTString());
                
                //Console << _T("Start Channel sublist: ") << ySequence << newl;

                if (ySequence == m_yListStartSequence)
                {
                    AutoCompleteClear();
                    m_mapChannelList.clear();
                    m_bFinishedList = false;

                    m_sProcessingListHead = sHead;
                    m_yProcessingListSequence = ySequence;
                }
            }
            break;

        case NET_CHAT_CL_CHANNEL_SUBLIST_END:
            {
                byte ySequence(pkt.ReadByte());

                //Console << _T("End Channel sublist: ") << ySequence << newl;

                if (ySequence == m_yProcessingListSequence)
                {
                    m_yFinishedListSequence = m_yProcessingListSequence;
                    m_sFinishedListHead = m_sProcessingListHead;
                    m_bFinishedList = true;

                    m_yProcessingListSequence = 0xff;
                }
            }
            break;

        case NET_CHAT_CL_USER_STATUS:
            HandleUserStatus(pkt);
            break;

        case CHAT_CMD_CHANGED_CHANNEL:
            HandleChannelChange(pkt);
            break;

        case CHAT_CMD_JOINED_CHANNEL:
            HandleChannelJoin(pkt);
            break;

        case CHAT_CMD_CHANNEL_MSG:
            HandleChannelMessage(pkt);
            break;

        case CHAT_CMD_LEFT_CHANNEL:
            HandleChannelLeave(pkt);
            break;

        case CHAT_CMD_WHISPER:
            HandleWhisper(pkt);
            break;

        case CHAT_CMD_WHISPER_BUDDIES:
            HandleWhisperBuddies(pkt);
            break;

        case CHAT_CMD_WHISPER_FAILED:
            HandleWhisperFailed();
            break;

        case CHAT_CMD_DISCONNECTED:
            HandleDisconnect();
            break;

        case CHAT_CMD_INITIAL_STATUS:
            HandleInitialStatusUpdate(pkt);
            break;

        case CHAT_CMD_UPDATE_STATUS:
            HandleStatusUpdate(pkt);
            break;

        case CHAT_CMD_CLAN_WHISPER:
            HandleClanWhisper(pkt);
            break;

        case CHAT_CMD_CLAN_WHISPER_FAILED:
            HandleClanWhisperFailed();
            break;

        case CHAT_CMD_LOOKING_FOR_CLAN:
            HandleLookingForClan(pkt);
            break;

        case CHAT_CMD_NOT_LOOKING_FOR_CLAN:
            HandleNotLookingForClan(pkt);
            break;

        case CHAT_CMD_MULT_LOOKING_FOR_CLAN:
            HandleMultipleLookingForClan(pkt);
            break;

        case CHAT_CMD_FLOODING:
            HandleFlooding();
            break;

        case CHAT_CMD_IM:
            HandleIM(pkt);
            break;

        case CHAT_CMD_IM_FAILED:
            HandleIMFailed(pkt);
            break;

        case CHAT_CMD_MAX_CHANNELS:
            HandleMaxChannels();
            break;

        case CHAT_CMD_INVITED_TO_SERVER:
            HandleServerInvite(pkt);
            break;

        case CHAT_CMD_INVITE_FAILED_USER:
            HandleInviteFailedUserNotFound();
            break;

        case CHAT_CMD_INVITE_FAILED_GAME:
            HandleInviteFailedNotInGame();
            break;

        case CHAT_CMD_INVITE_REJECTED:
            HandleInviteRejected(pkt);
            break;

        case CHAT_CMD_USER_INFO_NO_EXIST:
            HandleUserInfoNoExist(pkt);
            break;

        case CHAT_CMD_USER_INFO_OFFLINE:
            HandleUserInfoOffline(pkt);
            break;

        case CHAT_CMD_USER_INFO_ONLINE:
            HandleUserInfoOnline(pkt);
            break;

        case CHAT_CMD_USER_INFO_IN_GAME:
            HandleUserInfoInGame(pkt);
            break;

        case CHAT_CMD_CHANNEL_UPDATE:
            HandleChannelUpdate(pkt);
            break;

        case CHAT_CMD_CHANNEL_TOPIC:
            HandleChannelTopic(pkt);
            break;

        case CHAT_CMD_CHANNEL_KICK:
            HandleChannelKick(pkt);
            break;

        case CHAT_CMD_CHANNEL_BAN:
            HandleChannelBan(pkt);
            break;

        case CHAT_CMD_CHANNEL_UNBAN:
            HandleChannelUnban(pkt);
            break;

        case CHAT_CMD_CHANNEL_IS_BANNED:
            HandleBannedFromChannel(pkt);
            break;

        case CHAT_CMD_CHANNEL_SILENCED:
            HandleChannelSilenced(pkt);
            break;

        case CHAT_CMD_CHANNEL_SILENCE_LIFTED:
            HandleChannelSilenceLifted(pkt);
            break;

        case CHAT_CMD_CHANNEL_SILENCE_PLACED:
            HandleSilencePlaced(pkt);
            break;

        case CHAT_CMD_CHANNEL_PROMOTE:
            HandleChannelPromote(pkt);
            break;

        case CHAT_CMD_CHANNEL_DEMOTE:
            HandleChannelDemote(pkt);
            break;

        case CHAT_CMD_MESSAGE_ALL:
            HandleMessageAll(pkt);
            break;

        case CHAT_CMD_CHANNEL_SET_AUTH:
            HandleChannelAuthEnabled(pkt);
            break;

        case CHAT_CMD_CHANNEL_REMOVE_AUTH:
            HandleChannelAuthDisabled(pkt);
            break;

        case CHAT_CMD_CHANNEL_ADD_AUTH_USER:
            HandleChannelAddAuthUser(pkt);
            break;

        case CHAT_CMD_CHANNEL_REM_AUTH_USER:
            HandleChannelRemoveAuthUser(pkt);
            break;

        case CHAT_CMD_CHANNEL_ADD_AUTH_FAIL:
            HandleChannelAddAuthUserFailed(pkt);
            break;

        case CHAT_CMD_CHANNEL_REM_AUTH_FAIL:
            HandleChannelRemoveAuthUserFailed(pkt);
            break;

        case CHAT_CMD_CHANNEL_LIST_AUTH:
            HandleChannelListAuth(pkt);
            break;

        case CHAT_CMD_CHANNEL_SET_PASSWORD:
            HandleChannelSetPassword(pkt);
            break;

        case CHAT_CMD_JOIN_CHANNEL_PASSWORD:
            HandleChannelJoinPassword(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_MEMBER:
            HandleClanInvite(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_REJECTED:
            HandleClanInviteRejected(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_FAIL_ONLINE:
            HandleClanInviteFailedOnline(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_FAIL_CLAN:
            HandleClanInviteFailedClan(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_FAIL_INVITED:
            HandleClanInviteFailedInvite(pkt);
            break;
            
        case CHAT_CMD_CLAN_ADD_FAIL_PERMS:
            HandleClanInviteFailedPermissions(pkt);
            break;

        case CHAT_CMD_CLAN_ADD_FAIL_UNKNOWN:
            HandleClanInviteFailedUnknown(pkt);
            break;

        case CHAT_CMD_NEW_CLAN_MEMBER:
            HandleNewClanMember(pkt);
            break;

        case CHAT_CMD_CLAN_RANK_CHANGE:
            HandleClanRankChanged(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_ACCEPT:
            HandleClanCreateAccept(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_REJECT:
            HandleClanCreateRejected(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_COMPLETE:
            HandleClanCreateComplete(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_CLAN:
            HandleClanCreateFailedClan(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_INVITE:
            HandleClanCreateFailedInvite(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_FIND:
            HandleClanCreateFailedNotFound(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_DUPE:
            HandleClanCreateFailedDuplicate(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_PARAM:
            HandleClanCreateFailedParam(pkt);
            break;

        case CHAT_CMD_NAME_CHANGE:
            HandleNameChange(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_NAME:
            HandleClanCreateFailedClanName(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_TAG:
            HandleClanCreateFailedTag(pkt);
            break;

        case CHAT_CMD_CLAN_CREATE_FAIL_UNKNOWN:
            HandleClanCreateFailedUnknown(pkt);
            break;

        case CHAT_CMD_AUTO_MATCH_CONNECT:
            HandleAutoMatchConnect(pkt);
            break;

        case CHAT_CMD_SERVER_NOT_IDLE:
            HandleServerNotIdle(pkt);
            break;

        case CHAT_CMD_TOURN_MATCH_READY:
            HandleTournMatchReady(pkt);
            break;

        case CHAT_CMD_AUTO_MATCH_WAITING:
            HandleAutoMatchWaiting(pkt);

        case CHAT_CMD_CHAT_ROLL:
            HandleChatRoll(pkt);
            break;

        case CHAT_CMD_CHAT_EMOTE:
            HandleChatEmote(pkt);
            break;
            
        case CHAT_CMD_SET_CHAT_MODE_TYPE:
            HandleSetChatModeType(pkt);
            break;
            
        case CHAT_CMD_CHAT_MODE_AUTO_RESPONSE:
            HandleChatModeAutoResponse(pkt);
            break;

        case CHAT_CMD_PLAYER_COUNT:
            HandleUserCount(pkt);
            break;
            
        case CHAT_CMD_REQUEST_BUDDY_ADD_RESPONSE:
            HandleRequestBuddyAddResponse(pkt);
            break;          
            
        case CHAT_CMD_REQUEST_BUDDY_APPROVE_RESPONSE:
            HandleRequestBuddyApproveResponse(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_INVITE:
            HandleTMMInviteToGroup(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_INVITE_BROADCAST:
            HandleTMMInviteToGroupBroadcast(pkt);
            break;
            
        case NET_CHAT_CL_TMM_GROUP_REJECT_INVITE:
            HandleTMMRejectInvite(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_JOIN_QUEUE:
            HandleTMMJoinQueue(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_LEAVE_QUEUE:
            HandleTMMLeaveQueue(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_UPDATE:
            HandleTMMPlayerUpdates(pkt);
            break;

        case NET_CHAT_CL_TMM_POPULARITY_UPDATE:
            HandleTMMPopularityUpdates(pkt);
            break;

        case NET_CHAT_CL_TMM_GROUP_QUEUE_UPDATE:
            HandleTMMQueueUpdates(pkt);
            break;

        case NET_CHAT_CL_TMM_MATCH_FOUND_UPDATE:
            HandleTMMMatchFound(pkt);
            break;

        case NET_CHAT_CL_TMM_FAILED_TO_JOIN:
            HandleTMMJoinFailed(pkt);
            break;

        case CHAT_CMD_REQUEST_GAME_INFO:
            HandleRequestGameInfo(pkt);
            break;
            
        default:
        case NET_CHAT_INVALID:
            Console << _T("Invalid command from chat server: ") << SHORT_HEX_STR(unCmd) << newl;

            if (unPrevCmd != NET_CHAT_INVALID)
                Console << _T("Last valid cmd: ") << SHORT_HEX_STR(unPrevCmd) << newl;
            
            HandleDisconnect();
            return false;
        }

        unPrevCmd = unCmd;

        if (pkt.HasFaults())
        {
            Console << _T("Bad packet from chat server") << newl;
            HandleDisconnect();
            return false;
        }
    }

    return true;
}


/*====================
  CChatManager::ClearNotifications
  ====================*/
void    CChatManager::ClearNotifications()
{ 
    ChatNotificationHistoryPerformCMD.Trigger(_T("ClearItems();")); 
    m_mapNotifications.clear(); 
    ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
}


/*====================
  CChatManager::PushNotification
  ====================*/
void    CChatManager::PushNotification(const byte yType, const tstring &sParam1, const tstring &sParam2, const tstring &sParam3, const tsvector &vParam4, const uint uiExternalNotificationID, const bool bSilent, const tstring &sNotificationTime)
{
    if (yType >= NUM_NOTIFICATIONS)
        return;

    // 10 initial params + 19 for the game invite params + 1 for 0 based, + 1 for pushing params to interface silently
    tsvector vParams(31);
    m_cDate = CDate(true);

    vParams[0] = sParam1;                               // sParam1, sParam2, sParam3 are used different ways for each notification
    vParams[1] = sParam2;                   
    vParams[2] = XtoA(yType);                           // type (ENotifyType)
    vParams[3] = g_sNotifyText[yType];                  // stringtable text entry (notify_unknown)
    vParams[4] = g_sNotifyType[yType];                  // generic notify type (notfication_generic_info)
    vParams[5] = g_sNotifyAction[yType];                // specific action template within the notifytype
    
    // the time needs to be overriden because this notification is stored in the DB and we want that time
    if (sNotificationTime.empty())
        vParams[6] = XtoA(m_cDate.GetMonth(), FMT_PADZERO, 2) + _T("/") + XtoA(m_cDate.GetDay(), FMT_PADZERO, 2) + _T("  ") + m_cDate.GetTimeString(TIME_NO_SECONDS | TIME_TWELVE_HOUR);
    else
        vParams[6] = sNotificationTime;
        
    vParams[7] = XtoA(uiExternalNotificationID);        // external notification ID of notification in DB
    vParams[8] = XtoA(IncrementNotificationIndex());    // internal notification ID starts at 0 on signon and increases for each additional notification
    vParams[9] = sParam3;
    vParams[30] = XtoA(bSilent);                        // show this notification when logging in or keep it silent so it populates the notification history

    switch (yType)
    {
        case NOTIFY_TYPE_UNKNOWN:
            break;  
        case NOTIFY_TYPE_BUDDY_ADDER:
        {
            ChatNotificationBuddy.Trigger(vParams);
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));            
            if (!bSilent)
                PlaySound(_T("NotifyBuddyAdded"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_ADDED:
        {
            ChatNotificationBuddy.Trigger(vParams);
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            if (!bSilent)
                PlaySound(_T("NotifyAddedAsBuddy"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_REMOVER:
        {
            //ChatNotificationBuddy.Trigger(vParams);
            //AddNotification(GetNotificationIndex(), vParams);
            //ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            PlaySound(_T("NotifyBuddyRemoved"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_REMOVED:
        {
            //ChatNotificationBuddy.Trigger(vParams);
            //AddNotification(GetNotificationIndex(), vParams);
            //ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            PlaySound(_T("NotifyRemovedAsBuddy"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_ONLINE:
        {
            ChatNotificationBuddy.Trigger(vParams);
            PlaySound(_T("NotifyBuddyOnline"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_LEFT_GAME:
        {
            ChatNotificationBuddy.Trigger(vParams);
            PlaySound(_T("NotifyBuddyLeftGame"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_OFFLINE:
        {
            ChatNotificationBuddy.Trigger(vParams);
            PlaySound(_T("NotifyBuddyOffline"));
            break;
        }
        case NOTIFY_TYPE_CLAN_RANK:
        {
            ChatNotificationClan.Trigger(vParams);
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            PlaySound(_T("NotifyClanRank"));    
            break;
        }
        case NOTIFY_TYPE_CLAN_ADD:
        {
            ChatNotificationClan.Trigger(vParams);
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            PlaySound(_T("NotifyClanAdd"));     
            break;
        }
        case NOTIFY_TYPE_CLAN_REMOVE:
        {
            ChatNotificationClan.Trigger(vParams);
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));            
            PlaySound(_T("NotifyClanRemove"));
            break;
        }
        case NOTIFY_TYPE_CLAN_ONLINE:
        {
            ChatNotificationClan.Trigger(vParams);
            PlaySound(_T("NotifyClanOnline"));
            break;
        }
        case NOTIFY_TYPE_CLAN_LEFT_GAME:
        {
            ChatNotificationClan.Trigger(vParams);
            PlaySound(_T("NotifyClanLeftGame"));
            break;
        }
        case NOTIFY_TYPE_CLAN_OFFLINE:
        {
            ChatNotificationClan.Trigger(vParams);
            PlaySound(_T("NotifyClanOffline"));
            break;
        }
        case NOTIFY_TYPE_CLAN_WHISPER:
            break;
        case NOTIFY_TYPE_UPDATE:
        {
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));        
            ChatNotificationMessage.Trigger(vParams);
            break;
        }
        case NOTIFY_TYPE_GENERIC:
            break;
        case NOTIFY_TYPE_IM:
        {
            ChatReceivedIMCount.Trigger(XtoA(AddReceivedIM()));         
            AddUnreadIM(sParam1);           
            ChatUnreadIMCount.Trigger(XtoA(GetUnreadIMCount()));
            ChatOpenIMCount.Trigger(XtoA(GetOpenIMCount()));
            PlaySound(_T("RecievedIM"));
            
            // throttle IM notifications from popping up on every IM, to every 2 minutes
            if (m_uiLastIMNotificationTime < K2System.Milliseconds())
            {
                m_uiLastIMNotificationTime = K2System.Milliseconds() + 120000;              
                ChatNotificationBuddy.Trigger(vParams);
            }
            break;
        }
        case NOTIFY_TYPE_BUDDY_JOIN_GAME:
        case NOTIFY_TYPE_CLAN_JOIN_GAME:
        case NOTIFY_TYPE_GAME_INVITE:       
        case NOTIFY_TYPE_SELF_JOIN_GAME:
        {
            if (!vParam4.empty())
            {       
                vParams[10] = vParam4[0];                                   // Address:Port
                vParams[11] = StringReplace(vParam4[1], _T("'"), _T("`"));  // Game Name - Replace ' with ` to avoid UI errors
                vParams[12] = vParam4[2];                                   // Buddy Name/Clan Member Name/Inviter Name/Self
                vParams[13] = vParam4[3];                                   // Server Region
                vParams[14] = vParam4[4];                                   // Game Mode
                vParams[15] = vParam4[5];                                   // Team Size            
                vParams[16] = vParam4[6];                                   // Map Name
                vParams[17] = vParam4[7];                                   // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
                vParams[18] = vParam4[8];                                   // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
                vParams[19] = vParam4[9];                                   // No Leavers (1), Leavers (0)
                vParams[20] = vParam4[10];                                  // Private (1), Not Private (0)                                 
                vParams[21] = _T("0");                                      // All Heroes (1), Not All Heroes (0) -- (NOTE: Deprecated)
                vParams[22] = vParam4[12];                                  // Casual Mode (1), Not Casual Mode (0)
                vParams[23] = vParam4[13];                                  // Force Random (1), Not Force Random (0)
                vParams[24] = vParam4[14];                                  // Auto Balanced (1), Non Auto Balanced (0)
                vParams[25] = vParam4[15];                                  // Advanced Options (1), No Advanced Options (0)
                vParams[26] = vParam4[16];                                  // Min PSR
                vParams[27] = vParam4[17];                                  // Max PSR
                vParams[28] = vParam4[18];                                  // Dev Heroes (1), Non Dev Heroes (0)
                vParams[29] = vParam4[19];                                  // Hardcore (1), Non Hardcore (0)

                AddNotification(GetNotificationIndex(), vParams);
                ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
                
                switch (yType)
                {
                    case NOTIFY_TYPE_BUDDY_JOIN_GAME:
                        ChatNotificationBuddy.Trigger(vParams);
                        PlaySound(_T("NotifyBuddyJoinGame"));
                        break;
                    case NOTIFY_TYPE_CLAN_JOIN_GAME:
                        ChatNotificationClan.Trigger(vParams);
                        PlaySound(_T("NotifyClanJoinGame"));                        
                        break;
                    case NOTIFY_TYPE_GAME_INVITE:
                        ChatNotificationInvite.Trigger(vParams);
                        PlaySound(_T("NotifyGameInvite"));
                        break;
                    case NOTIFY_TYPE_SELF_JOIN_GAME:                
                        ChatNotificationInvite.Trigger(vParams);
                        PlaySound(_T("NotifySelfJoinGame"));                        
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case NOTIFY_TYPE_BUDDY_REQUESTED_ADDER:
        {
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            ChatNotificationBuddy.Trigger(vParams);
            if (!bSilent)           
                PlaySound(_T("NotifyBuddyRequester"));
            break;
        }
        case NOTIFY_TYPE_BUDDY_REQUESTED_ADDED:
        {
            AddNotification(GetNotificationIndex(), vParams);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
            ChatNotificationBuddy.Trigger(vParams);
            if (!bSilent)
                PlaySound(_T("NotifyBuddyRequested"));
            break;
        }
        case NOTIFY_TYPE_TMM_GROUP_INVITE:
        {
            if (!vParam4.empty())
            {       
                vParams[10] = vParam4[0];       // Map Names - caldavar, grimmscrossing, darkwoodvale, etc, can be multiple and pipe (|) delimited
                vParams[11] = vParam4[1];       // Game Type - 0 = Normal, 1 = Casual
                vParams[12] = vParam4[2];       // Game Modes - ap, sd, bd, bp, ar, pipe (|) delimited
                vParams[13] = vParam4[3];       // Regions - USE, USW, EU, pipe (|) delimited
                
                AddNotification(GetNotificationIndex(), vParams);
                ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
                ChatNotificationGroupInvite.Trigger(vParams);
                PlaySound(_T("NotifyGameInvite"));
            }
            break;      
        }
        default:
        {
            ChatNotificationMessage.Trigger(vParams);
            break;
        }
    }
}


/*====================
  CChatManager::RemoveNotification
  ====================*/
void    CChatManager::RemoveNotification(const uint uiIndex)
{
    NotificationMap_it it(m_mapNotifications.find(uiIndex));
    
    if (it != m_mapNotifications.end())
    {
        // this vParam size should match the vParam in ChatPushNotification
        tsvector vParams(31);
        
        vParams = it->second;
                
        // if the external ID isn't set...
        if (AtoI(vParams[7]) == 0)
        {
            // then remove the notification
            if (it != m_mapNotifications.end()) 
                STL_ERASE(m_mapNotifications, it);
                
            // update the notification count and tell the interface to remove this listitem
            const tstring sEraseNotification(_T("EraseListItemByValue('") + XtoA(uiIndex) + _T("');"));
            
            ChatNotificationHistoryPerformCMD.Trigger(sEraseNotification);
            ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
        }
        else
            RequestRemoveNotification(AtoI(vParams[8]), AtoI(vParams[7]));
    }
}


/*====================
  CChatManager::RemoveExternalNotification
  ====================*/
void    CChatManager::RemoveExternalNotification(const uint uiIndex)
{
    NotificationMap_it it(m_mapNotifications.find(uiIndex));
    if (it != m_mapNotifications.end()) 
        STL_ERASE(m_mapNotifications, it);
        
    // we tried to remove a notification, found it was an external notification stored in the DB,
    // got back the response, and now we are removing this and updating the interface
    ChatNotificationCount.Trigger(XtoA(GetNotificationCount()));
    
    const tstring sEraseNotification(_T("EraseListItemByValue('") + XtoA(uiIndex) + _T("');"));
    ChatNotificationHistoryPerformCMD.Trigger(sEraseNotification);
}


/*====================
  CChatManager::RequestSaveNotification (Depreciated, not being used but could be tweaked to save notifications inefficiently)
  ====================*/
void CChatManager::RequestSaveNotification(byte yType, const tstring &sParam1, const tstring &sParam2, const tstring &sParam3, const tsvector vParam4)
{
    if (yType >= NUM_NOTIFICATIONS)
        return;

    // 10 initial params + 19 for the game invite params + 1 for 0 based, this should match the vParam in ChatPushNotification - 1
    tsvector vParams(30);
    m_cDate = CDate(true);

    vParams[0] = sParam1;                               // sParam1, sParam2, sParam3 are used different ways for each notification
    vParams[1] = sParam2;                   
    vParams[2] = XtoA(yType);                           // type (ENotifyType)
    vParams[3] = g_sNotifyText[yType];                  // stringtable text entry (notify_unknown)
    vParams[4] = g_sNotifyType[yType];                  // generic notify type (notfication_generic_info)
    vParams[5] = g_sNotifyAction[yType];                // specific action template within the notifytype
    vParams[6] = XtoA(m_cDate.GetMonth(), FMT_PADZERO, 2) + _T("/") + XtoA(m_cDate.GetDay(), FMT_PADZERO, 2) + _T("  ") + m_cDate.GetTimeString(TIME_NO_SECONDS | TIME_TWELVE_HOUR);
    vParams[7] = _T("");                                // external notification ID of notification in DB
    vParams[8] = XtoA(IncrementNotificationIndex());    // internal notification ID starts at 0 on signon and increases for each additional notification
    vParams[9] = sParam3;
                        
    switch (yType)
    {
        case NOTIFY_TYPE_BUDDY_REQUESTED_ADDER:
        case NOTIFY_TYPE_BUDDY_REQUESTED_ADDED:
        case NOTIFY_TYPE_BUDDY_ADDER:
        case NOTIFY_TYPE_BUDDY_ADDED:
        case NOTIFY_TYPE_BUDDY_REMOVER:
        case NOTIFY_TYPE_BUDDY_REMOVED:
        case NOTIFY_TYPE_CLAN_RANK:
        case NOTIFY_TYPE_CLAN_ADD:
        case NOTIFY_TYPE_CLAN_REMOVE:
            break;
        case NOTIFY_TYPE_BUDDY_JOIN_GAME:
        case NOTIFY_TYPE_CLAN_JOIN_GAME:
        case NOTIFY_TYPE_GAME_INVITE:       
        case NOTIFY_TYPE_SELF_JOIN_GAME:
        {
            if (!vParam4.empty())
            {
                vParams[10] = vParam4[0];   // Address:Port
                vParams[11] = vParam4[1];   // Game Name
                vParams[12] = vParam4[2];   // Buddy Name/Clan Member Name/Inviter/Self
                vParams[13] = vParam4[3];   // Server Region
                vParams[14] = vParam4[4];   // Game Mode
                vParams[15] = vParam4[5];   // Team Size            
                vParams[16] = vParam4[6];   // Map Name
                vParams[17] = vParam4[7];   // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
                vParams[18] = vParam4[8];   // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
                vParams[19] = vParam4[9];   // No Leavers (1), Leavers (0)
                vParams[20] = vParam4[10];  // Private (1), Not Private (0)                                 
                vParams[21] = vParam4[11];  // All Heroes (1), Not All Heroes (0)
                vParams[22] = vParam4[12];  // Casual Mode (1), Not Casual Mode (0)
                vParams[23] = vParam4[13];  // Force Random (1), Not Force Random (0)
                vParams[24] = vParam4[14];  // Auto Balanced (1), Non Auto Balanced (0)
                vParams[25] = vParam4[15];  // Advanced Options (1), No Advanced Options (0)
                vParams[26] = vParam4[16];  // Min PSR
                vParams[27] = vParam4[17];  // Max PSR
                vParams[28] = vParam4[18];  // Dev Heroes (1), Non Dev Heroes (0)
                vParams[29] = vParam4[19];  // Hardcore (1), Non Hardcore (0)               
            }   
            break;      
        }
        default:
        {
            break;
        }
    }
    
    // escape the pipes with the escape sequence
    for (int i = 0; i < 30; i++)
        vParams[i] = StringReplace(vParams[i], SEARCH_PIPES, REPLACE_PIPES);
    
    tstring sNotification(ConcatinateArgs(vParams, _T("|")));
    
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;     

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("test_create_notification"));
    pHTTPRequest->AddVariable(_T("account_id[]"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
    pHTTPRequest->AddVariable(_T("type"), yType);
    pHTTPRequest->AddVariable(_T("params[notification]"), sNotification);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_SAVE_NOTIFICATION, TSNULL, TSNULL));
    m_lHTTPRequests.push_back(pNewRequest); 
}


/*====================
  CChatManager::RequestSaveNotification
  ====================*/
void CChatManager::RequestRemoveNotification(const uint uiInternalNotificationID, const uint uiExternalNotificationID)
{
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;     

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("delete_notification"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
    pHTTPRequest->AddVariable(_T("internal_id"), uiInternalNotificationID);
    pHTTPRequest->AddVariable(_T("notify_id"), uiExternalNotificationID);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_REMOVE_NOTIFICATION, TSNULL, TSNULL));
    m_lHTTPRequests.push_back(pNewRequest); 
}


/*====================
  CChatManager::RequestRemoveAllNotifications
  ====================*/
void CChatManager::RequestRemoveAllNotifications()
{
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;     

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("remove_all_notifications"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_REMOVE_ALL_NOTIFICATIONS, TSNULL, TSNULL));
    m_lHTTPRequests.push_back(pNewRequest); 
}


/*====================
  CChatManager::ParseNotification
  ====================*/
void CChatManager::ParseNotification(const tstring &sNotification, const uint uiExternalNotificationID, bool bSilent)
{
    // this is used to parse out notifications retrieved from the db that are delimited by pipes "|"
    // some values for vParams are generated on the fly, while others need to be read directly from the db response
    tstring sParam(TSNULL);
    tstring sParam1(TSNULL);
    tstring sParam2(TSNULL);
    tstring sParam3(TSNULL);
    tstring sNotificationTime(TSNULL);
    byte yType(0);
    static tsvector vParams(20);
    uint uiIndex(0);
    
    const tsvector vNotificationInfo(TokenizeString(sNotification, _T('|')));

    for (tsvector_cit it(vNotificationInfo.begin()), itEnd(vNotificationInfo.end()); it != itEnd; ++it)
    {
        // unescape the pipes in case there were any in the game name or some other field
        sParam = StringReplace(it->c_str(), REPLACE_PIPES, SEARCH_PIPES);
        
        switch(uiIndex)
        {
            case 0:
                sParam1 = sParam;
                break;
            case 1:
                sParam2 = sParam;
                break;
            case 2:
                yType = byte(AtoI(sParam));
                break;
            case 6:
                sNotificationTime = sParam;
                break;
            case 9:
                sParam3 = sParam;
                break;
            case 10:
                vParams[0] = sParam;
                break;
            case 11:
                vParams[1] = sParam;
                break;
            case 12:
                vParams[2] = sParam;
                break;
            case 13:
                vParams[3] = sParam;
                break;
            case 14:
                vParams[4] = sParam;
                break;
            case 15:
                vParams[5] = sParam;
                break;
            case 16:
                vParams[6] = sParam;
                break;
            case 17:
                vParams[7] = sParam;
                break;
            case 18:
                vParams[8] = sParam;
                break;
            case 19:
                vParams[9] = sParam;
                break;
            case 20:
                vParams[10] = sParam;
                break;
            case 21:
                vParams[11] = sParam;
                break;
            case 22:
                vParams[12] = sParam;
                break;
            case 23:
                vParams[13] = sParam;
                break;
            case 24:
                vParams[14] = sParam;
                break;
            case 25:
                vParams[15] = sParam;
                break;
            case 26:
                vParams[16] = sParam;
                break;
            case 27:
                vParams[17] = sParam;
                break;
            case 28:
                vParams[18] = sParam;
                break;
            case 29:
                vParams[19] = sParam;
                break;
            default:        
                break;
        }           
                    
        uiIndex++;              
    }
    
    if (yType == NOTIFY_TYPE_SELF_JOIN_GAME)
        bSilent = true;
                
    PushNotification(yType, sParam1, sParam2, sParam3, vParams, uiExternalNotificationID, bSilent, sNotificationTime);  
}


/*====================
  CChatManager::HandlePing
  ====================*/
void    CChatManager::HandlePing()
{
    if (!IsConnected())
        return;

    CPacket pktSend;
    pktSend << NET_CHAT_PONG;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::HandleDisconnect
  ====================*/
void    CChatManager::HandleDisconnect()
{
    if (m_uiAccountID == INVALID_ACCOUNT || m_eStatus == CHAT_STATUS_DISCONNECTED)
        return;

    m_sockChat.Close();
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_disconnected")));

    m_eStatus = CHAT_STATUS_DISCONNECTED;
    LeaveTMMGroup(true, _T("disconnected"));

    tsvector vParams(2);    
    vParams[0] = vParams[1] = TSNULL;
    ChatChanNumUsers.Trigger(vParams);
    
    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        it->second.yStatus = CHAT_STATUS_DISCONNECTED;
    }

    for (ChatChannelMap_it it(m_mapChannels.begin()); it != m_mapChannels.end(); it++)
    {
        if (it->second.uiFlags & CHAT_CHANNEL_FLAG_UNJOINABLE)
        {
            RemoveUnreadChannel(it->first);
            m_setChannelsIn.erase(it->first);           
            ChatLeftChannel.Trigger(XtoA(it->first));
        }
    }
                
    ChatUsersOnline.Trigger(_T("0"));

    vParams[0] = TSNULL;
    vParams[1] = _T("ClearItems();");
    ChatUserEvent.Trigger(vParams);

    if (m_uiConnectRetries < chat_maxReconnectAttempts)
    {
        ++m_uiConnectRetries;
        m_uiNextReconnectTime = K2System.Milliseconds() + M_Randnum(5000, 25000);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_reconnecting_delay"), _T("attempt"), XtoA(m_uiConnectRetries), _T("maxattempts"), XtoA(chat_maxReconnectAttempts)));
    }
}


/*====================
  CChatManager::HandleChannelChange
  ====================*/
void    CChatManager::HandleChannelChange(CPacket &pkt)
{
    tstring sChannel(pkt.ReadWStringAsTString());
    uint uiChannelID(pkt.ReadInt());
    byte yChannelFlags(pkt.ReadByte());
    tstring sTopic(pkt.ReadWStringAsTString());
    uint uiNumAdmins(pkt.ReadInt());
    
    m_mapChannels[uiChannelID].sChannelName = sChannel;
    m_mapChannels[uiChannelID].sTopic = sTopic;
    m_mapChannels[uiChannelID].uiFlags = yChannelFlags;
    m_mapChannels[uiChannelID].bUnread = false;
    
    static tsvector vParams(12);
    static tsvector vMiniParams(2);
    
    if (!(yChannelFlags & CHAT_CHANNEL_FLAG_HIDDEN))
    {
        vMiniParams[0] = XtoA(uiChannelID);
        vMiniParams[1] = sChannel;

        ChatNewChannel.Trigger(vMiniParams);

        if (chat_debugInterface)
            Console.UI << _T("HandleChannelChange - ChatNewChannel ") << uiChannelID << _T(" ") << QuoteStr(sChannel) << newl;
    }

    if (yChannelFlags & CHAT_CHANNEL_FLAG_SERVER)
    {
        cc_curGameChannel = sChannel;
        cc_curGameChannelID = uiChannelID;
    }

    m_setChannelsIn.insert(uiChannelID);
    m_mapChannels[uiChannelID].mapAdmins.clear();

    // Read admin list
    for (uint uiLoop(0); uiLoop < uiNumAdmins; uiLoop++)
    {
        if (pkt.HasFaults())
            break;

        uint uiID(pkt.ReadInt());
        byte yLevel(pkt.ReadByte());

        m_mapChannels[uiChannelID].mapAdmins.insert(ChatAdminPair(uiID, yLevel));
    }

    uint uiNumUsers(pkt.ReadInt());

    m_mapChannels[uiChannelID].uiUserCount = uiNumUsers + 1;

    // These stay the same throughout the rest of the function
    vParams[0] = vMiniParams[0] = sChannel; 
    
    vMiniParams[1] = _T("ClearItems();");
    ChatUserEvent.Trigger(vMiniParams);     

    for (uint uiLoop(0); uiLoop < uiNumUsers; uiLoop++)
    {
        if (pkt.HasFaults())
            break;

        tstring sName(pkt.ReadWStringAsTString());
        uint uiAccountID(pkt.ReadInt());
        byte yStatus(pkt.ReadByte());
        byte yUserFlags(pkt.ReadByte());
        uint uiChatSymbol(Host.LookupChatSymbol(pkt.ReadTString()));
        uint uiChatNameColor(Host.LookupChatNameColor(pkt.ReadTString()));
        uint uiAccountIcon(Host.LookupAccountIcon(pkt.ReadTString()));

        ChatClientMap_it findit(m_mapUserList.find(uiAccountID));

        if (findit == m_mapUserList.end())
        {
            SChatClient cNewClient;
            
            cNewClient.sName = sName;
            cNewClient.yStatus = yStatus;
            cNewClient.uiAccountID = uiAccountID;
            cNewClient.yFlags = yUserFlags;
            cNewClient.uiChatSymbol = uiChatSymbol;
            cNewClient.uiChatNameColor = uiChatNameColor;
            cNewClient.uiAccountIcon = uiAccountIcon;
            cNewClient.uiMatchID = -1;

            uint uiChatNameColor2(uiChatNameColor);

            if (cNewClient.yFlags & CHAT_CLIENT_IS_STAFF && uiChatNameColor2 == INVALID_INDEX)
            {
                uint uiDevChatNameColor(Host.LookupChatNameColor(_CTS("s2logo")));
                if (uiDevChatNameColor != INVALID_INDEX)
                    uiChatNameColor2 = uiDevChatNameColor;
            }
            if (cNewClient.yFlags & CHAT_CLIENT_IS_PREMIUM && uiChatNameColor2 == INVALID_INDEX)
            {
                uint uiGoldChatNameColor(Host.LookupChatNameColor(_CTS("goldshield")));
                if (uiGoldChatNameColor != INVALID_INDEX)
                    uiChatNameColor2 = uiGoldChatNameColor;
            }

            if (uiChatNameColor2 != INVALID_INDEX)
                cNewClient.uiSortIndex = Host.GetChatNameColorSortIndex(uiChatNameColor2);
            else
                cNewClient.uiSortIndex = 9;

            m_mapUserList.insert(ChatClientPair(uiAccountID, cNewClient));
            findit = m_mapUserList.find(uiAccountID);
        }
        else
        {
            UpdateClientChannelStatus(TSNULL, sName, uiAccountID, yStatus, yUserFlags, uiChatSymbol, uiChatNameColor, uiAccountIcon);
        }
        
        if (findit->second.yStatus >= CHAT_STATUS_CONNECTED)
        {
            findit->second.setChannels.insert(uiChannelID);

            if (!(yChannelFlags & CHAT_CHANNEL_FLAG_HIDDEN))
            {               
                vParams[1] = sName;
                vParams[2] = XtoA(GetAdminLevel(uiChannelID, findit->second.uiAccountID));
                vParams[3] = XtoA(findit->second.yStatus > CHAT_STATUS_CONNECTED, true);
                vParams[4] = XtoA((findit->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
                vParams[5] = XtoA(findit->second.uiAccountID);
                vParams[6] = Host.GetChatSymbolTexturePath(findit->second.uiChatSymbol);
                vParams[7] = Host.GetChatNameColorTexturePath(findit->second.uiChatNameColor);
                vParams[8] = Host.GetChatNameColorString(findit->second.uiChatNameColor);
                vParams[9] = Host.GetChatNameColorIngameString(findit->second.uiChatNameColor);
                vParams[10] = Host.GetAccountIconTexturePath(findit->second.uiAccountIcon);
                vParams[11] = XtoA(findit->second.uiSortIndex);
                ChatUserNames.Trigger(vParams);
            }

            if (m_eStatus > CHAT_STATUS_CONNECTED && (yChannelFlags & CHAT_CHANNEL_FLAG_SERVER))
                AddToRecentlyPlayed(sName);
        }
    }

    // Add us to the channel list
    ChatClientMap_it findit(m_mapUserList.find(m_uiAccountID));

    if (findit == m_mapUserList.end())
    {
        SChatClient cNewClient;

        cNewClient.sName = TSNULL;
        cNewClient.uiAccountID = m_uiAccountID;
        cNewClient.uiMatchID = -1;
        cNewClient.yFlags = 0;

        m_mapUserList.insert(ChatClientPair(m_uiAccountID, cNewClient));
        findit = m_mapUserList.find(m_uiAccountID);
    }

    findit->second.yStatus = m_eStatus;
    findit->second.setChannels.insert(uiChannelID);

    if (!(yChannelFlags & CHAT_CHANNEL_FLAG_HIDDEN))
    {
        vParams[1] = m_mapUserList[m_uiAccountID].sName;
        vParams[2] = XtoA(GetAdminLevel(uiChannelID, findit->first));
        vParams[3] = XtoA(findit->second.yStatus > CHAT_STATUS_CONNECTED, true);
        vParams[4] = XtoA((findit->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
        vParams[5] = XtoA(findit->second.uiAccountID);
        vParams[6] = Host.GetChatSymbolTexturePath(findit->second.uiChatSymbol);
        vParams[7] = Host.GetChatNameColorTexturePath(findit->second.uiChatNameColor);
        vParams[8] = Host.GetChatNameColorString(findit->second.uiChatNameColor);
        vParams[9] = Host.GetChatNameColorIngameString(findit->second.uiChatNameColor);
        vParams[10] = Host.GetAccountIconTexturePath(findit->second.uiAccountIcon);
        vParams[11] = XtoA(findit->second.uiSortIndex);
        ChatUserNames.Trigger(vParams);

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_new_channel"), _T("channel"), sChannel), sChannel);
        
        vMiniParams[1] = _T("SortListboxSortIndex();");             
        ChatUserEvent.Trigger(vMiniParams);

        vMiniParams[1] = XtoA(uiNumUsers + 1);
        ChatChanNumUsers.Trigger(vMiniParams);

        vMiniParams[1] = sTopic;
        ChatChanTopic.Trigger(vMiniParams);

        if (!sTopic.empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_topic"), _T("topic"), sTopic), sChannel);

        bool bMatchChannel(false);
        for (uiset_it it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); ++it)
        {
            ChatChannelMap_it itFind(m_mapChannels.find(*it));
            if (itFind == m_mapChannels.end())
                continue;

            if (itFind->second.uiFlags & CHAT_CHANNEL_FLAG_SERVER)
            {
                bMatchChannel = true;
                break;
            }
        }

        // Don't change focus if this is a new match channel or if we already have a match channel
        // and this is a general use channel
        if (!(yChannelFlags & CHAT_CHANNEL_FLAG_SERVER) && !(yChannelFlags & CHAT_CHANNEL_FLAG_GENERAL_USE && bMatchChannel))
            SetFocusedChannel(uiChannelID);
    }

    UpdateChannel(uiChannelID);
}


/*====================
  CChatManager::HandleChannelJoin
  ====================*/
void    CChatManager::HandleChannelJoin(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    uint uiAccountID(pkt.ReadInt());
    uint uiChannelID(pkt.ReadInt());
    byte yStatus(pkt.ReadByte());
    byte yFlags(pkt.ReadByte());
    uint uiChatSymbol(Host.LookupChatSymbol(pkt.ReadTString()));
    uint uiChatNameColor(Host.LookupChatNameColor(pkt.ReadTString()));  
    uint uiAccountIcon(Host.LookupAccountIcon(pkt.ReadTString()));  

    tstring sChannel(GetChannelName(uiChannelID));

    if (sChannel.empty())
        return;
        
    static tsvector vMiniParams(2);
    
    // These stay the same throughout the rest of the function
    vMiniParams[0] = sChannel;

    ChatClientMap_it findit(m_mapUserList.find(uiAccountID));

    if (findit == m_mapUserList.end())
    {
        SChatClient cNewClient;

        cNewClient.sName = sName;
        cNewClient.yStatus = CHAT_STATUS_DISCONNECTED;
        cNewClient.uiAccountID = uiAccountID;
        cNewClient.yFlags = 0;
        cNewClient.uiMatchID = -1;
        cNewClient.uiChatSymbol = INVALID_INDEX;
        cNewClient.uiChatNameColor = INVALID_INDEX;
        cNewClient.uiAccountIcon = INVALID_INDEX;
        cNewClient.uiSortIndex = 9;

        m_mapUserList.insert(ChatClientPair(uiAccountID, cNewClient));
        findit = m_mapUserList.find(uiAccountID);
    }

    if (yStatus >= CHAT_STATUS_CONNECTED && findit->second.setChannels.find(uiChannelID) == findit->second.setChannels.end())
    {
        findit->second.setChannels.insert(uiChannelID);

        if (m_eStatus > CHAT_STATUS_CONNECTED && m_mapChannels.find(uiChannelID) != m_mapChannels.end() && (m_mapChannels[uiChannelID].uiFlags & CHAT_CHANNEL_FLAG_SERVER))
            AddToRecentlyPlayed(sName);

        m_mapChannels[uiChannelID].uiUserCount++;

        vMiniParams[1] = XtoA(m_mapChannels[uiChannelID].uiUserCount);      
        ChatChanNumUsers.Trigger(vMiniParams);
    }

    UpdateClientChannelStatus(sChannel, sName, uiAccountID, yStatus, yFlags, uiChatSymbol, uiChatNameColor, uiAccountIcon);
}


/*====================
  CChatManager::HandleChannelMessage
  ====================*/
void    CChatManager::HandleChannelMessage(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    uint uiChannelID(pkt.ReadInt());
    tstring sMessage(pkt.ReadWStringAsTString());

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end() || sMessage.empty())
        return;

    if (IsIgnored(it->first))
        return;

    // only play channel sounds for the active channel
    if (m_uiFocusedChannel != uiChannelID)
        AddUnreadChannel(uiChannelID);
    else
        ChatRecievedChannelMessage.Trigger(GetChannelName(uiChannelID));

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_channel_message"), _T("sender"), it->second.sName, _T("message"), sMessage), GetChannelName(uiChannelID), true);
}


/*====================
  CChatManager::HandleChannelLeave
  ====================*/
void    CChatManager::HandleChannelLeave(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    uint uiChannelID(pkt.ReadInt());

    tstring sChannel(GetChannelName(uiChannelID));

    if (sChannel.empty())
        return;

    if (uiAccountID == m_uiAccountID)
    {
        // We left or were removed from the channel
        RemoveUnreadChannel(uiChannelID);
        m_setChannelsIn.erase(uiChannelID);
        ChatLeftChannel.Trigger(XtoA(uiChannelID));
        return;
    }

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return;

    if (it->second.setChannels.find(uiChannelID) != it->second.setChannels.end())
    {
        static tsvector vParams(2);
        vParams[0] = sChannel;
        vParams[1] = _T("EraseListItemByValue('") + it->second.sName + _T("');");
        ChatUserEvent.Trigger(vParams);

        it->second.setChannels.erase(uiChannelID);
        m_mapChannels[uiChannelID].uiUserCount--;

        vParams[1] = XtoA(m_mapChannels[uiChannelID].uiUserCount);
        ChatChanNumUsers.Trigger(vParams);
    }
}


/*====================
  CChatManager::HandleWhisper
  ====================*/
void    CChatManager::HandleWhisper(CPacket &pkt)
{
    tstring sSenderName(pkt.ReadWStringAsTString());
    tstring sMessage(pkt.ReadWStringAsTString());

    if (IsIgnored(sSenderName))
        return;

    m_lLastWhispers.remove(sSenderName);
    m_lLastWhispers.push_front(sSenderName);
    
    PlaySound(_T("RecievedWhisper"));
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_whisper"), _T("sender"), sSenderName, _T("message"), sMessage), TSNULL, true);
}


/*====================
  CChatManager::HandleWhisperBuddies
  ====================*/
void    CChatManager::HandleWhisperBuddies(CPacket &pkt)
{
    const tstring sSenderName(pkt.ReadWStringAsTString());
    const tstring sMessage(pkt.ReadWStringAsTString());

    if (IsIgnored(sSenderName))
        return;

    m_lLastWhispers.remove(sSenderName);
    m_lLastWhispers.push_front(sSenderName);

    PlaySound(_T("RecievedWhisper"));
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_whisper_to_buddies"), _T("sender"), sSenderName, _T("message"), sMessage), TSNULL, true);
}


/*====================
  CChatManager::HandleWhisperFailed
  ====================*/
void    CChatManager::HandleWhisperFailed()
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_offline")));
}


/*====================
  CChatManager::HandleIM
  ====================*/
void    CChatManager::HandleIM(CPacket &pkt)
{
    const tstring sSenderName(RemoveClanTag(pkt.ReadWStringAsTString()));
    const tstring sMessage(pkt.ReadWStringAsTString());

    if (IsIgnored(sSenderName))
        return;

    m_cDate = CDate(true);
    tstring sFinal(_T("^770[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + Translate(_T("chat_im"), _T("sender"), sSenderName, _T("message"), sMessage));

    m_mapIMs[sSenderName].push_back(sFinal);

    if (cc_showIMNotification)
        PushNotification(NOTIFY_TYPE_IM, sSenderName);

    static tsvector vParams(3);
    vParams[0] = sSenderName;
    vParams[1] = sFinal;
    vParams[2] = _T("1");
    ChatWhisperUpdate.Trigger(vParams);
}


/*====================
  CChatManager::HandleIMFailed
  ====================*/
void    CChatManager::HandleIMFailed(CPacket &pkt)
{
    tstring sTarget(RemoveClanTag(pkt.ReadWStringAsTString()));
    tstring sFinal(Translate(_T("chat_im_failed"), _T("target"), sTarget));

    m_mapIMs[sTarget].push_back(sFinal);

    static tsvector vParams(3);
    vParams[0] = sTarget;
    vParams[1] = sFinal;
    vParams[2] = _T("0");
    ChatWhisperUpdate.Trigger(vParams);
}


/*====================
  CChatManager::HandleClanWhisperFailed
  ====================*/
void    CChatManager::HandleClanWhisperFailed()
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_whisper_failed")));
}


/*====================
  CChatManager::HandleInitialStatusUpdate
  ====================*/
void    CChatManager::HandleInitialStatusUpdate(CPacket &pkt)
{
    uint uiNumUpdated(pkt.ReadInt());
    ChatClientMap_it it;

    for (uint uiLoop(0); uiLoop < uiNumUpdated; uiLoop++)
    {
        if (pkt.HasFaults())
            break;

        uint uiAccountID(pkt.ReadInt());
        byte yStatus(pkt.ReadByte());
        byte yFlags(pkt.ReadByte());
        tstring sServerAddressPort;
        tstring sGameName;

        if (yStatus > CHAT_STATUS_CONNECTED)
        {
            sServerAddressPort = pkt.ReadWStringAsTString();
            sGameName = pkt.ReadWStringAsTString();
        }

        it = m_mapUserList.find(uiAccountID);
        if (it != m_mapUserList.end())
        {
            it->second.yStatus = yStatus;
            it->second.yFlags = yFlags;
            it->second.sServerAddressPort = sServerAddressPort;
            it->second.sGameName = sGameName;
        }

        if (IsBuddy(uiAccountID))
            RefreshBuddyList();

        if (IsClanMember(uiAccountID))
            RefreshClanList();
    }
}


/*====================
  CChatManager::HandleStatusUpdate
  ====================*/
void    CChatManager::HandleStatusUpdate(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    byte yStatus(pkt.ReadByte());
    byte yFlags(pkt.ReadByte());
    int iClanID(pkt.ReadInt());
    tstring sClan(pkt.ReadWStringAsTString());
    uint uiChatSymbol(Host.LookupChatSymbol(pkt.ReadTString()));
    uint uiChatNameColor(Host.LookupChatNameColor(pkt.ReadTString()));  
    uint uiAccountIcon(Host.LookupAccountIcon(pkt.ReadTString()));
        
    tstring sServerAddressPort(TSNULL);
    tstring sGameName(TSNULL);
    uint uiMatchID(-1);
            
    byte    yArrangedType(0);
    tstring sPlayerName(TSNULL);
    tstring sRegion(TSNULL);;
    tstring sGameModeName(TSNULL);;
    byte    yTeamSize(0);
    tstring sMapName(TSNULL);
    byte    yTier(0);
    byte    yNoStats(0);
    byte    yNoLeavers(0);
    byte    yPrivate(0);
    byte    yAllHeroes(0);
    byte    yCasualMode(0);
    byte    yForceRandom(0);
    byte    yAutoBalanced(0);
    byte    yAdvancedOptions(0);    
    ushort  unMinPSR(0);
    ushort  unMaxPSR(0);
    byte    yDevHeroes(0);
    byte    yHardcore(0);
        
    if (yStatus > CHAT_STATUS_CONNECTED)
    {
        sServerAddressPort = pkt.ReadWStringAsTString();
    }
    
    if (yStatus == CHAT_STATUS_IN_GAME)
    {
        sGameName = pkt.ReadWStringAsTString();
        uiMatchID = pkt.ReadInt();

        byte yExtendedInfo(pkt.ReadByte());

        if (yExtendedInfo != 0)
        {
            // new stuff the chat server will send related to game info
            yArrangedType = pkt.ReadByte();
            sPlayerName = pkt.ReadWStringAsTString();
            sRegion = pkt.ReadWStringAsTString();
            sGameModeName = pkt.ReadWStringAsTString();
            yTeamSize = pkt.ReadByte();
            sMapName = pkt.ReadWStringAsTString();
            yTier = pkt.ReadByte();
            yNoStats = pkt.ReadByte();
            yNoLeavers = pkt.ReadByte();
            yPrivate = pkt.ReadByte();
            yAllHeroes = pkt.ReadByte();
            yCasualMode = pkt.ReadByte();
            yForceRandom = pkt.ReadByte();
            yAutoBalanced = pkt.ReadByte();
            yAdvancedOptions = pkt.ReadByte();
            unMinPSR = pkt.ReadShort();
            unMaxPSR = pkt.ReadShort();
            yDevHeroes = pkt.ReadByte();
            yHardcore = pkt.ReadByte();
        }
    }

    ChatClientMap_it it = m_mapUserList.find(uiAccountID);
    bool bAddedNotification(false);
    static tsvector vParams(20);    

    if (it == m_mapUserList.end())
        return;

    if (m_bFollow && m_sFollowName == RemoveClanTag(it->second.sName) && yStatus != CHAT_STATUS_IN_GAME &&
        CompareNoCase(sServerAddressPort.substr(0, 9), LOCALHOST) != 0 &&
        yArrangedType != 1)
        UpdateFollow(sServerAddressPort);

    it->second.uiMatchID = uiMatchID;

    if (GetAccountID() != uiAccountID && IsBuddy(uiAccountID))
    {
        RefreshBuddyList();

        if (yStatus == CHAT_STATUS_CONNECTED && it->second.yStatus == CHAT_STATUS_DISCONNECTED && cc_showBuddyConnectionNotification)
        {
            PushNotification(NOTIFY_TYPE_BUDDY_ONLINE, it->second.sName);
        }
        else if (yStatus < CHAT_STATUS_IN_GAME && it->second.yStatus == CHAT_STATUS_IN_GAME && cc_showBuddyLeaveGameNotification)
        {
            PushNotification(NOTIFY_TYPE_BUDDY_LEFT_GAME, it->second.sName, it->second.sGameName);
        }
        else if (yStatus == CHAT_STATUS_DISCONNECTED && it->second.yStatus > CHAT_STATUS_DISCONNECTED && cc_showBuddyDisconnectionNotification)
        {
            PushNotification(NOTIFY_TYPE_BUDDY_OFFLINE, it->second.sName);
        }
        else if (yStatus == CHAT_STATUS_IN_GAME && it->second.yStatus < CHAT_STATUS_IN_GAME && cc_showBuddyJoinGameNotification)
        {
            if (CompareNoCase(sServerAddressPort.substr(0, 9), LOCALHOST) != 0)
            {
                vParams[0] = sServerAddressPort;        // Address
                vParams[1] = sGameName;                 // Game Name
                vParams[2] = it->second.sName;          // Buddy Name
                vParams[3] = sRegion;                   // Server Region
                vParams[4] = sGameModeName;             // Game Mode Name (banningdraft)
                vParams[5] = XtoA(yTeamSize);           // Team Size            
                vParams[6] = sMapName;                  // Map Name (caldavar)
                vParams[7] = XtoA(yTier);               // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
                vParams[8] = XtoA(yNoStats);            // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
                vParams[9] = XtoA(yNoLeavers);          // No Leavers (1), Leavers (0)
                vParams[10] = XtoA(yPrivate);           // Private (1), Not Private (0)                                 
                vParams[11] = XtoA(yAllHeroes);         // All Heroes (1), Not All Heroes (0)
                vParams[12] = XtoA(yCasualMode);        // Casual Mode (1), Not Casual Mode (0)
                vParams[13] = XtoA(yForceRandom);       // Force Random (1), Not Force Random (0)
                vParams[14] = XtoA(yAutoBalanced);      // Auto Balanced (1), Non Auto Balanced (0)
                vParams[15] = XtoA(yAdvancedOptions);   // Advanced Options (1), No Advanced Options (0)
                vParams[16] = XtoA(unMinPSR);           // Min PSR
                vParams[17] = XtoA(unMaxPSR);           // Max PSR
                vParams[18] = XtoA(yDevHeroes);         // Dev Heroes (1), Non Dev Heroes (0)
                vParams[19] = XtoA(yHardcore);          // Hardcore (1), Non Hardcore (0)
            
                PushNotification(NOTIFY_TYPE_BUDDY_JOIN_GAME, XtoA(yArrangedType), TSNULL, TSNULL, vParams);
            }
        }

        bAddedNotification = true;
    }
    
    if (GetAccountID() != uiAccountID && IsClanMember(uiAccountID))
    {
        RefreshClanList();

        if (!bAddedNotification)
        {
            if (yStatus == CHAT_STATUS_CONNECTED && it->second.yStatus == CHAT_STATUS_DISCONNECTED && cc_showClanConnectionNotification)
            {
                PushNotification(NOTIFY_TYPE_CLAN_ONLINE, it->second.sName);
            }
            else if (yStatus < CHAT_STATUS_IN_GAME && it->second.yStatus == CHAT_STATUS_IN_GAME && cc_showClanLeaveGameNotification)
            {
                PushNotification(NOTIFY_TYPE_CLAN_LEFT_GAME, it->second.sName, it->second.sGameName);
            }
            else if (yStatus == CHAT_STATUS_DISCONNECTED && it->second.yStatus > CHAT_STATUS_DISCONNECTED && cc_showClanDisconnectionNotification)
            {
                PushNotification(NOTIFY_TYPE_CLAN_OFFLINE, it->second.sName);
            }
            else if (yStatus == CHAT_STATUS_IN_GAME && it->second.yStatus < CHAT_STATUS_IN_GAME && cc_showClanJoinGameNotification)
            {
                if (CompareNoCase(sServerAddressPort.substr(0, 9), LOCALHOST) != 0)
                {
                    vParams[0] = sServerAddressPort;                // Address
                    vParams[1] = sGameName;                         // Game Name
                    vParams[2] = RemoveClanTag(it->second.sName);   // Clan Member Name
                    vParams[3] = sRegion;                           // Server Region
                    vParams[4] = sGameModeName;                     // Game Mode Name (banningdraft)
                    vParams[5] = XtoA(yTeamSize);                   // Team Size            
                    vParams[6] = sMapName;                          // Map Name (caldavar)
                    vParams[7] = XtoA(yTier);                       // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
                    vParams[8] = XtoA(yNoStats);                    // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
                    vParams[9] = XtoA(yNoLeavers);                  // No Leavers (1), Leavers (0)
                    vParams[10] = XtoA(yPrivate);                   // Private (1), Not Private (0)                                 
                    vParams[11] = XtoA(yAllHeroes);                 // All Heroes (1), Not All Heroes (0)
                    vParams[12] = XtoA(yCasualMode);                // Casual Mode (1), Not Casual Mode (0)
                    vParams[13] = XtoA(yForceRandom);               // Force Random (1), Not Force Random (0)
                    vParams[14] = XtoA(yAutoBalanced);              // Auto Balanced (1), Non Auto Balanced (0)
                    vParams[15] = XtoA(yAdvancedOptions);           // Advanced Options (1), No Advanced Options (0)
                    vParams[16] = XtoA(unMinPSR);                   // Min PSR
                    vParams[17] = XtoA(unMaxPSR);                   // Max PSR                  
                    vParams[18] = XtoA(yDevHeroes);                 // Dev Heroes (1), Non Dev Heroes (0)
                    vParams[19] = XtoA(yHardcore);                  // Hardcore (1), Non Hardcore (0)
                
                    PushNotification(NOTIFY_TYPE_CLAN_JOIN_GAME, XtoA(yArrangedType), TSNULL, TSNULL, vParams);
                }
            }
        }
    }
    
    
    // save a notification that this player joined a game, in case they disconnect and want to rejoin
    if (GetAccountID() == uiAccountID && yStatus == CHAT_STATUS_IN_GAME)
    {
        if (CompareNoCase(sServerAddressPort.substr(0, 9), LOCALHOST) != 0)
        {
            vParams[0] = sServerAddressPort;        // Address
            vParams[1] = sGameName;                 // Game Name
            vParams[2] = it->second.sName;          // Self
            vParams[3] = sRegion;                   // Server Region
            vParams[4] = sGameModeName;             // Game Mode Name (banningdraft)
            vParams[5] = XtoA(yTeamSize);           // Team Size            
            vParams[6] = sMapName;                  // Map Name (caldavar)
            vParams[7] = XtoA(yTier);               // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
            vParams[8] = XtoA(yNoStats);            // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats
            vParams[9] = XtoA(yNoLeavers);          // No Leavers (1), Leavers (0)
            vParams[10] = XtoA(yPrivate);           // Private (1), Not Private (0)                                 
            vParams[11] = XtoA(yAllHeroes);         // All Heroes (1), Not All Heroes (0)
            vParams[12] = XtoA(yCasualMode);        // Casual Mode (1), Not Casual Mode (0)
            vParams[13] = XtoA(yForceRandom);       // Force Random (1), Not Force Random (0)
            vParams[14] = XtoA(yAutoBalanced);      // Auto Balanced (1), Non Auto Balanced (0)
            vParams[15] = XtoA(yAdvancedOptions);   // Advanced Options (1), No Advanced Options (0)
            vParams[16] = XtoA(unMinPSR);           // Min PSR
            vParams[17] = XtoA(unMaxPSR);           // Max PSR
            vParams[18] = XtoA(yDevHeroes);         // Dev Heroes (1), Non Dev Heroes (0)
            vParams[19] = XtoA(yHardcore);          // Hardcore (1), Non Hardcore (0)                   
        
            PushNotification(NOTIFY_TYPE_SELF_JOIN_GAME, TSNULL, TSNULL, TSNULL, vParams);
        }
    }

    if (GetAccountID() != uiAccountID && m_mapIMs.find(RemoveClanTag(it->second.sName)) != m_mapIMs.end())
    {
        m_cDate = CDate(true);
        tstring sFinal;
        
        if (it->second.yStatus > CHAT_STATUS_DISCONNECTED && yStatus == CHAT_STATUS_DISCONNECTED)
            sFinal = _T("^770[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + Translate(_T("chat_im_user_offline"), _T("name"), RemoveClanTag(it->second.sName));
        else if (it->second.yStatus == CHAT_STATUS_DISCONNECTED && yStatus > CHAT_STATUS_DISCONNECTED)
            sFinal = _T("^770[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + Translate(_T("chat_im_user_online"), _T("name"), RemoveClanTag(it->second.sName));
            
        if (!sFinal.empty())
        {
            m_mapIMs[RemoveClanTag(it->second.sName)].push_back(sFinal);

            static tsvector vMiniParams(3);
            vMiniParams[0] = RemoveClanTag(it->second.sName);
            vMiniParams[1] = sFinal;
            vMiniParams[2] = _T("0");
            ChatWhisperUpdate.Trigger(vMiniParams);
        }
    }

    it->second.sServerAddressPort = sServerAddressPort;
    it->second.sGameName = sGameName;
    it->second.iClanID = iClanID;
    it->second.sClan = sClan;

    UpdateClientChannelStatus(TSNULL, it->second.sName, uiAccountID, yStatus, yFlags, uiChatSymbol, uiChatNameColor, uiAccountIcon);
}


/*====================
  CChatManager::HandleClanWhisper
  ====================*/
void    CChatManager::HandleClanWhisper(CPacket &pkt)
{
    const uint uiAccountID(pkt.ReadInt());
    const tstring sMessage(pkt.ReadWStringAsTString());

    ChatClientMap_it itFind(m_mapUserList.find(uiAccountID));

    if (itFind == m_mapUserList.end())
        return;

    if (IsIgnored(itFind->first))
        return;

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_whisper"), _T("name"), itFind->second.sName, _T("message"), sMessage));

    const tstring sIM(Translate(_T("chat_clan_im"), _T("name"), itFind->second.sName, _T("message"), sMessage));

    m_vClanWhispers.push_back(sIM);
    ChatClanWhisperUpdate.Trigger(sIM);
    PlaySound(_T("RecievedClanMessage"));

    m_lLastWhispers.remove(itFind->second.sName);
    m_lLastWhispers.push_front(itFind->second.sName);

    //if (cc_showClanMessageNotification)
    //{
        //PushNotification(NOTIFY_TYPE_CLAN_WHISPER, Translate(_T("chat_notification_clan_whisper"), _T("name"), itFind->second.sName, _T("message"), sMessage), itFind->second.sName);     
    //}
}


/*====================
  CChatManager::HandleMultipleLookingForClan
  ====================*/
void    CChatManager::HandleMultipleLookingForClan(CPacket &pkt)
{
    uint uiNumEntries(pkt.ReadInt());

    for (uint ui(0); ui < uiNumEntries; ++ui)
    {
        if (pkt.HasFaults())
            break;

        tstring sName(pkt.ReadWStringAsTString());

        sset_it it(m_setLookingForClan.find(sName));

        if (it == m_setLookingForClan.end())
            m_setLookingForClan.insert(sName);
    }

    UpdateLookingForClan();
}


/*====================
  CChatManager::HandleLookingForClan
  ====================*/
void    CChatManager::HandleLookingForClan(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());

    sset_it it(m_setLookingForClan.find(sName));

    if (it == m_setLookingForClan.end())
    {
        m_setLookingForClan.insert(sName);
        UpdateLookingForClan();
    }
}


/*====================
  CChatManager::HandleNotLookingForClan
  ====================*/
void    CChatManager::HandleNotLookingForClan(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());

    sset_it it(m_setLookingForClan.find(sName));

    if (it != m_setLookingForClan.end())
    {
        m_setLookingForClan.erase(it);
        UpdateLookingForClan();
    }

}


/*====================
  CChatManager::HandleFlooding
  ====================*/
void    CChatManager::HandleFlooding()
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_flooding")));
}


/*====================
  CChatManager::HandleMaxChannels
  ====================*/
void    CChatManager::HandleMaxChannels()
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_max_channels")));
}


/*====================
  CChatManager::HandleChannelInfo
  ====================*/
void    CChatManager::HandleChannelInfo(CPacket &pkt)
{
    uint uiID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());
    ushort unUsers(pkt.ReadShort());
    if (pkt.HasFaults())
        return;
    
    SChatChannel &channel(m_mapChannels[uiID]);
    channel.sChannelName = sName;
    channel.uiUserCount = unUsers;

    ChatChannelList.Execute(_T("Data('") + XtoA(uiID) + _T("','") + sName + _T("','") + XtoA(unUsers) + _T("');"));
    ChatChannelList.Execute(_T("SortByCol(0);"));

    //Console << sName << _T(" ") << uiID << _T(" ") << unUsers << newl;
}


/*====================
  CChatManager::HandleChannelInfoSub
  ====================*/
void    CChatManager::HandleChannelInfoSub(CPacket &pkt)
{
    byte ySequence(pkt.ReadByte());
    uint uiID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());
    ushort unUsers(pkt.ReadShort());
    if (pkt.HasFaults())
        return;

    if (ySequence != m_yProcessingListSequence)
        return;

    SChatChannelInfo &channel(m_mapChannelList[uiID]);

    channel.sName = sName;
    channel.sLowerName = LowerString(sName);
    channel.uiUserCount = unUsers;
    
    //SChatChannel &channel(m_mapChannels[uiID]);
    //channel.sChannelName = sName;
    //channel.uiUserCount = unUsers;

    //ChatChannelList.Execute(_T("Data('") + XtoA(uiID) + _T("','") + sName + _T("','") + XtoA(unUsers) + _T("');"));
    //ChatChannelList.Execute(_T("SortByCol(0);"));

    ChatAutoCompleteAdd.Trigger(sName);

    //Console << sName << _T(" ") << uiID << _T(" ") << unUsers << newl;
}


/*====================
  CChatManager::HandleUserStatus
  ====================*/
void    CChatManager::HandleUserStatus(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    byte yStatus(pkt.ReadByte());
    if (pkt.HasFaults())
        return;

    tsvector vParams(2);
    vParams[0] = RemoveClanTag(sName);
    vParams[1] = XtoA(yStatus);

    ChatUserStatus.Trigger(vParams);
}


/*====================
  CChatManager::HandleServerInvite
  ====================*/
void    CChatManager::HandleServerInvite(CPacket &pkt)
{
    tstring sInviterName(pkt.ReadWStringAsTString());
    int iInviterAccountID(pkt.ReadInt());
    tstring sAddressPort(pkt.ReadWStringAsTString());

    if (IsIgnored(iInviterAccountID) || !cc_showGameInvites)
    {
        RejectServerInvite(iInviterAccountID);
        return;
    }

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient != NULL)
    {
        if (pClient->ServerInvite(sInviterName, iInviterAccountID, sAddressPort))
        {
            m_mapUserList[iInviterAccountID].sServerAddressPort = sAddressPort;
        }
    }
}


/*====================
  CChatManager::HandleInviteFailedUserNotFound
  ====================*/
void    CChatManager::HandleInviteFailedUserNotFound()
{
}


/*====================
  CChatManager::HandleInviteFailedNotInGame
  ====================*/
void    CChatManager::HandleInviteFailedNotInGame()
{
}


/*====================
  CChatManager::HandleInviteRejected
  ====================*/
void    CChatManager::HandleInviteRejected(CPacket &pkt)
{
    pkt.ReadWString(); // wstring sName - Name of the rejecting client
    pkt.ReadInt(); // int iAccountID - ID of the rejecting client
}


/*====================
  CChatManager::HandleUserInfoNoExist
  ====================*/
void    CChatManager::HandleUserInfoNoExist(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_not_found"), _T("name"), sName));
}


/*====================
  CChatManager::HandleUserInfoOffline
  ====================*/
void    CChatManager::HandleUserInfoOffline(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    tstring sLastOnline(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_offline"), _T("name"), sName, _T("seen"), sLastOnline));
}


/*====================
  CChatManager::HandleUserInfoInGame
  ====================*/
void    CChatManager::HandleUserInfoInGame(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    tstring sGameName(pkt.ReadWStringAsTString());
    tstring sCGT(pkt.ReadWStringAsTString());

    if (sCGT.empty())
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_in_game"), _T("name"), sName, _T("game"), sGameName));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_in_game_time"), _T("name"), sName, _T("game"), sGameName, _T("cgt"), sCGT));
}


/*====================
  CChatManager::HandleUserInfoOnline
  ====================*/
void    CChatManager::HandleUserInfoOnline(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    uint uiNumChannels(pkt.ReadInt());
    
    if (uiNumChannels == 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_online_no_channels"), _T("name"), sName));
    else if (uiNumChannels == 1)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_online_one_channel"), _T("name"), sName, _T("channel"), pkt.ReadWStringAsTString()));
    else
    {
        tstring sChannels;

        for (uint i(0); i < uiNumChannels; ++i)
        {
            if (pkt.HasFaults())
                break;

            if (i == 0)
                sChannels = pkt.ReadWStringAsTString();
            else
                sChannels += _T(", ") + pkt.ReadWStringAsTString();
        }

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_info_online_multi_channels"), _T("name"), sName, _T("channels"), sChannels));
    }
}


/*====================
  CChatManager::HandleChannelUpdate
  ====================*/
void    CChatManager::HandleChannelUpdate(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());
    byte yFlags(pkt.ReadByte());
    tstring sTopic(pkt.ReadWStringAsTString());

    m_mapChannels[uiChannelID].sChannelName = sName;
    m_mapChannels[uiChannelID].uiFlags = yFlags;
    m_mapChannels[uiChannelID].sTopic = sTopic;

    uint uiNumAdmins(pkt.ReadInt());

    m_mapChannels[uiChannelID].mapAdmins.clear();

    for (uint ui(0); ui < uiNumAdmins; ui++)
    {
        if (pkt.HasFaults())
            break;

        uint uiID(pkt.ReadInt());
        byte yLevel(pkt.ReadByte());

        m_mapChannels[uiChannelID].mapAdmins.insert(ChatAdminPair(uiID, yLevel));
    }

    UpdateChannel(uiChannelID);
}


/*====================
  CChatManager::HandleChannelTopic
  ====================*/
void    CChatManager::HandleChannelTopic(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sTopic(pkt.ReadWStringAsTString());

    m_mapChannels[uiChannelID].sTopic = sTopic;

    tsvector vsTopic(2);
    vsTopic[0] = m_mapChannels[uiChannelID].sChannelName;
    vsTopic[1] = m_mapChannels[uiChannelID].sTopic;

    ChatChanTopic.Trigger(vsTopic);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_topic_change"), _T("topic"), sTopic), m_mapChannels[uiChannelID].sChannelName);
}


/*====================
  CChatManager::HandleChannelKick
  ====================*/
void    CChatManager::HandleChannelKick(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiKickerID(pkt.ReadInt());
    uint uiKickeeID(pkt.ReadInt());

    ChatClientMap_it itKicker(m_mapUserList.find(uiKickerID));
    if (itKicker == m_mapUserList.end())
        return;

    ChatClientMap_it itKickee(m_mapUserList.find(uiKickeeID));
    if (itKickee == m_mapUserList.end())
        return;

    if (uiKickeeID != m_uiAccountID)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_kicked"), _T("kicker"), itKicker->second.sName, _T("kickee"), itKickee->second.sName), GetChannelName(uiChannelID));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_kicked"), _T("kicker"), itKicker->second.sName, _T("channel"), GetChannelName(uiChannelID)));
}


/*====================
  CChatManager::HandleChannelBan
  ====================*/
void    CChatManager::HandleChannelBan(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiBanningID(pkt.ReadInt());
    tstring sBannedName(pkt.ReadWStringAsTString());

    ChatClientMap_it it = m_mapUserList.find(uiBanningID);

    tstring sBanner;
    if (it != m_mapUserList.end())
        sBanner = it->second.sName;

    if (!CompareNames(sBannedName, m_mapUserList[m_uiAccountID].sName))
    {
        if (sBanner.empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_banned_no_name"), _T("banned"), sBannedName), GetChannelName(uiChannelID));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_banned"), _T("banner"), sBanner, _T("banned"), sBannedName), GetChannelName(uiChannelID));
    }
    else 
    {
        if (sBanner.empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_channel_banned_no_name"), _T("channel"), GetChannelName(uiChannelID)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_channel_banned"), _T("banner"), sBanner, _T("channel"), GetChannelName(uiChannelID)));
    }
}


/*====================
  CChatManager::HandleChannelUnban
  ====================*/
void    CChatManager::HandleChannelUnban(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiUnbanningID(pkt.ReadInt());
    tstring sBannedName(pkt.ReadWStringAsTString());

    ChatClientMap_it it = m_mapUserList.find(uiUnbanningID);

    tstring sBanner;
    if (it != m_mapUserList.end())
        sBanner = it->second.sName;

    if (!CompareNames(sBannedName, m_mapUserList[m_uiAccountID].sName))
    {
        if (sBanner.empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_unbanned_no_name"), _T("unbanned"), sBannedName), GetChannelName(uiChannelID));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_unbanned"), _T("unbanner"), sBanner, _T("unbanned"), sBannedName), GetChannelName(uiChannelID));
    }
    else
    {
        if (sBanner.empty())
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_unbanned_no_name"), _T("channel"), GetChannelName(uiChannelID)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_unbanned"), _T("unbanner"), sBanner, _T("channel"), GetChannelName(uiChannelID)));
    }
}


/*====================
  CChatManager::HandleBannedFromChannel
  ====================*/
void    CChatManager::HandleBannedFromChannel(CPacket &pkt)
{
    tstring sChannelName(pkt.ReadWStringAsTString());
    
    for (sset_it it(m_setAutoJoinChannels.begin()), itEnd(m_setAutoJoinChannels.end()); it != itEnd; ++it)
    {
        // Send a request to remove the channel from their autojoin channel list if they are banned from it.
        // If they are banned from a channel then they aren't able to remove the channel from their list because they aren't 
        // able to actually join the channel, and thus they never see the "Auto Connect" checkbox for the channel.
        if (CompareNoCase(sChannelName, it->c_str()) == 0)
        {
            RemoveChannel(sChannelName);
            break;  
        }
    }       
    
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_banned_from_channel"), _T("channel"), sChannelName));
}


/*====================
  CChatManager::HandleChannelSilenced
  ====================*/
void    CChatManager::HandleChannelSilenced(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_silenced")), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelSilenceLifted
  ====================*/
void    CChatManager::HandleChannelSilenceLifted(CPacket &pkt)
{
    tstring sChannelName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_silence_lifted"), _T("channel"), sChannelName));
}


/*====================
  CChatManager::HandleChannelPromote
  ====================*/
void    CChatManager::HandleChannelPromote(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiAccountID(pkt.ReadInt());
    uint uiPromoterID(pkt.ReadInt());

    ChatChannelMap_it channelit(m_mapChannels.find(uiChannelID));

    if (channelit == m_mapChannels.end())
        return;

    ChatClientMap_it userit(m_mapUserList.find(uiAccountID));

    if (userit == m_mapUserList.end())
        return;

    ChatClientMap_it promoterit(m_mapUserList.find(uiPromoterID));

    if (promoterit == m_mapUserList.end())
        return;

    ChatAdminMap_it adminit(channelit->second.mapAdmins.find(uiAccountID));

    if (adminit == channelit->second.mapAdmins.end())
    {
        channelit->second.mapAdmins.insert(ChatAdminPair(userit->first, CHAT_CLIENT_ADMIN_NONE));
        adminit = channelit->second.mapAdmins.find(userit->first);
    }

    if (adminit == channelit->second.mapAdmins.end())
        return;

    adminit->second++;

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_promote_success"), _T("name"), userit->second.sName, _T("rank"), Translate(g_sAdminNames[adminit->second]), _T("promoter"), promoterit->second.sName), channelit->second.sChannelName);

    UpdateChannel(uiChannelID);
}


/*====================
  CChatManager::HandleChannelDemote
  ====================*/
void    CChatManager::HandleChannelDemote(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiAccountID(pkt.ReadInt());
    uint uiDemoterID(pkt.ReadInt());

    ChatChannelMap_it channelit(m_mapChannels.find(uiChannelID));

    if (channelit == m_mapChannels.end())
        return;

    ChatClientMap_it userit(m_mapUserList.find(uiAccountID));

    if (userit == m_mapUserList.end())
        return;

    ChatClientMap_it demoterit(m_mapUserList.find(uiDemoterID));

    if (demoterit == m_mapUserList.end())
        return;

    ChatAdminMap_it adminit(channelit->second.mapAdmins.find(uiAccountID));

    if (adminit == channelit->second.mapAdmins.end())
        return;

    adminit->second--;

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_demote_success"), _T("name"), userit->second.sName, _T("rank"), Translate(g_sAdminNames[adminit->second]), _T("demoter"), demoterit->second.sName), channelit->second.sChannelName);

    UpdateChannel(uiChannelID);
}


/*====================
  CChatManager::HandleSilencePlaced
  ====================*/
void    CChatManager::HandleSilencePlaced(CPacket &pkt)
{
    tstring sChannel(pkt.ReadWStringAsTString());
    tstring sName(pkt.ReadWStringAsTString());
    tstring sSilenced(pkt.ReadWStringAsTString());
    uint uiDuration(pkt.ReadInt());

    uiDuration = MsToMin(uiDuration);

    if (CompareNames(sSilenced, m_mapUserList[m_uiAccountID].sName))
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_silence_placed"), _T("channel"), sChannel, _T("name"), sName, _T("duration"), XtoA(uiDuration)));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_silence_placed"), _T("name"), sName, _T("silenced"), sSilenced, _T("duration"), XtoA(uiDuration)), sChannel);
}


/*====================
  CChatManager::HandleMessageAll
  ====================*/
void    CChatManager::HandleMessageAll(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    tstring sMessage(pkt.ReadWStringAsTString());

    // Always show date/time when displaying a server message
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_message_all"), _T("name"), sName, _T("message"), sMessage), TSNULL, true);
}


/*====================
  CChatManager::HandleAuthAccepted
  ====================*/
void    CChatManager::HandleAuthAccepted()
{
    m_uiConnectRetries = 0;
    m_uiNextReconnectTime = INVALID_TIME;

    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_connected_invisible")));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_connected")));

    m_eStatus = CHAT_STATUS_CONNECTED;

    m_uiLastRecvTime = K2System.Milliseconds();

/*
    // Looking for clan stuff doesn't work properly or is not intended to be part of HoN
    // Update the server on our "looking for clan" status
    if (cc_lookingForClan)
    {
        cc_lookingForClan.SetModified(false);

        CPacket pktSend;
        pktSend << CHAT_CMD_LOOKING_FOR_CLAN;
        m_sockChat.SendPacket(pktSend);
    }
*/

    // Update server on our status
    if (Host.IsConnected())
        JoiningGame(Host.GetConnectedAddress());

    if (Host.IsInGame())
        FinishedJoiningGame(m_sGameName, m_uiMatchID);

    UpdateRecentlyPlayed();

    // always set these on logging in or else a player may see the previous account they logged in as
    // as being online, or in the clan panel
    RefreshBuddyList();
    RefreshClanList();

    // Update server on channels we're in
    for (uiset::iterator it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); it++)
        JoinChannel(GetChannelName(*it));

    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
    {
        // show the "Status" channel that lets them know they are logged in in invisible mode when logging in instead of no focused channels
        SetFocusedChannel(-1, true);
    }
    else
    {
        // Try to join each of the auto join channels here
        for (sset_it it(m_setAutoJoinChannels.begin()), itEnd(m_setAutoJoinChannels.end()); it != itEnd; ++it)      
        {
            tstring sChannelName(it->c_str());
            JoinChannel(sChannelName);
        }
    }
}


/*====================
  CChatManager::HandleRejected
  ====================*/
void    CChatManager::HandleRejected(CPacket &pkt)
{
    byte yReason(pkt.ReadByte(SERVER_REJECT_UNKNOWN));
    switch (yReason)
    {
    case CHAT_CLIENT_REJECT_UNKNOWN:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_reject_unknown")));
        break;
    case CHAT_CLIENT_REJECT_BAD_VERSION:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_version_mismatch")));
        break;
    case CHAT_CLIENT_REJECT_AUTH_FAILED:
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_reject_auth")));
        break;
    }

    Disconnect();
}


/*====================
  CChatManager::HandleChannelAuthEnabled
  ====================*/
void    CChatManager::HandleChannelAuthEnabled(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_enabled")), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelAuthDisabled
  ====================*/
void    CChatManager::HandleChannelAuthDisabled(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_disabled")), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelAddAuthUser
  ====================*/
void    CChatManager::HandleChannelAddAuthUser(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_add_success"), _T("name"), sName), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelAddAuthUserFailed
  ====================*/
void    CChatManager::HandleChannelAddAuthUserFailed(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_add_failure"), _T("name"), sName), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelRemoveAuthUser
  ====================*/
void    CChatManager::HandleChannelRemoveAuthUser(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_remove_success"), _T("name"), sName), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelRemoveAuthUserFailed
  ====================*/
void    CChatManager::HandleChannelRemoveAuthUserFailed(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_auth_remove_failure"), _T("name"), sName), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelListAuth
  ====================*/
void    CChatManager::HandleChannelListAuth(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    uint uiNumUsers(pkt.ReadInt());

    if (uiNumUsers == 0)
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_list_none")), GetChannelName(uiChannelID));
    else
    {
        for (uint i(0); i < uiNumUsers; ++i)
        {
            if (pkt.HasFaults())
                break;

            tstring sName(pkt.ReadWStringAsTString());
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_list_entry"), _T("name"), sName), GetChannelName(uiChannelID));
        }
    }
}


/*====================
  CChatManager::HandleChannelSetPassword
  ====================*/
void    CChatManager::HandleChannelSetPassword(CPacket &pkt)
{
    uint uiChannelID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_password_change"), _T("name"), sName), GetChannelName(uiChannelID));
}


/*====================
  CChatManager::HandleChannelJoinPassword
  ====================*/
void    CChatManager::HandleChannelJoinPassword(CPacket &pkt)
{
    tstring sChannelName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_password_required"), _T("name"), sChannelName));

    ChatPasswordRequired.Trigger(sChannelName);
}


/*====================
  CChatManager::HandleClanInvite
  ====================*/
void    CChatManager::HandleClanInvite(CPacket &pkt)
{
    const tstring sName(pkt.ReadWStringAsTString());
    const tstring sClan(pkt.ReadWStringAsTString());

    if (IsIgnored(sName))
    {
        CPacket pktReject;
        pktReject << CHAT_CMD_CLAN_ADD_REJECTED;
        m_sockChat.SendPacket(pktReject);
        return;
    }

    static tsvector vMiniParams(2);
    vMiniParams[0] = sName;
    vMiniParams[1] = sClan;

    ChatClanInvite.Trigger(vMiniParams);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_received"), _T("name"), sName, _T("clan"), sClan));
}


/*====================
  CChatManager::HandleClanInviteRejected
  ====================*/
void    CChatManager::HandleClanInviteRejected(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_rejected"), _T("name"), sName));
}


/*====================
  CChatManager::HandleClanInviteFailedOnline
  ====================*/
void    CChatManager::HandleClanInviteFailedOnline(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_online")));
}


/*====================
  CChatManager::HandleClanInviteFailedClan
  ====================*/
void    CChatManager::HandleClanInviteFailedClan(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_clan")));
}


/*====================
  CChatManager::HandleClanInviteFailedInvite
  ====================*/
void    CChatManager::HandleClanInviteFailedInvite(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_invite")));
}


/*====================
  CChatManager::HandleClanInviteFailedPermissions
  ====================*/
void    CChatManager::HandleClanInviteFailedPermissions(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_permissions")));
}


/*====================
  CChatManager::HandleClanInviteRejected
  ====================*/
void    CChatManager::HandleClanInviteFailedUnknown(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());

    if (!CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_unknown"), _T("name"), sName));
    else
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_failed_unknown_self")));
}


/*====================
  CChatManager::HandleClanCreateFailedClan
  ====================*/
void    CChatManager::HandleClanCreateFailedClan(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_clan"), _T("name"), sName));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_clan"), _T("name"), sName));
}


/*====================
  CChatManager::HandleClanCreateFailedInvite
  ====================*/
void    CChatManager::HandleClanCreateFailedInvite(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_invite"), _T("name"), sName));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_invite"), _T("name"), sName));
}


/*====================
  CChatManager::HandleClanCreateFailedNotFound
  ====================*/
void    CChatManager::HandleClanCreateFailedNotFound(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_not_found"), _T("name"), sName));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_not_found"), _T("name"), sName));
}


/*====================
  CChatManager::HandleClanCreateFailedDuplicate
  ====================*/
void    CChatManager::HandleClanCreateFailedDuplicate(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_dupliate")));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_dupliate")));
}


/*====================
  CChatManager::HandleClanCreateFailedParam
  ====================*/
void    CChatManager::HandleClanCreateFailedParam(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_param")));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_param")));
}


/*====================
  CChatManager::HandleClanCreateFailedClanName
  ====================*/
void    CChatManager::HandleClanCreateFailedClanName(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_clan_name")));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_clan_name")));
}


/*====================
  CChatManager::HandleClanCreateFailedTag
  ====================*/
void    CChatManager::HandleClanCreateFailedTag(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_tag")));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_tag")));
}


/*====================
  CChatManager::HandleClanCreateFailedUnknown
  ====================*/
void    CChatManager::HandleClanCreateFailedUnknown(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_unknown")));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_unknown")));
}


/*====================
  CChatManager::HandleNameChange
  ====================*/
void    CChatManager::HandleNameChange(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    tstring sName(pkt.ReadWStringAsTString());

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return;

    if (uiAccountID != m_uiAccountID)
    {
        if (IsBuddy(uiAccountID))
            RefreshBuddyList();

        if (IsClanMember(uiAccountID))
            RefreshClanList();

        static tsvector vParams(12);
        static tsvector vMiniParams(2);

        for (uiset_it itChan(it->second.setChannels.begin()); itChan != it->second.setChannels.end(); ++itChan)
        {
            if (m_setChannelsIn.find(*itChan) == m_setChannelsIn.end())
                continue;

            // These stay the same throughout the rest of the function
            vParams[0] = vMiniParams[0] = GetChannelName(*itChan);

            vMiniParams[1] = _T("EraseListItemByValue('") + it->second.sName + _T("');");
            ChatUserEvent.Trigger(vMiniParams);

            if (it->second.yStatus > CHAT_STATUS_DISCONNECTED)
            {
                vParams[1] = sName;
                vParams[2] = XtoA(GetAdminLevel(*itChan, it->first));
                vParams[3] = XtoA(it->second.yStatus > CHAT_STATUS_CONNECTED, true);
                vParams[4] = XtoA((it->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
                vParams[5] = XtoA(it->second.uiAccountID);
                vParams[6] = Host.GetChatSymbolTexturePath(it->second.uiChatSymbol);
                vParams[7] = Host.GetChatNameColorTexturePath(it->second.uiChatNameColor);
                vParams[8] = Host.GetChatNameColorString(it->second.uiChatNameColor);
                vParams[9] = Host.GetChatNameColorIngameString(it->second.uiChatNameColor);
                vParams[10] = Host.GetAccountIconTexturePath(it->second.uiAccountIcon);
                vParams[11] = XtoA(it->second.uiSortIndex);
                ChatUserNames.Trigger(vParams);
            }

            vMiniParams[1] = _T("SortListboxSortIndex();");
            ChatUserEvent.Trigger(vMiniParams);
        }

        it->second.sName = sName;
    }
    else
    {
        // always refresh this and not the buddy list because a player in a clan is always listed on the clan panel
        RefreshClanList();

/*      tsvector vParams(5);

        vParams[0] = TSNULL;
        vParams[1] = _T("EraseListItemByValue('") + it->second.sName + _T("');");

        ChatUserEvent.Trigger(vParams);

        for (uiset_it itChan(m_setChannelsIn.begin()); itChan != m_setChannelsIn.end(); ++itChan)
        {
            vParams[0] = GetChannelName(*itChan);
            vParams[1] = sName;
            vParams[2] = XtoA(GetAdminLevel(*itChan, it->first));
            vParams[3] = XtoA(it->second.yStatus > CHAT_STATUS_CONNECTED, true);
            vParams[4] = XtoA((it->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true)
            ChatUserNames.Trigger(vParams);
        }

        vParams[0] = TSNULL;
        vParams[1] = _T("SortListboxSortIndex();");
        ChatUserEvent.Trigger(vParams);*/

        it->second.sName = sName;

        UpdateChannels();
        
        CHostClient *pClient(Host.GetActiveClient());
        if (pClient != NULL)
            pClient->SetNickname(sName);
    }
}


/*====================
  CChatManager::HandleAutoMatchConnect
  ====================*/
void    CChatManager::HandleAutoMatchConnect(CPacket &pkt)
{
    const uint uiMatchupID(pkt.ReadInt());
    const tstring sAddress(pkt.ReadWStringAsTString());
    const ushort unPort(pkt.ReadShort());

    Console << _T("Received AutoMatchConnect for MatchupID#") << XtoA(uiMatchupID) << _T(" ") << XtoA(sAddress) << _T(":") << XtoA(unPort) << _T("...") << newl;

    if (sAddress.empty() || unPort == 0 || Host.IsConnected() || pkt.HasFaults())
        return;

    LeaveTMMGroup(true, _T("foundmatch"));

    TMMReset.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
    TMMJoinMatch.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

    Console << _T("Connecting to ") << XtoA(sAddress) << _T(":") << XtoA(unPort) << _T("...") << newl;

    UnFollow();

    Host.Connect(sAddress + _T(":") + XtoA(unPort), false, false, _T("loading_matchmaking_connecting"));
}


/*====================
  CChatManager::HandleServerNotIdle
  ====================*/
void    CChatManager::HandleServerNotIdle(CPacket &pkt)
{
    const uint uiMatchupID(pkt.ReadInt());
    const tstring sAddress(pkt.ReadWStringAsTString());
    const ushort unPort(pkt.ReadShort());

    Console << _T("Received ServerNotIdle for MatchupID#") << XtoA(uiMatchupID) << _T(" ") << XtoA(sAddress) << _T(":") << XtoA(unPort) << _T("...") << newl;

    if (sAddress.empty() || unPort == 0 || Host.IsConnected() || pkt.HasFaults())
        return;

    LeaveTMMGroup(true, _T("servernotidle"));

    TMMReset.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
    TMMServerNotIdle.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
}


/*====================
  CChatManager::HandleTournMatchReady
  ====================*/
void    CChatManager::HandleTournMatchReady(CPacket &pkt)
{
    tstring sAddress(pkt.ReadWStringAsTString());
    ushort unPort(pkt.ReadShort());
    uint uiTournMatchID(pkt.ReadInt());
    tstring sMatchName(pkt.ReadWStringAsTString());

    if (pkt.HasFaults() || sAddress.empty() || unPort == 0 || uiTournMatchID == -1 || m_mapTournGameAddresses.find(uiTournMatchID) != m_mapTournGameAddresses.end())
        return;

    ChatManager.AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_tourn_match_ready"), _T("name"), sMatchName));

    m_mapTournGameAddresses.insert(pair<uint, tstring>(uiTournMatchID, sAddress + _T(":") + XtoA(unPort)));
}


/*====================
  CChatManager::HandleAutoMatchWaiting
  ====================*/
void    CChatManager::HandleAutoMatchWaiting(CPacket &pkt)
{
    if (pkt.HasFaults())
        return;
}


/*====================
  CChatManager::HandleChatRoll
  ====================*/
void    CChatManager::HandleChatRoll(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    uint uiChannelID(pkt.ReadInt());
    tstring sMessage(pkt.ReadWStringAsTString());

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end() || sMessage.empty())
        return;

    if (IsIgnored(it->first))
        return;

    AddIRCChatMessage(CHAT_MESSAGE_ROLL, sMessage, GetChannelName(uiChannelID), true);
}


/*====================
  CChatManager::HandleChatEmote
  ====================*/
void    CChatManager::HandleChatEmote(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    uint uiChannelID(pkt.ReadInt());
    tstring sMessage(pkt.ReadWStringAsTString());

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end() || sMessage.empty())
        return;

    if (IsIgnored(it->first))
        return;

    AddIRCChatMessage(CHAT_MESSAGE_EMOTE, sMessage, GetChannelName(uiChannelID), true);
}


/*====================
  CChatManager::HandleSetChatModeType
  ====================*/
void    CChatManager::HandleSetChatModeType(CPacket &pkt)
{
    uint uiChatModeType(pkt.ReadInt());
    tstring sReason(pkt.ReadWStringAsTString());
    
    m_uiChatModeType = uiChatModeType;
    
    switch (uiChatModeType)
    {
        case CHAT_MODE_AVAILABLE:
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(sReason));
            break;
            
        case CHAT_MODE_AFK:
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_afk_message"), _T("reason"), sReason));
            break;
            
        case CHAT_MODE_DND:             
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_dnd_message"), _T("reason"), sReason));
            break;
            
        case CHAT_MODE_INVISIBLE:       
            break;
        
        default:
            break;  
    }
}


/*====================
  CChatManager::HandleChatModeAutoResponse
  ====================*/
void    CChatManager::HandleChatModeAutoResponse(CPacket &pkt)
{
    uint uiChatModeType(pkt.ReadInt());
    tstring sTargetName(pkt.ReadWStringAsTString());
    tstring sMessage(pkt.ReadWStringAsTString());

    switch (uiChatModeType)
    {
        case CHAT_MODE_AFK:
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_mode_afk_auto_response"), _T("target"), sTargetName, _T("message"), sMessage), TSNULL, true);
            break;
        
        case CHAT_MODE_DND:
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_mode_dnd_auto_response"), _T("target"), sTargetName, _T("message"), sMessage), TSNULL, true);
            break;      
            
        default:
            break;  
    }   
}


/*====================
  CChatManager::HandleUserCount
  ====================*/
void    CChatManager::HandleUserCount(CPacket &pkt)
{
    uint uiUserCount(pkt.ReadInt());
    ChatUsersOnline.Trigger(XtoA(uiUserCount));
}


/*====================
  CChatManager::UpdateReadyStatus
  ====================*/
void    CChatManager::UpdateReadyStatus()
{
    tsvector vReadyParams(5);
    vReadyParams[0] = XtoA(m_uiAccountID == m_uiTMMGroupLeaderID);
    vReadyParams[1] = XtoA(m_bTMMOtherPlayersReady);
    vReadyParams[2] = XtoA(m_bTMMAllPlayersReady);
    vReadyParams[3] = XtoA(m_aGroupInfo[m_uiTMMSelfGroupIndex].yReadyStatus > 0);
    vReadyParams[4] = XtoA(m_uiTMMStartTime != INVALID_TIME);

    TMMReadyStatus.Trigger(vReadyParams, cc_forceTMMInterfaceUpdate);
}


/*====================
  CChatManager::HandleTMMPlayerUpdates
  ====================*/
void    CChatManager::HandleTMMPlayerUpdates(CPacket &pkt)
{
    uint uiStartTime(K2System.Microseconds());

    // This handles new groups being created, and players getting kicked/leaving/joining from the groups because once the group 
    // changes another update would need to be sent anyways.  It is designed to be stateless so any update will always provide 
    // all the information required so we can avoid synchronization complications
    byte yUpdateType(pkt.ReadByte());
    uint uiAccountID(pkt.ReadInt());
    byte yGroupSize(pkt.ReadByte());
    ushort unAverageTMR(pkt.ReadShort());
    uint uiGroupLeaderAccountID(pkt.ReadInt());
    byte yGameType(pkt.ReadByte());
    tstring sMapNames(pkt.ReadTString());
    tstring sGameModes(pkt.ReadTString());
    tstring sRegions(pkt.ReadTString());
    tstring sPlayerInvitationResponses(pkt.ReadTString());
    byte yTeamSize(pkt.ReadByte());

    if (yGroupSize > MAX_GROUP_SIZE)
        return;
        
    for (uint ui(0); ui < MAX_GROUP_SIZE; ++ui)
        m_aGroupInfo[ui].Clear();
    
    for (uint i(0); i < yGroupSize; ++i)
    {
        m_aGroupInfo[i].uiAccountID = pkt.ReadInt();
        m_aGroupInfo[i].sName = pkt.ReadTString();
        m_aGroupInfo[i].ySlot = pkt.ReadByte();
        m_aGroupInfo[i].nRating = pkt.ReadShort();
        m_aGroupInfo[i].yLoadingPercent = pkt.ReadByte();
        m_aGroupInfo[i].yReadyStatus = pkt.ReadByte();

        // If someone is leaving or being kicked from the group
        if (yUpdateType == TMM_PLAYER_LEFT_GROUP || yUpdateType == TMM_PLAYER_KICKED_FROM_GROUP)
        {
            // Don't display their information in the update as they aren't there anymore
            if (uiAccountID == m_aGroupInfo[i].uiAccountID)
                m_aGroupInfo[i].Clear();
        }
    }

    if (pkt.HasFaults())
        return;

    m_uiTMMGroupLeaderID = uiGroupLeaderAccountID;
    m_uiTMMSelfGroupIndex = 0;
    m_bTMMOtherPlayersReady = true;
    m_bTMMAllPlayersReady = true;

    for (uint i(0); i < yGroupSize; ++i)
    {
        if (m_aGroupInfo[i].uiAccountID == INVALID_INDEX)
            continue;

        if (m_aGroupInfo[i].uiAccountID == m_uiAccountID)
            m_uiTMMSelfGroupIndex = i;

        if (m_aGroupInfo[i].yReadyStatus != 1)
        {
            if (m_aGroupInfo[i].uiAccountID != m_uiTMMGroupLeaderID)
                m_bTMMOtherPlayersReady = false;
            m_bTMMAllPlayersReady = false;
        }
    }

    tsvector vParams(36);

    uint uiIndex(0);
    for (uint i(0); i < yGroupSize; ++i)
    {
        const uint uiSlotAccountID(m_aGroupInfo[i].uiAccountID);

        vParams[uiIndex++] = XtoA(uiSlotAccountID);             // Slot Account ID
        vParams[uiIndex++] = m_aGroupInfo[i].sName;             // Slot Username
        vParams[uiIndex++] = XtoA(m_aGroupInfo[i].ySlot);       // Slot number
        vParams[uiIndex++] = XtoA(m_aGroupInfo[i].nRating);     // Slot TMR

        const byte yLoadingPercent(m_aGroupInfo[i].yLoadingPercent);
        const byte yReadyStatus(m_aGroupInfo[i].yReadyStatus);

        vParams[uiIndex++] = XtoA(XtoA(yLoadingPercent) + _T("|") + XtoA(yReadyStatus));       // Player Loading TMM Status | Player Ready Status

        // If someone is leaving or being kicked from the group
        if (yUpdateType == TMM_PLAYER_LEFT_GROUP || yUpdateType == TMM_PLAYER_KICKED_FROM_GROUP)
        {
            // Don't display their information in the update as they aren't there anymore
            if (uiAccountID == uiSlotAccountID)
            {
                vParams[uiIndex - 5] = TSNULL;
                vParams[uiIndex - 4] = TSNULL;
                vParams[uiIndex - 3] = TSNULL;
                vParams[uiIndex - 2] = TSNULL;
                vParams[uiIndex - 1] = TSNULL;
            }
        }
    }

    vParams[25] = XtoA(yUpdateType);
    vParams[26] = XtoA(yGroupSize);
    vParams[27] = XtoA(unAverageTMR);
    vParams[28] = XtoA(uiGroupLeaderAccountID);
    vParams[29] = XtoA(yGameType);
    vParams[30] = sMapNames;
    vParams[31] = sGameModes;
    vParams[32] = sRegions;
    vParams[33] = TSNULL;
    vParams[34] = sPlayerInvitationResponses;
    vParams[35] = XtoA(yTeamSize);

    bool bTriggerReset(false);

    if (yUpdateType == TMM_CREATE_TEAM_GROUP)
    {
        Console << _T("Created team TMM group...") << newl;

        m_bInGroup = true;
    }
    else if (yUpdateType == TMM_GROUP_UPDATE)
    {
        Console << _T("Received TMM group update...") << newl;

        m_bInGroup = true;
    }
    else if (yUpdateType == TMM_PLAYER_JOINED_GROUP)
    {
        Console << _T("AccountID ") << uiAccountID << _T(" connected to the TMM group...") << newl;
        
        m_bInGroup = true;

        if (uiAccountID == GetAccountID())
        {
            UnFollow();

            TMMJoinGroup.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
            Console << _T("You (") << uiAccountID << _T(") joined the TMM group...") << newl;
        }
    }
    else if (yUpdateType == TMM_PLAYER_FINISHED_LOADING)
    {
        Console << _T("AccountID ") << uiAccountID << _T(" finished loading into the TMM group...") << newl;
        
        m_bInGroup = true;
    }
    else if (yUpdateType == TMM_PLAYER_LEFT_GROUP)
    {
        Console << _T("AccountID ") << uiAccountID << _T(" left the TMM group...") << newl;
        
        if (uiAccountID == uiGroupLeaderAccountID)
        {
            LeaveTMMGroup(true, _T("disbanded"));
            Console << _T("The group was disbanded by the group leader ") << uiAccountID << newl;
        }
        
        if (uiAccountID == GetAccountID())
        {
            LeaveTMMGroup(true, _T("left"));
            Console << _T("You (") << uiAccountID << _T(") left the TMM group...") << newl;
            bTriggerReset = true;
        }
    }
    else if (yUpdateType == TMM_PLAYER_KICKED_FROM_GROUP)
    {
        Console << _T("AccountID ") << uiAccountID << _T(" was kicked from TMM group...") << newl;
        
        if (uiAccountID == GetAccountID())
        {
            LeaveTMMGroup(true, _T("kicked"));
            Console << _T("You (") << uiAccountID << _T(") were kicked from the TMM group...") << newl;
        }
    }
    
    if (m_bInGroup)
        TMMDisplay.Trigger(vParams, cc_forceTMMInterfaceUpdate);
    
    if (bTriggerReset)
    {
        TMMReset.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
    }

    UpdateReadyStatus();

    for (uint ui(0); ui < MAX_GROUP_SIZE; ++ui)
    {
        tsvector vPlayerParams(8);

        if (m_aGroupInfo[ui].uiAccountID != INVALID_INDEX)
        {
            vPlayerParams[0] = XtoA(m_aGroupInfo[ui].uiAccountID);
            vPlayerParams[1] = XtoA(m_aGroupInfo[ui].sName);
            vPlayerParams[2] = XtoA(m_aGroupInfo[ui].ySlot);
            vPlayerParams[3] = XtoA(m_aGroupInfo[ui].nRating);
            vPlayerParams[4] = XtoA(m_aGroupInfo[ui].yLoadingPercent);
            vPlayerParams[5] = XtoA(m_aGroupInfo[ui].yReadyStatus);
            vPlayerParams[6] = XtoA(m_aGroupInfo[ui].uiAccountID == m_uiTMMGroupLeaderID);
            vPlayerParams[7] = XtoA(true);
        }
        else
        {
            vPlayerParams[7] = XtoA(ui < yTeamSize);
        }

        TMMPlayerStatus[ui]->Trigger(vPlayerParams, cc_forceTMMInterfaceUpdate);
    }

    if (m_bTMMAllPlayersReady)
    {
        if (!m_bTMMMapLoaded)
        {
            m_bTMMMapLoaded = true;

            Host.PreloadWorld(sMapNames);
        }
    }
    else
    {
        m_bTMMMapLoaded = false;
    }

    if (chat_profile)
        Console << _T("HandleTMMPlayerUpdates - ") << K2System.Microseconds() - uiStartTime << _T(" us") << newl;
}


/*====================
  CChatManager::HandleTMMPopularityUpdates
  ====================*/
void    CChatManager::HandleTMMPopularityUpdates(CPacket &pkt)
{
    const byte yTMMEnabled(pkt.ReadByte());
    const tstring sAvailableMapNames(pkt.ReadStringAsTString());
    const tstring sAvailableGameTypes(pkt.ReadStringAsTString());
    const tstring sAvailableGameModes(pkt.ReadStringAsTString());
    const tstring sRegions(pkt.ReadStringAsTString());

    const byte yNormalGameType(pkt.ReadByte());
    const byte yCasualGameType(pkt.ReadByte());
    const byte yNormal(pkt.ReadByte());
    const byte ySingleDraft(pkt.ReadByte());
    const byte yBanningDraft(pkt.ReadByte());
    const byte yBanningPick(pkt.ReadByte());
    const byte yAllRandom(pkt.ReadByte());
    const byte yUSE(pkt.ReadByte());
    const byte yUSW(pkt.ReadByte());
    const byte yEU(pkt.ReadByte());

    if (pkt.HasFaults())
        return;

    static tsvector vParams(10);

    vParams[0] = XtoA(yNormalGameType);
    vParams[1] = XtoA(yCasualGameType);
    vParams[2] = XtoA(yNormal);
    vParams[3] = XtoA(ySingleDraft);
    vParams[4] = XtoA(yBanningDraft);
    vParams[5] = XtoA(yBanningPick);
    vParams[6] = XtoA(yAllRandom);
    vParams[7] = XtoA(yUSE);
    vParams[8] = XtoA(yUSW);
    vParams[9] = XtoA(yEU);

    TMMDisplayPopularity.Trigger(vParams, cc_forceTMMInterfaceUpdate);

    static tsvector vOptionsAvailableParams(5);

    vOptionsAvailableParams[0] = sAvailableGameTypes;
    vOptionsAvailableParams[1] = sAvailableMapNames;
    vOptionsAvailableParams[2] = sAvailableGameModes;
    vOptionsAvailableParams[3] = sRegions;
    vOptionsAvailableParams[4] = XtoA(yTMMEnabled);
    
    TMMOptionsAvailable.Trigger(vOptionsAvailableParams, cc_forceTMMInterfaceUpdate);

    if (yTMMEnabled)
        m_bTMMEnabled = true;
    else
        m_bTMMEnabled = false;
    
    TMMAvailable.Trigger(XtoA(m_bTMMEnabled), cc_forceTMMInterfaceUpdate);
}


/*====================
  CChatManager::HandleTMMQueueUpdates
  ====================*/
void    CChatManager::HandleTMMQueueUpdates(CPacket &pkt)
{
    const byte yUpdateType(pkt.ReadByte());
    
    if (yUpdateType == TMM_GROUP_QUEUE_UPDATE)
    {
        const uint uiAverageTimeQueued(pkt.ReadInt());
        const uint uiStdDev(pkt.ReadInt());

        if (pkt.HasFaults())
            return;

        m_uiTMMAverageQueueTime = uiAverageTimeQueued;
        m_uiTMMStdDevQueueTime = uiStdDev;
    }
    else if (yUpdateType == TMM_GROUP_FOUND_SERVER)
    {
        TMMFoundServer.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

        Console << _T("Server found, waiting for response") << newl;
    }
    else if (yUpdateType == TMM_GROUP_NO_MATCHES_FOUND)
    {
        TMMNoMatchesFound.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
    }   
    else if (yUpdateType == TMM_GROUP_NO_SERVERS_FOUND)
    {
        TMMNoServersFound.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);
    }
    else if (yUpdateType == TMM_MATCHMAKING_DISABLED)
    {
        LeaveTMMGroup(true, _T("disabled"));
    }
}


/*====================
  CChatManager::HandleTMMJoinQueue
  ====================*/
void    CChatManager::HandleTMMJoinQueue(CPacket &pkt)
{
    m_uiTMMStartTime = Host.GetTime();

    Console << _T("Your group joined the TMM queue...") << newl;

    TMMJoinQueue.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

    UpdateReadyStatus();
}


/*====================
  CChatManager::HandleTMMLeaveQueue
  ====================*/
void    CChatManager::HandleTMMLeaveQueue(CPacket &pkt)
{
    m_uiTMMStartTime = INVALID_TIME;
    m_uiTMMAverageQueueTime = INVALID_TIME;
    m_uiTMMStdDevQueueTime = INVALID_TIME;

    static tsvector vMiniParams(3);

    vMiniParams[0] = XtoA(0);
    vMiniParams[1] = XtoA(0);
    vMiniParams[2] = XtoA(0);

    TMMTime.Trigger(vMiniParams, cc_forceTMMInterfaceUpdate);   

    Console << _T("Your group left the TMM queue...") << newl;

    TMMLeaveQueue.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

    UpdateReadyStatus();
}


/*====================
  CChatManager::HandleTMMInviteToGroup
  ====================*/
void    CChatManager::HandleTMMInviteToGroup(CPacket &pkt)
{
    const tstring sInviter(pkt.ReadWStringAsTString());
    const tstring sMapName(pkt.ReadWStringAsTString());
    const byte yGameType(pkt.ReadByte());
    const tstring sGameModes(pkt.ReadWStringAsTString());
    const tstring sRegions(pkt.ReadWStringAsTString());
    
    if (pkt.HasFaults())
        return;
    
    if (IsIgnored(sInviter))
        return;

    static tsvector vInvite(4);
    vInvite[0] = sMapName;
    vInvite[1] = XtoA(yGameType);
    vInvite[2] = sGameModes;
    vInvite[3] = sRegions;
    
    Console << _T("You were invited to join the TMM group by ") << sInviter << newl;
    
    PushNotification(NOTIFY_TYPE_TMM_GROUP_INVITE, sInviter, TSNULL, TSNULL, vInvite);
}


/*====================
  CChatManager::HandleTMMInviteToGroupBroadcast
  ====================*/
void    CChatManager::HandleTMMInviteToGroupBroadcast(CPacket &pkt)
{
    const tstring sInvited(pkt.ReadWStringAsTString());
    const tstring sInviter(pkt.ReadWStringAsTString());
    
    if (pkt.HasFaults())
        return;
    
    Console << sInvited << _T(" was invited to join the TMM group by ") << sInviter << _T("...") << newl;
}


/*====================
  CChatManager::HandleTMMRejectInvite
  ====================*/
void    CChatManager::HandleTMMRejectInvite(CPacket &pkt)
{
    const tstring sInvited(pkt.ReadWStringAsTString());
    const tstring sInviter(pkt.ReadWStringAsTString());
    
    if (pkt.HasFaults())
        return;
    
    Console << sInvited << _T(" rejected the TMM group invite from ") << sInviter << _T("...") << newl;
}


/*====================
  CChatManager::HandleTMMMatchFound
  ====================*/
void    CChatManager::HandleTMMMatchFound(CPacket &pkt)
{
    const tstring sMapName(pkt.ReadWStringAsTString());
    const byte yTeamSize(pkt.ReadByte());
    const byte yGameType(pkt.ReadByte());
    const tstring sGameMode(pkt.ReadWStringAsTString());
    const tstring sRegion(pkt.ReadWStringAsTString());
    tstring sExtraMatchInfo(pkt.ReadWStringAsTString());
    
    if (pkt.HasFaults())
        return;

    TMMFoundMatch.Trigger(TSNULL, cc_forceTMMInterfaceUpdate);

    tstring sOtherMatchInfo;
    sOtherMatchInfo = _T("Game Type:") + XtoA(yGameType);
    sOtherMatchInfo += _T("|Map Name:") + sMapName;
    sOtherMatchInfo += _T("|Team Size:") + XtoA(yTeamSize);
    sOtherMatchInfo += _T("|Game Mode:") + sGameMode;
    sOtherMatchInfo += _T("|Region:") + sRegion + _T("|");
    
    sExtraMatchInfo = sOtherMatchInfo + sExtraMatchInfo;

    TMMDebugInfo.Trigger(sExtraMatchInfo, cc_forceTMMInterfaceUpdate);
    
    // Just add in some newlines so the console appears properly
    sExtraMatchInfo = StringReplace(sExtraMatchInfo, _T("Team1:"), _T("\nTeam1:"));
    sExtraMatchInfo = StringReplace(sExtraMatchInfo, _T("Team2:"), _T("\nTeam2:"));
    sExtraMatchInfo = StringReplace(sExtraMatchInfo, _T("Group Count:"), _T("\nGroup Count:"));
    sExtraMatchInfo = StringReplace(sExtraMatchInfo, _T("Average Matchup %:"), _T("\nAverage Matchup %:"));
    sExtraMatchInfo = StringReplace(sExtraMatchInfo, _T("Group Mismatch %:"), _T("\nGroup Mismatch %:"));

    Console << _T("Your TMM group left the queue and was placed into a match!!") << newl;
    Console << _T("Game Type: ") << yGameType << newl;
    Console << _T("Map Name: ") << sMapName << _T(" Team Size:") << yTeamSize << newl;
    Console << _T("Game Mode: ") << sGameMode << newl;
    Console << _T("Region: ") << sRegion << newl;
    Console << _T("Extra Match Info: ") << sExtraMatchInfo << newl;
}


/*====================
  CChatManager::HandleTMMJoinFailed
  ====================*/
void    CChatManager::HandleTMMJoinFailed(CPacket &pkt)
{
    const byte yUpdateType(pkt.ReadByte());
    
    if (pkt.HasFaults())
        return;

    tstring sReason;
    
    if (yUpdateType == 0)
    {
        sReason = _T("isleaver");
        Console << _T("Players who are leavers are not allowed in TMM games...") << newl;
    }
    else if (yUpdateType == 1)
    {
        sReason = _T("disabled");
        Console << _T("TMM is currently disabled, please try back at a later time.") << newl;
    }
    else if (yUpdateType == 2)
    {
        sReason = _T("busy");
        Console << _T("TMM is currently too busy to accept new groups, please try again in a few minutes.") << newl;
    }
    else if (yUpdateType == 3)
    {
        sReason = _T("optionunavailable");
        Console << _T("You have tried to create a group using options that are disabled or invalid, please try again.") << newl;
    }
    else if (yUpdateType == 4)
    {
        sReason = _T("invalidversion");
        Console << _T("The version of your client is not compatible with the matchmaking server.  Please update your client and try again.") << newl;
    }
    else if (yUpdateType == 5)
    {
        sReason = _T("groupfull");
        Console << _T("Unable to join, the group you are trying to join is full.") << newl;
    }
    else if (yUpdateType == 6)
    {
        sReason = _T("badstats");
        Console << _T("Unable to join, invalid stats.") << newl;
    }
    else if (yUpdateType == 7)
    {
        sReason = _T("groupqueued");
        Console << _T("Unable to join, the group has already entered the queue.") << newl;
    }
    else
    {
        sReason = _T("unknown");
        Console << _T("Unable to join, unknown reason.") << newl;
    }

    LeaveTMMGroup(true, sReason);
}


/*====================
  CChatManager::HandleRequestBuddyAddResponse
  ====================*/
void    CChatManager::HandleRequestBuddyAddResponse(CPacket &pkt)
{
    // Rather than having two separate methods for the requester and the requested,
    // these vars serve both purposes, so don't get confused
    const byte yType(pkt.ReadByte());
    const uint uiNotifyID(pkt.ReadInt());
    const tstring sAccountNickName(UTF8ToTString(pkt.ReadString()));

    if (pkt.HasFaults())
        return;
        
    if (yType == 1)
    {
        // This requester/adder is getting this message
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requested_approval_adder"), _T("name"), sAccountNickName));
        if (cc_showBuddyRequestNotification)
            PushNotification(NOTIFY_TYPE_BUDDY_REQUESTED_ADDER, sAccountNickName, TSNULL, TSNULL, VSNULL, uiNotifyID);
    }
    else if (yType == 2)
    {
        // The requested/added is getting this message
        if (cc_showBuddyRequestNotification)
        {
            PushNotification(NOTIFY_TYPE_BUDDY_REQUESTED_ADDED, sAccountNickName, TSNULL, TSNULL, VSNULL, uiNotifyID);      
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requested_approval_added"), _T("name"), sAccountNickName));
        }
    }
    else if (yType == 3)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requested_approval_duplicate"), _T("name"), sAccountNickName));
    }   
    else if (yType == 4)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requested_approval_ignored"), _T("name"), sAccountNickName));
    }   
    else if (yType == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requested_approval_failed"), _T("name"), sAccountNickName));
    }   
}


/*====================
  CChatManager::HandleRequestBuddyApproveResponse
  ====================*/
void    CChatManager::HandleRequestBuddyApproveResponse(CPacket &pkt)
{
    // rather than having two separate methods for the approver and the approved,
    // these vars serve both purposes, so don't get confused
    const byte yType(pkt.ReadByte());
    const uint uiAccountID(pkt.ReadInt());
    const uint uiNotifyID(pkt.ReadInt());
    const tstring sAccountNickName(UTF8ToTString(pkt.ReadString()));
    
    if (pkt.HasFaults())
        return;
        
    if (yType == 1)
    {
        // This requester/adder is getting this message
        AddBuddy(uiAccountID, sAccountNickName);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_approved_buddy_adder"), _T("name"), sAccountNickName));
        if (cc_showBuddyAddNotification)
            PushNotification(NOTIFY_TYPE_BUDDY_ADDER, sAccountNickName, TSNULL, TSNULL, VSNULL, uiNotifyID);
    }
    else if (yType == 2)
    {
        // The requested/added is getting this message
        AddBuddy(uiAccountID, sAccountNickName);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_approved_buddy_added"), _T("name"), sAccountNickName));
        if (cc_showBuddyAddNotification)
            PushNotification(NOTIFY_TYPE_BUDDY_ADDED, sAccountNickName, TSNULL, TSNULL, VSNULL, uiNotifyID);
    }
    else if (yType == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_approving_buddy"), _T("name"), sAccountNickName));
    }
}


/*====================
  CChatManager::HandleClanCreateAccept
  ====================*/
void    CChatManager::HandleClanCreateAccept(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_accept"), _T("name"), sName));
    ChatClanCreateAccept.Trigger(sName);
}


/*====================
  CChatManager::HandleClanCreateComplete
  ====================*/
void    CChatManager::HandleClanCreateComplete(CPacket &pkt)
{
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_success")));
    ChatClanCreateSuccess.Trigger(Translate(_T("chat_clan_create_result_success")));

    m_uiCreateTimeSent = INVALID_TIME;
}


/*====================
  CChatManager::HandleClanCreateRejected
  ====================*/
void    CChatManager::HandleClanCreateRejected(CPacket &pkt)
{
    tstring sName(pkt.ReadWStringAsTString());
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_reject"), _T("name"), sName));
    ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_reject"), _T("name"), sName));
}


/*====================
  CChatManager::HandleNewClanMember
  ====================*/
void    CChatManager::HandleNewClanMember(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return;

    if (uiAccountID != m_uiAccountID)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_new_user"), _T("name"), it->second.sName));
        if (cc_showClanAddNotification)
            PushNotification(NOTIFY_TYPE_CLAN_ADD, it->second.sName);

        AddClanMember(uiAccountID, it->second.sName);       
    }
    else
    {
        it->second.iClanID = pkt.ReadInt();
        it->second.sClan = pkt.ReadWStringAsTString();
        it->second.sClanTag = pkt.ReadWStringAsTString();

        m_setClanList.insert(uiAccountID);

        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("clan_list"));
        pHTTPRequest->AddVariable(_T("clan_id"), it->second.iClanID);
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_UPDATE_CLAN, 0, it->second.sClanTag));
        m_lHTTPRequests.push_back(pNewRequest);

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_new_user_self"), _T("clan"), it->second.sClan));
        if (cc_showClanAddNotification)
            PushNotification(NOTIFY_TYPE_CLAN_ADD, RemoveClanTag(it->second.sName));
        
        // update the displayed login username on systembar, note clan tags are removed from name because the first time data is returned
        // there are no tags in the name, the next time there are though, this ensures whether its the 1st or 2nd time changing the name 
        // it will appear correctly.
        ChatUpdateName.Trigger(_T("[") + it->second.sClanTag + _T("]") + RemoveClanTag(it->second.sName));
    }
}


/*====================
  CChatManager::HandleClanRankChanged
  ====================*/
void    CChatManager::HandleClanRankChanged(CPacket &pkt)
{
    uint uiAccountID(pkt.ReadInt());
    byte yRank(pkt.ReadByte());
    uint uiChangerID(pkt.ReadInt());

    if (yRank >= NUM_CLAN_RANKS)
        return;

    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return;

    ChatClientMap_it changeit(m_mapUserList.find(uiChangerID));

    if (changeit == m_mapUserList.end())
        return;

    if (yRank == CLAN_RANK_NONE && uiAccountID == uiChangerID)
    {
        // User removed
        if (uiAccountID != m_uiAccountID)
        {
            // Another player in the clan other than the one leaving it sees this message
            if (m_setClanList.find(uiAccountID) != m_setClanList.end())
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_left"), _T("name"), RemoveClanTag(it->second.sName)));
                if (cc_showClanRemoveNotification)
                    PushNotification(NOTIFY_TYPE_CLAN_REMOVE, RemoveClanTag(it->second.sName));
                RemoveClanMember(uiAccountID);
            }
        }
        else
        {
            // We left the clan on our own
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_left_self")));
            if (cc_showClanRemoveNotification)
                PushNotification(NOTIFY_TYPE_CLAN_REMOVE, RemoveClanTag(it->second.sName));

            m_mapUserList[m_uiAccountID].iClanID = -1;
            m_mapUserList[m_uiAccountID].sClan = TSNULL;
            m_mapUserList[m_uiAccountID].sClanTag = TSNULL;
            
            // update the displayed login username on systembar
            ChatUpdateName.Trigger(RemoveClanTag(it->second.sName));
        }
    }
    else if (yRank == CLAN_RANK_NONE && uiAccountID != uiChangerID)
    {
        // User removed
        if (uiAccountID != m_uiAccountID)
        {
            // Members of the clan see this message when someone is kicked
            if (m_setClanList.find(uiAccountID) != m_setClanList.end())
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_kick"), _T("name"), RemoveClanTag(it->second.sName), _T("changer"), RemoveClanTag(changeit->second.sName)));
                RemoveClanMember(uiAccountID);
            }
            // keep this out here so the player that kicked the user gets the notification too
            if (cc_showClanRemoveNotification)
                PushNotification(NOTIFY_TYPE_CLAN_REMOVE, RemoveClanTag(it->second.sName), RemoveClanTag(changeit->second.sName));
            
            it->second.iClanID = -1;
            it->second.sClan = TSNULL;
            it->second.sClanTag = TSNULL;
        }
        else
        {
            // We were kicked from the clan by someone else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_kick_self"), _T("changer"), RemoveClanTag(changeit->second.sName)));
            if (cc_showClanRemoveNotification)          
                PushNotification(NOTIFY_TYPE_CLAN_REMOVE, RemoveClanTag(it->second.sName), RemoveClanTag(changeit->second.sName));
            
            // update the displayed login username on systembar
            ChatUpdateName.Trigger(RemoveClanTag(it->second.sName));        
        }
    }
    else
    {
        if (m_setClanList.find(uiAccountID) != m_setClanList.end())
        {
            if (uiAccountID != m_uiAccountID)
            { 
                // If an officer or leader demoted/promoted a member the demoted member and other clan members see this
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_rank_change"), _T("name"), RemoveClanTag(it->second.sName), _T("rank"), Translate(g_sClanRankNames[yRank]), _T("changer"), RemoveClanTag(changeit->second.sName)));
                if (cc_showClanRankNotification)                    
                    PushNotification(NOTIFY_TYPE_CLAN_RANK, RemoveClanTag(it->second.sName), RemoveClanTag(changeit->second.sName), Translate(g_sClanRankNames[yRank]));
            }
            else
            { 
                // If an officer or leader demoted/promoted a member the demoted member sees this
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_rank_change_self"), _T("rank"), Translate(g_sClanRankNames[yRank]), _T("changer"), RemoveClanTag(changeit->second.sName)));
                if (cc_showClanRankNotification)
                    PushNotification(NOTIFY_TYPE_CLAN_RANK, RemoveClanTag(it->second.sName), TSNULL, Translate(g_sClanRankNames[yRank]));
            }
        }
    }

    if (yRank == CLAN_RANK_NONE)
        it->second.yFlags &= ~(CHAT_CLIENT_IS_OFFICER | CHAT_CLIENT_IS_CLAN_LEADER);
    else if (yRank == CLAN_RANK_MEMBER)
        it->second.yFlags &= ~(CHAT_CLIENT_IS_OFFICER | CHAT_CLIENT_IS_CLAN_LEADER);
    else if (yRank == CLAN_RANK_OFFICER)
    {
        it->second.yFlags &= ~CHAT_CLIENT_IS_CLAN_LEADER;
        it->second.yFlags |= CHAT_CLIENT_IS_OFFICER;
    }
    else
    {
        it->second.yFlags &= ~CHAT_CLIENT_IS_OFFICER;
        it->second.yFlags |= CHAT_CLIENT_IS_CLAN_LEADER;
    }

    RefreshClanList();
}


/*====================
  CChatManager::CheckClanName
  ====================*/
void    CChatManager::CheckClanName(const tstring &sName, const tstring &sTag)
{
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("clan_nameCheck"));
    pHTTPRequest->AddVariable(_T("name"), sName);
    pHTTPRequest->AddVariable(_T("tag"), sTag);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_CHECK_CLAN_NAME, sName, sTag));
    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::AddBuddy
  ====================*/
void    CChatManager::AddBuddy(const uint uiAccountID, const tstring &sName, byte yFlags)
{
    ChatClientMap_it it = m_mapUserList.find(uiAccountID);
    byte yNewFlags = yFlags;

    if (it == m_mapUserList.end())
    {
        m_mapUserList.insert(ChatClientPair(uiAccountID, SChatClient()));
        it = m_mapUserList.find(uiAccountID);

        if (it == m_mapUserList.end())
            return;

        it->second.yStatus = CHAT_STATUS_DISCONNECTED;
        it->second.sServerAddressPort = TSNULL;
        it->second.sGameName = TSNULL;
        it->second.uiMatchID = -1;
    }
    else
    {
        yNewFlags |= it->second.yFlags;
    }

    it->second.sName = sName;
    it->second.uiAccountID = uiAccountID;
    it->second.yFlags = yNewFlags;

    if (m_setBuddyList.find(uiAccountID) == m_setBuddyList.end())
        m_setBuddyList.insert(uiAccountID);

    RefreshBuddyList();
}


/*====================
  CChatManager::AddClanMember
  ====================*/
void    CChatManager::AddClanMember(const uint uiAccountID, const tstring &sName, byte yFlags)
{
    ChatClientMap_it it = m_mapUserList.find(uiAccountID);
    byte yNewFlags = yFlags;

    if (it == m_mapUserList.end())
    {
        m_mapUserList.insert(ChatClientPair(uiAccountID, SChatClient()));
        it = m_mapUserList.find(uiAccountID);

        if (it == m_mapUserList.end())
            return;

        it->second.yStatus = CHAT_STATUS_DISCONNECTED;
        it->second.sServerAddressPort = TSNULL;
        it->second.sGameName = TSNULL;
        it->second.uiMatchID = -1;
    }
    else
    {
        yNewFlags |= it->second.yFlags;
    }

    it->second.sName = sName;
    it->second.uiAccountID = uiAccountID;
    it->second.yFlags = yNewFlags;
    it->second.sClan = m_mapUserList[m_uiAccountID].sClan;

    if (m_setClanList.find(uiAccountID) == m_setClanList.end())
        m_setClanList.insert(uiAccountID);

    RefreshClanList();
}


/*====================
  CChatManager::AddBan
  ====================*/
void    CChatManager::AddBan(const uint uiAccountID, const tstring &sName, const tstring &sReason)
{
    ChatBanMap_it it = m_mapBanList.find(uiAccountID);

    if (it == m_mapBanList.end())
    {
        m_mapBanList.insert(ChatBanPair(uiAccountID, SChatBanned()));
        it = m_mapBanList.find(uiAccountID);
    }

    if (it == m_mapBanList.end())
        return;

    it->second.sName = sName;
    it->second.uiAccountID = uiAccountID;
    it->second.sReason = sReason;
}


/*====================
  CChatManager::RemoveBuddy
  ====================*/
void    CChatManager::RemoveBuddy(const uint uiAccountID)
{
    uiset_it itBuddy(m_setBuddyList.find(uiAccountID));
    if (itBuddy == m_setBuddyList.end())
        return;

    m_setBuddyList.erase(itBuddy);
    RefreshBuddyList();
}


/*====================
  CChatManager::RemoveClanMember
  ====================*/
void    CChatManager::RemoveClanMember(uint uiAccountID)
{
    uiset_it findit(m_setClanList.find(uiAccountID));

    if (findit == m_setClanList.end())
        return;

    m_setClanList.erase(findit);
    
    m_mapUserList[uiAccountID].sClan = TSNULL;
    m_mapUserList[uiAccountID].iClanID = -1;

    RefreshClanList();
}


/*====================
  CChatManager::RemoveClanMember
  ====================*/
void    CChatManager::RemoveClanMember(const tstring &sName)
{
    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        RemoveClanMember(it->first);
        break;
    }
}

/*====================
  CChatManager::RemoveBan
  ====================*/
void    CChatManager::RemoveBan(const uint uiAccountID)
{
    ChatBanMap_it it = m_mapBanList.find(uiAccountID);

    if (it == m_mapBanList.end())
        return;

    m_mapBanList.erase(it);
}


/*====================
  CChatManager::RemoveBan
  ====================*/
void    CChatManager::RemoveBan(const tstring &sName)
{
    ChatBanMap_it it = m_mapBanList.begin();

    while (it != m_mapBanList.end() && !CompareNames(it->second.sName, sName))
        it++;

    if (it == m_mapBanList.end())
        return;

    m_mapBanList.erase(it);
}


/*====================
  CChatManager::AddIgnore
  ====================*/
void    CChatManager::AddIgnore(const uint uiAccountID, const tstring &sName)
{
    ChatIgnoreMap_it it = m_mapIgnoreList.find(uiAccountID);

    if (it == m_mapIgnoreList.end())
        m_mapIgnoreList.insert(ChatIgnorePair(uiAccountID, sName));
}


/*====================
  CChatManager::RemoveIgnore
  ====================*/
void    CChatManager::RemoveIgnore(const uint uiAccountID)
{
    ChatIgnoreMap_it it = m_mapIgnoreList.find(uiAccountID);

    if (it == m_mapIgnoreList.end())
        return;

    m_mapIgnoreList.erase(it);
}


/*====================
  CChatManager::RemoveIgnore
  ====================*/
void    CChatManager::RemoveIgnore(const tstring &sName)
{
    ChatIgnoreMap_it it = m_mapIgnoreList.begin();

    while (it != m_mapIgnoreList.end() && !CompareNames(it->second, sName))
        it++;

    if (it == m_mapIgnoreList.end())
        return;

    m_mapIgnoreList.erase(it);
}


/*====================
  CChatManager::RequestBuddyAdd
  ====================*/
void    CChatManager::RequestBuddyAdd(const tstring &sBuddyNickName)
{
    if (sBuddyNickName.empty())
        return;

    if (CompareNames(sBuddyNickName, m_mapUserList[m_uiAccountID].sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_self")));
        return;
    }

    if (IsBuddy(sBuddyNickName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_add_duplicate")));
        return;
    }

    CPacket pktSend;
    pktSend << CHAT_CMD_REQUEST_BUDDY_ADD << sBuddyNickName;
    m_sockChat.SendPacket(pktSend); 

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_requesting_buddy"), _T("name"), sBuddyNickName));
}


/*====================
  CChatManager::RequestBuddyApprove
  ====================*/
void    CChatManager::RequestBuddyApprove(const tstring &sBuddyNickName)
{
    if (sBuddyNickName.empty())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_REQUEST_BUDDY_APPROVE << sBuddyNickName;
    m_sockChat.SendPacket(pktSend); 

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_approving_buddy"), _T("name"), sBuddyNickName));
}


/*====================
  CChatManager::RequestBuddyRemove
  ====================*/
void    CChatManager::RequestBuddyRemove(const uint uiAccountID)
{
    if (m_uiAccountID == uiAccountID)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_remove_self")));
        return;
    }
    
    ChatClientMap_it itClient(m_mapUserList.find(uiAccountID));
    if (itClient == m_mapUserList.end())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_remove_not_found")));
        return;
    }

    if (!IsBuddy(uiAccountID))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_buddy_remove"), _T("target"), itClient->second.sName));
        return;
    }

    // Send a request to delete a buddy
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("remove_buddy2"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("buddy_id"), uiAccountID);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_DELETE_BUDDY, uiAccountID));
    pNewRequest->sTarget = itClient->second.sName;
    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::RequestBanlistAdd
  ====================*/
void    CChatManager::RequestBanlistAdd(const tstring &sName, const tstring &sReason)
{
    if (sName.empty())
        return;

    if (CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_self")));
        return;
    }

    if (IsBanned(sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_add_duplicate")));
        return;
    }

    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("nick2id"));
    pHTTPRequest->AddVariable(_T("nickname[0]"), sName);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_ADD_BANNED_NICK2ID, sName, sReason));
    m_lHTTPRequests.push_back(pNewRequest);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_adding_banlist"), _T("name"), sName));
}


/*====================
  CChatManager::RequestBanlistRemove
  ====================*/
void    CChatManager::RequestBanlistRemove(uint uiAccountID)
{
    ChatBanMap_it it(m_mapBanList.find(uiAccountID));

    if (it == m_mapBanList.end())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_banlist_remove_not_found")));
        return;
    }

    // Send a request to delete a ban
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("remove_banned"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("banned_id"), uiAccountID);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_REMOVE_BANNED, uiAccountID));
    pNewRequest->sTarget = it->second.sName;
    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::RequestIgnoreAdd
  ====================*/
void    CChatManager::RequestIgnoreAdd(const tstring &sName)
{
    if (sName.empty())
        return;

    if (CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_self")));
        return;
    }

    if (IsIgnored(sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_add_duplicate")));
        return;
    }

    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("nick2id"));
    pHTTPRequest->AddVariable(_T("nickname[0]"), sName);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_ADD_IGNORED_NICK2ID, sName));
    m_lHTTPRequests.push_back(pNewRequest);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_adding_ignore"), _T("name"), sName));
}


/*====================
  CChatManager::RequestIgnoreRemove
  ====================*/
void    CChatManager::RequestIgnoreRemove(const uint uiAccountID)
{
    ChatIgnoreMap_it it(m_mapIgnoreList.find(uiAccountID));

    if (it == m_mapIgnoreList.end())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_ignore_remove_not_found")));
        return;
    }

    //Send a request to delete the remove
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("remove_ignored"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("ignored_id"), uiAccountID);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_REMOVE_IGNORED, uiAccountID));
    pNewRequest->sTarget = it->second;
    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::RequestPromoteClanMember
  ====================*/
void    CChatManager::RequestPromoteClanMember(const tstring &sName)
{
    if (!(m_mapUserList[m_uiAccountID].yFlags & CHAT_CLIENT_IS_CLAN_LEADER))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_rank")));
        return;
    }

    if (sName.empty())
        return;

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        if (CompareNoCase(it->second.sClan, m_mapUserList[m_uiAccountID].sClan) != 0)
            return;

        if (it->second.yFlags & CHAT_CLIENT_IS_OFFICER || it->second.yFlags & CHAT_CLIENT_IS_CLAN_LEADER)
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_promote_cannot_promote")));
            return;
        }

        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("set_rank"));
        pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
        pHTTPRequest->AddVariable(_T("target_id"), it->second.uiAccountID);
        pHTTPRequest->AddVariable(_T("clan_id"), m_mapUserList[m_uiAccountID].iClanID);
        pHTTPRequest->AddVariable(_T("rank"), _T("Officer"));
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_CLAN_PROMOTE, it->first));
        pNewRequest->sTarget = it->second.sName;
        m_lHTTPRequests.push_back(pNewRequest);

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_promoting"), _T("name"), RemoveClanTag(it->second.sName)));
        return;
    }

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_not_found"), _T("name"), sName)); 
}


/*====================
  CChatManager::RequestDemoteClanMember
  ====================*/
void    CChatManager::RequestDemoteClanMember(const tstring &sName)
{
    if (!(m_mapUserList[m_uiAccountID].yFlags & CHAT_CLIENT_IS_CLAN_LEADER))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_rank")));
        return;
    }

    if (sName.empty())
        return;

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        if (CompareNoCase(it->second.sClan, m_mapUserList[m_uiAccountID].sClan) != 0)
            return;

        if (!(it->second.yFlags & CHAT_CLIENT_IS_OFFICER) || it->second.yFlags & CHAT_CLIENT_IS_CLAN_LEADER)
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_demote_cannot_demote")));
            return;
        }

        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("set_rank"));
        pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
        pHTTPRequest->AddVariable(_T("target_id"), it->second.uiAccountID);
        pHTTPRequest->AddVariable(_T("clan_id"), m_mapUserList[m_uiAccountID].iClanID);
        pHTTPRequest->AddVariable(_T("rank"), _T("Member"));
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_CLAN_DEMOTE, it->first));
        pNewRequest->sTarget = it->second.sName;
        m_lHTTPRequests.push_back(pNewRequest);

        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_demoting"), _T("name"), RemoveClanTag(it->second.sName)));
        return;
    }

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_not_found"), _T("name"), sName)); 
}


/*====================
  CChatManager::RequestRemoveClanMember
  ====================*/
void    CChatManager::RequestRemoveClanMember(const tstring &sName)
{
    if (sName.empty())
        return;

    if (!(m_mapUserList[m_uiAccountID].yFlags & (CHAT_CLIENT_IS_CLAN_LEADER | CHAT_CLIENT_IS_OFFICER)) && !CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_rank")));
        return;
    }

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        if (CompareNoCase(it->second.sClan, m_mapUserList[m_uiAccountID].sClan) != 0)
            return;

        if ((it->second.yFlags & CHAT_CLIENT_IS_OFFICER || it->second.yFlags & CHAT_CLIENT_IS_CLAN_LEADER) && !CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_remove_demote")));
            return;
        }

        CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
        if (pHTTPRequest == NULL)
            return;

        pHTTPRequest->SetTargetURL(m_sMasterServerURL);
        pHTTPRequest->AddVariable(_T("f"), _T("set_rank"));
        pHTTPRequest->AddVariable(_T("member_ck"), m_sCookie);
        pHTTPRequest->AddVariable(_T("target_id"), it->second.uiAccountID);
        pHTTPRequest->AddVariable(_T("clan_id"), m_mapUserList[m_uiAccountID].iClanID);
        pHTTPRequest->AddVariable(_T("rank"), _T("Remove"));
        pHTTPRequest->SendPostRequest();

        SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_CLAN_REMOVE, it->first));
        pNewRequest->sTarget = it->second.sName;
        m_lHTTPRequests.push_back(pNewRequest);

        if (it->second.uiAccountID != m_uiAccountID)
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_removing"), _T("name"), RemoveClanTag(it->second.sName)));
        else
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_removing_self"), _T("name"), RemoveClanTag(it->second.sName)));

        return;
    }

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_not_found"), _T("name"), sName)); 
}


/*====================
  CChatManager::InviteToClan
  ====================*/
void    CChatManager::InviteToClan(const tstring &sName)
{
    if (!(m_mapUserList[m_uiAccountID].yFlags & (CHAT_CLIENT_IS_CLAN_LEADER | CHAT_CLIENT_IS_OFFICER)))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_error_clan_rank")));
        return;
    }

    if (sName.empty())
        return;

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        if (!it->second.sClan.empty())
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_clan_invite_in_clan"), _T("name"), it->second.sName));
            return;
        }

        break;
    }

    CPacket pkt;
    pkt << CHAT_CMD_CLAN_ADD_MEMBER << sName;
    m_sockChat.SendPacket(pkt);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_invite_sent"), _T("name"), sName)); 
}


/*====================
  CChatManager::CreateClan
  ====================*/
void    CChatManager::CreateClan(const tstring &sName, const tstring &sTag, const tstring &sMember1, const tstring &sMember2, const tstring &sMember3, const tstring &sMember4)
{
    ChatClientMap_it itLocalClient(m_mapUserList.find(m_uiAccountID));
    if (itLocalClient == m_mapUserList.end())
        return;

    SChatClient &localClient(itLocalClient->second);

    if (!localClient.sClan.empty())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_clan")));
        ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_clan")));
        return;
    }

    if (sName.empty() || sTag.empty() || sMember1.empty() || sMember2.empty() || sMember3.empty() || sMember4.empty())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_param")));
        ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_param")));
        return;
    }

    if (CompareNames(localClient.sName, sMember1) ||
        CompareNames(localClient.sName, sMember2) ||
        CompareNames(localClient.sName, sMember3) ||
        CompareNames(localClient.sName, sMember4) ||
        CompareNames(sMember1, sMember2) ||
        CompareNames(sMember1, sMember3) ||
        CompareNames(sMember1, sMember4) ||
        CompareNames(sMember2, sMember3) ||
        CompareNames(sMember2, sMember4) ||
        CompareNames(sMember3, sMember4))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_duplicate")));
        ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_duplicate")));
        return;
    }

    if (sTag.size() > 4)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_tag")));
        ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_tag")));
        return;
    }

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sMember1) && !CompareNames(it->second.sName, sMember2) && !CompareNames(it->second.sName, sMember3) && !CompareNames(it->second.sName, sMember4))
            continue;

        if (!it->second.sClan.empty())
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_fail_in_clan"), _T("name"), it->second.sName));
            ChatClanCreateFail.Trigger(Translate(_T("chat_clan_create_result_fail_in_clan"), _T("name"), it->second.sName));
            return;
        }
    }

    CPacket pkt;
    pkt << CHAT_CMD_CLAN_CREATE_REQUEST << sName << sTag << sMember1 << sMember2 << sMember3 << sMember4;
    m_sockChat.SendPacket(pkt);

    m_uiCreateTimeSent = K2System.Milliseconds();

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_create_sent")));    
}


/*====================
  CChatManager::RemoveClanTag
  ====================*/
tstring CChatManager::RemoveClanTag(const tstring &sName)
{
    auto uiPos(sName.find(_T("]")));

    if (uiPos != tstring::npos)
        return sName.substr(uiPos + 1);

    return sName;
}


/*====================
  CChatManager::CompareNames
  ====================*/
bool    CChatManager::CompareNames(const tstring &sOrig, const tstring &sName)
{
    if (CompareNoCase(sOrig, sName) == 0)
        return true;

    auto uiPos(sOrig.find(_T("]")));
    auto uiPos2(sName.find(_T("]")));

    if (uiPos != tstring::npos && uiPos2 != tstring::npos)
        return (CompareNoCase(sOrig.substr(uiPos + 1), sName.substr(uiPos2 + 1)) == 0);
    else if (uiPos != tstring::npos)
        return (CompareNoCase(sOrig.substr(uiPos + 1), sName) == 0);
    else if (uiPos2 != tstring::npos)
        return (CompareNoCase(sOrig, sName.substr(uiPos2 + 1)) == 0);

    return false;
}

bool    CChatManager::CompareNames(uint uiAccountID, const tstring &sName)
{
    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return false;

    return CompareNames(it->second.sName, sName);
}


/*====================
  CChatManager::GetBanList
  ====================*/
void    CChatManager::GetBanList()
{
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("ban_list"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_GET_BANNED, INVALID_ACCOUNT));
    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::AddToRecentlyPlayed
  ====================*/
void    CChatManager::AddToRecentlyPlayed(const tstring &sName)
{
    if (CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
        return;

    if (m_eStatus < CHAT_STATUS_JOINING_GAME)
        return;

    if (m_eStatus == CHAT_STATUS_JOINING_GAME)
    {
        m_setRecentlyPlayed.insert(sName);
        return;
    }

    ChatBanMap_it it = m_mapBanList.begin();

    while (it != m_mapBanList.end() && !CompareNames(it->second.sName, sName))
        it++;

    if (it != m_mapBanList.end())
        AddGameChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_on_banlist"), _T("name"), it->second.sName, _T("reason"), it->second.sReason));

    m_pRecentlyPlayed->NewNode(_T("player"));
    m_pRecentlyPlayed->AddProperty(_T("name"), sName);
    m_pRecentlyPlayed->EndNode();

    UpdateRecentlyPlayed();
}


/*====================
  CChatManager::UpdateRecentlyPlayed
  ====================*/
void    CChatManager::UpdateRecentlyPlayed()
{
    ChatRecentlyPlayedEvent.Trigger(_T("ClearItems"));

    if (m_uiAccountID == INVALID_ACCOUNT)
        return;

    if (m_eStatus == CHAT_STATUS_IN_GAME && m_bMatchStarted)
        m_pRecentlyPlayed->EndNode();

    bool bContinue = m_pRecentlyPlayed->TraverseChildrenReverse();
    bool bTraversed = bContinue;
    uint uiNumTraversed(0);

    tsvector vHeader(3);
    tsvector vPlayer(2);

    while (bContinue && uiNumTraversed < 5)
    {
        if (m_pRecentlyPlayed->GetNodeName() != "match")
        {
            bContinue = m_pRecentlyPlayed->TraversePrevChild();
            continue;
        }

        uiNumTraversed++;

        vHeader[0] = m_pRecentlyPlayed->GetProperty(_T("id"));
        vHeader[1] = m_pRecentlyPlayed->GetProperty(_T("time"));
        vHeader[2] = m_pRecentlyPlayed->GetProperty(_T("name"));

        ChatRecentlyPlayedHeader.Trigger(vHeader, true);

        bool bPlayersContinue = m_pRecentlyPlayed->TraverseChildren();
        bool bPlayersEndNode = bPlayersContinue;

        while (bPlayersContinue)
        {
            if (m_pRecentlyPlayed->GetNodeName() != "player")
            {
                bPlayersContinue = m_pRecentlyPlayed->TraverseNextChild();
                continue;
            }

            vPlayer[0] = m_pRecentlyPlayed->GetProperty(_T("name"));
            vPlayer[1] = vHeader[0] + vPlayer[0];

            ChatRecentlyPlayedPlayer.Trigger(vPlayer, true);

            bPlayersContinue = m_pRecentlyPlayed->TraverseNextChild();
        }

        if (bPlayersEndNode)
            m_pRecentlyPlayed->EndNode();

        bContinue = m_pRecentlyPlayed->TraversePrevChild();
    }

    if (bTraversed)
        m_pRecentlyPlayed->EndNode();

    if (m_eStatus == CHAT_STATUS_IN_GAME && m_bMatchStarted)
    {
        bool bTraversed = m_pRecentlyPlayed->TraverseChildren();
        bool bFound = false;

        if (bTraversed)
        {
            bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(m_uiMatchID));

            while (!bFound && m_pRecentlyPlayed->TraverseNextChild())
                bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(m_uiMatchID));

            if (!bFound)
            {
                m_pRecentlyPlayed->EndNode();

                m_cDate = CDate(true);

                m_pRecentlyPlayed->NewNode(_T("match"));
                m_pRecentlyPlayed->AddProperty(_T("name"), m_sGameName);
                m_pRecentlyPlayed->AddProperty(_T("id"), XtoA(m_uiMatchID));
                m_pRecentlyPlayed->AddProperty(_T("time"), m_cDate.GetTimeString(TIME_NO_SECONDS) + _T(" ") + m_cDate.GetDateString(DATE_SHORT_YEAR | DATE_MONTH_FIRST));
            }
        }
    }
}


/*====================
  CChatManager::ShowPostGameStats
  ====================*/
void    CChatManager::ShowPostGameStats(uint uiMatchID)
{
    m_bWaitingToShowStats = true;
    m_uiShowStatsMatchID = uiMatchID;
    ChatShowPostGameStats.Trigger(TSNULL);
}


/*====================
  CChatManager::GetTournamentAddress
  ====================*/
tstring CChatManager::GetTournamentAddress(uint uiTournMatchID)
{
    map<uint, tstring>::iterator it(m_mapTournGameAddresses.find(uiTournMatchID));

    if (it == m_mapTournGameAddresses.end())
        return TSNULL;

    return it->second;
}


/*====================
  CChatManager::JoinGame
  ====================*/
bool    CChatManager::JoinGame(const tstring &sName)
{
    if (sName.empty())
        return false;

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        if (it->second.sServerAddressPort.empty())
            return false;

        Host.Connect(it->second.sServerAddressPort);
        return true;
    }

    return false;
}


/*====================
  CChatManager::UpdateUserList
  ====================*/
void    CChatManager::UpdateUserList(uint uiChannelID)
{
    if (uiChannelID == -1)
        return;

    tstring sChannel(GetChannelName(uiChannelID));

    if (sChannel.empty())
        return;

    uint uiStartTime(K2System.Microseconds());

    static tsvector vParams(12);
    static tsvector vMiniParams(2);
    
    // These stay the same throughout the rest of the function
    vParams[0] = vMiniParams[0] = sChannel; 
    
    vMiniParams[1] = _T("ClearItems();");   
    ChatUserEvent.Trigger(vMiniParams);
    
    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (it->second.yStatus < CHAT_STATUS_CONNECTED || it->second.setChannels.find(uiChannelID) == it->second.setChannels.end())
            continue;

        vParams[1] = it->second.sName;
        vParams[2] = XtoA(GetAdminLevel(uiChannelID, it->first));
        vParams[3] = XtoA(it->second.yStatus > CHAT_STATUS_CONNECTED, true);
        vParams[4] = XtoA((it->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
        vParams[5] = XtoA(it->second.uiAccountID);
        vParams[6] = Host.GetChatSymbolTexturePath(it->second.uiChatSymbol);
        vParams[7] = Host.GetChatNameColorTexturePath(it->second.uiChatNameColor);
        vParams[8] = Host.GetChatNameColorString(it->second.uiChatNameColor);
        vParams[9] = Host.GetChatNameColorIngameString(it->second.uiChatNameColor);
        vParams[10] = Host.GetAccountIconTexturePath(it->second.uiAccountIcon);
        vParams[11] = XtoA(it->second.uiSortIndex);
        ChatUserNames.Trigger(vParams);
    }

    vMiniParams[1] = _T("SortListboxSortIndex();");
    ChatUserEvent.Trigger(vMiniParams);

    if (chat_profile)
        Console << _T("UpdateUserList - ") << K2System.Microseconds() - uiStartTime << _T(" us") << newl;
}


/*====================
  CChatManager::UpdateBuddyList
  ====================*/
void    CChatManager::UpdateBuddyList()
{
    uint uiStartTime(K2System.Microseconds());
    uint uiTotalOnline(0);

    ChatBuddyEvent.Trigger(_T("ClearItems"));

    for (uiset_it it(m_setBuddyList.begin()); it != m_setBuddyList.end(); it++)
    {
        if (m_mapUserList[*it].yStatus == CHAT_STATUS_CONNECTED)
        {
            ChatBuddyOnline.Trigger(m_mapUserList[*it].sName);
            uiTotalOnline++;
        }
        else if (m_mapUserList[*it].yStatus > CHAT_STATUS_CONNECTED)
        {
            ChatBuddyGame.Trigger(m_mapUserList[*it].sName);
            uiTotalOnline++;
        }
        else
            ChatBuddyOffline.Trigger(m_mapUserList[*it].sName);
    }

    ChatBuddyEvent.Trigger(_T("SortListboxSortIndex"));
    
    ChatTotalFriends.Trigger(XtoA(INT_SIZE(m_setBuddyList.size())));
    ChatOnlineFriends.Trigger(XtoA(uiTotalOnline));

    if (chat_profile)
        Console << _T("UpdateBuddyList - ") << K2System.Microseconds() - uiStartTime << _T(" us") << newl;
}


/*====================
  CChatManager::UpdateClanList
  ====================*/
void    CChatManager::UpdateClanList()
{
    uint uiStartTime(K2System.Microseconds());
    uint uiTotalOnline(0);

    ChatClanEvent.Trigger(_T("ClearItems"));

    for (uiset_it it(m_setClanList.begin()); it != m_setClanList.end(); it++)
    {
        ChatClientMap_it userit(m_mapUserList.find(*it));

        if (userit == m_mapUserList.end())
            continue;

        tstring sColor;

/*      if (userit->second.yFlags & CHAT_CLIENT_IS_CLAN_LEADER)
            sColor = _T("^900");
        else if (userit->second.yFlags & CHAT_CLIENT_IS_OFFICER)
            sColor = _T("^666");
        else
            sColor = _T("^999");*/

        if (userit->second.yStatus == CHAT_STATUS_CONNECTED)
        {
            ChatClanOnline.Trigger(sColor + userit->second.sName);
            uiTotalOnline++;
        }
        else if (userit->second.yStatus > CHAT_STATUS_CONNECTED)
        {
            ChatClanGame.Trigger(sColor + userit->second.sName);
            uiTotalOnline++;
        }
        else if (userit->second.yStatus < CHAT_STATUS_CONNECTED)
            ChatClanOffline.Trigger(sColor + userit->second.sName);
    }

    ChatClanEvent.Trigger(_T("SortListboxSortIndex"));

    ChatTotalClanMembers.Trigger(XtoA(INT_SIZE(m_setClanList.size())));
    ChatOnlineClanMembers.Trigger(XtoA(uiTotalOnline));

    if (chat_profile)
        Console << _T("UpdateClanList - ") << K2System.Microseconds() - uiStartTime << _T(" us") << newl;
}


/*====================
  CChatManager::UpdateLookingForClan
  ====================*/
void    CChatManager::UpdateLookingForClan()
{
    ChatLookingForClanEvent.Execute(_T("ClearItems();"));

    for (sset_it it(m_setLookingForClan.begin()); it != m_setLookingForClan.end(); it++)
        ChatLookingForClanEvent.Execute(_T("AddItem('^777") + (*it) + _T("','") + (*it) + _T("');"));
}

/*====================
  CChatManager::InviteUser
  ====================*/
void    CChatManager::InviteUser(const tstring &sName)
{
    if (GetStatus() >= CHAT_STATUS_JOINING_GAME)
    {
        CHostClient *pClient(Host.GetActiveClient());
        if (pClient != NULL)
        {
            pClient->InviteUser(sName);
        }
    }
    else
    {
        // TODO: Invite to current channel
    }
}


/*====================
  CChatManager::IsBuddy
  ====================*/
bool    CChatManager::IsBuddy(const tstring &sName)
{
    if (CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
        return true;

    for (uiset_it it(m_setBuddyList.begin()); it != m_setBuddyList.end(); it++)
        if (CompareNames(m_mapUserList[*it].sName, sName))
            return true;

    return false;
}

/*====================
  CChatManager::IsClanMember
  ====================*/
bool    CChatManager::IsClanMember(const tstring &sName)
{
    if (CompareNames(sName, m_mapUserList[m_uiAccountID].sName))
        return true;

    for (uiset_it it(m_setClanList.begin()); it != m_setClanList.end(); it++)
        if (CompareNames(m_mapUserList[*it].sName, sName))
            return true;

    return false;
}


/*====================
  CChatManager::IsBuddy
  ====================*/
bool    CChatManager::IsBuddy(uint uiAccountID)
{
    return m_setBuddyList.find(uiAccountID) != m_setBuddyList.end();
}


/*====================
  CChatManager::IsClanMember
  ====================*/
bool    CChatManager::IsClanMember(uint uiAccountID)
{
    return m_setClanList.find(uiAccountID) != m_setClanList.end();
}


/*====================
  CChatManager::IsBanned
  ====================*/
bool    CChatManager::IsBanned(uint uiAccountID)
{
    ChatBanMap_it it = m_mapBanList.find(uiAccountID);

    if (it == m_mapBanList.end())
        return false;

    return true;
}


/*====================
  CChatManager::IsBanned
  ====================*/
bool    CChatManager::IsBanned(const tstring &sName)
{
    ChatBanMap_it it = m_mapBanList.begin();

    while (it != m_mapBanList.end() && !CompareNames(it->second.sName, sName))
        it++;

    if (it == m_mapBanList.end())
        return false;

    return true;
}


/*====================
  CChatManager::IsIgnored
  ====================*/
bool    CChatManager::IsIgnored(uint uiAccountID)
{
    ChatIgnoreMap_it it = m_mapIgnoreList.find(uiAccountID);

    if (it == m_mapIgnoreList.end())
        return false;

    return true;
}

/*====================
  CChatManager::IsIgnored
  ====================*/
bool    CChatManager::IsIgnored(const tstring &sName)
{
    ChatIgnoreMap_it it = m_mapIgnoreList.begin();

    while (it != m_mapIgnoreList.end() && !CompareNames(it->second, sName))
        it++;

    if (it == m_mapIgnoreList.end())
        return false;

    return true;
}


/*====================
  CChatManager::SetChatModeType
  ====================*/
void    CChatManager::SetChatModeType(const uint uiChatModeType, const tstring &sReason, bool bSetDefaultMode)
{
    // they are just logging into the chat server, so default the chat mode to available
    if (bSetDefaultMode)
    {
        m_uiChatModeType = uiChatModeType;
        return;     
    }

    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_mode_switch_fail")));
        return;
    }
        
    CPacket pktSend;
    pktSend << CHAT_CMD_SET_CHAT_MODE_TYPE << uiChatModeType << sReason.substr(0, CHAT_MESSAGE_MAX_LENGTH);
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::HasFlags
  ====================*/
bool    CChatManager::HasFlags(uint uiAccountID, byte yFlags)
{
    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return false;

    return ((it->second.yFlags & yFlags) == yFlags);
}


/*====================
  CChatManager::HasFlags
  ====================*/
bool    CChatManager::HasFlags(const tstring &sName, byte yFlags)
{
    ChatClientMap_it it(m_mapUserList.begin());

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it == m_mapUserList.end())
        return false;

    return ((it->second.yFlags & yFlags) == yFlags);
}

/*====================
  CChatManager::IsInAClan
  ====================*/
bool    CChatManager::IsInAClan(uint uiAccountID)
{
    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return false;

    return !it->second.sClan.empty();
}

/*====================
  CChatManager::IsInAClan
  ====================*/
bool    CChatManager::IsInAClan(const tstring &sName)
{
    ChatClientMap_it it(m_mapUserList.begin());

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it == m_mapUserList.end())
        return false;

    return !it->second.sClan.empty();
}


/*====================
  CChatManager::GetBanReason
  ====================*/
tstring CChatManager::GetBanReason(uint uiAccountID)
{
    ChatBanMap_it it = m_mapBanList.find(uiAccountID);

    if (it == m_mapBanList.end())
        return TSNULL;

    return it->second.sReason;
}


/*====================
  CChatManager::GetBanReason
  ====================*/
tstring CChatManager::GetBanReason(const tstring &sName)
{
    ChatBanMap_it it = m_mapBanList.begin();

    while (it != m_mapBanList.end() && !CompareNames(it->second.sName, sName))
        it++;

    if (it == m_mapBanList.end())
        return TSNULL;

    return it->second.sReason;
}


/*====================
  CChatManager::PlaySound
  ====================*/
void    CChatManager::PlaySound(const tstring &sSoundName)
{
    PROFILE("CChatManager::PlaySound");

    CStringTable *pSounds(g_ResourceManager.GetStringTable(m_hStringTable));

    if (pSounds == NULL)
        return;

    ResHandle hHandle(g_ResourceManager.Register(pSounds->Get(sSoundName), RES_SAMPLE));

    if (hHandle == INVALID_RESOURCE)
        return;

    K2SoundManager.Play2DSound(hHandle);
}


/*====================
  CChatManager::GetChannelName
  ====================*/
const tstring&  CChatManager::GetChannelName(uint uiChannelID)
{
    return m_mapChannels[uiChannelID].sChannelName;
}


/*====================
  CChatManager::GetChannelID
  ====================*/
uint    CChatManager::GetChannelID(const tstring &sName)
{
    for (ChatChannelMap_it it(m_mapChannels.begin()), itEnd(m_mapChannels.end()); it != itEnd; ++it)
    {
        if (CompareNames(it->second.sChannelName, sName))
            return it->first;
    }

    return -1;
}


/*====================
  CChatManager::IsSavedChannel
  ====================*/
bool    CChatManager::IsSavedChannel(const tstring &sChannel)
{
    for (sset_it it(m_setAutoJoinChannels.begin()), itEnd(m_setAutoJoinChannels.end()); it != itEnd; ++it)
    {
        tstring sChannelName(it->c_str());
        if (CompareNoCase(sChannel, sChannelName) == 0)
            return true;
    }
    return false;
}
    
    
/*====================
  CChatManager::SaveChannelLocal
  ====================*/
void    CChatManager::SaveChannelLocal(const tstring &sChannel)
{
    // this is used to temporarily store the list of auto join channels retrieved from master server in CClientAccount::ProcessLoginResponse()
    m_setAutoJoinChannels.insert(sChannel.substr(0, CHAT_CHANNEL_MAX_LENGTH));
}


/*====================
  CChatManager::RemoveChannelLocal
  ====================*/
void    CChatManager::RemoveChannelLocal(const tstring &sChannel)
{
    // this is used to remove the given auto join channel after the master server returns the response from CChatManager::RemoveChannel()
    m_setAutoJoinChannels.erase(sChannel.substr(0, CHAT_CHANNEL_MAX_LENGTH));
}


/*====================
  CChatManager::SaveChannel
  ====================*/
void    CChatManager::SaveChannel(const tstring &sChannel)
{
    // Don't save off channels with wierd % and ^ in them
    if (sChannel.find(_T("%")) != tstring::npos || sChannel.find(_T("^")) != tstring::npos)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_invalid_channel"), _T("channel"), sChannel));    
        return;
    }
        
    // this is used to save the specified channel to the db
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("add_room"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("chatroom_name"), sChannel);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_SAVE_CHANNEL, sChannel));
    m_lHTTPRequests.push_back(pNewRequest);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_saving_channel"), _T("channel"), sChannel));
}


/*====================
  CChatManager::RemoveChannel
  ====================*/
void    CChatManager::RemoveChannel(const tstring &sChannel)
{
    // this is used to remove the specified channel from the db
    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("remove_room"));
    pHTTPRequest->AddVariable(_T("account_id"), m_uiAccountID);
    pHTTPRequest->AddVariable(_T("chatroom_name"), sChannel);
    pHTTPRequest->AddVariable(_T("cookie"), m_sCookie);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_REMOVE_CHANNEL, sChannel));
    m_lHTTPRequests.push_back(pNewRequest);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_removing_channel"), _T("channel"), sChannel));
}


/*====================
  CChatManager::JoinChannel
  ====================*/
void    CChatManager::JoinChannel(const tstring &sChannel)
{
    if (!IsConnected() || sChannel.empty())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_JOIN_CHANNEL << sChannel.substr(0, CHAT_CHANNEL_MAX_LENGTH);

    m_sockChat.SendPacket(pktSend);
}

void    CChatManager::JoinChannel(const tstring &sChannel, const tstring &sPassword)
{
    if (!IsConnected() || sChannel.empty() || sPassword.empty())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_JOIN_CHANNEL_PASSWORD << sChannel.substr(0, CHAT_CHANNEL_MAX_LENGTH) << sPassword;

    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::LeaveChannel
  ====================*/
void    CChatManager::LeaveChannel(const tstring &sChannel)
{
    if (!IsConnected() || sChannel.empty())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_LEAVE_CHANNEL << sChannel.substr(0, CHAT_CHANNEL_MAX_LENGTH);

    m_sockChat.SendPacket(pktSend);

    uint uiChannelID(GetChannelID(sChannel));

    m_setChannelsIn.erase(uiChannelID);

    if (uiChannelID != -1)
        ChatLeftChannel.Trigger(XtoA(uiChannelID));
}


/*====================
  CChatManager::RequestChannelList
  ====================*/
void    CChatManager::RequestChannelList()
{
    if (!IsConnected())
        return;

    ChatChannelList.Execute(_T("ClearData();"));

    CPacket pktSend;
    pktSend << NET_CHAT_CL_GET_CHANNEL_LIST;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::RequestChannelSublist
  ====================*/
void    CChatManager::RequestChannelSublist(const tstring &sHead)
{
    if (!IsConnected())
        return;

    tstring sLowerHead(LowerString(sHead));

    // If the last finished list is a super-set of the requested list use the finished set instead
    if (m_bFinishedList && sLowerHead.compare(0, m_sFinishedListHead.length(), m_sFinishedListHead) == 0)
    {
        for (ChatChannelInfoMap_it it(m_mapChannelList.begin()); it != m_mapChannelList.end(); ++it)
        {
            if (it->second.sLowerName.compare(0, sLowerHead.length(), sLowerHead) != 0)
                continue;

            ChatAutoCompleteAdd.Trigger(it->second.sName);
        }

        return;
    }

    ++m_yListStartSequence;
    
    CPacket pktSend;
    pktSend << NET_CHAT_CL_GET_CHANNEL_SUBLIST << m_yListStartSequence << sLowerHead;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::ChannelSublistCancel
  ====================*/
void    CChatManager::ChannelSublistCancel()
{
    m_mapChannelList.clear();
    m_bFinishedList = false;
    m_sFinishedListHead.clear();
    m_yListStartSequence = 0xff;
    m_yProcessingListSequence = 0xff;
    m_sProcessingListHead.clear();
}


/*====================
  CChatManager::SendChannelMessage
  ====================*/
bool    CChatManager::SendChannelMessage(const tstring &sMessage, uint uiChannelID, uint eChatMessageType)
{
    if (sMessage.empty())
        return true;

    if (!IsConnected())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_connected")));
        return false;
    }

    CPacket pktSend;
    
    if (eChatMessageType == CHAT_MESSAGE_ADD)
    {
        pktSend << CHAT_CMD_CHANNEL_MSG << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH) << uiChannelID;
    }
    else if (eChatMessageType == CHAT_MESSAGE_ROLL)
    {
        pktSend << CHAT_CMD_CHAT_ROLL << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH) << uiChannelID;
    }
    else if (eChatMessageType == CHAT_MESSAGE_EMOTE)
    {
        pktSend << CHAT_CMD_CHAT_EMOTE << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH) << uiChannelID;
    }

    m_sockChat.SendPacket(pktSend);

    if (eChatMessageType == CHAT_MESSAGE_ADD)
    {
        PlaySound(_T("SentChannelMessage"));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_channel_message_sent"), _T("name"), m_mapUserList[m_uiAccountID].sName, _T("message"), sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)), GetChannelName(uiChannelID), true);
    }
    else if (eChatMessageType == CHAT_MESSAGE_ROLL)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ROLL, sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH), GetChannelName(uiChannelID), true);
    }
    else if (eChatMessageType == CHAT_MESSAGE_EMOTE)
    {
        AddIRCChatMessage(CHAT_MESSAGE_EMOTE, sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH), GetChannelName(uiChannelID), true);
    }

    return true;
}


/*====================
  CChatManager::SendWhisper
  ====================*/
bool    CChatManager::SendWhisper(const tstring &sTarget, const tstring &sMessage)
{
    if (sMessage.empty())
        return true;

    if (!IsConnected())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_connected")));
        return false;
    }

    if (CompareNames(sTarget, m_mapUserList[m_uiAccountID].sName))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_whisper_failed_self")));
        return true;
    }

    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_invisible")));
        return true;        
    }
    
    // the sender is currently in AFK/DND mode, automatically set their status to available so they can receive messages
    if (GetChatModeType() == CHAT_MODE_AFK || GetChatModeType() == CHAT_MODE_DND) 
        SetChatModeType(CHAT_MODE_AVAILABLE, _T("chat_command_available_message"));
        
    CPacket pktSend;
    pktSend << CHAT_CMD_WHISPER << sTarget << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH);

    m_sockChat.SendPacket(pktSend);

    PlaySound(_T("SentWhisper"));
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_whisper_sent"), _T("target"), sTarget, _T("message"), sMessage));

    return true;
}


/*====================
  CChatManager::SendIM
  ====================*/
bool    CChatManager::SendIM(const tstring &sOrigTarget, const tstring &sMessage)
{
    if (sMessage.empty())
        return true;

    tstring sTarget = RemoveClanTag(sOrigTarget);

    if (!IsConnected())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_connected")));
        return false;
    }
    
    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_invisible")));
        return true;        
    }   

    CPacket pktSend;
    pktSend << CHAT_CMD_IM << sTarget << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH);

    m_sockChat.SendPacket(pktSend);

    m_cDate = CDate(true);
    tstring sFinal(_T("^770[") + m_cDate.GetTimeString(TIME_NO_SECONDS) + _T("] ") + Translate(_T("chat_im_sent"), _T("name"), RemoveClanTag(m_mapUserList[m_uiAccountID].sName), _T("message"), sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)));

    m_mapIMs[sTarget].push_back(sFinal);

    static tsvector vParams(3);
    vParams[0] = sTarget;
    vParams[1] = sFinal;
    vParams[2] = _T("0");   
    ChatWhisperUpdate.Trigger(vParams);
    
    PlaySound(_T("SentIM"));    
    
    ChatSentIMCount.Trigger(XtoA(ChatManager.AddSentIM()));
    ChatOpenIMCount.Trigger(XtoA(GetOpenIMCount()));

    return true;
}


/*====================
  CChatManager::SendClanWhisper
  ====================*/
bool    CChatManager::SendClanWhisper(const tstring &sMessage)
{
    if (sMessage.empty())
        return true;

    if (!IsConnected())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_connected")));
        return false;
    }
    
    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_invisible")));
        return true;        
    }       

    CPacket pktSend;
    pktSend << CHAT_CMD_CLAN_WHISPER << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH);

    m_sockChat.SendPacket(pktSend);

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_whisper_sent"), _T("name"), m_mapUserList[m_uiAccountID].sName, _T("message"), sMessage));

    tstring sIM(Translate(_T("chat_clan_im_sent"), _T("name"), m_mapUserList[m_uiAccountID].sName, _T("message"), sMessage));

    m_vClanWhispers.push_back(sIM);
    ChatClanWhisperUpdate.Trigger(sIM);
    PlaySound(_T("SentClanMessage"));

    //if (cc_showClanMessageNotification)
    //{
        //PushNotification(NOTIFY_TYPE_CLAN_WHISPER, Translate(_T("chat_notification_clan_whisper_sent"), _T("name"), m_mapUserList[m_uiAccountID].sName, _T("message"), sMessage), m_mapUserList[m_uiAccountID].sName);
    //}

    return true;
}


/*====================
  CChatManager::UpdateWhispers
  ====================*/
void    CChatManager::UpdateWhispers(const tstring &sOrigName)
{
    const tstring sName(RemoveClanTag(sOrigName));

    IMMap_it findit(m_mapIMs.find(sName));

    if (findit != m_mapIMs.end())
    {
        tsvector vParams(3);
        vParams[0] = sName;
        vParams[2] = _T("0");

        for (tsvector_it it(findit->second.begin()); it != findit->second.end(); it++)
        {
            vParams[1] = *it;
            ChatWhisperUpdate.Trigger(vParams, true);
        }
    }
}


/*====================
  CChatManager::UpdateClanWhispers
  ====================*/
void    CChatManager::UpdateClanWhispers()
{
    for (tsvector::iterator it(m_vClanWhispers.begin()); it != m_vClanWhispers.end(); it++)
        ChatClanWhisperUpdate.Trigger(*it);
}


/*====================
  CChatManager::UpdateHoverInfo
  ====================*/
bool    CChatManager::UpdateHoverInfo(const tstring &sName)
{
    if (sName.empty())
        return false;

    ChatClientMap_it itFind;
    bool bFound(false);

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (CompareNames(it->second.sName, sName))
        {
            itFind = it;
            bFound = true;
            break;
        }
    }

    if (bFound)
    {
        ChatHoverName.Trigger(itFind->second.sName);

        if (itFind->second.sClan.empty())
            ChatHoverClan.Trigger(_T("None"));
        else
            ChatHoverClan.Trigger(itFind->second.sClan);

/*      FIXME
        if (itFind->second.uiServerID == -1)
        {*/
            ChatHoverServer.Trigger(_T("None"));
            ChatHoverGameTime.Trigger(_T("N/A"));
/*      }
        else
        {
            ServerInfo &info(BrowserManager.GetServerInfoByUniqueID(itFind->second.uiServerID));

            if (info.sGameName.empty())
            {
                ChatHoverServer.Trigger(_T("Unknown"));
                ChatHoverGameTime.Trigger(_T("??:??:??"));
            }
            else
            {
                ChatHoverServer.Trigger(info.sGameName);
                ChatHoverGameTime.Trigger(info.sGameTime);
            }
        }*/
    }

    return bFound;
}


/*====================
  CChatManager::IsUserInGame
  ====================*/
bool    CChatManager::IsUserInGame(const tstring &sName)
{
    if (sName.empty())
        return false;

    ChatClientMap_it it = m_mapUserList.begin();

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it != m_mapUserList.end())
        return (it->second.yStatus > CHAT_STATUS_CONNECTED);

    return false;
}


/*====================
  CChatManager::IsUserInCurrentGame
  ====================*/
bool    CChatManager::IsUserInCurrentGame(const tstring &sName)
{
    if (sName.empty())
        return false;

    ChatClientMap_it it = m_mapUserList.begin();

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it != m_mapUserList.end())
    {
        for (uiset_it channelit(it->second.setChannels.begin()); channelit != it->second.setChannels.end(); channelit++)
            if (m_mapChannels.find(*channelit) != m_mapChannels.end() && m_mapChannels[*channelit].uiFlags & CHAT_CHANNEL_FLAG_SERVER && m_setChannelsIn.find(*channelit) != m_setChannelsIn.end())
                return true;
    }       

    return false;
}


/*====================
  CChatManager::SaveNotes
  ====================*/
void    CChatManager::SaveNotes()
{
    CFile *pFile(FileManager.GetFile(_T("~/notes.txt"), FILE_WRITE | FILE_TEXT));

    if (pFile == NULL || !pFile->IsOpen())
        return;

    uint uiNum(0);

    while (uiNum < m_vNotes.size() && uiNum < m_vNoteTimes.size())
    {
        tstring sValue(m_vNotes[uiNum]);

        if (uiNum + 1 != m_vNotes.size())
            sValue += newl;

        pFile->WriteString(XtoA(uiNum + 1) + _T("|") + m_vNoteTimes[uiNum] + _T("|") + sValue);
        uiNum++;
    }

    pFile->Close();
    SAFE_DELETE(pFile);
}


/*====================
  CChatManager::AdminKick
  ====================*/
void    CChatManager::AdminKick(const tstring &sName)
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_ADMIN_KICK << sName;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::GetCurrentChatHistory
  ====================*/
tstring CChatManager::GetCurrentChatHistory()
{
    if (m_uiHistoryPos == 0 || m_vChatHistory.size() < m_uiHistoryPos - 1)
        return TSNULL;

    return m_vChatHistory[m_uiHistoryPos - 1];
}


/*====================
  CChatManager::AcceptClanInvite
  ====================*/
void    CChatManager::AcceptClanInvite()
{
    CPacket pkt;
    pkt << CHAT_CMD_CLAN_ADD_ACCEPTED;
    m_sockChat.SendPacket(pkt);
}


/*====================
  CChatManager::RejectClanInvite
  ====================*/
void    CChatManager::RejectClanInvite()
{
    CPacket pkt;
    pkt << CHAT_CMD_CLAN_ADD_REJECTED;
    m_sockChat.SendPacket(pkt);
}


/*====================
  CChatManager::IsUserOnline
  ====================*/
bool    CChatManager::IsUserOnline(const tstring &sName)
{
    if (sName.empty())
        return false;

    ChatClientMap_it it = m_mapUserList.begin();

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it != m_mapUserList.end())
        return (it->second.yStatus >= CHAT_STATUS_CONNECTED);

    return false;
}


/*====================
  CChatManager::JoiningGame
  ====================*/
void    CChatManager::JoiningGame(const tstring &sAddr)
{
    if (m_eStatus != CHAT_STATUS_CONNECTED)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_JOINING_GAME << sAddr;

    m_sockChat.SendPacket(pktSend);

    m_eStatus = CHAT_STATUS_JOINING_GAME;

    SetCurrentChatMessage(_T(""));
    m_setRecentlyPlayed.clear();

    m_bMatchStarted = false;
    m_bWaitingToShowStats = false;

    UpdateChannels();
}


/*====================
  CChatManager::FinishedJoiningGame
  ====================*/
void    CChatManager::FinishedJoiningGame(const tstring &sName, const uint uiMatchID)
{
    if (m_eStatus != CHAT_STATUS_JOINING_GAME)
        return;

    m_uiMatchID = uiMatchID;
    m_sGameName = sName;

    CPacket pktSend;
    pktSend << CHAT_CMD_JOINED_GAME << sName << uiMatchID;

    m_sockChat.SendPacket(pktSend);

    m_eStatus = CHAT_STATUS_IN_GAME;

    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_joining_game"), _T("name"), sName), TSNULL, true);

    UpdateChannels();
}


/*====================
  CChatManager::MatchStarted
  ====================*/
void    CChatManager::MatchStarted()
{
    if (m_eStatus != CHAT_STATUS_IN_GAME)
        return;

    if (m_uiMatchID == -1)
        return;

    if (m_bMatchStarted)
        return;

    bool bTraversed = m_pRecentlyPlayed->TraverseChildren();
    bool bFound = false;

    if (bTraversed)
    {
        bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(m_uiMatchID));

        while (!bFound && m_pRecentlyPlayed->TraverseNextChild())
            bFound = (m_pRecentlyPlayed->GetProperty(_T("id")) == XtoA(m_uiMatchID));

        if (!bFound)
            m_pRecentlyPlayed->EndNode();
        else
            m_pRecentlyPlayed->DeleteNode();
    }

    m_cDate = CDate(true);

    m_pRecentlyPlayed->NewNode(_T("match"));
    m_pRecentlyPlayed->AddProperty(_T("name"), m_sGameName);
    m_pRecentlyPlayed->AddProperty(_T("id"), XtoA(m_uiMatchID));
    m_pRecentlyPlayed->AddProperty(_T("time"), m_cDate.GetTimeString(TIME_NO_SECONDS) + _T(" ") + m_cDate.GetDateString(DATE_SHORT_YEAR | DATE_MONTH_FIRST));

    for (sset_it it(m_setRecentlyPlayed.begin()); it != m_setRecentlyPlayed.end(); it++)
        AddToRecentlyPlayed(*it);

    m_setRecentlyPlayed.clear();

    m_bMatchStarted = true;
}


/*====================
  CChatManager::ResetGame
  ====================*/
void    CChatManager::ResetGame()
{
    cc_curGameChannel = _T("");
    cc_curGameChannelID = -1;
}


/*====================
  CChatManager::LeaveMatchChannels
  ====================*/
void    CChatManager::LeaveMatchChannels()
{
    uiset setChannelsIn(m_setChannelsIn); // Copy
    for (uiset_it it(setChannelsIn.begin()); it != setChannelsIn.end(); ++it)
    {
        ChatChannelMap_it itFind(m_mapChannels.find(*it));
        if (itFind == m_mapChannels.end())
            continue;

        if (itFind->second.uiFlags & CHAT_CHANNEL_FLAG_SERVER)
            LeaveChannel(itFind->second.sChannelName);
    }
}


/*====================
  CChatManager::JoinGameLobby
  ====================*/
void    CChatManager::JoinGameLobby(bool bAllConnected)
{
    m_bInGameLobby = true;
    ChatNewGame.Trigger(TSNULL);
    SetFocusedChannel(-2, true);
}


/*====================
  CChatManager::LeaveGameLobby
  ====================*/
void    CChatManager::LeaveGameLobby()
{
    m_bInGameLobby = false;
    ChatLeftGame.Trigger(TSNULL);
}


/*====================
  CChatManager::LeftGame
  ====================*/
void    CChatManager::LeftGame()
{
    if (m_eStatus <= CHAT_STATUS_CONNECTED)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_LEFT_GAME;

    m_sockChat.SendPacket(pktSend);

    if (m_eStatus == CHAT_STATUS_IN_GAME && m_bMatchStarted)
    {
        m_pRecentlyPlayed->EndNode();
        m_pRecentlyPlayed->WriteFile(_T("~/recentlyplayed.xml"));
    }

    m_eStatus = CHAT_STATUS_CONNECTED;

    ChatClientMap_it itFind(m_mapUserList.find(m_uiAccountID));

    if (itFind != m_mapUserList.end())
        itFind->second.yStatus = byte(m_eStatus);
    
    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_left_game")), TSNULL, true);

    m_setRecentlyPlayed.clear();

    UpdateChannels();

    if (m_bMatchStarted)
        m_bMatchStarted = false;
}


/*====================
  CChatManager::SendServerInvite
  ====================*/
void    CChatManager::SendServerInvite(const tstring &sName)
{
    if (GetStatus() < CHAT_STATUS_JOINING_GAME)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_INVITE_USER_NAME << sName;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::SendServerInvite
  ====================*/
void    CChatManager::SendServerInvite(int iAccountID)
{
    if (GetStatus() < CHAT_STATUS_JOINING_GAME)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_INVITE_USER_ID << iAccountID;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::RejectServerInvite
  ====================*/
void    CChatManager::RejectServerInvite(int iAccountID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_INVITE_REJECTED << iAccountID;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::GetUserInfo
  ====================*/
void    CChatManager::GetUserInfo(const tstring &sName)
{
    if (sName.empty())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_USER_INFO << sName;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::RequestUserStatus
  ====================*/
void    CChatManager::RequestUserStatus(const tstring &sName)
{
    if (sName.empty())
        return;

    CPacket pktSend;
    pktSend << NET_CHAT_CL_GET_USER_STATUS << sName;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::GetAccountIDFromName
  ====================*/
uint    CChatManager::GetAccountIDFromName(const tstring &sName)
{
    for (ChatClientMap_it itUser(m_mapUserList.begin()), itEnd(m_mapUserList.end()); itUser != itEnd; ++itUser)
    {
        if (CompareNames(itUser->second.sName, sName))
            return itUser->second.uiAccountID;
    }

    for (ChatBanMap_it itUser(m_mapBanList.begin()), itEnd(m_mapBanList.end()); itUser != itEnd; ++itUser)
    {
        if (CompareNames(itUser->second.sName, sName))
            return itUser->second.uiAccountID;
    }
    
    for (ChatIgnoreMap_it itUser(m_mapIgnoreList.begin()), itEnd(m_mapIgnoreList.end()); itUser != itEnd; ++itUser)
    {
        if (CompareNames(itUser->second.c_str(), sName))
            return itUser->first;
    }

    return INVALID_ACCOUNT;
}


/*====================
  CChatManager::GetAccountNameFromID
  ====================*/
const tstring&  CChatManager::GetAccountNameFromID(uint uiAccountID)
{
    for (ChatClientMap_it itUser(m_mapUserList.begin()), itEnd(m_mapUserList.end()); itUser != itEnd; ++itUser)
    {
        if (itUser->second.uiAccountID == uiAccountID)
            return itUser->second.sName;
    }

    for (ChatBanMap_it itUser(m_mapBanList.begin()), itEnd(m_mapBanList.end()); itUser != itEnd; ++itUser)
    {
        if (itUser->second.uiAccountID == uiAccountID)
            return itUser->second.sName;
    }
    
    for (ChatIgnoreMap_it itUser(m_mapIgnoreList.begin()), itEnd(m_mapIgnoreList.end()); itUser != itEnd; ++itUser)
    {
        if (itUser->first == uiAccountID)
            return itUser->second;
    }

    return TSNULL;
}


/*====================
  CChatManager::UpdateChannels
  ====================*/
void    CChatManager::UpdateChannels()
{
    if (chat_debugInterface)
        Console.UI << _T("UpdateChannels") << newl;

    if (m_bInGameLobby)
        ChatNewGame.Trigger(TSNULL);
    
    static tsvector vParams(12);
    static tsvector vMiniParams(2); 

    vMiniParams[0] = _T("irc_status_chan");
    vMiniParams[1] = K2System.GetGameName() + _T(" - v") + K2_Version(K2System.GetVersionString());
    ChatChanTopic.Trigger(vMiniParams);

    for (uiset_it it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); it++)
    {
        if (m_mapChannels[*it].uiFlags & CHAT_CHANNEL_FLAG_HIDDEN)
            continue;

        ChatChannelMap_it itChannel(m_mapChannels.find(*it));

        if (itChannel == m_mapChannels.end())
            continue;

        vMiniParams[0] = XtoA(*it);
        vMiniParams[1] = itChannel->second.sChannelName;
        ChatNewChannel.Trigger(vMiniParams, true);

        if (chat_debugInterface)
            Console.UI << _T("UpdateChannels - ChatNewChannel ") << *it << _T(" ") << QuoteStr(GetChannelName(*it)) << newl;

        vMiniParams[0] = itChannel->second.sChannelName;
        vMiniParams[1] = itChannel->second.sTopic;
        ChatChanTopic.Trigger(vMiniParams);

        vMiniParams[0] = itChannel->second.sChannelName;
        vMiniParams[1] = XtoA(itChannel->second.uiUserCount);
        ChatChanNumUsers.Trigger(vMiniParams);
    }

    vMiniParams[0] = TSNULL;
    vMiniParams[1] = _T("ClearItems();");
    ChatUserEvent.Trigger(vMiniParams);         
        
    for (ChatClientMap_it userit(m_mapUserList.begin()); userit != m_mapUserList.end(); ++userit)
    {
        if (userit->second.yStatus < CHAT_STATUS_CONNECTED)
            continue;

        vParams[1] = userit->second.sName;

        for (uiset_it it(userit->second.setChannels.begin()); it != userit->second.setChannels.end(); ++it)
        {
            if (m_mapChannels[*it].uiFlags & CHAT_CHANNEL_FLAG_HIDDEN)
                continue;

            if (GetChannelName(*it).empty())
                continue;
                
            vParams[0] = GetChannelName(*it);
            vParams[2] = XtoA(GetAdminLevel(*it, userit->first));
            vParams[3] = XtoA(userit->second.yStatus > CHAT_STATUS_CONNECTED, true);
            vParams[4] = XtoA((userit->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
            vParams[5] = XtoA(userit->second.uiAccountID);
            vParams[6] = Host.GetChatSymbolTexturePath(userit->second.uiChatSymbol);
            vParams[7] = Host.GetChatNameColorTexturePath(userit->second.uiChatNameColor);
            vParams[8] = Host.GetChatNameColorString(userit->second.uiChatNameColor);
            vParams[9] = Host.GetChatNameColorIngameString(userit->second.uiChatNameColor);
            vParams[10] = Host.GetAccountIconTexturePath(userit->second.uiAccountIcon);
            vParams[11] = XtoA(userit->second.uiSortIndex);
            ChatUserNames.Trigger(vParams);
        }
    }

    vMiniParams[0] = TSNULL;
    vMiniParams[1] = _T("SortListboxSortIndex();");
    ChatUserEvent.Trigger(vMiniParams);
}


/*====================
  CChatManager::UpdateChannel
  ====================*/
void    CChatManager::UpdateChannel(const uint uiChannelID)
{
    ChatChannelMap_it it(m_mapChannels.find(uiChannelID));

    if (it == m_mapChannels.end())
        return;

    if (it->second.uiFlags & CHAT_CHANNEL_FLAG_HIDDEN)
        return;

    static tsvector vParams(12);
    static tsvector vMiniParams(2);
    
    // These stay the same throughout the rest of the function
    vParams[0] = vMiniParams[0] = it->second.sChannelName;  
    
    vMiniParams[1] = _T("ClearItems();");
    ChatUserEvent.Trigger(vMiniParams);
    
    for (ChatClientMap_it userit(m_mapUserList.begin()); userit != m_mapUserList.end(); userit++)
    {
        if (userit->second.yStatus < CHAT_STATUS_CONNECTED || userit->second.setChannels.find(uiChannelID) == userit->second.setChannels.end())
            continue;

        vParams[1] = userit->second.sName;
        vParams[2] = XtoA(GetAdminLevel(uiChannelID, userit->first));
        vParams[3] = XtoA(userit->second.yStatus > CHAT_STATUS_CONNECTED, true);
        vParams[4] = XtoA((userit->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
        vParams[5] = XtoA(userit->second.uiAccountID);
        vParams[6] = Host.GetChatSymbolTexturePath(userit->second.uiChatSymbol);
        vParams[7] = Host.GetChatNameColorTexturePath(userit->second.uiChatNameColor);
        vParams[8] = Host.GetChatNameColorString(userit->second.uiChatNameColor);
        vParams[9] = Host.GetChatNameColorIngameString(userit->second.uiChatNameColor);
        vParams[10] = Host.GetAccountIconTexturePath(userit->second.uiAccountIcon);
        vParams[11] = XtoA(userit->second.uiSortIndex);
        ChatUserNames.Trigger(vParams);
    }

    vMiniParams[1] = it->second.sTopic;
    ChatChanTopic.Trigger(vMiniParams);

    vMiniParams[1] = XtoA(it->second.uiUserCount);
    ChatChanNumUsers.Trigger(vMiniParams);

    vMiniParams[1] = _T("SortListboxSortIndex();");
    ChatUserEvent.Trigger(vMiniParams);
}


/*====================
  CChatManager::RebuildChannels
  ====================*/
void    CChatManager::RebuildChannels()
{
    UpdateChannels();

    uint uiFocusedChannel(m_uiFocusedChannel);

    m_uiFocusedChannel = -9001;

    SetFocusedChannel(uiFocusedChannel);
}


/*====================
  CChatManager::Translate
  ====================*/
tstring CChatManager::Translate(const tstring &sKey, const tstring &sParamName1, const tstring &sParamValue1, const tstring &sParamName2, const tstring &sParamValue2, const tstring &sParamName3, const tstring &sParamValue3, const tstring &sParamName4, const tstring &sParamValue4)
{
    CHostClient *pClient(Host.GetActiveClient());

    if (pClient == NULL)
        return TSNULL;

    tsmapts mapParams;

    if (sParamName1 != TSNULL)
        mapParams[sParamName1] = sParamValue1;

    if (sParamName2 != TSNULL)
        mapParams[sParamName2] = sParamValue2;

    if (sParamName3 != TSNULL)
        mapParams[sParamName3] = sParamValue3;

    if (sParamName4 != TSNULL)
        mapParams[sParamName4] = sParamValue4;
        
    return pClient->Translate(sKey, mapParams);
}

tstring CChatManager::Translate(const tstring &sKey, const tsmapts &mapParams)
{
    CHostClient *pClient(Host.GetActiveClient());

    if (pClient == NULL)
        return TSNULL;

    return pClient->Translate(sKey, mapParams);
}


/*====================
  CChatManager::AutoCompleteNick
  ====================*/
void    CChatManager::AutoCompleteNick(const tstring &sName)
{
    ChatAutoCompleteClear.Trigger(TSNULL);

    if (sName.length() < 3)
        return;

    // Cancel any current auto-complete requests
    for (ChatRequestList_it it(m_lHTTPRequests.begin()); it != m_lHTTPRequests.end();)
    {
        if ((*it)->eType != REQUEST_COMPLETE_NICK)
        {
            ++it;
            continue;
        }

        m_pHTTPManager->KillRequest((*it)->pRequest);
        K2_DELETE(*it);
        STL_ERASE(m_lHTTPRequests, it);
    }


    CHTTPRequest *pHTTPRequest(m_pHTTPManager->SpawnRequest());
    if (pHTTPRequest == NULL)
        return;

    pHTTPRequest->SetTargetURL(m_sMasterServerURL);
    pHTTPRequest->AddVariable(_T("f"), _T("autocompleteNicks"));
    pHTTPRequest->AddVariable(_T("nickname"), sName);
    pHTTPRequest->SendPostRequest();

    SChatDBRequest *pNewRequest(K2_NEW(ctx_Net,  SChatDBRequest)(pHTTPRequest, REQUEST_COMPLETE_NICK, sName));

    m_lHTTPRequests.push_back(pNewRequest);
}


/*====================
  CChatManager::AutoCompleteClear
  ====================*/
void    CChatManager::AutoCompleteClear()
{
    ChatAutoCompleteClear.Trigger(TSNULL);
}


/*====================
  CChatManager::AddWidgetReference
  ====================*/
void    CChatManager::AddWidgetReference(CTextBuffer *pBuffer, bool bIsGameChat, tstring sChannelName)
{
    m_vWidgets.push_back(SChatWidget(pBuffer, bIsGameChat, sChannelName));
}


/*====================
  CChannelManager::IsAdmin
  ====================*/
bool    CChatManager::IsAdmin(uint uiChannelID, uint uiAccountID, EAdminLevel eMinLevel)
{
    ChatChannelMap_it findit(m_mapChannels.find(uiChannelID));

    if (findit == m_mapChannels.end())
        return false;

    ChatAdminMap_it it(findit->second.mapAdmins.find(uiAccountID));

    if (it == findit->second.mapAdmins.end())
        return false;

    if (it->second < eMinLevel)
        return false;

    return true;
}


/*====================
  CChannelManager::IsAdmin
  ====================*/
bool    CChatManager::IsAdmin(uint uiChannelID, const tstring &sName, EAdminLevel eMinLevel)
{
    ChatChannelMap_it findit(m_mapChannels.find(uiChannelID));

    if (findit == m_mapChannels.end())
        return false;

    ChatClientMap_it it(m_mapUserList.begin());

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it == m_mapUserList.end())
        return false;

    ChatAdminMap_it adminit(findit->second.mapAdmins.find(it->second.uiAccountID));

    if (adminit == findit->second.mapAdmins.end())
        return false;

    if (adminit->second < eMinLevel)
        return false;

    return true;
}


/*====================
  CChannelManager::GetAdminLevel
  ====================*/
EAdminLevel CChatManager::GetAdminLevel(uint uiChannelID, const tstring &sName)
{
    ChatChannelMap_it findit(m_mapChannels.find(uiChannelID));

    if (findit == m_mapChannels.end())
        return CHAT_CLIENT_ADMIN_NONE;

    ChatClientMap_it it(m_mapUserList.begin());

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it == m_mapUserList.end())
        return CHAT_CLIENT_ADMIN_NONE;

    ChatAdminMap_it adminit(findit->second.mapAdmins.find(it->second.uiAccountID));

    if (adminit == findit->second.mapAdmins.end())
        return CHAT_CLIENT_ADMIN_NONE;

    return EAdminLevel(adminit->second);
}


/*====================
  CChannelManager::GetAdminLevel
  ====================*/
EAdminLevel CChatManager::GetAdminLevel(uint uiChannelID, uint uiAccountID)
{
    ChatChannelMap_it findit(m_mapChannels.find(uiChannelID));

    if (findit == m_mapChannels.end())
        return CHAT_CLIENT_ADMIN_NONE;

    ChatAdminMap_it it(findit->second.mapAdmins.find(uiAccountID));

    if (it == findit->second.mapAdmins.end())
        return CHAT_CLIENT_ADMIN_NONE;

    return EAdminLevel(it->second);
}


/*====================
  CChatManager::AddUnreadChannel
  ====================*/
void    CChatManager::AddUnreadChannel(uint uiChannelID)
{
    if (m_uiFocusedChannel == uiChannelID)
        return;

    uint uiNumUnread(0);

    m_mapChannels[uiChannelID].bUnread = true;

    tsvector vParams(2);
    vParams[0] = m_mapChannels[uiChannelID].sChannelName;
    vParams[1] = XtoA(true, true);
    ChatUnreadChannel.Trigger(vParams);

    for (uiset_it it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); it++)
    {
        if (!m_mapChannels[*it].bUnread)
            continue;

        uiNumUnread++;
    }

    ChatNumUnreadChannels.Trigger(XtoA(uiNumUnread));
}


/*====================
  CChatManager::RemoveUnreadChannel
  ====================*/
void    CChatManager::RemoveUnreadChannel(uint uiChannelID)
{
    uint uiNumUnread(0);

    m_mapChannels[uiChannelID].bUnread = false;

    tsvector vParams(2);
    vParams[0] = m_mapChannels[uiChannelID].sChannelName;
    vParams[1] = XtoA(false, true);
    ChatUnreadChannel.Trigger(vParams);

    for (uiset_it it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); it++)
    {
        if (!m_mapChannels[*it].bUnread)
            continue;

        uiNumUnread++;
    }

    ChatNumUnreadChannels.Trigger(XtoA(uiNumUnread));
}


/*====================
  CChatManager::SetFocusedChannel
  ====================*/
void    CChatManager::SetFocusedChannel(const tstring &sChannel, const bool bForceFocus)
{
    SetFocusedChannel(GetChannelID(sChannel), bForceFocus);
}


/*====================
  CChatManager::SetFocusedChannel
  ====================*/
void    CChatManager::SetFocusedChannel(const uint uiChannel, const bool bForceFocus)
{
    // when logging in in invisible mode, the default uiChannel is going to be the same as the m_uiFocusedChannel
    // bypass the return here and force the UI to update the channel to show the user is in the "Status" channel when logging in    
    if (bForceFocus)
    {
        tsvector vParam(2);
        vParam[0] = XtoA((int)uiChannel);
        vParam[1] = GetChannelName(uiChannel);
        ChatSetFocusChannel.Trigger(vParam);
        return; 
    }   
    
    if (uiChannel == m_uiFocusedChannel)
        return;

    uint uiOldFocus(m_uiFocusedChannel);

    m_uiFocusedChannel = uiChannel;

    if (m_uiFocusedChannel != -1)
        RemoveUnreadChannel(m_uiFocusedChannel);

    tsvector vParam(2);
    vParam[0] = XtoA((int)uiChannel);
    vParam[1] = GetChannelName(uiChannel);
    ChatSetFocusChannel.Trigger(vParam);

    if (uiOldFocus != -1)
        RemoveUnreadChannel(uiOldFocus);

    if (chat_debugInterface)
        Console.UI << _T("SetFocusedChannel ") << uiChannel << _T(" ") << QuoteStr(GetChannelName(uiChannel)) << newl;

    ChatChannelMap_it itFind(m_mapChannels.find(uiChannel));
    if (itFind != m_mapChannels.end())
        itFind->second.uiFocusPriority = m_uiFocusCount++;
}


/*====================
  CChatManager::SetNextFocusedChannel
  ====================*/
void    CChatManager::SetNextFocusedChannel()
{
    uint uiMaxFocusPriority(0);
    uint uiMaxChannelID(uint(-1));

    for (uiset_it it(m_setChannelsIn.begin()); it != m_setChannelsIn.end(); ++it)
    {
        ChatChannelMap_it itFind(m_mapChannels.find(*it));
        if (itFind == m_mapChannels.end())
            continue;

        if (itFind->second.uiFocusPriority >= uiMaxFocusPriority)
        {
            uiMaxChannelID = *it;
            uiMaxFocusPriority = itFind->second.uiFocusPriority;
        }
    }

    SetFocusedChannel(uiMaxChannelID);
}


/*====================
  CChatManager::SetFocusedIM
  ====================*/
void    CChatManager::SetFocusedIM(const tstring &sName)
{
    if (CompareNoCase(sName, m_sFocusedIM) == 0)
        return;

    if (chat_debugInterface)
        Console.UI << _T("SetFocusedIM - ") << sName << newl;

    tsvector vParams(2);
    vParams[0] = sName;
    vParams[1] = m_sFocusedIM;

    m_sFocusedIM = sName;
    ChatFocusedIM.Trigger(vParams);

    if (!sName.empty())
        m_mapIMFocusPriority[sName] = m_uiFocusCount++;
}


/*====================
  CChatManager::SetNextFocusedIM
  ====================*/
void    CChatManager::SetNextFocusedIM()
{
    uint uiMaxFocusPriority(0);
    tstring sMaxFocusedIM;

    for (IMCountMap_it it(m_mapIMFocusPriority.begin()); it != m_mapIMFocusPriority.end(); ++it)
    {
        if (it->second >= uiMaxFocusPriority)
        {
            sMaxFocusedIM = it->first;
            uiMaxFocusPriority = it->second;
        }
    }

    if (sMaxFocusedIM.empty())
        return;

    SetFocusedIM(sMaxFocusedIM);
}


/*====================
  CChatManager::CloseIM
  ====================*/
void    CChatManager::CloseIM(const tstring &sName)
{
    IMCountMap_it it(m_mapIMFocusPriority.find(sName));
    if (it != m_mapIMFocusPriority.end())
        m_mapIMFocusPriority.erase(it);

    ChatCloseIM.Trigger(sName);

    if (sName == m_sFocusedIM)
        m_sFocusedIM.clear();
}


/*====================
  CChatManager::SetChannelTopic
  ====================*/
void    CChatManager::SetChannelTopic(uint uiChannelID, const tstring &sTopic)
{
    // limit the topic length to avoid some channel name + topic UI problems
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_TOPIC << uiChannelID << sTopic.substr(0, CHAT_CHANNEL_TOPIC_MAX_LENGTH);
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::KickUserFromChannel
  ====================*/
void    CChatManager::KickUserFromChannel(uint uiChannelID, const tstring &sName)
{
    ChatClientMap_it it(m_mapUserList.begin());

    while (it != m_mapUserList.end())
    {
        if (CompareNames(it->second.sName, sName))
            break;

        it++;
    }

    if (it == m_mapUserList.end())
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_KICK << uiChannelID << it->first;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::BanUserFromChannel
  ====================*/
void    CChatManager::BanUserFromChannel(uint uiChannelID, const tstring &sName)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_BAN << uiChannelID << sName;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::UnbanUserFromChannel
  ====================*/
void    CChatManager::UnbanUserFromChannel(uint uiChannelID, const tstring &sName)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_UNBAN << uiChannelID << sName;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::SilenceChannelUser
  ====================*/
void    CChatManager::SilenceChannelUser(uint uiChannelID, const tstring &sName, uint uiDuration)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_SILENCE_USER << uiChannelID << sName << uiDuration;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::PromoteUserInChannel
  ====================*/
void    CChatManager::PromoteUserInChannel(uint uiChannelID, uint uiAccountID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_PROMOTE << uiChannelID << uiAccountID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::PromoteUserInChannel
  ====================*/
void    CChatManager::PromoteUserInChannel(uint uiChannelID, const tstring &sName)
{
    uint uiAccountID(-1);

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        uiAccountID = it->first;
        break;
    }

    if (uiAccountID == -1)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_PROMOTE << uiChannelID << uiAccountID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::DemoteUserInChannel
  ====================*/
void    CChatManager::DemoteUserInChannel(uint uiChannelID, uint uiAccountID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_DEMOTE << uiChannelID << uiAccountID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::DemoteUserInChannel
  ====================*/
void    CChatManager::DemoteUserInChannel(uint uiChannelID, const tstring &sName)
{
    uint uiAccountID(-1);

    for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
    {
        if (!CompareNames(it->second.sName, sName))
            continue;

        uiAccountID = it->first;
        break;
    }

    if (uiAccountID == -1)
        return;

    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_DEMOTE << uiChannelID << uiAccountID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::RequestAuthEnable
  ====================*/
void    CChatManager::RequestAuthEnable(uint uiChannelID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_SET_AUTH << uiChannelID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::RequestAuthDisable
  ====================*/
void    CChatManager::RequestAuthDisable(uint uiChannelID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_REMOVE_AUTH << uiChannelID;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::RequestAuthAdd
  ====================*/
void    CChatManager::RequestAuthAdd(uint uiChannelID, const tstring &sName)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_ADD_AUTH_USER << uiChannelID << sName;
    m_sockChat.SendPacket(pktSend);
}

/*====================
  CChatManager::RequestAuthRemove
  ====================*/
void    CChatManager::RequestAuthRemove(uint uiChannelID, const tstring &sName)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_REM_AUTH_USER << uiChannelID << sName;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::RequestAuthList
  ====================*/
void    CChatManager::RequestAuthList(uint uiChannelID)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_LIST_AUTH << uiChannelID;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::SetChannelPassword
  ====================*/
void    CChatManager::SetChannelPassword(uint uiChannelID, const tstring &sPassword)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_CHANNEL_SET_PASSWORD << uiChannelID << sPassword;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::SendGlobalMessage
  ====================*/
void    CChatManager::SendGlobalMessage(const tstring &sMessage)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_MESSAGE_ALL << sMessage;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::IsInChannel
  ====================*/
bool    CChatManager::IsInChannel(uint uiChannelID)
{
    return m_setChannelsIn.find(uiChannelID) != m_setChannelsIn.end();
}


/*====================
  CChatManager::InitCensor
  ====================*/
void    CChatManager::InitCensor()
{
    m_mapCensor.insert(pair<tstring, tstring>(_T("niggers"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("nigger"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fuk"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fuck"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fuking"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fucking"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fuker"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fucker"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fukers"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fuckers"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("motherfuker"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("motherfucker"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("motherfukers"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("motherfuckers"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("dick"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("dong"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("cock"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("cocks"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("bitch"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("bitches"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("bitching"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("son of a bitch"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("bullshit"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("shit"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fag"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("fags"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("faggot"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("faggots"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("ass hole"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("asshole"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("cunt"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("cunts"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("pussy"), _T("****")));    
    m_mapCensor.insert(pair<tstring, tstring>(_T("jesus christ"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("goddamn"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("god damn"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("god damn it"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("hell"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("damn"), _T("****")));
    m_mapCensor.insert(pair<tstring, tstring>(_T("bastard"), _T("****")));
}


/*====================
  CChatManager::CensorChat

  TODO: Make this function more thorough!
  ====================*/
bool    CChatManager::CensorChat(tstring &sMessage, bool bInGameChat)
{
    bool bCensored(false);
    bool bFound(true);
    tstring::size_type pos(0);
    tstring sStartingChars;
    
    // Strip off everything before the space - ""^444PlayerName:^* "
    if (!bInGameChat)
    {
        pos = sMessage.find_first_of(_T(" "));
        if (pos != tstring::npos)
        {
            sStartingChars = sMessage.substr(0, pos);
            sMessage = sMessage.substr(pos);
        }
        
        pos = 0;
    }
    
    tstring sLower(StripColorCodes(LowerString(sMessage))); 
    
    while (bFound)
    {
        bFound = false;

        for (tsmapts::reverse_iterator it(m_mapCensor.rbegin()); it != m_mapCensor.rend(); it++)
        {
            pos = sLower.find(it->first);

            if (pos != tstring::npos)
            {
                // Only censor if it is not part of a larger word
                if ((pos == 0 || sLower[pos - 1] == _T(' ') || (IsNotDigit(sLower[pos - 1]) && !IsLetter(sLower[pos - 1]))) &&
                    (pos + it->first.length() == sLower.length() || sLower[pos + it->first.length()] == _T(' ') || 
                    (IsNotDigit(sLower[pos + it->first.length()]) && !IsLetter(sLower[pos + it->first.length()]))))
                {
                    sLower.erase(pos, it->first.length());
                    sMessage.erase(pos, it->first.length());

                    sLower.insert(pos, it->second);
                    sMessage.insert(pos, it->second);

                    bFound = true;
                    bCensored = true;
                }
            }
        }
    }
    
    // Re-add ""^444PlayerName:^* " back to the beginning
    if (!bInGameChat)
    {
        sMessage = sStartingChars + sMessage;
    }

    return bCensored;
}


/*====================
  CChatManager::AddUnreadIM
  ====================*/
uint    CChatManager::AddUnreadIM(const tstring &sName)
{
    ChatUnreadIM.Trigger(sName);

    IMCountMap_it it(m_mapIMUnreadCount.find(sName));
    if (it != m_mapIMUnreadCount.end())
    {
        ++it->second;
        return it->second;
    }
    else
    {
        m_mapIMUnreadCount[sName] = 1;
        return 1;
    }
}


/*====================
  CChatManager::RemoveUnreadIMs
  ====================*/
uint    CChatManager::RemoveUnreadIMs(const tstring &sName)
{   
    IMCountMap_it it(m_mapIMUnreadCount.find(sName));
    if (it != m_mapIMUnreadCount.end())
    {
        uint uiUnread(it->second);
        STL_ERASE(m_mapIMUnreadCount, it);

        return uiUnread;
    }
    else
    {
        return 0;
    }
}


/*====================
  CChatManager::IsFollowing
  ====================*/
bool    CChatManager::IsFollowing(const tstring &sName)
{
    if (m_bFollow && m_sFollowName == RemoveClanTag(sName))
        return true;
        
    return false;
}


/*====================
  CChatManager::RequestRefreshUpgrades
  ====================*/
void    CChatManager::RequestRefreshUpgrades()
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_REFRESH_UPGRADES;
    m_sockChat.SendPacket(pktSend);
}


/*====================
  CChatManager::SubmitChatMessage

  PLEASE NOTE: I know this function is messy. It was copied directly from
  CIRCManager and modified slightly to retain functionality. It will be
  cleaned up and restructured shortly. *FIX ME*
  ====================*/
bool    CChatManager::SubmitChatMessage(const tstring &sMessage, uint uiChannelID)
{
    if (!IsConnected())
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_connected")));
        return false;
    }

    //Note that the commands can all be concatinated
    //when they come to us, so we have to tokenize them
    //again... Hence the seemingly useless line.
    tsvector vsTokens(TokenizeString(sMessage, ' '));

    if (vsTokens.size() < 1)
    {
        m_vChatHistory.push_front(sMessage);
        m_uiHistoryPos = 0;
        return true;
    }

    //If they want to whisper, do so
    if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_whisper"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_whisper_short"))) == 0)
    {
        if (vsTokens.size() >= 3 && !vsTokens[1].empty())
        {
            m_vChatHistory.push_front(sMessage);
            m_uiHistoryPos = 0;

            return SendWhisper(vsTokens[1], ConcatinateArgs(vsTokens.begin() + 2, vsTokens.end()));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_whisper_help")));
        }
    }
    //Replying to last whisper
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_reply"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_reply_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (!m_lLastWhispers.empty())
            {
                m_vChatHistory.push_front(sMessage);
                m_uiHistoryPos = 0;
                    
                return SendWhisper(m_lLastWhispers.front(), ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
            }
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_reply_invalid")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_reply_help")));
        }
    }
    //Buddy list commands
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_buddy"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_buddy_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_list_message")));

                if (m_setBuddyList.size() > 0)
                {
                    for (uiset_it it(m_setBuddyList.begin()); it != m_setBuddyList.end(); it++)
                        if (m_mapUserList[*it].yStatus == CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_online"), _T("name"), m_mapUserList[*it].sName));

                    for (uiset_it it(m_setBuddyList.begin()); it != m_setBuddyList.end(); it++)
                        if (m_mapUserList[*it].yStatus > CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_in_game"), _T("name"), m_mapUserList[*it].sName, _T("game"), m_mapUserList[*it].sGameName));

                    for (uiset_it it(m_setBuddyList.begin()); it != m_setBuddyList.end(); it++)
                        if (m_mapUserList[*it].yStatus < CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_offline"), _T("name"), m_mapUserList[*it].sName));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_message"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_message_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {               
                    if (GetChatModeType() == CHAT_MODE_INVISIBLE)
                    {
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_failed_invisible")));
                        return true;        
                    }   
                
                    tstring sMessage(ConcatinateArgs(vsTokens.begin() + 2, vsTokens.end()));

                    CPacket pktSend;
                    pktSend << CHAT_CMD_WHISPER_BUDDIES << sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH);
                    m_sockChat.SendPacket(pktSend);

                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_whisper_buddies"), _T("message"), sMessage.substr(0, CHAT_MESSAGE_MAX_LENGTH)));
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_message_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_add_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestBuddyAdd(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_add_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_buddy_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestBuddyRemove(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_delete_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_list_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_message_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_list_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_buddy_message_help")));
        }
    }
    //Clan list commands
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_clan"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_clan_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_list_message")));

                if (m_setClanList.size() > 0)
                {
                    for (uiset_it it(m_setClanList.begin()); it != m_setClanList.end(); it++)
                        if (m_mapUserList[*it].yStatus == CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_online"), _T("name"), m_mapUserList[*it].sName));

                    for (uiset_it it(m_setClanList.begin()); it != m_setClanList.end(); it++)
                        if (m_mapUserList[*it].yStatus > CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_in_game"), _T("name"), m_mapUserList[*it].sName, _T("game"), m_mapUserList[*it].sGameName));

                    for (uiset_it it(m_setClanList.begin()); it != m_setClanList.end(); it++)
                        if (m_mapUserList[*it].yStatus < CHAT_STATUS_CONNECTED)
                            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_user_info_offline"), _T("name"), m_mapUserList[*it].sName));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_message"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_message_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    m_vChatHistory.push_front(sMessage);
                    m_uiHistoryPos = 0;

                    return SendClanWhisper(ConcatinateArgs(vsTokens.begin() + 2, vsTokens.end()));
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_message_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_promote"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_promote_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    RequestPromoteClanMember(vsTokens[2]);
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_promote_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_demote"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_demote_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    RequestDemoteClanMember(vsTokens[2]);
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_demote_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_remove"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_remove_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    RequestRemoveClanMember(vsTokens[2]);
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_remove_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_invite"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_invite_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    InviteToClan(vsTokens[2]);
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_invite_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_leave"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_clan_leave_short"))) == 0)
            {
                if (!m_mapUserList[m_uiAccountID].sClan.empty())
                    RequestRemoveClanMember(m_mapUserList[m_uiAccountID].sName);
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_clan_no_clan")));
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_list_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_message_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_promote_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_demote_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_remove_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_invite_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_leave_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_list_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_message_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_promote_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_demote_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_remove_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_invite_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_clan_leave_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_clear"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_clear_short"))) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_CLEAR);
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_join"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_join_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            tstring sChannel(ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
            const size_t zPos(sChannel.find(_T("\"")));
            
            // if no password enclosed in quotes found, join normally
            if (zPos == tstring::npos)
                JoinChannel(sChannel);
            else
            {
                // grab full channel name by taking everything after the / command up until the space before the first quote
                tstring sTempChannel(sChannel);
                sChannel = Trim(StringReplace(sChannel.substr(0, zPos-1), _T("\""), _T("")));
                
                // anything in quotes at the end becomes the password
                tstring sPassword(Trim(StringReplace(sTempChannel.substr(zPos), _T("\""), _T(""))));
                if (!sChannel.empty() && !sPassword.empty())
                    JoinChannel(sChannel, sPassword);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_join_help")));
                }
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_join_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_invite"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_invite_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
            InviteUser(vsTokens[1]);
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invite_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_whois"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_whois_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
            GetUserInfo(vsTokens[1]);
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_whois_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_stats"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_stats_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (vsTokens.size() >= 3 && CompareNoCase(vsTokens[2], Translate(_T("chat_command_stats_pop"))) == 0)
            {
                Console.Execute(_T("ShowCCPanel"));
                Console.Execute(_T("ShowCCStatistics true"));
            }
            else
                SetRetrievingStats(true);

            Console.Execute(_T("GetPlayerStatsName ") + vsTokens[1]);
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_stats_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_joingame"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_joingame_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_buddy"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_buddy_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    if (!JoinGame(vsTokens[2]))
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_buddy_failed")));
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_buddy_help")));
                }               
            }
/*          else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_name"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_name_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    // TODO: Join game by name here
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_name_help")));
                }
            }*/
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_ip"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_joingame_ip_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    Host.Connect(vsTokens[2]);
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_ip_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_buddy_help")));
                //AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_name_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_ip_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_buddy_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_name_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_joingame_ip_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_topic"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_topic_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (IsAdmin(uiChannelID, m_uiAccountID))
                SetChannelTopic(uiChannelID, ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        }
        else if (uiChannelID != -1)
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_topic"), _T("topic"), m_mapChannels[uiChannelID].sTopic), GetChannelName(uiChannelID));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_kick"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_kick_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (IsAdmin(uiChannelID, m_uiAccountID))
                KickUserFromChannel(uiChannelID, vsTokens[1]);
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_kick_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_ban"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (IsAdmin(uiChannelID, m_uiAccountID))
                BanUserFromChannel(uiChannelID, vsTokens[1]);
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ban_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_unban"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (IsAdmin(uiChannelID, m_uiAccountID))
                UnbanUserFromChannel(uiChannelID, vsTokens[1]);
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_unban_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_silence"))) == 0)
    {
        if (vsTokens.size() >= 3)
        {
            if (IsAdmin(uiChannelID, m_uiAccountID))
                SilenceChannelUser(uiChannelID, vsTokens[1], MinToMs(uint(AtoI(vsTokens[2]))));
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_silence_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_message_all"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_message_all_long"))) == 0)
    {
        if (vsTokens.size() >= 2)
            SendGlobalMessage(ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_banlist"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_banlist_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_message")));

                if (m_mapBanList.size() > 0)
                {
                    for (ChatBanMap_it it(m_mapBanList.begin()); it != m_mapBanList.end(); it++)
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_entry"), _T("id"), XtoA(it->first), _T("name"), it->second.sName, _T("reason"), it->second.sReason));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_add_short"))) == 0)
            {
                if (vsTokens.size() >= 4)
                    RequestBanlistAdd(vsTokens[2], ConcatinateArgs(vsTokens.begin() + 3, vsTokens.end()));
                else
                {
                    if (vsTokens.size() >= 3)
                        RequestBanlistAdd(vsTokens[2], _T("None"));
                    else
                    {
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
                    }
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestBanlistRemove(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_banlist"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_banlist_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_message")));

                if (m_mapBanList.size() > 0)
                {
                    for (ChatBanMap_it it(m_mapBanList.begin()); it != m_mapBanList.end(); it++)
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_entry"), _T("id"), XtoA(it->first), _T("name"), it->second.sName, _T("reason"), it->second.sReason));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_add_short"))) == 0)
            {
                if (vsTokens.size() >= 4)
                    RequestBanlistAdd(vsTokens[2], ConcatinateArgs(vsTokens.begin() + 3, vsTokens.end()));
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_banlist_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestBanlistRemove(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_banlist_list_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_ignore"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_ignore_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_list_message")));

                if (m_mapIgnoreList.size() > 0)
                {
                    for (ChatIgnoreMap_it it(m_mapIgnoreList.begin()); it != m_mapIgnoreList.end(); it++)
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_list_entry"), _T("id"), XtoA(it->first), _T("name"), it->second));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_add_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestIgnoreAdd(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_add_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_ignore_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestIgnoreRemove(vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_delete_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_list_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignore_list_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_notes"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_notes_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_list_message")));

                if (!m_vNotes.empty())
                {
                    uint uiNoteNum(1);

                    for (tsvector_it it(m_vNotes.begin()); it != m_vNotes.end(); it++)
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_list_entry"), _T("note"), *it, _T("id"), XtoA(uiNoteNum++)));
                }
                else
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_list_none")));
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_add_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    m_cDate = CDate(true);

                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_add_success")));
                    m_vNotes.push_back(ConcatinateArgs(vsTokens.begin() + 2, vsTokens.end()));
                    m_vNoteTimes.push_back(m_cDate.GetDateString(DATE_SHORT_YEAR | DATE_YEAR_LAST | DATE_MONTH_FIRST) + _T(" @ ") + m_cDate.GetTimeString());
                    SaveNotes();
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_add_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_notes_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                {
                    uint uiNoteNum(AtoI(vsTokens[2]));

                    if (uiNoteNum > 0 && INT_SIZE(m_vNotes.size()) >= uiNoteNum)
                    {
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_delete_success")));
                        m_vNotes.erase(m_vNotes.begin() + (uiNoteNum - 1));
                        SaveNotes();
                    }
                    else
                        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_delete_invalid")));
                }
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_delete_help")));
                }
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_list_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_notes_list_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_promote"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_promote_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            uint uiAccountID(-1);
            bool bFound(false);

            for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
            {
                if (!CompareNames(it->second.sName, vsTokens[1]))
                    continue;

                if (it->second.setChannels.find(uiChannelID) == it->second.setChannels.end())
                    break;

                bFound = true;

                if (GetAdminLevel(uiChannelID, m_uiAccountID) > GetAdminLevel(uiChannelID, it->second.uiAccountID) + 1)
                    uiAccountID = it->second.uiAccountID;

                break;
            }

            if (uiAccountID != -1)
                PromoteUserInChannel(uiChannelID, uiAccountID);
            else if (bFound)
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_promote_failure")));
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_in_channel")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_promote_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_demote"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_demote_short"))) == 0)
    {
        if (vsTokens.size() >= 2)
        {
            uint uiAccountID(-1);
            bool bFound(false);
            bool bLowest(false);

            for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
            {
                if (!CompareNames(it->second.sName, vsTokens[1]))
                    continue;

                if (it->second.setChannels.find(uiChannelID) == it->second.setChannels.end())
                    break;

                bFound = true;
                bLowest = (GetAdminLevel(uiChannelID, it->second.uiAccountID) == CHAT_CLIENT_ADMIN_NONE);

                if (GetAdminLevel(uiChannelID, m_uiAccountID) > GetAdminLevel(uiChannelID, it->second.uiAccountID))
                    uiAccountID = it->second.uiAccountID;

                break;
            }

            if (bLowest)
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_demote_failure_too_low")));
            else if (uiAccountID != -1)
                DemoteUserInChannel(uiChannelID, uiAccountID);
            else if (bFound)
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_demote_failure")));
            else
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_in_channel")));
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_demote_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_auth"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_auth_short"))) == 0)
    {
        if (GetAdminLevel(uiChannelID, m_uiAccountID) < CHAT_CLIENT_ADMIN_LEADER)
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_no_permissions")));
        }
        else if (vsTokens.size() >= 2)
        {
            if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_list"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_list_short"))) == 0)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_list_message")));
                RequestAuthList(uiChannelID);
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_add"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_add_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestAuthAdd(uiChannelID, vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_add_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_delete"))) == 0 || CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_delete_short"))) == 0)
            {
                if (vsTokens.size() >= 3)
                    RequestAuthRemove(uiChannelID, vsTokens[2]);
                else
                {
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format")));
                    AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_delete_help")));
                }
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_enable"))) == 0)
            {
                RequestAuthEnable(uiChannelID);
            }
            else if (CompareNoCase(vsTokens[1], Translate(_T("chat_command_auth_disable"))) == 0)
            {
                RequestAuthDisable(uiChannelID);
            }
            else
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_delete_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_add_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_list_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_enable_help")));
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_disable_help")));
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_format_multi")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_delete_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_add_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_list_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_enable_help")));
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_auth_disable_help")));
        }
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_password"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_password_short"))) == 0)
    {
        if (!IsAdmin(uiChannelID, m_uiAccountID))
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_not_operator")));
        else if (vsTokens.size() >= 2)
            SetChannelPassword(uiChannelID, ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
        else
            SetChannelPassword(uiChannelID, TSNULL);
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_roll"))) == 0)
    {   
        if (vsTokens.size() == 2)
        {
            if (AtoF(vsTokens[1]) <= 0 || AtoF(vsTokens[1]) > 32767)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_roll_help")));
            }
            else
            {               
                const uint uiRand(M_Randnum(1, AtoI(vsTokens[1])));
                
                ChatClientMap_it findit(m_mapUserList.find(m_uiAccountID));
                
                const tstring sRollMessage = Translate(_T("chat_roll_message"), _T("player"), findit->second.sName, _T("low"), _T("1"), _T("high"), XtoA(AtoI(vsTokens[1])), _T("number"), XtoA(uiRand));
                
                if (!sRollMessage.empty() && uiChannelID != -1)
                {
                    m_vChatHistory.push_front(vsTokens[0] + _T(" ") + vsTokens[1]);
                    m_uiHistoryPos = 0;

                    return SendChannelMessage(sRollMessage, uiChannelID, CHAT_MESSAGE_ROLL);
                }
            }
        }   
        else if (vsTokens.size() >= 3)
        {
            if (AtoF(vsTokens[1]) <= 0 || AtoF(vsTokens[2]) <= 0 || AtoF(vsTokens[2]) <= AtoF(vsTokens[1]) || AtoF(vsTokens[2]) > 32767)
            {
                AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_roll_help")));
            }
            else
            {               
                const uint uiRand(M_Randnum(AtoI(vsTokens[1]), AtoI(vsTokens[2])));
                
                ChatClientMap_it findit(m_mapUserList.find(m_uiAccountID));
                
                const tstring sRollMessage = Translate(_T("chat_roll_message"), _T("player"), findit->second.sName, _T("low"), XtoA(vsTokens[1]), _T("high"), XtoA(vsTokens[2]), _T("number"), XtoA(uiRand));
                
                if (!sRollMessage.empty() && uiChannelID != -1)
                {
                    m_vChatHistory.push_front(vsTokens[0] + _T(" ") + vsTokens[1] + _T(" ") + vsTokens[2]);
                    m_uiHistoryPos = 0;

                    return SendChannelMessage(sRollMessage, uiChannelID, CHAT_MESSAGE_ROLL);
                }
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_roll_help")));
        }   
    }   
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_emote"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_emote_short"))) == 0)
    {
        if (vsTokens.size() > 1)
        {       
            ChatClientMap_it findit(m_mapUserList.find(m_uiAccountID));
                        
            const tstring sEmoteMessage = findit->second.sName + _T(" ") + ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end());
            
            if (!sEmoteMessage.empty() && uiChannelID != -1)
            {
                m_vChatHistory.push_front(ConcatinateArgs(vsTokens.begin(), vsTokens.end()));
                m_uiHistoryPos = 0;

                return SendChannelMessage(sEmoteMessage, uiChannelID, CHAT_MESSAGE_EMOTE);
            }
        }
        else
        {
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_emote_help")));
        }   
    }       
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_available"))) == 0)
    {
        SetChatModeType(CHAT_MODE_AVAILABLE, _T("chat_command_available_message")); 
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_afk"))) == 0)
    {
        if (vsTokens.size() == 1)
        {
            if (GetChatModeType() == CHAT_MODE_AFK) 
            {
                SetChatModeType(CHAT_MODE_AVAILABLE, _T("chat_command_available_message")); 
            }
            else
            {
                SetChatModeType(CHAT_MODE_AFK, Translate(_T("chat_mode_afk_default_response")));
            }       
        }                   
        else if (vsTokens.size() > 1)
        {                   
            SetChatModeType(CHAT_MODE_AFK, ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
        }
    }   
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_dnd"))) == 0)
    {
        if (vsTokens.size() == 1)
        {
            if (GetChatModeType() == CHAT_MODE_DND) 
            {
                SetChatModeType(CHAT_MODE_AVAILABLE, _T("chat_command_available_message")); 
            }
            else
            {
                SetChatModeType(CHAT_MODE_DND, Translate(_T("chat_mode_dnd_default_response")));
            }       
        }                   
        else if (vsTokens.size() > 1)
        {                   
            SetChatModeType(CHAT_MODE_DND, ConcatinateArgs(vsTokens.begin() + 1, vsTokens.end()));
        }
    }   
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_time"))) == 0)
    {
        m_cDate = CDate(true);
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_current_time"), _T("time"), m_cDate.GetTimeString() + _T(" ") + m_cDate.GetDateString(DATE_MONTH_FIRST)));
    }
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_ignorechat"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_ignorechat_short"))) == 0)
    {
        if (GetIgnoreChat() == CHAT_IGNORE_NONE) 
        {
            SetIgnoreChat(CHAT_IGNORE_ENEMY_ALL); // ignore enemy all chat
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignorechat_enemy_all_on")));
        }
        else if (GetIgnoreChat() == CHAT_IGNORE_ENEMY_ALL) 
        {
            SetIgnoreChat(CHAT_IGNORE_ALL); // ignore all chat
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignorechat_all_on")));
        }
        else if (GetIgnoreChat() == CHAT_IGNORE_ALL) 
        {
            SetIgnoreChat(CHAT_IGNORE_TEAM); // ignore team chat
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignorechat_team_on")));
        }
        else if (GetIgnoreChat() == CHAT_IGNORE_TEAM)
        {
            SetIgnoreChat(CHAT_IGNORE_EVERYONE); // ignore every one
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignorechat_every_on")));
        }
        else if (GetIgnoreChat() == CHAT_IGNORE_EVERYONE)
        {
            SetIgnoreChat(CHAT_IGNORE_NONE); // ignore no chat
            AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_ignorechat_off")));
        }
    }
    else if (CompareNoCase(vsTokens[0], _T("/AdminKick")) == 0)
    {
        AdminKick(vsTokens[1]);
    }   
    else if (CompareNoCase(vsTokens[0], Translate(_T("chat_command_help"))) == 0 || CompareNoCase(vsTokens[0], Translate(_T("chat_command_help_short"))) == 0)
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_valid")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_whisper")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_reply")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_buddy")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_clan")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_join")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_clear")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_invite")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_whois")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_stats")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_joingame")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_topic")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_kick")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_ban")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_unban")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_silence")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_banlist")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_ignore")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_ignorechat")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_notes")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_promote")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_demote")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_auth")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_password")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_password_clear")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_roll")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_emote")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_time")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_ping")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_available")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_afk")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_dnd")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_matchup")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_gameinfo")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_misc")));
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_help_weather")));
    }
    else if (vsTokens[0].substr(0, 1) == Translate(_T("chat_command_character")))
    {
        AddIRCChatMessage(CHAT_MESSAGE_ADD, Translate(_T("chat_command_invalid_command"), _T("helpcommand"), Translate(_T("chat_command_help"))));
    }
    else
    {
        if (!sMessage.empty() && uiChannelID != -1)
        {
            m_vChatHistory.push_front(sMessage);
            m_uiHistoryPos = 0;

            return SendChannelMessage(sMessage, uiChannelID);
        }
    }

    m_vChatHistory.push_front(sMessage);
    m_uiHistoryPos = 0;

    return true;
}


/*====================
  CChatManager::UpdateFollow
  ====================*/
void CChatManager::UpdateFollow(const tstring &sServer)
{
    if (!m_bFollow)
        return;

    ChatClientMap_it itLocalClient(m_mapUserList.find(m_uiAccountID));
    
    if (sServer.empty())
        Host.Disconnect(_T("CChatManager::UpdateFollow"));
    else if (sServer != itLocalClient->second.sServerAddressPort)// && Host.GetConnectedAddress() != itLocalClient->second.sServerAddressPort)
        Host.Connect(sServer);
}


/*====================
  CChatManager::SetFollowing
  ====================*/
bool CChatManager::SetFollowing(const tstring &sName)
{
    if (IsBuddy(sName) || IsClanMember(sName))
    {
        m_bFollow = true;
        m_sFollowName = sName;
        
        // Loop over all the clients we have chat info on
        for (ChatClientMap_it it(m_mapUserList.begin()); it != m_mapUserList.end(); it++)
        {
            // If one of the clients we have chat info on matches the player we are trying to follow
            if (RemoveClanTag(it->second.sName) == RemoveClanTag(sName))
            {
                // If the server/address is set, then try to connect to it but check to makee sure we aren't already 
                // connected to the server the one we are following is connected to
                if (!it->second.sServerAddressPort.empty())// && Host.GetConnectedAddress() != it->second.sServerAddressPort)
                    Host.Connect(it->second.sServerAddressPort);
                else
                    Host.Disconnect(_T("CChatManager::SetFollowing"));
            }
        }
        
        return true;
    }
    
    return false;
}


/*====================
  CChatManager::GetFollowing
  ====================*/
tstring CChatManager::GetFollowing()
{
    if (!m_bFollow)
        return _T("");

    return m_sFollowName;
}


/*====================
  CChatManager::UnFollow
  ====================*/
void CChatManager::UnFollow()
{
    m_bFollow = false;
    m_sFollowName = _T("");
}


/*====================
  CChatManager::TabChatMessage
  ====================*/
tstring CChatManager::TabChatMessage(const tstring &sMessage)
{
    tstring m_sCurrentMessage(sMessage);

    if (m_bWhisperMode)
    {
        ++m_uiTabNumber;
        if (m_uiTabNumber > m_lLastWhispers.size() - 1)
            m_uiTabNumber = 0;
        
        uint uiTempCount(0);
        list<tstring>::iterator it(m_lLastWhispers.begin());
        while (uiTempCount < m_uiTabNumber)
        {
            ++it;
            ++uiTempCount;
        }

        m_sCurrentMessage = Translate(_T("chat_command_whisper_short")) + _T(" ") + (*it) + _T(" ");
    }

    return m_sCurrentMessage;
}


/*====================
  CChatManager::UpdateClientChannelStatus
  ====================*/
void    CChatManager::UpdateClientChannelStatus(const tstring &sNewChannel, const tstring &sName, uint uiAccountID, byte yStatus, byte yFlags, uint uiChatSymbol, uint uiChatNameColor, uint uiAccountIcon)
{
    ChatClientMap_it it(m_mapUserList.find(uiAccountID));

    if (it == m_mapUserList.end())
        return;

    bool bChannelUpdate(false);
    
    if (it->second.sName != sName ||
        it->second.yStatus != yStatus ||
        it->second.uiAccountID != uiAccountID ||
        it->second.uiChatSymbol != uiChatSymbol || 
        it->second.uiChatNameColor != uiChatNameColor ||
        it->second.uiAccountIcon != uiAccountIcon ||
        it->second.yFlags != yFlags)
        bChannelUpdate = true;

    it->second.sName = sName;
    it->second.yStatus = yStatus;
    it->second.uiAccountID = uiAccountID;
    it->second.yFlags = yFlags;
    it->second.uiChatSymbol = uiChatSymbol;
    it->second.uiChatNameColor = uiChatNameColor;
    it->second.uiAccountIcon = uiAccountIcon;

    uint uiChatNameColor2(uiChatNameColor);

    if (yFlags & CHAT_CLIENT_IS_STAFF && uiChatNameColor2 == INVALID_INDEX)
    {
        uint uiDevChatNameColor(Host.LookupChatNameColor(_CTS("s2logo")));
        if (uiDevChatNameColor != INVALID_INDEX)
            uiChatNameColor2 = uiDevChatNameColor;
    }
    if (yFlags & CHAT_CLIENT_IS_PREMIUM && uiChatNameColor2 == INVALID_INDEX)
    {
        uint uiGoldChatNameColor(Host.LookupChatNameColor(_CTS("goldshield")));
        if (uiGoldChatNameColor != INVALID_INDEX)
            uiChatNameColor2 = uiGoldChatNameColor;
    }

    if (uiChatNameColor2 != INVALID_INDEX)
        it->second.uiSortIndex = Host.GetChatNameColorSortIndex(uiChatNameColor2);
    else
        it->second.uiSortIndex = 9;

    if (bChannelUpdate || !sNewChannel.empty())
    {
        static tsvector vParams(12);
        static tsvector vMiniParams(2);

        for (uiset_it itChan(it->second.setChannels.begin()); itChan != it->second.setChannels.end(); ++itChan)
        {
            bool bNewChannel(!sNewChannel.empty() && GetChannelName(*itChan) == sNewChannel);

            if (!bNewChannel && m_setChannelsIn.find(*itChan) == m_setChannelsIn.end())
                continue;

            // These stay the same throughout the rest of the function
            vParams[0] = vMiniParams[0] = GetChannelName(*itChan);

            if (!bNewChannel)
            {
                vMiniParams[1] = _T("EraseListItemByValue('") + it->second.sName + _T("');");
                ChatUserEvent.Trigger(vMiniParams);
            }

            if (it->second.yStatus > CHAT_STATUS_DISCONNECTED)
            {
                vParams[1] = it->second.sName;
                vParams[2] = XtoA(GetAdminLevel(*itChan, it->first));
                vParams[3] = XtoA(it->second.yStatus > CHAT_STATUS_CONNECTED, true);
                vParams[4] = XtoA((it->second.yFlags & CHAT_CLIENT_IS_PREMIUM) != 0, true);
                vParams[5] = XtoA(it->second.uiAccountID);
                vParams[6] = Host.GetChatSymbolTexturePath(it->second.uiChatSymbol);
                vParams[7] = Host.GetChatNameColorTexturePath(it->second.uiChatNameColor);
                vParams[8] = Host.GetChatNameColorString(it->second.uiChatNameColor);
                vParams[9] = Host.GetChatNameColorIngameString(it->second.uiChatNameColor);
                vParams[10] = Host.GetAccountIconTexturePath(it->second.uiAccountIcon);
                vParams[11] = XtoA(it->second.uiSortIndex);
                ChatUserNames.Trigger(vParams);
            }

            vMiniParams[1] = _T("SortListboxSortIndex();");
            ChatUserEvent.Trigger(vMiniParams);
        }
    }
}


/*====================
  CChatManager::CreateTMMGroup
  ====================*/
void    CChatManager::CreateTMMGroup(const byte yGameType, const tstring &sMapName, const tstring &sGameModes, const tstring &sRegions)
{
    Console << _T("Creating TMM Group") << newl;
    Console << _T("GameType: ") << yGameType << newl;
    Console << _T("MapName: ") << sMapName << newl;
    Console << _T("GameModes: ") << sGameModes << newl;
    Console << _T("Regions: ") << sRegions << newl;

    m_bInGroup = false;
    m_uiTMMGroupLeaderID = INVALID_INDEX;
    m_bTMMOtherPlayersReady = false;
    m_bTMMAllPlayersReady = false;
    m_bTMMMapLoaded = false;
    
    UnFollow();
        
    for (uint ui(0); ui < MAX_GROUP_SIZE; ++ui)
        m_aGroupInfo[ui].Clear();

    m_uiTMMSelfGroupIndex = 0;

    m_uiTMMStartTime = INVALID_TIME;
    m_uiTMMAverageQueueTime = INVALID_TIME;
    m_uiTMMStdDevQueueTime = INVALID_TIME;

    UpdateReadyStatus();

    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_CREATE << K2_Version3(K2System.GetVersionString()) << yGameType << sMapName << sGameModes << sRegions;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  CreateTMMGroup
  --------------------*/
CMD(CreateTMMGroup)
{
    if (vArgList.size() < 1)
        return false;

    if (vArgList.size() < 4)
    {
        ChatManager.CreateTMMGroup(AtoI(vArgList[0]));
        return false;
    }

    if (vArgList.size() >= 4)
        ChatManager.CreateTMMGroup(AtoI(vArgList[0]), vArgList[1], vArgList[2], vArgList[3]);

    return true;
}


/*--------------------
  CreateTMMGroup
  --------------------*/
UI_VOID_CMD(CreateTMMGroup, 1)
{
    if (vArgList.size() < 1)
        return;

    if (vArgList.size() < 4)
    {
        ChatManager.CreateTMMGroup(AtoI(vArgList[0]->Evaluate()));
        return;
    }

    if (vArgList.size() >= 4)
        ChatManager.CreateTMMGroup(AtoI(vArgList[0]->Evaluate()), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), vArgList[3]->Evaluate());
        
    return;
}


/*====================
  CChatManager::JoinTMMGroup
  ====================*/
void CChatManager::JoinTMMGroup(const tstring sNickname)
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_JOIN << K2_Version3(K2System.GetVersionString()) << sNickname;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  JoinTMMGroup
  --------------------*/
CMD(JoinTMMGroup)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.JoinTMMGroup(vArgList[0]);
    return true;
}


/*--------------------
  JoinTMMGroup
  --------------------*/
UI_VOID_CMD(JoinTMMGroup, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.JoinTMMGroup(vArgList[0]->Evaluate());
}


/*====================
  CChatManager::LeaveTMMGroup
  ====================*/
void    CChatManager::LeaveTMMGroup(bool bLocalOnly, const tstring &sReason)
{
    bool bWasInGroup(m_bInGroup);

    m_bInGroup = false;
    m_uiTMMGroupLeaderID = INVALID_INDEX;
    m_bTMMOtherPlayersReady = false;
    m_bTMMAllPlayersReady = false;
    m_bTMMMapLoaded = false;
        
    for (uint ui(0); ui < MAX_GROUP_SIZE; ++ui)
        m_aGroupInfo[ui].Clear();

    m_uiTMMSelfGroupIndex = 0;

    m_uiTMMStartTime = INVALID_TIME;
    m_uiTMMAverageQueueTime = INVALID_TIME;
    m_uiTMMStdDevQueueTime = INVALID_TIME;

    UpdateReadyStatus();

    tsvector vParams(2);
    vParams[0] = sReason;
    vParams[1] = XtoA(bWasInGroup);

    TMMLeaveGroup.Trigger(vParams, cc_forceTMMInterfaceUpdate);

    Console << _T("Left TMM Group - ") << (!sReason.empty() ? sReason : _T("NULL")) << newl;

    if (!bLocalOnly)
    {
        CPacket pktSend;
        pktSend << NET_CHAT_CL_TMM_GROUP_LEAVE;
        m_sockChat.SendPacket(pktSend);
    }
}


/*--------------------
  LeaveTMMGroup
  --------------------*/
CMD(LeaveTMMGroup)
{
    ChatManager.LeaveTMMGroup();
    return true;
}


/*--------------------
  LeaveTMMGroup
  --------------------*/
UI_VOID_CMD(LeaveTMMGroup, 0)
{
    ChatManager.LeaveTMMGroup();
}


/*====================
  CChatManager::InviteToTMMGroup
  ====================*/
void CChatManager::InviteToTMMGroup(const tstring sNickname)
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_INVITE << sNickname;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  InviteToTMMGroup
  --------------------*/
CMD(InviteToTMMGroup)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.InviteToTMMGroup(vArgList[0]);
    return true;
}


/*--------------------
  InviteToTMMGroup
  --------------------*/
UI_VOID_CMD(InviteToTMMGroup, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.InviteToTMMGroup(vArgList[0]->Evaluate());
}


/*====================
  CChatManager::JoinTMMQueue
  ====================*/
void CChatManager::JoinTMMQueue()
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_JOIN_QUEUE;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  JoinTMMQueue
  --------------------*/
CMD(JoinTMMQueue)
{
    ChatManager.JoinTMMQueue();
    return true;
}


/*--------------------
  JoinTMMQueue
  --------------------*/
UI_VOID_CMD(JoinTMMQueue, 0)
{
    ChatManager.JoinTMMQueue();
}


/*====================
  CChatManager::LeaveTMMQueue
  ====================*/
void CChatManager::LeaveTMMQueue()
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_LEAVE_QUEUE;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  LeaveTMMQueue
  --------------------*/
CMD(LeaveTMMQueue)
{
    ChatManager.LeaveTMMQueue();
    return true;
}


/*--------------------
  LeaveTMMQueue
  --------------------*/
UI_VOID_CMD(LeaveTMMQueue, 0)
{
    ChatManager.LeaveTMMQueue();
}


/*====================
  CChatManager::RejectTMMInvite
  ====================*/
void CChatManager::RejectTMMInvite(const tstring sNickname)
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_REJECT_INVITE << sNickname;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  RejectTMMInvite
  --------------------*/
CMD(RejectTMMInvite)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.RejectTMMInvite(vArgList[0]);
    return true;
}


/*--------------------
  RejectTMMInvite
  --------------------*/
UI_VOID_CMD(RejectTMMInvite, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.RejectTMMInvite(vArgList[0]->Evaluate());
}


/*====================
  CChatManager::KickFromTMMGroup
  ====================*/
void CChatManager::KickFromTMMGroup(const byte ySlotNumber)
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GROUP_KICK << ySlotNumber;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  KickFromTMMGroup
  --------------------*/
CMD(KickFromTMMGroup)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.KickFromTMMGroup(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  KickFromTMMGroup
  --------------------*/
UI_VOID_CMD(KickFromTMMGroup, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.KickFromTMMGroup(AtoI(vArgList[0]->Evaluate()));
}


/*====================
  CChatManager::SendTMMGroupOptionsUpdate
  ====================*/
void    CChatManager::SendTMMGroupOptionsUpdate(byte yGameType, const tstring &sMapName, const tstring &sGameModes, const tstring &sRegions)
{
    Console << _T("Updating group options") << newl;
    Console << _T("GameType: ") << yGameType << newl;
    Console << _T("MapName: ") << sMapName << newl;
    Console << _T("GameModes: ") << sGameModes << newl;
    Console << _T("Regions: ") << sRegions << newl;

#if 0
    m_uiTMMStartTime = INVALID_TIME;
    m_uiTMMAverageQueueTime = INVALID_TIME;
    m_uiTMMStdDevQueueTime = INVALID_TIME;
#else
    if (m_uiTMMStartTime != INVALID_TIME)
        Console.Warn << _T("TMM options reset while in queue") << newl;
#endif

    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_GAME_OPTION_UPDATE << yGameType << sMapName << sGameModes << sRegions;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  SendTMMGroupOptionsUpdate
  --------------------*/
CMD(SendTMMGroupOptionsUpdate)
{
    if (vArgList.size() < 4)
        return false;

    ChatManager.SendTMMGroupOptionsUpdate(AtoI(vArgList[0]), vArgList[1], vArgList[2], vArgList[3]);

    return true;
}


/*--------------------
  SendTMMGroupOptionsUpdate
  --------------------*/
UI_VOID_CMD(SendTMMGroupOptionsUpdate, 4)
{
    ChatManager.SendTMMGroupOptionsUpdate(AtoI(vArgList[0]->Evaluate()), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), vArgList[3]->Evaluate());
    
    return;
}


/*====================
  CChatManager::RequestTMMPopularityUpdate
  ====================*/
void CChatManager::RequestTMMPopularityUpdate()
{
    CPacket pktSend;
    pktSend << NET_CHAT_CL_TMM_POPULARITY_UPDATE;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  RequestTMMPopularityUpdate
  --------------------*/
CMD(RequestTMMPopularityUpdate)
{
    ChatManager.RequestTMMPopularityUpdate();
    return true;
}


/*--------------------
  RequestTMMPopularityUpdate
  --------------------*/
UI_VOID_CMD(RequestTMMPopularityUpdate, 0)
{
    ChatManager.RequestTMMPopularityUpdate();
}


/*====================
  CChatManager::SendTMMPlayerLoadingUpdate
  ====================*/
void    CChatManager::SendTMMPlayerLoadingUpdate(byte yPercent)
{
    if (!IsInGroup())
        return;

    CPacket pktSend;    
    pktSend << NET_CHAT_CL_TMM_GROUP_PLAYER_LOADING_STATUS << yPercent;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  SendTMMPlayerLoadingUpdate
  --------------------*/
CMD(SendTMMPlayerLoadingUpdate)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.SendTMMPlayerLoadingUpdate(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  SendTMMPlayerLoadingUpdate
  --------------------*/
UI_VOID_CMD(SendTMMPlayerLoadingUpdate, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.SendTMMPlayerLoadingUpdate(AtoI(vArgList[0]->Evaluate()));
}


/*====================
  CChatManager::SendTMMPlayerReadyStatus
  ====================*/
void CChatManager::SendTMMPlayerReadyStatus(const byte yReadyStatus)
{
    CPacket pktSend;    
    pktSend << NET_CHAT_CL_TMM_GROUP_PLAYER_READY_STATUS << yReadyStatus;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  SendTMMPlayerReadyStatus
  --------------------*/
CMD(SendTMMPlayerReadyStatus)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.SendTMMPlayerReadyStatus(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  SendTMMPlayerReadyStatus
  --------------------*/
UI_VOID_CMD(SendTMMPlayerReadyStatus, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.SendTMMPlayerReadyStatus(AtoI(vArgList[0]->Evaluate()));
}


/*====================
  CChatManager::IsInGroup
  ====================*/
bool    CChatManager::IsInGroup()
{
    if (!ChatManager.IsConnected())
        return false;

    return m_bInGroup;
}


/*--------------------
  IsInGroup
  --------------------*/
UI_CMD(IsInGroup, 0)
{
    return XtoA(ChatManager.IsInGroup());
}


/*====================
  CChatManager::IsTMMEnabled
  ====================*/
bool    CChatManager::IsTMMEnabled()
{
    if (!ChatManager.IsConnected())
        return false;

    return m_bTMMEnabled;
}


/*--------------------
  IsTMMEnabled
  --------------------*/
UI_CMD(IsTMMEnabled, 0)
{
    return XtoA(ChatManager.IsTMMEnabled());
}


/*====================
  CChatManager::IsInQueue
  ====================*/
bool CChatManager::IsInQueue()
{
    if (m_uiTMMStartTime == INVALID_TIME || !ChatManager.IsConnected())
        return false;
    else
        return true;
}


/*--------------------
  IsInQueue
  --------------------*/
UI_CMD(IsInQueue, 0)
{
    return XtoA(ChatManager.IsInQueue());
}


/*====================
  CChatManager::GetGroupLeaderID
  ====================*/
uint    CChatManager::GetGroupLeaderID()
{
    if (!ChatManager.IsConnected())
        return INVALID_INDEX;

    return m_uiTMMGroupLeaderID;
}


/*--------------------
  GetTMMGroupLeaderID
  --------------------*/
UI_CMD(GetTMMGroupLeaderID, 0)
{
    uint uiAccountID(ChatManager.GetGroupLeaderID());

    if (uiAccountID != INVALID_INDEX)
        return XtoA(uiAccountID);
    else
        return TSNULL;
}


/*====================
  CChatManager::GetOtherPlayersReady
  ====================*/
bool    CChatManager::GetOtherPlayersReady()
{
    if (!ChatManager.IsConnected())
        return false;

    return m_bTMMOtherPlayersReady;
}


/*--------------------
  GetTMMOtherPlayersReady
  --------------------*/
UI_CMD(GetTMMOtherPlayersReady, 0)
{
    return XtoA(ChatManager.GetOtherPlayersReady());
}


/*====================
  CChatManager::GetAllPlayersReady
  ====================*/
bool    CChatManager::GetAllPlayersReady()
{
    if (!ChatManager.IsConnected())
        return false;

    return m_bTMMAllPlayersReady;
}


/*--------------------
  GetTMMAllPlayersReady
  --------------------*/
UI_CMD(GetTMMAllPlayersReady, 0)
{
    return XtoA(ChatManager.GetAllPlayersReady());
}


/*--------------------
  ChatSendMessage
  --------------------*/
UI_CMD(ChatSendMessage, 2)
{
    ChatManager.ResetTabCounter();
    return XtoA(ChatManager.SubmitChatMessage(vArgList[0]->Evaluate(), ChatManager.GetChannelID(vArgList[1]->Evaluate())), true);
}


/*====================
  cmdChatAddBuddy
  ====================*/
UI_VOID_CMD(ChatAddBuddy, 1)
{
    ChatManager.RequestBuddyAdd(ChatManager.RemoveClanTag(vArgList[0]->Evaluate()));
}


/*====================
  cmdChatApproveBuddy
  ====================*/
UI_VOID_CMD(ChatApproveBuddy, 1)
{
    ChatManager.RequestBuddyApprove(ChatManager.RemoveClanTag(vArgList[0]->Evaluate()));
}


/*--------------------
  ChatRemoveBuddy
  --------------------*/
UI_VOID_CMD(ChatRemoveBuddy, 1)
{
    ChatManager.RequestBuddyRemove(ChatManager.RemoveClanTag(vArgList[0]->Evaluate()));
}


/*--------------------
  GetCurrentChatMessage
  --------------------*/
UI_CMD(GetCurrentChatMessage, 0)
{
    return ChatManager.GetCurrentChatMessage();
}

/*--------------------
  GetCurrentChatType
  --------------------*/
UI_CMD(GetCurrentChatType, 0)
{
    return ChatManager.GetCurrentChatType();
}


/*--------------------
  SetCurrentChatType
  --------------------*/
UI_VOID_CMD(SetCurrentChatType, 1)
{
    ChatManager.SetCurrentChatType(vArgList[0]->Evaluate());
}


/*--------------------
  SetCurrentChatMessage
  --------------------*/
UI_CMD(SetCurrentChatMessage, 1)
{
    return ChatManager.SetCurrentChatMessage(vArgList[0]->Evaluate());
}


/*--------------------
  ClearCurrentChatMessage
  --------------------*/
UI_VOID_CMD(ClearCurrentChatMessage, 0)
{
    ChatManager.SetCurrentChatMessage(_T(""));
}


/*--------------------
  ChatUpdateUserList
  --------------------*/
UI_VOID_CMD(ChatUpdateUserList, 1)
{
    ChatManager.UpdateUserList(ChatManager.GetChannelID(vArgList[0]->Evaluate()));
}


/*--------------------
  ChatUpdateBuddyList
  --------------------*/
UI_VOID_CMD(ChatUpdateBuddyList, 0)
{
    ChatManager.UpdateBuddyList();
}


/*--------------------
  ChatUpdateClanList
  --------------------*/
UI_VOID_CMD(ChatUpdateClanList, 0)
{
    ChatManager.UpdateClanList();
}


/*--------------------
  ChatInGame
  --------------------*/
UI_CMD(ChatInGame, 0)
{
    return XtoA(ChatManager.GetStatus() > CHAT_STATUS_CONNECTED, true);
}


/*--------------------
  ChatUserInGame
  --------------------*/
UI_CMD(ChatUserInGame, 1)
{
    return XtoA(ChatManager.IsUserInGame(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatUserInCurrentGame
  --------------------*/
UI_CMD(ChatUserInCurrentGame, 1)
{
    return XtoA(ChatManager.IsUserInCurrentGame(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatUserOnline
  --------------------*/
UI_CMD(ChatUserOnline, 1)
{
    return XtoA(ChatManager.IsUserOnline(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatUpdateWhispers
  --------------------*/
UI_VOID_CMD(ChatUpdateWhispers, 1)
{
    ChatManager.UpdateWhispers(vArgList[0]->Evaluate());
}


/*--------------------
  ChatUpdateClanWhispers
  --------------------*/
UI_VOID_CMD(ChatUpdateClanWhispers, 1)
{
    ChatManager.UpdateClanWhispers();
}


/*--------------------
  ChatSendWhisper
  --------------------*/
UI_CMD(ChatSendWhisper, 2)
{
    return XtoA(ChatManager.SendWhisper(vArgList[0]->Evaluate(), vArgList[1]->Evaluate()), true);
}


/*--------------------
  ChatSendIM
  --------------------*/
UI_CMD(ChatSendIM, 2)
{
    return XtoA(ChatManager.SendIM(vArgList[0]->Evaluate(), vArgList[1]->Evaluate()));
}


/*--------------------
  ChatSendClanWhisper
  --------------------*/
UI_CMD(ChatSendClanWhisper, 1)
{
    return XtoA(ChatManager.SendClanWhisper(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatRemoveClanMember
  --------------------*/
UI_VOID_CMD(ChatRemoveClanMember, 1)
{
    ChatManager.RequestRemoveClanMember(vArgList[0]->Evaluate());
}


/*--------------------
  ChatPromoteClanMember
  --------------------*/
UI_VOID_CMD(ChatPromoteClanMember, 1)
{
    ChatManager.RequestPromoteClanMember(vArgList[0]->Evaluate());
}


/*--------------------
  ChatDemoteClanMember
  --------------------*/
UI_VOID_CMD(ChatDemoteClanMember, 1)
{
    ChatManager.RequestDemoteClanMember(vArgList[0]->Evaluate());
}


/*--------------------
  ChatIsClanOfficer
  --------------------*/
UI_CMD(ChatIsClanOfficer, 1)
{
    return XtoA(ChatManager.HasFlags(vArgList[0]->Evaluate(), CHAT_CLIENT_IS_OFFICER), true);
}


/*--------------------
  ChatIsClanLeader
  --------------------*/
UI_CMD(ChatIsClanLeader, 1)
{
    return XtoA(ChatManager.HasFlags(vArgList[0]->Evaluate(), CHAT_CLIENT_IS_CLAN_LEADER), true);
}


/*--------------------
  ChatOpenMessage
  --------------------*/
UI_VOID_CMD(ChatOpenMessage, 1)
{
    ChatMessageTrigger.Trigger(ChatManager.RemoveClanTag(vArgList[0]->Evaluate()));
}

/*--------------------
  TabChat
  --------------------*/
UI_CMD(TabChat, 1)
{
    return XtoA(ChatManager.TabChatMessage(vArgList[0]->Evaluate()));
}

/*--------------------
  IsFollowing
  --------------------*/
UI_CMD(IsFollowing, 1)
{
    return XtoA(ChatManager.IsFollowing(vArgList[0]->Evaluate()));
}

/*--------------------
  Follow
  --------------------*/
UI_CMD(Follow, 1)
{
    if (vArgList.size() != 1)
        return  XtoA(false);

    return  XtoA(ChatManager.SetFollowing(vArgList[0]->Evaluate()));
}

/*--------------------
  UnFollow
  --------------------*/
UI_VOID_CMD(UnFollow, 0)
{
    ChatManager.UnFollow();
}

/*--------------------
  ChatOpenMessage
  --------------------*/
CMD(ChatOpenMessage)
{
    if (vArgList.size() < 1)
        return false;

    ChatMessageTrigger.Trigger(ChatManager.RemoveClanTag(vArgList[0]));
    return true;
}


/*--------------------
  ChatOpenClanMessage
  --------------------*/
UI_VOID_CMD(ChatOpenClanMessage, 0)
{
    ChatClanMessageTrigger.Trigger(TSNULL);
}


/*--------------------
  ChatJoinGame
  --------------------*/
UI_VOID_CMD(ChatJoinGame, 1)
{
    ChatManager.JoinGame(vArgList[0]->Evaluate());
}


/*--------------------
  ChatUpdateHoverInfo
  --------------------*/
UI_CMD(ChatUpdateHoverInfo, 1)
{
    return XtoA(ChatManager.UpdateHoverInfo(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatRefresh
  --------------------*/
UI_VOID_CMD(ChatRefresh, 0)
{
    ChatManager.RefreshBuddyList();
    ChatManager.RefreshClanList();
}


/*--------------------
  ChatConnect
  --------------------*/
UI_VOID_CMD(ChatConnect, 0)
{
    // if they pass any parameters then it *should* be a request to connnect in 'invisible' mode
    if (vArgList.size() < 1)
        ChatManager.Connect(false);
    else
        ChatManager.Connect(AtoB(vArgList[0]->Evaluate()));
}

CMD(ChatConnect)
{
    ChatManager.Connect(false);
    return true;
}


/*--------------------
  ChatDisconnect
  --------------------*/
UI_VOID_CMD(ChatDisconnect, 0)
{
    ChatManager.Disconnect();
}


/*--------------------
  ChatDisconnect
  --------------------*/
CMD(ChatDisconnect)
{
    ChatManager.Disconnect();
    return true;
}


/*--------------------
  ChatIsConnected
  --------------------*/
UI_CMD(ChatIsConnected, 1)
{

    return XtoA(ChatManager.IsConnected(), true);
}


/*--------------------
  ChatPushNotification
  --------------------*/
CMD(ChatPushNotification)
{
    // For testing only, I know it's ugly but its fast and it works
    if (vArgList.size() < 1)
        return false;
        
    tstring sArg1(_T(""));
    tstring sArg2(_T(""));
    tstring sArg3(_T(""));
    
    if (vArgList.size() > 1) 
        sArg1 = vArgList[1];
        
    if (vArgList.size() > 2)
        sArg2 = vArgList[2];
        
    if (vArgList.size() > 3)
        sArg3 = vArgList[3];        
        
    if (AtoI(vArgList[0]) <= NUM_NOTIFICATIONS)
    {
        if (AtoI(vArgList[0]) == NOTIFY_TYPE_GAME_INVITE || AtoI(vArgList[0]) == NOTIFY_TYPE_BUDDY_JOIN_GAME || AtoI(vArgList[0]) == NOTIFY_TYPE_CLAN_JOIN_GAME)
        {
            static tsvector vInvite(20);
            
            if (vArgList.size() > 4) vInvite[0] = vArgList[4]; else vInvite[0] = _T("");        // Address
            if (vArgList.size() > 5) vInvite[1] = vArgList[5]; else vInvite[1] = _T("");        // Game Name
            if (vArgList.size() > 6) vInvite[2] = vArgList[6]; else vInvite[2] = _T("");        // Inviter/Game Joiner Name
            if (vArgList.size() > 7) vInvite[3] = vArgList[7]; else vInvite[3] = _T("");        // Server Region
            if (vArgList.size() > 8) vInvite[4] = vArgList[8]; else vInvite[4] = _T("");        // Game Mode
            if (vArgList.size() > 9) vInvite[5] = vArgList[9]; else vInvite[5] = _T("");        // Team Size            
            if (vArgList.size() > 10) vInvite[6] = vArgList[10]; else vInvite[6] = _T("");      // Map Name
            if (vArgList.size() > 11) vInvite[7] = vArgList[11]; else vInvite[7] = _T("");      // Tier - Noobs Only (0), Noobs Allowed (1), Pro (2) (Depreciated)
            if (vArgList.size() > 12) vInvite[8] = vArgList[12]; else vInvite[8] = _T("");      // 0 - Unofficial, 1 - Official w/ stats, 2 - Official w/o stats    
            if (vArgList.size() > 13) vInvite[9] = vArgList[13]; else vInvite[9] = _T("");      // No Leavers (1), Leavers (0)
            if (vArgList.size() > 14) vInvite[10] = vArgList[14]; else vInvite[10] = _T("");    // Private (1), Not Private (0)                                     
            if (vArgList.size() > 15) vInvite[11] = vArgList[15]; else vInvite[11] = _T("");    // All Heroes (1), Not All Heroes (0)
            if (vArgList.size() > 16) vInvite[12] = vArgList[16]; else vInvite[12] = _T("");    // Casual Mode (1), Not Casual Mode (0)
            if (vArgList.size() > 17) vInvite[13] = vArgList[17]; else vInvite[13] = _T("");    // Force Random (1), Not Force Random (0)
            if (vArgList.size() > 18) vInvite[14] = vArgList[18]; else vInvite[14] = _T("");    // Auto Balanced (1), Non Auto Balanced (0)
            if (vArgList.size() > 19) vInvite[15] = vArgList[19]; else vInvite[15] = _T("");    // Advanced Options (1), No Advanced Options (0)
            if (vArgList.size() > 20) vInvite[16] = vArgList[20]; else vInvite[16] = _T("");    // Min PSR
            if (vArgList.size() > 21) vInvite[17] = vArgList[21]; else vInvite[17] = _T("");    // Max PSR
            if (vArgList.size() > 22) vInvite[18] = vArgList[22]; else vInvite[18] = _T("");    // Dev Heroes (1), Non Dev Heroes (0)
            if (vArgList.size() > 23) vInvite[19] = vArgList[23]; else vInvite[19] = _T("");    // Hardcore (1), Non Hardcore (0)
                
            ChatManager.PushNotification(AtoI(vArgList[0]), sArg1, sArg2, sArg3, vInvite);      
        }           
        else
            ChatManager.PushNotification(AtoI(vArgList[0]), sArg1, sArg2, sArg3);
        return true;
    }
    else
    {
        Console << _T("Notification type specified is > ") << NUM_NOTIFICATIONS << ", choose a valid notification type." << newl;
        return false;
    }
}


/*--------------------
  ChatRemoveUnreadIMs
  --------------------*/
UI_VOID_CMD(ChatRemoveUnreadIMs, 1)
{
    ChatManager.AddReadIM(ChatManager.RemoveUnreadIMs(vArgList[0]->Evaluate()));
    ChatUnreadIMCount.Trigger(XtoA(ChatManager.GetUnreadIMCount()));
    ChatOpenIMCount.Trigger(XtoA(ChatManager.GetOpenIMCount()));
}


/*--------------------
  ChatRemoveNotification
  --------------------*/
UI_VOID_CMD(ChatRemoveNotification, 1)
{
    ChatManager.RemoveNotification(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  ChatRemoveAllNotifications
  --------------------*/
UI_VOID_CMD(ChatRemoveAllNotifications, 0)
{
    ChatManager.RequestRemoveAllNotifications();
}


/*--------------------
  ChatJoinChannel
  --------------------*/
UI_VOID_CMD(ChatJoinChannel, 1)
{
    if (vArgList.size() == 1)
        ChatManager.JoinChannel(vArgList[0]->Evaluate());
    else if (vArgList.size() > 1)
        ChatManager.JoinChannel(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  ChatIsSavedChannel
  --------------------*/
UI_CMD(ChatIsSavedChannel, 1)
{
    if (vArgList.size() == 1)
        return XtoA(ChatManager.IsSavedChannel(vArgList[0]->Evaluate()));
    else
        return XtoA(false);
}


/*--------------------
  ChatSaveChannel
  --------------------*/
UI_VOID_CMD(ChatSaveChannel, 1)
{
    if (vArgList.size() == 1)
        ChatManager.SaveChannel(vArgList[0]->Evaluate());
}


/*--------------------
  ChatRemoveChannel
  --------------------*/
UI_VOID_CMD(ChatRemoveChannel, 1)
{
    if (vArgList.size() == 1)
        ChatManager.RemoveChannel(vArgList[0]->Evaluate());
}


/*--------------------
  ChatUpdateChannels
  --------------------*/
UI_VOID_CMD(ChatUpdateChannels, 0)
{
    ChatManager.RequestChannelList();
}


/*--------------------
  ChatRebuildChannels
  --------------------*/
UI_VOID_CMD(ChatRebuildChannels, 0)
{
    ChatManager.RebuildChannels();
}


/*--------------------
  RequestChannelList
  --------------------*/
CMD(RequestChannelList)
{
    ChatManager.RequestChannelList();
    return true;
}


/*--------------------
  ChatLeaveChannel
  --------------------*/
UI_VOID_CMD(ChatLeaveChannel, 1)
{
    ChatManager.LeaveChannel(vArgList[0]->Evaluate());
}


/*--------------------
  IsBuddy
  --------------------*/
UI_CMD(IsBuddy, 1)
{
    return XtoA(ChatManager.IsBuddy(vArgList[0]->Evaluate()), true);
}


/*--------------------
  IsClanMember
  --------------------*/
UI_CMD(IsClanMember, 1)
{
    return XtoA(ChatManager.IsClanMember(vArgList[0]->Evaluate()), true);
}


/*--------------------
  SendServerInviteID
  --------------------*/
UI_VOID_CMD(SendServerInviteID, 1)
{
    if (!ChatManager.IsConnected() || ChatManager.GetStatus() < CHAT_STATUS_JOINING_GAME)
        return;

    ChatManager.SendServerInvite(AtoI(vArgList[0]->Evaluate()));
}

CMD(SendServerInviteID)
{
    if (vArgList.size() < 1)
        return false;

    if (!ChatManager.IsConnected() || ChatManager.GetStatus() < CHAT_STATUS_JOINING_GAME)
        return true;

    ChatManager.SendServerInvite(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  SendServerInviteName
  --------------------*/
UI_VOID_CMD(SendServerInviteName, 1)
{
    if (!ChatManager.IsConnected() || ChatManager.GetStatus() < CHAT_STATUS_JOINING_GAME)
        return;

    ChatManager.SendServerInvite(vArgList[0]->Evaluate());
}

CMD(SendServerInviteName)
{
    if (vArgList.size() < 1)
        return false;

    if (!ChatManager.IsConnected() || ChatManager.GetStatus() < CHAT_STATUS_JOINING_GAME)
        return false;

    ChatManager.SendServerInvite(vArgList[0]);
    return true;
}


/*--------------------
  RejectServerInvite
  --------------------*/
UI_VOID_CMD(RejectServerInvite, 1)
{
    ChatManager.RejectServerInvite(AtoI(vArgList[0]->Evaluate()));
}

CMD(RejectServerInvite)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.RejectServerInvite(AtoI(vArgList[0]));

    return true;
}


/*--------------------
  IsPrivateGame
  --------------------*/
UI_CMD(IsPrivateGame, 0)
{
    return XtoA(ChatManager.GetPrivateGame());
}


/*--------------------
  IsHost
  --------------------*/
UI_CMD(IsHost, 0)
{
    return XtoA(ChatManager.GetHost());
}


/*--------------------
  GameInvite
  --------------------*/
UI_VOID_CMD(GameInvite, 1)
{
    ChatManager.InviteUser(vArgList[0]->Evaluate());
}


/*--------------------
  GetUserInfo
  --------------------*/
UI_VOID_CMD(GetUserInfo, 1)
{
    ChatManager.GetUserInfo(vArgList[0]->Evaluate());
}


/*--------------------
  ChatCloseIM
  --------------------*/
UI_VOID_CMD(ChatCloseIM, 1)
{
    ChatManager.CloseIM(vArgList[0]->Evaluate());
}


/*--------------------
  ChatRefreshChannels
  --------------------*/
UI_VOID_CMD(ChatRefreshChannels, 0)
{
    ChatManager.UpdateChannels();
}


/*--------------------
  ChatAutocompleteNick
  --------------------*/
UI_VOID_CMD(ChatAutocompleteNick, 1)
{
    ChatManager.AutoCompleteNick(vArgList[0]->Evaluate());
}


/*--------------------
  ChatAutocompleteClear
  --------------------*/
UI_VOID_CMD(ChatAutocompleteClear, 0)
{
    ChatManager.AutoCompleteClear();
}


/*--------------------
  ChatAutocompleteChannel
  --------------------*/
UI_VOID_CMD(ChatAutocompleteChannel, 0)
{
    ChatManager.RequestChannelSublist(vArgList.size() > 0 ? vArgList[0]->Evaluate() : TSNULL);
}


/*--------------------
  ChatAutocompleteChannelCancel
  --------------------*/
UI_VOID_CMD(ChatAutocompleteChannelCancel, 0)
{
    ChatManager.ChannelSublistCancel();
}


/*--------------------
  RequestChannelSublist
  --------------------*/
CMD(RequestChannelSublist)
{
    ChatManager.RequestChannelSublist(vArgList.size() > 0 ? vArgList[0] : TSNULL);
    return true;
}


/*--------------------
  ChatCloseNotifications
  --------------------*/
UI_VOID_CMD(ChatCloseNotifications, 0)
{
    ChatCloseNotifications.Trigger(TSNULL);
}


/*--------------------
  ChatChannelKick
  --------------------*/
UI_VOID_CMD(ChatChannelKick, 2)
{
    uint uiChannelID = ChatManager.GetChannelID(vArgList[1]->Evaluate());
    ChatManager.KickUserFromChannel(uiChannelID, vArgList[0]->Evaluate());
}


/*--------------------
  ChatChannelBan
  --------------------*/
UI_VOID_CMD(ChatChannelBan, 2)
{
    uint uiChannelID = ChatManager.GetChannelID(vArgList[1]->Evaluate());
    ChatManager.BanUserFromChannel(uiChannelID, vArgList[0]->Evaluate());
}


/*--------------------
  ChatChannelSilence
  --------------------*/
UI_VOID_CMD(ChatChannelSilence, 3)
{
    uint uiChannelID = ChatManager.GetChannelID(vArgList[1]->Evaluate());
    ChatManager.SilenceChannelUser(uiChannelID, vArgList[0]->Evaluate(), MinToMs(uint(AtoI(vArgList[2]->Evaluate()))));
}


/*--------------------
  ChatChannelPromote
  --------------------*/
UI_VOID_CMD(ChatChannelPromote, 2)
{
    uint uiChannelID = ChatManager.GetChannelID(vArgList[1]->Evaluate());
    ChatManager.PromoteUserInChannel(uiChannelID, vArgList[0]->Evaluate());
}


/*--------------------
  ChatChannelDemote
  --------------------*/
UI_VOID_CMD(ChatChannelDemote, 2)
{
    uint uiChannelID = ChatManager.GetChannelID(vArgList[1]->Evaluate());
    ChatManager.DemoteUserInChannel(uiChannelID, vArgList[0]->Evaluate());
}


/*--------------------
  ChatIgnore
  --------------------*/
UI_VOID_CMD(ChatIgnore, 1)
{
    ChatManager.RequestIgnoreAdd(vArgList[0]->Evaluate());
}


/*--------------------
  ChatUnignore
  --------------------*/
UI_VOID_CMD(ChatUnignore, 1)
{
    ChatManager.RequestIgnoreRemove(vArgList[0]->Evaluate());
}


/*--------------------
  ChatIgnoreID
  --------------------*/
UI_VOID_CMD(ChatIgnoreID, 1)
{
    ChatManager.RequestIgnoreAdd(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  ChatUnignoreID
  --------------------*/
UI_VOID_CMD(ChatUnignoreID, 1)
{
    ChatManager.RequestIgnoreRemove(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  ChatAddBanlist
  --------------------*/
UI_VOID_CMD(ChatAddBanlist, 2)
{
    ChatManager.RequestBanlistAdd(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  ChatRemoveBanlist
  --------------------*/
UI_VOID_CMD(ChatRemoveBanlist, 1)
{
    ChatManager.RequestBanlistRemove(vArgList[0]->Evaluate());
}


/*--------------------
  ChatIsAdmin
  --------------------*/
UI_CMD(ChatIsAdmin, 2)
{
    return XtoA(ChatManager.IsAdmin(ChatManager.GetChannelID(vArgList[1]->Evaluate()), vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatGetAdminLevel
  --------------------*/
UI_CMD(ChatGetAdminLevel, 2)
{
    return XtoA(ChatManager.GetAdminLevel(ChatManager.GetChannelID(vArgList[1]->Evaluate()), vArgList[0]->Evaluate()));
}


/*--------------------
  ChatIsIgnored
  --------------------*/
UI_CMD(ChatIsIgnored, 1)
{
    return XtoA(ChatManager.IsIgnored(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatIsBanned
  --------------------*/
UI_CMD(ChatIsBanned, 1)
{
    return XtoA(ChatManager.IsBanned(vArgList[0]->Evaluate()), true);
}


/*--------------------
  ChatGetCurHistory
  --------------------*/
UI_CMD(ChatGetCurHistory, 0)
{
    return ChatManager.GetCurrentChatHistory();
}


/*--------------------
  ChatPrevHistory
  --------------------*/
UI_VOID_CMD(ChatPrevHistory, 0)
{
    ChatManager.PreviousHistory();
}


/*--------------------
  ChatNextHistory
  --------------------*/
UI_VOID_CMD(ChatNextHistory, 0)
{
    ChatManager.NextHistory();
}


/*--------------------
  ChatAcceptClanInvite
  --------------------*/
UI_VOID_CMD(ChatAcceptClanInvite, 0)
{
    ChatManager.AcceptClanInvite();
}


/*--------------------
  ChatRejectClanInvite
  --------------------*/
UI_VOID_CMD(ChatRejectClanInvite, 0)
{
    ChatManager.RejectClanInvite();
}


/*--------------------
  ChatInviteUserToClan
  --------------------*/
UI_VOID_CMD(ChatInviteUserToClan, 1)
{
    ChatManager.InviteToClan(vArgList[0]->Evaluate());
}


/*--------------------
  ChatUserIsInClan
  --------------------*/
UI_CMD(ChatUserIsInClan, 1)
{
    return XtoA(ChatManager.IsInAClan(vArgList[0]->Evaluate()), true);
}


/*--------------------
  IsInClan
  --------------------*/
UI_CMD(IsInClan, 0)
{
    return XtoA(ChatManager.IsInAClan(ChatManager.GetLocalAccountID()), true);
}


/*--------------------
  ChatCreateClan
  --------------------*/
UI_VOID_CMD(ChatCreateClan, 6)
{
    ChatManager.CreateClan(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), vArgList[3]->Evaluate(), vArgList[4]->Evaluate(), vArgList[5]->Evaluate());
}


/*--------------------
  ChatCheckClanName
  --------------------*/
UI_VOID_CMD(ChatCheckClanName, 2)
{
    ChatManager.CheckClanName(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  ShowPostGameStats
  --------------------*/
CMD(ShowPostGameStats)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.ShowPostGameStats(AtoUI(vArgList[0]));
    return true;
}


/*--------------------
  ShowPostGameStats
  --------------------*/
UI_VOID_CMD(ShowPostGameStats, 1)
{
    ChatManager.ShowPostGameStats(AtoUI(vArgList[0]->Evaluate()));
}


/*--------------------
  WaitingToShowStats
  --------------------*/
UI_CMD(WaitingToShowStats, 0)
{
    return XtoA(ChatManager.IsWaitingToShowStats());
}


/*--------------------
  ClearWaitingToShowStats
  --------------------*/
UI_VOID_CMD(ClearWaitingToShowStats, 0)
{
    ChatManager.ClearWaitingToShowStats();
}


/*--------------------
  GetShowStatsMatchID
  --------------------*/
UI_CMD(GetShowStatsMatchID, 0)
{
    return XtoA(ChatManager.GetShowStatsMatchID());
}


/*--------------------
  SetFocusedChannel
  --------------------*/
UI_VOID_CMD(SetFocusedChannel, 1)
{
    ChatManager.SetFocusedChannel(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  SetNextFocusedChannel
  --------------------*/
UI_VOID_CMD(SetNextFocusedChannel, 0)
{
    ChatManager.SetNextFocusedChannel();
}


/*--------------------
  SetFocusedIM
  --------------------*/
UI_VOID_CMD(SetFocusedIM, 1)
{
    ChatManager.SetFocusedIM(vArgList[0]->Evaluate());
}


/*--------------------
  SetNextFocusedIM
  --------------------*/
UI_VOID_CMD(SetNextFocusedIM, 0)
{
    ChatManager.SetNextFocusedIM();
}


/*--------------------
  GetFocusedIM
  --------------------*/
UI_CMD(GetFocusedIM, 0)
{
    return ChatManager.GetFocusedIM();
}


/*--------------------
  ChatIsInChannel
  --------------------*/
UI_CMD(ChatIsInChannel, 1)
{
    return XtoA(ChatManager.IsInChannel(AtoUI(vArgList[0]->Evaluate())));
}


/*--------------------
  RequestUserStatus
  --------------------*/
UI_VOID_CMD(RequestUserStatus, 1)
{
    ChatManager.RequestUserStatus(vArgList[0]->Evaluate());
}

CMD(RequestUserStatus)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.RequestUserStatus(vArgList[0]);
    return true;
}


/*--------------------
  PlayChatSound
  --------------------*/
UI_VOID_CMD(PlayChatSound, 1)
{
    ChatManager.PlaySound(vArgList[0]->Evaluate());
}


/*--------------------
  ChatRefreshUpgrades
  --------------------*/
CMD(ChatRefreshUpgrades)
{
    ChatManager.RequestRefreshUpgrades();
    return true;
}


/*--------------------
  ChatRefreshUpgrades
  --------------------*/
UI_VOID_CMD(ChatRefreshUpgrades, 0)
{
    ChatManager.RequestRefreshUpgrades();
}


/*====================
  CChatManager::RequestGameInfo
  ====================*/
void CChatManager::RequestGameInfo(const tstring sNickname)
{
    CPacket pktSend;
    pktSend << CHAT_CMD_REQUEST_GAME_INFO << sNickname;
    m_sockChat.SendPacket(pktSend);
}


/*--------------------
  RequestGameInfo
  --------------------*/
CMD(RequestGameInfo)
{
    if (vArgList.size() < 1)
        return false;

    ChatManager.RequestGameInfo(vArgList[0]);
    return true;
}


/*--------------------
  RequestGameInfo
  --------------------*/
UI_VOID_CMD(RequestGameInfo, 1)
{
    if (vArgList.size() < 1)
        return;

    ChatManager.RequestGameInfo(vArgList[0]->Evaluate());
}


/*====================
  CChatManager::HandleRequestGameInfo
  ====================*/
void CChatManager::HandleRequestGameInfo(CPacket &pkt)
{
    const tstring sNickName(pkt.ReadWStringAsTString());
    const tstring sGameName(pkt.ReadWStringAsTString());
    const tstring sMapName(pkt.ReadWStringAsTString());
    const byte yGameType(pkt.ReadByte());
    const tstring sGameModeName(pkt.ReadWStringAsTString());
    const tstring sCGT(pkt.ReadWStringAsTString());
    const tstring sTeamInfo1(pkt.ReadWStringAsTString());
    const tstring sTeamInfo2(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo0(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo1(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo2(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo3(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo4(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo5(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo6(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo7(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo8(pkt.ReadWStringAsTString());
    const tstring sPlayerInfo9(pkt.ReadWStringAsTString());
    
    if (pkt.HasFaults())
        return;
    
    static tsvector vParams(18);
    
    vParams[0] = sNickName;
    vParams[1] = sTeamInfo1;
    vParams[2] = sTeamInfo2;
    vParams[3] = sPlayerInfo0;
    vParams[4] = sPlayerInfo1;
    vParams[5] = sPlayerInfo2;
    vParams[6] = sPlayerInfo3;
    vParams[7] = sPlayerInfo4;
    vParams[8] = sPlayerInfo5;
    vParams[9] = sPlayerInfo6;
    vParams[10] = sPlayerInfo7;
    vParams[11] = sPlayerInfo8;
    vParams[12] = sPlayerInfo9;
    vParams[13] = sGameName;
    vParams[14] = sMapName;
    vParams[15] = sGameModeName;
    vParams[16] = sCGT;
    vParams[17] = XtoA(yGameType);

    ChatRequestGameInfo.Trigger(vParams);
}
