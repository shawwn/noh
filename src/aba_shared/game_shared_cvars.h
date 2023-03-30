// (C)2008 S2 Games
// game_shared_cvars.h
//
//=============================================================================
#ifndef __GAME_SHARED_CVARS_H__
#define __GAME_SHARED_CVARS_H__

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_UINT	(g_corpseTime);
EXTERN_CVAR_FLOAT	(g_transferRange);
EXTERN_CVAR_FLOAT	(g_touchRange);
EXTERN_CVAR_FLOAT	(g_blockSlope);

EXTERN_CVAR_UINT	(g_unitBlockRepathTime);
EXTERN_CVAR_UINT	(g_unitBlockRepathTimeExtra);
EXTERN_CVAR_FLOAT	(g_experienceRange);
EXTERN_CVAR_FLOAT	(g_unitMoveAngle);
EXTERN_CVAR_FLOAT	(g_unitActionAngle);
EXTERN_CVAR_BOOL	(g_unitActionOnTurn);
EXTERN_CVAR_FLOAT	(g_pathPad);
EXTERN_CVAR_FLOAT	(g_threatRange);
EXTERN_CVAR_STRING	(g_heroLevelupEffectPath);
EXTERN_CVAR_FLOAT	(g_unitMoveSpeedMin);
EXTERN_CVAR_FLOAT	(g_unitMoveSpeedMax);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_unitSelectionIndicatorPath);
EXTERN_CVAR_UINT	(g_unitGuardChaseTime);
EXTERN_CVAR_FLOAT	(g_unitGuardDistance);
EXTERN_CVAR_UINT	(g_unitGuardReaggroChaseTime);
EXTERN_CVAR_FLOAT	(g_unitGuardReaggroDistance);
EXTERN_CVAR_UINT	(g_unitBehaviorStartAggroDelay);
GAME_SHARED_API EXTERN_CVAR_UINT	(g_unitAggressionSightTime);
EXTERN_CVAR_BOOL	(g_unitAttackMoveDAC);
EXTERN_CVAR_UINT	(g_unitAttackAggroTriggerRange);

EXTERN_CVAR_STRING	(g_effectStunPath);
EXTERN_CVAR_STRING	(g_effectRecipePath);

GAME_SHARED_API EXTERN_ARRAY_CVAR_STRING	(g_powerups);
GAME_SHARED_API EXTERN_ARRAY_CVAR_STRING	(g_critters);
GAME_SHARED_API EXTERN_CVAR_FLOAT		(g_occlusionHeight);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_waypoint);

EXTERN_CVAR_UINT	(g_multiKillTime);

EXTERN_CVAR_FLOAT	(g_heroAttackAggroRange);
EXTERN_CVAR_UINT	(g_heroAttackAggroTime);
EXTERN_CVAR_UINT	(g_heroAttackAggroDelay);
EXTERN_CVAR_BOOL	(g_heroAttackReaggroBlock);

EXTERN_CVAR_UINT	(g_creepBlockRepathTime);
EXTERN_CVAR_UINT	(g_creepBlockRepathTimeExtra);

GAME_SHARED_API EXTERN_CVAR_STRING	(g_defaultActiveShop);
GAME_SHARED_API EXTERN_CVAR_UINT	(g_fogofwarSize);
GAME_SHARED_API EXTERN_CVAR_UINT	(g_fogofwarUpdateTime);
GAME_SHARED_API EXTERN_CVAR_UINT	(g_fogofwarStyle);

GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterDecrementInterval);
GAME_SHARED_API EXTERN_CVAR_UINT(sv_chatCounterFloodThreshold);

GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam1Melee);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam1Ranged);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam1Siege);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam2Melee);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam2Ranged);
GAME_SHARED_API EXTERN_CVAR_STRING	(g_creepTeam2Siege);
//=============================================================================

#endif //__GAME_SHARED_CVARS_H__
