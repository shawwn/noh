// (C)2008 S2 Games
// game_shared_cvars.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "game_shared_cvars.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTF      (g_corpseTime,                  SecToMs(20u),       CVAR_GAMECONFIG);
CVAR_FLOATF     (g_transferRange,               64.0f,              CVAR_GAMECONFIG);
CVAR_FLOATF     (g_touchRange,                  64.0f,              CVAR_GAMECONFIG);
CVAR_FLOATF     (g_blockSlope,                  0.6f,               CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitMoveAngle,               120.0f,             CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitActionAngle,             90.0f,              CVAR_GAMECONFIG);
CVAR_BOOLF      (g_unitActionOnTurn,            false,              CVAR_GAMECONFIG);
CVAR_FLOATF     (g_pathPad,                     4.0f,               CVAR_GAMECONFIG);
CVAR_STRINGF    (g_heroLevelupEffectPath,       "",                 CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitMoveSpeedMin,            100.0f,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitMoveSpeedMax,            522.0f,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_unitSelectionIndicatorPath,  "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG); 
CVAR_UINTF      (g_unitAggressionSightTime,     3000,               CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (g_unitGuardChaseTime,          7000,               CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitGuardDistance,           750.0f,             CVAR_GAMECONFIG);
CVAR_UINTF      (g_unitGuardReaggroChaseTime,   3000,               CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitGuardReaggroDistance,    500.0f,             CVAR_GAMECONFIG);
CVAR_UINTF      (g_unitBehaviorStartAggroDelay, 50,                 CVAR_GAMECONFIG);
CVAR_BOOLF      (g_unitAttackMoveDAC,           true,               CVAR_GAMECONFIG);
CVAR_UINTF      (g_unitAttackAggroTriggerRange, 700,                CVAR_GAMECONFIG);

CVAR_STRINGF    (g_effectStunPath,              "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_effectRecipePath,            "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);

ARRAY_CVAR_STRINGF  (g_powerups,                "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);
ARRAY_CVAR_STRINGF  (g_critters,                "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (g_occlusionHeight,             64.0f,              CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF      (g_multiKillTime,               SecToMs(13.5f),     CVAR_GAMECONFIG);

CVAR_FLOATF     (g_heroAttackAggroRange,        500.0f,             CVAR_GAMECONFIG);
CVAR_UINTF      (g_heroAttackAggroTime,         2000u,              CVAR_GAMECONFIG);
CVAR_UINTF      (g_heroAttackAggroDelay,        0u,                 CVAR_GAMECONFIG);
CVAR_BOOLF      (g_heroAttackReaggroBlock,      false,              CVAR_GAMECONFIG);

CVAR_UINTF      (g_creepBlockRepathTime,        100,                CVAR_GAMECONFIG);
CVAR_UINTF      (g_creepBlockRepathTimeExtra,   50,                 CVAR_GAMECONFIG);

CVAR_STRINGF    (g_waypoint,                    "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_STRINGF    (g_defaultActiveShop,           "",                 CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF      (g_fogofwarSize,                1,                  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (g_fogofwarUpdateTime,          400,                CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF      (sv_chatCounterDecrementInterval,   3500,                   CVAR_SAVECONFIG);
CVAR_UINTF      (sv_chatCounterFloodThreshold,      5,                      CVAR_SAVECONFIG);

CVAR_STRINGF    (g_creepTeam1Melee,     "Creep_LegionMelee",        CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_creepTeam1Ranged,    "Creep_LegionRanged",       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_creepTeam1Siege,     "Creep_LegionSiege",        CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_creepTeam2Melee,     "Creep_HellbourneMelee",    CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_creepTeam2Ranged,    "Creep_HellbourneRanged",   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_creepTeam2Siege,     "Creep_HellbourneSiege",    CVAR_TRANSMIT | CVAR_GAMECONFIG);

//
// Normal
//

CVAR_UINTF      (hero_respawnTimePerLevel,          4000,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldLossPerLevel,             30,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_buyBackCost,                  100,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_buyBackCostPerLevel,          50,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_buyBackCostScale,             1.0f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF      (hero_goldBounty,                   200,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyPerLevel,           5,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyPerStreak,          50,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyMinStreak,          3,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyMaxStreak,          10,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyFirstBlood,         200,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyRadiusBase,         30,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyRadiusPerLevel,     5,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_goldBountyAssistPercent,      0.0f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_expUnsharedBountyPerLevel,    12.0f,          CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_maxLevel,                     25,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
ARRAY_CVAR_FLOATF(hero_experienceTable, _T("0,200,500,900,1400,2000,2700,3500,4400,5400,6500,7700,9000,10400,11900,13500,15200,17000,18900,20900,23000,25200,27500,29900,32400"), CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF     (g_experienceRange,                 1000.0f,        CVAR_GAMECONFIG);
CVAR_BOOLF      (g_towerHeroArmorReduction,         false,          CVAR_TRANSMIT | CVAR_GAMECONFIG);

//
// Casual
//

CVAR_UINTF      (hero_respawnTimePerLevel_Casual,           4000,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldLossPerLevel_Casual,              30,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_buyBackCost_Casual,                   100,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_buyBackCostPerLevel_Casual,           50,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_buyBackCostScale_Casual,              1.0f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF      (hero_goldBounty_Casual,                    200,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyPerLevel_Casual,            5,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyPerStreak_Casual,           50,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyMinStreak_Casual,           3,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyMaxStreak_Casual,           10,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyFirstBlood_Casual,          200,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyRadiusBase_Casual,          30,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_goldBountyRadiusPerLevel_Casual,      5,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_goldBountyAssistPercent_Casual,       0.0f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (hero_expUnsharedBountyPerLevel_Casual,     12.0f,          CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF      (hero_maxLevel_Casual,                      25,             CVAR_TRANSMIT | CVAR_GAMECONFIG);
ARRAY_CVAR_FLOATF(hero_experienceTable_Casual, _T("0,200,500,900,1400,2000,2700,3500,4400,5400,6500,7700,9000,10400,11900,13500,15200,17000,18900,20900,23000,25200,27500,29900,32400"), CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF     (g_experienceRange_Casual,                  1000.0f,        CVAR_GAMECONFIG);
CVAR_BOOLF      (g_towerHeroArmorReduction_Casual,          false,          CVAR_TRANSMIT | CVAR_GAMECONFIG);
//=============================================================================