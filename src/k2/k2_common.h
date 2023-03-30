// (C)2005 S2 Games
// k2_common.h
//
// Precompiled header file for the k2 module
// *ONLY* add things here that are used in *ALL* (or at least almost all) of
// the k2 source files!
//
// This is also a good place for headers external to the game that we won't
// ever be altering, especially if their compile time is long, like STL.
//=============================================================================
#ifndef __K2_COMMON_H__
#define __K2_COMMON_H__

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
#define _CRT_RAND_S
//=============================================================================

//=============================================================================
// Unicode
//
// This comes before everything else, because it affects a lot of the other
// libraries initialization
//=============================================================================
#include "k2_secure_crt.h"
#include "k2_unicode.h"
//=============================================================================

//=============================================================================
// Standard headers
//=============================================================================
#include <float.h>
#include <errno.h>
#include <cassert>
#include <stdlib.h>
//=============================================================================

//=============================================================================
// External Libraries
//=============================================================================
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <ft2build.h>
#include <freetype/freetype.h>
#ifdef K2_STEAM
#include <Steam.h>
#include <SteamCommon.h>
#endif
//=============================================================================

//=============================================================================
// K2 modules
//=============================================================================
#include "k2_api.h"
#include "k2_types.h"
#include "k2_strings.h"
#include "k2_stl.h"
#include "k2_endian.h"
#include "k2_mathlib.h"
#include "k2_constants.h"
#include "k2_singleton.h"
#include "k2_utils.h"

#include "c_exception.h"
#include "c_memmanager.h"
#include "c_system.h"
#include "c_console.h"
#include "c_cvar.h"
#include "c_cvararray.h"
#include "c_cvarreference.h"
#include "c_profilemanager.h"
#include "c_filemanager.h"
#include "c_filehandle.h"
#include "c_cmd.h"
#include "c_cmdprecache.h"
#include "i_resourcecommon.h"
#include "c_host.h"
#include "c_timermanager.h"
#include "c_buffer.h"
#include "stringutils.h"
#include "xtoa.h"
//=============================================================================
#endif //__K2_COMMON_H__
