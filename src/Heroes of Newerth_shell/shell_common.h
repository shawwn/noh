// (C)2005 S2 Games
// shell_common.h
//
// Heroes of Newerth shell
//=============================================================================
#ifndef __SHELL_COMMON_H__
#define __SHELL_COMMON_H__

//=============================================================================
// Settings
//=============================================================================
#include "k2_settings.h"
//=============================================================================

//=============================================================================
// Compile Defines
//=============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
//=============================================================================

//=============================================================================
// Game info strings
//=============================================================================
#define GAME_NAME       _T("Heroes of Newerth")
#define DEFAULT_GAME    _T("game")
#define DEFAULT_EDITOR  _T("game;editor")

#define MAJOR_VERSION   0
#define MINOR_VERSION   0
#define MICRO_VERSION   61
#define HOTFIX_VERSION  0
#include "buildnumber.h"

#define _MAKE_VERSION_STRING(a, b, c, d)    _T(#a)_T(".")_T(#b)_T(".")_T(#c)_T(".")_T(#d)
#define MAKE_VERSION_STRING(a, b, c, d)     _MAKE_VERSION_STRING(a, b, c, d)
#define VERSION_STRING                      MAKE_VERSION_STRING(MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION, HOTFIX_VERSION)

#if 1
#if defined(K2_GARENA)
#define MASTER_SERVER_ADDRESS               "masterserver.garena.s2games.com"
#else
#define MASTER_SERVER_ADDRESS               "masterserver.hon.s2games.com"
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
#define BUILD_ARCH      _T("universal")

#endif
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "../k2/k2_secure_crt.h"
#include "../k2/k2_unicode.h"
//=============================================================================

//=============================================================================
// Standard headers
//=============================================================================
#include <float.h>
#include <time.h>
#include <stdlib.h>
#include <cassert>
//=============================================================================

//=============================================================================
// Common headers
//=============================================================================
#include "../k2/k2_api.h"
#include "../k2/k2_types.h"
#include "../k2/k2_strings.h"
#include "../k2/k2_stl.h"
#include "../k2/k2_endian.h"
#include "../k2/k2_mathlib.h"
#include "../k2/k2_constants.h"
#include "../k2/k2_singleton.h"
#include "../k2/k2_utils.h"

#include "../k2/c_system.h"
#include "../k2/c_memmanager.h"
#include "../k2/i_resourcecommon.h"
#include "../k2/c_console.h"
#include "../k2/c_cvar.h"
#include "../k2/c_cmd.h"
#include "../k2/c_exception.h"
#include "../k2/c_profilemanager.h"
#include "../k2/c_filemanager.h"
#include "../k2/c_host.h"
#include "../k2/c_input.h"
//=============================================================================

#endif // __SHELL_COMMON_H__
