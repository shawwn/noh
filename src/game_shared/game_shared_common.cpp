// (C)2005 S2 Games
// game_shared_common.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CHeap	g_heapSharedGame("_game_shared");
//=============================================================================

#ifdef K2_DEBUG_MEM

/*====================
  operator new
  ====================*/
void*	operator new(size_t z)
{
	return MemManager.Allocate(z, &g_heapSharedGame);
}


/*====================
  operator new[]
  ====================*/
void*	operator new[](size_t z)
{
	return MemManager.Allocate(z, &g_heapSharedGame);
}

void*	operator new[](size_t z, CHeap *pHeap)
{
	return MemManager.Allocate(z, pHeap);
}

#endif
