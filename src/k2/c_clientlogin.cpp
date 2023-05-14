// (C)2007 S2 Games
// c_clientlogin.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_clientlogin.h"

#include "c_chatmanager.h"
#include "c_accountmanager.h"
#include "c_uicmd.h"
#include "md5.h"
#include "c_hostclient.h"
#include "c_updater.h"
#include "c_filehttp.h"
#include "c_phpdata.h"
#include "c_httpmanager.h"
#include "c_httprequest.h"
#include "c_srp.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_STRING(    login_name,             "");
CVAR_STRING(    login_password,         "");
CVAR_STRING(    login_temp_password,    "");    // used to store the temporary password for the change password functionality of the UI
CVAR_BOOL(      login_rememberName,     false);
CVAR_BOOL(      login_rememberPassword, false);
CVAR_BOOLF(     login_invisible,        false,  CVAR_SAVECONFIG);
CVAR_BOOL(      login_useSSL,           false);
CVAR_BOOL(      login_useSRP,           true);

CVAR_BOOLF(     _testTrialAccount,      false,  CVAR_SAVECONFIG);

EXTERN_CVAR_STRING(cl_reconnectAddress);
EXTERN_CVAR_UINT(cl_reconnectMatchID);

UI_TRIGGER(ClientLoginError);
UI_TRIGGER(ClientLoginExpired);
UI_TRIGGER(ClientLoginMustPurchase);
UI_TRIGGER(GarenaClientLoginResponse);
UI_TRIGGER(SelectUpgradesStatus);
UI_TRIGGER(RefreshUpgradesStatus);
UI_TRIGGER(RefreshInfosStatus);
//=============================================================================

/*====================
  CClientAccount::CClientAccount
  ====================*/
CClientAccount::CClientAccount(CHTTPManager *pHTTPManager) :
m_pHTTPManager(pHTTPManager),
m_pSRP(nullptr),
m_pPreAuthRequest(nullptr),
m_pAuthRequest(nullptr),
m_pChangePasswordRequest(nullptr),
m_pSelectUpgradesRequest(nullptr),
m_pRefreshUpgradesRequest(nullptr),
m_pRefreshInfosRequest(nullptr),

m_eStatus(CLIENT_LOGIN_OFFLINE),
m_eChangePasswordStatus(CLIENT_CHANGE_PASSWORD_UNUSED),
m_iPasswordExpiration(-1),
m_uiTimeRemaining(-1),
m_uiAccountID(INVALID_ACCOUNT),
m_uiClanID(-1),
m_eClanRank(CLAN_RANK_NONE),
m_sNick(_T("UnnamedNewbie")),
m_fLeaverThreshold(0.0f),
m_iLevel(0),
m_unRank(0),
m_iGames(0),
m_iDisconnects(0),
m_iTrialGames(0),
m_iAccountType(0),
m_iTrialStatus(0),
m_yFlags(0),
m_uiChatSymbol(INVALID_INDEX),
m_uiChatNameColor(INVALID_INDEX),
m_uiAccountIcon(INVALID_INDEX),
m_uiAnnouncerVoice(INVALID_INDEX),
m_uiTaunt(INVALID_INDEX),
m_bRequestChatServerRefresh(false),
m_uiCoins(0)
{
}


/*====================
  CClientAccount::~CClientAccount
  ====================*/
CClientAccount::~CClientAccount()
{
    SAFE_DELETE(m_pSRP);
    m_pHTTPManager->ReleaseRequest(m_pPreAuthRequest);
    m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
    m_pHTTPManager->ReleaseRequest(m_pChangePasswordRequest);
    m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
    m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
    m_pHTTPManager->ReleaseRequest(m_pRefreshInfosRequest);
}


/*====================
  CClientAccount::Disconnect
  ====================*/
void    CClientAccount::Disconnect(const tstring &sReason, EClientLoginStatus eStatus)
{
    SAFE_DELETE(m_pSRP);

    m_pHTTPManager->ReleaseRequest(m_pPreAuthRequest);
    m_pPreAuthRequest = nullptr;

    m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
    m_pAuthRequest = nullptr;

    m_pHTTPManager->ReleaseRequest(m_pChangePasswordRequest);
    m_pChangePasswordRequest = nullptr;

    m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
    m_pSelectUpgradesRequest = nullptr;

    m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
    m_pRefreshUpgradesRequest = nullptr;

    m_pHTTPManager->ReleaseRequest(m_pRefreshInfosRequest);
    m_pRefreshInfosRequest = nullptr;

    m_eStatus = eStatus;
    m_sStatusDescription = sReason;
    m_uiAccountID = INVALID_ACCOUNT;
    m_uiClanID = -1;
    m_eClanRank = CLAN_RANK_NONE;
    m_iAccountType = 0;
    m_iTrialStatus = 0;
    m_sCookie.clear();
    m_sNick = _T("UnnamedNewbie");
    m_sClanName.clear();
    m_sClanTag.clear();
    m_sEmail.clear();

    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    m_uiAnnouncerVoice = INVALID_INDEX;
    m_uiTaunt = INVALID_INDEX;

    m_iLevel = 0;
    m_unRank = 0;
    m_iGames = 0;
    m_iDisconnects = 0;
    m_iTrialGames = 0;
    
    m_uiCoins = 0;
    m_mapAvatarInfo.clear();
    
    Console.Client << _T("Disconnected from authentication server: ") << sReason << newl;

    ChatManager.ClearBuddyList();
    ChatManager.ClearClanList();
    ChatManager.ClearBanList();
    ChatManager.ClearIgnoreList();  
    ChatManager.Disconnect();
}


/*====================
  CClientAccount::Cancel
  ====================*/
void    CClientAccount::Cancel()
{
    Disconnect(_T("Canceled"));
    m_eStatus = CLIENT_LOGIN_OFFLINE;
    m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_UNUSED;
}


/*====================
  CClientAccount::Logout
  ====================*/
void    CClientAccount::Logout()
{
    CHTTPRequest *pLogoutRequest(m_pHTTPManager->SpawnRequest());
    if (pLogoutRequest == nullptr)
        return;

    pLogoutRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    pLogoutRequest->AddVariable(_T("f"), _T("logout"));
    pLogoutRequest->AddVariable(_T("cookie"), m_sCookie);
    pLogoutRequest->SetReleaseOnCompletion(true);
    pLogoutRequest->SendPostRequest();

    Disconnect(_T("Logged out"));
    m_eStatus = CLIENT_LOGIN_OFFLINE;
    m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_UNUSED;
}


/*====================
  CClientAccount::Connect
  ====================*/
