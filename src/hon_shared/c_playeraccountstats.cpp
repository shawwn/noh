// (C)2010 S2 Games
// c_playeraccount\stats.h
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_playeraccountstats.h"

#include "../k2/c_phpdata.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPHPData;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EPlayerAccountStatsType
{
    PLAYER_STATS_TYPE_PUBLIC = 0,
    PLAYER_STATS_TYPE_RANKED,
    PLAYER_STATS_TYPE_CASUAL,
};
//=============================================================================

/*====================
  CPlayerAccountHistory::CLastGameInfo::CLastGameInfo
  ====================*/
CPlayerAccountHistory::CLastGameInfo::CLastGameInfo(const CPHPData *pMatch) :
m_usHeroID(0),
m_bWin(false),
m_usKills(0),
m_usDeaths(0),
m_usAssists(0)
{
    if (pMatch)
    {
        tstring tsHeroDevName = pMatch->GetString(_CTS("cli_name"), TSNULL);
        m_usHeroID = EntityRegistry.LookupID(tsHeroDevName);
        m_bWin = pMatch->GetBool(_CTS("wins"), false);
        m_usKills = pMatch->GetInteger(_CTS("herokills"), 0);
        m_usDeaths = pMatch->GetInteger(_CTS("deaths"), 0);
        m_usAssists = pMatch->GetInteger(_CTS("heroassists"), 0);
    }
}

        
/*====================
  CPlayerAccountHistory::CFavHeroes::CFavHeroes
  ====================*/
CPlayerAccountHistory::CFavHeroes::CFavHeroes(const CPHPData *pFavHero) :
m_usHeroID(0),
m_uiSecondsPlayed(0)
{
    if (pFavHero)
    {
        tstring tsHeroDevName = pFavHero->GetString(_CTS("cli_name"), TSNULL);
        m_usHeroID = EntityRegistry.LookupID(tsHeroDevName);
        m_uiSecondsPlayed = pFavHero->GetInteger(_CTS("ph_secs"), 0);
    }
}


/*====================
  CPlayerAccountHistory::CPlayerAccountHistory
  ====================*/        
CPlayerAccountHistory::CPlayerAccountHistory(const CPHPData *pData)
{
    if (pData == nullptr)
        return;
        
    const CPHPData *pMatchHistory(pData->GetVar(_CTS("match_history")));
    const CPHPData *pFavoritHeroes(pData->GetVar(_CTS("fav_heroes")));
    if (pMatchHistory != nullptr)
    {
        for(uint uiLastMatch = 0; uiLastMatch < pMatchHistory->GetSize(); ++uiLastMatch)
        {
            const CPHPData *pMatch(pMatchHistory->GetVar(uiLastMatch));
            if (pMatch == nullptr)
                break;
                
            m_vLastGameInfo.push_back(CLastGameInfo(pMatch));
        }
    }
    if (pFavoritHeroes != nullptr)
    {
        for(uint uiFavHeroes = 0; uiFavHeroes < pFavoritHeroes->GetSize(); ++uiFavHeroes)
        {
            const CPHPData *pFavHero(pFavoritHeroes->GetVar(uiFavHeroes));
            if (pFavHero == nullptr)
                break;
        
            m_vFavHeroesInfo.push_back(CFavHeroes(pFavHero));
        }
    }
}


/*====================
  CPlayerAccountStats::CStats::CStats
  ====================*/
