// (C)2006 S2 Games
// c_projectilegrenade.h
//
//=============================================================================
#ifndef __C_PROJECTILEGRENADE_H__
#define __C_PROJECTILEGRENADE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileGrenade
//=============================================================================
class CProjectileGrenade : public IProjectile
{
private:
    DECLARE_ENT_ALLOCATOR2(Projectile, Grenade);

public:
    ~CProjectileGrenade()   {}
    CProjectileGrenade();
};
//=============================================================================

#endif //__C_PROJECTILEGRENADE_H__
