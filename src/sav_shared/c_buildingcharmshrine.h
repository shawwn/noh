// (C)2007 S2 Games
// c_buildingcharmshrine.h
//
//=============================================================================
#ifndef __C_BUILDINGCHARMSHRINE_H__
#define __C_BUILDINGCHARMSHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingCharmShrine
//=============================================================================
class CBuildingCharmShrine : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, CharmShrine)

public:
    ~CBuildingCharmShrine() {}
    CBuildingCharmShrine() :
    IBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGCHARMSHRINE_H__
