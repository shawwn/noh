// (C)2008 S2 Games
// c_player.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_player.h"

#include "c_teaminfo.h"
#include "i_buildingentity.h"
#include "i_petentity.h"
#include "i_heroentity.h"
#include "c_gameinfo.h"
#include "c_gamestats.h"
#include "c_entitychest.h"
#include "i_entityitem.h"
#include "c_replaymanager.h"
#include "c_playeraccountstats.h"

#include "../k2/c_camera.h"
#include "../k2/c_clientconnection.h"
#include "../k2/intersection.h"
#include "../k2/c_vid.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_input.h"
#include "../k2/c_hostserver.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint    CPlayer::s_uiBaseType(ENTITY_BASE_TYPE_PLAYER);

DEFINE_ENT_ALLOCATOR4(Player);

ARRAY_CVAR_STRINGF( g_playerColors,                 "#0042FF,#1CE6B9,#9000C0,#FFFC01,#FE8A0E,#E55BB0,#959697,#7EBFF1,#106246,#8B4513",  CVAR_GAMECONFIG | CVAR_TRANSMIT);
ARRAY_CVAR_STRINGF( g_playerColorNames,             "Blue,Teal,Purple,Yellow,Orange,Pink,Gray,Light Blue,Dark Green,Brown",             CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(         g_apmLogPeriod,                 SecToMs(10u),   CVAR_GAMECONFIG);

CVAR_BOOLF(         cam_edgeScroll,                 true,           CVAR_SAVECONFIG);
CVAR_FLOATF(        cam_scrollSpeed,                3000.0f,        CVAR_SAVECONFIG);
CVAR_FLOATF(        cam_distanceStep,               80.0f,          CVAR_SAVECONFIG);
CVAR_FLOATF(        cam_heightStep,                 40.0f,          CVAR_SAVECONFIG);
CVAR_BOOLF(         cam_mapConstraints,             true,           CVAR_SAVECONFIG);
CVAR_FLOATF(        cam_scrollScale,                5000.0f,        CVAR_SAVECONFIG);
CVAR_FLOATF(        cam_scrollPower,                1.3f,           CVAR_SAVECONFIG);
CVAR_FLOAT(         cam_flySpeed,                   300.0f);

CVAR_INT(           cam_mode,                       0);

CVAR_FLOATF(        g_camDistanceDefault,           1650.0f,        CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camDistanceMin,               600.0f,         CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camDistanceMax,               1650.0f,        CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camHeightMin,                 0.0f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camHeightMax,                 1000.0f,        CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camPitch,                     -56.0f,         CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camYaw,                       0.0f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camFov,                       53.75f,         CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camAspect,                    1.515f,         CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camWeightX,                   1.0f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        g_camWeightY,                   3.0f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);

CVAR_FLOATF(        psf_medianScalingRank,          1800.0f,        CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        psf_baseKFactor,                30.0f,          CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        psf_minKFactor,                 10.0f,          CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        psf_maxKFactor,                 50.0f,          CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        psf_KFactorScale,               30.0f,          CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_INTF(          psf_gammaCurveRange,            200,            CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_INTF(          psf_gammaCurveK,                18,             CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(        psf_gammaCurveTheta,            5.0f,           CVAR_GAMECONFIG | CVAR_TRANSMIT);

CVAR_BOOL(          d_allowMultipleHeroes,          false);
CVAR_BOOL(          d_assignRandomPSF,              false);

DEFINE_ENTITY_DESC(CPlayer, 10)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unSelectedHeroID"), TYPE_SHORT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiHeroIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiStatsIndex"), TYPE_GAMEINDEX, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iAccountID"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iClientNumber"), TYPE_INT, 5, -1));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unNameIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unClanNameIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unClanRankIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iKarma"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fRank"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fMatchWinValue"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fMatchLossValue"), TYPE_FLOAT, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yVote"), TYPE_CHAR, 2, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLastVoteCallTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLastVoteKickTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unFlags"), TYPE_SHORT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySwapRequests"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fLoadingProgress"), TYPE_BYTEPERCENT, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 32, TEAM_INVALID));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iTeamIndex"), TYPE_INT, 5, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yFullSharedControl"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yPartialSharedControl"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yNoHelp"), TYPE_CHAR, 8, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unGold"), TYPE_SHORT, 16, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unPing"), TYPE_SHORT, 10, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTerminationTime"), TYPE_INT, 32, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_HERO_KILLS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_DEATHS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_ASSISTS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_CREEP_KILLS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_NEUTRAL_KILLS]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_DENIES]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_STARTING_GOLD]"), TYPE_INT, 24, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_GOLD_SPENT]"), TYPE_INT, 24, 0));
        // version 8
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_PLAYER_KILL_GOLD]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_PLAYER_ASSIST_GOLD]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_BUILDING_GOLD]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_DEATH_GOLD]"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aStatTotals[PLAYER_STAT_CREEP_GOLD]"), TYPE_INT, 16, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aFloatStatTotals[PLAYER_STAT_HERO_DAMAGE]"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_aFloatStatTotals[PLAYER_STAT_BUILDING_DAMAGE]"), TYPE_FLOAT, 0, 0));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yKillStreak"), TYPE_CHAR, 8, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unInterfaceIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unOverlayInterfaceIndex"), TYPE_SHORT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCameraIndex"), TYPE_SHORT, 0, 0));
    
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountWins"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountLosses"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountDisconnects"), TYPE_INT, 16, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountKills"), TYPE_INT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountAssists"), TYPE_INT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountDeaths"), TYPE_INT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fAccountExpMin"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fAccountGoldMin"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fAccountWardsPerGame"), TYPE_FLOAT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_bitAccountRecentMatchWin"), TYPE_CHAR, 0, 0));

    for(int iWriteCount = 0; iWriteCount < 7; ++iWriteCount)
    {
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_usAccountRecentMatchKills[") + XtoA(iWriteCount) + _T("]"), TYPE_SHORT, 0, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_usAccountRecentMatchDeaths[") + XtoA(iWriteCount) + _T("]"), TYPE_SHORT, 0, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_usAccountRecentMatchAssists[") + XtoA(iWriteCount) + _T("]"), TYPE_SHORT, 0, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_usAccountRecentMatchHeroID[") + XtoA(iWriteCount) + _T("]"), TYPE_SHORT, 0, 0));
    }

    for(int iWriteCount = 0; iWriteCount < 5; ++iWriteCount)
    {
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_usFavHeroID[") + XtoA(iWriteCount) + _T("]"), TYPE_SHORT, 0, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fFavHeroPlayedPercent[") + XtoA(iWriteCount) + _T("]"), TYPE_FLOAT, 0, 0));
    }
    
    // TODO: Trim bits
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiChatSymbol"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiChatNameColor"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiAccountIcon"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unSelectedAvatarBits"), TYPE_SHORT, 16, 0));
}
//=============================================================================

/*====================
  CPlayer::~CPlayer
  ====================*/
CPlayer::~CPlayer()
{
}


/*====================
  CPlayer::CPlayer
  ====================*/
CPlayer::CPlayer() :
IGameEntity(NULL),

m_iClientNumber(-1),
m_unNameIndex(INVALID_NETWORK_STRING),
m_unClanNameIndex(INVALID_NETWORK_STRING),
m_unClanRankIndex(INVALID_NETWORK_STRING),

m_unSelectedHeroID(INVALID_ENT_TYPE),
m_unSelectedAvatarBits(INVALID_ENT_TYPE),
m_uiSelectedAvatarKey(INVALID_INDEX),
m_unPotentialHeroID(INVALID_ENT_TYPE),
m_uiHeroIndex(INVALID_INDEX),
m_uiStatsIndex(INVALID_INDEX),

m_v3Position(0.0f, 0.0f, 0.0f),
m_cMemChecker(_T(__FILE__)),
m_v3Angles(g_camPitch, 0.0f, g_camYaw),
m_fCameraDistance(g_camDistanceDefault),
m_fCameraHeight(0.0f),

m_bCameraDrag(false),
m_bCameraScroll(false),

m_ySwapRequests(0),
m_fLoadingProgress(0.0f),

m_iClanID(-1),
m_fRank(PLAYER_RANK_UNKNOWN),
m_fMatchWinValue(0.0f),
m_fMatchLossValue(0.0f),

m_uiTeamID(TEAM_INVALID),
m_iTeamIndex(-1),
m_yFullSharedControl(0),
m_yPartialSharedControl(0),
m_yNoHelp(0),
m_yVote(VOTE_NONE),
m_uiLastVoteCallTime(INVALID_TIME),
m_uiLastVoteKickTime(INVALID_TIME),
m_uiDraftRound(0),

m_unGold(0),

m_uiDisconnectTime(INVALID_TIME),
m_uiTotalDisconnectedTime(0),
m_uiTerminationTime(INVALID_TIME),
m_uiPlayTime(0),

m_uiActionCount(0),
m_uiStartActionCount(INVALID_TIME),
m_yKillStreak(0),
m_yDeathStreak(0),

m_ySmackdowns(0),
m_yHumiliations(0),

m_yMultiKill(0),
m_uiLastKillTime(INVALID_TIME),

m_uiLastMapPingTime(INVALID_TIME),
m_uiLastUnitPingTime(INVALID_TIME),
m_uiLastInteractionTime(INVALID_TIME),

m_bBlindPick(false),
m_bIsolated(false),
m_bFullVision(false),
m_bReferee(false),

m_uiChatCounter(0),
m_bAllowChat(true),

m_bHasSecretInfo(false),
m_iSecretAccountID(-1),
m_iSecretClanID(-1),
m_fSecretRank(PLAYER_RANK_UNKNOWN),

m_unInterfaceIndex(INVALID_NETWORK_STRING),
m_unOverlayInterfaceIndex(INVALID_NETWORK_STRING),
m_uiCameraIndex(INVALID_INDEX),
m_bMoveHeroToSpawnOnDisconnect(true),
m_uiNotificationFlags(0),
m_uiAccountWins(0),
m_uiAccountLosses(0),
m_uiAccountDisconnects(0),
m_uiAccountKills(0),
m_uiAccountAssists(0),
m_uiAccountDeaths(0),
m_fAccountEmPercent(0.0f),
m_fAccountExpMin(0.0f),
m_fAccountGoldMin(0.0f),
m_fAccountWardsPerGame(0.0f),
m_uiChatSymbol(INVALID_INDEX),
m_uiChatNameColor(INVALID_INDEX),
m_uiAccountIcon(INVALID_INDEX),
m_uiAnnouncerVoice(INVALID_INDEX),
m_uiTaunt(INVALID_INDEX),
m_bAccountFirstGame(false),
m_bHistoryFirstGame(false),
m_uiTotalSecondsPlayed(0)
{
    for (int i(0); i < NUM_PLAYER_STATS; ++i)
        m_aStatTotals[i] = 0;

    for (int i(0); i < NUM_PLAYER_FLOAT_STATS; ++i)
        m_aFloatStatTotals[i] = 0.0f;

    m_bitAccountRecentMatchWin = 0;

    for(int iWriteCount = 0; iWriteCount < 7; ++iWriteCount)
    {
        m_usAccountRecentMatchKills[iWriteCount] = 0;
        m_usAccountRecentMatchDeaths[iWriteCount] = 0;
        m_usAccountRecentMatchAssists[iWriteCount] = 0;
        m_usAccountRecentMatchHeroID[iWriteCount] = 0;
    }

    for(int iWriteCount = 0; iWriteCount < 5; ++iWriteCount)
    {
        m_usFavHeroID[iWriteCount] = 0;
        m_fFavHeroPlayedPercent[iWriteCount] = 0;
    }
}


/*====================
  CPlayer::Baseline
  ====================*/
