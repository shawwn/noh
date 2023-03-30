// (C)2007 S2 Games
// c_gunimpfire.h
//
//=============================================================================
#ifndef __C_GUNIMPFIRE_H__
#define __C_GUNIMPFIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// CGunImpFire
//=============================================================================
class CGunImpFire: public IGunItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Gun, ImpFire);

	IProjectile*	FireProjectile(const CVec3f &v3Origin, const CVec3f &v3Dir, float fCharge);

public:
	~CGunImpFire()	{}
	CGunImpFire() :
	IGunItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_GUNIMPFIRE_H__

