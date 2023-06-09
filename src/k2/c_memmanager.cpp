// (C)2005 S2 Games
// c_memmanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_memmanager.h"
#include "c_cmd.h"
#include "c_heap.h"
#include "c_fontmap.h"
#include "c_draw2d.h"
#include "c_resourcemanager.h"
#include "c_restorevalue.h"
#include "c_mutex.h"
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
#ifdef K2_DEBUG_MEM
CMemManager *g_pMemManager(CMemManager::GetInstance());
CK2Mutex g_cMemMutex;
#else
CMemManager *g_pMemManager(nullptr);
#endif

CVAR_STRINGF    (mem_font,          "system_medium",        CVAR_SAVECONFIG);
CVAR_BOOL       (mem_drawInfo,      false);
CVAR_UINT       (mem_BreakHeap,     uint(-1));
CVAR_UINT       (mem_BreakSize,     uint(-1));
CVAR_UINT       (mem_BreakTag,      uint(-1));


K2_API CMemManager  *CMemManager::s_pInstance;
K2_API bool     CMemManager::s_bReleased;
K2_API bool     CMemManager::s_bRequested;
K2_API bool     CMemManager::s_bTrackAllocs = true;

K2_API MICRO_ALLOCATOR::HeapManager*        CMemManager::m_cMicroHeapManager;

// note: these must be std::maps so that the memory tracking does not cause a stack overflow!
typedef std::map<size_t, char*>             StringMap;
typedef std::map<const char*, char*>            PtrStringMap;
static StringMap                        m_mapStrings;
static PtrStringMap                     m_mapPtrStrings;
//=============================================================================
#ifdef __GNUC__
// Ensure that the MemManager pointer gets initialized right away!
class MemManagerInitClass
{
    public:
    MemManagerInitClass() { g_pMemManager = CMemManager::GetInstance(); };
} MemManagerInitClassInstance __attribute__(( init_priority(101) ));
#endif


#ifdef K2_TRACK_MEM
/*====================
  CMemManager::GetStr
  ====================*/
const char*     CMemManager::GetStr(const char* sStr)
{
    if (physx::shdfnd2::gMemoryTracker == nullptr)
        return sStr;

    // prevent stack overflow.
    CRestoreValue<bool> cDoNotTrackAllocs(s_bTrackAllocs, false);

    PtrStringMap::iterator itPtrFind(m_mapPtrStrings.find(sStr));
    if (itPtrFind != m_mapPtrStrings.end())
    {
        char* sFinalStr(itPtrFind->second);
        return sFinalStr;
    }

    size_t uiLen(strlen(sStr));
    size_t uiHash(K2_HashMem((const void*)sStr, uiLen + 1));
    StringMap::iterator itFind(m_mapStrings.find(uiHash));
    if (itFind != m_mapStrings.end())
    {
        char* sFinalStr(itFind->second);
        m_mapPtrStrings[sStr] = sFinalStr;
        return sFinalStr;
    }

    char* sFinalStr((char*)::malloc(uiLen+1));
    memcpy(sFinalStr, sStr, uiLen+1);
    m_mapStrings[uiHash] = sFinalStr;
    m_mapPtrStrings[sStr] = sFinalStr;
    return sFinalStr;
}
#endif


/*====================
  CMemManager::CMemManager
  ====================*/
CMemManager::CMemManager()
{
}


/*====================
  CMemManager::Init
  ====================*/
void    CMemManager::Init()
{
    m_cMicroHeapManager = MICRO_ALLOCATOR::createHeapManager(MEM_MICRO_CHUNK_SIZE);

#ifdef K2_DEBUG_MEM
    m_zTotalAllocs = 0;
    m_zTotalAllocSize = 0;
    m_zTotalDeallocs = 0;
    m_zTotalDeallocSize = 0;
    m_zMemoryManagerOverhead = 0;

    m_zTrackAllocs = 0;
    m_zTrackAllocSize = 0;
    m_zTrackDeallocs = 0;
    m_zTrackDeallocSize = 0;

    m_uiTimeStamp = 0;
    m_uiSequence = 0;

    m_zCopyCount = 0;
    m_zMoveCount = 0;
    m_zWriteCount = 0;

    m_zCopyBytes = 0;
    m_zMoveBytes = 0;
    m_zWriteBytes = 0;

    m_pHead = nullptr;
    m_pTail = nullptr;

    m_pTrackHead = nullptr;
    m_pTrackTail = nullptr;
#endif //K2_DEBUG_MEM
}


