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
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_STRING(	login_name,				"");
CVAR_STRING(	login_password,			"");
CVAR_STRING(	login_temp_password,	"");	// used to store the temporary password for the change password functionality of the UI
CVAR_BOOL(		login_rememberName,		false);
CVAR_BOOL(		login_rememberPassword,	false);
CVAR_BOOLF(		login_invisible,		false,	CVAR_SAVECONFIG);
CVAR_BOOL(		login_useSSL,			false);

CVAR_BOOLF(		_testTrialAccount,		false,	CVAR_SAVECONFIG);

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
m_pAuthRequest(NULL),
m_pChangePasswordRequest(NULL),
m_pSelectUpgradesRequest(NULL),
m_pRefreshUpgradesRequest(NULL),
m_pRefreshInfosRequest(NULL),

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
	m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
	m_pHTTPManager->ReleaseRequest(m_pChangePasswordRequest);
	m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
	m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
	m_pHTTPManager->ReleaseRequest(m_pRefreshInfosRequest);
}


/*====================
  CClientAccount::Disconnect
  ====================*/
void	CClientAccount::Disconnect(const tstring &sReason, EClientLoginStatus eStatus)
{
	m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
	m_pAuthRequest = NULL;

	m_pHTTPManager->ReleaseRequest(m_pChangePasswordRequest);
	m_pChangePasswordRequest = NULL;

	m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
	m_pSelectUpgradesRequest = NULL;

	m_pHTTPManager->ReleaseRequest(m_pRefreshUpgradesRequest);
	m_pRefreshUpgradesRequest = NULL;

	m_pHTTPManager->ReleaseRequest(m_pRefreshInfosRequest);
	m_pRefreshInfosRequest = NULL;

	m_eStatus = eStatus;
	m_sStatusDescription = sReason;
	m_uiAccountID = INVALID_ACCOUNT;
	m_uiClanID = -1;
	m_eClanRank = CLAN_RANK_NONE;
	m_iAccountType = 0;
	m_iTrialStatus = 0;
	m_sCookie.clear();
	m_sNick = L"UnnamedNewbie";
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
void	CClientAccount::Cancel()
{
	Disconnect(L"Canceled");
	m_eStatus = CLIENT_LOGIN_OFFLINE;
	m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_UNUSED;
}


/*====================
  CClientAccount::Logout
  ====================*/
void	CClientAccount::Logout()
{
	CHTTPRequest *pLogoutRequest(m_pHTTPManager->SpawnRequest());
	if (pLogoutRequest == NULL)
		return;

	pLogoutRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
	pLogoutRequest->AddVariable(L"f", L"logout");
	pLogoutRequest->AddVariable(L"cookie", m_sCookie);
	pLogoutRequest->SetReleaseOnCompletion(true);
	pLogoutRequest->SendPostRequest();

	Disconnect(L"Logged out");
	m_eStatus = CLIENT_LOGIN_OFFLINE;
	m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_UNUSED;
}


/*====================
  CClientAccount::Connect
  ====================*/
#ifdef K2_GARENA
void	CClientAccount::Connect(const tstring &sToken)
#else
void	CClientAccount::Connect(const tstring &sUser, const tstring &sPassword)
#endif
{
#ifndef K2_GARENA
	if (sUser.empty() || sPassword.empty())
	{
		Console << L"No username or password specified" << newl;
		return;
	}
#endif

	m_eStatus = CLIENT_LOGIN_WAITING;
	m_sStatusDescription.clear();
	m_uiAccountID = INVALID_ACCOUNT;
	m_iAccountType = 0;
	m_iTrialStatus = 0;
	m_sCookie.clear();
	m_sNick = L"UnnamedNewbie";
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

	// Send an authorization message to the DB
	m_pAuthRequest = m_pHTTPManager->SpawnRequest();
	if (m_pAuthRequest == NULL)
		return;

	m_pAuthRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");

#ifdef K2_GARENA
	m_pAuthRequest->AddVariable(L"f", L"token_auth");
	m_pAuthRequest->AddVariable(L"token", sToken);
#else
	m_pAuthRequest->AddVariable(L"f", L"auth");
	m_pAuthRequest->AddVariable(L"login", sUser);
	m_pAuthRequest->AddVariable(L"password", sPassword);
#endif

	m_pAuthRequest->SetTimeout(0);
	m_pAuthRequest->SetConnectTimeout(0);
	m_pAuthRequest->SetLowSpeedTimeout(0, 0);
		
	if (login_useSSL)
		m_pAuthRequest->SendSecurePostRequest();
	else
		m_pAuthRequest->SendPostRequest();

	AccountManager.ClearAccountID();
}


/*====================
  CClientAccount::ChangePassword
  ====================*/
void	CClientAccount::ChangePassword(const tstring &sUser, const tstring &sOldPassword, const tstring &sNewPassword, const tstring &sConfirmPassword)
{
	if (sUser.empty() || sOldPassword.empty() || sNewPassword.empty() || sConfirmPassword.empty())
	{
		Console << _T("You must specify a username, old password, new password and a confirm password to use this command.") << newl;
		return;
	}
	
	m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_WAITING;
	m_sChangePasswordStatusDescription = _T("Sending the request to change your password, please wait...");

	m_pChangePasswordRequest = m_pHTTPManager->SpawnRequest();
	if (m_pChangePasswordRequest == NULL)
		return;

	m_pChangePasswordRequest->SetTargetURL(L"www.heroesofnewerth.com/clientPassUpdate.php");
	m_pChangePasswordRequest->AddVariable(L"username", sUser);
	m_pChangePasswordRequest->AddVariable(L"old_password", sOldPassword);
	m_pChangePasswordRequest->AddVariable(L"new_password", sNewPassword);
	m_pChangePasswordRequest->AddVariable(L"confirm_password", sConfirmPassword);
	
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
void	CClientAccount::Frame()
{
	if (m_pAuthRequest != NULL && !m_pAuthRequest->IsActive())
	{
		if (m_pAuthRequest->WasSuccessful())
		{
			ProcessLoginResponse(m_pAuthRequest->GetResponse());
		}
		else
		{
			Disconnect(_T("Connection failed"));
			ClientLoginError.Trigger(L"1");
		}

		m_pHTTPManager->ReleaseRequest(m_pAuthRequest);
		m_pAuthRequest = NULL;
	}

	if (m_pChangePasswordRequest != NULL && !m_pChangePasswordRequest->IsActive())
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
		m_pChangePasswordRequest = NULL;
	}

	if (m_pSelectUpgradesRequest != NULL)
		SelectUpgradesStatus.Trigger(XtoA(m_pSelectUpgradesRequest->GetStatus()));

	if (m_pSelectUpgradesRequest != NULL && !m_pSelectUpgradesRequest->IsActive())
	{
		if (m_pSelectUpgradesRequest->WasSuccessful())
		{
			Console << _T("Upgrade select successful") << newl;

			if (m_bRequestChatServerRefresh)
			{
				ChatManager.RequestRefreshUpgrades();

				CHostClient *pClient(Host.GetActiveClient());
				if (pClient != NULL)
					Console.Execute(_T("ServerRefreshUpgrades"));
			}
		}
		else
		{
			Console << _T("Upgrade select failed") << newl;
		}

		m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);
		m_pSelectUpgradesRequest = NULL;
	}

	if (m_pRefreshUpgradesRequest != NULL)
		RefreshUpgradesStatus.Trigger(XtoA(m_pRefreshUpgradesRequest->GetStatus()));

	if (m_pRefreshUpgradesRequest != NULL && !m_pRefreshUpgradesRequest->IsActive())
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
		m_pRefreshUpgradesRequest = NULL;
	}

	if (m_pRefreshInfosRequest != NULL)
		RefreshInfosStatus.Trigger(XtoA(m_pRefreshInfosRequest->GetStatus()));

	if (m_pRefreshInfosRequest != NULL && !m_pRefreshInfosRequest->IsActive())
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
		m_pRefreshInfosRequest = NULL;
	}
}