#ifdef K2_GARENA
void    CClientAccount::Connect(const tstring &sToken)
#else
void    CClientAccount::Connect(const tstring &sUser, const tstring &sPassword)
#endif
{
#ifndef K2_GARENA
    if (sUser.empty() || sPassword.empty())
    {
        Console << _T("No username or password specified") << newl;
        return;
    }
#endif

    m_eStatus = CLIENT_LOGIN_WAITING;
    m_sStatusDescription.clear();
    m_uiAccountID = INVALID_ACCOUNT;
    m_iAccountType = 0;
    m_iTrialStatus = 0;
    m_sCookie.clear();
    m_sNick = _T("UnnamedNewbie");
    m_sEmail.clear();
    m_setAvailableUpgrades.clear();
    m_mapSelectedUpgrades.clear();
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    m_uiAnnouncerVoice = INVALID_INDEX;
    m_uiTaunt = INVALID_INDEX;

    ChatManager.Disconnect();

    Console.Client << _T("Connecting to authentication server...") << newl;

    if (login_useSRP) {
        SAFE_DELETE(m_pSRP);
        m_pSRP = K2_NEW(ctx_Net, CSRP)();
        tstring sA = m_pSRP->Start(sUser, sPassword);

        m_pPreAuthRequest = m_pHTTPManager->SpawnRequest();
        if (m_pPreAuthRequest == nullptr)
            return;

        m_pPreAuthRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
        m_pPreAuthRequest->AddVariable(_T("f"), _T("pre_auth"));
        m_pPreAuthRequest->AddVariable(_T("login"), sUser);
        m_pPreAuthRequest->AddVariable(_T("A"), sA);
#ifdef WIN32
        m_pPreAuthRequest->AddVariable(_T("SysInfo"), "running on windows");
#else
        m_pPreAuthRequest->AddVariable(_T("SysInfo"), "not running on windows");
#endif

        m_pPreAuthRequest->SetTimeout(0);
        m_pPreAuthRequest->SetConnectTimeout(0);
        m_pPreAuthRequest->SetLowSpeedTimeout(0, 0);
        if (login_useSSL)
            m_pPreAuthRequest->SendSecurePostRequest();
        else
            m_pPreAuthRequest->SendPostRequest();
    } else {
        // Send an authorization message to the DB
        m_pAuthRequest = m_pHTTPManager->SpawnRequest();
        if (m_pAuthRequest == nullptr)
            return;

        m_pAuthRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");

#ifdef K2_GARENA
        m_pAuthRequest->AddVariable(_T("f"), _T("token_auth"));
        m_pAuthRequest->AddVariable(_T("token"), sToken);
#else
        m_pAuthRequest->AddVariable(_T("f"), _T("auth"));
        m_pAuthRequest->AddVariable(_T("login"), sUser);
        m_pAuthRequest->AddVariable(_T("password"), sPassword);
#endif

        m_pAuthRequest->SetTimeout(0);
        m_pAuthRequest->SetConnectTimeout(0);
        m_pAuthRequest->SetLowSpeedTimeout(0, 0);

        if (login_useSSL)
            m_pAuthRequest->SendSecurePostRequest();
        else
            m_pAuthRequest->SendPostRequest();
    }

    AccountManager.ClearAccountID();
}


/*====================
  CClientAccount::ChangePassword
  ====================*/
void    CClientAccount::ChangePassword(const tstring &sUser, const tstring &sOldPassword, const tstring &sNewPassword, const tstring &sConfirmPassword)
{
    if (sUser.empty() || sOldPassword.empty() || sNewPassword.empty() || sConfirmPassword.empty())
    {
        Console << _T("You must specify a username, old password, new password and a confirm password to use this command.") << newl;
        return;
    }
    
    m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_WAITING;
    m_sChangePasswordStatusDescription = _T("Sending the request to change your password, please wait...");

    m_pChangePasswordRequest = m_pHTTPManager->SpawnRequest();
    if (m_pChangePasswordRequest == nullptr)
        return;

    m_pChangePasswordRequest->SetTargetURL(_T("www.heroesofnewerth.com/clientPassUpdate.php"));
    m_pChangePasswordRequest->AddVariable(_T("username"), sUser);
    m_pChangePasswordRequest->AddVariable(_T("old_password"), sOldPassword);
    m_pChangePasswordRequest->AddVariable(_T("new_password"), sNewPassword);
    m_pChangePasswordRequest->AddVariable(_T("confirm_password"), sConfirmPassword);
    
    m_pChangePasswordRequest->SetTimeout(0);
    m_pChangePasswordRequest->SetConnectTimeout(0);
    m_pChangePasswordRequest->SetLowSpeedTimeout(0, 0);
    
    if (login_useSSL)
        m_pChangePasswordRequest->SendSecurePostRequest();
    else
        m_pChangePasswordRequest->SendPostRequest();    
}


/*====================
  CClientAccount::Frame
  ====================*/
void    CClientAccount::Frame()
{
    if (m_pPreAuthRequest != nullptr && !m_pPreAuthRequest->IsActive())
    {
        if (m_pPreAuthRequest->WasSuccessful())
        {
            ProcessLoginPreAuth(m_pPreAuthRequest->GetResponse());
        }
        else
        {
            Disconnect(_T("Connection failed"));
            ClientLoginError.Trigger(_T("1"));
        }

        m_pHTTPManager->ReleaseRequest(m_pPreAuthRequest);
        m_pPreAuthRequest = nullptr;
    }

    if (m_pAuthRequest != nullptr && !m_pAuthRequest->IsActive())
    {
        if (m_pAuthRequest->WasSuccessful())
        {
            ProcessLoginResponse(m_pAuthRequest->GetResponse());
        }
        else
        {
            Disconnect(_T("Connection failed"));
            ClientLoginError.Trigger(_T("1"));
        }

        m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
        m_pAuthRequest = nullptr;
    }

    if (m_pChangePasswordRequest != nullptr && !m_pChangePasswordRequest->IsActive())
    {
        if (m_pChangePasswordRequest->WasSuccessful())
        {
            ProcessPasswordChangeResponse(m_pChangePasswordRequest->GetResponse());
        }
        else
        {
            m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_FAILURE;
            m_sChangePasswordStatusDescription = _T("HTTP request failed: ") + m_pChangePasswordRequest->GetErrorBuffer();
        }

        m_pHTTPManager->ReleaseRequest(m_pChangePasswordRequest);
        m_pChangePasswordRequest = nullptr;
    }

    if (m_pSelectUpgradesRequest != nullptr)
        SelectUpgradesStatus.Trigger(XtoA(m_pSelectUpgradesRequest->GetStatus()));

    if (m_pSelectUpgradesRequest != nullptr && !m_pSelectUpgradesRequest->IsActive())
    {
        if (m_pSelectUpgradesRequest->WasSuccessful())
        {
            Console << _T("Upgrade select successful") << newl;

            if (m_bRequestChatServerRefresh)
            {
                ChatManager.RequestRefreshUpgrades();

                CHostClient *pClient(Host.GetActiveClient());
                if (pClient != nullptr)
                    Console.Execute(_T("ServerRefreshUpgrades"));
            }
        }
        else
        {
            Console << _T("Upgrade select failed") << newl;
        }

        m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
        m_pSelectUpgradesRequest = nullptr;
    }

    if (m_pRefreshUpgradesRequest != nullptr)
        RefreshUpgradesStatus.Trigger(XtoA(m_pRefreshUpgradesRequest->GetStatus()));

    if (m_pRefreshUpgradesRequest != nullptr && !m_pRefreshUpgradesRequest->IsActive())
    {
        if (m_pRefreshUpgradesRequest->WasSuccessful())
        {
            ProcessRefreshUpgradesResponse(m_pRefreshUpgradesRequest->GetResponse());
        }
        else
        {
            Console << _T("Refresh upgrades failed") << newl;
        }

        m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
        m_pRefreshUpgradesRequest = nullptr;
    }

    if (m_pRefreshInfosRequest != nullptr)
        RefreshInfosStatus.Trigger(XtoA(m_pRefreshInfosRequest->GetStatus()));

    if (m_pRefreshInfosRequest != nullptr && !m_pRefreshInfosRequest->IsActive())
    {
        if (m_pRefreshInfosRequest->WasSuccessful())
        {
            ProcessRefreshInfosResponse(m_pRefreshInfosRequest->GetResponse());
        }
        else
        {
            Console << _T("Refresh infos failed") << newl;
        }

        m_pHTTPManager->ReleaseRequest(m_pRefreshInfosRequest);
        m_pRefreshInfosRequest = nullptr;
    }
}


