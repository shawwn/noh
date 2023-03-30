// (C)2010 S2 Games
// k2_platform.h
//
//=============================================================================
#ifndef __K2_PLATFORM_H__
#define __K2_PLATFORM_H__

//=============================================================================
// CPU
//=============================================================================
#if _WIN32||_WIN64
#	if defined(_M_AMD64)
#		define K2_CPU_X86_64			1
#	elif defined(_M_IX86)
#		define K2_CPU_X86_32			1
#	elif defined(_M_IA64)
#		define K2_CPU_INTEL_64			1
#	elif _XBOX
#		define K2_CPU_POWERPC			1
#	endif
#else
#   if __x86_64__
#       define K2_CPU_X86_64			1
#   elif __i386__||__i386
#       define K2_CPU_X86_32			1
#   elif __ia64__
#       define K2_CPU_INTEL_64			1
#	elif __powerpc__
#		define K2_CPU_POWERPC			1
#   else
#		error K2_CPU architecture
#   endif
#endif
//=============================================================================

//=============================================================================
// Platform
//=============================================================================
#if _WIN32||_WIN64

//	Windows
#define K2_OS_WINDOWS				1
#if _WIN32
#	define K2_OS_WIN32				1
#elif _WIN64
#	define K2_OS_WIN64				1
#elif _XBOX
#	define K2_OS_XBOX				1
#endif

#if defined(_M_IX86)
#define K2_ARCH_WIN_IA32			1
#elif defined(_M_AMD64) 
#define K2_ARCH_WIN_INTEL64			1
#elif _XBOX 
#define K2_ARCH_XBOX_PPC			1
#endif

#elif __linux__ || _FreeBSD__

// Linux
#define K2_OS_LINUX					1
#if __i386__
#define K2_ARCH_LINUX_IA32			1
#elif __ia64__
#define K2_ARCH_LINUX_IA64			1
#elif __x86_64__
#define K2_ARCH_LINUX_INTEL64		1
#elif __powerpc__
#define K2_ARCH_MAC_POWERPC			1
#endif

#elif __APPLE__

// Mac
#define K2_OS_MAC					1
#if __i386__
#define K2_ARCH_LINUX_IA32			1
#elif __x86_64__
#define K2_ARCH_LINUX_INTEL64		1
#elif __POWERPC__
#define K2_ARCH_MAC_POWERPC			1
#endif

#endif
//=============================================================================

#endif // __K2_PLATFORM_H__