/*====================
  CClientAccount::ProcessLoginResponse
  ====================*/
void	CClientAccount::ProcessLoginResponse(const tstring &sResponse)
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
	if (pError != NULL)
	{
		const CPHPData *pErrorCode(pError->GetVar(0));
		Disconnect(pErrorCode == NULL ? _T("Unknown error") : pErrorCode->GetString());
		return;
	}
	
	if (!phpResponse.GetString(_T("garena_auth")).empty())
	{
		wstring sResponse(phpResponse.GetString(L"garena_auth"));
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
		vParams[2] = phpResponse.GetString(L"nickname");
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
		vParams[2] = phpResponse.GetString(L"nickname");
		ClientLoginMustPurchase.Trigger(vParams);
		Disconnect(phpResponse.GetString(_T("auth")));
		return;
	}			

	// Authed successfully and got our data.
	m_iAccountType = phpResponse.GetInteger(L"account_type", -1);
	if (_testTrialAccount)
		m_iAccountType = 1;

	// MikeG Trial Account info
	if (m_iAccountType == 1)
	{
		m_iTrialStatus = phpResponse.GetInteger(L"trial", CLIENT_TRIAL_NONE);
		tsvector vParams(3);
		vParams[0] = _T("0"); // dont pop up the add just fill in the name.
		vParams[1] = XtoA(m_uiAccountID);	
		vParams[2] = phpResponse.GetString(L"nickname");
		ClientLoginMustPurchase.Trigger(vParams);
	}
	else
		m_iTrialStatus = CLIENT_TRIAL_NONE;

	m_sNick = phpResponse.GetString(L"nickname");
	m_sEmail = phpResponse.GetString(L"email");
	m_sIP = phpResponse.GetString(L"ip");
	m_sCookie = phpResponse.GetString(L"cookie");
	
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
	if (pInfos != NULL && pInfos->GetVar(_T("error")) == NULL)
	{
		pInfos = pInfos->GetVar(0);

		if (pInfos != NULL)
		{
			m_iLevel = pInfos->GetInteger(_T("level"));
			m_unRank = pInfos->GetInteger(_T("acc_pub_skill"));
			m_iTrialGames = pInfos->GetInteger(_T("acc_trial_games_played"));
			m_iGames = pInfos->GetInteger(_T("acc_games_played")) + pInfos->GetInteger(_T("rnk_games_played")) + pInfos->GetInteger(_T("cs_games_played"));// + pInfos->GetInteger(_T("acc_no_stats_played"));
			m_iDisconnects = pInfos->GetInteger(_T("acc_discos")) + pInfos->GetInteger(_T("rnk_discos")) + pInfos->GetInteger(_T("cs_discos"));

			const CPHPData *pMyUpgrades(phpResponse.GetVar(_T("my_upgrades")));
			if (pMyUpgrades != NULL)
			{
				uint uiNum(0);
				const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

				while (pUpgrade != NULL)
				{
					m_setAvailableUpgrades.insert(pUpgrade->GetString());
					pUpgrade = pMyUpgrades->GetVar(uiNum++);
				}
			}

			const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
			if (pSelectedUpgrades != NULL)
			{
				uint uiNum(0);
				const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

				while (pUpgrade != NULL)
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
	if (pClanInfo != NULL && pClanInfo->GetVar(_T("error")) == NULL)
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
	if (pTiers != NULL && pTiers->GetVar(_T("error")) == NULL)
	{
		uint uiNum(0);
		const CPHPData *pTierItem(pTiers->GetVar(uiNum++));

		while (pTierItem != NULL)
		{
			m_vTiers.push_back(pair<int, int>(pTierItem->GetInteger(_T("min")), pTierItem->GetInteger(_T("max"))));
			pTierItem = pTiers->GetVar(uiNum++);
		}
	}
	*/	

	// Grab banlist
	const CPHPData *pBanned(phpResponse.GetVar(_T("banned_list")));
	if (pBanned != NULL && pBanned->GetVar(_T("error")) == NULL)
	{
		pBanned = pBanned->GetVar(0);

		uint uiNum(0);
		const CPHPData *pBannedItem(pBanned->GetVar(uiNum++));

		while (pBannedItem != NULL)
		{
			ChatManager.AddBan(AtoI(pBannedItem->GetString(_T("banned_id"))), pBannedItem->GetString(_T("nickname")), pBannedItem->GetString(_T("reason")));
			pBannedItem = pBanned->GetVar(uiNum++);
		}
	}

	// Grab ignore list
	const CPHPData *pIgnore(phpResponse.GetVar(_T("ignored_list")));
	if (pIgnore != NULL && pIgnore->GetVar(_T("error")) == NULL)
	{
		pIgnore = pIgnore->GetVar(0);

		uint uiNum(0);
		const CPHPData *pIgnoreItem(pIgnore->GetVar(uiNum++));

		while (pIgnoreItem != NULL)
		{
			ChatManager.AddIgnore(AtoI(pIgnoreItem->GetString(_T("ignored_id"))), pIgnoreItem->GetString(_T("nickname")));
			pIgnoreItem = pIgnore->GetVar(uiNum++);
		}
	}

	// Grab clan list
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
	if (pBuddy != NULL && pBuddy->GetVar(_T("error")) == NULL)
	{
		pBuddy = pBuddy->GetVar(0);

		uint uiNum(0);
		const CPHPData *pBuddyItem(pBuddy->GetVar(uiNum++));

		while (pBuddyItem != NULL)
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
	if (pAutoJoinChannels != NULL && pAutoJoinChannels->GetVar(_T("error")) == NULL)
	{
		uint uiNum(0);
		const CPHPData *pAutoJoinChannel(pAutoJoinChannels->GetVar(uiNum++));

		while (pAutoJoinChannel != NULL)
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
	if (pNotifications != NULL && pNotifications->GetVar(_T("error")) == NULL)
	{
		uint uiNum(0);
		const CPHPData *pNotification(pNotifications->GetVar(uiNum++));

		while (pNotification != NULL)
		{
			tstring sNotification(pNotification->GetString(_T("notification")));
			ChatManager.ParseNotification(sNotification, pNotification->GetInteger(_T("notify_id")), true);
			pNotification = pNotifications->GetVar(uiNum++);
		}
	}

	m_uiCoins = phpResponse.GetInteger(_CTS("points"), 0);

	// Grab alt avatar info
	const CPHPData *pProducts(phpResponse.GetVar(_CTS("products")));
	if (pProducts != NULL)
	{
		const CPHPData *pAvatarList(pProducts->GetVar(_CTS("Alt Avatar")));
		if (pAvatarList != NULL)
		{
			uint uiNum(0);
			const CPHPData *pAvatar(pAvatarList->GetVar(uiNum++));
			while (pAvatar != NULL)
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
	Console.ExecuteScript(_CWS("~/reconnect.cfg"));
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
void	CClientAccount::ProcessPasswordChangeResponse(const tstring &sResponse)
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
	if (pError != NULL)
	{
		m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_FAILURE;
		m_sChangePasswordStatusDescription = pError->GetString();
		return;
	}

	const CPHPData *pSuccess(phpResponse.GetVar(_T("success")));
	if (pSuccess != NULL)
	{
		m_eChangePasswordStatus = CLIENT_CHANGE_PASSWORD_SUCCESS;
		m_sChangePasswordStatusDescription = pSuccess->GetString();
		return;
	}
}


/*====================
  CClientAccount::GetLeaverThreshold
  ====================*/
float	CClientAccount::GetLeaverThreshold(int iNumGames)
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
float	CClientAccount::GetLeaverThreshold() const
{
	return GetLeaverThreshold(GetGames());
}


/*====================
  CClientAccount::GetNextLeaverThreshold
  ====================*/
float	CClientAccount::GetNextLeaverThreshold() const
{
	return GetLeaverThreshold(GetGames() + 1);
}


/*====================
  CClientAccount::IsValidTier
  ====================*/
bool	CClientAccount::IsValidTier(int iTier) const
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
bool	CClientAccount::IsLeaver() const
{
	return GetLeaverPercent() > GetLeaverThreshold() + 0.001f;
}


/*====================
  CClientAccount::IsValidPSR
  ====================*/
bool	CClientAccount::IsValidPSR(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR) const
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
bool	CClientAccount::IsValidPSRForGameList(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR, const bool bFilter) const
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
bool	CClientAccount::WillBeLeaver() const
{
	return GetNextLeaverPercent() > GetNextLeaverThreshold() + 0.001f;
}


/*====================
  CClientAccount::GetUpdaterStatus
  ====================*/
uint	CClientAccount::GetUpdaterStatus() const
{
	return uint(K2Updater.GetStatus());
}


/*====================
  CClientAccount::UpdateUpgrades
  ====================*/
void	CClientAccount::UpdateUpgrades()
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
		}
	}
}


/*====================
  CClientAccount::SendSelectUpgradesRequest
  ====================*/
void	CClientAccount::SendSelectUpgradesRequest()
{
	m_pHTTPManager->ReleaseRequest(m_pSelectUpgradesRequest);

	m_pSelectUpgradesRequest = m_pHTTPManager->SpawnRequest();
	if (m_pSelectUpgradesRequest == NULL)
		return;

	m_pSelectUpgradesRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
	m_pSelectUpgradesRequest->AddVariable(L"f", L"selected_upgrades");
	m_pSelectUpgradesRequest->AddVariable(L"cookie", m_sCookie);

	for (tsmapts_it it(m_mapSelectedUpgrades.begin()); it != m_mapSelectedUpgrades.end(); ++it)
		m_pSelectUpgradesRequest->AddVariable(L"selected_upgrades[]", Upgrade_GetName(it->second));
	
	m_pSelectUpgradesRequest->SendPostRequest();
}


/*====================
  CClientAccount::SelectUpgrade
  ====================*/
void	CClientAccount::SelectUpgrade(const tstring &sProductCode)
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
void	CClientAccount::ClearUpgrade(const tstring &sType)
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
bool	CClientAccount::CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)
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
const SAvatarInfo*	CClientAccount::GetAvatarInfo(const tstring &sName) const
{
	MapAvatarInfo::const_iterator itFind(m_mapAvatarInfo.find(sName));
	if (itFind != m_mapAvatarInfo.end())
		return &itFind->second;
	else
		return NULL;
}


/*====================
  CClientAccount::RefreshUpgrades
  ====================*/
void	CClientAccount::RefreshUpgrades()
{
	// Send an authorization message to the DB
	m_pRefreshUpgradesRequest = m_pHTTPManager->SpawnRequest();
	if (m_pRefreshUpgradesRequest == NULL)
		return;

	m_pRefreshUpgradesRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
	m_pRefreshUpgradesRequest->AddVariable(L"f", L"get_upgrades");
	m_pRefreshUpgradesRequest->AddVariable(L"cookie", m_sCookie);
	m_pRefreshUpgradesRequest->SendPostRequest();
}


/*====================
  CClientAccount::RefreshInfos
  ====================*/
void	CClientAccount::RefreshInfos()
{
	// Send an authorization message to the DB
	m_pRefreshInfosRequest = m_pHTTPManager->SpawnRequest();
	if (m_pRefreshInfosRequest == NULL)
		return;

	m_pRefreshInfosRequest->SetTargetURL(K2System.GetMasterServerAddress() + "/client_requester.php");
	m_pRefreshInfosRequest->AddVariable(L"f", L"get_initStats");
	m_pRefreshInfosRequest->AddVariable(L"cookie", m_sCookie);
	m_pRefreshInfosRequest->SendPostRequest();
}


/*====================
  CClientAccount::ProcessRefreshUpgradesResponse
  ====================*/
void	CClientAccount::ProcessRefreshUpgradesResponse(const tstring &sResponse)
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
	if (pMyUpgrades != NULL)
	{
		uint uiNum(0);
		const CPHPData *pUpgrade(pMyUpgrades->GetVar(uiNum++));

		while (pUpgrade != NULL)
		{
			m_setAvailableUpgrades.insert(pUpgrade->GetString());
			pUpgrade = pMyUpgrades->GetVar(uiNum++);
		}
	}

	const CPHPData *pSelectedUpgrades(phpResponse.GetVar(_T("selected_upgrades")));
	if (pSelectedUpgrades != NULL)
	{
		uint uiNum(0);
		const CPHPData *pUpgrade(pSelectedUpgrades->GetVar(uiNum++));

		while (pUpgrade != NULL)
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
void	CClientAccount::ProcessRefreshInfosResponse(const tstring &sResponse)
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
	if (pInfos != NULL && pInfos->GetVar(_T("error")) == NULL)
	{
		pInfos = pInfos->GetVar(0);

		if (pInfos != NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
		return _T("-1");

	return XtoA(pClient->GetAccountID());
}


/*--------------------
  GetAccountName
  --------------------*/
UI_CMD(GetAccountName, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
		return TSNULL;

	return pClient->GetAccount().GetNickname();
}


/*--------------------
  GetCookie
  --------------------*/
UI_CMD(GetCookie, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
		return TSNULL;

	return pClient->GetCookie();
}


/*--------------------
  GetPSR
  --------------------*/
UI_CMD(GetPSR, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
		return TSNULL;

	return XtoA(pClient->GetAccount().GetPSR());
}


/*--------------------
  GetAccountEmail
  --------------------*/
UI_CMD(GetAccountEmail, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
		return TSNULL;

	return pClient->GetAccount().GetEmailAddress();
}


/*--------------------
  IsLoggedIn
  --------------------*/
UI_CMD(IsLoggedIn, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
		return _T("false");

	return XtoA(pClient->IsLoggedIn(), true);
}


/*--------------------
  IsLeaver
  --------------------*/
UI_CMD(IsLeaver, 0)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
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

#if 0	//Moved to CheckReconnect
	Console.ExecuteScript(_CWS("~/reconnect.cfg"));
#endif
}


/*--------------------
  SaveLoginInfo
  --------------------*/
UI_VOID_CMD(SaveLoginInfo, 0)
{
	CFileHandle hFile(_CWS("~/login.cfg"), FILE_WRITE | FILE_TEXT | FILE_TRUNCATE);
	if (!hFile.IsOpen())
		return;

	hFile << _CWS("// *** DO NOT EVER SHARE THIS FILE WITH ANYONE *** ") << newl;
	hFile << _CWS("// *** S2 STAFF WILL NOT ASK FOR THIS FILE *** ") << newl;
	hFile << _CWS("// *** EVEN THOUGH YOUR PASSWORD IS NOT VISIBLE *** ") << newl;
	hFile << _CWS("// *** THIS INFORMATION CAN BE USED TO STEAL YOUR ACCOUNT *** ") << newl;

	hFile << _CWS("login_rememberName ") << login_rememberName << newl;
	if (login_rememberName)
		hFile << _CWS("login_name ") << login_name << newl;
	hFile << _CWS("login_rememberPassword ") << login_rememberPassword << newl;
	if (login_rememberPassword)
		hFile << _CWS("login_password ") << login_password << newl;
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
	if (pClient == NULL)
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
	if (pClient == NULL)
		return WSNULL;
		
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
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
	if (pClient == NULL)
		return;

	pClient->RefreshUpgrades();
}


/*--------------------
  ClientRefreshInfos
  --------------------*/
CMD(ClientRefreshInfos)
{
	CHostClient *pClient(Host.GetActiveClient());
	if (pClient == NULL)
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
	if (pClient == NULL)
		return;

	pClient->RefreshInfos();
}


/*====================
  GetAccountLevel

  Get the current level from experience
  ====================*/
int		GetAccountLevel(float fExperience)
{
	return int(floor(sqrt((2.0f * ((fExperience / 100.0f) + 1.0f)) + 0.25f) - 0.5f));
}


/*====================
  GetAccountExperienceForLevel

  Get the experience required to reach a certain level
  ====================*/
float	GetAccountExperienceForLevel(int iLevel)
{
	return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountExperienceForCurrentLevel

  Get the experience required to reach a certain level
  ====================*/
float	GetAccountExperienceForCurrentLevel(float fExperience)
{
	int iLevel(GetAccountLevel(fExperience));

	return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountExperienceForNextLevel

  Get the experience required to reach a certain level
  ====================*/
float	GetAccountExperienceForNextLevel(float fExperience)
{
	int iLevel(GetAccountLevel(fExperience) + 1);

	return (((iLevel * iLevel + iLevel) / 2.0f) - 1.0f) * 100.0f;
}


/*====================
  GetAccountPercentNextLevel

  Get the experience required to reach a certain level
  ====================*/
float	GetAccountPercentNextLevel(float fExperience)
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