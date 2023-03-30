// (C)2007 S2 Games
// c_buildingsublair.h
//
//=============================================================================
#ifndef __C_BUILDINGSUBLAIR_H__
#define __C_BUILDINGSUBLAIR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingSubLair
//=============================================================================
class CBuildingSubLair : public IBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, SubLair)

public:
	~CBuildingSubLair()	{}
	CBuildingSubLair() :
	IBuildingEntity(GetEntityConfig())
	{}

	void	Use(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_BUILDINGSUBLAIR_H__