/*====================
  CMemManager::Release
  ====================*/
void    CMemManager::Release()
{
    if (m_cMicroHeapManager)
    {
        MICRO_ALLOCATOR::releaseHeapManager(m_cMicroHeapManager);
        m_cMicroHeapManager = nullptr;
    }

    assert(!s_bReleased);
    if (s_pInstance != nullptr)
        free(s_pInstance);
}


/*====================
  CMemManager::Frame
  ====================*/
void    CMemManager::Frame()
{
#ifdef K2_TRACK_MEM
    TRACK_FRAME();
#endif
}


#ifndef K2_DEBUG_MEM
/*====================
  CMemManager::Allocate
  ====================*/
void*   CMemManager::Allocate(size_t z, const char *szContext, MemoryType eMemType, const char *szType, const char *szFile, short nLine)
{
    void* pResult;
#ifdef K2_USE_MICRO_ALLOCATOR
    pResult = MICRO_ALLOCATOR::heap_malloc(m_cMicroHeapManager, z);
#else
    pResult = malloc(z);
#endif

#ifdef K2_TRACK_MEM
    if (s_bTrackAllocs)
    {
        if (szContext != nullptr)
            TRACK_ALLOC(pResult, (uint)z, eMemType, GetStr(szContext), GetStr(szType), GetStr(szFile), nLine);
    }
#endif
    return pResult;
}
#endif


#ifndef K2_DEBUG_MEM
/*====================
  CMemManager::Deallocate
  ====================*/
void    CMemManager::Deallocate(void *p, const char *szContext, MemoryType eMemType, const char *szFile, short nLine)
{
#ifdef K2_TRACK_MEM
    if (s_bTrackAllocs)
    {
        if (szContext != nullptr)
            TRACK_FREE(p, eMemType, GetStr(szContext), GetStr(szFile), nLine);
    }
#endif
#ifdef K2_USE_MICRO_ALLOCATOR
    MICRO_ALLOCATOR::heap_free(m_cMicroHeapManager, p);
#else
    free(p);
#endif
}
#endif // #ifndef K2_DEBUG_MEM


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::Allocate
  ====================*/
