// (C)2010 S2 Games
// c_thread_win32.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//
//  - From CreateThread Platform SDK Documentation:
//
//  "A thread that uses functions from the static C run-time 
//    libraries should use the beginthread and endthread C run-time
//    functions for thread management rather than CreateThread and
//    ExitThread. Failure to do so results in small memory leaks
//    when ExitThread is called. Note that this is not a problem
//    with the C run-time in a DLL."
//
//  With regards to this, I have decided to use the CreateThread
//  API, unless you define K2_USING_CRT in which case there are two
//  possibilities:
//
//    1. Define K2_USE_BEGINTHREAD: Uses _beginthread/_endthread
//        (said to be *unreliable* in the SDK docs)
//
//    2. Don't - Uses _beginthreaded/_endthreadex
//
//  A note about _endthread:
//
//    It will call CloseHandle() on exit, and if it was already
//    closed then you will get an exception. To prevent this, I
//    removed the CloseHandle() functionality - this means that
//    a Join() WILL wait on a Detach()'ed thread.
//=============================================================================
#ifndef __C_THREAD_WIN32_H__
#define __C_THREAD_WIN32_H__
#ifdef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include "k2_include_windows.h"
#include "c_semaphore.h"
#include "c_mutex.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
#ifdef K2_USING_CRT
# include <process.h>
# ifdef K2_USE_BEGINTHREAD
#  define K2_THREAD_CALL                __cdecl
#  define K2_THREAD_HANDLE              uintptr_t
#  define K2_THREAD_RET_T               void
#  define K2_CREATE_THREAD_FAILED       (-1L)
#  define K2_CREATE_THREAD_ERROR        (errno)
#  define K2_CREATE_THREAD(_S,_F,_P)    ((Handle)_beginthread((void (__cdecl *)(void *))_F,_S,(void *)_P))
#  define K2_EXIT_THREAD                _endthread()
#  define K2_CLOSE_HANDLE(x)            1
#  define K2_THREAD_RETURN(x)           return
# else
#  define K2_THREAD_CALL                WINAPI
#  define K2_THREAD_HANDLE              HANDLE
#  define K2_THREAD_RET_T               UINT
#  define K2_CREATE_THREAD_FAILED       (0L)
#  define K2_CREATE_THREAD_ERROR        (errno)
#  define K2_CREATE_THREAD(_S,_F,_P)    ((Handle)_beginthreadex(0,_S,(UINT (WINAPI *)(void *))_F,(void *)_P,0,0))
#  define K2_EXIT_THREAD                _endthreadex(0)
#  define K2_CLOSE_HANDLE(x)            CloseHandle(x)
#  define K2_THREAD_RETURN(x)           return(x)
# endif
#else
# define K2_THREAD_CALL                WINAPI
# define K2_THREAD_HANDLE              HANDLE
# define K2_THREAD_RET_T               DWORD
# define K2_CREATE_THREAD_FAILED       (0L)
# define K2_CREATE_THREAD_ERROR        GetLastError()
# define K2_CREATE_THREAD(_S,_F,_P)    ((Handle)CreateThread(0,_S,(DWORD (WINAPI *)(void *))_F,(void *)_P,0,0))
# define K2_EXIT_THREAD                ExitThread(0)
# define K2_CLOSE_HANDLE(x)            CloseHandle(x)
# define K2_THREAD_RETURN(x)           return(x)
#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint K2_INVALID_THREAD(0);
//=============================================================================

