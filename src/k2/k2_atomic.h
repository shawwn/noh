// (C)2010 S2 Games
// k2_atomic.h
//
//=============================================================================
#ifndef __K2_ATOMIC_H__
#define __K2_ATOMIC_H__
#if 0

//=============================================================================
// Headers
//=============================================================================
#include "k2_platform.h"
//=============================================================================

#if K2_ARCH_WIN_IA32
//=============================================================================
// Windows IA32
//=============================================================================

// K2_Atomic_ReleaseConsistencyHelper
#if _MSC_VER >= 1300
extern "C" void                                 _ReadWriteBarrier();
#pragma intrinsic                               (_ReadWriteBarrier)
#define K2_Atomic_ReleaseConsistencyHelper()    _ReadWriteBarrier()
#else
#error Unsupported compiler (K2_Atomic_ReleaseConsistencyHelper)
#endif

// K2_Atomic_FullMemoryFence
#define K2_Atomic_FullMemoryFence()             __asm { __asm mfence }

//*****************************************************************************
// Timing
//*****************************************************************************

// K2_Atomic_Yield
extern "C" __declspec(dllimport) int __stdcall  SwitchToThread(void);
#define K2_Atomic_Yield()                       SwitchToThread()

// K2_Atomic_Pause
#define K2_Atomic_Pause                         K2_Atomic_Pause_
static inline void  K2_Atomic_Pause_(uint uiDelay)
{
    _asm
    {
        mov     eax,        uiDelay
L_PAUSING:
        pause
        add     eax,        -1
        jne     L_PAUSING
    }
    return;
}

//*****************************************************************************
// CmpAndSwap1
//*****************************************************************************

