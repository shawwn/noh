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
//=============================================================================

//=============================================================================
// Declarations / Definitions
//=============================================================================
#ifdef K2_DEBUG_MEM
CMemManager	*g_pMemManager(CMemManager::GetInstance());
#else
CMemManager	*g_pMemManager(NULL);
#endif

CVAR_STRINGF	(mem_font,			"system_medium",		CVAR_SAVECONFIG);
CVAR_BOOL		(mem_drawInfo,		false);
CVAR_UINT		(mem_BreakHeap,		uint(-1));
CVAR_UINT		(mem_BreakSize,		uint(-1));
CVAR_UINT		(mem_BreakTag,		uint(-1));


K2_API CMemManager	*CMemManager::s_pInstance;
K2_API bool		CMemManager::s_bReleased;
K2_API bool		CMemManager::s_bRequested;
K2_API bool		CMemManager::s_bTrackAllocs = true;

K2_API MICRO_ALLOCATOR::HeapManager*		CMemManager::m_cMicroHeapManager;

// note: these must be std::maps so that the memory tracking does not cause a stack overflow!
typedef std::map<size_t, char*>				StringMap;
typedef std::map<const char*, char*>			PtrStringMap;
static StringMap						m_mapStrings;
static PtrStringMap						m_mapPtrStrings;
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
const char*		CMemManager::GetStr(const char* sStr)
{
	if (physx::shdfnd2::gMemoryTracker == NULL)
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
void	CMemManager::Init()
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

	m_pHead = NULL;
	m_pTail = NULL;

	m_pTrackHead = NULL;
	m_pTrackTail = NULL;
#endif //K2_DEBUG_MEM
}


/*====================
  CMemManager::Release
  ====================*/
void	CMemManager::Release()
{
	if (m_cMicroHeapManager)
	{
		MICRO_ALLOCATOR::releaseHeapManager(m_cMicroHeapManager);
		m_cMicroHeapManager = NULL;
	}

	assert(!s_bReleased);
	if (s_pInstance != NULL)
		free(s_pInstance);
}


/*====================
  CMemManager::Frame
  ====================*/
void	CMemManager::Frame()
{
#ifdef K2_TRACK_MEM
	TRACK_FRAME();
#endif
}


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::Allocate
  ====================*/
void*	CMemManager::Allocate(size_t z, const char *szContext, MemoryType eMemType, const char *szType, const char *szFile, short nLine)
{
#if 0
	if (CSystem::IsInitialized())
		assert(K2System.GetCurrentThread() == K2System.GetMainThread());
#endif

	PROFILE("CMemManager::Allocate");

	if (szContext == NULL)
		szContext = "stl";

	//Validate();

	/*
	if (K2System.IsInitialized() && pHeap->GetID() == mem_BreakHeap)
		K2System.DebugBreak();
	if (K2System.IsInitialized() && z >= mem_BreakSize)
		K2System.DebugBreak();
	/**/

	char *p = (char*)malloc(sizeof(SMemHeader) + z + sizeof(MEM_END_TAG));
	assert (p != NULL);

	SMemHeader *pe = (SMemHeader*)p;
	pe->uiMarker = MEM_START_TAG;
#ifdef K2_DEBUG_MEM_EX
	Set(pe->szFile, 0, MEM_DEBUG_MAX_FILE_NAME_LENGTH);
	if (szFile != NULL)
		STRNCPY_S(pe->szFile, MEM_DEBUG_MAX_FILE_NAME_LENGTH, szFile, _TRUNCATE);
	pe->nLine = nLine;
	pe->uiTimeStamp = CHost::IsAllocated() ? Host.GetTime() : 0;
#endif //K2_DEBUG_MEM_EX
	pe->zSize = z;
	pe->pContext = szContext;

	if (m_pHead != NULL)
		m_pHead->pNext = pe;
	pe->pPrev = m_pHead;
	pe->pNext = NULL;
	m_pHead = pe;
	if (m_pTail == NULL)
		m_pTail = m_pHead;

	if (m_pTrackHead != NULL)
		m_pTrackHead->pTrackNext = pe;
	pe->pTrackPrev = m_pTrackHead;
	pe->pTrackNext = NULL;
	m_pTrackHead = pe;
	if (m_pTrackTail == NULL)
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

	return p;
}
#endif


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::Deallocate
  ====================*/
void	CMemManager::Deallocate(void *p, const char *szContext, MemoryType eMemType, const char *szFile, short nLine)
{
#if 0
	if (CSystem::IsInitialized())
		assert(K2System.GetCurrentThread() == K2System.GetMainThread());
#endif

	PROFILE("CMemManager::Deallocate");

	if (!p) // Microsoft documentation says deallocate should simply ignore NULL pointers
		return;

	//Validate();

	SMemHeader *pe = (SMemHeader*)((char*)p - sizeof(SMemHeader));

	assert(pe->uiMarker == MEM_START_TAG);

	/*
	if (K2System.IsInitialized() && pe->uiSequence == mem_BreakTag)
		K2System.DebugBreak();
	/**/

	if (pe->pNext != NULL)
		pe->pNext->pPrev = pe->pPrev;
	if (pe->pPrev != NULL)
		pe->pPrev->pNext = pe->pNext;
	if (m_pHead == pe)
		m_pHead = pe->pPrev;
	if (m_pTail == pe)
		m_pTail = pe->pNext;

	if (pe->pTrackNext != NULL)
		pe->pTrackNext->pTrackPrev = pe->pTrackPrev;
	if (pe->pTrackPrev != NULL)
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
}
#endif