void*   CMemManager::Allocate(size_t z, const char *szContext, MemoryType eMemType, const char *szType, const char *szFile, short nLine)
{
    assert(g_pMemManager != nullptr);
    g_cMemMutex.Lock();

//    PROFILE("CMemManager::Allocate"); // TKTK 2023: This causes a crash when K2_PROFILE is enabled

    if (szContext == nullptr)
        szContext = "stl";

    //Validate();

    /*
    if (K2System.IsInitialized() && pHeap->GetID() == mem_BreakHeap)
        K2System.DebugBreak();
    if (K2System.IsInitialized() && z >= mem_BreakSize)
        K2System.DebugBreak();
    */

    char *p = (char*)malloc(sizeof(SMemHeader) + z + sizeof(MEM_END_TAG));
    assert (p != nullptr);

    SMemHeader *pe = (SMemHeader*)p;
    pe->uiMarker = MEM_START_TAG;
#ifdef K2_DEBUG_MEM_EX
    Set(pe->szFile, 0, MEM_DEBUG_MAX_FILE_NAME_LENGTH);
    if (szFile != nullptr)
        STRNCPY_S(pe->szFile, MEM_DEBUG_MAX_FILE_NAME_LENGTH, szFile, _TRUNCATE);
    pe->nLine = nLine;
    pe->uiTimeStamp = CHost::IsAllocated() ? Host.GetTime() : 0;
#endif //K2_DEBUG_MEM_EX
    pe->zSize = z;
    pe->pContext = szContext;

    if (m_pHead != nullptr)
        m_pHead->pNext = pe;
    pe->pPrev = m_pHead;
    pe->pNext = nullptr;
    m_pHead = pe;
    if (m_pTail == nullptr)
        m_pTail = m_pHead;

    if (m_pTrackHead != nullptr)
        m_pTrackHead->pTrackNext = pe;
    pe->pTrackPrev = m_pTrackHead;
    pe->pTrackNext = nullptr;
    m_pTrackHead = pe;
    if (m_pTrackTail == nullptr)
        m_pTrackTail = m_pTrackHead;

    pe->uiSequence = m_uiSequence;

    p += sizeof(SMemHeader);

    Set(p, MEM_INIT_TAG, z);
    *(uint*)(p + z) = MEM_END_TAG;

    ++m_zTotalAllocs;
    m_zTotalAllocSize += z;
    ++m_zTrackAllocs;
    m_zTrackAllocSize += z;
    m_zMemoryManagerOverhead += sizeof(SMemHeader) + sizeof(MEM_END_TAG);

    //Validate();

    g_cMemMutex.Unlock();

    return p;
}
#endif


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::Deallocate
  ====================*/
void    CMemManager::Deallocate(void *p, const char *szContext, MemoryType eMemType, const char *szFile, short nLine)
{
//    PROFILE("CMemManager::Deallocate"); // TKTK 2023: This causes a crash when K2_PROFILE is enabled

    if (!p) // Microsoft documentation says deallocate should simply ignore nullptr pointers
        return;
    
    g_cMemMutex.Lock();

    //Validate();

    SMemHeader *pe = (SMemHeader*)((char*)p - sizeof(SMemHeader));

    assert(pe->uiMarker == MEM_START_TAG);

    /*
    if (K2System.IsInitialized() && pe->uiSequence == mem_BreakTag)
        K2System.DebugBreak();
    */

    if (pe->pNext != nullptr)
        pe->pNext->pPrev = pe->pPrev;
    if (pe->pPrev != nullptr)
        pe->pPrev->pNext = pe->pNext;
    if (m_pHead == pe)
        m_pHead = pe->pPrev;
    if (m_pTail == pe)
        m_pTail = pe->pNext;

    if (pe->pTrackNext != nullptr)
        pe->pTrackNext->pTrackPrev = pe->pTrackPrev;
    if (pe->pTrackPrev != nullptr)
        pe->pTrackPrev->pTrackNext = pe->pTrackNext;
    if (m_pTrackHead == pe)
        m_pTrackHead = pe->pTrackPrev;
    if (m_pTrackTail == pe)
        m_pTrackTail = pe->pTrackNext;

    assert(*(uint*)((char*)p + pe->zSize) == MEM_END_TAG);

    ++m_zTotalDeallocs;
    m_zTotalDeallocSize += pe->zSize;
    ++m_zTrackDeallocs;
    m_zTrackDeallocSize += pe->zSize;
    m_zMemoryManagerOverhead -= sizeof(SMemHeader) + sizeof(MEM_END_TAG);

#ifdef K2_DEBUG_MEM_EX
    size_t zSize(pe->zSize);
    Set(pe, MEM_FREE_TAG, sizeof(SMemHeader) + zSize + sizeof(MEM_END_TAG));
    free(pe);
#else
    free(pe);
#endif

    //Validate();
    
    g_cMemMutex.Unlock();
}
#endif


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::PrintStats
  ====================*/
