// (C)2007 S2 Games
// c_buildingnexus.h
//
//=============================================================================
#ifndef __C_BUILDINGNEXUS_H__
#define __C_BUILDINGNEXUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingNexus
//=============================================================================
class CBuildingNexus : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, Nexus)

public:
    ~CBuildingNexus()   {}
    CBuildingNexus() :
    IBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGNEXUS_H__
