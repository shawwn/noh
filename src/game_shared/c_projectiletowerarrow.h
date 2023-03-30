// (C)2006 S2 Games
// c_projectiletowerarrow.h
//
//=============================================================================
#ifndef __C_PROJECTILETOWERARROW_H__
#define __C_PROJECTILETOWERARROW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// CProjectileTowerArrow
//=============================================================================
class CProjectileTowerArrow : public IProjectile
{
private:
	DECLARE_ENT_ALLOCATOR2(Projectile, TowerArrow);

public:
	~CProjectileTowerArrow()	{}
	CProjectileTowerArrow();
};
//=============================================================================

#endif //__C_PROJECTILETOWERARROW_H__
