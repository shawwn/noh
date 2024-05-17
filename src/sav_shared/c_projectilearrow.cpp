// (C)2006 S2 Games
// c_projectilearrow.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectilearrow.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Projectile, Arrow);
//=============================================================================

/*====================
  CProjectileArrow::CProjectileArrow
  ====================*/
CProjectileArrow::CProjectileArrow() :
IProjectile(GetEntityConfig())
{
}


/*====================
  CProjectileArrow::ApplyCharge
  ====================*/
void    CProjectileArrow::ApplyCharge(float fValue)
{
    m_v3Velocity *= fValue;
}