/*====================
  CClientAccount::ProcessLoginPreAuth
  ====================*/
void    CClientAccount::ProcessLoginPreAuth(const tstring &sResponse)
{
    if (sResponse.empty())
    {
        Disconnect(_T("Empty response from server"));
        return;
    }

    //Console << _T("Login response: ") << sResponse << newl;

    CPHPData phpResponse(sResponse);
    if (!phpResponse.IsValid())
    {
        Disconnect(_T("Bad Data - Please wait a few minutes and try again."));
        return;
    }

    phpResponse.Print();

    const CPHPData *pError(phpResponse.GetVar(_T("error")));
    if (pError != nullptr)
    {
        const CPHPData *pErrorCode(pError->GetVar(0));
        Disconnect(pErrorCode == nullptr ? _T("Unknown error") : pErrorCode->GetString());
        return;
    }

    auto sSalt(phpResponse.GetString(_T("salt")));
    auto sSalt2(phpResponse.GetString(_T("salt2")));
    auto sB(phpResponse.GetString(_T("B")));

    if (sSalt.empty())
    {
        Disconnect(_T("SRP salt empty"));
        return;
    }

    if (sSalt2.empty())
    {
        Disconnect(_T("SRP salt2 empty"));
        return;
    }

    if (sB.empty())
    {
        Disconnect(_T("SRP B empty"));
        return;
    }

    if (m_pSRP == nullptr)
    {
        Disconnect(_T("SRP is null"));
        return;
    }



    tstring sProof = m_pSRP->ProcessChallenge(sSalt, sSalt2, sB);
    if (sProof.empty())
    {
        Disconnect(_T("SRP proof empty"));
        return;
    }

    // Send an authorization message to the DB
    m_pAuthRequest = m_pHTTPManager->SpawnRequest();
    if (m_pAuthRequest == nullptr)
        return;

    m_pAuthRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");

    tstring sUser(m_pSRP->GetUsername());
    m_pAuthRequest->AddVariable(_T("f"), _T("srpAuth"));
    m_pAuthRequest->AddVariable(_T("login"), sUser);
    m_pAuthRequest->AddVariable(_T("proof"), sProof);
    m_pAuthRequest->AddVariable(_T("SysInfo"), _T("0"));

    m_pAuthRequest->SetTimeout(0);
    m_pAuthRequest->SetConnectTimeout(0);
    m_pAuthRequest->SetLowSpeedTimeout(0, 0);

    if (login_useSSL)
        m_pAuthRequest->SendSecurePostRequest();
    else
        m_pAuthRequest->SendPostRequest();
}


/*====================
  CClientAccount::ProcessLoginResponse
  ====================*/
