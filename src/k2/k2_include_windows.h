// (C)2010 S2 Games
// k2_include_windows.h
//
//=============================================================================
#ifndef __K2_INCLUDE_WINDOWS_H__
#define __K2_INCLUDE_WINDOWS_H__

#ifdef _WIN32
# define _WIN32_DCOM
# define _WIN32_WINNT 0x0500
# define _WIN32_WINDOWS 0x0410
# define WIN32_LEAN_AND_MEAN
#pragma push_macro("Console")
#undef Console
#include <windows.h>
#pragma pop_macro("Console")
#endif

#endif  //__K2_INCLUDE_WINDOWS_H__

