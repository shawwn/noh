// (C)2007 S2 Games
// c_buildingpredatorden.h
//
//=============================================================================
#ifndef __C_BUILDINGPREDATORDEN_H__
#define __C_BUILDINGPREDATORDEN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingPredatorDen
//=============================================================================
class CBuildingPredatorDen : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, PredatorDen)

public:
    ~CBuildingPredatorDen() {}
    CBuildingPredatorDen() :
    IBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGPREDATORDEN_H__
