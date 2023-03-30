// (C)2007 S2 Games
// c_projectilevenomspore.h
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
// CProjectileVenomSpore
//=============================================================================
class CProjectileVenomSpore : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, VenomSpore)

public:
	~CProjectileVenomSpore()	{}
	CProjectileVenomSpore() :
	IProjectile(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_PROJECTILEVENOMSPORE_H__
