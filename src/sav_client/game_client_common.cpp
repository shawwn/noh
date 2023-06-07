// (C)2005 S2 Games
// game_client_common.cpp
//
// Generates precompiled header for game_client module
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"
//=============================================================================

#ifdef K2_DEBUG_MEM

/*====================
  operator new
  ====================*/
void*   operator new(size_t z)
{
    return MemManager.Allocate(z/*, &MemManager.GetHeap(HEAP_CLIENT_GAME)*/);
}


/*====================
  operator new[]
  ====================*/
void*   operator new[](size_t z)
{
    return MemManager.Allocate(z/*, &MemManager.GetHeap(HEAP_CLIENT_GAME)*/);
}

#endif
