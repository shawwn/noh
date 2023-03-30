// (C)2006 S2 Games
// c_buildingmonastery.h
//
//=============================================================================
#ifndef __C_BUILDINGMONASTERY_H__
#define __C_BUILDINGMONASTERY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingMonastery
//=============================================================================
class CBuildingMonastery : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, Monastery);

public:
    ~CBuildingMonastery()   {}
    CBuildingMonastery();
};
//=============================================================================

#endif //__C_BUILDINGMONASTERY_H__
