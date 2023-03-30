// (C)2010 S2 Games
// c_PlayerAccountStats.h
//
//=============================================================================
#ifndef __C_PLAYERACCOUNTSTATS_H__
#define __C_PLAYERACCOUNTSTATS_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_phpdata.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CPHPData;
//=============================================================================

//=============================================================================
// CPlayerAccountHistory
//=============================================================================
class CPlayerAccountHistory
{
private:
    class CLastGameInfo
    {
    private:

        ushort      m_usHeroID;
        bool        m_bWin;
        ushort      m_usKills;
        ushort      m_usDeaths;
        ushort      m_usAssists;

    public:

        CLastGameInfo() :
        m_usHeroID(0),
        m_bWin(false),
        m_usKills(0),
        m_usDeaths(0),
        m_usAssists(0)
        {}

        CLastGameInfo(const CPHPData *pMatch);

        ~CLastGameInfo() {}

        ushort  GetHeroPlayedID() const     { return m_usHeroID; }
        bool    GetWon() const              { return m_bWin; }
        ushort  GetKills() const            { return m_usKills; }
        ushort  GetDeaths() const           { return m_usDeaths; }
        ushort  GetAssists() const          { return m_usAssists; }

    };

    class CFavHeroes
    {
    private:

        ushort      m_usHeroID;
        uint        m_uiSecondsPlayed;

    public:

        CFavHeroes() :
        m_usHeroID(0),
        m_uiSecondsPlayed(0)
        {};

        ~CFavHeroes() {};

        CFavHeroes(const CPHPData *pFavHero);

        ushort  GetHeroID() const                   { return m_usHeroID; }
        uint    GetSecondsPlayedAsHero() const      { return m_uiSecondsPlayed; }
    };
    
public:
    ~CPlayerAccountHistory()    {}

    CPlayerAccountHistory()     {}
    GAME_SHARED_API CPlayerAccountHistory(const CPHPData *pData);

    vector<CLastGameInfo>           m_vLastGameInfo;
    vector<CLastGameInfo>::iterator m_itLastGameInfo;
    vector<CFavHeroes>              m_vFavHeroesInfo;
    vector<CFavHeroes>::iterator    m_itFavHeroesInfo;

    bool    IsFirstGame()                   { return m_vLastGameInfo.size() == 0; }
};
//=============================================================================

//=============================================================================
// CPlayerAccountStats
//=============================================================================
class CPlayerAccountStats
{
private:
    class CStats
    {
    private:

        uint                            m_uiRating;
        uint                            m_uiTeamCount;
        float                           m_fTeamConf;
        uint                            m_uiSoloCount;
        float                           m_fSoloConf;
        uint                            m_uiWins;
        uint                            m_uiLosses;
        uint                            m_uiDisconnects;
        uint                            m_uiKills;
        uint                            m_uiAssists;
        uint                            m_uiDeaths;
        uint                            m_uiWards;
        uint                            m_uiGamesPlayed;
        uint                            m_uiGoldTotal;
        uint                            m_uiExpTotal;
        uint                            m_uiSecondsPlayed;
        uint                            m_uiSecondsGettingExp;

    public:
        
        CStats() :
        m_uiRating(1500),
        m_uiTeamCount(0),
        m_fTeamConf(0.0f),
        m_uiSoloCount(0),
        m_fSoloConf(0.0f),
        m_uiWins(0),
        m_uiLosses(0),
        m_uiDisconnects(0),
        m_uiKills(0),
        m_uiAssists(0),
        m_uiDeaths(0),
        m_uiWards(0),
        m_uiGamesPlayed(0),
        m_uiGoldTotal(0),
        m_uiExpTotal(0),
        m_uiSecondsPlayed(0),
        m_uiSecondsGettingExp(0)
        {}
        
        ~CStats() {}

        CStats(const CPHPData *pInfosArray, byte yType);

        uint    GetRating() const                   { return m_uiRating; }
        uint    GetTeamCount() const                { return m_uiTeamCount; }
        float   GetTeamConfig() const               { return m_fTeamConf; }
        uint    GetSoloCount() const                { return m_uiSoloCount; }
        float   GetSoloConfig() const               { return m_fSoloConf; }
        uint    GetWins() const                     { return m_uiWins; }
        uint    GetLosses() const                   { return m_uiLosses; }
        uint    GetDisconnects() const              { return m_uiDisconnects; }
        uint    GetKills() const                    { return m_uiKills; }
        uint    GetAssists() const                  { return m_uiAssists; }
        uint    GetDeaths() const                   { return m_uiDeaths; }
        uint    GetWards() const                    { return m_uiWards; }
        uint    GetGamesPlayed() const              { return m_uiGamesPlayed; }
        uint    GetGoldTotal() const                { return m_uiGoldTotal; }
        uint    GetExpTotal() const                 { return m_uiExpTotal; }
        uint    GetSecondsPlayed() const            { return m_uiSecondsPlayed; }
        uint    GetSecondsEarningExp() const        { return m_uiSecondsGettingExp; }
    };

    int                             m_iAccountID;
    int                             m_iSuperID;
    uint                            m_uiAccountType;
    uint                            m_uiTrialStatus;
    uint                            m_uiTrialGamesPlayed;
    uint                            m_uiClan_id;
    tstring                         m_sTag;
    uint                            m_uiLevel;
    float                           m_fLeaverPercent;
    CStats                          m_Stats;
    CStats                          m_RankedStats;
    CStats                          m_CasualStats;
    tstring                         m_sGameCookie;

public:
    CPlayerAccountStats() :
    m_iAccountID(0),
    m_iSuperID(0),
    m_uiAccountType(0),
    m_uiTrialStatus(0),
    m_uiTrialGamesPlayed(0),
    m_uiClan_id(0),
    m_sTag(_T("")),
    m_uiLevel(0),
    m_fLeaverPercent(0.0f),
    m_sGameCookie(_T(""))
    {}