void    CClientAccount::ProcessLoginResponse(const tstring &sResponse)
{
    if (sResponse.empty())
    {
        Disconnect(_T("Empty response from server"));
        return;
    }

    //Console << _T("Login response: ") << sResponse << newl;

    CPHPData phpResponse(sResponse);
    if (!phpResponse.IsValid())
    {
        Disconnect(_T("Bad Data - Please wait a few minutes and try again."));
        return;
    }
        
    //phpResponse.Print();

    const CPHPData *pError(phpResponse.GetVar(_T("error")));
    if (pError != nullptr)
    {
        const CPHPData *pErrorCode(pError->GetVar(0));
        Disconnect(pErrorCode == nullptr ? _T("Unknown error") : pErrorCode->GetString());
        return;
    }
    
    if (!phpResponse.GetString(_T("garena_auth")).empty())
    {
        tstring sResponse(phpResponse.GetString(_T("garena_auth")));
        GarenaClientLoginResponse.Trigger(sResponse);
        Disconnect(sResponse);
        return;
    }
    
    m_uiAccountID = phpResponse.GetInteger(_T("account_id"), INVALID_ACCOUNT);

    if (m_uiAccountID == INVALID_ACCOUNT)
    {
        Disconnect(phpResponse.GetString(_T("auth")));
        return;
    }
    
    if (CompareNoCase(phpResponse.GetString(_T("auth")), _T("#You must purchase an account to log in.")) == 0)
    {
        tsvector vParams(3);
        vParams[0] = _T("1");
        vParams[1] = XtoA(m_uiAccountID);   
        vParams[2] = phpResponse.GetString(_T("nickname"));
        ClientLoginMustPurchase.Trigger(vParams);
        Disconnect(phpResponse.GetString(_T("auth")));
        return;
    }
    
    // Trial Account Expired
    if (CompareNoCase(phpResponse.GetString(_T("auth")), _T("Trial account expired.")) == 0)
    {
        tsvector vParams(3);
        vParams[0] = _T("1");
        vParams[1] = XtoA(m_uiAccountID);   
        vParams[2] = phpResponse.GetString(_T("nickname"));
        ClientLoginMustPurchase.Trigger(vParams);
        Disconnect(phpResponse.GetString(_T("auth")));
        return;
    }           

    // Authed successfully and got our data.
    m_iAccountType = phpResponse.GetInteger(_T("account_type"), -1);
    if (_testTrialAccount)
        m_iAccountType = 1;

    // MikeG Trial Account info
    if (m_iAccountType == 1)
    {
        m_iTrialStatus = phpResponse.GetInteger(_T("trial"), CLIENT_TRIAL_NONE);
        tsvector vParams(3);
        vParams[0] = _T("0"); // dont pop up the add just fill in the name.
        vParams[1] = XtoA(m_uiAccountID);   
        vParams[2] = phpResponse.GetString(_T("nickname"));
        ClientLoginMustPurchase.Trigger(vParams);
    }
    else
        m_iTrialStatus = CLIENT_TRIAL_NONE;

    m_sNick = phpResponse.GetString(_T("nickname"));
    m_sEmail = phpResponse.GetString(_T("email"));
    m_sIP = phpResponse.GetString(_T("ip"));
    m_sCookie = phpResponse.GetString(_T("cookie"));
    
    // pass_exp null, no passchange needed, 
    // pass_exp > 0 show the passchange dialogue
    // pass_exp = 0, force the passchange dialogue
    if (phpResponse.GetString(_T("pass_exp")) == TSNULL)
        m_iPasswordExpiration = -1;
    else
        m_iPasswordExpiration = phpResponse.GetInteger(_T("pass_exp"));
    
    if (m_iPasswordExpiration == 0)
    {
        m_eStatus = CLIENT_LOGIN_EXPIRED;
        Disconnect(_T("Your password has expired"), CLIENT_LOGIN_EXPIRED);
        ClientLoginExpired.Trigger(XtoA(m_iPasswordExpiration));
        return;
    }
        
    m_eStatus = CLIENT_LOGIN_SUCCESS;   
    Console << _T("Login success: #") << m_uiAccountID << SPACE << m_sNick << _T(" [") << m_sCookie << _T("]") << newl;
        
    ChatManager.ClearBanList();
    ChatManager.ClearIgnoreList();
    ChatManager.ClearClanList();
    ChatManager.ClearBuddyList();
    ChatManager.ClearAutoJoinChannels();
    // Clear this out just in case, occasionally this acts a bit wierd but haven't tracked down exactly why
    ChatManager.ClearNotifications();   

    // Check for updates
    K2Updater.CheckForUpdates(true);

    // Set chat server address
    ChatManager.SetAddress(phpResponse.GetString(_T("chat_url")));

    // Grab infos
    const CPHPData *pInfos(phpResponse.GetVar(_T("infos")));
    if (pInfos != nullptr && pInfos->GetVar(_T("error")) == nullptr)
    {
        pInfos = pInfos->GetVar(0);

        if (pInfos != nullptr)
        {
            m_iLevel = pInfos->GetInteger(_T("level"));
            m_unRank = pInfos->GetInteger(_T("acc_pub_skill"));
            m_iTrialGames = pInfos->GetInteger(_T("acc_trial_games_played"));
            m_iGames = pInfos->GetInteger(_T("acc_games_played")) + pInfos->GetInteger(_T("rnk_games_played")) + pInfos->GetInteger(_T("cs_games_played"));// + pInfos->GetInteger(_T("acc_no_stats_played"));
            m_iDisconnects = pInfos->GetInteger(_T("acc_discos")) + pInfos->GetInteger(_T("rnk_discos")) + pInfos->GetInteger(_T("cs_discos"));

            const CPHPData *pMyUpgrades(phpResponse.GetVar(_T("my_upgrades")));
            if (pMyUpgrades != nullptr)
            {
                uint uiNum(0);
                const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

                while (pUpgrade != nullptr)
                {
                    m_setAvailableUpgrades.insert(pUpgrade->GetString());
                    pUpgrade = pMyUpgrades->GetVar(uiNum++);
                }
            }

            const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
            if (pSelectedUpgrades != nullptr)
            {
                uint uiNum(0);
                const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

                while (pUpgrade != nullptr)
                {
                    tstring sType(Upgrade_GetType(pUpgrade->GetString()));

                    m_mapSelectedUpgrades[sType] = pUpgrade->GetString();
                    pUpgrade = pSelectedUpgrades->GetVar(uiNum++);
                }
            }
        }
    }

    UpdateUpgrades();

    // Get clan info
    const CPHPData *pClanInfo(phpResponse.GetVar(_T("clan_member_info")));
    if (pClanInfo != nullptr && pClanInfo->GetVar(_T("error")) == nullptr)
    {
        m_sClanName = pClanInfo->GetString(_T("name"));
        m_uiClanID = pClanInfo->GetInteger(_T("clan_id"));
        m_sClanTag = pClanInfo->GetString(_T("tag"));
        
        tstring sRank(pClanInfo->GetString(_T("rank")));

        if (CompareNoCase(sRank, _T("Officer")) == 0)
            m_eClanRank = CLAN_RANK_OFFICER;
        else if (CompareNoCase(sRank, _T("Leader")) == 0)
            m_eClanRank = CLAN_RANK_LEADER;
        else
            m_eClanRank = CLAN_RANK_MEMBER;
    }

    if (!m_sClanTag.empty())
        m_sNick = _T("[") + m_sClanTag + _T("]") + m_sNick;

    m_yFlags = 0;

    if (m_iAccountType == 5)
        m_yFlags |= CHAT_CLIENT_IS_STAFF;
    else if (m_iAccountType == 4)
        m_yFlags |= CHAT_CLIENT_IS_PREMIUM;

    ChatManager.SetInfo(m_uiAccountID, m_sCookie, m_sNick, m_sClanName, m_sClanTag, m_uiClanID, m_eClanRank, m_yFlags, m_uiChatSymbol, m_uiChatNameColor, m_uiAccountIcon);

    m_fLeaverThreshold = phpResponse.GetFloat(_T("leaverthreshold"));
    
    // Grab tier info (Depreciated)
    /*  
    m_vTiers.clear();
    
    const CPHPData *pTiers(phpResponse.GetVar(_T("tiers")));
    if (pTiers != nullptr && pTiers->GetVar(_T("error")) == nullptr)
    {
        uint uiNum(0);
        const CPHPData *pTierItem(pTiers->GetVar(uiNum++));

        while (pTierItem != nullptr)
        {
            m_vTiers.push_back(pair<int, int>(pTierItem->GetInteger(_T("min")), pTierItem->GetInteger(_T("max"))));
            pTierItem = pTiers->GetVar(uiNum++);
        }
    }
    */  

    // Grab banlist
    const CPHPData *pBanned(phpResponse.GetVar(_T("banned_list")));
    if (pBanned != nullptr && pBanned->GetVar(_T("error")) == nullptr)
    {
        pBanned = pBanned->GetVar(0);

        uint uiNum(0);
        const CPHPData *pBannedItem(pBanned->GetVar(uiNum++));

        while (pBannedItem != nullptr)
        {
            ChatManager.AddBan(AtoI(pBannedItem->GetString(_T("banned_id"))), pBannedItem->GetString(_T("nickname")), pBannedItem->GetString(_T("reason")));
            pBannedItem = pBanned->GetVar(uiNum++);
        }
    }

    // Grab ignore list
    const CPHPData *pIgnore(phpResponse.GetVar(_T("ignored_list")));
    if (pIgnore != nullptr && pIgnore->GetVar(_T("error")) == nullptr)
    {
        pIgnore = pIgnore->GetVar(0);

        uint uiNum(0);
        const CPHPData *pIgnoreItem(pIgnore->GetVar(uiNum++));

        while (pIgnoreItem != nullptr)
        {
            ChatManager.AddIgnore(AtoI(pIgnoreItem->GetString(_T("ignored_id"))), pIgnoreItem->GetString(_T("nickname")));
            pIgnoreItem = pIgnore->GetVar(uiNum++);
        }
    }

    // Grab clan list
    const CPHPData *pClan(phpResponse.GetVar(_T("clan_roster")));
    if (pClan != nullptr && pClan->GetVar(_T("error")) == nullptr)
    {
        uint uiNum(0);
        const CPHPData *pClanItem(pClan->GetVar(uiNum++));

        while (pClanItem != nullptr)
        {
            tstring sRank(pClanItem->GetString(_T("rank")));
            tstring sName(pClanItem->GetString(_T("nickname")));
            int iAccountID(pClanItem->GetInteger(_T("account_id")));

            if (!m_sClanTag.empty())
                sName = _T("[") + m_sClanTag + _T("]") + sName;
            
            byte yFlags(0);

            if (CompareNoCase((sRank), _T("Leader")) == 0)
                yFlags |= CHAT_CLIENT_IS_CLAN_LEADER;
            else if (CompareNoCase((sRank), _T("Officer")) == 0)
                yFlags |= CHAT_CLIENT_IS_OFFICER;

            ChatManager.AddClanMember(iAccountID, sName, yFlags);
            pClanItem = pClan->GetVar(uiNum++);
        }
    }

    // Grab buddy list
    const CPHPData *pBuddy(phpResponse.GetVar(_T("buddy_list")));
    if (pBuddy != nullptr && pBuddy->GetVar(_T("error")) == nullptr)
    {
        pBuddy = pBuddy->GetVar(0);

        uint uiNum(0);
        const CPHPData *pBuddyItem(pBuddy->GetVar(uiNum++));

        while (pBuddyItem != nullptr)
        {
            tstring sName(pBuddyItem->GetString(_T("nickname")));
            tstring sTag(pBuddyItem->GetString(_T("clan_tag")));
            int iAccountID(pBuddyItem->GetInteger(_T("buddy_id")));

            if (!sTag.empty())
                sName = _T("[") + sTag + _T("]") + sName;

            ChatManager.AddBuddy(iAccountID, sName);
            pBuddyItem = pBuddy->GetVar(uiNum++);
        }
    }
    
    // Grab auto join channels
    const CPHPData *pAutoJoinChannels(phpResponse.GetVar(_T("chatrooms")));
    if (pAutoJoinChannels != nullptr && pAutoJoinChannels->GetVar(_T("error")) == nullptr)
    {
        uint uiNum(0);
        const CPHPData *pAutoJoinChannel(pAutoJoinChannels->GetVar(uiNum++));

        while (pAutoJoinChannel != nullptr)
        {
            // only allow them to join a maximum of 8 channels for now
            if (uiNum < 8)
            {           
                ChatManager.SaveChannelLocal(pAutoJoinChannel->GetString());
                pAutoJoinChannel = pAutoJoinChannels->GetVar(uiNum++);
            }
            else
                break;
        }
    }
    
    // Grab notifications
    const CPHPData *pNotifications(phpResponse.GetVar(_T("notifications")));
    if (pNotifications != nullptr && pNotifications->GetVar(_T("error")) == nullptr)
    {
        uint uiNum(0);
        const CPHPData *pNotification(pNotifications->GetVar(uiNum++));

        while (pNotification != nullptr)
        {
            tstring sNotification(pNotification->GetString(_T("notification")));
            ChatManager.ParseNotification(sNotification, pNotification->GetInteger(_T("notify_id")), true);
            pNotification = pNotifications->GetVar(uiNum++);
        }
    }

    m_uiCoins = phpResponse.GetInteger(_CTS("points"), 0);

    // Grab alt avatar info
    const CPHPData *pProducts(phpResponse.GetVar(_CTS("products")));
    if (pProducts != nullptr)
    {
        const CPHPData *pAvatarList(pProducts->GetVar(_CTS("Alt Avatar")));
        if (pAvatarList != nullptr)
        {
            uint uiNum(0);
            const CPHPData *pAvatar(pAvatarList->GetVar(uiNum++));
            while (pAvatar != nullptr)
            {
                SAvatarInfo info;
                info.sName = pAvatar->GetString(_CTS("name"));
                info.sCName = pAvatar->GetString(_CTS("cname"));
                info.uiCost = pAvatar->GetInteger(_CTS("cost"));

                m_mapAvatarInfo[info.sName] = info;

                pAvatar = pAvatarList->GetVar(uiNum++);
            }
        }
    }

    // Check Reconnect
    Console.ExecuteScript(_CTS("~/reconnect.cfg"));
    Host.GetActiveClient()->SetReconnect(cl_reconnectAddress, cl_reconnectMatchID); 

    //phpResponse.Print();
    const CPHPData *pReconnect(phpResponse.GetVar(_T("reconnect")));
    if (pReconnect)
    {
        //pReconnect->Print();
        tstring sIP(pReconnect->GetString(_T("ip"), _T("127.0.0.1")));
        uint uiPort(pReconnect->GetInteger(_T("port"), -1));
        uint uiMatchID(pReconnect->GetInteger(_T("match_id"), -1));
        Host.GetActiveClient()->SetReconnect(sIP + _T(":") + XtoA(uiPort), uiMatchID ); 
    }

    Host.GetActiveClient()->CheckReconnect();
}