#ifdef K2_DEBUG_MEM
/*====================
  CMemManager::PrintStats
  ====================*/
void	CMemManager::PrintStats()
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
void	CMemManager::PrintTrackingStats()
{
	size_t zAllocs(m_zTrackAllocs);
	size_t zDeallocs(m_zTrackDeallocs);
	size_t zAllocSize(m_zTrackAllocSize);
	size_t zDeallocSize(m_zTrackDeallocSize);

	/*
	SMemHeader *pHeader(m_pTrackTail);
	SMemHeader *pHeaderEnd(m_pTrackHead);

	while (pHeader != NULL && pHeader != pHeaderEnd)
	{
		Console.Mem << _T("#") << pHeader->uiSequence << SPACE << pHeader->pHeap->GetName() << SPACE << INT_SIZE(pHeader->zSize) << newl;
		pHeader = pHeader->pTrackNext;
	}
	/**/

	Console.Mem << _T("Total allocations: ") << INT_SIZE(zAllocs) << _T(" (") << INT_SIZE(zAllocSize) << _T(" bytes)") << newl
		<< _T("Total deallocations: ") << INT_SIZE(zDeallocs) << _T(" (") << INT_SIZE(zDeallocSize) << _T(" bytes)") << newl
		<< _T("Active alloations: ") << INT_SIZE(zAllocs - zDeallocs) << _T(" (") << INT_SIZE(zAllocSize - zDeallocSize) << _T(" bytes)") << newl;
}


/*====================
  CMemManager::PrintAllocations
  ====================*/
void	CMemManager::PrintAllocations(const char *szHeapName, uint uiTime)
{
#if 0
	SMemHeader *pHeader(m_pTail);
	SMemHeader *pStop(m_pHead);
	uint uiTotal(0);
	bool bFoundHeap(false);
	if (szHeapName != NULL && strlen(szHeapName) > 0)
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

	if (szHeapName != NULL && !bFoundHeap)
		return;

	if (uiTime == -1)
		uiTime = m_uiTimeStamp;

	while (pHeader != NULL && pHeader != pStop)
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
void	CMemManager::PrintAllocationsNoDuplicates(const char *szHeapName, uint uiTime)
{
#if 0
	SMemHeader *pHeader(m_pTail);
	SMemHeader *pStop(m_pHead);
	uint uiTotal(0);
	uint uiTotalNoFile(0);
	bool bFoundHeap(false);
	if (szHeapName != NULL && strlen(szHeapName) > 0)
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

	if (szHeapName != NULL && !bFoundHeap)
		return;

	if (uiTime == -1)
		uiTime = m_uiTimeStamp;

	map<string, map<uint, uint> > mapAllocations;

	while (pHeader != NULL && pHeader != pStop)
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
void	CMemManager::ResetTracking()
{
	m_zTrackAllocs = m_zTrackDeallocs = 0;
	m_zTrackAllocSize = m_zTrackDeallocSize = 0;
	m_uiTimeStamp = Host.GetTime();
	
	SMemHeader *pHeader(m_pTrackTail);
	while (pHeader != NULL)
	{
		if (pHeader->pTrackPrev != NULL)
			pHeader->pTrackPrev->pTrackNext = NULL;
		pHeader->pTrackPrev = NULL;
		pHeader = pHeader->pTrackNext;
	}

	m_pTrackHead = NULL;
	m_pTrackTail = NULL;

	++m_uiSequence;
}


/*====================
  CMemManager::PrintSequenceAllocations
  ====================*/
void	CMemManager::PrintSequenceAllocations(uint uiSequence)
{
	if (uiSequence >= m_uiSequence)
	{
		Console.Mem << _T("Current sequence is: ") << m_uiSequence << newl;
		return;
	}

	SMemHeader *pHeader(m_pTail);

	while (pHeader != NULL)
	{
		if (pHeader->uiSequence == uiSequence)
			Console.Mem << pHeader->pContext << SPACE << INT_SIZE(pHeader->zSize) << newl;
		pHeader = pHeader->pNext;
	}
}
#endif //K2_DEBUG_MEM


/*====================
  CMemManager::Draw
  ====================*/
void	CMemManager::Draw()
{
	if (!mem_drawInfo)
		return;

	PROFILE("CMemManager::Draw");

#if 0
#ifdef K2_DEBUG_MEM
	ResHandle hMemFont(g_ResourceManager.LookUpName(mem_font, RES_FONTMAP));
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(hMemFont));
	if (pFontMap == NULL)
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
		if (s_apHeaps[ui] != NULL)
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
		if (pHeap == NULL)
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
bool	CMemManager::Validate()
{
	return true;
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
	assert(physx::shdfnd2::gMemoryTracker != NULL);
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

	assert(physx::shdfnd2::gMemoryTracker != NULL);
	if (physx::shdfnd2::gMemoryTracker)
		physx::shdfnd2::gMemoryTracker->detectMemoryLeaks("memreport.html", bReportAllLeaks);

	return true;
}
#endif
