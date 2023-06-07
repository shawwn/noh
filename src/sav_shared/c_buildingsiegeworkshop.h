// (C)2006 S2 Games
// c_buildingsiegeworkshop.h
//
//=============================================================================
#ifndef __C_BUILDINGSIEGEWORKSHOP_H__
#define __C_BUILDINGSIEGEWORKSHOP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingSiegeWorkshop
//=============================================================================
class CBuildingSiegeWorkshop : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, SiegeWorkshop);

public:
    ~CBuildingSiegeWorkshop()   {}
    CBuildingSiegeWorkshop();
};
//=============================================================================

#endif //__C_BUILDINGSIEGEWORKSHOP_H__