/*====================
  CClientAccount::ProcessPasswordChangeResponse
  ====================*/
void    CClientAccount::ProcessPasswordChangeResponse(const tstring &sResponse)
{
    if (sResponse.empty())
    {
        m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_FAILURE;
        m_sChangePasswordStatusDescription = _T("Empty response");
        return;
    }

    const CPHPData phpResponse(sResponse);
    if (!phpResponse.IsValid())
    {
        m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_FAILURE;
        m_sChangePasswordStatusDescription = _T("Invalid response");
        return;
    }

    const CPHPData *pError(phpResponse.GetVar(_T("error")));
    if (pError != nullptr)
    {
        m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_FAILURE;
        m_sChangePasswordStatusDescription = pError->GetString();
        return;
    }

    const CPHPData *pSuccess(phpResponse.GetVar(_T("success")));
    if (pSuccess != nullptr)
    {
        m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_SUCCESS;
        m_sChangePasswordStatusDescription = pSuccess->GetString();
        return;
    }
}


/*====================
  CClientAccount::GetLeaverThreshold
  ====================*/
float   CClientAccount::GetLeaverThreshold(int iNumGames)
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
  CClientAccount::GetLeaverThreshold
  ====================*/
float   CClientAccount::GetLeaverThreshold() const
{
    return GetLeaverThreshold(GetGames());
}


/*====================
  CClientAccount::GetNextLeaverThreshold
  ====================*/
float   CClientAccount::GetNextLeaverThreshold() const
{
    return GetLeaverThreshold(GetGames() + 1);
}


/*====================
  CClientAccount::IsValidTier
  ====================*/
bool    CClientAccount::IsValidTier(int iTier) const
{
    return true;

    if (m_vTiers.size() == 0)
        return true;

    if (iTier < 0 || iTier >= int(m_vTiers.size()))
        return false;

    if (m_iLevel < m_vTiers[iTier].first ||
        m_iLevel > m_vTiers[iTier].second)
        return false;

    return true;
}


/*====================
  CClientAccount::IsLeaver
  ====================*/
bool    CClientAccount::IsLeaver() const
{
    return GetLeaverPercent() > GetLeaverThreshold() + 0.001f;
}


/*====================
  CClientAccount::IsValidPSR
  ====================*/
bool    CClientAccount::IsValidPSR(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR) const
{
    if (unMinPSR)
    {   
        const ushort unMinTolerance(unServerMinPSR / 1.02f);
        
        if (iRank < unMinTolerance)
            return false;
    }

    if (unMaxPSR)
    {
        const int unMaxTolerance(unServerMaxPSR * 1.02f);

        if (iRank > unMaxTolerance)
            return false;
    }
    
    return true;
}

/*====================
  CClientAccount::IsValidPSRForGameList
  ====================*/
