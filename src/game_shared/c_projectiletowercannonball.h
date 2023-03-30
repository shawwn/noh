// (C)2007 S2 Games
// c_projectiletowercannonball.h
//
//=============================================================================
#ifndef __C_PROJECTILETOWERCANNONBALL_H__
#define __C_PROJECTILETOWERCANNONBALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileTowerCannonball
//=============================================================================
class CProjectileTowerCannonball : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, TowerCannonball);

public:
	~CProjectileTowerCannonball()	{}
	CProjectileTowerCannonball();
};
//=============================================================================

#endif //__C_PROJECTILETOWERCANNONBALL_H__
