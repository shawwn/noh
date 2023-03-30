// (C)2007 S2 Games
// c_projectilefrostbolt.h
//
//=============================================================================
#ifndef __C_PROJECTILEFROSTBOLT_H__
#define __C_PROJECTILEFROSTBOLT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileFrostBolt
//=============================================================================
class CProjectileFrostBolt : public IProjectile
{
private:
    DECLARE_ENT_ALLOCATOR2(Projectile, FrostBolt)

public:
    ~CProjectileFrostBolt() {}
    CProjectileFrostBolt() :
    IProjectile(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_PROJECTILEFROSTBOLT_H__
