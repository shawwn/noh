// (C)2006 S2 Games
// c_projectilearrow.h
//
//=============================================================================
#ifndef __C_PROJECTILEARROW_H__
#define __C_PROJECTILEARROW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileArrow
//=============================================================================
class CProjectileArrow : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, Arrow);

public:
	~CProjectileArrow()	{}
	CProjectileArrow();

	void	ApplyCharge(float fValue);
};
//=============================================================================

#endif //__C_PROJECTILEARROW_H__
