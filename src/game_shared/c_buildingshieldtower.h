// (C)2006 S2 Games
// c_buildingshieldtower.h
//
//=============================================================================
#ifndef __C_BUILDINGSHIELDTOWER_H__
#define __C_BUILDINGSHIELDTOWER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingShieldTower
//=============================================================================
class CBuildingShieldTower : public IBuildingEntity
{
private:
	
	DECLARE_ENT_ALLOCATOR2(Building, ShieldTower);

public:
	~CBuildingShieldTower()	{}
	CBuildingShieldTower();


};
//=============================================================================

#endif //__C_BUILDINGSHIELDTOWER_H__
