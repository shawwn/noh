// (C)2007 S2 Games
// c_projectilevenusspore.h
//
//=============================================================================
#ifndef __C_PROJECTILEVENOMSPORE_H__
#define __C_PROJECTILEVENOMSPORE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileVenusSpore
//=============================================================================
class CProjectileVenusSpore : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, VenusSpore)

public:
	~CProjectileVenusSpore()	{}
	CProjectileVenusSpore() :
	IProjectile(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_PROJECTILEVENUSSPORE_H__
