// (C)2006 S2 Games
// k2_secure_crt.h
//
// Definitions to enable the use of the new "secure crt" functions
// in the Visual Studio 2005 standard library
//=============================================================================
#ifndef __K2_SECURE_CRT_H__
#define __K2_SECURE_CRT_H__

//=============================================================================
// Definitions
//=============================================================================

#ifdef USE_SECURE_CRT
#ifndef _WIN32
#error USE_SECURE_CRT is only valid for _WIN32 builds
#endif // _WIN32

#pragma message ("*** Secure CRT functions are enabled ***")
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1

#define DECLARE_TIME_STRUCT_S(name)				struct tm	_##name, *##name(&_##name);
#define LOCALTIME_S(time, time_struct)			(localtime_s(time_struct, time) == 0)

#define STRCPY_S(dest, size, src)				(strcpy_s(dest, size, src) == 0)

#define _TCSNCPY_S(dest, size, src, count)		(_tcsncpy_s(dest, size, src, count) == 0)
#define STRNCPY_S(dest, size, src, count)		(strncpy_s(dest, size, src, count) == 0)
#define WCSNCPY_S(dest, size, src, count)		(wcsncpy_s(dest, size, src, count) == 0)

#define MEMCPY_S(dest, size, src, count)		(memcpy_s(dest, size, src, count) == 0)

#define DECLARE_TCSERROR_S_BUFFER(name, size)	TCHAR name[size];
#define _TCSERROR_S(buffer, size, errno)		(_tcserror_s(buffer, size, errno) == 0)

#define _TFOPEN_S(file, filename, mode)			(_tfopen_s(&file, filename, mode) == 0)

#define _STSCANF_S_BEGIN(buffer, fmt)			_stscanf_s(buffer, fmt,
#define _STSCANF_S_END							)

#define	WCTOMB_S(count, buffer, size, in)		wctomb_s(&(count), (buffer), (size), (in))

#else	// USE_SECURE_CRT

#ifdef _WIN32
#pragma message ("*** Secure CRT functions are disabled ***")
#pragma warning (disable: 4996)	// Warnings about depricated functions
#endif // _WIN32

#ifndef errno_t
typedef int errno_t;
#endif

#define DECLARE_TIME_STRUCT_S(name)				struct tm	*name(NULL);
#define LOCALTIME_S(time, time_struct)			((time_struct = localtime(time)) != NULL)

#define STRCPY_S(dest, size, src)				(strncpy(dest, src, size) != NULL)

#define _TRUNCATE
#define _TCSNCPY_S(dest, size, src, count)		(_tcsncpy(dest, src, size) != NULL)
#define STRNCPY_S(dest, size, src, count)		(strncpy(dest, src, size) != NULL)
#define WCSNCPY_S(dest, size, src, count)		(wcsncpy(dest, src, size) != NULL)

#define MEMCPY_S(dest, size, src, count)		(memcpy(dest, src, count) != dest)

#define DECLARE_TCSERROR_S_BUFFER(name, size)	TCHAR *name(NULL);
#define _TCSERROR_S(buffer, size, errno)		((buffer = _tcserror(errno)) != NULL)

#define _TFOPEN_S(file, filename, mode)			((file = _tfopen(filename, mode)) != NULL)

#define _STSCANF_S_BEGIN(buffer, fmt)			_stscanf(buffer, fmt,
#define _STSCANF_S_END							)

#define	WCTOMB_S(count, buffer, size, in)		(count = wctomb((buffer), (in)))

#endif	// USE_SECURE_CRT
//=============================================================================

#endif //__K2_SECURE_CRT_H__
