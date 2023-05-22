// (C)2005 S2 Games
// product_settings.h
//=============================================================================
#ifndef __PRODUCT_SETTINGS_H__
#define __PRODUCT_SETTINGS_H__

//=============================================================================
// Settings
//=============================================================================
#include "../k2/k2_settings.h"
//=============================================================================

//=============================================================================
// Game info strings
//=============================================================================
#ifndef GAME_NAME
#error Expected GAME_NAME to be defined
#endif
#ifndef GAME_MODS
#error Expected GAME_MODS to be defined
#endif
#ifndef GAME_COMPONENT
#define GAME_TITLE GAME_NAME
#else
#define GAME_TITLE GAME_NAME " " GAME_COMPONENT
#endif

#if !defined(MAJOR_VERSION) || !defined(MINOR_VERSION) || !defined(MICRO_VERSION) || !defined(HOTFIX_VERSION)
#error Expected MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION, and HOTFIX_VERSION to be defined as integers
#endif

#define _MAKE_VERSION_STRING(a, b, c, d)    _T(#a) _T(".") _T(#b) _T(".") _T(#c) _T(".") _T(#d)
#define MAKE_VERSION_STRING(a, b, c, d)     _MAKE_VERSION_STRING(a, b, c, d)
#define VERSION_STRING                      MAKE_VERSION_STRING(MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION, HOTFIX_VERSION)

#if 1
#if defined(K2_GARENA)
#define MASTER_SERVER_ADDRESS               "masterserver.garena.s2games.com"
#else
//#define MASTER_SERVER_ADDRESS               "masterserver.hon.s2games.com"
//#define MASTER_SERVER_ADDRESS               "api.kongor.online"
#define MASTER_SERVER_ADDRESS               "127.0.0.1"
#endif
#else
#if defined(K2_GARENA)
#define MASTER_SERVER_ADDRESS               "masterserver.garenatest.s2games.com"
#else
#define MASTER_SERVER_ADDRESS               "masterserver.hontest.s2games.com"
#endif
#endif

// BUILD_ARCH
// shared == All architectures
// 10 Character MAX

#if defined(_WIN32)

#if defined(K2_GARENA)
    #if defined(K2_TEST)
        #if defined(K2_SERVER)
#define BUILD_OS        _T("wns")
#define BUILD_OS_CODE   _T("wns-PhI43roaQLAxl3S5oumI")
        #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wnc")
        #else
#define BUILD_OS        _T("wn1")
        #endif
    #else
        #if defined(K2_SERVER)
#define BUILD_OS        _T("wgs")
#define BUILD_OS_CODE   _T("wgs-wi3BRLEsluRlUprieYIa")
        #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wgc")
        #else
#define BUILD_OS        _T("wg1")
        #endif
    #endif
#elif defined(K2_TEST)
    #if defined(K2_SERVER)
#define BUILD_OS        _T("wts")
#define BUILD_OS_CODE   _T("wts-laqouwlutrl9frOavlub")
    #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wtc")
#define BUILD_OS_CODE   _T("wtc-vou2iajLABiu")
    #else
#define BUILD_OS        _T("wt1")
    #endif
#elif defined(K2_BALANCE_TEST)
    #if defined(K2_SERVER)
#define BUILD_OS        _T("wbs")
#define BUILD_OS_CODE   _T("wbs-nudaS4Wuruta6apheStE")
    #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wbc")
#define BUILD_OS_CODE   _T("wbc-v6yUdre3emaS")
    #else
#define BUILD_OS        _T("wb1")
    #endif
#elif defined(K2_EXPERIMENTAL)
    #if defined(K2_SERVER)
#define BUILD_OS        _T("wxs")
#define BUILD_OS_CODE   _T("wxs-rIaniuwLatrlavies4l1")
    #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wxc")
#define BUILD_OS_CODE   _T("wxc-c50sTO4rieST")
    #else
#define BUILD_OS        _T("wx1")
    #endif
#else
    #if defined(K2_SERVER)
#define BUILD_OS        _T("was")
#define BUILD_OS_CODE   _T("was-crIac6LASwoafrl8FrOa")
    #elif defined(K2_CLIENT)
#define BUILD_OS        _T("wac")
    #else
#define BUILD_OS        _T("wa1")
    #endif
#endif

#define BUILD_ARCH      _T("i686")

#elif defined(linux)

#if defined(K2_SERVER)
#define BUILD_OS        _T("las")
#elif defined(K2_CLIENT)
#define BUILD_OS        _T("lac")
#else
#define BUILD_OS        _T("la1")
#endif
#define BUILD_ARCH      _T("x86-biarch")

#elif defined(__APPLE__)

#if defined(K2_SERVER)
#define BUILD_OS        _T("mas")
#elif defined(K2_CLIENT)
#define BUILD_OS        _T("mac")
#else
#define BUILD_OS        _T("ma1")
#endif
#define BUILD_ARCH      _T("arm64")

#endif

#ifndef BUILD_OS_CODE
#define BUILD_OS_INFO BUILD_OS
#else
#define BUILD_OS_INFO BUILD_OS_CODE
#endif
//=============================================================================

#endif // __PRODUCT_SETTINGS_H__