bool    CClientAccount::IsValidPSRForGameList(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR, const bool bFilter) const
{
    // if bFilter == true, this is used for filtering out games in the game list per their filter settings  
    // if bFilter == false, this is used for graying/disabling out games on the game list that the client is ineligable for
    
    if (bFilter)
    {
        if (unMinPSR)
        {   
            if (unServerMinPSR >= unMinPSR)
                return true;
            else
                return false;
        }

        if (unMaxPSR)
        {
            if (unServerMaxPSR <= unMaxPSR)
                return true;
            else
                return false;
        }       
    }
    else
    {           
        if (unServerMinPSR)
        {   
            const ushort unMinTolerance(unServerMinPSR / 1.02f);
            
            if (iRank <= unMinTolerance)
                return false;
        }

        if (unServerMaxPSR)
        {
            const int unMaxTolerance(unServerMaxPSR * 1.02f);

            if (iRank >= unMaxTolerance)
                return false;
        }
    }
    
    return true;        
}


/*====================
  CClientAccount::WillBeLeaver
  ====================*/
bool    CClientAccount::WillBeLeaver() const
{
    return GetNextLeaverPercent() > GetNextLeaverThreshold() + 0.001f;
}


/*====================
  CClientAccount::GetUpdaterStatus
  ====================*/
uint    CClientAccount::GetUpdaterStatus() const
{
    return uint(K2Updater.GetStatus());
}


/*====================
  CClientAccount::UpdateUpgrades
  ====================*/
void    CClientAccount::UpdateUpgrades()
{
    // Clear all upgrades so that omitted entries reset back to defaults
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    m_uiAnnouncerVoice = INVALID_INDEX;
    m_uiTaunt = INVALID_INDEX;

    for (tsmapts_it it(m_mapSelectedUpgrades.begin()); it != m_mapSelectedUpgrades.end(); ++it)
    {
        tstring sType(Upgrade_GetType(it->second));
        tstring sName(Upgrade_GetName(it->second));

        EUpgradeType eType(GetUpgradeTypeFromString(sType));

        switch (eType)
        {
        case UPGRADE_CHAT_SYMBOL:
            m_uiChatSymbol = Host.LookupChatSymbol(sName);
            break;
        case UPGRADE_CHAT_NAME_COLOR:
            m_uiChatNameColor = Host.LookupChatNameColor(sName);
            break;
        case UPGRADE_ACCOUNT_ICON:
            m_uiAccountIcon = Host.LookupAccountIcon(sName);
            break;
        case UPGRADE_ANNOUNCER_VOICE:
            m_uiAnnouncerVoice = Host.LookupAnnouncerVoice(sName);
            break;
        case UPGRADE_TAUNT:
            m_uiTaunt = Host.LookupTaunt(sName);
            break;
        case UPGRADE_VALUE_INVALID:
            K2_UNREACHABLE();
            break;
        }
    }
}


/*====================
  CClientAccount::SendSelectUpgradesRequest
  ====================*/
void    CClientAccount::SendSelectUpgradesRequest()
{
    m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);

    m_pSelectUpgradesRequest = m_pHTTPManager->SpawnRequest();
    if (m_pSelectUpgradesRequest == nullptr)
        return;

    m_pSelectUpgradesRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pSelectUpgradesRequest->AddVariable(_T("f"), _T("selected_upgrades"));
    m_pSelectUpgradesRequest->AddVariable(_T("cookie"), m_sCookie);

    for (tsmapts_it it(m_mapSelectedUpgrades.begin()); it != m_mapSelectedUpgrades.end(); ++it)
        m_pSelectUpgradesRequest->AddVariable(_T("selected_upgrades[]"), Upgrade_GetName(it->second));
    
    m_pSelectUpgradesRequest->SendPostRequest();
}


/*====================
  CClientAccount::SelectUpgrade
  ====================*/
void    CClientAccount::SelectUpgrade(const tstring &sProductCode)
{
    tstring sType(Upgrade_GetType(sProductCode));

    if (m_setAvailableUpgrades.find(sProductCode) == m_setAvailableUpgrades.end())
        return;

    EUpgradeType eType(GetUpgradeTypeFromString(sType));

    if (eType == UPGRADE_CHAT_SYMBOL ||
        eType == UPGRADE_CHAT_NAME_COLOR ||
        eType == UPGRADE_ACCOUNT_ICON)
    {
        m_bRequestChatServerRefresh = true;
    }
    else
    {
        m_bRequestChatServerRefresh = false;
    }

    m_mapSelectedUpgrades[sType] = sProductCode;

    SendSelectUpgradesRequest();

    UpdateUpgrades();
}


/*====================
  CClientAccount::ClearUpgrade
  ====================*/
void    CClientAccount::ClearUpgrade(const tstring &sType)
{
    if (m_mapSelectedUpgrades.find(sType) == m_mapSelectedUpgrades.end())
        return;

    EUpgradeType eType(GetUpgradeTypeFromString(sType));

    if (eType == UPGRADE_CHAT_SYMBOL ||
        eType == UPGRADE_CHAT_NAME_COLOR ||
        eType == UPGRADE_ACCOUNT_ICON)
    {
        m_bRequestChatServerRefresh = true;
    }
    else
    {
        m_bRequestChatServerRefresh = false;
    }

    m_mapSelectedUpgrades.erase(sType);

    SendSelectUpgradesRequest();

    UpdateUpgrades();
}


/*====================
  CClientAccount::CanAccessAltAvatar
  ====================*/
bool    CClientAccount::CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)
{
#if 0
    return true;
#endif

    tstring sProductCode(_T("aa.") + sHero + _T(".") + sAltAvatar);

    return m_setAvailableUpgrades.find(sProductCode) != m_setAvailableUpgrades.end();
}


/*====================
  CClientAccount::GetAvatarInfo
  ====================*/
const SAvatarInfo*  CClientAccount::GetAvatarInfo(const tstring &sName) const
{
    MapAvatarInfo::const_iterator itFind(m_mapAvatarInfo.find(sName));
    if (itFind != m_mapAvatarInfo.end())
        return &itFind->second;
    else
        return nullptr;
}


/*====================
  CClientAccount::RefreshUpgrades
  ====================*/
void    CClientAccount::RefreshUpgrades()
{
    // Send an authorization message to the DB
    m_pRefreshUpgradesRequest = m_pHTTPManager->SpawnRequest();
    if (m_pRefreshUpgradesRequest == nullptr)
        return;

    m_pRefreshUpgradesRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pRefreshUpgradesRequest->AddVariable(_T("f"), _T("get_upgrades"));
    m_pRefreshUpgradesRequest->AddVariable(_T("cookie"), m_sCookie);
    m_pRefreshUpgradesRequest->SendPostRequest();
}


/*====================
  CClientAccount::RefreshInfos
  ====================*/
void    CClientAccount::RefreshInfos()
{
    // Send an authorization message to the DB
    m_pRefreshInfosRequest = m_pHTTPManager->SpawnRequest();
    if (m_pRefreshInfosRequest == nullptr)
        return;

    m_pRefreshInfosRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
    m_pRefreshInfosRequest->AddVariable(_T("f"), _T("get_initStats"));
    m_pRefreshInfosRequest->AddVariable(_T("cookie"), m_sCookie);
    m_pRefreshInfosRequest->SendPostRequest();
}


/*====================
  CClientAccount::ProcessRefreshUpgradesResponse
  ====================*/
