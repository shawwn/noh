// (C)2006 S2 Games
// c_buildinghumanhellshrine.h
//
//=============================================================================
#ifndef __C_BUILDINGHUMANHELLSHRINE_H__
#define __C_BUILDINGHUMANHELLSHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingHumanHellShrine
//=============================================================================
class CBuildingHumanHellShrine : public IBuildingEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Building, HumanHellShrine);

public:
    ~CBuildingHumanHellShrine() {}
    CBuildingHumanHellShrine();

    virtual void Spawn();
    virtual void Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    void    Use(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_BUILDINGHUMANHELLSHRINE_H__
