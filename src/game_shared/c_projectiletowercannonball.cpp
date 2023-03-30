// (C)2007 S2 Games
// c_projectiletowercannonball.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectiletowercannonball.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Projectile, TowerCannonball);
//=============================================================================

/*====================
  CProjectileTowerCannonball::CProjectileTowerCannonball
  ====================*/
CProjectileTowerCannonball::CProjectileTowerCannonball() :
IProjectile(GetEntityConfig())
{
}
