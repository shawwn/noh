// (C)2010 S2 Games
// c_thread_posix.h
//
//  Written by Phillip Sitbon (http://www.codeproject.com/KB/threads/thread_class.aspx)
//=============================================================================
#ifndef __C_THREAD_POSIX_H__
#define __C_THREAD_POSIX_H__
#ifndef _WIN32

//=============================================================================
// Headers
//=============================================================================
#include "c_semaphore.h"
#include "c_mutex.h"
#include <pthread.h>
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
    typedef pthread_t          Handle;
    typedef ThreadParam&       ThreadParamRef;
    typedef const ThreadParam& ThreadParamCRef;
    typedef void (*Handler)(ThreadParamRef);

protected:
    // Inherited interface
    CK2Thread() {}
    virtual void    ThreadMain(ThreadParam&) = 0;
    static void     Exit()          { pthread_exit(0); } 
    static void     TestCancel()    { pthread_testcancel(); }
    static Handle   Self()          { return (Handle)pthread_self(); } 

public:
    // Create
    static int      Create(
        const Handler&      pFunc,
        ThreadParam&        cFuncParam,
        Handle*             pOutHandle = NULL,
        const bool          bCreateDetached = false,
        const size_t        uiStackSize = 0,
        const bool          bCancelEnable = false,
        const bool          bCancelAsync = false)
    {
        M_Create().Lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (bCreateDetached)
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (uiStackSize > 0)
            pthread_attr_setstacksize(&attr, uiStackSize);

        static Instance I(cFuncParam, nullptr, pFunc, bCancelEnable, bCancelAsync);

        int R = pthread_create((pthread_t *)pOutHandle, &attr, ThreadMainHandler, (void *)&I);
        pthread_attr_destroy(&attr);

        if (!R)
            S_Create().Wait();
        else if (pOutHandle)
            *pOutHandle = (Handle)K2_INVALID_THREAD;

        M_Create().Unlock();
        return errno;
    }


    // Create
    int             Create(
        const ThreadParam&  cFuncParam,
        Handle*             pOutHandle = NULL,
        const bool          bCreateDetached = false,
        const size_t        uiStackSize = 0,
        const bool          bCancelEnable = false,
        const bool          bCancelAsync = false) const
    {
        M_Create().Lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (bCreateDetached)
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (uiStackSize > 0)
            pthread_attr_setstacksize(&attr, uiStackSize);

        static Instance I(cFuncParam, const_cast<CK2Thread *>(this), 0, bCancelEnable, bCancelAsync);

        int R = pthread_create((pthread_t *)pOutHandle,&attr,ThreadMainHandler,(void *)&I);
        pthread_attr_destroy(&attr);

        if (!R)
            S_Create().Wait();
        else if (pOutHandle)
            *pOutHandle = (Handle)K2_INVALID_THREAD;

        M_Create().Unlock();
        return errno;
    }


    // Join
    static int      Join(Handle hThread)
    {
        return pthread_join(hThread,0);
    }


    // Kill
    static int      Kill(Handle hThread)
    {
        return pthread_cancel(hThread);
    }


    // Detach
    static int      Detach(Handle hThread)
    {
        return pthread_detach(hThread);
    }


private:
    static const CK2Mutex&      M_Create()  { static CK2Mutex M; return M; }
    static const CK2Semaphore&  S_Create()  { static CK2Semaphore S; return S; }


    static void*    ThreadMainHandler(void* Param)
    {
        Instance*  I((Instance*)Param);
        S_Create().Post();

        if (I->Flags & 1 /*bCancelEnable*/)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);

            if (I->Flags & 2 /*bCancelAsync*/)
                pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
            else
                pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
        }
        else
        {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
        }

        if (I->Owner)
            I->Owner->ThreadMain(I->Data);
        else
            I->pFN(I->Data);

        return 0;
    }


    struct Instance
    {
        Instance(
            ThreadParamRef P,
            CK2Thread<ThreadParam>* O,
            const Handler& pH = 0,
            bool CE = false,
            bool CA = false) :
        pFN(pH), Data(P), Owner(O), Flags(0)
        {
            if (CE)
                Flags|=1;
            if (CA)
                Flags|=2;
        }

        ThreadParamRef          Data;
        CK2Thread<ThreadParam>* Owner;
        Handler                 pFN;
        unsigned char           Flags;
    };
};
//=============================================================================