void    CClientAccount::ProcessRefreshUpgradesResponse(const tstring &sResponse)
{
    if (sResponse.empty())
        return;

    CPHPData phpResponse(sResponse);
    if (!phpResponse.IsValid())
    {
        Console << _T("Bad Data") << newl;
        return;
    }

    m_setAvailableUpgrades.clear();
    m_mapSelectedUpgrades.clear();

    const CPHPData *pMyUpgrades(phpResponse.GetVar(_T("my_upgrades")));
    if (pMyUpgrades != nullptr)
    {
        uint uiNum(0);
        const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

        while (pUpgrade != nullptr)
        {
            m_setAvailableUpgrades.insert(pUpgrade->GetString());
            pUpgrade = pMyUpgrades->GetVar(uiNum++);
        }
    }

    const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
    if (pSelectedUpgrades != nullptr)
    {
        uint uiNum(0);
        const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

        while (pUpgrade != nullptr)
        {
            tstring sType(Upgrade_GetType(pUpgrade->GetString()));

            m_mapSelectedUpgrades[sType] = pUpgrade->GetString();
            pUpgrade = pSelectedUpgrades->GetVar(uiNum++);
        }
    }

    UpdateUpgrades();

    m_uiCoins = phpResponse.GetInteger(_CTS("points"), 0);
}


/*====================
  CClientAccount::ProcessRefreshInfosResponse
  ====================*/
void    CClientAccount::ProcessRefreshInfosResponse(const tstring &sResponse)
{
    if (sResponse.empty())
        return;

    CPHPData phpResponse(sResponse);
    if (!phpResponse.IsValid())
    {
        Console << _T("Bad Data") << newl;
        return;
    }

    // Grab infos
    const CPHPData *pInfos(phpResponse.GetVar(_T("infos")));
    if (pInfos != nullptr && pInfos->GetVar(_T("error")) == nullptr)
    {
        pInfos = pInfos->GetVar(0);

        if (pInfos != nullptr)
        {
            m_iLevel = pInfos->GetInteger(_T("level"));
            m_unRank = pInfos->GetInteger(_T("acc_pub_skill"));
            m_iTrialGames = pInfos->GetInteger(_T("acc_trial_games_played"));
            m_iGames = pInfos->GetInteger(_T("acc_games_played")) + pInfos->GetInteger(_T("rnk_games_played")) + pInfos->GetInteger(_T("cs_games_played"));// + pInfos->GetInteger(_T("acc_no_stats_played"));
            m_iDisconnects = pInfos->GetInteger(_T("acc_discos")) + pInfos->GetInteger(_T("rnk_discos")) + pInfos->GetInteger(_T("cs_discos"));
        }
    }
}


/*--------------------
  Login
  --------------------*/
CMD(Login)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

#ifdef K2_GARENA
    pClient->Login(Host.GetGarenaToken());
#else
    pClient->Login(login_name, login_password);
#endif

    return true;
}

UI_VOID_CMD(Login, 0)
{
    cmdLogin();
}


/*--------------------
  CancelLogin
  --------------------*/
CMD(CancelLogin)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->CancelLogin();
    return true;
}

UI_VOID_CMD(CancelLogin, 0)
{
    cmdCancelLogin();
}


/*--------------------
  Logout
  --------------------*/
CMD(Logout)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;

    pClient->Logout();
    return true;
}

UI_VOID_CMD(Logout, 0)
{
    cmdLogout();
}


/*--------------------
  GetAccountID
  --------------------*/
UI_CMD(GetAccountID, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return _T("-1");

    return XtoA(pClient->GetAccountID());
}


/*--------------------
  GetAccountName
  --------------------*/
UI_CMD(GetAccountName, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return pClient->GetAccount().GetNickname();
}


/*--------------------
  GetCookie
  --------------------*/
UI_CMD(GetCookie, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return pClient->GetCookie();
}


/*--------------------
  GetPSR
  --------------------*/
UI_CMD(GetPSR, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return XtoA(pClient->GetAccount().GetPSR());
}


/*--------------------
  GetAccountEmail
  --------------------*/
UI_CMD(GetAccountEmail, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;

    return pClient->GetAccount().GetEmailAddress();
}


/*--------------------
  IsLoggedIn
  --------------------*/
UI_CMD(IsLoggedIn, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return _T("false");

    return XtoA(pClient->IsLoggedIn(), true);
}


/*--------------------
  IsLeaver
  --------------------*/
UI_CMD(IsLeaver, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return _T("false");

    if (pClient->IsLeaver())
        return _T("true");
    else
        return _T("false");
}


/*--------------------
  CheckReconnect
  --------------------*/
UI_VOID_CMD(CheckReconnect, 0)
{
    if (!Host.GetActiveClient())
        return;
    
    Host.GetActiveClient()->CheckReconnect();

#if 0   //Moved to CheckReconnect
    Console.ExecuteScript(_CTS("~/reconnect.cfg"));
#endif
}


/*--------------------
  SaveLoginInfo
  --------------------*/
UI_VOID_CMD(SaveLoginInfo, 0)
{
    CFileHandle hFile(_CTS("~/login.cfg"), FILE_WRITE | FILE_TEXT | FILE_TRUNCATE);
    if (!hFile.IsOpen())
        return;

    hFile << _CTS("// *** DO NOT EVER SHARE THIS FILE WITH ANYONE *** ") << newl;
    hFile << _CTS("// *** S2 STAFF WILL NOT ASK FOR THIS FILE *** ") << newl;
    hFile << _CTS("// *** EVEN THOUGH YOUR PASSWORD IS NOT VISIBLE *** ") << newl;
    hFile << _CTS("// *** THIS INFORMATION CAN BE USED TO STEAL YOUR ACCOUNT *** ") << newl;

    hFile << _CTS("login_rememberName ") << login_rememberName << newl;
    if (login_rememberName)
        hFile << _CTS("login_name ") << login_name << newl;
    hFile << _CTS("login_rememberPassword ") << login_rememberPassword << newl;
    if (login_rememberPassword)
        hFile << _CTS("login_password ") << login_password << newl;
}


/*--------------------
  MD5
  --------------------*/
UI_CMD(MD5, 1)
{
    return MD5String(TStringToUTF8(vArgList[0]->Evaluate()));
}


/*--------------------
  ChangePassword
  --------------------*/
CMD(ChangePassword)
{
    if (vArgList.size() < 2)
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;
        
    pClient->ChangePassword(login_name, login_temp_password, vArgList[0], vArgList[1]);
    
    return true;
}

UI_VOID_CMD(ChangePassword, 2)
{
    cmdChangePassword(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  IsTrialAccount
  --------------------*/
UI_CMD(IsTrialAccount, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return TSNULL;
        
    return XtoA(pClient->GetAccount().GetAccountType() == 1);
}


/*--------------------
  SelectUpgrade
  --------------------*/
CMD(SelectUpgrade)
{
    if (vArgList.size() < 1)
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;
        
    pClient->SelectUpgrade(vArgList[0]);
    return true;
}


/*--------------------
  SelectUpgrade
  --------------------*/
UI_VOID_CMD(SelectUpgrade, 1)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;
        
    pClient->SelectUpgrade(vArgList[0]->Evaluate());
}


/*--------------------
  ClearUpgrade
  --------------------*/
CMD(ClearUpgrade)
{
    if (vArgList.size() < 1)
        return false;

    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return false;
        
    pClient->ClearUpgrade(vArgList[0]);
    return true;
}


/*--------------------
  ClearUpgrade
  --------------------*/
UI_VOID_CMD(ClearUpgrade, 1)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;
        
    pClient->ClearUpgrade(vArgList[0]->Evaluate());
}


