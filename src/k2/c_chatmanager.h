// (C)2007 S2 Games
// c_chatmanager.h
//
//=============================================================================
#ifndef __C_CHATMANAGER_H__
#define __C_CHATMANAGER_H__

//=============================================================================
// Headers
//=============================================================================
#include "chatserver_protocol.h"

#include "c_widgetreference.h"
#include "c_textbuffer.h"
#include "c_clientlogin.h"
#include "c_socket.h"
#include "c_date.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CHTTPManager;
class CHTTPRequest;
class CHTTPPostRequest;

K2_API EXTERN_CVAR(bool, cg_censorChat);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EChatMessageType
{
    CHAT_MESSAGE_CLEAR,
    CHAT_MESSAGE_ADD,
    CHAT_MESSAGE_ROLL,
    CHAT_MESSAGE_EMOTE,
};

enum EChatStatus
{
    CHAT_STATUS_DISCONNECTED,
    CHAT_STATUS_CONNECTING,
    CHAT_STATUS_WAITING_FOR_AUTH,
    CHAT_STATUS_CONNECTED,
    CHAT_STATUS_JOINING_GAME,
    CHAT_STATUS_IN_GAME,
};

enum ENotifyType
{
    NOTIFY_TYPE_UNKNOWN,
    NOTIFY_TYPE_BUDDY_ADDER,
    NOTIFY_TYPE_BUDDY_ADDED,
    NOTIFY_TYPE_BUDDY_REMOVER,
    NOTIFY_TYPE_BUDDY_REMOVED,
    NOTIFY_TYPE_CLAN_RANK,
    NOTIFY_TYPE_CLAN_ADD,
    NOTIFY_TYPE_CLAN_REMOVE,
    NOTIFY_TYPE_BUDDY_ONLINE,
    NOTIFY_TYPE_BUDDY_LEFT_GAME,
    NOTIFY_TYPE_BUDDY_OFFLINE,
    NOTIFY_TYPE_BUDDY_JOIN_GAME,
    NOTIFY_TYPE_CLAN_ONLINE,
    NOTIFY_TYPE_CLAN_LEFT_GAME,
    NOTIFY_TYPE_CLAN_OFFLINE,
    NOTIFY_TYPE_CLAN_JOIN_GAME,
    NOTIFY_TYPE_CLAN_WHISPER,
    NOTIFY_TYPE_UPDATE,
    NOTIFY_TYPE_GENERIC,
    NOTIFY_TYPE_IM,
    NOTIFY_TYPE_GAME_INVITE,
    NOTIFY_TYPE_SELF_JOIN_GAME,
    NOTIFY_TYPE_BUDDY_REQUESTED_ADDER,
    NOTIFY_TYPE_BUDDY_REQUESTED_ADDED,
    NOTIFY_TYPE_TMM_GROUP_INVITE,

    NUM_NOTIFICATIONS
};

const tstring g_sNotifyText[NUM_NOTIFICATIONS] =
{
    _T("notify_unknown"),
    _T("notify_buddy_adder"),
    _T("notify_buddy_added"),
    _T("notify_buddy_remover"),
    _T("notify_buddy_removed"),
    _T("notify_clan_rank"),
    _T("notify_clan_add"),
    _T("notify_clan_remove"),
    _T("notify_buddy_online"),
    _T("notify_buddy_left_game"),
    _T("notify_buddy_offline"),
    _T("notify_buddy_join_game"),
    _T("notify_clan_online"),
    _T("notify_clan_left_game"),
    _T("notify_clan_offline"),
    _T("notify_clan_join_game"),
    _T("notify_clan_whisper"),
    _T("notify_update"),
    _T("notify_generic"),
    _T("notify_im"),
    _T("notify_game_invite"),
    _T("notify_self_join_game"),
    _T("notify_buddy_requested_adder"),
    _T("notify_buddy_requested_added"),
    _T("notify_group_invite")
};


const tstring g_sNotifyType[NUM_NOTIFICATIONS] =
{
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info_join"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info_join"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info"),
    _T("notfication_generic_info_join"),
    _T("notfication_generic_info_join"),
    _T("notfication_generic_info"),
    _T("notfication_generic_action"),
    _T("notfication_generic_action")
};


const tstring g_sNotifyAction[NUM_NOTIFICATIONS] =
{
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T(""),
    _T("action_joingame"),
    _T(""),
    _T(""),
    _T(""),
    _T("action_joingame"),
    _T(""),
    _T("action_update"),
    _T(""),
    _T(""),
    _T("action_joininvite"),
    _T("action_joingame"),
    _T(""),
    _T("action_friend_request"),
    _T("action_groupinvite")
};

enum EChatIgnoreType
{
    CHAT_IGNORE_NONE,
    CHAT_IGNORE_ENEMY_ALL,
    CHAT_IGNORE_ALL,
    CHAT_IGNORE_TEAM,
    CHAT_IGNORE_EVERYONE,
};

enum EChatModeType
{
    CHAT_MODE_AVAILABLE,
    CHAT_MODE_AFK,
    CHAT_MODE_DND,
    CHAT_MODE_INVISIBLE,
};

#define CHAT_CHANNEL_FLAG_PERMANENT     BIT(0)
#define CHAT_CHANNEL_FLAG_SERVER        BIT(1)
#define CHAT_CHANNEL_FLAG_HIDDEN        BIT(2)
#define CHAT_CHANNEL_FLAG_RESERVED      BIT(3)
#define CHAT_CHANNEL_FLAG_GENERAL_USE   BIT(4)
#define CHAT_CHANNEL_FLAG_UNJOINABLE    BIT(5)
#define CHAT_CHANNEL_FLAG_AUTH_REQUIRED BIT(6)
#define CHAT_CHANNEL_FLAG_CLAN          BIT(7)

