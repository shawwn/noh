// (C)2006 S2 Games
// k2_settings.h
//
//=============================================================================
#ifndef __K2_SETTINGS_H__
#define __K2_SETTINGS_H__

//=============================================================================
// Definitions
//=============================================================================
#define UNICODE
//#define K2_PROFILE

#ifdef _WIN32
#define USE_SECURE_CRT

#ifndef _DEBUG
#define _SECURE_SCL 0
#endif

#endif
//#define K2_NOSOUND

#define TERRAIN_OCCLUSION	0
const bool REQUIRE_AUTHENTICATION(true);

//#define K2_PATHFINDING_RESPECTS_TEAM_VISION

//#define K2_GARENA

// Uncomment only one of K2_SERVER or K2_CLIENT, both commented out means the full client
// GARENA + TEST is valid
//#define K2_SERVER
//#define K2_CLIENT
//#define K2_TEST
//#define K2_EXPERIMENTAL
//#define K2_BALANCE_TEST
//=============================================================================

#ifdef UNICODE
#define UNICODE_STRING _T(" [UNICODE]")
#else
#define UNICODE_STRING _T("")
#endif

#ifdef K2_PROFILE
#define PROFILER_STRING _T(" [PROFILER]")
#else
#define PROFILER_STRING _T("")
#endif

#ifdef USE_SECURE_CRT
#define	SECURE_CRT_STRING _T(" [SECURE CRT]")
#else
#define	SECURE_CRT_STRING _T("")
#endif

#ifdef _DEBUG
#define DEBUG_STRING _T(" [DEBUG]")
#else
#define DEBUG_STRING _T("")
#endif

#ifdef K2_NOSOUND
#define NOSOUND_STRING _T(" [NO SOUND]")
#else
#define NOSOUND_STRING _T("")
#endif

#ifdef K2_GARENA
	#ifdef K2_TEST
#define GARENA_STRING _T(" [GARENA TEST]")
	#else
#define GARENA_STRING _T(" [GARENA]")
	#endif
#else
#define GARENA_STRING _T("")
#endif

#define BUILD_INFO_STRING GARENA_STRING DEBUG_STRING UNICODE_STRING PROFILER_STRING SECURE_CRT_STRING NOSOUND_STRING
//=============================================================================

#endif //__K2_SETTINGS_H__