void    CMemManager::PrintStats()
{
    Console.Mem << _T("Total allocations: ") << m_zTotalAllocs << SPACE
        << ParenStr(GetByteString(m_zTotalAllocSize)) << _T(" (") << m_zTotalAllocSize << _T(" bytes)") << newl
        << _T("Total deallocations: ") << m_zTotalDeallocs << SPACE
        << ParenStr(GetByteString(m_zTotalDeallocSize)) << _T(" (") << m_zTotalDeallocSize << _T(" bytes)") << newl
        << _T("Active alloations: ") << m_zTotalAllocs - m_zTotalDeallocs << SPACE
        << ParenStr(GetByteString(m_zTotalAllocSize - m_zTotalDeallocSize)) << _T(" (") << m_zTotalAllocSize - m_zTotalDeallocSize << _T(" bytes)") << newl
        << _T("MemoryManager overhead: ") << m_zMemoryManagerOverhead << SPACE << ParenStr(GetByteString(m_zMemoryManagerOverhead)) << newl
        << _T("Total memory allocated: ") << GetByteString(m_zMemoryManagerOverhead + (m_zTotalAllocSize - m_zTotalDeallocSize)) << newl
        << _T("Copies: ") << m_zCopyCount << SPACE << ParenStr(GetByteString(m_zCopyBytes))<< newl
        << _T("Moves: ") << m_zMoveCount << SPACE  << ParenStr(GetByteString(m_zMoveBytes)) << newl
        << _T("Writes: ") << m_zWriteCount << SPACE  << ParenStr(GetByteString(m_zWriteBytes)) << newl
        << _T("----------------------------------------") << newl;
}


/*====================
  CMemManager::PrintTrackingStats
  ====================*/
void    CMemManager::PrintTrackingStats()
{
    size_t zAllocs(m_zTrackAllocs);
    size_t zDeallocs(m_zTrackDeallocs);
    size_t zAllocSize(m_zTrackAllocSize);
    size_t zDeallocSize(m_zTrackDeallocSize);

    /*
    SMemHeader *pHeader(m_pTrackTail);
    SMemHeader *pHeaderEnd(m_pTrackHead);

    while (pHeader != nullptr && pHeader != pHeaderEnd)
    {
        Console.Mem << _T("#") << pHeader->uiSequence << SPACE << pHeader->pHeap->GetName() << SPACE << INT_SIZE(pHeader->zSize) << newl;
        pHeader = pHeader->pTrackNext;
    }
    */

    Console.Mem << _T("Total allocations: ") << INT_SIZE(zAllocs) << _T(" (") << INT_SIZE(zAllocSize) << _T(" bytes)") << newl
        << _T("Total deallocations: ") << INT_SIZE(zDeallocs) << _T(" (") << INT_SIZE(zDeallocSize) << _T(" bytes)") << newl
        << _T("Active alloations: ") << INT_SIZE(zAllocs - zDeallocs) << _T(" (") << INT_SIZE(zAllocSize - zDeallocSize) << _T(" bytes)") << newl;
}


/*====================
  CMemManager::PrintAllocations
  ====================*/
void    CMemManager::PrintAllocations(const char *szHeapName, uint uiTime)
{
#if 0
    SMemHeader *pHeader(m_pTail);
    SMemHeader *pStop(m_pHead);
    uint uiTotal(0);
    bool bFoundHeap(false);
    if (szHeapName != nullptr && strlen(szHeapName) > 0)
    {
        for (int i(0); i < MAX_HEAPS + NUM_RESERVED_HEAPS; ++i)
        {
            if (strcmp(szHeapName, s_apHeaps[i]->GetName()) == 0)
            {
                pHeader = s_apHeaps[i]->GetTail();
                pStop = s_apHeaps[i]->GetHead();
                bFoundHeap = true;
                break;
            }
        }
    }

    if (szHeapName != nullptr && !bFoundHeap)
        return;

    if (uiTime == -1)
        uiTime = m_uiTimeStamp;

    while (pHeader != nullptr && pHeader != pStop)
    {
#ifdef K2_DEBUG_MEM_EX
        if (pHeader->uiTimeStamp >= uiTime)
        {
            Console << _T("File: ") << pHeader->szFile << _T(" Line: ") << pHeader->nLine << _T(" Context: ") << pHeader->pContext << _T(" Size: ") << INT_SIZE(pHeader->zSize) << newl;
            uiTotal += INT_SIZE(pHeader->zSize);
        }
#endif

        if (bFoundHeap)
            pHeader = pHeader->pHeapNext;
        else
            pHeader = pHeader->pNext;
    }

    Console << _T("Total allocated: ") << uiTotal << newl;
#endif
}