CPlayerAccountStats::CStats::CStats(const CPHPData *pData, byte yType) :
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
{
    if (pData == nullptr)
        return;

    if (yType == PLAYER_STATS_TYPE_RANKED)
    {
        m_uiRating = pData->GetInteger(_CTS("rnk_amm_team_rating"), 1500);
        m_uiTeamCount = pData->GetInteger(_CTS("rnk_amm_team_count"), 0);
        m_fTeamConf = pData->GetFloat(_CTS("rnk_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CTS("rnk_amm_solo_count"), 0);
        m_fSoloConf = pData->GetFloat(_CTS("rnk_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CTS("rnk_wins"), 0);
        m_uiLosses = pData->GetInteger(_CTS("rnk_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CTS("rnk_discos"), 0);
        m_uiKills = pData->GetInteger(_CTS("rnk_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CTS("rnk_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CTS("rnk_deaths"), 0);
        m_uiWards = pData->GetInteger(_CTS("rnk_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CTS("rnk_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CTS("rnk_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CTS("rnk_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CTS("rnk_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CTS("rnk_time_earning_exp"), 0);
    }
    else if (yType == PLAYER_STATS_TYPE_PUBLIC)
    {
        m_uiRating = pData->GetInteger(_CTS("acc_pub_skill"), 1500);
        m_uiTeamCount = pData->GetInteger(_CTS("acc_pub_count"), 0);
        m_fTeamConf = pData->GetFloat(_CTS("rnk_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CTS("acc_pub_count"), 0);
        m_fSoloConf = pData->GetFloat(_CTS("rnk_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CTS("acc_wins"), 0);
        m_uiLosses = pData->GetInteger(_CTS("acc_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CTS("acc_discos"), 0);
        m_uiKills = pData->GetInteger(_CTS("acc_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CTS("acc_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CTS("acc_deaths"), 0);
        m_uiWards = pData->GetInteger(_CTS("acc_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CTS("acc_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CTS("acc_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CTS("acc_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CTS("acc_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CTS("acc_time_earning_exp"), 0);
    }
    else if (yType == PLAYER_STATS_TYPE_CASUAL)
    {
        m_uiRating = pData->GetInteger(_CTS("cs_amm_team_rating"), 1500);
        m_uiTeamCount = pData->GetInteger(_CTS("cs_amm_team_count"), 0);
        m_fTeamConf = pData->GetFloat(_CTS("cs_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CTS("cs_amm_solo_count"), 0);
        m_fSoloConf = pData->GetFloat(_CTS("cs_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CTS("cs_wins"), 0);
        m_uiLosses = pData->GetInteger(_CTS("cs_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CTS("cs_discos"), 0);
        m_uiKills = pData->GetInteger(_CTS("cs_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CTS("cs_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CTS("cs_deaths"), 0);
        m_uiWards = pData->GetInteger(_CTS("cs_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CTS("cs_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CTS("cs_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CTS("cs_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CTS("cs_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CTS("cs_time_earning_exp"), 0);
    }
}

/*====================
  CPlayerAccountStats::CPlayerAccountStats
  ====================*/        
CPlayerAccountStats::CPlayerAccountStats(const CPHPData *pData) :
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
{
    if (pData)
    {
        m_iAccountID = pData->GetInteger(_CTS("account_id"), 0);
        m_iSuperID = pData->GetInteger(_CTS("super_id"), 0);
        m_uiAccountType = pData->GetInteger(_CTS("account_type"), 0);
        m_uiTrialStatus = pData->GetInteger(_CTS("trial"), 0);
        m_uiTrialGamesPlayed = pData->GetInteger(_CTS("acc_trial_games_played"), 0);
        m_uiClan_id = pData->GetInteger(_CTS("clan_id"), 0);
        m_sTag = pData->GetString(_CTS("tag"), TSNULL);
        m_uiLevel = pData->GetInteger(_CTS("level"), 0);
        m_sGameCookie = pData->GetString(_CTS("game_cookie"), TSNULL);

        pData = pData->GetVar(_CTS("infos"));
        if (!pData)
            return;

        pData = pData->GetVar(0);

        if (!pData)
            return;
        
        m_Stats = CStats(pData, PLAYER_STATS_TYPE_PUBLIC);
        m_RankedStats = CStats(pData, PLAYER_STATS_TYPE_RANKED);
        m_CasualStats = CStats(pData, PLAYER_STATS_TYPE_CASUAL);
        
        /*const CPHPData *pMatchHistory(pData->GetVar(_CTS("match_history")));
        const CPHPData *pFavoritHeroes(pData->GetVar(_CTS("fav_heroes")));
        if (pMatchHistory != nullptr)
        {
            for(uint uiLastMatch = 0; uiLastMatch < pMatchHistory->GetSize(); ++uiLastMatch)
            {
                const CPHPData *pMatch(pMatchHistory->GetVar(uiLastMatch));
                if (pMatch == nullptr)
                    break;
                    
                m_vLastGameInfo.push_back(CLastGameInfo(pMatch));
            }
        }
        if (pFavoritHeroes != nullptr)
        {
            for(uint uiFavHeroes = 0; uiFavHeroes < pFavoritHeroes->GetSize(); ++uiFavHeroes)
            {
                const CPHPData *pFavHero(pFavoritHeroes->GetVar(uiFavHeroes));
                if (pFavHero == nullptr)
                    break;
            
                m_vFavHeroesInfo.push_back(CFavHeroes(pFavHero));
            }
        }*/
    }
}