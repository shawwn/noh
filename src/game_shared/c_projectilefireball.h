// (C)2007 S2 Games
// c_projectilefireball.h
//
//=============================================================================
#ifndef __C_PROJECTILEFIREBALL_H__
#define __C_PROJECTILEFIREBALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileFireball
//=============================================================================
class CProjectileFireball : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, Fireball)

public:
	~CProjectileFireball()	{}
	CProjectileFireball() :
	IProjectile(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_PROJECTILEFIREBALL_H__