/*====================
  CMemManager::PrintAllocationsNoDuplicates
  ====================*/
void    CMemManager::PrintAllocationsNoDuplicates(const char *szHeapName, uint uiTime)
{
#if 0
    SMemHeader *pHeader(m_pTail);
    SMemHeader *pStop(m_pHead);
    uint uiTotal(0);
    uint uiTotalNoFile(0);
    bool bFoundHeap(false);
    if (szHeapName != nullptr && strlen(szHeapName) > 0)
    {
        for (int i(0); i < MAX_HEAPS + NUM_RESERVED_HEAPS; ++i)
        {
            if (strcmp(szHeapName, s_apHeaps[i]->GetName()) == 0)
            {
                pHeader = s_apHeaps[i]->GetTail();
                pStop = s_apHeaps[i]->GetHead();
                bFoundHeap = true;
                break;
            }
        }
    }

    if (szHeapName != nullptr && !bFoundHeap)
        return;

    if (uiTime == -1)
        uiTime = m_uiTimeStamp;

    map<string, map<uint, uint> > mapAllocations;

    while (pHeader != nullptr && pHeader != pStop)
    {
#ifdef K2_DEBUG_MEM_EX
        if (pHeader->uiTimeStamp >= uiTime)
        {
            if (strlen(pHeader->szFile) != 0)
            {
                map<string, map<uint, uint> >::iterator findit(mapAllocations.find(pHeader->szFile));

                if (findit == mapAllocations.end())
                {
                    mapAllocations.insert(pair<string, map<uint, uint> >(pHeader->szFile, map<uint, uint>()));
                    findit = mapAllocations.find(pHeader->szFile);
                }

                map<uint, uint>::iterator lineit(findit->second.find(pHeader->nLine));

                if (lineit == findit->second.end())
                {
                    findit->second.insert(pair<uint, uint>(pHeader->nLine, 0));
                    lineit = findit->second.find(pHeader->nLine);
                }

                lineit->second += INT_SIZE(pHeader->zSize);
            }
            else
                uiTotalNoFile += INT_SIZE(pHeader->zSize);

            uiTotal += INT_SIZE(pHeader->zSize);
        }
#endif

        if (bFoundHeap)
            pHeader = pHeader->pHeapNext;
        else
            pHeader = pHeader->pNext;
    }

    for (map<string, map<uint, uint> >::iterator fileit(mapAllocations.begin()); fileit != mapAllocations.end(); fileit++)
        for (map<uint, uint>::iterator lineit(fileit->second.begin()); lineit != fileit->second.end(); lineit++)
            Console << _T("File: ") << fileit->first << _T(" Line: ") << lineit->first << _T(" Total Size: ") << lineit->second << newl;

    Console << _T("Allocated not using K2_NEW: ") << uiTotalNoFile << newl;
    Console << _T("Total allocated: ") << uiTotal << newl;
#endif
}


/*====================
  CMemManager::ResetTracking
  ====================*/
void    CMemManager::ResetTracking()
{
    g_cMemMutex.Lock();
    
    m_zTrackAllocs = m_zTrackDeallocs = 0;
    m_zTrackAllocSize = m_zTrackDeallocSize = 0;
    m_uiTimeStamp = Host.GetTime();
    
    SMemHeader *pHeader(m_pTrackTail);
    while (pHeader != nullptr)
    {
        if (pHeader->pTrackPrev != nullptr)
            pHeader->pTrackPrev->pTrackNext = nullptr;
        pHeader->pTrackPrev = nullptr;
        pHeader = pHeader->pTrackNext;
    }

    m_pTrackHead = nullptr;
    m_pTrackTail = nullptr;

    ++m_uiSequence;
    
    g_cMemMutex.Unlock();
}


