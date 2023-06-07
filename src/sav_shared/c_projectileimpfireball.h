// (C)2007 S2 Games
// c_projectileimpfireball.h
//
//=============================================================================
#ifndef __C_PROJECTILEIMPFIREBALL_H__
#define __C_PROJECTILEIMPFIREBALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileImpFireball
//=============================================================================
class CProjectileImpFireball : public IProjectile
{
private:
    DECLARE_ENT_ALLOCATOR2(Projectile, ImpFireball);

public:
    ~CProjectileImpFireball()   {}
    CProjectileImpFireball();
};
//=============================================================================

#endif //__C_PROJECTILEIMPFIREBALL_H__
