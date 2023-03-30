// (C)2008 S2 Games
// game_shared_cvars.h
//
//=============================================================================
#ifndef __GAME_SHARED_CVARS_H__
#define __GAME_SHARED_CVARS_H__

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_UINT(g_corpseTime);
EXTERN_CVAR_FLOAT(g_transferRange);
EXTERN_CVAR_FLOAT(g_touchRange);
EXTERN_CVAR_FLOAT(g_blockSlope);

EXTERN_CVAR_UINT(g_unitBlockRepathTime);
EXTERN_CVAR_UINT(g_unitBlockRepathTimeExtra);
EXTERN_CVAR_FLOAT(g_unitMoveAngle);
EXTERN_CVAR_FLOAT(g_unitActionAngle);
EXTERN_CVAR_BOOL(g_unitActionOnTurn);
EXTERN_CVAR_FLOAT(g_pathPad);
EXTERN_CVAR_FLOAT(g_threatRange);
EXTERN_CVAR_STRING(g_heroLevelupEffectPath);
EXTERN_CVAR_FLOAT(g_unitMoveSpeedMin);
EXTERN_CVAR_FLOAT(g_unitMoveSpeedMax);
GAME_SHARED_API EXTERN_CVAR_STRING(g_unitSelectionIndicatorPath);
EXTERN_CVAR_UINT(g_unitGuardChaseTime);
EXTERN_CVAR_FLOAT(g_unitGuardDistance);
EXTERN_CVAR_UINT(g_unitGuardReaggroChaseTime);
EXTERN_CVAR_FLOAT(g_unitGuardReaggroDistance);
EXTERN_CVAR_UINT(g_unitBehaviorStartAggroDelay);
GAME_SHARED_API EXTERN_CVAR_UINT(g_unitAggressionSightTime);
EXTERN_CVAR_BOOL(g_unitAttackMoveDAC);
EXTERN_CVAR_UINT(g_unitAttackAggroTriggerRange);

EXTERN_CVAR_STRING(g_effectStunPath);
EXTERN_CVAR_STRING(g_effectRecipePath);

GAME_SHARED_API EXTERN_ARRAY_CVAR_STRING(g_powerups);
GAME_SHARED_API EXTERN_ARRAY_CVAR_STRING(g_critters);
GAME_SHARED_API EXTERN_CVAR_FLOAT(g_occlusionHeight);
GAME_SHARED_API EXTERN_CVAR_STRING(g_waypoint);

EXTERN_CVAR_UINT(g_multiKillTime);

EXTERN_CVAR_FLOAT(g_heroAttackAggroRange);
EXTERN_CVAR_UINT(g_heroAttackAggroTime);
EXTERN_CVAR_UINT(g_heroAttackAggroDelay);
EXTERN_CVAR_BOOL(g_heroAttackReaggroBlock);

EXTERN_CVAR_UINT(g_creepBlockRepathTime);
EXTERN_CVAR_UINT(g_creepBlockRepathTimeExtra);

GAME_SHARED_API EXTERN_CVAR_STRING(g_defaultActiveShop);
GAME_SHARED_API EXTERN_CVAR_UINT(g_fogofwarSize);
GAME_SHARED_API EXTERN_CVAR_UINT(g_fogofwarUpdateTime);

GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterDecrementInterval);
GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterFloodThreshold);

GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam1Melee);
GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam1Ranged);
GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam1Siege);
GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam2Melee);
GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam2Ranged);
GAME_SHARED_API EXTERN_CVAR_STRING(g_creepTeam2Siege);

GAME_SHARED_API EXTERN_CVAR_UINT(hero_respawnTimePerLevel);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldLossPerLevel);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCost);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCostPerLevel);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_buyBackCostScale);

GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBounty);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyPerLevel);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyPerStreak);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyMinStreak);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyMaxStreak);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyFirstBlood);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusBase);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusPerLevel);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_goldBountyAssistPercent);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_expUnsharedBountyPerLevel);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_maxLevel);
GAME_SHARED_API EXTERN_ARRAY_CVAR_FLOAT(hero_experienceTable);

GAME_SHARED_API EXTERN_CVAR_FLOAT(g_experienceRange);
GAME_SHARED_API EXTERN_CVAR_BOOL(g_towerHeroArmorReduction);

GAME_SHARED_API EXTERN_CVAR_UINT(hero_respawnTimePerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldLossPerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCost_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCostPerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_buyBackCostScale_Casual);

GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBounty_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyPerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyPerStreak_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyMinStreak_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyMaxStreak_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyFirstBlood_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusBase_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusPerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_goldBountyAssistPercent_Casual);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_expUnsharedBountyPerLevel_Casual);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_maxLevel_Casual);
GAME_SHARED_API EXTERN_ARRAY_CVAR_FLOAT(hero_experienceTable_Casual);

GAME_SHARED_API EXTERN_CVAR_FLOAT(g_experienceRange_Casual);
GAME_SHARED_API EXTERN_CVAR_BOOL(g_towerHeroArmorReduction_Casual);
//=============================================================================

#endif //__GAME_SHARED_CVARS_H__
