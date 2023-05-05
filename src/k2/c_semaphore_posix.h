// (C)2010 S2 Games
// c_semaphore_posix.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//=============================================================================
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#ifndef __C_SEMAPHORE_POSIX_H__
#define __C_SEMAPHORE_POSIX_H__
#ifndef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include <semaphore.h>
//=============================================================================

//=============================================================================
// CK2Semaphore
//=============================================================================
class CK2Semaphore
{
    // Prevent copying
    CK2Semaphore(const CK2Semaphore &)  {}
    void    operator=(const CK2Semaphore &) {}

private:
    // Member variables
    sem_t   m_hHandle;

public:
    // Constructor
    CK2Semaphore(int iVal = 0)
    {
        sem_init(&m_hHandle, 0, iVal);
    }


    // Destructor
    virtual ~CK2Semaphore()
    {
        sem_destroy(&m_hHandle);
    }


    // Wait
    void    Wait() const
    {
        sem_wait((sem_t *)&m_hHandle);
    }


    // Wait_Try
    int     Wait_Try() const
    {
        if (sem_trywait((sem_t *)&m_hHandle))
            return errno;
        else
            return 0;
    }


    // Post
    int     Post() const
    {
        if (sem_post((sem_t *)&m_hHandle))
            return errno;
        else
            return 0;
    }


    // Value
    int     Value() const
    {
        int iVal = -1;
        sem_getvalue((sem_t *)&m_hHandle, &iVal);
        return iVal;
    }


    // Reset
    void    Reset(int iVal = 0)
    {
        sem_destroy(&m_hHandle);
        sem_init(&m_hHandle, 0, iVal);
    }
};
//=============================================================================

#endif // !_WIN32
#endif // __C_SEMAPHORE_POSIX_H__

#pragma clang diagnostic pop