void    CPlayer::Baseline()
{
    IGameEntity::Baseline();

    m_unNameIndex = INVALID_NETWORK_STRING;
    m_unClanNameIndex = INVALID_NETWORK_STRING;
    m_unClanRankIndex = INVALID_NETWORK_STRING;

    m_unSelectedHeroID = INVALID_ENT_TYPE;
    m_uiHeroIndex = INVALID_INDEX;
    m_uiStatsIndex = INVALID_INDEX;

    m_iAccountID = -1;
    m_iClientNumber = -1;
    m_iKarma = 0;
    m_fRank = PLAYER_RANK_UNKNOWN;
    m_fMatchWinValue = 0.0f;
    m_fMatchLossValue = 0.0f;
    m_sMatchComment.clear();

    m_uiAccountWins = 0;
    m_uiAccountLosses = 0;
    m_uiAccountDisconnects = 0;
    m_uiAccountKills = 0;
    m_uiAccountAssists = 0;
    m_uiAccountDeaths = 0;
    m_fAccountEmPercent = 0.0f;
    m_fAccountExpMin = 0.0f;
    m_fAccountGoldMin = 0.0f;
    m_fAccountWardsPerGame = 0.0f;

    m_yVote = VOTE_NONE;
    m_uiLastVoteCallTime = INVALID_TIME;
    m_uiLastVoteKickTime = INVALID_TIME;
    m_unFlags = 0;
    m_ySwapRequests = 0;
    m_fLoadingProgress = 0.0f;

    m_uiTeamID = TEAM_INVALID;
    m_iTeamIndex = -1;

    m_yFullSharedControl = 0;
    m_yPartialSharedControl = 0;
    m_yNoHelp = 0;

    m_unGold = 0;

    m_uiTerminationTime = INVALID_TIME;
    m_unPing = 0;

    for (int i(0); i < NUM_PLAYER_STATS; ++i)
        m_aStatTotals[i] = 0;

    for (int i(0); i < NUM_PLAYER_FLOAT_STATS; ++i)
        m_aFloatStatTotals[i] = 0.0f;

    m_yKillStreak = 0;
    m_yDeathStreak = 0;

    m_unInterfaceIndex = INVALID_NETWORK_STRING;
    m_unOverlayInterfaceIndex = INVALID_NETWORK_STRING;
    m_uiCameraIndex = INVALID_INDEX;

    m_bitAccountRecentMatchWin = 0;

    for(int iWriteCount = 0; iWriteCount < 7; ++iWriteCount)
    {
        m_usAccountRecentMatchKills[iWriteCount] = 0;
        m_usAccountRecentMatchDeaths[iWriteCount] = 0;
        m_usAccountRecentMatchAssists[iWriteCount] = 0;
        m_usAccountRecentMatchHeroID[iWriteCount] = 0;
    }

    for(int iWriteCount = 0; iWriteCount < 5; ++iWriteCount)
    {
        m_usFavHeroID[iWriteCount] = 0;
        m_fFavHeroPlayedPercent[iWriteCount] = 0;
    }
    
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    m_unSelectedAvatarBits = INVALID_ENT_TYPE;
}


/*====================
  CPlayer::GetSnapshot
  ====================*/
void    CPlayer::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    if ((uiFlags & SNAPSHOT_HIDDEN) == 0)
        snapshot.WriteField(m_unSelectedHeroID);
    else
        snapshot.WriteField(INVALID_ENT_TYPE);

    snapshot.WriteGameIndex(m_uiHeroIndex);
    snapshot.WriteGameIndex(m_uiStatsIndex);

    snapshot.WriteField(m_iAccountID);
    snapshot.WriteInteger(m_iClientNumber);
    snapshot.WriteField(m_unNameIndex);
    snapshot.WriteField(m_unClanNameIndex);
    snapshot.WriteField(m_unClanRankIndex);
    snapshot.WriteField(m_iKarma);
    snapshot.WriteField(m_fRank);
    snapshot.WriteField(m_fMatchWinValue);
    snapshot.WriteField(m_fMatchLossValue);

    snapshot.WriteField(m_yVote);
    snapshot.WriteField(m_uiLastVoteCallTime);
    snapshot.WriteField(m_uiLastVoteKickTime);
    snapshot.WriteField(m_unFlags);
    snapshot.WriteField(m_ySwapRequests);
    snapshot.WriteBytePercent(m_fLoadingProgress);
    
    snapshot.WriteInteger(m_uiTeamID);
    snapshot.WriteField(m_iTeamIndex);

    snapshot.WriteField(m_yFullSharedControl);
    snapshot.WriteField(m_yPartialSharedControl);
    snapshot.WriteField(m_yNoHelp);

    snapshot.WriteField(m_unGold);

    snapshot.WriteField(m_unPing);
    snapshot.WriteField(m_uiTerminationTime);

    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_HERO_KILLS]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_DEATHS]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_ASSISTS]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_CREEP_KILLS]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_NEUTRAL_KILLS]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_DENIES]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_STARTING_GOLD]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_GOLD_SPENT]);

    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_PLAYER_KILL_GOLD]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_PLAYER_ASSIST_GOLD]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_BUILDING_GOLD]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_DEATH_GOLD]);
    snapshot.WriteField(m_aStatTotals[PLAYER_STAT_CREEP_GOLD]);

    snapshot.WriteField(m_aFloatStatTotals[PLAYER_STAT_HERO_DAMAGE]);
    snapshot.WriteField(m_aFloatStatTotals[PLAYER_STAT_BUILDING_DAMAGE]);

    snapshot.WriteField(m_yKillStreak);

    snapshot.WriteField(m_unInterfaceIndex);
    snapshot.WriteField(m_unOverlayInterfaceIndex);
    snapshot.WriteGameIndex(m_uiCameraIndex);
    
    snapshot.WriteField(m_uiAccountWins);
    snapshot.WriteField(m_uiAccountLosses);
    snapshot.WriteField(m_uiAccountDisconnects);
    snapshot.WriteField(m_uiAccountKills);
    snapshot.WriteField(m_uiAccountAssists);
    snapshot.WriteField(m_uiAccountDeaths);
    snapshot.WriteField(m_fAccountExpMin);
    snapshot.WriteField(m_fAccountGoldMin);

    snapshot.WriteField(m_fAccountWardsPerGame);
    snapshot.WriteField(m_bitAccountRecentMatchWin);

    for(int iWriteCount = 0; iWriteCount < 7; ++iWriteCount)
    {
        snapshot.WriteField(m_usAccountRecentMatchKills[iWriteCount]);
        snapshot.WriteField(m_usAccountRecentMatchDeaths[iWriteCount]);
        snapshot.WriteField(m_usAccountRecentMatchAssists[iWriteCount]);
        snapshot.WriteField(m_usAccountRecentMatchHeroID[iWriteCount]);
    }

    for(int iWriteCount = 0; iWriteCount < 5; ++iWriteCount)
    {
        snapshot.WriteField(m_usFavHeroID[iWriteCount]);
        snapshot.WriteField(m_fFavHeroPlayedPercent[iWriteCount]);
    }
    
    snapshot.WriteField(m_uiChatSymbol);
    snapshot.WriteField(m_uiChatNameColor);
    snapshot.WriteField(m_uiAccountIcon);
    snapshot.WriteField(m_unSelectedAvatarBits);
}


/*====================
  CPlayer::ReadSnapshot
  ====================*/
bool    CPlayer::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot, 1);

        snapshot.ReadField(m_unSelectedHeroID);
        snapshot.ReadGameIndex(m_uiHeroIndex);
        snapshot.ReadGameIndex(m_uiStatsIndex);

        snapshot.ReadField(m_iAccountID);
        snapshot.ReadInteger(m_iClientNumber);
        snapshot.ReadField(m_unNameIndex);
        snapshot.ReadField(m_unClanNameIndex);
        snapshot.ReadField(m_unClanRankIndex);
        snapshot.ReadField(m_iKarma);

        if (uiVersion >= 3)
        {
            snapshot.ReadField(m_fRank);
            snapshot.ReadField(m_fMatchWinValue);
            snapshot.ReadField(m_fMatchLossValue);
        }
        else
        {
            int iRank(m_fRank);
            int iMatchWinValue(m_fMatchWinValue);
            int iMatchLossValue(m_fMatchLossValue);
            snapshot.ReadField(iRank);
            snapshot.ReadField(iMatchWinValue);
            snapshot.ReadField(iMatchLossValue);
            m_fRank = iRank;
            m_fMatchWinValue = iMatchWinValue;
            m_fMatchLossValue = iMatchLossValue;
        }

        snapshot.ReadField(m_yVote);
        snapshot.ReadField(m_uiLastVoteCallTime);

        if (uiVersion >= 10)
            snapshot.ReadField(m_uiLastVoteKickTime);

        snapshot.ReadField(m_unFlags);
        snapshot.ReadField(m_ySwapRequests);
        snapshot.ReadBytePercent(m_fLoadingProgress);

        snapshot.ReadInteger(m_uiTeamID);
        snapshot.ReadField(m_iTeamIndex);

        snapshot.ReadField(m_yFullSharedControl);
        snapshot.ReadField(m_yPartialSharedControl);
        if (uiVersion >= 4)
            snapshot.ReadField(m_yNoHelp);

        snapshot.ReadField(m_unGold);

        snapshot.ReadField(m_unPing);
        snapshot.ReadField(m_uiTerminationTime);

        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_HERO_KILLS]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_DEATHS]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_ASSISTS]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_CREEP_KILLS]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_NEUTRAL_KILLS]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_DENIES]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_STARTING_GOLD]);
        snapshot.ReadField(m_aStatTotals[PLAYER_STAT_GOLD_SPENT]);

        if (uiVersion >= 8)
        {
            snapshot.ReadField(m_aStatTotals[PLAYER_STAT_PLAYER_KILL_GOLD]);
            snapshot.ReadField(m_aStatTotals[PLAYER_STAT_PLAYER_ASSIST_GOLD]);
            snapshot.ReadField(m_aStatTotals[PLAYER_STAT_BUILDING_GOLD]);
            snapshot.ReadField(m_aStatTotals[PLAYER_STAT_DEATH_GOLD]);
            snapshot.ReadField(m_aStatTotals[PLAYER_STAT_CREEP_GOLD]);
        }

        snapshot.ReadField(m_aFloatStatTotals[PLAYER_STAT_HERO_DAMAGE]);
        snapshot.ReadField(m_aFloatStatTotals[PLAYER_STAT_BUILDING_DAMAGE]);

        snapshot.ReadField(m_yKillStreak);

        if (uiVersion >= 2)
        {
            snapshot.ReadField(m_unInterfaceIndex);
            snapshot.ReadField(m_unOverlayInterfaceIndex);
            snapshot.ReadGameIndex(m_uiCameraIndex);
        }
        
        if (uiVersion >= 5)
        {
            snapshot.ReadField(m_uiAccountWins);
            snapshot.ReadField(m_uiAccountLosses);
            snapshot.ReadField(m_uiAccountDisconnects);
            snapshot.ReadField(m_uiAccountKills);
            snapshot.ReadField(m_uiAccountAssists);
            snapshot.ReadField(m_uiAccountDeaths);

            if (uiVersion <= 8)
                snapshot.ReadField(m_fAccountEmPercent);

            snapshot.ReadField(m_fAccountExpMin);
            snapshot.ReadField(m_fAccountGoldMin);
            if (uiVersion >= 6)
            {
                snapshot.ReadField(m_fAccountWardsPerGame);
                snapshot.ReadField(m_bitAccountRecentMatchWin);

                for(int iReadCount = 0; iReadCount < 7; ++iReadCount)
                {
                    snapshot.ReadField(m_usAccountRecentMatchKills[iReadCount]);
                    snapshot.ReadField(m_usAccountRecentMatchDeaths[iReadCount]);
                    snapshot.ReadField(m_usAccountRecentMatchAssists[iReadCount]);
                    snapshot.ReadField(m_usAccountRecentMatchHeroID[iReadCount]);
                }

                for(int iReadCount = 0; iReadCount < 5; ++iReadCount)
                {
                    snapshot.ReadField(m_usFavHeroID[iReadCount]);
                    snapshot.ReadField(m_fFavHeroPlayedPercent[iReadCount]);
                }
            }
        }
        
        if (uiVersion >= 7)
        {
            snapshot.ReadField(m_uiChatSymbol);
            snapshot.ReadField(m_uiChatNameColor);
            snapshot.ReadField(m_uiAccountIcon);
            snapshot.ReadField(m_unSelectedAvatarBits);
        }
        else
        {
            m_uiChatSymbol = INVALID_INDEX;
            m_uiChatNameColor = INVALID_INDEX;
            m_uiAccountIcon = INVALID_INDEX;
            m_unSelectedAvatarBits = 0;
        }
        
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CPlayer::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CPlayer::SetAccountStats
  ====================*/
