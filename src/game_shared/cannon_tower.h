// (C)2006 S2 Games
// c_buildingcannontower.h
//
//=============================================================================
#ifndef __C_BUILDINGCANNONTOWER_H__
#define __C_BUILDINGCANNONTOWER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingCannonTower
//=============================================================================
class CBuildingCannonTower : public IBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR(Building, CannonTower);
	DECLARE_ENTITY_CVARS

public:
	~CBuildingCannonTower()	{}
	CBuildingCannonTower()	{}
};
//=============================================================================

#endif //__C_BUILDINGCANNONTOWER_H__
