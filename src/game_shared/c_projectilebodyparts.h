// (C)2006 S2 Games
// c_projectilebodyparts.h
//
//=============================================================================
#ifndef __C_PROJECTILEBODYPARTS_H__
#define __C_PROJECTILEBODYPARTS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileBodyParts
//=============================================================================
class CProjectileBodyParts : public IProjectile
{
private:
    DECLARE_ENT_ALLOCATOR2(Projectile, BodyParts);

public:
    ~CProjectileBodyParts() {}
    CProjectileBodyParts();
};
//=============================================================================

#endif //__C_PROJECTILEBODYPARTS_H__
