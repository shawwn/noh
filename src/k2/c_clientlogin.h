// (C)2007 S2 Games
// c_clientlogin.h
//
//=============================================================================
#ifndef __C_CLIENTLOGIN_H__
#define __C_CLIENTLOGIN_H__

//=============================================================================
// Declarations
//=============================================================================
class CHTTPManager;
class CHTTPRequest;
class CSRP;
//=============================================================================

//=============================================================================
// Defintions
//=============================================================================
struct SServerInfo
{
    tstring m_sAddress;
    uint    m_uiPing;
};

enum EClientLoginStatus
{
    CLIENT_LOGIN_OFFLINE,
    CLIENT_LOGIN_WAITING,
    CLIENT_LOGIN_SUCCESS,
    CLIENT_LOGIN_FAILURE,
    CLIENT_LOGIN_EXPIRED
};

enum EClientChangePasswordStatus
{
    CLIENT_CHANGE_PASSWORD_UNUSED,
    CLIENT_CHANGE_PASSWORD_WAITING,
    CLIENT_CHANGE_PASSWORD_SUCCESS,
    CLIENT_CHANGE_PASSWORD_FAILURE
};

enum EClanRank
{
    CLAN_RANK_NONE,
    CLAN_RANK_MEMBER,
    CLAN_RANK_OFFICER,
    CLAN_RANK_LEADER,
    NUM_CLAN_RANKS
};

enum ETrialStatus
{
    CLIENT_TRIAL_NONE,
    CLIENT_TRIAL_ACTIVE,
    CLIENT_TRIAL_EXPIRED
};

const tstring g_sClanRankNames[NUM_CLAN_RANKS] =
{
    _T("chat_clan_rank_none"),
    _T("chat_clan_rank_member"),
    _T("chat_clan_rank_officer"),
    _T("chat_clan_rank_leader"),
};

struct SAvatarInfo
{
    tstring sName;
    tstring sCName;
    uint    uiCost;
};

typedef pair<int, int>  iipair;
typedef vector<iipair>  TierVector;
typedef map<tstring, SAvatarInfo>   MapAvatarInfo;
//=============================================================================

//=============================================================================
// CClientAccount
//=============================================================================
class CClientAccount
{
    friend class CAccountManager;

private:
    CHTTPManager*               m_pHTTPManager;
    CSRP*                       m_pSRP;
    CHTTPRequest*               m_pPreAuthRequest;
    CHTTPRequest*               m_pAuthRequest;
    CHTTPRequest*               m_pChangePasswordRequest;
    CHTTPRequest*               m_pSelectUpgradesRequest;
    CHTTPRequest*               m_pRefreshUpgradesRequest;
    CHTTPRequest*               m_pRefreshInfosRequest;

    uint                        m_uiTimeRemaining;
    int                         m_iPasswordExpiration;
    uint                        m_uiAccountID;
    int                         m_iAccountType;
    int                         m_iTrialStatus;
    tstring                     m_sCookie;
    tstring                     m_sNick;
    tstring                     m_sEmail;
    tstring                     m_sIP;
    byte                        m_yFlags;
    
    uint                        m_uiClanID;
    tstring                     m_sClanName;
    tstring                     m_sClanTag;
    EClanRank                   m_eClanRank;

    EClientLoginStatus          m_eStatus;
    tstring                     m_sStatusDescription;
    
    EClientChangePasswordStatus m_eChangePasswordStatus;
    tstring                     m_sChangePasswordStatusDescription;

    int                         m_iLevel;
    ushort                      m_unRank;
    int                         m_iGames;
    int                         m_iDisconnects;
    int                         m_iTrialGames;

    TierVector                  m_vTiers;
    float                       m_fLeaverThreshold;

    sset                        m_setAvailableUpgrades;
    tsmapts                     m_mapSelectedUpgrades;

    uint                        m_uiChatSymbol;
    uint                        m_uiChatNameColor;
    uint                        m_uiAccountIcon;
    uint                        m_uiAnnouncerVoice;
    uint                        m_uiTaunt;

    bool                        m_bRequestChatServerRefresh;

    uint                        m_uiCoins;
    MapAvatarInfo               m_mapAvatarInfo;

    CClientAccount();

    void    ProcessLoginPreAuth(const tstring &sResponse);
    void    ProcessLoginResponse(const tstring &sResponse);
    void    ProcessPasswordChangeResponse(const tstring &sResponse);
    void    ProcessRefreshUpgradesResponse(const tstring &sResponse);
    void    ProcessRefreshInfosResponse(const tstring &sResponse);

    void    UpdateUpgrades();