void    CPlayer::SetAccountStats(CPlayerAccountStats *pAccStats)
{ 
    if (!pAccStats)
        return; 

    if (Game.IsClient())
        return;

    if (Host.GetServer()->IsArrangedMatch() && !Game.HasGameOptions(GAME_OPTION_CASUAL))
    {
        m_fRank = PLAYER_RANK_UNKNOWN;
        m_fSecretRank = pAccStats->GetRankedStats().GetRating();
        m_uiAccountWins = pAccStats->GetRankedStats().GetWins();
        m_uiAccountLosses = pAccStats->GetRankedStats().GetLosses();
        m_uiAccountDisconnects = pAccStats->GetRankedStats().GetDisconnects();
        m_uiAccountKills = pAccStats->GetRankedStats().GetKills();
        m_uiAccountAssists = pAccStats->GetRankedStats().GetAssists();
        m_uiAccountDeaths = pAccStats->GetRankedStats().GetDeaths();
        m_fAccountEmPercent = 0.0f;
        m_fAccountExpMin = pAccStats->GetRankedExpPerMin();
        m_fAccountGoldMin = pAccStats->GetRankedGoldPerMin();
        m_fAccountWardsPerGame = pAccStats->GetRankedWardsPerGame();
    }
    else if (Host.GetServer()->IsArrangedMatch() && Game.HasGameOptions(GAME_OPTION_CASUAL))
    {
        m_fRank = PLAYER_RANK_UNKNOWN;
        m_fSecretRank = pAccStats->GetCasualStats().GetRating();
        m_uiAccountWins = pAccStats->GetCasualStats().GetWins();
        m_uiAccountLosses = pAccStats->GetCasualStats().GetLosses();
        m_uiAccountDisconnects = pAccStats->GetCasualStats().GetDisconnects();
        m_uiAccountKills = pAccStats->GetCasualStats().GetKills();
        m_uiAccountAssists = pAccStats->GetCasualStats().GetAssists();
        m_uiAccountDeaths = pAccStats->GetCasualStats().GetDeaths();
        m_fAccountEmPercent = 0.0f;
        m_fAccountExpMin = pAccStats->GetCasualExpPerMin();
        m_fAccountGoldMin = pAccStats->GetCasualGoldPerMin();
        m_fAccountWardsPerGame = pAccStats->GetCasualWardsPerGame();
    }
    else
    {
        m_fRank = pAccStats->GetStats().GetRating();
        m_fSecretRank = pAccStats->GetStats().GetRating();
        m_uiAccountWins = pAccStats->GetStats().GetWins();
        m_uiAccountLosses = pAccStats->GetStats().GetLosses();
        m_uiAccountDisconnects = pAccStats->GetStats().GetDisconnects();
        m_uiAccountKills = pAccStats->GetStats().GetKills();
        m_uiAccountAssists = pAccStats->GetStats().GetAssists();
        m_uiAccountDeaths = pAccStats->GetStats().GetDeaths();
        m_fAccountEmPercent = 0.0f;
        m_fAccountExpMin = pAccStats->GetPubExpPerMin();
        m_fAccountGoldMin = pAccStats->GetPubGoldPerMin();
        m_fAccountWardsPerGame = pAccStats->GetPubWardsPerGame();
    }

    m_bAccountFirstGame = pAccStats->IsFirstGame();

    if (m_bAccountFirstGame && m_bHistoryFirstGame)
        m_bitAccountRecentMatchWin |= 0x80;
    else
        m_bitAccountRecentMatchWin &= ~0x80;

    m_uiTotalSecondsPlayed = pAccStats->GetTotalSecondsPlayed();
}


/*====================
  CPlayer::SetAccountHistory

  Only call after calling SetAccountStats
  ====================*/
void    CPlayer::SetAccountHistory(CPlayerAccountHistory *pAccHistory)
{ 
    if (!pAccHistory)
        return; 

    if (Game.IsClient())
        return;

    m_bitAccountRecentMatchWin = 0;

    pAccHistory->m_itFavHeroesInfo = pAccHistory->m_vFavHeroesInfo.begin();
    pAccHistory->m_itLastGameInfo = pAccHistory->m_vLastGameInfo.begin();

    for(int iReadCount = 0; iReadCount < 7; ++iReadCount)
    {
        if (pAccHistory->m_itLastGameInfo != pAccHistory->m_vLastGameInfo.end())
        {
            if (pAccHistory->m_itLastGameInfo->GetWon())
            {
                char iBit(1);
                if (iReadCount != 0)
                    iBit = BIT(iReadCount);
                
                m_bitAccountRecentMatchWin ^= iBit;
            }
            m_usAccountRecentMatchKills[iReadCount] = pAccHistory->m_itLastGameInfo->GetKills();
            m_usAccountRecentMatchDeaths[iReadCount] = pAccHistory->m_itLastGameInfo->GetDeaths();
            m_usAccountRecentMatchAssists[iReadCount] = pAccHistory->m_itLastGameInfo->GetAssists();
            m_usAccountRecentMatchHeroID[iReadCount] = pAccHistory->m_itLastGameInfo->GetHeroPlayedID();
            ++pAccHistory->m_itLastGameInfo;
        }
        else
        {
            m_usAccountRecentMatchKills[iReadCount] = 0;
            m_usAccountRecentMatchDeaths[iReadCount] = 0;
            m_usAccountRecentMatchAssists[iReadCount] = 0;
            m_usAccountRecentMatchHeroID[iReadCount] = 0;
        }
    }

    for (int iReadCount(0); iReadCount < 5; ++iReadCount)
    {
        if (pAccHistory->m_itFavHeroesInfo != pAccHistory->m_vFavHeroesInfo.end())
        {
            m_usFavHeroID[iReadCount] = pAccHistory->m_itFavHeroesInfo->GetHeroID();
            m_fFavHeroPlayedPercent[iReadCount] = (float)pAccHistory->m_itFavHeroesInfo->GetSecondsPlayedAsHero() / (float)m_uiTotalSecondsPlayed;
            ++pAccHistory->m_itFavHeroesInfo;
        }
        else
        {
            m_usFavHeroID[iReadCount] = 0;
            m_fFavHeroPlayedPercent[iReadCount] = 0.0f;
        }
    }

    m_bHistoryFirstGame = pAccHistory->IsFirstGame();

    if (m_bAccountFirstGame && m_bHistoryFirstGame)
        m_bitAccountRecentMatchWin |= 0x80;
    else
        m_bitAccountRecentMatchWin &= ~0x80;
}


/*====================
  CPlayer::Initialize
  ====================*/
void    CPlayer::Initialize(CClientConnection *pClientConnection, CHostServer *pHostServer)
{
    if (pClientConnection == NULL || pHostServer == NULL)
        return;

    m_iClientNumber = pClientConnection->GetClientNum();

    m_unNameIndex = NetworkResourceManager.ReserveString();
    m_unClanNameIndex = NetworkResourceManager.ReserveString();
    m_unClanRankIndex = NetworkResourceManager.ReserveString();

    if (pHostServer->IsArrangedMatch())
    {
        m_bHasSecretInfo = true;
        m_iSecretAccountID = pClientConnection->GetAccountID();
        m_iSecretClanID = pClientConnection->GetClanID();

        m_sSecretName = pClientConnection->GetName();
        m_sSecretClanName = TSNULL;

        NetworkResourceManager.SetString(m_unNameIndex, _T("?????"));
        NetworkResourceManager.SetString(m_unClanNameIndex, _T("?????"));
        NetworkResourceManager.SetString(m_unClanRankIndex, _T("?????"));
    }
    else
    {
        m_iAccountID = pClientConnection->GetAccountID();
        m_iClanID = pClientConnection->GetClanID();

        NetworkResourceManager.SetString(m_unNameIndex, pClientConnection->GetName());
        NetworkResourceManager.SetString(m_unClanNameIndex, TSNULL);
        NetworkResourceManager.SetString(m_unClanRankIndex, TSNULL);
    }

    m_sAddress = pClientConnection->GetAddress();
    m_uiHeroIndex = INVALID_INDEX;
    m_uiActionCount = 0;
    m_uiStartActionCount = INVALID_TIME;
    m_unGold = 0;
    m_yVote = VOTE_NONE;
    m_uiLastVoteCallTime = INVALID_TIME;
    m_uiLastVoteKickTime = INVALID_TIME;
    m_uiLastInteractionTime = Game.GetGameTime();

    m_yFullSharedControl = 0;
    m_yPartialSharedControl = 0;

    m_sMatchComment.clear();

    ResetCamera();

    m_uiLastInputTime = Game.GetGameTime();
    m_GoldReport.ResetReportTimer();

    m_unFlags = 0;
    m_ySwapRequests = 0;

    m_yKillStreak = 0;
    m_yDeathStreak = 0;

    m_uiChatCounter = 0;

    if (pClientConnection->HasFlags(CLIENT_CONNECTION_GAME_HOST))
        SetFlags(PLAYER_FLAG_HOST);
    if (pClientConnection->HasFlags(CLIENT_CONNECTION_LOCAL))
        SetFlags(PLAYER_FLAG_LOCAL);
    if (pClientConnection->HasFlags(CLIENT_CONNECTION_STAFF))
        SetFlags(PLAYER_FLAG_STAFF);
    if (pClientConnection->HasFlags(CLIENT_CONNECTION_PREMIUM))
        SetFlags(PLAYER_FLAG_PREMIUM);

    m_uiCameraIndex = INVALID_INDEX;
    
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    
    m_mapKilledBy.clear();

    UpdateUpgrades(pClientConnection);

    m_unSelectedHeroID = INVALID_ENT_TYPE;
    m_unSelectedAvatarBits = INVALID_ENT_TYPE;
    m_uiSelectedAvatarKey = INVALID_INDEX;

    m_bAccountFirstGame = false;
    m_bHistoryFirstGame = false;
}


/*====================
  CPlayer::FakeInitialize
  ====================*/
void    CPlayer::FakeInitialize(CHostServer *pHostServer, int iClientNum, const tstring &sName)
{
    if (pHostServer == NULL)
        return;

    m_iClientNumber = iClientNum;

    m_unNameIndex = NetworkResourceManager.ReserveString();
    m_unClanNameIndex = NetworkResourceManager.ReserveString();
    m_unClanRankIndex = NetworkResourceManager.ReserveString();

    m_iAccountID = -1;
    m_iClanID = -1;

    NetworkResourceManager.SetString(m_unNameIndex, sName);
    NetworkResourceManager.SetString(m_unClanNameIndex, TSNULL);
    NetworkResourceManager.SetString(m_unClanRankIndex, TSNULL);

    m_sAddress = TSNULL;
    m_uiHeroIndex = INVALID_INDEX;
    m_uiActionCount = 0;
    m_uiStartActionCount = INVALID_TIME;
    m_unGold = 0;
    m_yVote = VOTE_NONE;
    m_uiLastVoteCallTime = INVALID_TIME;
    m_uiLastVoteKickTime = INVALID_TIME;
    m_uiLastInteractionTime = Game.GetGameTime();

    m_yFullSharedControl = 0;
    m_yPartialSharedControl = 0;

    m_sMatchComment.clear();

    ResetCamera();

    m_uiLastInputTime = Game.GetGameTime();
    m_GoldReport.ResetReportTimer();

    m_unFlags = 0;
    m_ySwapRequests = 0;

    m_yKillStreak = 0;
    m_yDeathStreak = 0;

    m_uiChatCounter = 0;

    m_uiCameraIndex = INVALID_INDEX;
    
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    
    m_mapKilledBy.clear();

    m_setAvailableUpgrades.clear();
    m_mapSelectedUpgrades.clear();

    UpdateUpgrades();
}


/*====================
  CPlayer::RevealSecretInfo
  ====================*/
void    CPlayer::RevealSecretInfo()
{
    if (!m_bHasSecretInfo)
        return;

    m_iAccountID = m_iSecretAccountID;
    m_iClanID = m_iSecretClanID;
    m_fRank = m_fSecretRank;

    NetworkResourceManager.SetString(m_unNameIndex, m_sSecretName);
    NetworkResourceManager.SetString(m_unClanNameIndex, m_sSecretClanName);
}


