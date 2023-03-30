// (C)2006 S2 Games
// c_buildingarrowtower.h
//
//=============================================================================
#ifndef __C_BUILDINGARROWTOWER_H__
#define __C_BUILDINGARROWTOWER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_attackbuildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingArrowTower
//=============================================================================
class CBuildingArrowTower : public IAttackBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, ArrowTower);

public:
	~CBuildingArrowTower()	{}
	CBuildingArrowTower();
};
//=============================================================================

#endif //__C_BUILDINGARROWTOWER_H__