enum EBanReason
{
    NOT_BANNED,
    BAN_REASON_CONNECTIONS,
    BAN_REASON_FLOODING,
};

struct SChatClient
{
    uint        uiAccountID;
    uint        uiMatchID;
    tstring     sServerAddressPort;
    tstring     sGameName;
    byte        yFlags;
    byte        yStatus;
    tstring     sName;
    tstring     sClan;
    tstring     sClanTag;
    int         iClanID;
    uiset       setChannels;
    uint        uiChatSymbol;
    uint        uiChatNameColor;
    uint        uiAccountIcon;
    uint        uiSortIndex;
};

struct SChatBanned
{
    uint        uiAccountID;
    tstring     sName;
    tstring     sReason;
};


enum ERequestType
{
    REQUEST_ADD_BUDDY_NICK2ID,
    REQUEST_ADD_BUDDY,
    REQUEST_DELETE_BUDDY,
    REQUEST_CLAN_PROMOTE,
    REQUEST_CLAN_DEMOTE,
    REQUEST_CLAN_REMOVE,
    REQUEST_ADD_IGNORED_NICK2ID,
    REQUEST_ADD_IGNORED,
    REQUEST_REMOVE_IGNORED,
    REQUEST_ADD_BANNED_NICK2ID,
    REQUEST_ADD_BANNED,
    REQUEST_REMOVE_BANNED,
    REQUEST_GET_BANNED,
    REQUEST_UPDATE_CLAN,
    REQUEST_CHECK_CLAN_NAME,
    REQUEST_COMPLETE_NICK,
    REQUEST_SAVE_CHANNEL,
    REQUEST_REMOVE_CHANNEL,
    REQUEST_SAVE_NOTIFICATION,
    REQUEST_REMOVE_NOTIFICATION,
    REQUEST_REMOVE_ALL_NOTIFICATIONS
};

struct SChatDBRequest
{
    CHTTPRequest*       pRequest;
    ERequestType        eType;
    uint                uiTarget;
    tstring             sTarget;
    tstring             sText;

    SChatDBRequest(CHTTPRequest *_pRequest, ERequestType _eType, const tstring &_sTarget, const tstring & _sText = TSNULL) :
    pRequest(_pRequest),
    eType(_eType),
    uiTarget(INVALID_ACCOUNT),
    sTarget(_sTarget),
    sText(_sText)
    {
    }

    SChatDBRequest(CHTTPRequest *_pRequest, ERequestType _eType, uint _uiTarget, const tstring & _sText = TSNULL) :
    pRequest(_pRequest),
    eType(_eType),
    uiTarget(_uiTarget),
    sTarget(TSNULL),
    sText(_sText)
    {
    }
};

typedef map<uint, byte>         ChatAdminMap;
typedef pair<uint, byte>        ChatAdminPair;
typedef ChatAdminMap::iterator  ChatAdminMap_it;

struct SChatChannel
{
    tstring             sChannelName;
    uint                uiUserCount;
    uint                uiFlags;
    ChatAdminMap        mapAdmins;
    tstring             sTopic;
    bool                bUnread;    // Locally managed boolean
    uint                uiFocusPriority;

    SChatChannel() :
    uiUserCount(0),
    uiFlags(0),
    bUnread(false),
    uiFocusPriority(0)
    {}
};

struct SChatChannelInfo
{
    tstring             sName;
    tstring             sLowerName;
    uint                uiUserCount;

    SChatChannelInfo() :
    uiUserCount(0)
    {}
};

struct SChatWidget
{
    CWidgetReference    refWidget;
    bool                bGameChat;
    tstring             sChannel;

    SChatWidget(CTextBuffer *pBuffer, bool bIsGameChat, tstring sChannelName) :
    refWidget(NULL),
    bGameChat(bIsGameChat),
    sChannel(sChannelName)
    {
        refWidget = pBuffer;
    }
};

const int CHAT_CLIENT_IS_OFFICER        (BIT(0));
const int CHAT_CLIENT_IS_CLAN_LEADER    (BIT(1));
const int CHAT_CLIENT_LOOKING_FOR_CLAN  (BIT(2));
const int CHAT_CLIENT_SENT_PING         (BIT(3));
const int CHAT_CLIENT_SENT_PING_2       (BIT(4));
const int CHAT_CLIENT_IS_STAFF          (BIT(5));
const int CHAT_CLIENT_IS_PREMIUM        (BIT(6));

typedef map<uint, SChatClient>  ChatClientMap;
typedef pair<uint, SChatClient> ChatClientPair;
typedef ChatClientMap::iterator ChatClientMap_it;

typedef map<tstring, tsvector>      IMMap;
typedef IMMap::iterator             IMMap_it;

typedef map<tstring, uint>          IMCountMap;
typedef IMCountMap::iterator        IMCountMap_it;

typedef map<uint, tsvector>         NotificationMap;
typedef pair<uint, tsvector>        NotificationPair;
typedef NotificationMap::iterator   NotificationMap_it;

typedef map<uint, SChatChannel>     ChatChannelMap;
typedef pair<uint, SChatChannel>    ChatChannelPair;
typedef ChatChannelMap::iterator    ChatChannelMap_it;

typedef map<uint, SChatBanned>      ChatBanMap;
typedef pair<uint, SChatBanned>     ChatBanPair;
typedef ChatBanMap::iterator        ChatBanMap_it;