/*====================
  CPlayer::ServerFrameMovement
  ====================*/
bool    CPlayer::ServerFrameMovement()
{
    if (HasFlags(PLAYER_FLAG_TERMINATED))
        return true;

    if (Game.GetGamePhase() < GAME_PHASE_ACTIVE || HasFlags(PLAYER_FLAG_DISCONNECTED))
        SetLastInteractionTime(Game.GetGameTime());

    if (m_GoldReport.IsTimeForReport())
    {
        m_GoldReport.ResetReportTimer();
        m_aStatTotals[PLAYER_STAT_PLAYER_KILL_GOLD] = m_GoldReport.GetPlayerGoldEarned();
        m_aStatTotals[PLAYER_STAT_PLAYER_ASSIST_GOLD] = m_GoldReport.GetPlayerAssistGoldEarned();
        m_aStatTotals[PLAYER_STAT_BUILDING_GOLD] = m_GoldReport.GetBuildingGoldEarned();
        m_aStatTotals[PLAYER_STAT_DEATH_GOLD] = m_GoldReport.GetDeathGoldLost();
        m_aStatTotals[PLAYER_STAT_CREEP_GOLD] = m_GoldReport.GetCreepGoldEarned();
        m_aStatTotals[PLAYER_STAT_PASSIVE_GOLD] = m_GoldReport.GetPassiveGoldEarned();
    }

    return true;
}


/*====================
  CPlayer::ServerFrameCleanup
  ====================*/
static bool PetListSort(uint a, uint b)
{
    if (a == b)
        return false;

    if (b == INVALID_INDEX)
        return true;
    if (a == INVALID_INDEX)
        return false;

    IUnitEntity *pA(Game.GetUnitFromUniqueID(a));
    IUnitEntity *pB(Game.GetUnitFromUniqueID(b));
    if (pA == pB)
        return false;

    if (pB == NULL)
        return true;
    if (pA == NULL)
        return false;

    if (pA->GetStatus() == ENTITY_STATUS_ACTIVE && pB->GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;
    if (pA->GetStatus() != ENTITY_STATUS_ACTIVE && pB->GetStatus() == ENTITY_STATUS_ACTIVE)
        return false;

    if (pA->GetSpawnTime() > pB->GetSpawnTime())
        return true;
    if (pA->GetSpawnTime() < pB->GetSpawnTime())
        return false;

    return a > b;
}

bool    CPlayer::ServerFrameCleanup()
{
    // Log APM
    if (m_uiStartActionCount == INVALID_TIME)
    {
        ResetActionCount();
    }
    else if (GetActionCountPeriod() >= g_apmLogPeriod)
    {
        Game.LogPlayer(GAME_LOG_PLAYER_ACTIONS, this);
        ResetActionCount();
    }

    // Clean up pet list
    for (uivector_it it(m_vPetUIDs.begin()); it != m_vPetUIDs.end(); ++it)
    {
        uint uiGameIndex(Game.GetGameIndexFromUniqueID(*it));
        if (Game.GetUnitEntity(uiGameIndex) == NULL)
            *it = INVALID_INDEX;
    }

    sort(m_vPetUIDs.begin(), m_vPetUIDs.end(), PetListSort);

    for (uint ui(0); ui < m_vPetUIDs.size(); ++ui)
    {
        if (m_vPetUIDs[ui] == INVALID_INDEX)
        {
            m_vPetUIDs.resize(ui);
            break;
        }
    }

    return true;
}


/*====================
  CPlayer::ShareFullControl
  ====================*/
void    CPlayer::ShareFullControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return;

    m_yFullSharedControl |= (1 << uiTeamIndex);
    m_yPartialSharedControl |= (1 << uiTeamIndex);
}


/*====================
  CPlayer::SharePartialControl
  ====================*/
void    CPlayer::SharePartialControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return;

    m_yPartialSharedControl |= (1 << uiTeamIndex);
}


/*====================
  CPlayer::UnshareFullControl
  ====================*/
void    CPlayer::UnshareFullControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return;

    m_yFullSharedControl &= ~(1 << uiTeamIndex);
}


/*====================
  CPlayer::UnsharePartialControl
  ====================*/
void    CPlayer::UnsharePartialControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return;

    m_yPartialSharedControl &= ~(1 << uiTeamIndex);
    m_yFullSharedControl &= ~(1 << uiTeamIndex);
}


/*====================
  CPlayer::HasSharedFullControl
  ====================*/
bool    CPlayer::HasSharedFullControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return false;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return false;

    return (m_yFullSharedControl & (1 << uiTeamIndex)) != 0;
}


/*====================
  CPlayer::HasSharedPartialControl
  ====================*/
bool    CPlayer::HasSharedPartialControl(int iClientNumber)
{
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return false;

    uint uiTeamIndex(pTeam->GetTeamIndexFromClientID(iClientNumber));
    if (uiTeamIndex == -1)
        return false;

    return (m_yPartialSharedControl & (1 << uiTeamIndex)) != 0;
}


/*====================
  CPlayer::SetNoHelp
  ====================*/
void    CPlayer::SetNoHelp(CPlayer *pPlayer, bool bEnable)
{
    if (pPlayer == NULL || pPlayer->GetTeam() != GetTeam())
        return;

    if (bEnable)
        m_yNoHelp |= BIT(pPlayer->GetTeamIndex());
    else
        m_yNoHelp &= ~BIT(pPlayer->GetTeamIndex());
}


/*====================
  CPlayer::GetNoHelp
  ====================*/
bool    CPlayer::GetNoHelp(CPlayer *pPlayer)
{
    if (pPlayer == NULL)
        return false;

    if (pPlayer->GetTeam() != GetTeam())
        return false;

    if (BIT(pPlayer->GetTeamIndex()) & m_yNoHelp)
        return true;

    return false;
}


/*====================
  CPlayer::ClearAffiliations
  ====================*/
void    CPlayer::ClearAffiliations()
{
    bool bWasDisconnected(IsDisconnected());

    m_yVote = VOTE_NONE;
    if (bWasDisconnected)
        SetFlags(PLAYER_FLAG_DISCONNECTED);
    else
        RemoveFlags(PLAYER_FLAG_DISCONNECTED);

    SetTeam(TEAM_INVALID);
    m_uiLastVoteCallTime = INVALID_TIME;
    m_uiLastVoteKickTime = INVALID_TIME;
    m_ySwapRequests = 0;
    m_uiDraftRound = 0;
}


/*====================
  CPlayer::CanRepick
  ====================*/
bool    CPlayer::CanRepick() const
{
    if (HasSpawnedHero())
        return false;

    if (HasFlags(PLAYER_FLAG_HAS_REPICKED) || HasFlags(PLAYER_FLAG_READY))
        return false;
    
    if (!HasFlags(PLAYER_FLAG_CAN_PICK))
        return false;

    if (Game.GetGamePhase() != GAME_PHASE_HERO_SELECT)
        return false;

    if (Game.HasGameOptions(GAME_OPTION_NO_REPICK))
        return false;

    if (!HasSelectedHero())
        return false;

    return true;
}


/*====================
  CPlayer::CanSwap
  ====================*/
bool    CPlayer::CanSwap() const
{
    if (HasSpawnedHero())
        return false;

    if (HasFlags(PLAYER_FLAG_HAS_REPICKED) || HasFlags(PLAYER_FLAG_READY))
        return false;
    
    if (Game.GetGamePhase() != GAME_PHASE_HERO_SELECT)
        return false;

    if (Game.HasGameOptions(GAME_OPTION_NO_SWAP))
        return false;

    if (!HasSelectedHero())
        return false;

    return true;
}

bool    CPlayer::CanSwap(int iTeamMateIndex) const
{
    if (!CanSwap())
        return false;

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return false;

    CPlayer *pPlayer(Game.GetPlayer(pTeam->GetClientIDFromTeamIndex(iTeamMateIndex)));
    if (pPlayer == NULL)
        return false;

    if (!pPlayer->CanSwap())
        return false;

    if (pPlayer->HasSwapRequest(pTeam->GetTeamIndexFromClientID(GetClientNumber())))
        return true;

    return false;
}


/*====================
  CPlayer::IsCurrentPicker
  ====================*/
bool    CPlayer::IsCurrentPicker() const
{
    if (!HasFlags(PLAYER_FLAG_CAN_PICK))
        return false;

    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo != NULL && !pGameInfo->GetAlternatePicks())
        return false;
    if (Game.HasFlags(GAME_FLAG_FINAL_HERO_SELECT))
        return false;

    return true;
}


/*====================
  CPlayer::IsReferee
  ====================*/
bool    CPlayer::IsReferee() const
{
    if (m_uiTeamID == TEAM_SPECTATOR && m_bReferee)
        return true;

    return false;
}


/*====================
  CPlayer::SetTeam
  ====================*/
void    CPlayer::SetTeam(uint uiTeamID)
{
    if (IsReferee() && uiTeamID != TEAM_SPECTATOR)
        m_bReferee = false;

    m_uiTeamID = uiTeamID;
}


/*====================
  CPlayer::AddPet
  ====================*/
void    CPlayer::AddPet(IUnitEntity *pPet, uint uiSummonMax, uint uiControllerUID)
{
    if (pPet == NULL)
        return;

    if (uiSummonMax > 0)
    {
        // Store both unit entities and interator so we only have to traverse the list once
        list<pair<IUnitEntity*, uivector_it> > listOldest;

        for (uivector_it it(m_vPetUIDs.begin()); it != m_vPetUIDs.end(); ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitFromUniqueID(*it));
            if (pUnit == NULL)
            {
                *it = INVALID_INDEX;
                continue;
            }

            if (uiControllerUID != INVALID_INDEX)
            {
                if (pUnit->GetControllerUID() != uiControllerUID)
                    continue;
            }
            else
            {
                if (pUnit->GetType() != pPet->GetType() || pUnit->GetStatus() != ENTITY_STATUS_ACTIVE)
                    continue;
            }

            list<pair<IUnitEntity*, uivector_it> >::iterator listit = listOldest.begin();

            while (listit != listOldest.end() && listit->first->GetSpawnTime() < pUnit->GetSpawnTime())
                listit++;
            
            listOldest.insert(listit, pair<IUnitEntity*, uivector_it>(pUnit, it));
        }

        while (listOldest.size() >= uiSummonMax)
        {
            listOldest.front().first->Kill();
            *(listOldest.front().second) = INVALID_INDEX;
            listOldest.pop_front();
        }
    }

    pPet->SetOwnerClientNumber(GetClientNumber());
    pPet->SetControllerUID(uiControllerUID);

    m_vPetUIDs.push_back(pPet->GetUniqueID());

    CBufferFixed<1 + 1 + 4> buffer;
    const byte yVersion(1); // If we ever want to change the structure of GAME_CMD_PET_ADDED, we could increment this version
    buffer << GAME_CMD_PET_ADDED << yVersion << pPet->GetIndex();
    Game.SendGameData(m_iClientNumber, buffer, false);
}


/*====================
  CPlayer::GetPersistentPet
  ====================*/
IUnitEntity*    CPlayer::GetPersistentPet(ushort unTypeID)
{
    for (uivector_it it(m_vPetUIDs.begin()); it != m_vPetUIDs.end(); ++it)
    {
        IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
        if (pEntity == NULL)
            continue;
        IPetEntity *pPet(pEntity->GetAsPet());
        if (pPet == NULL)
            continue;
        if (!pPet->GetIsPersistent())
            continue;
        if (pPet->GetType() == unTypeID)
            return pPet;
    }

    return NULL;
}


/*====================
  CPlayer::RecallPets
  ====================*/
void    CPlayer::RecallPets(IUnitEntity *pCaller, ushort unPetType)
{
    if (pCaller == NULL)
        return;

    for (uivector_it it(m_vPetUIDs.begin()); it != m_vPetUIDs.end(); ++it)
    {
        IUnitEntity *pPet(Game.GetUnitFromUniqueID(*it));
        if (pPet == NULL)
            continue;
        if (pPet->GetType() != unPetType)
            continue;

        SUnitCommand cmd;
        cmd.eCommandID = UNITCMD_TOUCH;
        cmd.uiIndex = pCaller->GetIndex();
        pPet->PlayerCommand(cmd);
    }
}


/*====================
  CPlayer::LevelPets
  ====================*/
