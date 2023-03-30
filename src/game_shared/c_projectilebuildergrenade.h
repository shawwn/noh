// (C)2007 S2 Games
// c_projectilebuildergrenade.h
//
//=============================================================================
#ifndef __C_PROJECTILEBUILDERGRENADE_H__
#define __C_PROJECTILEBUILDERGRENADE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileBuilderGrenade
//=============================================================================
class CProjectileBuilderGrenade : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, BuilderGrenade);

public:
	~CProjectileBuilderGrenade()	{}
	CProjectileBuilderGrenade();
};
//=============================================================================

#endif //__C_PROJECTILEBUILDERGRENADE_H__
