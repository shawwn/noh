// (C)2005 S2 Games
// k2_unicode.h
//
// This file sets up all the unicode definitions necessary for all modules of the game
//=============================================================================
#ifndef __K2_UNICODE_H__
#define __K2_UNICODE_H__

//=============================================================================
// Definitions
//=============================================================================
//#define UNICODE

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif


#ifdef _UNICODE

#ifndef UNICODE
#define UNICODE
#endif

#ifndef __GNUC__
#pragma message("*** Unicode is enabled ***")
#endif
#define WideToTCHAR(out, size, in, len)     WCSNCPY_S(out, size, in, len)
#define TCHARToWide(out, size, in, len)     WCSNCPY_S(out, sizem in, len)
#define SingleToTCHAR(out, size, in, len)   SingleToWide(out, in, len)
#define TCHARToSingle(out, size, in, len)   WideToSingle(out, in, len)

#define TStringToString(in)         WideToSingle(in)
#define StringToTString(in)         SingleToWide(in)
#define WStringToTString(in)        (in)
#define TStringToWString(in)        (in)

#define StrToTString(out, in)       { size_t z(strlen(in) + 1); TCHAR *pBuffer(K2_NEW_ARRAY(ctx_FileSystem, TCHAR, z)); SingleToWide(pBuffer, (in), z); (out) = pBuffer; SAFE_DELETE_ARRAY(pBuffer); }

#ifdef _WIN32
// system libraries use wchar_t versions
#define NativeToTString(in) (in)
#define TStringToNative(in) (in)
#define _TNative(x) _T(x)
#elif defined(linux)
// system libraries use multi-byte character strings in the current locale's charset
// wchar_t strings are UCS-4 when using mbstowcs/wcstombs conversion functions
#define NativeToTString(in) MBSToWCS(in)
#define TStringToNative(in) WCSToMBS(in)
#define _TNative(x) x
#elif defined(__APPLE__)
// system libraries use utf-8
// wchar_t strings have no specific encoding when using mbstowcs/wcstombs conversion functions
#define NativeToTString(in) UTF8ToWString(in)
#define TStringToNative(in) WStringToUTF8(in)
#define _TNative(x) x
#endif

#else //_UNICODE

#ifndef __GNUC__
#pragma message("*** Unicode is disabled ***")
#endif
#define WideToTCHAR(out, size, in, len)     WideToSingle(out, in, len)
#define TCHARToWide(out, size, in, len)     SingleToWide(out, in, len)
#define SingleToTCHAR(out, size, in, len)   STRNCPY_S(out, size, in, len)
#define TCHARToSingle(out, size, in, len)   STRNCPY_S(out, size, in, len)

#define TStringToString(in)         (in)
#define StringToTString(in)         (in)
#define WStringToTString(in)        WideToSingle(in)
#define TStringToWString(in)        SingleToWide(in)

#define StrToTString(out, in)       { if ((in) != nullptr) (out) = (in); }

#define NativeToTString(in) (in)
#define TStringToNative(in) (in)
#define _TNative(x) x

#endif //_UNICODE

#ifdef UNICODE
#define XtoA            XtoW
#define FormatTime      FormatTimeW
#define UTF8ToTString   UTF8ToWString
#define TStringToUTF8   WStringToUTF8
#else
#define XtoA            XtoS
#define FormatTime      FormatTimeS
#define UTF8ToTString   UTF8ToString
#define TStringToUTF8   StringToUTF8
#endif

#ifdef _WIN32
#include <tchar.h>
#else
#if defined(__APPLE__) 
#include <AvailabilityMacros.h>
#if (MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_3)
#include <ctype.h> // 10.3.9 sdk defines _T
#undef _T
#endif
#endif
#include "tchar_linux.h"
#endif
#include <string>
#include <fstream>

#ifdef _WIN32
#define tfopen( path, mode )        _wfopen( path, _T( mode ) )
#else
#define tfopen( path, mode )        fopen( path, mode )
#endif
//=============================================================================

#endif //__K2_UNICODE_H__