void    CPlayer::LevelPets(IUnitEntity *pCaller, ushort unPetType, uint uiLevel)
{
    if (pCaller == NULL)
        return;

    for (uivector_it it(m_vPetUIDs.begin()); it != m_vPetUIDs.end(); ++it)
    {
        IUnitEntity *pPet(Game.GetUnitFromUniqueID(*it));
        if (pPet == NULL)
            continue;
        if (pPet->GetType() != unPetType)
            continue;

        pPet->SetLevel(uiLevel);
    }
}


/*====================
  CPlayer::RewardKill
  ====================*/
void    CPlayer::RewardKill()
{
    ++m_yKillStreak;
    m_yDeathStreak = 0;

    if (m_yKillStreak > 2)
    {
        if (m_uiAnnouncerVoice != INVALID_INDEX)
        {   
            CBufferFixed<7> buffer;
            buffer << GAME_CMD_KILLSTREAK_MESSAGE2 << m_iClientNumber << m_yKillStreak << byte(m_uiAnnouncerVoice);
            Game.BroadcastGameData(buffer, true);
            Game.LogAward(GAME_LOG_AWARD_KILL_STREAK, GetHero(), NULL);
        }
        else
        {
            CBufferFixed<6> buffer;
            buffer << GAME_CMD_KILLSTREAK_MESSAGE << m_iClientNumber << m_yKillStreak;
            Game.BroadcastGameData(buffer, true);
            Game.LogAward(GAME_LOG_AWARD_KILL_STREAK, GetHero(), NULL);     
        }
    }

    if (m_uiLastKillTime + g_multiKillTime > Game.GetGameTime())
    {
        ++m_yMultiKill;

        if (m_uiAnnouncerVoice != INVALID_INDEX)
        {   
            CBufferFixed<7> buffer;
            buffer << GAME_CMD_MULTIKILL_MESSAGE2 << m_iClientNumber << m_yMultiKill << byte(m_uiAnnouncerVoice);
            Game.BroadcastGameData(buffer, true, -1, 1000);
        }
        else
        {
            CBufferFixed<6> buffer;
            buffer << GAME_CMD_MULTIKILL_MESSAGE << m_iClientNumber << m_yMultiKill;
            Game.BroadcastGameData(buffer, true, -1, 1000);
        }

        Game.LogAward(GAME_LOG_AWARD_MULTI_KILL, GetHero(), NULL);
    }
    else
        m_yMultiKill = 1;

    m_uiLastKillTime = Game.GetGameTime();

    CTeamInfo *pTeam(Game.GetTeam(m_uiTeamID));
    if (pTeam == NULL)
        return;

    pTeam->RewardKill();
}


/*====================
  CPlayer::ResetKillStreak
  ====================*/
void    CPlayer::ResetKillStreak()
{
    m_yKillStreak = 0;

    CTeamInfo *pTeam(Game.GetTeam(m_uiTeamID));
    if (pTeam == NULL)
        return;

    pTeam->ResetKillStreak();
}


/*====================
  CPlayer::IsRival
  ====================*/
bool    CPlayer::IsRival(const int iClientNumber)
{
    imapy_it it(m_mapKilledBy.find(iClientNumber));

    // See if this player has been killed by this attacking player before
    if (it == m_mapKilledBy.end())  
        return false;
        
    if (it->second < 4)
        return false;
    else
        return true;
}


/*====================
  CPlayer::ResetKilledBy
  ====================*/
void    CPlayer::ResetKilledBy(const int iClientNumber)
{
    imapy_it it(m_mapKilledBy.find(iClientNumber));

    // See if this player has been killed by this attacking player before
    if (it == m_mapKilledBy.end())
    {
        m_mapKilledBy.insert(pair<int, byte>(iClientNumber, 0));    
        it = m_mapKilledBy.find(iClientNumber);
    }
    
    // If they have been killed 4 times or more by them, then send the Payback message to everybody in game, but display a the effect/sound only for the killer/victim
    if (it->second > 3)
    {
        CPlayer *pVictim(Game.GetPlayerFromClientNumber(iClientNumber));
        
        if (pVictim != NULL)
        {       
            CBufferFixed<11> buffer;
            buffer << GAME_CMD_PAYBACK_MESSAGE << m_iClientNumber << pVictim->GetClientNumber() << it->second << byte(GetAnnouncerVoice());
            Game.BroadcastGameData(buffer, true, -1, 3000);
            AddPaybackCount();
            Game.LogAward(GAME_LOG_AWARD_PAYBACK, GetHero()->GetAsUnit(), pVictim->GetHero()->GetAsUnit());
        }
    }   
    
    // Reset the number of times they were killed by this client to 0
    it->second = 0; 
}


/*====================
  CPlayer::UpdateKilledBy
  ====================*/
void    CPlayer::UpdateKilledBy(const int iClientNumber)
{
    imapy_it it(m_mapKilledBy.find(iClientNumber));

    // See if this player has been killed by this attacking player before
    if (it == m_mapKilledBy.end())
    {
        m_mapKilledBy.insert(pair<int, byte>(iClientNumber, 1));    
        it = m_mapKilledBy.find(iClientNumber);
    }
    
    // If they have been killed 4 times or more by them, then send the Rival message to everybody in game, but display a special effect and sound for the victim
    if (it->second > 3)
    {
        CPlayer *pKiller(Game.GetPlayerFromClientNumber(iClientNumber));
        
        if (pKiller != NULL)
        {       
            CBufferFixed<11> buffer;
            buffer << GAME_CMD_RIVAL_MESSAGE << pKiller->GetClientNumber() << m_iClientNumber << it->second << byte(pKiller->GetAnnouncerVoice());
            Game.BroadcastGameData(buffer, true, -1, 3000);

            if (it->second == 4)
                pKiller->AddRivalCount();

            Game.LogAward(GAME_LOG_AWARD_RIVAL, pKiller->GetHero()->GetAsUnit(), this->GetHero()->GetAsUnit());
        }
    }
    
    // Increase the number of times they have been killed by them
    it->second++;   
}


/*====================
  CPlayer::Connected
  ====================*/
void    CPlayer::Connected(uint uiTime)
{
    SetFlags(PLAYER_FLAG_LOADING);
    RemoveFlags(PLAYER_FLAG_DISCONNECTED);
    SetTerminationTime(INVALID_TIME);
}


/*====================
  CPlayer::FinishedLoading
  ====================*/
void    CPlayer::FinishedLoading(uint uiTime, uint uiMaxDisconnectTime)
{
    if (m_uiDisconnectTime < uiTime)
        m_uiTotalDisconnectedTime += uiTime - m_uiDisconnectTime;

    // Announce connection
    uint uiRemaingDisconnectTime(0);
    if (m_uiTotalDisconnectedTime < uiMaxDisconnectTime)
        uiRemaingDisconnectTime = uiMaxDisconnectTime - m_uiTotalDisconnectedTime;

    if (HasFlags(PLAYER_FLAG_WAS_CONNECTED))
    {
        CBufferFixed<9> buffer; 
        buffer << GAME_CMD_RECONNECT_MESSAGE << GetClientNumber() << uiRemaingDisconnectTime;
        Game.BroadcastGameData(buffer, true);
    }
    else
    {
        CBufferFixed<5> buffer; 
        buffer << GAME_CMD_CONNECT_MESSAGE << GetClientNumber();
        Game.BroadcastGameData(buffer, true);
    }
        

    SetLastInputTime(uiTime);
    RemoveFlags(PLAYER_FLAG_LOADING);
    SetDisconnectedTime(INVALID_TIME);

    m_fLoadingProgress = 0.0f;

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));

    if (pTeam != NULL && pTeam->IsActiveTeam())
        pTeam->UpdateVoiceTargets(m_iClientNumber);
}


/*====================
  CPlayer::Disconnected
  ====================*/
void    CPlayer::Disconnected(uint uiTime, uint uiMaxDisconnectTime)
{
    SetFlags(PLAYER_FLAG_DISCONNECTED | PLAYER_FLAG_WAS_CONNECTED);
    SetDisconnectedTime(uiTime);

    if (m_uiTotalDisconnectedTime >= uiMaxDisconnectTime && GetTeam() != TEAM_SPECTATOR)
        Terminate();

    SetTerminationTime(uiTime + (uiMaxDisconnectTime - m_uiTotalDisconnectedTime));
}


/*====================
  CPlayer::CanSelectHero
  ====================*/
bool    CPlayer::CanSelectHero(ushort unHeroID)
{
    if (unHeroID == INVALID_ENT_TYPE)
        return false;

    if (HasSelectedHero() && !d_allowMultipleHeroes)
        return false;

    return true;
}


/*====================
  CPlayer::SelectHero
  ====================*/
void    CPlayer::SelectHero(ushort unHeroID)
{
    m_unSelectedHeroID = unHeroID;

    CHeroDefinition *pHero(EntityRegistry.GetDefinition<CHeroDefinition>(unHeroID));
    if (pHero != NULL)
    {
        if (!pHero->HasAltAvatars())
        {
            m_unSelectedAvatarBits = 0;
            m_uiSelectedAvatarKey = INVALID_INDEX;
        }
        else
        {
            m_unSelectedAvatarBits = INVALID_ENT_TYPE;
            m_uiSelectedAvatarKey = INVALID_INDEX;
        }
    }
    else
    {
        m_unSelectedAvatarBits = INVALID_ENT_TYPE;
        m_uiSelectedAvatarKey = INVALID_INDEX;
    }

    CBufferFixed<3> buffer;
    buffer << GAME_CMD_SELECT_HERO << m_unSelectedHeroID;
    Game.SendGameData(m_iClientNumber, buffer, true);
}


/*====================
  CPlayer::SpawnHero
  ====================*/
bool    CPlayer::SpawnHero()
{
    if (m_unSelectedHeroID == INVALID_ENT_TYPE)
        return false;

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return false;

    // Spawn the unit
    IGameEntity *pNewEnt(Game.AllocateEntity(m_unSelectedHeroID));
    if (pNewEnt == NULL || pNewEnt->GetAsHero() == NULL)
    {
        Console.Warn << _T("Failed to spawn hero: ") << EntityRegistry.LookupName(m_unSelectedHeroID) << newl;
        return false;
    }

    tstring sPrevCtxCategory(g_ResourceInfo.GetGameContextCategory());
    g_ResourceInfo.SetGameContextCategory(_T("heroes"));
    Game.Precache(m_unSelectedHeroID, PRECACHE_ALL, _T("All"));
    g_ResourceInfo.SetGameContextCategory(sPrevCtxCategory);

    IHeroEntity *pHero(pNewEnt->GetAsHero());

    uivector vModifierKeys;

    if (m_uiSelectedAvatarKey != INVALID_INDEX)
        vModifierKeys.push_back(m_uiSelectedAvatarKey);

    if (m_uiTaunt != INVALID_INDEX)
    {
        const tstring &sTauntModifierKey(Host.GetTauntModifier(m_uiTaunt));

        uint uiTauntModifierKey(EntityRegistry.LookupModifierKey(sTauntModifierKey));

        if (uiTauntModifierKey != INVALID_INDEX)
            vModifierKeys.push_back(uiTauntModifierKey);
    }
    
    pHero->SetPersistentModifierKeys(vModifierKeys);
    pHero->UpdateModifiers();

    AssignHero(pHero);
    pTeam->SetHeroSpawnPosition(pHero);
    pHero->Spawn();

    pHero->PlayerCommand(SUnitCommand(UNITCMD_STOP));
    return true;
}


/*====================
  CPlayer::AssignHero
  ====================*/
void    CPlayer::AssignHero(IHeroEntity *pHero)
{
    if (pHero != NULL && pHero->GetIndex() == m_uiHeroIndex)
        return;
    if (pHero == NULL && m_uiHeroIndex == INVALID_INDEX)
        return;

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != NULL)
        pTeam->IncrementRosterChangeSequence();

    if (pHero == NULL)
    {
        m_uiHeroIndex = INVALID_INDEX;
        return;
    }

    m_uiHeroIndex = pHero->GetIndex();
    pHero->SetOwnerClientNumber(GetClientNumber());
    pHero->SetTeam(GetTeam());
    ClearAllSwapRequests();
}


/*====================
  CPlayer::IsIsolated
  ====================*/
