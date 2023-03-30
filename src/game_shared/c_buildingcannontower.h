// (C)2006 S2 Games
// c_buildingcannontower.h
//
//=============================================================================
#ifndef __C_BUILDINGCANNONTOWER_H__
#define __C_BUILDINGCANNONTOWER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_attackbuildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingCannonTower
//=============================================================================
class CBuildingCannonTower : public IAttackBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, CannonTower);

public:
	~CBuildingCannonTower()	{}
	CBuildingCannonTower();
};
//=============================================================================

#endif //__C_BUILDINGCANNONTOWER_H__
