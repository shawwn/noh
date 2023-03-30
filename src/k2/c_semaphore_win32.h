// (C)2010 S2 Games
// c_semaphore_win32.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//=============================================================================
#ifndef __C_SEMAPHORE_WIN32_H__
#define __C_SEMAPHORE_WIN32_H__
#ifdef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include "k2_include_windows.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define SEM_VALUE_MAX ((int) ((~0u) >> 1))
//=============================================================================

//=============================================================================
// CK2Semaphore
//=============================================================================
class CK2Semaphore
{
	// Prevent copying
	CK2Semaphore(const CK2Semaphore &)	{}
	void	operator=(const CK2Semaphore &)	{}

private:
	// Member variables
	HANDLE	m_hHandle;

public:
	// Constructor
	CK2Semaphore(int iVal = 0)
	{
		m_hHandle = CreateSemaphore(0, iVal, SEM_VALUE_MAX, 0);
	}


	// Destructor
	virtual ~CK2Semaphore()
	{
		CloseHandle(m_hHandle);
	}


	// Wait
	void	Wait() const
	{
		WaitForSingleObject((HANDLE)m_hHandle, INFINITE);
	}


	// Wait_Try
	int		Wait_Try() const
	{
		if (WaitForSingleObject((HANDLE)m_hHandle, INFINITE) == WAIT_OBJECT_0)
			return 0;
		else
			return EAGAIN;
	}


	// Post
	int		Post() const
	{
		if (ReleaseSemaphore((HANDLE)m_hHandle, 1, 0))
			return 0;
		else
			return ERANGE;
	}


	// Value
	int		Value() const
	{
		LONG iVal(-1);
		ReleaseSemaphore((HANDLE)m_hHandle, 0, &iVal);
		return iVal;
	}


	// Reset
	void	Reset(int iVal = 0)
	{
		CloseHandle(m_hHandle);
		m_hHandle = CreateSemaphore(0, iVal, SEM_VALUE_MAX, 0);
	}
};
//=============================================================================

#endif // _WIN32
#endif // __C_SEMAPHORE_WIN32_H__