typedef map<uint, tstring>          ChatIgnoreMap;
typedef pair<uint, tstring>         ChatIgnorePair;
typedef ChatIgnoreMap::iterator     ChatIgnoreMap_it;

typedef vector<SChatWidget>             ChatWidgetReference;
typedef ChatWidgetReference::iterator   ChatWidgetReference_it;

typedef map<uint, SChatChannelInfo>     ChatChannelInfoMap;
typedef pair<uint, SChatChannelInfo>    ChatChannelInfoPair;
typedef ChatChannelInfoMap::iterator    ChatChannelInfoMap_it;

enum EAdminLevel
{
    CHAT_CLIENT_ADMIN_NONE = 0,
    CHAT_CLIENT_ADMIN_OFFICER,
    CHAT_CLIENT_ADMIN_LEADER,
    CHAT_CLIENT_ADMIN_ADMINISTRATOR,
    CHAT_CLIENT_ADMIN_STAFF,
    CHAT_NUM_ADMIN_LEVELS,
};

const tstring g_sAdminNames[CHAT_NUM_ADMIN_LEVELS] =
{
    _T("chat_admin_level_none"),
    _T("chat_admin_level_officer"),
    _T("chat_admin_level_leader"),
    _T("chat_admin_level_administrator"),
    _T("chat_admin_level_staff"),
};

struct SGroupMemberInfo
{
    uint        uiAccountID;
    tstring     sName;
    byte        ySlot;
    short       nRating;
    byte        yLoadingPercent;
    byte        yReadyStatus;

    SGroupMemberInfo()
    {
        Clear();
    }

    void    Clear()
    {
        uiAccountID = INVALID_INDEX;
        sName.clear();
        ySlot = 0xff;
        nRating = 0;
        yLoadingPercent = 0;
        yReadyStatus = 0;
    }
};

const uint MAX_GROUP_SIZE(5);
//=============================================================================

//=============================================================================
// CChatManager
//=============================================================================
class CChatManager
{
    SINGLETON_DEF(CChatManager)

private:
    typedef list<SChatDBRequest*>       ChatRequestList;
    typedef ChatRequestList::iterator   ChatRequestList_it;

    CHTTPManager*   m_pHTTPManager;
    string          m_sMasterServerURL;

    // Old chat manager functionality
    tstring         m_sCurrentMessage;
    tstring         m_sChatType;


    // New chat manager functionality
    CSocket         m_sockChat;

    CDate           m_cDate;

    uint            m_uiConnectTimeout;
    uint            m_uiLastRecvTime;

    ChatClientMap   m_mapUserList;
    uiset           m_setBuddyList;
    uiset           m_setClanList;
    ChatBanMap      m_mapBanList;
    ChatIgnoreMap   m_mapIgnoreList;
    uint            m_uiIgnoreChat;
    uint            m_uiChatModeType;

    bool            m_bBuddyUpdateRequired;
    bool            m_bClanUpdateRequired;

    ChatRequestList m_lHTTPRequests;

    uint            m_uiConnectRetries;

    tsvector        m_vClanWhispers;
    sset            m_setLookingForClan;

    uint            m_uiAccountID;
    tstring         m_sCookie;

    sset            m_setAutoJoinChannels;

    ChatChannelMap  m_mapChannels;
    uiset           m_setChannelsIn;

    tsmapts         m_mapCensor;

    ResHandle       m_hStringTable;

    bool            m_bPrivateGame;
    bool            m_bHost;

    bool            m_bWhisperMode;
    list<tstring>   m_lLastWhispers;
    uint            m_uiTabNumber;

    bool            m_bRetrievingStats;

    uint            m_uiNextReconnectTime;

    CXMLDoc*        m_pRecentlyPlayed;

    tsvector        m_vNoteTimes;
    tsvector        m_vNotes;

    uint            m_uiMatchID;
    tstring         m_sGameName;

    sset            m_setRecentlyPlayed;

    CHTTPRequest*   m_pNamesRequest;

    ChatWidgetReference m_vWidgets;

    deque<tstring>  m_vChatHistory;
    uint            m_uiHistoryPos;

    uint            m_uiCreateTimeSent;

    tstring         m_sFollowName;
    bool            m_bFollow;

    tstring         m_sChatAddress;

    bool            m_bMatchStarted;
    bool            m_bWaitingToShowStats;
    uint            m_uiShowStatsMatchID;

    map<uint, tstring>  m_mapTournGameAddresses;

    uint            m_uiFocusedChannel;
    uint            m_uiFocusCount;
    tstring         m_sFocusedIM;

    IMMap           m_mapIMs;                       // Map of all IMs
    IMCountMap      m_mapIMUnreadCount;             // Map of all IM counts
    IMCountMap      m_mapIMFocusPriority;           // IM focus priority

    uint            m_uiLastIMNotificationTime;     // Timestamp of last IM received, to throttle IM notifications
    uint            m_uiReceivedIMCount;            // Total received IMs for current session
    uint            m_uiReadIMCount;                // Total read IMs for current session
    uint            m_uiSentIMCount;                // Total sent IMs for current session

    NotificationMap     m_mapNotifications;         // All the notifications for the client
    uint                m_uiNotificationIndex;      // The client keeps track of all notifications now so this is just a sequence number incremented

    byte            m_unChatSymbol;                 // Contains the id of the chat symbol this user has that should override the default symbol
    byte            m_unChatNameColor;              // Contains the id of the chat name color of the players name in chat channels

    friend class CWidgetReference;


    // Local client info
    EChatStatus         m_eStatus;
    
