// (C)2010 S2 Games
// c_mutex_win32.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//=============================================================================
#ifndef __C_MUTEX_WIN32_H__
#define __C_MUTEX_WIN32_H__
#ifdef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include "k2_include_windows.h"
//=============================================================================

//=============================================================================
// CK2Mutex
//=============================================================================
class CK2Mutex
{
    // Prevent copying
    CK2Mutex(const CK2Mutex &cMtx)  {}
    void    operator=(CK2Mutex &cMtx)   {}

private:
    // Member variables
    mutable CRITICAL_SECTION    m_hHandle;

public:
    // Constructor
    CK2Mutex()
    {
        InitializeCriticalSection(&m_hHandle);
    }


    // Destructor
    virtual ~CK2Mutex()
    {
        DeleteCriticalSection(&m_hHandle);
    }


    // Lock
    int     Lock() const
    {
        EnterCriticalSection(&m_hHandle);
        return 0;
    }


#if(_WIN32_WINNT >= 0x0400)
    // Lock_Try
    int     Lock_Try() const
    {
        return (TryEnterCriticalSection(&m_hHandle) ? 0 : EBUSY);
    }
#endif


    // Unlock
    int     Unlock() const
    {
        LeaveCriticalSection(&m_hHandle);
        return 0;
    }
};
//=============================================================================

#endif // _WIN32
#endif // __C_MUTEX_WIN32_H__
