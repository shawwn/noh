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
        tstring tsHeroDevName = pMatch->GetString(_CWS("cli_name"), TSNULL);
        m_usHeroID = EntityRegistry.LookupID(tsHeroDevName);
        m_bWin = pMatch->GetBool(_CWS("wins"), false);
        m_usKills = pMatch->GetInteger(_CWS("herokills"), 0);
        m_usDeaths = pMatch->GetInteger(_CWS("deaths"), 0);
        m_usAssists = pMatch->GetInteger(_CWS("heroassists"), 0);
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
        tstring tsHeroDevName = pFavHero->GetString(_CWS("cli_name"), TSNULL);
        m_usHeroID = EntityRegistry.LookupID(tsHeroDevName);
        m_uiSecondsPlayed = pFavHero->GetInteger(_CWS("ph_secs"), 0);
    }
}


/*====================
  CPlayerAccountHistory::CPlayerAccountHistory
  ====================*/        
CPlayerAccountHistory::CPlayerAccountHistory(const CPHPData *pData)
{
    if (pData == NULL)
        return;
        
    const CPHPData *pMatchHistory(pData->GetVar(_CWS("match_history")));
    const CPHPData *pFavoritHeroes(pData->GetVar(_CWS("fav_heroes")));
    if (pMatchHistory != NULL)
    {
        for(uint uiLastMatch = 0; uiLastMatch < pMatchHistory->GetSize(); ++uiLastMatch)
        {
            const CPHPData *pMatch(pMatchHistory->GetVar(uiLastMatch));
            if (pMatch == NULL)
                break;
                
            m_vLastGameInfo.push_back(CLastGameInfo(pMatch));
        }
    }
    if (pFavoritHeroes != NULL)
    {
        for(uint uiFavHeroes = 0; uiFavHeroes < pFavoritHeroes->GetSize(); ++uiFavHeroes)
        {
            const CPHPData *pFavHero(pFavoritHeroes->GetVar(uiFavHeroes));
            if (pFavHero == NULL)
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
    if (pData == NULL)
        return;

    if (yType == PLAYER_STATS_TYPE_RANKED)
    {
        m_uiRating = pData->GetInteger(_CWS("rnk_amm_team_rating"), 1500);
        m_uiTeamCount = pData->GetInteger(_CWS("rnk_amm_team_count"), 0);
        m_fTeamConf = pData->GetFloat(_CWS("rnk_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CWS("rnk_amm_solo_count"), 0);
        m_fSoloConf = pData->GetFloat(_CWS("rnk_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CWS("rnk_wins"), 0);
        m_uiLosses = pData->GetInteger(_CWS("rnk_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CWS("rnk_discos"), 0);
        m_uiKills = pData->GetInteger(_CWS("rnk_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CWS("rnk_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CWS("rnk_deaths"), 0);
        m_uiWards = pData->GetInteger(_CWS("rnk_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CWS("rnk_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CWS("rnk_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CWS("rnk_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CWS("rnk_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CWS("rnk_time_earning_exp"), 0);
    }
    else if (yType == PLAYER_STATS_TYPE_PUBLIC)
    {
        m_uiRating = pData->GetInteger(_CWS("acc_pub_skill"), 1500);
        m_uiTeamCount = pData->GetInteger(_CWS("acc_pub_count"), 0);
        m_fTeamConf = pData->GetFloat(_CWS("rnk_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CWS("acc_pub_count"), 0);
        m_fSoloConf = pData->GetFloat(_CWS("rnk_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CWS("acc_wins"), 0);
        m_uiLosses = pData->GetInteger(_CWS("acc_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CWS("acc_discos"), 0);
        m_uiKills = pData->GetInteger(_CWS("acc_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CWS("acc_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CWS("acc_deaths"), 0);
        m_uiWards = pData->GetInteger(_CWS("acc_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CWS("acc_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CWS("acc_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CWS("acc_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CWS("acc_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CWS("acc_time_earning_exp"), 0);
    }
    else if (yType == PLAYER_STATS_TYPE_CASUAL)
    {
        m_uiRating = pData->GetInteger(_CWS("cs_amm_team_rating"), 1500);
        m_uiTeamCount = pData->GetInteger(_CWS("cs_amm_team_count"), 0);
        m_fTeamConf = pData->GetFloat(_CWS("cs_amm_team_conf"), 0.0f);
        m_uiSoloCount = pData->GetInteger(_CWS("cs_amm_solo_count"), 0);
        m_fSoloConf = pData->GetFloat(_CWS("cs_amm_solo_conf"), 0.0f);
        m_uiWins = pData->GetInteger(_CWS("cs_wins"), 0);
        m_uiLosses = pData->GetInteger(_CWS("cs_losses"), 0);
        m_uiDisconnects = pData->GetInteger(_CWS("cs_discos"), 0);
        m_uiKills = pData->GetInteger(_CWS("cs_herokills"), 0);
        m_uiAssists = pData->GetInteger(_CWS("cs_heroassists"), 0);
        m_uiDeaths = pData->GetInteger(_CWS("cs_deaths"), 0);
        m_uiWards = pData->GetInteger(_CWS("cs_wards"), 0);
        m_uiGamesPlayed = pData->GetInteger(_CWS("cs_games_played"), 0);
        m_uiGoldTotal = pData->GetInteger(_CWS("cs_gold"), 0);
        m_uiExpTotal = pData->GetInteger(_CWS("cs_exp"), 0);
        m_uiSecondsPlayed = pData->GetInteger(_CWS("cs_secs"), 0);
        m_uiSecondsGettingExp = pData->GetInteger(_CWS("cs_time_earning_exp"), 0);
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
        m_iAccountID = pData->GetInteger(_CWS("account_id"), 0);
        m_iSuperID = pData->GetInteger(_CWS("super_id"), 0);
        m_uiAccountType = pData->GetInteger(_CWS("account_type"), 0);
        m_uiTrialStatus = pData->GetInteger(_CWS("trial"), 0);
        m_uiTrialGamesPlayed = pData->GetInteger(_CWS("acc_trial_games_played"), 0);
        m_uiClan_id = pData->GetInteger(_CWS("clan_id"), 0);
        m_sTag = pData->GetString(_CWS("tag"), TSNULL);
        m_uiLevel = pData->GetInteger(_CWS("level"), 0);
        m_sGameCookie = pData->GetString(_CWS("game_cookie"), TSNULL);

        pData = pData->GetVar(_CWS("infos"));
        if (!pData)
            return;

        pData = pData->GetVar(0);

        if (!pData)
            return;
        
        m_Stats = CStats(pData, PLAYER_STATS_TYPE_PUBLIC);
        m_RankedStats = CStats(pData, PLAYER_STATS_TYPE_RANKED);
        m_CasualStats = CStats(pData, PLAYER_STATS_TYPE_CASUAL);
        
        /*const CPHPData *pMatchHistory(pData->GetVar(_CWS("match_history")));
        const CPHPData *pFavoritHeroes(pData->GetVar(_CWS("fav_heroes")));
        if (pMatchHistory != NULL)
        {
            for(uint uiLastMatch = 0; uiLastMatch < pMatchHistory->GetSize(); ++uiLastMatch)
            {
                const CPHPData *pMatch(pMatchHistory->GetVar(uiLastMatch));
                if (pMatch == NULL)
                    break;
                    
                m_vLastGameInfo.push_back(CLastGameInfo(pMatch));
            }
        }
        if (pFavoritHeroes != NULL)
        {
            for(uint uiFavHeroes = 0; uiFavHeroes < pFavoritHeroes->GetSize(); ++uiFavHeroes)
            {
                const CPHPData *pFavHero(pFavoritHeroes->GetVar(uiFavHeroes));
                if (pFavHero == NULL)
                    break;
            
                m_vFavHeroesInfo.push_back(CFavHeroes(pFavHero));
            }
        }*/
    }
}