    // TMM info
    uint                m_uiTMMStartTime;
    uint                m_uiTMMAverageQueueTime;
    uint                m_uiTMMStdDevQueueTime;
    bool                m_bInGroup;                     // Is the player in a group
    bool                m_bTMMEnabled;                  // Whether or not TMM is enabled
    uint                m_uiTMMGroupLeaderID;
    bool                m_bTMMOtherPlayersReady;
    bool                m_bTMMAllPlayersReady;
    bool                m_bTMMMapLoaded;
    SGroupMemberInfo    m_aGroupInfo[MAX_GROUP_SIZE];
    uint                m_uiTMMSelfGroupIndex;

    ChatChannelInfoMap  m_mapChannelList;
    bool                m_bFinishedList;
    byte                m_yListStartSequence;
    byte                m_yProcessingListSequence;
    byte                m_yFinishedListSequence;

    tstring             m_sProcessingListHead;
    tstring             m_sFinishedListHead;

    bool                m_bInGameLobby;

    uint                GetAccountIDFromName(const tstring &sName);
    const tstring&      GetAccountNameFromID(uint uiAccountID);

    void                UpdateClientChannelStatus(const tstring &sNewChannel, const tstring &sName, uint uiAccountID, byte yStatus, byte yFlags, uint uiChatSymbol, uint uiChatNameColor, uint uiAccountIcon);

    void                UpdateReadyStatus();

public:
    ~CChatManager();

    // Old chat manager functionality
    K2_API void         AddIRCChatMessage(EChatMessageType eType, const tstring &sMessage = TSNULL, const tstring &sChannel = TSNULL, bool bTimeStamp = false);
    K2_API void         AddGameChatMessage(EChatMessageType eType, const tstring &sMessage = TSNULL);

    K2_API tstring      SetCurrentChatMessage(const tstring &sMessage);
    K2_API tstring      GetCurrentChatMessage()                             { return m_sCurrentMessage; }

    K2_API tstring      TabChatMessage(const tstring &sMessage);

    K2_API void         SetCurrentChatType(const tstring &sType)            { m_sChatType = sType; }
    K2_API tstring      GetCurrentChatType()                                { return m_sChatType; }

    // New chat manager functionality, to handle custom chat server
    void                Init(CHTTPManager *pHTTPManager);

    void                Frame();
    void                Connect(bool bInvisible = false);
    void                Disconnect();

    uint                GetAccountID() const                                { return m_uiAccountID; }

    void                SetInfo(int iAccountID, const tstring &sCookie, const tstring &sNickname, const tstring &sClan, const tstring &sClanTag, int iClanID, EClanRank eClanRank, byte yFlags, uint uiChatSymbol, uint uiChatNameColor, uint uiAccountIcon);

    void                SetPrivateGame(bool bPrivateGame)                   { m_bPrivateGame = bPrivateGame; }
    bool                GetPrivateGame() const                              { return m_bPrivateGame; }

    void                SetHost(bool bHost)                                 { m_bHost = bHost; }
    bool                GetHost() const                                     { return m_bHost; }

    void                ConnectingFrame();
    void                AuthFrame();
    void                ConnectedFrame();

    void                ProcessFailedRequest(SChatDBRequest *pRequest);
    void                ProcessAddBuddyLookupIDSuccess(SChatDBRequest *pRequest);
    void                ProcessAddBuddySuccess(SChatDBRequest *pRequest);
    void                ProcessRemoveBuddySuccess(SChatDBRequest *pRequest);
    void                ProcessClanPromoteSuccess(SChatDBRequest *pRequest);
    void                ProcessClanDemoteSuccess(SChatDBRequest *pRequest);
    void                ProcessClanRemoveSuccess(SChatDBRequest *pRequest);
    void                ProcessClanUpdateSuccess(SChatDBRequest *pRequest);
    void                ProcessClanNameCheckSuccess(SChatDBRequest *pRequest);
    void                ProcessBanLookupIDSuccess(SChatDBRequest *pRequest);
    void                ProcessAddBanSuccess(SChatDBRequest *pRequest);
    void                ProcessRemoveBanSuccess(SChatDBRequest *pRequest);
    void                ProcessIgnoreLookupIDSuccess(SChatDBRequest *pRequest);
    void                ProcessIgnoreAddSuccess(SChatDBRequest *pRequest);
    void                ProcessIgnoreRemoveSuccess(SChatDBRequest *pRequest);
    void                ProcessCompleteNickSuccess(SChatDBRequest *pRequest);
    void                ProcessSaveChannelSuccess(SChatDBRequest *pRequest);
    void                ProcessRemoveChannelSuccess(SChatDBRequest *pRequest);
    void                ProcessSaveNotificationResponse(SChatDBRequest *pRequest);
    void                ProcessRemoveNotificationResponse(SChatDBRequest *pRequest);
    void                ProcessRemoveAllNotificationsResponse(SChatDBRequest *pRequest);

    void                DatabaseFrame();

    bool                ProcessData(CPacket &pkt);

    void                AddBuddy(const uint uiAccountID, const tstring &sName, byte yFlags = 0);
    void                AddClanMember(const uint uiAccountID, const tstring &sName, byte yFlags = 0);
    void                AddBan(const uint uiAccountID, const tstring &sName, const tstring &sReason);
    void                AddIgnore(const uint uiAccountID, const tstring &sName);

    void                RemoveBuddy(const uint uiAccountID);
    inline void         RemoveBuddy(const tstring &sName)                   { RemoveBuddy(GetAccountIDFromName(sName)); }

