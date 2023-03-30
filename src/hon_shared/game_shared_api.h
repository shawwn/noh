// (C)2005 S2 Games
// game_shared_api.h
//
//=============================================================================
#ifndef __GAME_SHARED_API_H__
#define __GAME_SHARED_API_H__

//=============================================================================
// Definitions
//=============================================================================
#ifdef GAME_SHARED_DLL

#ifdef __GNUC__
#define GAME_SHARED_API __attribute__ ((visibility("default")))
#ifdef GAME_SHARED_EXPORTS
#define GAME_SHARED_EXTERN
#else
#define GAME_SHARED_EXTERN extern
#endif
#else
#ifdef GAME_SHARED_EXPORTS
#define GAME_SHARED_API __declspec(dllexport)
#define GAME_SHARED_EXTERN
#else
#define GAME_SHARED_API __declspec(dllimport)
#define GAME_SHARED_EXTERN extern
#endif
#endif

#elif GAME_SHARED_LIB

#define GAME_SHARED_API
#define GAME_SHARED_EXTERN

#else

#error Must define either GAME_SHARED_DLL or GAME_SHARED_LIB

#endif
//=============================================================================

#endif //__GAME_SHARED_API_H__