    ~CPlayerAccountStats() {};

    GAME_SHARED_API CPlayerAccountStats(const CPHPData *pData);

    int     GetAccountID() const            { return m_iAccountID; }
    int     GetSuperID() const              { return m_iSuperID; }
    uint    GetAccountType() const          { return m_uiAccountType; }
    uint    GetTrialStatus() const          { return m_uiTrialStatus; }
    uint    GetTrialGamesPlayed() const     { return m_uiTrialGamesPlayed; }
    uint    GetClanID() const               { return m_uiClan_id; }
    tstring GetClanTag() const              { return m_sTag; }
    uint    GetLevel() const                { return m_uiLevel; }
    float   GetLeaverPercent() const        { return m_fLeaverPercent; }
    CStats  GetStats() const                { return m_Stats; }
    CStats  GetRankedStats() const          { return m_RankedStats; }
    CStats  GetCasualStats() const          { return m_CasualStats; }
    tstring GetGameCookie() const           { return m_sGameCookie; }

    void    IncTrialGamesPlayed()           { ++m_uiTrialGamesPlayed; }
    
    uint    GetTotalGamesPlayed()           { return GetStats().GetGamesPlayed() + GetRankedStats().GetGamesPlayed() + GetCasualStats().GetGamesPlayed(); }
    uint    GetTotalWards()                 { return GetStats().GetWards() + GetRankedStats().GetWards() + GetCasualStats().GetWards(); }
    uint    GetTotalSecondsPlayed()         { return GetStats().GetSecondsPlayed() + GetRankedStats().GetSecondsPlayed() + GetCasualStats().GetSecondsPlayed(); }
    float   GetTotalMinsPlayed()            { return (float)GetTotalSecondsPlayed() / 60.0f; }
    uint    GetTotalSecondsEarningExp()     { return GetStats().GetSecondsEarningExp() + GetRankedStats().GetSecondsEarningExp() + GetCasualStats().GetSecondsEarningExp(); }
    float   GetTotalMinsEarningExp()        { return (float)GetTotalSecondsEarningExp() / 60.0f; }
    float   GetTotalExp()                   { return (float)GetStats().GetExpTotal() + (float)GetRankedStats().GetExpTotal() + (float)GetCasualStats().GetExpTotal(); }
    float   GetTotalGold()                  { return (float)GetStats().GetGoldTotal() + (float)GetRankedStats().GetGoldTotal() + (float)GetCasualStats().GetGoldTotal(); }

    float   GetRankedMinsPlayed()           { return (float)GetRankedStats().GetSecondsPlayed() / 60.0f; }
    float   GetRankedMinsEarningExp()       { return (float)GetRankedStats().GetSecondsEarningExp() / 60.0f; }

    float   GetPubMinsPlayed()              { return (float)GetStats().GetSecondsPlayed() / 60.0f; }
    float   GetPubMinsEarningExp()          { return (float)GetStats().GetSecondsEarningExp() / 60.0f; }

    float   GetCasMinsPlayed()              { return (float)GetCasualStats().GetSecondsPlayed() / 60.0f; }
    float   GetCasMinsEarningExp()          { return (float)GetCasualStats().GetSecondsEarningExp() / 60.0f; }
    
    float   GetTotalWardsPerGame()          { return (float)GetTotalWards() / (float)GetTotalGamesPlayed(); }
    float   GetTotalGoldPerMin()            { return GetTotalGold() / GetTotalMinsPlayed(); }   
    float   GetTotalExpPerMin()             { return GetTotalExp() / GetTotalMinsEarningExp(); }    
    float   GetTotalWinPercent()            { return (float)(GetStats().GetWins() + GetRankedStats().GetWins() + GetCasualStats().GetWins()) / (float)GetTotalGamesPlayed(); }

    float   GetRankedWardsPerGame()         { return (float)GetRankedStats().GetWards() / (float)GetRankedStats().GetGamesPlayed(); }
    float   GetRankedGoldPerMin()           { return GetRankedStats().GetGoldTotal() / GetRankedMinsPlayed(); } 
    float   GetRankedExpPerMin()            { return GetRankedStats().GetExpTotal() / GetRankedMinsEarningExp(); }  
    float   GetRankedWinPercent()           { return (float)(GetRankedStats().GetWins()) / (float)GetRankedStats().GetGamesPlayed(); }

    float   GetPubWardsPerGame()            { return (float)GetStats().GetWards() / (float)GetStats().GetGamesPlayed(); }
    float   GetPubGoldPerMin()              { return GetStats().GetGoldTotal() / GetPubMinsPlayed(); }  
    float   GetPubExpPerMin()               { return GetStats().GetExpTotal() / GetPubMinsEarningExp(); }   
    float   GetPubWinPercent()              { return (float)(GetStats().GetWins()) / (float)GetStats().GetGamesPlayed(); }

    float   GetCasualWardsPerGame()         { return (float)GetCasualStats().GetWards() / (float)GetCasualStats().GetGamesPlayed(); }
    float   GetCasualGoldPerMin()           { return GetCasualStats().GetGoldTotal() / GetCasMinsPlayed(); }    
    float   GetCasualExpPerMin()            { return GetCasualStats().GetExpTotal() / GetCasMinsEarningExp(); }
    float   GetCasualWinPercent()           { return (float)(GetCasualStats().GetWins()) / (float)GetCasualStats().GetGamesPlayed(); }

    bool    IsFirstGame()                   { return GetTotalGamesPlayed() == 0; }
};
//=============================================================================

#endif //__C_PLAYERACCOUNTSTATS_H__