/*====================
  CMemManager::PrintSequenceAllocations
  ====================*/
void    CMemManager::PrintSequenceAllocations(uint uiSequence)
{
    if (uiSequence >= m_uiSequence)
    {
        Console.Mem << _T("Current sequence is: ") << m_uiSequence << newl;
        return;
    }
    
    g_cMemMutex.Lock();

    SMemHeader *pHeader(m_pTail);

    while (pHeader != nullptr)
    {
        if (pHeader->uiSequence == uiSequence)
            Console.Mem << pHeader->pContext << SPACE << INT_SIZE(pHeader->zSize) << newl;
        pHeader = pHeader->pNext;
    }
    
    g_cMemMutex.Unlock();
}
#endif //K2_DEBUG_MEM


/*====================
  CMemManager::Draw
  ====================*/
void    CMemManager::Draw()
{
    if (!mem_drawInfo)
        return;

    PROFILE("CMemManager::Draw");

#if 0
#ifdef K2_DEBUG_MEM
    ResHandle hMemFont(g_ResourceManager.LookUpName(mem_font, RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hMemFont));
    if (pFontMap == nullptr)
        return;

    const float FONT_WIDTH = pFontMap->GetFixedAdvance();
    const float FONT_HEIGHT = pFontMap->GetMaxHeight();
    const float START_X = FONT_WIDTH;
    const float START_Y = FONT_HEIGHT;

    float iDrawY = START_Y;
    tstring sStr;

    uint uiCount(0);
    for (uint ui(0); ui < MAX_HEAPS; ++ui)
    {
        if (s_apHeaps[ui] != nullptr)
            ++uiCount;
    }

    Draw2D.SetColor(0.2f, 0.2f, 0.2f, 0.5f);
    Draw2D.Rect(START_X - 2, START_Y - 2, FONT_WIDTH * 63 + 4, FONT_HEIGHT * (8.0f + uiCount) + 4.0f);

    Draw2D.SetColor(1.0f, 0.0f, 1.0f, 1.0f);

    sStr = _T("Total allocations: ") + XtoA(m_zTotalAllocs) + SPACE
        + ParenStr(GetByteString(m_zTotalAllocSize)) + _T(" (") + XtoA(m_zTotalAllocSize) + _T(" bytes)");

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("Total deallocations: ") + XtoA(m_zTotalDeallocs) + SPACE
        + ParenStr(GetByteString(m_zTotalDeallocSize)) + _T(" (") + XtoA(m_zTotalDeallocSize) + _T(" bytes)");

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("Active alloations: ") + XtoA(m_zTotalAllocs - m_zTotalDeallocs) + SPACE
        + ParenStr(GetByteString(m_zTotalAllocSize - m_zTotalDeallocSize)) + _T(" (") + XtoA(m_zTotalAllocSize - m_zTotalDeallocSize) + _T(" bytes)");

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("MemoryManager overhead: ") + XtoA(m_zMemoryManagerOverhead) + SPACE + ParenStr(GetByteString(m_zMemoryManagerOverhead));

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("Copies: ") + XtoA(m_zCopyCount) + SPACE + ParenStr(GetByteString(m_zCopyBytes));

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("Moves: ") + XtoA(m_zMoveCount) + SPACE + ParenStr(GetByteString(m_zMoveBytes));

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("Writes: ") + XtoA(m_zWriteCount) + SPACE + ParenStr(GetByteString(m_zWriteBytes));

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    sStr = _T("----------------------------------------");

    Draw2D.String(START_X, iDrawY, sStr, hMemFont);
    iDrawY += FONT_HEIGHT;

    for (uint ui(0); ui < MAX_HEAPS; ++ui)
    {
        CHeap *pHeap(s_apHeaps[ui]);
        if (pHeap == nullptr)
            continue;

        tstring sName(StringToTString(pHeap->GetName()));
        sStr = sName + _T(": ") + GetByteString(INT_SIZE(pHeap->GetAllocSize() - pHeap->GetDeallocSize()))
            + _T(" (") + XtoA(INT_SIZE(pHeap->GetAllocSize() - pHeap->GetDeallocSize())) + _T(" bytes)");

        if (pHeap->HasPool())
            sStr += SPACE + ParenStr(XtoA(pHeap->GetPoolUsage() * 100.0f, 0, 0, 0) + _T("%"));

        Draw2D.String(START_X, iDrawY, sStr, hMemFont);
        iDrawY += FONT_HEIGHT;
    }
#endif //K2_DEBUG_MEM
#endif
}


