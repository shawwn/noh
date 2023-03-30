// (C)2010 S2 Games
// c_mutex_posix.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//=============================================================================
#ifndef __C_MUTEX_POSIX_H__
#define __C_MUTEX_POSIX_H__
#ifndef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include <pthread.h>
//=============================================================================

//=============================================================================
// CK2Mutex
//=============================================================================
class CK2Mutex
{
	// Prevent copying
	CK2Mutex(const CK2Mutex &cMtx)	{}
	void	operator=(CK2Mutex &cMtx)	{}

private:
	// Member variables
	mutable pthread_mutex_t		m_hHandle;

public:
	// Constructor
	CK2Mutex()
	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_hHandle, &attr);
		pthread_mutexattr_destroy(&attr);
	}


	// Destructor
	virtual	~CK2Mutex()
	{
		pthread_mutex_unlock(&m_hHandle);
		pthread_mutex_destroy(&m_hHandle);
	}


	// Lock
	int		Lock() const
	{
		return pthread_mutex_lock(&m_hHandle);
	}


	// Lock_Try
	int		Lock_Try() const
	{
		return pthread_mutex_trylock(&m_hHandle);
	}


	// Unlock
	int		Unlock() const
	{
		return pthread_mutex_unlock(&m_hHandle);
	}
};
//=============================================================================

#endif // !_WIN32
#endif // __C_MUTEX_POSIX_H__