bool    CPlayer::IsIsolated() const
{
    IHeroEntity *pHero(GetHero());
    if (pHero == NULL)
        return false;

    return pHero->IsIsolated();
}


/*====================
  CPlayer::AssignStats
  ====================*/
void    CPlayer::AssignStats(CGameStats *pStats)
{
    if (pStats == NULL)
    {
        m_uiStatsIndex = INVALID_INDEX;
        return;
    }

    m_uiStatsIndex = pStats->GetIndex();
    pStats->SetPlayerClientID(GetClientNumber());
}


/*====================
  CPlayer::GetKFactor
  ====================*/
float   CPlayer::GetKFactor() const
{
    float fKFactor(((psf_medianScalingRank - GetRank()) / psf_KFactorScale) +  psf_baseKFactor);
    return CLAMP(fKFactor, psf_minKFactor.GetValue(), psf_maxKFactor.GetValue());
}


/*====================
  CPlayer::GetMatchWinValue
  ====================*/
float   CPlayer::GetMatchWinValue() const
{
    const CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL || pTeam->GetTeamID() <= TEAM_SPECTATOR || pTeam->GetTeamID() >= TEAM_INVALID)
        return 0;

    return (1.0f - pTeam->GetWinChance()) * GetKFactor();
}


/*====================
  CPlayer::GetMatchLossValue
  ====================*/
float   CPlayer::GetMatchLossValue() const
{
    const CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL || pTeam->GetTeamID() <= TEAM_SPECTATOR || pTeam->GetTeamID() >= TEAM_INVALID)
        return 0;

    return -pTeam->GetWinChance() * GetKFactor();
}


/*====================
  CPlayer::GetSkillDifferenceAdjustment
  ====================*/
float   CPlayer::GetSkillDifferenceAdjustment() const
{
#if 1
    const CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL || pTeam->GetTeamID() <= TEAM_SPECTATOR || pTeam->GetTeamID() >= TEAM_INVALID)
        return 1.0f;

    float fDiffFromTeamAverage(MAX(GetRank() - pTeam->GetAverageRank(), 0.0f));
    float fClampedDiff(MAX(psf_gammaCurveRange - fDiffFromTeamAverage, 0.0f));
    return M_GammaDistribution(fClampedDiff, psf_gammaCurveK, psf_gammaCurveTheta);
#else
    return 1.0f;
#endif
}


/*====================
  CPlayer::GetAdjustedMatchWinValue
  ====================*/
float   CPlayer::GetAdjustedMatchWinValue() const
{
    const CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL || pTeam->GetTeamID() <= TEAM_SPECTATOR || pTeam->GetTeamID() >= TEAM_INVALID)
        return 0.0f;

    float fBasePointValue((1.0f - pTeam->GetWinChance()) * GetKFactor());
    return fBasePointValue * GetSkillDifferenceAdjustment();
}


/*====================
  CPlayer::GetAdjustedMatchLossValue
  ====================*/
float   CPlayer::GetAdjustedMatchLossValue() const
{
    const CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL || pTeam->GetTeamID() <= TEAM_SPECTATOR || pTeam->GetTeamID() >= TEAM_INVALID)
        return 0.0f;

    float fBasePointValue(-pTeam->GetWinChance() * GetKFactor());
    return fBasePointValue * GetSkillDifferenceAdjustment();
}


/*====================
  CPlayer::GetColor
  ====================*/
CVec4f  CPlayer::GetColor(uint uiIndex)
{
    if (uiIndex < g_playerColors.GetSize())
        return GetColorFromString(g_playerColors.GetValue(uiIndex));

    return WHITE;
}

CVec4f  CPlayer::GetColor() const
{
    if (m_uiTeamID == TEAM_1 || m_uiTeamID == TEAM_2)
        return GetColor((m_uiTeamID - 1) * 5 + m_iTeamIndex);

    return WHITE;
}


/*====================
  CPlayer::GetAccountColor
  ====================*/
CVec4f  CPlayer::GetAccountColor() const
{
    if (m_uiChatNameColor != INVALID_INDEX)
        return GetColorFromString(Host.GetChatNameColorString(m_uiChatNameColor));
    else if (HasFlags(PLAYER_FLAG_STAFF))
        return RED;
    else if (HasFlags(PLAYER_FLAG_PREMIUM))
        return GOLDENSHIELD;

    return WHITE;
}


/*====================
  CPlayer::GetPlayerIndex
  ====================*/
uint    CPlayer::GetPlayerIndex() const
{
    if (m_uiTeamID == TEAM_1 || m_uiTeamID == TEAM_2)
        return (m_uiTeamID - 1) * 5 + m_iTeamIndex;

    return uint(-1);
}


/*====================
  CPlayer::GetColorName
  ====================*/
const tstring&  CPlayer::GetColorName(uint uiIndex)
{
    if (uiIndex < g_playerColors.GetSize())
        return g_playerColorNames.GetValue(uiIndex);

    return TSNULL;
}

const tstring&  CPlayer::GetColorName() const
{
    if (m_uiTeamID == TEAM_1 || m_uiTeamID == TEAM_2)
        return GetColorName((m_uiTeamID - 1) * 5 + m_iTeamIndex);

    return TSNULL;
}


/*====================
  CPlayer::Terminate
  ====================*/
void    CPlayer::Terminate()
{
    SetFlags(PLAYER_FLAG_TERMINATED);

    if (GetTeam() != TEAM_1 && GetTeam() != TEAM_2)
        return;

    if (Game.GetGamePhase() >= GAME_PHASE_PRE_MATCH)
    {
        CBufferFixed<9> buffer;
        buffer << GAME_CMD_TERMINATED_MESSAGE << GetClientNumber() << uint(0);
        Game.BroadcastGameData(buffer, true);
    }   

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam == NULL)
        return;
        
    // Distribute gold
    ushort unGold(INT_FLOOR(GetGold() / float(pTeam->GetNumActiveClients())));
    const ivector &vClients(pTeam->GetClientList());
    for (ivector_cit itClient(vClients.begin()); itClient != vClients.end(); ++itClient)
    {
        CPlayer *pPlayer(Game.GetPlayer(*itClient));
        if (pPlayer == NULL || pPlayer->HasFlags(PLAYER_FLAG_TERMINATED))
            continue;

        pPlayer->GiveGold(unGold, NULL);
    }

    m_unGold = 0;

    // Remove their ownership tag on any items being carried by other units
    IGameEntity *pEntity(Game.GetFirstEntity());
    while (pEntity != NULL)
    {
        IEntityItem *pItem(pEntity->GetAsItem());
        if (pItem != NULL && pItem->GetPurchaserClientNumber() == GetClientNumber())
            pItem->SetPurchaserClientNumber(-1);

        pEntity = Game.GetNextEntity(pEntity);
    }

    IHeroEntity *pHero(GetHero());
    if (pHero != NULL)
    {
        // Drop items
        CVec3f v3HeroSpawnPos(pTeam->GetHeroSpawnPosition());
        for (int iSlot(INVENTORY_START_BACKPACK); iSlot <= INVENTORY_END_STASH; ++iSlot)
        {
            IEntityItem *pItem(pHero->GetItem(iSlot));
            if (pItem == NULL)
                continue;

            pItem->Drop(Game.GetTerrainPosition(v3HeroSpawnPos.xy() + M_RandomPointInCircle() * 32.0f), true);
        }

        pHero->Terminate();
    }
}


/*====================
  CPlayer::MatchRemake
  ====================*/
void    CPlayer::MatchRemake()
{
    m_unSelectedHeroID = INVALID_ENT_TYPE;
    m_unSelectedAvatarBits = INVALID_ENT_TYPE;
    m_uiSelectedAvatarKey = INVALID_INDEX;
    m_uiHeroIndex = INVALID_INDEX;
    m_uiStatsIndex = INVALID_INDEX;

    ResetCamera();
    ClearAffiliations();
    
    m_yFullSharedControl = 0;
    m_yPartialSharedControl = 0;

    // Assets
    m_unGold = 0;

    m_vPetUIDs.clear();

    m_uiActionCount = 0;
    m_uiStartActionCount = INVALID_TIME;

    m_uiDisconnectTime = INVALID_TIME;
    m_uiTotalDisconnectedTime = 0;
    m_uiTerminationTime = INVALID_TIME;
    m_uiPlayTime = 0;

    m_uiLastInputTime = INVALID_TIME;

    m_yKillStreak = 0;
    m_yMultiKill = 0;
    m_uiLastKillTime = INVALID_TIME;

    MemManager.Set(m_aStatTotals, 0, sizeof(uint) * NUM_PLAYER_STATS);

    m_setSelection.clear();

    m_uiLastVoteCallTime = INVALID_TIME;
    m_uiLastMapPingTime = INVALID_TIME;
    m_uiLastUnitPingTime = INVALID_TIME;
    m_uiChatCounter = 0;

    m_uiDraftRound = 0;

    m_fLoadingProgress = 0.0f;

    RemoveFlags(PLAYER_FLAG_READY);
    RemoveFlags(PLAYER_FLAG_HAS_REPICKED);
    RemoveFlags(PLAYER_FLAG_CAN_PICK);
    RemoveFlags(PLAYER_FLAG_LOADING);
    RemoveFlags(PLAYER_FLAG_WAS_CONNECTED);
    RemoveFlags(PLAYER_FLAG_LOADED_HEROES);
    RemoveFlags(PLAYER_FLAG_EXCUSED);
    RemoveFlags(PLAYER_FLAG_KICKED);
    
    m_uiChatSymbol = INVALID_INDEX;
    m_uiChatNameColor = INVALID_INDEX;
    m_uiAccountIcon = INVALID_INDEX;
    m_unSelectedAvatarBits = INVALID_ENT_TYPE;

    UpdateUpgrades();
}


/*====================
  CPlayer::DrawViewBox
  ====================*/
void    CPlayer::DrawViewBox(CUITrigger &minimap, CCamera &camera)
{
    CBufferFixed<48> buffer;

    CVec3f v3Start(camera.GetOrigin());
    CPlane plane(V_UP, Game.GetCameraHeight(camera.GetCenter().x, camera.GetCenter().y));

    CVec3f  v3Dir(camera.ConstructRay(0.0f, 0.0f));
    v3Dir *= FAR_AWAY;
    CVec3f v3End(v3Start + v3Dir);
    float fFraction(1.0f);
    I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
    CVec3f v3TL(LERP(fFraction, v3Start, v3End));

    v3Dir = camera.ConstructRay(float(camera.GetX() + camera.GetWidth()), 0.0f);
    v3Dir *= FAR_AWAY;
    v3End = v3Start + v3Dir;
    fFraction = 1.0f;
    I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
    CVec3f v3TR(LERP(fFraction, v3Start, v3End));

    v3Dir = camera.ConstructRay(0.0f, float(camera.GetY() + camera.GetHeight()));
    v3Dir *= FAR_AWAY;
    v3End = v3Start + v3Dir;
    fFraction = 1.0f;
    I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
    CVec3f v3BL(LERP(fFraction, v3Start, v3End));

    v3Dir = camera.ConstructRay(float(camera.GetWidth()), float(camera.GetHeight()));
    v3Dir *= FAR_AWAY;
    v3End = v3Start + v3Dir;
    fFraction = 1.0f;
    I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
    CVec3f v3BR(LERP(fFraction, v3Start, v3End));

    buffer.Clear();
    buffer << v3TL.x / Game.GetWorldWidth() << 1.0f - v3TL.y / Game.GetWorldHeight()
        << v3TR.x / Game.GetWorldWidth() << 1.0f - v3TR.y / Game.GetWorldHeight()
        << 1.0f << 1.0f << 1.0f << 1.0f
        << 1.0f << 1.0f << 1.0f << 1.0f;
    minimap.Execute(_T("line"), buffer);

    buffer.Clear();
    buffer << v3TR.x / Game.GetWorldWidth() << 1.0f - v3TR.y / Game.GetWorldHeight()
        << v3BR.x / Game.GetWorldWidth() << 1.0f - v3BR.y / Game.GetWorldHeight()
        << 1.0f << 1.0f << 1.0f << 1.0f
        << 1.0f << 1.0f << 1.0f << 1.0f;
    minimap.Execute(_T("line"), buffer);

    buffer.Clear();
    buffer << v3TL.x / Game.GetWorldWidth() << 1.0f - v3TL.y / Game.GetWorldHeight()
        << v3BL.x / Game.GetWorldWidth() << 1.0f - v3BL.y / Game.GetWorldHeight()
        << 1.0f << 1.0f << 1.0f << 1.0f
        << 1.0f << 1.0f << 1.0f << 1.0f;
    minimap.Execute(_T("line"), buffer);

    buffer.Clear();
    buffer << v3BL.x / Game.GetWorldWidth() << 1.0f - v3BL.y / Game.GetWorldHeight()
        << v3BR.x / Game.GetWorldWidth() << 1.0f - v3BR.y / Game.GetWorldHeight()
        << 1.0f << 1.0f << 1.0f << 1.0f
        << 1.0f << 1.0f << 1.0f << 1.0f;
    minimap.Execute(_T("line"), buffer);
}