    void                RemoveClanMember(uint uiAccountID);
    void                RemoveClanMember(const tstring &sName);
    void                RemoveBan(const uint uiAccountID);
    void                RemoveBan(const tstring &sName);
    void                RemoveIgnore(const uint uiAccountID);
    void                RemoveIgnore(const tstring &sName);

    void                ClearBuddyList()                                    { m_setBuddyList.clear(); }
    void                ClearClanList()                                     { m_setClanList.clear(); }
    void                ClearBanList()                                      { m_mapBanList.clear(); }
    void                ClearIgnoreList()                                   { m_mapIgnoreList.clear(); }

    void                GetBanList();

    void                RequestBuddyAdd(const tstring &sName);
    void                RequestBuddyApprove(const tstring &sName);

    void                RequestBuddyRemove(const uint uiAccountID);
    inline void         RequestBuddyRemove(const tstring &sName)            { RequestBuddyRemove(GetAccountIDFromName(sName)); }

    void                RequestBanlistAdd(const tstring &sName, const tstring &sReason);
    void                RequestBanlistRemove(uint uiAccountID);
    inline void         RequestBanlistRemove(const tstring &sName)          { RequestBanlistRemove(GetAccountIDFromName(sName)); }

    void                RequestIgnoreAdd(const tstring &sName);
    void                RequestIgnoreAdd(uint uiAccountID)                  { RequestIgnoreAdd(GetAccountNameFromID(uiAccountID)); }

    void                RequestIgnoreRemove(const uint uiAccountID);
    void                RequestIgnoreRemove(const tstring &sName)           { RequestIgnoreRemove(GetAccountIDFromName(sName)); }

    void                UpdateUserList(uint uiChannelID);
    void                UpdateBuddyList();
    void                UpdateClanList();

    void                UpdateRecentlyPlayed();

    inline byte         GetStatus() const                               { return byte(m_eStatus); }

    bool                IsUserInGame(const tstring &sUser);
    bool                IsUserInCurrentGame(const tstring &sUser);
    bool                IsUserOnline(const tstring &sUser);

    bool                SendChannelMessage(const tstring &sMessage, uint uiChannelID, uint eChatMessageType = CHAT_MESSAGE_ADD);
    bool                SendWhisper(const tstring &sName, const tstring &sMessage);
    bool                SendIM(const tstring &sName, const tstring &sMessage);
    bool                SendClanWhisper(const tstring &sMessage);

    K2_API bool         SubmitChatMessage(const tstring &sMessage, uint uiChannelID);

    void                UpdateWhispers(const tstring &sName);
    void                UpdateClanWhispers();
    void                UpdateLookingForClan();

    void                RequestPromoteClanMember(const tstring &sName);
    void                RequestDemoteClanMember(const tstring &sName);
    void                RequestRemoveClanMember(const tstring &sName);

    void                InviteToClan(const tstring &sName);

    void                AddToRecentlyPlayed(const tstring &sName);

    bool                UpdateHoverInfo(const tstring &sName);

    inline bool         IsConnected()                                       { return m_eStatus >= CHAT_STATUS_CONNECTED; }
    inline bool         IsConnecting()                                      { return m_eStatus > CHAT_STATUS_DISCONNECTED && m_eStatus < CHAT_STATUS_CONNECTED; }

    void                RefreshBuddyList()                                  { m_bBuddyUpdateRequired = true; }
    void                RefreshClanList()                                   { m_bClanUpdateRequired = true; }

    const tstring&      GetChannelName(uint uiChannelID);
    uint                GetChannelID(const tstring &sName);

    void                ClearAutoJoinChannels()                             { m_setAutoJoinChannels.clear(); }
    bool                IsSavedChannel(const tstring &sChannel);
    void                SaveChannelLocal(const tstring &sChannel);
    void                RemoveChannelLocal(const tstring &sChannel);
    void                SaveChannel(const tstring &sChannel);
    void                RemoveChannel(const tstring &sChannel);
    void                JoinChannel(const tstring &sChannel);
    void                JoinChannel(const tstring &sChannel, const tstring &sPassword);
    void                LeaveChannel(const tstring &sChannel);
    void                RequestChannelList();
    void                RequestChannelSublist(const tstring &sHead);
    void                ChannelSublistCancel();

    bool                IsFollowing(const tstring &sName);
    K2_API tstring      GetFollowing();
    K2_API void         UnFollow();
    void                UpdateFollow(const tstring &sServer);
    bool                SetFollowing(const tstring &sName);

    void                InviteUser(const tstring &sName);

    K2_API bool         IsBuddy(const tstring &sName);
    K2_API bool         IsBuddy(uint uiAccountID);
    bool                IsClanMember(const tstring &sName);
    bool                IsClanMember(uint uiAccountID);
    K2_API bool         IsBanned(const tstring &sName);
    K2_API bool         IsBanned(uint uiAccountID);
    K2_API bool         IsIgnored(const tstring &sName);
    K2_API bool         IsIgnored(uint uiAccountID);
    K2_API uint         GetIgnoreChat()                                     { return m_uiIgnoreChat; }
    K2_API void         SetIgnoreChat(const uint uiIgnoreChat)              { m_uiIgnoreChat = uiIgnoreChat; }

    uint                GetChatModeType()                                   { return m_uiChatModeType; }
    void                SetChatModeType(const uint uiChatModeType, const tstring &sReason, bool bSetDefaultMode = false);

    bool                IsInAClan(const tstring &sName);
    bool                IsInAClan(uint uiAccountID);

    bool                HasFlags(const tstring &sName, byte yFlags);
    bool                HasFlags(uint uiAccountID, byte yFlags);

