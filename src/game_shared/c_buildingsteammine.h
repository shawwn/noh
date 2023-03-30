// (C)2006 S2 Games
// c_buildingsteammine.h
//
//=============================================================================
#ifndef __C_BUILDINGSTEAMMINE_H__
#define __C_BUILDINGSTEAMMINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingmine.h"
//=============================================================================

//=============================================================================
// CBuildingSteamMine
//=============================================================================
class CBuildingSteamMine : public IBuildingMine
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, SteamMine);

public:
	~CBuildingSteamMine()	{}
	CBuildingSteamMine() :
	IBuildingMine(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_BUILDINGSTEAMMINE_H__