/*====================
  CPlayer::SetupCamera
  ====================*/
void    CPlayer::SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles)
{
    if (cam_mode == 0)
    {
        camera.RemoveFlags(CAM_FIRST_PERSON);
        camera.SetFovFromAspect(g_camFov, g_camAspect, g_camWeightX, g_camWeightY);

        CVec3f v3Target(Game.GetCameraPosition(v3InputPosition.xy()));
        //v3Target.z += m_fCameraHeight;
        camera.SetCenter(v3Target);

        // Determine camera position and angles
        camera.SetAngles(v3InputAngles);

        CAxis axis(v3InputAngles);
        camera.SetOrigin(M_PointOnLine(v3Target, axis.Forward(), -m_v3Position.z));

        camera.SetLodDistance(m_v3Position.z);

        if (m_fCameraDistance > g_camDistanceMax)
        {
            camera.SetZFar(camera.GetZNear() + M_PI);
            camera.AddFlags(CAM_NO_CLIFFS | CAM_NO_WORLD);
        }
        else
        {
            camera.SetZFar(0.0f);
            camera.RemoveFlags(CAM_NO_CLIFFS | CAM_NO_WORLD);
        }
    }
    else
    {
        camera.RemoveFlags(CAM_FIRST_PERSON);
        camera.SetFovFromAspect(g_camFov, g_camAspect, g_camWeightX, g_camWeightY);
        camera.SetAngles(v3InputAngles);
        camera.SetOrigin(v3InputPosition);
    }
}


/*====================
  CPlayer::ReadClientSnapshot
  ====================*/
void    CPlayer::ReadClientSnapshot(const CClientSnapshot &snapshot)
{
    m_v3Position = snapshot.GetCameraPosition();
    m_v2Cursor = snapshot.GetCursorPosition();
    m_v3Angles = snapshot.GetAngles();

    if (Game.GetGamePhase() == GAME_PHASE_ENDED)
        return;
}


/*====================
  CPlayer::PrepareClientSnapshot
  ====================*/
void    CPlayer::PrepareClientSnapshot(CClientSnapshot &snapshot)
{
    float fFrameTime(MsToSec(Host.GetFrameLength()));

    if (!m_bCameraDrag && !m_bCameraScroll && cam_edgeScroll)
    {
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_LEFT, Input.GetCursorPos().x == 0);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_UP, Input.GetCursorPos().y == 0);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT, Input.GetCursorPos().x == Vid.GetScreenW() - 1);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_DOWN, Input.GetCursorPos().y == Vid.GetScreenH() - 1);
    }
    else
    {
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_LEFT, false);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_UP, false);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT, false);
        snapshot.SetButton(GAME_CMDR_BUTTON_EDGESCROLL_DOWN, false);
    }

    if (!ReplayManager.IsPlaying()
        && !HasFlags(PLAYER_FLAG_LOCAL)
        && (GetTeam() == TEAM_1 || GetTeam() == TEAM_2))
    {
        cam_mode.Reset();
        g_camDistanceDefault.Reset();
        g_camDistanceMin.Reset();
        g_camDistanceMax.Reset();
        g_camHeightMin.Reset();
        g_camHeightMax.Reset();
        g_camPitch.Reset();
        g_camYaw.Reset();
        g_camFov.Reset();
        g_camAspect.Reset();
        g_camWeightX.Reset();
        g_camWeightY.Reset();

        m_v3Angles.Set(g_camPitch, 0.0f, g_camYaw);
    }

    if (ReplayManager.IsPlaying())
        fFrameTime /= ReplayManager.GetSpeedScale();

    CVec2f v2Cursor(Input.GetCursorPos());
    v2Cursor.x /= Vid.GetScreenW();
    v2Cursor.y /= Vid.GetScreenH();
    snapshot.SetCursorPosition(v2Cursor);

    CVec3f v3Position(snapshot.GetCameraPosition());
    
    CVec3f v3Velocity(V3_ZERO);

    if (cam_mode == 0)
    {
        m_fCameraDistance = CLAMP(m_fCameraDistance, float(g_camDistanceMin), float(g_camDistanceMax));

        if (HasFlags(PLAYER_FLAG_LOCAL) || ReplayManager.IsPlaying())
            m_fCameraHeight = CLAMP(m_fCameraHeight, float(g_camHeightMin), float(g_camHeightMax));
        else
            m_fCameraHeight = 0.0f;

        if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD | GAME_CMDR_BUTTON_EDGESCROLL_UP))
            v3Velocity += CVec3f(-sin(DEG2RAD(m_v3Angles[YAW])), cos(DEG2RAD(m_v3Angles[YAW])), 0.0f);
        if (snapshot.IsButtonDown(GAME_BUTTON_BACK | GAME_CMDR_BUTTON_EDGESCROLL_DOWN))
            v3Velocity += CVec3f(sin(DEG2RAD(m_v3Angles[YAW])), -cos(DEG2RAD(m_v3Angles[YAW])), 0.0f);
        if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT | GAME_CMDR_BUTTON_EDGESCROLL_RIGHT))
            v3Velocity += CVec3f(cos(DEG2RAD(m_v3Angles[YAW])), sin(DEG2RAD(m_v3Angles[YAW])), 0.0f);
        if (snapshot.IsButtonDown(GAME_BUTTON_LEFT | GAME_CMDR_BUTTON_EDGESCROLL_LEFT))
            v3Velocity += CVec3f(-cos(DEG2RAD(m_v3Angles[YAW])), -sin(DEG2RAD(m_v3Angles[YAW])), 0.0f);

        v3Velocity.Normalize();
        v3Velocity *= cam_scrollSpeed;

        if (m_bCameraScroll)
        {
            CVec2f v2Delta(v2Cursor - m_v2StartScrollCursor);

            v2Delta.Rotate(m_v3Angles[YAW]);

            float fSignX(SIGN(v2Delta.x));
            float fDistanceX(ABS(v2Delta.x));

            float fSignY(SIGN(v2Delta.y));
            float fDistanceY(ABS(v2Delta.y));

            v3Velocity.x += fSignX * pow(fDistanceX * cam_scrollScale, cam_scrollPower);
            v3Velocity.y -= fSignY * pow(fDistanceY * cam_scrollScale, cam_scrollPower);
        }

        v3Position += v3Velocity * fFrameTime * (m_fCameraDistance / g_camDistanceMax);
        v3Position.z = m_fCameraDistance;
    }
    else if (cam_mode == 1)
    {
        CAxis aAxis(m_v3Angles);

        if (snapshot.IsButtonDown(GAME_CMDR_BUTTON_MOUSELOOK))
        {
            if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
                v3Velocity += aAxis.Forward();
            if (snapshot.IsButtonDown(GAME_BUTTON_BACK))
                v3Velocity -= aAxis.Forward();
            if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                v3Velocity += aAxis.Right();
            if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                v3Velocity -= aAxis.Right();

            v3Velocity.Normalize();
            v3Velocity *= cam_flySpeed;

            v3Position += v3Velocity * fFrameTime;
        }
        else
        {
            v3Position.z -= Game.GetCameraHeight(v3Position.x, v3Position.y);

            CVec3f v3Forward(aAxis.Forward());
            v3Forward.z = 0.0f;
            v3Forward.Normalize();

            CVec3f v3Right(aAxis.Right());
            v3Right.z = 0.0f;
            v3Right.Normalize();

            if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
                v3Velocity += v3Forward;
            if (snapshot.IsButtonDown(GAME_BUTTON_BACK))
                v3Velocity -= v3Forward;
            if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                v3Velocity += v3Right;
            if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                v3Velocity -= v3Right;

            v3Velocity.Normalize();
            v3Velocity *= cam_flySpeed;

            v3Position += v3Velocity * fFrameTime;

            v3Position.z += Game.GetCameraHeight(v3Position.x, v3Position.y);
        }
    }   

    if (m_bCameraDrag)
    {
        STraceInfo trace;
    
        CVec3f v3Dir(m_cDragCamera.ConstructRay(Input.GetCursorPos() - m_cDragCamera.GetXY()));
        CVec3f v3End(M_PointOnLine(m_cDragCamera.GetOrigin(), v3Dir, FAR_AWAY));

#if 1 // This behaves a bit better on sloped terrain
        float fFraction(1.0f);
        if (I_LinePlaneIntersect(m_cDragCamera.GetOrigin(), v3End, m_plDragPlane, fFraction))
        {
            CVec3f v3EndPos(LERP(fFraction, m_cDragCamera.GetOrigin(), v3End));
            
            v3Position.x = m_v3StartDragCamera.x - v3EndPos.x + m_v3StartDragWorld.x;
            v3Position.y = m_v3StartDragCamera.y - v3EndPos.y + m_v3StartDragWorld.y;
        }
#else
        if (Game.TraceLine(trace, m_cDragCamera.GetOrigin(), v3End, TRACE_TERRAIN))
        {
            v3Position.x = m_v3StartDragCamera.x - trace.v3EndPos.x + m_v3StartDragWorld.x;
            v3Position.y = m_v3StartDragCamera.y - trace.v3EndPos.y + m_v3StartDragWorld.y;
        }
#endif
    }

    // HACK: using a temp camera to calculate fovy
    CCamera camera;
    camera.SetWidth(float(Vid.GetScreenW()));
    camera.SetHeight(float(Vid.GetScreenH()));
    camera.SetFovFromAspect(g_camFov, g_camAspect, g_camWeightX, g_camWeightY);
    camera.SetAngles(m_v3Angles);
    camera.SetDistance(m_fCameraDistance);

    // FIXME: This works pretty well, but is imperfect
    if (cam_mapConstraints && cam_mode == 0)
    {       
        CVec3f v3Start(camera.GetOrigin());
        CPlane plane(V_UP, 0.0f);

        CVec3f  v3Dir(camera.ConstructRay(0.0f, 0.0f));
        v3Dir *= 2500.0f;
        CVec3f v3End(v3Start + v3Dir);
        float fFraction(1.0f);
        I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
        CVec3f v3TL(LERP(fFraction, v3Start, v3End));

        v3Dir = camera.ConstructRay(float(camera.GetX() + camera.GetWidth()), 0.0f);
        v3Dir *= 2500.0f;
        v3End = v3Start + v3Dir;
        fFraction = 1.0f;
        I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
        CVec3f v3TR(LERP(fFraction, v3Start, v3End));

        v3Dir = camera.ConstructRay(0.0f, float(camera.GetY() + camera.GetHeight()));
        v3Dir *= 2500.0f;
        v3End = v3Start + v3Dir;
        fFraction = 1.0f;
        I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
        CVec3f v3BL(LERP(fFraction, v3Start, v3End));

        v3Dir = camera.ConstructRay(float(camera.GetWidth()), float(camera.GetHeight()));
        v3Dir *= 2500.0f;
        v3End = v3Start + v3Dir;
        fFraction = 1.0f;
        I_LinePlaneIntersect(v3Start, v3End, plane, fFraction);
        CVec3f v3BR(LERP(fFraction, v3Start, v3End));

        CRectf bbBounds(FAR_AWAY, FAR_AWAY, -FAR_AWAY, -FAR_AWAY);

        bbBounds.AddPoint(v3TL.xy());
        bbBounds.AddPoint(v3TR.xy());
        bbBounds.AddPoint(v3BL.xy());
        bbBounds.AddPoint(v3BR.xy());

        const CRectf &bbCameraBounds(Game.GetCameraBounds());

        v3Position.x = CLAMP(v3Position.x, bbCameraBounds.left - bbBounds.left + 128.0f, bbCameraBounds.right - bbBounds.right - 128.0f);
        v3Position.y = CLAMP(v3Position.y, bbCameraBounds.top - bbBounds.top + 128.0f, bbCameraBounds.bottom - bbBounds.bottom - 128.0f);
    }

    snapshot.SetCameraPosition(v3Position);
    snapshot.SetAngles(m_v3Angles);
    snapshot.SetFov(g_camFov);
}


