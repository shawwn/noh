//
// tchar_linux.h
//
//=============================================================================

#ifndef __TCHAR_LINUX_H__
#define __TCHAR_LINUX_H__

#ifdef UNICODE

#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(linux)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#define TCHAR wchar_t
#define _L(x) L ## x
#define _T(x) _L(x)

#define _stscanf swscanf
#define _tcschr wcschr
#define _tcscmp wcscmp
#define _tcsicmp wcscasecmp
#define _tcscpy wcscpy
#define _tcslen wcslen
#define _tcsncmp wcsncmp
#define _tcstol wcstol
#define _tstof(x) wcstof((x), nullptr)
#define _tstoi(x) wcstol((x), nullptr, 0)
#define _tcsncpy wcsncpy
#define _vstprintf vswprintf
#define _vsntprintf vswprintf

#ifndef wcscasecmp
int wcscasecmp(const wchar_t *s1, const wchar_t *s2);
#endif

#else //UNICODE

#define TCHAR char
#define _L(x) L ## x
#define _T(x) x

#define _stscanf sscanf
#define _tcschr strchr
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcscpy strcpy
#define _tcslen strlen
#define _tcsncmp strncmp
#define _tcstol strtol
#define _tstof atof
#define _tstoi atoi
#define _tcsncpy strncpy
#define _vstprintf vsprintf
#define _vsntprintf vsnprintf
    
#endif //UNICODE

// the following do not have wide char versions, use TStringToNative(...) to convert the string args
#define _taccess access
#define _tremove remove
#define _trename rename
#define _tstat stat
#define _tmkdir mkdir
#define _trmdir rmdir
#define _tfopen fopen
#define _topen open

//=============================================================================
#endif //__TCHAR_LINUX_H__