/*--------------------
  GetChatSymbolTexturePathFromUpgrades
  --------------------*/
UI_CMD(GetChatSymbolTexturePathFromUpgrades, 1)
{
    tsvector vUpgrades(TokenizeString(vArgList[0]->Evaluate(), _T(',')));

    for (tsvector_it it(vUpgrades.begin()); it != vUpgrades.end(); ++it)
    {
        tstring sType(Upgrade_GetType(*it));
        tstring sName(Upgrade_GetName(*it));

        EUpgradeType eType(GetUpgradeTypeFromString(sType));

        switch (eType)
        {
        case UPGRADE_CHAT_SYMBOL:
            return Host.GetChatSymbolTexturePath(Host.LookupChatSymbol(sName));
        default:
            break;
        }
    }

    return TSNULL;
}


/*--------------------
  GetChatNameColorTexturePathFromUpgrades
  --------------------*/
UI_CMD(GetChatNameColorTexturePathFromUpgrades, 1)
{
    tsvector vUpgrades(TokenizeString(vArgList[0]->Evaluate(), _T(',')));

    for (tsvector_it it(vUpgrades.begin()); it != vUpgrades.end(); ++it)
    {
        tstring sType(Upgrade_GetType(*it));
        tstring sName(Upgrade_GetName(*it));

        EUpgradeType eType(GetUpgradeTypeFromString(sType));

        switch (eType)
        {
        case UPGRADE_CHAT_NAME_COLOR:
            return Host.GetChatNameColorTexturePath(Host.LookupChatNameColor(sName));
        default:
            break;
        }
    }

    return TSNULL;
}


/*--------------------
  GetChatNameColorStringFromUpgrades
  --------------------*/
UI_CMD(GetChatNameColorStringFromUpgrades, 1)
{
    tsvector vUpgrades(TokenizeString(vArgList[0]->Evaluate(), _T(',')));

    for (tsvector_it it(vUpgrades.begin()); it != vUpgrades.end(); ++it)
    {
        tstring sType(Upgrade_GetType(*it));
        tstring sName(Upgrade_GetName(*it));

        EUpgradeType eType(GetUpgradeTypeFromString(sType));

        switch (eType)
        {
        case UPGRADE_CHAT_NAME_COLOR:
            return Host.GetChatNameColorString(Host.LookupChatNameColor(sName));
        default:
            break;
        }
    }

    return TSNULL;
}


/*--------------------
  GetAccountIconTexturePathFromUpgrades
  --------------------*/
UI_CMD(GetAccountIconTexturePathFromUpgrades, 1)
{
    tsvector vUpgrades(TokenizeString(vArgList[0]->Evaluate(), _T(',')));

    for (tsvector_it it(vUpgrades.begin()); it != vUpgrades.end(); ++it)
    {
        tstring sType(Upgrade_GetType(*it));
        tstring sName(Upgrade_GetName(*it));

        EUpgradeType eType(GetUpgradeTypeFromString(sType));

        switch (eType)
        {
        case UPGRADE_ACCOUNT_ICON:
            return Host.GetAccountIconTexturePath(Host.LookupAccountIcon(sName));
        default:
            break;
        }
    }

    return TSNULL;
}


/*--------------------
  PrintAvailableUpgrades
  --------------------*/
CMD(PrintAvailableUpgrades)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return true;
        
    //pClient->PrintAvailableUpgrades();
    return true;
}


/*--------------------
  ClientRefreshUpgrades
  --------------------*/
CMD(ClientRefreshUpgrades)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return true;

    pClient->RefreshUpgrades();

    return true;
}


/*--------------------
  ClientRefreshUpgrades
  --------------------*/
UI_VOID_CMD(ClientRefreshUpgrades, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    pClient->RefreshUpgrades();
}


/*--------------------
  ClientRefreshInfos
  --------------------*/
CMD(ClientRefreshInfos)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return true;

    pClient->RefreshInfos();

    return true;
}


/*--------------------
  ClientRefreshInfos
  --------------------*/
UI_VOID_CMD(ClientRefreshInfos, 0)
{
    CHostClient *pClient(Host.GetActiveClient());
    if (pClient == nullptr)
        return;

    pClient->RefreshInfos();
}


/*====================
  GetAccountLevel

  Get the current level from experience
  ====================*/
int     GetAccountLevel(float fExperience)
{
    return int(floor(sqrt((2.0f * ((fExperience / 100.0f) + 1.0f)) + 0.25f) - 0.5f));
}


/*====================
  GetAccountExperienceForLevel

  Get the experience required to reach a certain level
  ====================*/
float   GetAccountExperienceForLevel(int iLevel)
{
    return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountExperienceForCurrentLevel

  Get the experience required to reach a certain level
  ====================*/
float   GetAccountExperienceForCurrentLevel(float fExperience)
{
    int iLevel(GetAccountLevel(fExperience));

    return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountExperienceForNextLevel

  Get the experience required to reach a certain level
  ====================*/
float   GetAccountExperienceForNextLevel(float fExperience)
{
    int iLevel(GetAccountLevel(fExperience) + 1);

    return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountPercentNextLevel

  Get the experience required to reach a certain level
  ====================*/
float   GetAccountPercentNextLevel(float fExperience)
{
    return ILERP(fExperience, GetAccountExperienceForLevel(GetAccountLevel(fExperience)), GetAccountExperienceForNextLevel(fExperience));
}


/*--------------------
  GetAccountLevel
  --------------------*/
UI_CMD(GetAccountLevel, 1)
{
    float fExperience(AtoF(vArgList[0]->Evaluate()));

    return XtoA(GetAccountLevel(fExperience));
}


/*--------------------
  GetAccountExperienceForLevel
  --------------------*/
UI_CMD(GetAccountExperienceForLevel, 1)
{
    int iLevel(AtoI(vArgList[0]->Evaluate()));

    return XtoA(GetAccountExperienceForLevel(iLevel));
}


/*--------------------
  GetAccountExperienceForCurrentLevel
  --------------------*/
UI_CMD(GetAccountExperienceForCurrentLevel, 1)
{
    float fExperience(AtoF(vArgList[0]->Evaluate()));

    return XtoA(GetAccountExperienceForCurrentLevel(fExperience));
}



/*--------------------
  GetAccountExperienceForNextLevel
  --------------------*/
UI_CMD(GetAccountExperienceForNextLevel, 1)
{
    float fExperience(AtoF(vArgList[0]->Evaluate()));

    return XtoA(GetAccountExperienceForNextLevel(fExperience));
}


/*--------------------
  GetAccountPercentNextLevel
  --------------------*/
UI_CMD(GetAccountPercentNextLevel, 1)
{
    float fExperience(AtoF(vArgList[0]->Evaluate()));

    return XtoA(GetAccountPercentNextLevel(fExperience));
}