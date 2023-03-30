// (C)2007 S2 Games
// c_buildingentanglespire.h
//
//=============================================================================
#ifndef __C_BUILDINGENTANGLESPIRE_H__
#define __C_BUILDINGENTANGLESPIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_attackbuildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingEntangleSpire
//=============================================================================
class CBuildingEntangleSpire : public IAttackBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, EntangleSpire)

public:
    ~CBuildingEntangleSpire()   {}
    CBuildingEntangleSpire() :
    IAttackBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGENTANGLESPIRE_H__