//=============================================================================
// CK2Thread<ThreadParam>
//=============================================================================
template <typename ThreadParam>
class CK2Thread
{
	typedef struct Instance;

public:
	typedef K2_THREAD_HANDLE			Handle;
	typedef typename ThreadParam&		ThreadParamRef;
	typedef typename const ThreadParam&	ThreadParamCRef;
	typedef typename void (*Handler)(ThreadParamRef);

protected:
	// Inherited interface
	CK2Thread() {}
	virtual void	ThreadMain(ThreadParam&) = 0;
	static void		Exit()			{ K2_EXIT_THREAD; }
	static void		TestCancel()	{ Sleep(0); }
	static Handle	Self()
	{
		//Handle Hnd = K2_INVALID_THREAD;
		//DuplicateHandle(GetCurrentProcess(),GetCurrentThread(),GetCurrentProcess(),(LPHANDLE)&Hnd,NULL,0,NULL);
		//return Hnd;

		// only a pseudo-handle!
		return (Handle)GetCurrentThread();
	}

public:
	// Create
	static int	Create(
		const Handler&		pFunc,
		const ThreadParam&	cFuncParam,
		Handle* const&		pOutHandle = NULL,
		const bool			bCreateDetached = false,
		const uint			uiStackSize = 0,
		const bool			bCancelEnable = false, // Used for Posix
		const bool			bCancelAsync = false) // Used for Posix
	{
		M_Create().Lock();

		Instance I(cFuncParam, 0, pFunc);

		Handle hHnd(K2_CREATE_THREAD(uiStackSize, ThreadMainHandler, &I));

		if (hHnd == K2_CREATE_THREAD_FAILED)
		{
			if (pOutHandle)
				*pOutHandle = K2_INVALID_THREAD;
			M_Create().Unlock();
			return K2_CREATE_THREAD_ERROR;
		}

		if (pOutHandle)
			*pOutHandle = hHnd;

		S_Create().Wait();
		M_Create().Unlock();

		if (bCreateDetached)
			K2_CLOSE_HANDLE(hHnd);
		return 0;
	}


	// Create
	int				Create(
		const ThreadParam&	cFuncParam,
		Handle* const&		pOutHandle = NULL,
		const bool			bCreateDetached = false,
		const uint			uiStackSize = 0,
		const bool			bCancelEnable = false,		// Used for Posix
		const bool			bCancelAsync = false) const	// Used for Posix
	{
		M_Create().Lock();

		Instance I(cFuncParam, const_cast<CK2Thread *>(this));

		Handle hHnd(K2_CREATE_THREAD(uiStackSize, ThreadMainHandler, &I));

		if (hHnd == K2_CREATE_THREAD_FAILED)
		{
			if (pOutHandle)
				*pOutHandle = K2_INVALID_THREAD;
			M_Create().Unlock();
			return K2_CREATE_THREAD_ERROR;
		}

		if (pOutHandle)
			*pOutHandle = hHnd;

		S_Create().Wait();
		M_Create().Unlock();

		if (bCreateDetached)
			K2_CLOSE_HANDLE(hHnd);
		return 0;
	}


	// Join
	static int		Join(const Handle &hThread)
	{
		DWORD R = WaitForSingleObject((HANDLE)hThread, INFINITE);

		if ((R == WAIT_OBJECT_0) || (R == WAIT_ABANDONED))
		{
			K2_CLOSE_HANDLE(hThread);
			return 0;
		}

		if (R == WAIT_TIMEOUT)
			return EAGAIN;
		return EINVAL;
	}


	// Kill
	static int		Kill(const Handle &hThread)
	{
		if (TerminateThread((HANDLE)hThread, 0))
			return 0;
		else
			return EINVAL;
	}


	// Detach
	static int		Detach(const Handle &hThread)
	{
		if (K2_CLOSE_HANDLE(hThread))
			return 0;
		else
			return EINVAL;
	}


private:
	static const CK2Mutex&		M_Create()	{ static CK2Mutex M; return M; }
	static const CK2Semaphore&	S_Create()	{ static CK2Semaphore S; return S; }


	static K2_THREAD_RET_T K2_THREAD_CALL	ThreadMainHandler(Instance *cFuncParam)
	{
		Instance I(*cFuncParam);
		ThreadParam  Data(I.Data);
		S_Create().Post();

		if (I.Owner)
			I.Owner->ThreadMain(Data);
		else
			I.pFN(Data);

		Exit();
		K2_THREAD_RETURN(0);
	}


