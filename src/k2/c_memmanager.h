// (C)2005 S2 Games
// c_memmanager.h
//
//=============================================================================
#ifndef __C_MEMMANAGER_H__
#define __C_MEMMANAGER_H__

//=============================================================================
// Configuration
//=============================================================================
//#define K2_FAULT_NOTEXIST
//#define K2_DEBUG_MEM
//#define K2_DEBUG_MEM_EX

// use John Ratcliff's memory reporting tool.
//#define K2_TRACK_MEM

// use John Ratcliff's micro allocator for allocations <= 256 bytes.
#define K2_USE_MICRO_ALLOCATOR
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "c_heap.h"
#include <cstring>
#include <cstdlib>
#include "MicroAllocator.h"
#include "MemoryTracker.h"
#include "c_memmanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CMemManager *g_pMemManager;

#if defined(K2_DEBUG_MEM) || defined(K2_TRACK_MEM)
#define K2_TRAP_ALLOCS
#endif
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MEM_STACK_ALLOC_MAX      (1024);
const uint MEM_START_TAG            (0xaaaa1111);
const uint MEM_END_TAG              (0xffff9999);
const byte MEM_INIT_TAG             (0xc3);
const byte MEM_FREE_TAG             (0xb2);
const uint MEM_MICRO_CHUNK_SIZE     (262144); // 0.25MB chunks

#ifdef K2_TRAP_ALLOCS

#ifdef K2_EXPORTS
#define MemManager  (*CMemManager::GetInstance())
#else
#define MemManager  (*g_pMemManager)
#endif

#else

#define MemManager  (*g_pMemManager)

#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)          do { if ((p) != nullptr) { K2_DELETE(p); (p) = nullptr; } } while (0)
#define SAFE_DELETE_ARRAY(p)    do { if ((p) != nullptr) { K2_DELETE_ARRAY(p); (p) = nullptr; } } while (0)
#endif

#ifndef errno_t
typedef int errno_t;
#endif

#ifdef K2_TRAP_ALLOCS
# define K2_NEW(heap, type)                 new (#heap, #type, __FILE__, __LINE__) type
# define K2_NEW_ARRAY(heap, type, count)    new (#heap, #type, __FILE__, __LINE__) type[count]
# define K2_DELETE(p)                       delete p
# define K2_DELETE_ARRAY(p)                 delete [] p
#else // K2_DEBUG_MEM
# define K2_NEW(heap, type)                 new type
# define K2_NEW_ARRAY(heap, type, count)    new type[count]
# define K2_DELETE(p)                       delete p
# define K2_DELETE_ARRAY(p)                 delete [] p
#endif // K2_DEBUG_MEM

#define K2_STACK_ALLOC(heap, size)          MemManager.StackAlloc(size, #heap, MT_MALLOC, "none", __FILE__, __LINE__)
#define K2_STACK_FREE(heap, size, p)        MemManager.StackFree(size, p, #heap, MT_FREE, "none", __FILE__, __LINE__)

//=============================================================================

//=============================================================================
// CMemManager
//=============================================================================
class CMemManager
{

private:
    // Singleton
    K2_API static CMemManager*  s_pInstance;
    K2_API static bool          s_bRequested;
    K2_API static bool          s_bReleased;
    K2_API static bool          s_bTrackAllocs;
    CMemManager();
    CMemManager(CMemManager&);
    CMemManager&    operator=(CMemManager&);

    K2_API void             Init();

    static K2_API MICRO_ALLOCATOR::HeapManager*     m_cMicroHeapManager;

    // Stats
#ifdef K2_DEBUG_MEM
    ULONGLONG   m_zCopyCount;
    ULONGLONG   m_zMoveCount;
    ULONGLONG   m_zWriteCount;

    ULONGLONG   m_zCopyBytes;
    ULONGLONG   m_zMoveBytes;
    ULONGLONG   m_zWriteBytes;

    ULONGLONG   m_zTotalAllocs;
    ULONGLONG   m_zTotalAllocSize;
    ULONGLONG   m_zTotalDeallocs;
    ULONGLONG   m_zTotalDeallocSize;
    ULONGLONG   m_zMemoryManagerOverhead;

