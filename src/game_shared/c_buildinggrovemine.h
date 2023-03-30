// (C)2007 S2 Games
// c_buildinggrovemine.h
//
//=============================================================================
#ifndef __C_BUILDINGGROVEMINE_H__
#define __C_BUILDINGGROVEMINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingmine.h"
//=============================================================================

//=============================================================================
// CBuildingGroveMine
//=============================================================================
class CBuildingGroveMine : public IBuildingMine
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, GroveMine)

public:
	~CBuildingGroveMine()	{}
	CBuildingGroveMine() :
	IBuildingMine(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_BUILDINGGROVEMINE_H__
