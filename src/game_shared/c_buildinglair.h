// (C)2007 S2 Games
// c_buildinglair.h
//
//=============================================================================
#ifndef __C_BUILDINGLAIR_H__
#define __C_BUILDINGLAIR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingLair
//=============================================================================
class CBuildingLair : public IBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, Lair)

public:
	~CBuildingLair()	{}
	CBuildingLair() :
	IBuildingEntity(GetEntityConfig())
	{}

	virtual bool	IsCommandCenter() const		{ return true; }

	void	Use(IGameEntity *pActivator)		{ PlayerEnter(pActivator); }
};
//=============================================================================

#endif //__C_BUILDINGLAIR_H__
