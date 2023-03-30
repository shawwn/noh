// (C)2005 S2 Games
// c_heap.h
//
//=============================================================================
#ifndef __C_HEAP_H__
#define __C_HEAP_H__

//=============================================================================
// Definitions
//=============================================================================
const uint MEM_DEBUG_MAX_FILE_NAME_LENGTH(48);

struct SMemHeader
{
	uint				uiSequence;
	uint				uiMarker;
	size_t				zSize;
#ifdef K2_DEBUG_MEM_EX
	char				szFile[MEM_DEBUG_MAX_FILE_NAME_LENGTH];
	short				nLine;
	uint				uiTimeStamp;
#endif //K2_DEBUG_MEM_EX
	const char			*pContext;
	struct SMemHeader	*pPrev;
	struct SMemHeader	*pNext;
	struct SMemHeader	*pTrackPrev;
	struct SMemHeader	*pTrackNext;
};
//=============================================================================

#endif //__C_HEAP_H__