    ULONGLONG   m_zTrackAllocs;
    ULONGLONG   m_zTrackAllocSize;
    ULONGLONG   m_zTrackDeallocs;
    ULONGLONG   m_zTrackDeallocSize;

    uint        m_uiTimeStamp;
    uint        m_uiSequence;

    // Allocation list
    SMemHeader*     m_pHead;
    SMemHeader*     m_pTail;

    SMemHeader*     m_pTrackHead;
    SMemHeader*     m_pTrackTail;
#endif //K2_DEBUG_MEM

#ifdef K2_TRACK_MEM
    static K2_API const char*   GetStr(const char* sStr);
#endif

public:
    ~CMemManager()  {}

    static K2_API CMemManager*  GetInstance();
    static void                 Release();
    static void                 Frame();

#ifdef K2_DEBUG_MEM
    K2_API void*                Allocate(size_t z, const char *szContext = "global", MemoryType eMemType = MT_MALLOC, const char *szType = "none", const char *szFile = __FILE__, short nLine = __LINE__);
    template<class T> T*        Reallocate(T *&p, size_t z);
    K2_API void                 Deallocate(void *p, const char *szContext = "global", MemoryType eMemType = MT_FREE, const char *szFile = __FILE__, short nLine = __LINE__);
#else
    K2_API static void*         Allocate(size_t z, const char *szContext = "global", MemoryType eMemType = MT_MALLOC, const char *szType = "none", const char *szFile = __FILE__, short nLine = __LINE__);
    template<class T> static T* Reallocate(T *&p, size_t z);
    K2_API static void          Deallocate(void *p, const char *szContext = "global", MemoryType eMemType = MT_FREE, const char *szFile = __FILE__, short nLine = __LINE__);
#endif

#ifdef K2_TRACK_MEM
    inline static K2_API void   TrackExternalAlloc(void* p, size_t z, const char *szContext = "global", MemoryType eMemType = MT_MALLOC, const char *szType = "none", const char *szFile = __FILE__, short nLine = __LINE__);
    inline static K2_API void   TrackExternalFree(void *p, const char *szContext = "global", MemoryType eMemType = MT_FREE, const char *szFile = __FILE__, short nLine = __LINE__);
#define EXTERNAL_ALLOC(p, size, ctx, memType, type)     MemManager.TrackExternalAlloc(p, size, ctx, MT_MALLOC, type, __FILE__, __LINE__)
#define EXTERNAL_FREE(p, ctx, memType, type)            MemManager.TrackExternalFree(p, ctx, MT_FREE, type, __FILE__, __LINE__)
#else
#define EXTERNAL_ALLOC(p, size, ctx, memType, type)     
#define EXTERNAL_FREE(p, ctx, memType, type)            
#endif

#ifdef K2_DEBUG_MEM
    void*                Copy(void *pDest, const void *pSrc, size_t z);
    errno_t              Copy_s(void *pDest, size_t dstSize, const void *pSrc, size_t z);
    void*                Move(void *pDest, const void *pSrc, size_t z);
    void*                Set(void *pDest, byte y, size_t z);
#else
    static K2_API void*         Copy(void *pDest, const void *pSrc, size_t z);
    static K2_API errno_t       Copy_s(void *pDest, size_t dstSize, const void *pSrc, size_t z);
    static K2_API void*         Move(void *pDest, const void *pSrc, size_t z);
    static K2_API void*         Set(void *pDest, byte y, size_t z);
#endif

    inline static void*         StackAlloc(size_t z, const char *szContext = "global", MemoryType eMemType = MT_MALLOC, const char *szType = "none", const char *szFile = __FILE__, short nLine = __LINE__)
    {
        // if the requested size is > 1024 bytes, then perform a dynamic allocation.
        if (z > MEM_STACK_ALLOC_MAX)
            return MemManager.Allocate(z, szContext, eMemType, szType, szFile, nLine);

        // otherwise allocate on the stack.
        return alloca(z);
    }