    void    SendSelectUpgradesRequest();

public:
    ~CClientAccount();
    CClientAccount(CHTTPManager *pHTTPManager);

#ifdef K2_GARENA
    void            Connect(const tstring &sToken);
#else
    void            Connect(const tstring &sUser, const tstring &sPassword);
#endif
    void            Disconnect(const tstring &sReason, EClientLoginStatus eStatus = CLIENT_LOGIN_FAILURE);
    void            Cancel();
    void            Logout();
    void            Frame();

    EClientLoginStatus  GetStatus() const               { return m_eStatus; }
    const tstring&      GetStatusDescription() const    { return m_sStatusDescription; }
    const int           GetPasswordExpiration() const   { return m_iPasswordExpiration; }

    EClientChangePasswordStatus GetChangePasswordStatus() const             { return m_eChangePasswordStatus; }
    const tstring&              GetChangePasswordStatusDescription() const  { return m_sChangePasswordStatusDescription; }
    
    const tstring&  GetCookie() const           { return m_sCookie; }
    uint            GetAccountID() const        { return m_uiAccountID; }
    int             GetAccountType() const      { return m_iAccountType; }
    const tstring&  GetIP() const               { return m_sIP; }
    const tstring&  GetNickname() const         { return m_sNick; }
    const tstring&  GetEmailAddress() const     { return m_sEmail; }
    int             GetLevel() const            { return m_iLevel; }
    ushort          GetPSR() const              { return m_unRank; }
    int             GetGames() const            { return m_iGames; }
    int             GetDisconnects() const      { return m_iDisconnects; }
    const tstring&  GetClanName() const         { return m_sClanName; }
    const tstring&  GetClanTag() const          { return m_sClanTag; }
    uint            GetClanID() const           { return m_uiClanID; }
    EClanRank       GetClanRank() const         { return m_eClanRank; }
    byte            GetFlags() const            { return m_yFlags; }
    uint            GetChatSymbol() const       { return m_uiChatSymbol; }
    uint            GetChatNameColor() const    { return m_uiChatNameColor; }
    uint            GetAccountIcon() const      { return m_uiAccountIcon; }
    uint            GetAnnouncerVoice() const   { return m_uiAnnouncerVoice; }
    uint            GetTaunt() const            { return m_uiTaunt; }

    inline void     SetNickname(const tstring &sName)   { m_sNick = sName; }
    void            ChangePassword(const tstring &sUser, const tstring &sOldPassword, const tstring &sNewPassword, const tstring &sConfirmPassword);

    bool            IsLoggedIn() const              { return (m_uiAccountID != INVALID_ACCOUNT); }

    K2_API bool     IsValidTier(int iTier) const;
    K2_API bool     IsLeaver() const;
    K2_API bool     WillBeLeaver() const;
    K2_API bool     IsValidPSR(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR) const;
    K2_API bool     IsValidPSRForGameList(const int iRank, const ushort unMinPSR, const ushort unMaxPSR, const ushort unServerMinPSR, const ushort unServerMaxPSR, const bool bFilter) const;
    
    // MikeG Trial Account Client Stuff
    int                 GetNoStatsGames() const             { return m_iTrialGames; }
    int                 GetTrialStatus() const              { return m_iTrialStatus; }
    K2_API bool         IsTrialExpired() const              { return (m_iTrialStatus >= CLIENT_TRIAL_ACTIVE) ? ((m_iTrialGames >= MAX_TRIAL_GAMES) || m_iTrialStatus == CLIENT_TRIAL_EXPIRED) : false; };
    inline void         SetTrialExpired()                   { m_iTrialStatus = CLIENT_TRIAL_EXPIRED; }
    inline void         SetTrialGames(uint uiTrialCount)    { m_iTrialGames = uiTrialCount; }

    float           GetLeaverPercent() const        { return GetGames() > 0 ? float(GetDisconnects()) / GetGames() : 0.0f; }
    float           GetNextLeaverPercent() const    { return float(GetDisconnects() + 1) / (GetGames() + 1); }
    K2_API float    GetLeaverThreshold() const;
    K2_API float    GetNextLeaverThreshold() const;

    K2_API static float     GetLeaverThreshold(int iNumGames);

    K2_API uint     GetUpdaterStatus() const;

    K2_API void     SelectUpgrade(const tstring &sProductCode);
    K2_API void     ClearUpgrade(const tstring &sType);
    K2_API bool     CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar);

    K2_API uint     GetCoins() const                        { return m_uiCoins; }
    K2_API const SAvatarInfo*   GetAvatarInfo(const tstring &sName) const;

    K2_API void     RefreshUpgrades();
    K2_API void     RefreshInfos();
};
//=============================================================================

#endif //__C_CLIENTLOGIN_H__
