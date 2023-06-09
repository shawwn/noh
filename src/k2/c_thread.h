// (C)2010 S2 Games
// c_thread.h
//=============================================================================
#ifndef __C_THREAD_H__
#define __C_THREAD_H__

//=============================================================================
// Headers
//=============================================================================
#include <errno.h>

#ifdef _WIN32
# include "c_thread_win32.h"
#else
# include "c_thread_posix.h"
#endif
//=============================================================================

#endif // __C_THREAD_H__