    inline static void          StackFree(size_t z, void *p, const char *szContext = "global", MemoryType eMemType = MT_FREE, const char *szFile = __FILE__, short nLine = __LINE__)
    {
        // if the allocation was > 1024 bytes, then the allocation was dynamic, so free it.
        if (z > MEM_STACK_ALLOC_MAX)
            MemManager.Deallocate(p, szContext, eMemType, szFile, nLine);
    }

    void                        Draw();

    bool                        Validate();
#ifdef K2_DEBUG_MEM
    void    PrintStats();
    void    PrintTrackingStats();
    void    PrintAllocations(const char *szHeapName = nullptr, uint uiTime = -1);
    void    PrintAllocationsNoDuplicates(const char *szHeapName = nullptr, uint uiTime = -1);
    void    ResetTracking();
    void    PrintSequenceAllocations(uint uiSequence);

    ULONGLONG   GetActiveAllocs()       { return m_zTotalAllocs - m_zTotalDeallocs; }
    ULONGLONG   GetActiveAllocsSize()   { return m_zTotalAllocSize - m_zTotalDeallocSize; }
#endif //K2_DEBUG_MEM
};
//=============================================================================


template<class T>
inline const T& MIN(const T& _Left, const T& _Right);

/*====================
  CMemManager::Reallocate
  ====================*/
template<class T>
T*  CMemManager::Reallocate(T *&p, size_t z)
{
#ifdef K2_DEBUG_MEM
    // Just allocate if p is nullptr
    if (p == nullptr)
        return p = static_cast<T*>(Allocate(z));

    SMemHeader *pe = (SMemHeader*)((char*)p - sizeof(SMemHeader));
    assert(pe->uiMarker == MEM_START_TAG);

    void *pNew = Allocate(z, pe->pContext);
    Copy(pNew, p, MIN(pe->zSize, z));
    Deallocate(p);

    return p = static_cast<T*>(pNew);
#endif //K2_DEBUG_MEM


#ifdef K2_USE_MICRO_ALLOCATOR
    return p = static_cast<T*>(MICRO_ALLOCATOR::heap_realloc(m_cMicroHeapManager, p, z));
#else
    return p = static_cast<T*>(realloc(p, z));
#endif
}


#ifdef K2_TRACK_MEM
/*====================
  CMemManager::TrackExternalAlloc
  ====================*/
void    CMemManager::TrackExternalAlloc(void *p, size_t z, const char *szContext, MemoryType eMemType, const char *szType, const char *szFile, short nLine)
{
    if (s_bTrackAllocs)
    {
        if (szContext != nullptr)
            TRACK_ALLOC(p, (uint)z, eMemType, GetStr(szContext), GetStr(szType), GetStr(szFile), nLine);
    }
}
#endif


#ifdef K2_TRACK_MEM
/*====================
  CMemManager::TrackExternalFree
  ====================*/
void    CMemManager::TrackExternalFree(void *p, const char *szContext, MemoryType eMemType, const char *szFile, short nLine)
{
    if (s_bTrackAllocs)
    {
        if (szContext != nullptr)
            TRACK_FREE(p, eMemType, GetStr(szContext), GetStr(szFile), nLine);
    }
}
#endif


#if defined(K2_TRAP_ALLOCS)

/*====================
  operator new
  ====================*/
void*   operator new(size_t z);
void*   operator new(size_t z, const char *szContext, const char *szType);
void*   operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine);


/*====================
  operator new[]
  ====================*/
void*   operator new[](size_t z);
void*   operator new[](size_t z, const char *szContext, const char *szType);
void*   operator new[](size_t z, const char *szContext, const char *szType, const char *szFile, short nLine);


/*====================
  operator delete
  ====================*/
void    operator delete(void *p) _NOEXCEPT;
void    operator delete(void *p, const char *szContext, const char *szType) _NOEXCEPT;
void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) _NOEXCEPT;


/*====================
  operator delete[]
  ====================*/
void    operator delete[](void *p) _NOEXCEPT;
void    operator delete[](void *p, const char *szContext, const char *szType) _NOEXCEPT;
void    operator delete[](void *p, const char *szContext, const char *szType, const char *szFile, short nLine) _NOEXCEPT;

#endif //K2_DEBUG_MEM

#endif //__C_MEMMANAGER_H__
