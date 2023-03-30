// (C)2007 S2 Games
// c_statestormshield.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statestormshield.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, StormShield)
//=============================================================================

/*====================
  CStateStormShield::OwnerDamaged
  ====================*/
float   CStateStormShield::OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker)
{
    if (iFlags & (DAMAGE_FLAG_PROJECTILE | DAMAGE_FLAG_GUN))
        return 0.0f;

    return fDamage;
}
