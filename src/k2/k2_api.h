// (C)2005 S2 Games
// k2_api.h
//
// Contains all functions exported from the k2 dll
//=============================================================================
#ifndef __K2_API_H__
#define __K2_API_H__

//=============================================================================
// Linking characteristics
//=============================================================================
#ifdef K2_DLL

#if defined(WIN32)
#define K2_DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define K2_DLL_EXPORT extern "C" __attribute__ ((visibility("default")))
#endif

#ifdef __GNUC__
#define K2_API __attribute__ ((visibility("default")))
#ifdef K2_EXPORTS
#define K2_EXTERN
#else
#define K2_EXTERN extern
#endif
#else
#define K2_DLL_EXPORT
#ifdef K2_EXPORTS
#define K2_API  __declspec(dllexport)
#define K2_EXTERN
#else
#define K2_API  __declspec(dllimport)
#define K2_EXTERN extern
#endif
#endif

#elif K2_LIB

#define K2_API
#define K2_EXTERN

#else

#error Must define either K2_DLL or K2_LIB

#endif
//=============================================================================

#endif //__K2_API_H__