    K2_API tstring      GetBanReason(const tstring &sName);
    K2_API tstring      GetBanReason(uint uiAccountID);

    K2_API void         PlaySound(const tstring &sSoundName);

    K2_API void         SendServerInvite(int iAccountID);
    K2_API void         SendServerInvite(const tstring &sName);
    K2_API void         RejectServerInvite(int iAccountID);

    void                GetUserInfo(const tstring &sName);
    void                RequestUserStatus(const tstring &sName);

    K2_API tstring      Translate(const tstring &sKey, const tstring &sParamName1 = TSNULL, const tstring &sParamValue1 = TSNULL, const tstring &sParamName2 = TSNULL, const tstring &sParamValue2 = TSNULL, const tstring &sParamName3 = TSNULL, const tstring &sParamValue3 = TSNULL, const tstring &sParamName4 = TSNULL, const tstring &sParamValue4 = TSNULL);
    K2_API tstring      Translate(const tstring &sKey, const tsmapts &mapParams);

    void                UpdateChannels();
    void                UpdateChannel(const uint uiChannelID);
    void                RebuildChannels();

    void                AutoCompleteNick(const tstring &sName);
    void                AutoCompleteClear();

    uint                GetLocalAccountID()                             { return m_uiAccountID; }

    void                AddWidgetReference(CTextBuffer *pBuffer, bool bIsGameChat, tstring sChannelName);

    void                SetRetrievingStats(bool bValue)                 { m_bRetrievingStats = bValue; }
    bool                IsRetrievingStats()                             { return m_bRetrievingStats; }

    bool                IsAdmin(uint uiChannelID, uint uiAccountID, EAdminLevel eMinLevel = CHAT_CLIENT_ADMIN_OFFICER);
    bool                IsAdmin(uint uiChannelID, const tstring &sName, EAdminLevel eMinLevel = CHAT_CLIENT_ADMIN_OFFICER);
    EAdminLevel         GetAdminLevel(uint uiChannelID, uint uiAccountID);
    EAdminLevel         GetAdminLevel(uint uiChannelID, const tstring &sName);

    void                SetChannelTopic(uint uiChannel, const tstring &sTopic);
    void                KickUserFromChannel(uint uiChannel, const tstring &sUser);
    void                BanUserFromChannel(uint uiChannel, const tstring &sUser);
    void                UnbanUserFromChannel(uint uiChannel, const tstring &sUser);
    void                SilenceChannelUser(uint uiChannel, const tstring &sUser, uint uiDuration);
    void                PromoteUserInChannel(uint uiChannelID, uint uiAccountID);
    void                PromoteUserInChannel(uint uiChannelID, const tstring &sName);
    void                DemoteUserInChannel(uint uiChannelID, uint uiAccountID);
    void                DemoteUserInChannel(uint uiChannelID, const tstring &sName);

    void                RequestAuthEnable(uint uiChannelID);
    void                RequestAuthDisable(uint uiChannelID);
    void                RequestAuthAdd(uint uiChannelID, const tstring &sName);
    void                RequestAuthRemove(uint uiChannelID, const tstring &sName);
    void                RequestAuthList(uint uiChannelID);

    void                SetChannelPassword(uint uiChannelID, const tstring &sPassword);

    void                SendGlobalMessage(const tstring &sMessage);

    void                SaveNotes();

    void                AdminKick(const tstring &sName);

    void                PreviousHistory()                   { if (m_uiHistoryPos < m_vChatHistory.size()) m_uiHistoryPos++; }
    void                NextHistory()                       { if (m_uiHistoryPos > 0) m_uiHistoryPos--; }
    tstring             GetCurrentChatHistory();
    void                AddChatHistory(const tstring &sChat)    { m_uiHistoryPos = 0; m_vChatHistory.push_front(sChat); }

    void                AcceptClanInvite();
    void                RejectClanInvite();

    void                CreateClan(const tstring &sName, const tstring &sTag, const tstring &sMember1, const tstring &sMember2, const tstring &sMember3, const tstring &sMember4);

    bool                CompareNames(const tstring &sOrig, const tstring &sName);
    bool                CompareNames(uint uiAccountID, const tstring &sName);

    void                CheckClanName(const tstring &sName, const tstring &sTag);

    void                SetAddress(const tstring &sAddress)     { m_sChatAddress = sAddress; }

    tstring             RemoveClanTag(const tstring &sName);

    K2_API void         ShowPostGameStats(uint uiMatchID);

    K2_API tstring      GetTournamentAddress(uint uiTournMatchID);

    void                AddUnreadChannel(uint uiChannelID);
    void                RemoveUnreadChannel(uint uiChannelID);
    K2_API void         SetFocusedChannel(const tstring &sChannel, const bool bForceFocus = false);
    K2_API void         SetFocusedChannel(const uint uiChannel, const bool bForceFocus = false);
    void                SetNextFocusedChannel();
    void                SetFocusedIM(const tstring &sName);
    void                SetNextFocusedIM();
    const tstring&      GetFocusedIM()              { return m_sFocusedIM; }
    void                CloseIM(const tstring &sName);

    uint                GetReceivedIMCount()        { return m_uiReceivedIMCount; }
    uint                GetReadIMCount()            { return m_uiReadIMCount; }
    uint                GetUnreadIMCount()          { int uiIMCount = 0; for (IMCountMap_it it(m_mapIMUnreadCount.begin()); it != m_mapIMUnreadCount.end(); it++) { uiIMCount += it->second; } return uiIMCount; }
    uint                GetSentIMCount()            { return m_uiSentIMCount; }
    uint                GetOpenIMCount()            { return (uint)m_mapIMs.size(); }