/*====================
  CPlayer::ZoomIn
  ====================*/
void    CPlayer::ZoomIn(CClientSnapshot &snapshot)
{
    m_fCameraDistance = CLAMP(m_fCameraDistance - cam_distanceStep, float(g_camDistanceMin), float(g_camDistanceMax));
}


/*====================
  CPlayer::ZoomOut
  ====================*/
void    CPlayer::ZoomOut(CClientSnapshot &snapshot)
{
    m_fCameraDistance = CLAMP(m_fCameraDistance + cam_distanceStep, float(g_camDistanceMin), float(g_camDistanceMax));
}


/*====================
  CPlayer::AdjustPitch
  ====================*/
void    CPlayer::AdjustPitch(float fPitch)
{
    if (!HasFlags(PLAYER_FLAG_LOCAL) && !ReplayManager.IsPlaying())
        return;

    m_v3Angles[PITCH] -= fPitch;

    if (cam_mode == 0)
        m_v3Angles[PITCH] = CLAMP(m_v3Angles[PITCH], -89.0f, -1.0f);
    else
        m_v3Angles[PITCH] = CLAMP(m_v3Angles[PITCH], -89.9f, 89.9f);
}


/*====================
  CPlayer::AdjustYaw
  ====================*/
void    CPlayer::AdjustYaw(float fYaw)
{
    if (!HasFlags(PLAYER_FLAG_LOCAL) && !ReplayManager.IsPlaying())
        return;

    m_v3Angles[YAW] -= fYaw;
    if (_isnan(m_v3Angles[YAW]) || !_finite(m_v3Angles[YAW]))
        m_v3Angles[YAW] = 0.0f;
    while (m_v3Angles[YAW] > 360.0f) m_v3Angles[YAW] -= 360.0f;
    while (m_v3Angles[YAW] < 0.0f) m_v3Angles[YAW] += 360.0f;
}


/*====================
  CPlayer::AdjustCameraHeight
  ====================*/
void    CPlayer::AdjustCameraHeight(float fAmount)
{
    if (HasFlags(PLAYER_FLAG_LOCAL) || ReplayManager.IsPlaying())
        m_fCameraHeight = CLAMP(m_fCameraHeight + (cam_heightStep * fAmount), float(g_camHeightMin), float(g_camHeightMax));
}


/*====================
  CPlayer::ResetCamera
  ====================*/
void    CPlayer::ResetCamera()
{
    m_fCameraDistance = g_camDistanceDefault;
    m_fCameraHeight = 0.0f;
    m_v3Angles.Set(g_camPitch, 0.0f, g_camYaw);
}


/*====================
  CPlayer::StartDrag
  ====================*/
void     CPlayer::StartDrag(CCamera *pCamera)
{
    STraceInfo trace;
    
    CVec3f v3Dir(pCamera->ConstructRay(Input.GetCursorPos() - pCamera->GetXY()));
    CVec3f v3End(M_PointOnLine(pCamera->GetOrigin(), v3Dir, FAR_AWAY));

    if (Game.TraceLine(trace, pCamera->GetOrigin(), v3End, TRACE_TERRAIN))
    {
        m_bCameraDrag = true;
        m_v2StartDragCursor = Input.GetCursorPos();
        m_v3StartDragCamera = m_v3Position;
        m_v3StartDragWorld = trace.v3EndPos;
        m_cDragCamera = *pCamera;
        m_plDragPlane = CPlane(V_UP, m_v3StartDragWorld);
    }
}


/*====================
  CPlayer::EndDrag
  ====================*/
void    CPlayer::EndDrag()
{
    m_bCameraDrag = false;
}


/*====================
  CPlayer::StartScroll
  ====================*/
void     CPlayer::StartScroll()
{
    m_bCameraScroll = true;
    m_v2StartScrollCursor = Input.GetCursorPos();
    m_v2StartScrollCursor.x /= Vid.GetScreenW();
    m_v2StartScrollCursor.y /= Vid.GetScreenH();
}


/*====================
  CPlayer::EndScroll
  ====================*/
void    CPlayer::EndScroll()
{
    m_bCameraScroll = false;
}


/*====================
  CPlayer::Spawn
  ====================*/
void    CPlayer::Spawn()
{
    if (!Game.IsClient() || m_iClientNumber != Game.GetLocalClientNum())
        return;

    // Get a nice initial camera position
    CTeamInfo *pTeam(Game.GetTeam(m_uiTeamID));
    if (pTeam != NULL)
    {
        IVisualEntity *pBase(Game.GetVisualEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != NULL)
            m_v3Position = pBase->GetPosition();

        ResetCamera();
        Game.GetCurrentSnapshot()->SetCameraPosition(m_v3Position);
        Game.GetCurrentSnapshot()->SetAngles(m_v3Angles);
    }

    return;
}


/*====================
  CPlayer::GiveGold
  ====================*/
void    CPlayer::GiveGold(ushort unGold, IUnitEntity *pSource, IUnitEntity *pTarget)
{
        // Clamp gold so it doesnt overflow
    unGold = min(unGold, ushort(USHRT_MAX - m_unGold));

    m_unGold += unGold;

    if (pTarget == NULL)
        pTarget = pSource;

    if (unGold > 0 && pTarget != NULL && pTarget->IsActive())
    {
        CBufferFixed<1> buffer;
        buffer << GAME_CMD_GOLD_EVENT;
        Game.SendGameData(m_iClientNumber, buffer, false);

        Game.SendPopup(POPUP_GOLD, pSource, pTarget, unGold);
    }
}


/*====================
  CPlayer::TakeGold
  ====================*/
void    CPlayer::TakeGold(ushort unGold)
{
    if (unGold >= m_unGold)
    {
        unGold = m_unGold;
        m_unGold = 0;
    }
    else
        m_unGold -= unGold;
}


/*====================
  CPlayer::SpendGold
  ====================*/
bool    CPlayer::SpendGold(ushort unCost)
{
    if (m_unGold < unCost)
        return false;
    
    m_unGold -= unCost;

    AdjustStat(PLAYER_STAT_GOLD_SPENT, unCost);
    return true;
}


/*====================
  CPlayer::IsEnemy
  ====================*/
bool    CPlayer::IsEnemy(const IUnitEntity *pOther) const
{
    if (pOther == NULL)
        return false;
    if (pOther->GetTeam() == TEAM_PASSIVE)
        return false;
    if (pOther->GetTeam() == GetTeam())
        return false;

    return true;
}


/*====================
  CPlayer::CanSee
  ====================*/
bool    CPlayer::CanSee(const IVisualEntity *pTarget) const
{
    if (pTarget == NULL)
        return false;

    if (pTarget->IsStatic())
        return true;

    if (m_bFullVision)
        return true;

    const IUnitEntity *pUnit(pTarget->GetAsUnit());

    if (pUnit != NULL && pUnit->GetOwnerClientNumber() == GetClientNumber())
        return true;
    if (m_bIsolated && !pTarget->HasVisibilityFlags(VIS_PLAYER_SIGHTED(GetTeam(), GetTeamIndex())))
        return false;
    if (pTarget->GetTeam() == GetTeam())
        return true;
    if (pUnit != NULL && pUnit->GetTeam() != GetTeam() && pUnit->GetHidden())
        return false;
    if (pUnit != NULL && pUnit->GetAlwaysVisible())
        return true;
    if (pUnit != NULL && pUnit->IsStealth() && !pTarget->HasVisibilityFlags(VIS_REVEALED(GetTeam())))
        return false;
    if (pTarget->HasVisibilityFlags(VIS_SIGHTED(GetTeam())))
        return true;

    return false;
}


/*====================
  CPlayer::SetInterface
  ====================*/
void     CPlayer::SetInterface(const tstring &sName)
{
    if (m_unInterfaceIndex == INVALID_NETWORK_STRING)
        m_unInterfaceIndex = NetworkResourceManager.ReserveString();

    NetworkResourceManager.SetString(m_unInterfaceIndex, sName);
}


/*====================
  CPlayer::SetOverlayInterface
  ====================*/
void     CPlayer::SetOverlayInterface(const tstring &sName)
{
    if (m_unOverlayInterfaceIndex == INVALID_NETWORK_STRING)
        m_unOverlayInterfaceIndex = NetworkResourceManager.ReserveString();

    NetworkResourceManager.SetString(m_unOverlayInterfaceIndex, sName);
}


/*====================
  CPlayer::MoveUnitsToSafety
  ====================*/
void    CPlayer::MoveUnitsToSafety(int iClientNum)
{
    // try to move the player's hero back to base.
    IUnitEntity *pHero(GetHero());
    if (pHero != NULL)
    {
        CTeamInfo *pTeam(Game.GetTeam(pHero->GetTeam()));
        if (pTeam != NULL)
        {
            SUnitCommand cmd;
            cmd.v2Dest = pTeam->GetHeroSpawnPosition().xy();

            // if we're channeling an ability, don't interrupt it.
            if (pHero->GetBrain().IsCurrentBehaviorChanneling())
                cmd.yQueue = QUEUE_BACK;
            else
                cmd.yQueue = QUEUE_NONE;

            cmd.iClientNumber = iClientNum;
            cmd.bShared = false;
            cmd.bDirectPathing = false;
            cmd.eCommandID = UNITCMD_MOVE;
            pHero->PlayerCommand(cmd);
        }
    }
}


/*====================
  CPlayer::ProcessGameplayOption
  ====================*/
bool    CPlayer::ProcessGameplayOption(const tstring &sOption, const tstring &sValue)
{
    if (sOption == _T("move_hero_on_disconnect"))
    {
        m_bMoveHeroToSpawnOnDisconnect = AtoB(sValue);
        return true;
    }

    Console.Server << _T("Unknown gameplay option '") << sOption << "' value '" << sValue << "'" << newl;
    return false;
}


/*====================
  CPlayer::UpdateUpgrades
  ====================*/
void    CPlayer::UpdateUpgrades()
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
        case UPGRADE_VALUE_INVALID:
            break;
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
  CPlayer::UpdateUpgrades
  ====================*/
void    CPlayer::UpdateUpgrades(CClientConnection *pClientConnection)
{
    m_setAvailableUpgrades = pClientConnection->GetAvailableUpgrades();
    m_mapSelectedUpgrades = pClientConnection->GetSelectedUpgrades();

    UpdateUpgrades();
}


/*====================
  CPlayer::CanAccessAltAvatar
  ====================*/
bool    CPlayer::CanAccessAltAvatar(const tstring &sHero, const tstring &sAltAvatar)
{
#if 0
    return true;
#endif

    tstring sProductCode(_T("aa.") + sHero + _T(".") + sAltAvatar);

    return m_setAvailableUpgrades.find(sProductCode) != m_setAvailableUpgrades.end();
}


/*====================
  CPlayer::SetAltAvatar
  ====================*/
bool    CPlayer::SetAltAvatar(const tstring &sAltAvatar)
{
    CHeroDefinition *pHero(EntityRegistry.GetDefinition<CHeroDefinition>(m_unSelectedHeroID));
    if (pHero == NULL)
        return false;

    // Check for selecting base definition
    if (TStringCompare(sAltAvatar, _T("Base")) == 0)
    {
        m_unSelectedAvatarBits = 0;
        m_uiSelectedAvatarKey = INVALID_INDEX;
        return true;
    }

    if (!CanAccessAltAvatar(pHero->GetName(), sAltAvatar))
        return false;

    uint uiModifierKey(EntityRegistry.LookupModifierKey(sAltAvatar));

    ushort unBit(pHero->GetModifierBit(uiModifierKey));
    if (unBit == 0)
        return false;
    
    m_unSelectedAvatarBits = unBit;
    m_uiSelectedAvatarKey = uiModifierKey;
    return true;
}