	struct Instance
	{
		Instance(
			ThreadParamCRef P,
			CK2Thread<ThreadParam>* const& O,
			const Handler& pH = 0) :
		pFN(pH), Data(P), Owner(O)
		{}

		ThreadParamCRef			Data;
		CK2Thread<ThreadParam>*	Owner;
		Handler					pFN;
	};
};
//=============================================================================

//=============================================================================
// CK2Thread<void>
//  Explicit Specialization of void
//=============================================================================
template<>
class CK2Thread<void>
{
	typedef struct Instance;

public:
	typedef K2_THREAD_HANDLE	Handle;
	typedef void	(*Handler)();

protected:
	// Inherited interface
	CK2Thread<void>() {}
	virtual void	ThreadMain() = 0;
	static void		Exit()			{ K2_EXIT_THREAD; }
	static void		TestCancel()	{ Sleep(0); }
	static Handle	Self()			{ return (Handle)GetCurrentThread(); }

public:
	// Create
	static int		Create(
		const Handler&		pFunc,
		Handle* const&		pOutHandle = NULL,
		const bool			bCreateDetached = false,
		const uint			uiStackSize = 0,
		const bool			bCancelEnable = false, // Used for Posix
		const bool			bCancelAsync = false) // Used for Posix
	{
		Handle hHnd(K2_CREATE_THREAD(uiStackSize, ThreadMainHandler_S, pFunc));

		if (hHnd == K2_CREATE_THREAD_FAILED)
		{
			if (pOutHandle)
				*pOutHandle = K2_INVALID_THREAD;
			return K2_CREATE_THREAD_ERROR;
		}

		if (pOutHandle)
			*pOutHandle = hHnd;

		if (bCreateDetached)
			K2_CLOSE_HANDLE(hHnd);

		return 0;
	}


	// Create
	int				Create(
		Handle* const&	pOutHandle = NULL,
		const bool		bCreateDetached = false,
		const uint		uiStackSize = 0,
		const bool		bCancelEnable = false,		// Used for Posix
		const bool		bCancelAsync = false) const	// Used for Posix
	{
		Handle hHnd(K2_CREATE_THREAD(uiStackSize, ThreadMainHandler, this));

		if (hHnd == K2_CREATE_THREAD_FAILED)
		{
			if (pOutHandle)
				*pOutHandle = K2_INVALID_THREAD;
			return K2_CREATE_THREAD_ERROR;
		}

		if (pOutHandle)
			*pOutHandle = hHnd;

		if (bCreateDetached)
			K2_CLOSE_HANDLE(hHnd);

		return 0;
	}


	// Join
	static int		Join(const Handle &hThread)
	{
		DWORD R = WaitForSingleObject((HANDLE)hThread, INFINITE);

		if ((R == WAIT_OBJECT_0) || (R == WAIT_ABANDONED))
		{
			K2_CLOSE_HANDLE(hThread);
			return 0;
		}

		if (R == WAIT_TIMEOUT)
			return EAGAIN;
		return EINVAL;
	}


	// Kill
	static int		Kill(const Handle &hThread)
	{
		if (TerminateThread((HANDLE)hThread, 0))
			return 0;
		else
			return EINVAL;
	}


	// Detach
	static int		Detach(const Handle &hThread)
	{
		if (K2_CLOSE_HANDLE(hThread))
			return 0;
		else
			return EINVAL;
	}


private:
	static K2_THREAD_RET_T K2_THREAD_CALL ThreadMainHandler(CK2Thread<void> *cFuncParam)
	{
		cFuncParam->ThreadMain();
		Exit();
		K2_THREAD_RETURN(0);
	}


	static K2_THREAD_RET_T K2_THREAD_CALL ThreadMainHandler_S(Handler cFuncParam)
	{
		cFuncParam();
		Exit();
		K2_THREAD_RETURN(0);
	}
};
//=============================================================================

#endif // _WIN32
#endif // __C_THREAD_WIN32_H__