    void                ResetTabCounter()           { m_uiTabNumber = 0; }

    uint                AddReceivedIM()                         { return ++m_uiReceivedIMCount; }
    uint                AddReadIM()                             { return ++m_uiReadIMCount; }
    uint                AddReadIM(const uint uiCount)           { return m_uiReadIMCount += uiCount; }
    uint                AddUnreadIM(const tstring &sName);
    uint                AddSentIM()                             { return ++m_uiSentIMCount; }
    uint                RemoveUnreadIMs(const tstring &sName);

    void                ClearNotifications();
    void                PushNotification(const byte yType, const tstring &sParam1 = TSNULL, const tstring &sParam2 = TSNULL, const tstring &sParam3 = TSNULL, const tsvector &vParam4 = VSNULL, const uint uiExternalNotificationID = 0, const bool bSilent = false, const tstring &sNotificationTime = TSNULL);    // pushes the notification to the client interface via triggers
    void                AddNotification(const uint uiIndex, const tsvector vParam)  { m_mapNotifications.insert(NotificationPair(uiIndex, vParam)); }   // adds notification to the client notification map
    void                RemoveNotification(const uint uiIndex); // tries to remove the notification locally, requests a DB removal if it's a stored notification
    void                RemoveExternalNotification(const uint uiIndex); // after getting a response back from DB, this is called to remove the stored notification locally

    uint                GetNotificationIndex()                  { return m_uiNotificationIndex; }
    uint                IncrementNotificationIndex()            { return ++m_uiNotificationIndex; }
    uint                GetNotificationCount()                  { return (uint)m_mapNotifications.size(); }

    void                RequestSaveNotification(byte yType, const tstring &sParam1 = TSNULL, const tstring &sParam2 = TSNULL, const tstring &sParam3 = TSNULL, const tsvector vParam4 = VSNULL);  // tries to save notification to the DB
    void                RequestRemoveNotification(const uint uiInternalNotificationID, const uint uiExternalNotificationID);  // tries to remove notification from the DB based on it's external ID
    void                RequestRemoveAllNotifications();  // tries to request that all notifications from the DB and clear out the list when getting a response back
    void                ParseNotification(const tstring &sNotification, const uint uiExternalNotificationID = 0, bool bSilent = false);  // parses the delimited notification string

    // Functions to handle TMM stuff
    K2_API void     CreateTMMGroup(const byte yGameType = 1, const tstring &sMapName = _T("caldavar"), const tstring &sGameModes = _T("ap|sd|bd|bp|ar"), const tstring &sRegions = _T("USE|USW|EU"));
    K2_API void     JoinTMMGroup(const tstring sNickname);
    K2_API void     LeaveTMMGroup(bool bLocalOnly = false, const tstring &sReason = TSNULL);
    K2_API void     InviteToTMMGroup(const tstring sNickname);
    K2_API void     JoinTMMQueue();
    K2_API void     LeaveTMMQueue();
    K2_API void     RejectTMMInvite(const tstring sNickname);
    K2_API void     KickFromTMMGroup(const byte ySlotNumber);
    K2_API void     SendTMMGroupOptionsUpdate(byte yGameType, const tstring &sMapName, const tstring &sGameModes, const tstring &sRegions);
    K2_API void     RequestTMMPopularityUpdate();
    K2_API void     SendTMMPlayerLoadingUpdate(byte yPercent);
    K2_API void     SendTMMPlayerReadyStatus(const byte yReadyStatus);
    K2_API bool     IsInQueue();
    K2_API bool     IsInGroup();
    K2_API bool     IsTMMEnabled();
    K2_API uint     GetGroupLeaderID();
    K2_API bool     GetOtherPlayersReady();
    K2_API bool     GetAllPlayersReady();


    // Functions to handle game joining/leaving
    K2_API bool         JoinGame(const tstring &sName);
    K2_API void         JoiningGame(const tstring &sAddr);
    K2_API void         FinishedJoiningGame(const tstring &sName, const uint uiMatchID);
    K2_API void         LeftGame();
    K2_API void         MatchStarted();
    K2_API void         ResetGame();
    K2_API void         LeaveMatchChannels();
    bool                IsWaitingToShowStats() const            { return m_bWaitingToShowStats; }
    
    void                ClearWaitingToShowStats()
    {
        m_bWaitingToShowStats = false;
        m_uiShowStatsMatchID = INVALID_INDEX;
    }
    
    uint                GetShowStatsMatchID() const             { return m_uiShowStatsMatchID; }

    K2_API void         JoinGameLobby(bool bAllConnected);
    void                LeaveGameLobby();
    
    bool                IsInChannel(uint uiChannelID);

    K2_API void         InitCensor();
    K2_API bool         CensorChat(tstring &sMessage, const bool bInGameChat = true);

    void                RequestRefreshUpgrades();
    
    void                RequestGameInfo(const tstring sNickname);

