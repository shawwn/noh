// (C)2006 S2 Games
// c_gadgetshieldgeneratorshield.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetshieldgeneratorshield.h"

#include "../k2/c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, ShieldGeneratorShield)
//=============================================================================

/*====================
  CGadgetShieldGeneratorShield::CGadgetShieldGeneratorShield
  ====================*/
CGadgetShieldGeneratorShield::CGadgetShieldGeneratorShield() :
IGadgetEntity(GetEntityConfig())
{
}
