// (C)2010 S2 Games
// c_spinmutex.h
//
//  A mutex which occupies just one byte.  Used for locking short critical
// sections, such as updating a reference count.
//
//  CLockSpinMutex is a helper object to lock a SpinMutex.  The helper's
// destructor releases the lock (making the mutex lock exception-safe,
// among other benefits).
//=============================================================================
#ifndef __C_SPINMUTEX_H__
#define __C_SPINMUTEX_H__

#if 0
//=============================================================================
// Headers
//=============================================================================
#include "k2_atomic.h"
//=============================================================================

//=============================================================================
// CSpinMutex
//=============================================================================
class CSpinMutex
{
private:
    friend class CLockSpinMutex;

    byte    m_yFlag;

public:
    CSpinMutex()
        : m_yFlag(0)
    {
    }

    // Locks the mutex, blocking if necessary
    void    Lock()
    {
        K2_Atomic_Synchronize(m_yFlag);
    }

    // Attempts to lock the mutex without blocking.  Returns true if mutex
    // was locked.
    bool    TryLock()
    {
        return K2_Atomic_TrySynchronize(m_yFlag);
    }

    // Unlocks the mutex.
    void    Unlock()
    {
        K2_Atomic_StoreWithRelease(m_yFlag, 0);
    }
};
//=============================================================================

//=============================================================================
// CLockSpinMutex
//=============================================================================
class CLockSpinMutex
{
private:
    CSpinMutex*     m_pMutex;

    inline bool     DoTryLock(CSpinMutex& cMutex)
    {
        assert(m_pMutex == nullptr);
        if (K2_Atomic_TrySynchronize(cMutex.m_yFlag))
        {
            m_pMutex = &cMutex;
            return true;
        }
        return false;
    }

    inline void     DoLock(CSpinMutex& cMutex)
    {
        assert(m_pMutex == nullptr);
        K2_Atomic_Synchronize(cMutex.m_yFlag);
        m_pMutex = &cMutex;
    }

    inline void     DoUnlock()
    {
        assert(m_pMutex != nullptr);
        K2_Atomic_StoreWithRelease(m_pMutex->m_yFlag, 0);
        m_pMutex = nullptr;
    }

public:
    // Constructs without locking
    CLockSpinMutex()
        : m_pMutex(nullptr)
    {}

    // Locks the mutex on construction.
    CLockSpinMutex(CSpinMutex& cMutex)
        : m_pMutex(nullptr)
    {
        DoLock(cMutex);
    }

    // Unlocks the mutex on destruction
    ~CLockSpinMutex()
    {
        if (m_pMutex != nullptr)
            DoUnlock();
    }

    // Releases any existing lock
    void    Reset()
    {
        if (m_pMutex != nullptr)
            DoUnlock();
    }

    // Releases any existing lock, then tries to acquire the new mutex.
    void    Reset(CSpinMutex& cMutex)
    {
        if (m_pMutex != nullptr)
            DoUnlock();
        DoLock(cMutex);
    }


    // Attempts to lock the mutex without blocking.  Returns true if mutex
    // was locked.
    bool    TryLock(CSpinMutex& cMutex)
    {
        if (m_pMutex != nullptr)
            DoUnlock();
        return DoTryLock(cMutex);
    }
};
//=============================================================================
#endif

#endif