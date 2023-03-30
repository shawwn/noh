// (C)2006 S2 Games
// c_buildingstronghold.h
//
//=============================================================================
#ifndef __C_BUILDINGSTRONGHOLD_H__
#define __C_BUILDINGSTRONGHOLD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingStronghold
//=============================================================================
class CBuildingStronghold : public IBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, Stronghold);

public:
	~CBuildingStronghold()	{}
	CBuildingStronghold();

	virtual bool	IsCommandCenter() const		{ return true; }

	bool	Use(IGameEntity *pActivator)	{ return PlayerEnter(pActivator); }
};
//=============================================================================

#endif //__C_BUILDINGSTRONGHOLD_H__
