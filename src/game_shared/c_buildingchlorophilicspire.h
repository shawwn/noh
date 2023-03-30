// (C)2007 S2 Games
// c_buildingchlorophilicspire.h
//
//=============================================================================
#ifndef __C_BUILDINGCHLOROPHILICSPIRE_H__
#define __C_BUILDINGCHLOROPHILICSPIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingChlorophilicSpire
//=============================================================================
class CBuildingChlorophilicSpire : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, ChlorophilicSpire)
    

public:
    ~CBuildingChlorophilicSpire()   {}
    CBuildingChlorophilicSpire() :
    IBuildingEntity(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_BUILDINGCHLOROPHILICSPIRE_H__