/*====================
  CMemManager::Validate
  ====================*/
bool    CMemManager::Validate()
{
    return true;
}


/*====================
  CMemManager::GetInstance
  ====================*/
CMemManager*    CMemManager::GetInstance()
{
    assert(!s_bReleased);
    if (s_pInstance == nullptr)
    {
        assert(!s_bRequested);
        s_bRequested = true;
        s_pInstance = (CMemManager*)malloc(sizeof(CMemManager));
#ifdef __GNUC__
        g_pMemManager = s_pInstance;
#endif
        s_pInstance->Init();
    }
    return s_pInstance;
}


/*====================
  CMemManager::Copy
  ====================*/
void*   CMemManager::Copy(void *pDest, const void *pSrc, size_t z)
{
#ifdef K2_DEBUG_MEM
    assert(pDest != nullptr);
    assert(pSrc != nullptr);
    ++m_zCopyCount;
    m_zCopyBytes += z;
#endif //K2_DEBUG_MEM
    return memcpy(pDest, pSrc, z);
}

errno_t CMemManager::Copy_s(void *pDest, size_t dstSize, const void *pSrc, size_t z)
{
#ifdef K2_DEBUG_MEM
    assert(pDest != nullptr);
    assert(pSrc != nullptr);
    ++m_zCopyCount;
    m_zCopyBytes += z;
#endif
    return MEMCPY_S(pDest, dstSize, pSrc, z);
}

/*====================
  CMemManager::Move
  ====================*/
void*   CMemManager::Move(void *pDest, const void *pSrc, size_t z)
{
#ifdef K2_DEBUG_MEM
    assert(pDest != nullptr);
    assert(pSrc != nullptr);
    ++m_zMoveCount;
    m_zMoveBytes += z;
#endif //K2_DEBUG_MEM
    return memmove(pDest, pSrc, z);
}


/*====================
  CMemManager::Set
  ====================*/
void*   CMemManager::Set(void *pDest, byte y, size_t z)
{
#ifdef K2_DEBUG_MEM
    assert(pDest != nullptr);
    ++m_zWriteCount;
    m_zWriteBytes += z;
#endif //K2_DEBUG_MEM
    return memset(pDest, y, z);
}


#ifdef K2_DEBUG_MEM
/*--------------------
  MemValidate
  --------------------*/
CMD(MemValidate)
{
    MemManager.Validate();
    return true;
}


/*--------------------
  MemInfo
  --------------------*/
CMD(MemInfo)
{
    MemManager.PrintStats();
    return true;
}


/*--------------------
  MemTrackInfo
  --------------------*/
CMD(MemTrackInfo)
{
    MemManager.PrintTrackingStats();
    return true;
}


/*--------------------
  MemResetTracking
  --------------------*/
CMD(MemResetTracking)
{
    MemManager.ResetTracking();
    return true;
}


/*--------------------
  MemPrintSequence
  --------------------*/
CMD(MemPrintSequence)
{
    if (vArgList.empty())
        return false;

    MemManager.PrintSequenceAllocations(AtoI(vArgList[0]));
    return true;
}


/*--------------------
  PrintAllocations
  --------------------*/
CMD(PrintAllocations)
{
    if (vArgList.size() >= 2)
        MemManager.PrintAllocations(TStringToString(vArgList[0]).c_str(), AtoI(vArgList[1]));
    else if (!vArgList.empty())
        MemManager.PrintAllocations(TStringToString(vArgList[0]).c_str());
    else
        MemManager.PrintAllocations();
    return true;
}