//=============================================================================
// CK2Thread<void>
//  Explicit Specialization of void
//=============================================================================
template<> class CK2Thread<void>
{
    typedef struct Instance;

public:
    typedef pthread_t   Handle;
    typedef void    (*Handler)();

protected:
    // Inherited interface
    CK2Thread<void>() {}
    virtual void    ThreadMain() = 0;
    static void     Exit()          { pthread_exit(0); }
    static void     TestCancel()    { pthread_testcancel(); }
    static Handle   Self()          { return (Handle)pthread_self(); }

public:
    // Create
    static int  Create(
        const Handler&      pFunc,
        Handle*             pOutHandle = NULL,
        const bool          bCreateDetached = false,
        const size_t        uiStackSize = 0,
        const bool          bCancelEnable = false,
        const bool          bCancelAsync = false)
    {
        M_Create().Lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (bCreateDetached)
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (uiStackSize > 0)
            pthread_attr_setstacksize(&attr, uiStackSize);

        static Instance I(0, pFunc, bCancelEnable, bCancelAsync);

        int R = pthread_create((pthread_t *)pOutHandle, &attr, ThreadMainHandler, (void *)&I);
        pthread_attr_destroy(&attr);

        if (!R)
            S_Create().Wait();
        else if (pOutHandle)
            *pOutHandle = (Handle)K2_INVALID_THREAD;

        M_Create().Unlock();
        return errno;
    }


    // Create
    int             Create(
        Handle*         pOutHandle = NULL,
        const bool      bCreateDetached = false,
        const size_t    uiStackSize = 0,
        const bool      bCancelEnable = false,
        const bool      bCancelAsync = false) const
    {
        M_Create().Lock();
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        if (bCreateDetached)
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (uiStackSize > 0)
            pthread_attr_setstacksize(&attr, uiStackSize);

        Instance I(const_cast<CK2Thread<void>*>(this), 0, bCancelEnable, bCancelAsync);

        int R = pthread_create((pthread_t *)pOutHandle, &attr, ThreadMainHandler, (void *)&I);
        pthread_attr_destroy(&attr);

        if (!R)
            S_Create().Wait();
        else if (pOutHandle)
            *pOutHandle = (Handle)K2_INVALID_THREAD;

        M_Create().Unlock();
        return errno;
    }


    // Join
    static int Join(Handle hThread)
    {
        return pthread_join(hThread, 0);
    }


    // Kill
    static int Kill(Handle hThread)
    {
        return pthread_cancel(hThread);
    }


    // Detach
    static int Detach(Handle hThread)
    {
        return pthread_detach(hThread);
    }


private:
    static const CK2Mutex&      M_Create()  { static CK2Mutex M; return M; }
    static const CK2Semaphore&  S_Create()  { static CK2Semaphore S; return S; }


    static void*    ThreadMainHandler(void *Param)
    {
        Instance  I(*(Instance*)Param);
        S_Create().Post();

        if (I.Flags & 1 /*bCancelEnable*/)
        {
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

            if (I.Flags & 2 /*bCancelAsync*/)
                pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
            else
                pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
        }
        else
        {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        }

        if (I.Owner)
            I.Owner->ThreadMain();
        else
            I.pFN();

        return 0;
    }


    struct Instance
    {
        Instance(
            CK2Thread<void>* O,
            const Handler& pH = 0,
            bool CE = false,
            bool CA = false) :
        pFN(pH), Owner(O), Flags(0)
        {
            if (CE)
                Flags|=1;
            if (CA)
                Flags|=2;
        }

        CK2Thread<void>*                Owner;
        CK2Thread<void>::Handler        pFN;
        unsigned char               Flags;
    };
};
//=============================================================================

#endif // !_WIN32
#endif // __C_THREAD_POSIX_H__
