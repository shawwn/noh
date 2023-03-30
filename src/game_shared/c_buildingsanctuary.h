// (C)2007 S2 Games
// c_buildingsanctuary.h
//
//=============================================================================
#ifndef __C_BUILDINGSANCTUARY_H__
#define __C_BUILDINGSANCTUARY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingSanctuary
//=============================================================================
class CBuildingSanctuary : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, Sanctuary)

public:
    ~CBuildingSanctuary()   {}
    CBuildingSanctuary() :
    IBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGSANCTUARY_H__
