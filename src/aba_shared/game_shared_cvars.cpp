// (C)2008 S2 Games
// game_shared_cvars.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "game_shared_cvars.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTF		(g_corpseTime,					SecToMs(20u),		CVAR_GAMECONFIG);
CVAR_FLOATF		(g_transferRange,				64.0f,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_touchRange,					64.0f,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_blockSlope,					0.6f,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_experienceRange,				1000.0f,			CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitMoveAngle,				120.0f,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitActionAngle,				90.0f,				CVAR_GAMECONFIG);
CVAR_BOOLF		(g_unitActionOnTurn,			false,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_pathPad,						4.0f,				CVAR_GAMECONFIG);
CVAR_STRINGF	(g_heroLevelupEffectPath,		"",					CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitMoveSpeedMin,			100.0f,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitMoveSpeedMax,			522.0f,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_unitSelectionIndicatorPath,	"",					CVAR_TRANSMIT | CVAR_GAMECONFIG); 
CVAR_UINTF		(g_unitAggressionSightTime,		3000,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF		(g_unitGuardChaseTime,			7000,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitGuardDistance,			750.0f,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_unitGuardReaggroChaseTime,	3000,				CVAR_GAMECONFIG);
CVAR_FLOATF		(g_unitGuardReaggroDistance,	500.0f,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_unitBehaviorStartAggroDelay,	50,					CVAR_GAMECONFIG);
CVAR_BOOLF		(g_unitAttackMoveDAC,			true,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_unitAttackAggroTriggerRange,	700,				CVAR_GAMECONFIG);

CVAR_STRINGF	(g_effectStunPath,				"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_effectRecipePath,			"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);

ARRAY_CVAR_STRINGF	(g_powerups,				"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);
ARRAY_CVAR_STRINGF	(g_critters,				"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF		(g_occlusionHeight,				64.0f,				CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF		(g_multiKillTime,				SecToMs(13.5f),		CVAR_GAMECONFIG);

CVAR_FLOATF		(g_heroAttackAggroRange,		500.0f,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_heroAttackAggroTime,			2000u,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_heroAttackAggroDelay,		0u,					CVAR_GAMECONFIG);
CVAR_BOOLF		(g_heroAttackReaggroBlock,		false,				CVAR_GAMECONFIG);

CVAR_UINTF		(g_creepBlockRepathTime,		100,				CVAR_GAMECONFIG);
CVAR_UINTF		(g_creepBlockRepathTimeExtra,	50,					CVAR_GAMECONFIG);

CVAR_STRINGF	(g_waypoint,					"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_STRINGF	(g_defaultActiveShop,			"",					CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF		(g_fogofwarSize,				1,					CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF		(g_fogofwarUpdateTime,			400,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF		(g_fogofwarStyle,				0,					CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF		(sv_chatCounterDecrementInterval,	3500,					CVAR_SAVECONFIG);
CVAR_UINTF		(sv_chatCounterFloodThreshold,		5,						CVAR_SAVECONFIG);

CVAR_STRINGF	(g_creepTeam1Melee,		"Creep_LegionMelee",		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_creepTeam1Ranged,	"Creep_LegionRanged",		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_creepTeam1Siege,		"Creep_LegionSiege",		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_creepTeam2Melee,		"Creep_HellbourneMelee",	CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_creepTeam2Ranged,	"Creep_HellbourneRanged",	CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF	(g_creepTeam2Siege,		"Creep_HellbourneSiege",	CVAR_TRANSMIT | CVAR_GAMECONFIG);
//=============================================================================