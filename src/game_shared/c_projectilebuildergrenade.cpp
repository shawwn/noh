// (C)2007 S2 Games
// c_projectilebuildergrenade.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectilebuildergrenade.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Projectile, BuilderGrenade)
//=============================================================================

/*====================
  CProjectileBuilderGrenade::CProjectileBuilderGrenade
  ====================*/
CProjectileBuilderGrenade::CProjectileBuilderGrenade() :
IProjectile(GetEntityConfig())
{
}
