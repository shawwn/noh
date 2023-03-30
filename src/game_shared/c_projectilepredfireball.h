// (C)2007 S2 Games
// c_projectilepredfireball.h
//
//=============================================================================
#ifndef __C_PROJECTILEPREDFIREBALL_H__
#define __C_PROJECTILEPREDFIREBALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectilePredFireball
//=============================================================================
class CProjectilePredFireball : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, PredFireball)

public:
	~CProjectilePredFireball()	{}
	CProjectilePredFireball() :
	IProjectile(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_PROJECTILEPREDFIREBALL_H__