    // Functions to handle data recieved from server
    void                HandlePing();
    void                HandleChannelInfo(CPacket &pkt);
    void                HandleChannelChange(CPacket &pkt);
    void                HandleChannelJoin(CPacket &pkt);
    void                HandleChannelLeave(CPacket &pkt);
    void                HandleChannelMessage(CPacket &pkt);
    void                HandleWhisper(CPacket &pkt);
    void                HandleWhisperBuddies(CPacket &pkt);
    void                HandleWhisperFailed();
    void                HandleIM(CPacket &pkt);
    void                HandleIMFailed(CPacket &pkt);
    void                HandleDisconnect();
    void                HandleInitialStatusUpdate(CPacket &pkt);
    void                HandleStatusUpdate(CPacket &pkt);
    void                HandleClanWhisper(CPacket &pkt);
    void                HandleClanWhisperFailed();
    void                HandleLookingForClan(CPacket &pkt);
    void                HandleNotLookingForClan(CPacket &pkt);
    void                HandleMultipleLookingForClan(CPacket &pkt);
    void                HandleFlooding();
    void                HandleMaxChannels();
    void                HandleServerInvite(CPacket &pkt);
    void                HandleInviteFailedUserNotFound();
    void                HandleInviteFailedNotInGame();
    void                HandleInviteRejected(CPacket &pkt);
    void                HandleUserInfoNoExist(CPacket &pkt);
    void                HandleUserInfoOffline(CPacket &pkt);
    void                HandleUserInfoOnline(CPacket &pkt);
    void                HandleUserInfoInGame(CPacket &pkt);
    void                HandleChannelUpdate(CPacket &pkt);
    void                HandleChannelTopic(CPacket &pkt);
    void                HandleChannelKick(CPacket &pkt);
    void                HandleChannelBan(CPacket &pkt);
    void                HandleChannelUnban(CPacket &pkt);
    void                HandleBannedFromChannel(CPacket &pkt);
    void                HandleChannelSilenced(CPacket &pkt);
    void                HandleChannelSilenceLifted(CPacket &pkt);
    void                HandleSilencePlaced(CPacket &pkt);
    void                HandleMessageAll(CPacket &pkt);
    void                HandleChannelPromote(CPacket &pkt);
    void                HandleChannelDemote(CPacket &pkt);
    void                HandleAuthAccepted();
    void                HandleRejected(CPacket &pkt);
    void                HandleChannelAuthEnabled(CPacket &pkt);
    void                HandleChannelAuthDisabled(CPacket &pkt);
    void                HandleChannelAddAuthUser(CPacket &pkt);
    void                HandleChannelRemoveAuthUser(CPacket &pkt);
    void                HandleChannelAddAuthUserFailed(CPacket &pkt);
    void                HandleChannelRemoveAuthUserFailed(CPacket &pkt);
    void                HandleChannelListAuth(CPacket &pkt);
    void                HandleChannelSetPassword(CPacket &pkt);
    void                HandleChannelJoinPassword(CPacket &pkt);
    void                HandleClanInvite(CPacket &pkt);
    void                HandleClanInviteRejected(CPacket &pkt);
    void                HandleClanInviteFailedOnline(CPacket &pkt);
    void                HandleClanInviteFailedClan(CPacket &pkt);
    void                HandleClanInviteFailedInvite(CPacket &pkt);
    void                HandleClanInviteFailedPermissions(CPacket &pkt);
    void                HandleClanInviteFailedUnknown(CPacket &pkt);
    void                HandleNewClanMember(CPacket &pkt);
    void                HandleClanRankChanged(CPacket &pkt);
    void                HandleClanCreateFailedClan(CPacket &pkt);
    void                HandleClanCreateFailedInvite(CPacket &pkt);
    void                HandleClanCreateFailedNotFound(CPacket &pkt);
    void                HandleClanCreateFailedDuplicate(CPacket &pkt);
    void                HandleClanCreateFailedParam(CPacket &pkt);
    void                HandleClanCreateFailedClanName(CPacket &pkt);
    void                HandleClanCreateFailedTag(CPacket &pkt);
    void                HandleClanCreateFailedUnknown(CPacket &pkt);
    void                HandleClanCreateAccept(CPacket &pkt);
    void                HandleClanCreateRejected(CPacket &pkt);
    void                HandleClanCreateComplete(CPacket &pkt);
    void                HandleNameChange(CPacket &pkt);
    void                HandleAutoMatchConnect(CPacket &pktfer);
    void                HandleServerNotIdle(CPacket &pktfer);
    void                HandleTournMatchReady(CPacket &pktfer);
    void                HandleAutoMatchStatus(CPacket &pktfer);
    void                HandleAutoMatchWaiting(CPacket &pktfer);
    void                HandleChatRoll(CPacket &pktfer);
    void                HandleChatEmote(CPacket &pktfer);
    void                HandleSetChatModeType(CPacket &pktfer);
    void                HandleChatModeAutoResponse(CPacket &pktfer);
    void                HandleUserCount(CPacket &pktfer);
    void                HandleTMMPlayerUpdates(CPacket &pkt);
    void                HandleTMMPopularityUpdates(CPacket &pkt);
    void                HandleTMMQueueUpdates(CPacket &pkt);
    void                HandleTMMJoinQueue(CPacket &pkt);
    void                HandleTMMLeaveQueue(CPacket &pkt);
    void                HandleTMMInviteToGroup(CPacket &pkt);
    void                HandleTMMInviteToGroupBroadcast(CPacket &pkt);
    void                HandleTMMRejectInvite(CPacket &pkt);
    void                HandleTMMMatchFound(CPacket &pkt);
    void                HandleTMMJoinFailed(CPacket &pkt);
    void                HandleRequestBuddyAddResponse(CPacket &pktfer);
    void                HandleRequestBuddyApproveResponse(CPacket &pktfer);
    void                HandleChannelInfoSub(CPacket &pkt);
    void                HandleUserStatus(CPacket &pkt);
    void                HandleRequestGameInfo(CPacket &pkt);
};

extern K2_API CChatManager *pChatManager;
#define ChatManager (*pChatManager)

#endif //__C_CHATMANAGER_H__
