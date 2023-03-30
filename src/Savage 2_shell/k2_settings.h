// (C)2006 S2 Games
// k2_settings.h
//
//=============================================================================
#ifndef __K2_SETTINGS_H__
#define __K2_SETTINGS_H__

//=============================================================================
// Definitions
//=============================================================================
//#define UNICODE
//#define K2_PROFILE
#ifdef _WIN32
#define USE_SECURE_CRT
#endif
//#define K2_NOSOUND
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
#define SECURE_CRT_STRING _T(" [SECURE CRT]")
#else
#define SECURE_CRT_STRING _T("")
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

#define BUILD_INFO_STRING DEBUG_STRING UNICODE_STRING PROFILER_STRING SECURE_CRT_STRING NOSOUND_STRING
//=============================================================================

#endif //__K2_SETTINGS_H__