/*--------------------
  PrintAllocationsNoDuplicates
  --------------------*/
CMD(PrintAllocationsNoDuplicates)
{
    if (vArgList.size() >= 2)
        MemManager.PrintAllocationsNoDuplicates(TStringToString(vArgList[0]).c_str(), AtoI(vArgList[1]));
    else if (!vArgList.empty())
        MemManager.PrintAllocationsNoDuplicates(TStringToString(vArgList[0]).c_str());
    else
        MemManager.PrintAllocationsNoDuplicates();
    return true;
}

#endif //K2_DEBUG_MEM


#ifdef K2_TRACK_MEM
/*--------------------
  MemoryLog
  --------------------*/
CMD(MemoryLog)
{
    if (vArgList.size() < 2)
    {
        Console << _T("MemoryLog <bool logEveryAllocation> <bool logEveryFrame>") << newl;
        return true;
    }

    bool bLogEveryAllocation(AtoB(vArgList[0]));
    bool bLogEveryFrame(AtoB(vArgList[1]));
    assert(physx::shdfnd2::gMemoryTracker != nullptr);
    if (physx::shdfnd2::gMemoryTracker)
        physx::shdfnd2::gMemoryTracker->setLogLevel(bLogEveryAllocation, bLogEveryFrame);

    return true;
}

/*--------------------
  MemoryReport
  --------------------*/
CMD(MemoryReport)
{
    bool bReportAllLeaks(false);
    if (vArgList.size() >= 1)
        bReportAllLeaks = AtoB(vArgList[0]);

    assert(physx::shdfnd2::gMemoryTracker != nullptr);
    if (physx::shdfnd2::gMemoryTracker)
        physx::shdfnd2::gMemoryTracker->detectMemoryLeaks("memreport.html", bReportAllLeaks);

    return true;
}
#endif

#if defined(K2_TRAP_ALLOCS)
/*====================
  operator new
  ====================*/
void*   operator new(size_t z)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, "global", MT_GLOBAL_NEW);
}

void*   operator new(size_t z, const char *szContext, const char *szType)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, szContext, MT_GLOBAL_NEW, szType);
}

void*   operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, szContext, MT_GLOBAL_NEW, szType, szFile, nLine);
}


/*====================
  operator new[]
  ====================*/
void*   operator new[](size_t z)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, "global", MT_GLOBAL_NEW_ARRAY);
}

void*   operator new[](size_t z, const char *szContext, const char *szType)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, szContext, MT_GLOBAL_NEW_ARRAY, szType);
}

void*   operator new[](size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Allocate(z, szContext, MT_GLOBAL_NEW_ARRAY, szType, szFile, nLine);
}


/*====================
  operator delete
  ====================*/
void    operator delete(void *p) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    MemManager.Deallocate(p, "global", MT_GLOBAL_DELETE);
}

void    operator delete(void *p, const char *szContext, const char *szType) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    MemManager.Deallocate(p, szContext, MT_GLOBAL_DELETE);
}

void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Deallocate(p, szContext, MT_GLOBAL_DELETE, szFile, nLine);
}


/*====================
  operator delete[]
  ====================*/
void    operator delete[](void *p) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    MemManager.Deallocate(p, "global", MT_GLOBAL_DELETE_ARRAY);
}

void    operator delete[](void *p, const char *szContext, const char *szType) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    MemManager.Deallocate(p, szContext, MT_GLOBAL_DELETE_ARRAY);
}

void    operator delete[](void *p, const char *szContext, const char *szType, const char *szFile, short nLine) _NOEXCEPT
{
    CMemManager::GetInstance(); // ensure MemManager is initialized
    return MemManager.Deallocate(p, szContext, MT_GLOBAL_DELETE_ARRAY, szFile, nLine);
}

#endif //K2_DEBUG_MEM