// K2_Atomic_CmpAndSwap1
#define K2_Atomic_CompareAndSwap1               K2_Atomic_CmpAndSwap1
static inline __int8    K2_Atomic_CmpAndSwap1(volatile void* pPtr, __int8 iValue, __int8 iCompare)
{
    __int8 iResult;
    volatile __int8* pMem((__int8*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   cl,             iValue
        __asm mov   al,             iCompare
        __asm lock  cmpxchg [edx],  cl
        __asm mov   iResult,        al
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchAdd1
static inline __int8    K2_Atomic_FetchAdd1(volatile void* pPtr, __int8 iAdd)
{
    __int8 iResult;
    volatile __int8* pMem((__int8*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   al,             iAdd
        __asm lock  xadd [edx],     al
        __asm mov   iResult,        al
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchStore1
static inline __int8    K2_Atomic_FetchStore1(volatile void* pPtr, __int8 iValue)
{
    __int8 iResult;
    volatile __int8* pMem((__int8*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   al,             iValue
        __asm lock  xchg [edx],     al
        __asm mov   iResult,        al
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

//*****************************************************************************
// CmpAndSwap4
//*****************************************************************************

// K2_Atomic_CmpAndSwap4
#define K2_Atomic_CompareAndSwap4               K2_Atomic_CmpAndSwap4
static inline __int32   K2_Atomic_CmpAndSwap4(volatile void* pPtr, __int32 iValue, __int32 iCompare)
{
    __int32 iResult;
    volatile __int32* pMem((__int32*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   ecx,            iValue
        __asm mov   eax,            iCompare
        __asm lock  cmpxchg [edx],  ecx
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchAdd4
static inline __int32   K2_Atomic_FetchAdd4(volatile void* pPtr, __int32 iAdd)
{
    __int32 iResult;
    volatile __int32* pMem((__int32*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   eax,            iAdd
        __asm lock  xadd [edx],     eax
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchStore4
static inline __int32   K2_Atomic_FetchStore4(volatile void* pPtr, __int32 iValue)
{
    __int32 iResult;
    volatile __int32* pMem((__int32*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   eax,            iValue
        __asm lock  xchg [edx],     eax
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

//*****************************************************************************
// CmpAndSwapPtr
//*****************************************************************************

// K2_Atomic_CmpAndSwapPtr
#define K2_Atomic_CompareAndSwapPtr             K2_Atomic_CmpAndSwapPtr
static inline ptrdiff_t K2_Atomic_CmpAndSwapPtr(volatile void* pPtr, ptrdiff_t pValue, ptrdiff_t pCompare)
{
    ptrdiff_t iResult;
    volatile ptrdiff_t* pMem((ptrdiff_t*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   ecx,            pValue
        __asm mov   eax,            pCompare
        __asm lock  cmpxchg [edx],  ecx
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchAddPtr
static inline ptrdiff_t K2_Atomic_FetchAddPtr(volatile void* pPtr, ptrdiff_t iAdd)
{
    ptrdiff_t iResult;
    volatile ptrdiff_t* pMem((ptrdiff_t*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   eax,            iAdd
        __asm lock  xadd [edx],     eax
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}

// K2_Atomic_FetchStorePtr
static inline ptrdiff_t K2_Atomic_FetchStorePtr(volatile void* pPtr, ptrdiff_t pValue)
{
    ptrdiff_t iResult;
    volatile ptrdiff_t* pMem((ptrdiff_t*)pPtr);
    K2_Atomic_ReleaseConsistencyHelper();
    __asm
    {
        __asm mov   edx,            pMem
        __asm mov   eax,            pValue
        __asm lock  xchg [edx],     eax
        __asm mov   iResult,        eax
    }
    K2_Atomic_ReleaseConsistencyHelper();
    return iResult;
}
//=============================================================================
#endif

#if K2_ARCH_LINUX_IA32
//=============================================================================
// Linux IA32
//=============================================================================

//=============================================================================
#endif

//=============================================================================
// Platform-Specific Requirements
//=============================================================================
#if !defined(K2_Atomic_CompareAndSwap4) ||                          \
    !defined(K2_Atomic_Yield) ||                                    \
    !defined(K2_Atomic_FullMemoryFence) ||                          \
    !defined(K2_Atomic_ReleaseConsistencyHelper)
#error Minimum platform-specific atomic operations were not defined in k2_atomic.h
#endif
//=============================================================================

//=============================================================================
// Higher-Level Operations
//=============================================================================

//*****************************************************************************
// Wait
//*****************************************************************************

// CK2AtomicWait
class CK2AtomicWait
{
    static const uint   SPIN_LOOPS = 16;

    CK2AtomicWait(const CK2AtomicWait&) {}
    void operator = (const CK2AtomicWait&) const {}

private:
    uint    m_uiCount;

public:
    CK2AtomicWait()
        : m_uiCount(1)
    { }


    void    Reset()     { m_uiCount = 1; }


    /*====================
      CK2AtomicWait::Wait
      ====================*/
    void    Wait()
    {
        if (m_uiCount <= SPIN_LOOPS)
        {
            K2_Atomic_Pause(m_uiCount);
            m_uiCount *= 2;
        }
        else
        {
            K2_Atomic_Yield();
        }
    }


    /*====================
      CK2AtomicWait::WaitNoYield
      ====================*/
    bool    WaitNoYield()
    {
        if (m_uiCount <= SPIN_LOOPS)
        {
            K2_Atomic_Pause(m_uiCount);
            m_uiCount *= 2;
            return true;
        }
        else
        {
            return false;
        }
    }
};

//*****************************************************************************
// LoadWithAcquire / StoreWithRelease
//*****************************************************************************

// CK2AtomicLoadStore
template <typename T, size_t S>
struct  CK2AtomicLoadStore
{
    static inline T     LoadWithAcquire(const volatile T& tVar)
    {
        T tReturning = tVar;
        K2_Atomic_ReleaseConsistencyHelper();
        return tReturning;
    }

    static inline void  StoreWithRelease(volatile T& tVar, T tSet)
    {
        K2_Atomic_ReleaseConsistencyHelper();
        tVar = tSet;
    }
};


// K2_Atomic_LoadWithAcquire
template <typename T>
static inline T     K2_Atomic_LoadWithAcquire(const volatile T& tVar)
{
    return CK2AtomicLoadStore<T, sizeof(T)>::LoadWithAcquire(tVar);
}


// K2_Atomic_StoreWithRelease
template <typename T, typename V>
static inline void  K2_Atomic_StoreWithRelease(volatile T& tVar, V tSet)
{
    CK2AtomicLoadStore<T, sizeof(T)>::StoreWithRelease(tVar, T(tSet));
}


//*****************************************************************************
// CompareAndSwap
//*****************************************************************************

// K2_Atomic_CompareAndSwap
template <typename T, size_t S>
inline T        K2_Atomic_CompareAndSwap(volatile void* pPtr, T tVal, T tCmp)
{
    return K2_Atomic_CompareAndSwapPtr((T*)pPtr, tVal, tCmp);
}


// K2_Atomic_CompareAndSwap<byte>
template<>
inline byte     K2_Atomic_CompareAndSwap<byte, 1>(volatile void* pPtr, byte yVal, byte yCmp)
{
#ifndef K2_Atomic_CompareAndSwap1
#error K2_Atomic_CompareAndSwap1 undefined
#endif

    return K2_Atomic_CompareAndSwap1(pPtr, yVal, yCmp);
}


// K2_Atomic_CompareAndSwap<uint>
template<>
inline uint     K2_Atomic_CompareAndSwap<uint, 4>(volatile void* pPtr, uint uiVal, uint uiCmp)
{
#ifndef K2_Atomic_CompareAndSwap4
#error K2_Atomic_CompareAndSwap4 undefined
#endif

    return K2_Atomic_CompareAndSwap4(pPtr, uiVal, uiCmp);
}

// K2_Atomic_FetchAndAdd
template<typename T, size_t S>
inline T        K2_Atomic_FetchAndAdd(volatile void* pPtr, T tAdd)
{
    CK2AtomicWait cWait;
    T tResult;
    for(;;)
    {
        tResult = *reinterpret_cast<volatile T*>(pPtr);
        // K2_Atomic_CompareAndSwap must have full memory fence. 
        if(K2_Atomic_CompareAndSwap<T, S>(pPtr, tResult + tAdd, tResult) == tResult) 
            break;
        cWait.Wait();
    }
    return tResult;
}

#ifndef K2_Atomic_FetchAndAdd1
#define K2_Atomic_FetchAndAdd1  K2_Atomic_FetchAndAdd<byte,1>
#endif

#ifndef K2_Atomic_FetchAndAdd4
#define K2_Atomic_FetchAndAdd4  K2_Atomic_FetchAndAdd<uint,4>
#endif

#ifndef K2_Atomic_FetchAndAddW
#define K2_Atomic_FetchAndAddW  K2_Atomic_FetchAndAdd<ptrdiff_t, sizeof(ptrdiff_t)>
#endif


//*****************************************************************************
// LockByte
//*****************************************************************************

// K2_Atomic_TrySynchronize
inline bool K2_Atomic_TrySynchronize(unsigned char& yFlag)
{
    return (K2_Atomic_CompareAndSwap1(&yFlag, 1, 0) == 0);
}


// K2_Atomic_Synchronize
inline void K2_Atomic_Synchronize(unsigned char& yFlag)
{
    if (!K2_Atomic_TrySynchronize(yFlag))
    {
        CK2AtomicWait cWait;
        do 
        {
            cWait.Wait();
        } while (!K2_Atomic_TrySynchronize(yFlag));
    }
}

//=============================================================================

#endif // #if 0
#endif // __K2_ATOMIC_H__

