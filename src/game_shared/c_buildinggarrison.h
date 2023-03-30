// (C)2006 S2 Games
// c_buildinggarrison.h
//
//=============================================================================
#ifndef __C_BUILDINGGARISSON_H__
#define __C_BUILDINGGARISSON_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingGarrison
//=============================================================================
class CBuildingGarrison : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, Garrison);

public:
    ~CBuildingGarrison()    {}
    CBuildingGarrison();

    void    Use(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_BUILDINGGARISSON_H__
