// (C)2008 S2 Games
// c_buildingdefinition.h
//
//=============================================================================
#ifndef __C_BUILDINGDEFINITION_H__
#define __C_BUILDINGDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IBuildingEntity, Building, building)
//=============================================================================

//=============================================================================
// CBuildingDefinition
//=============================================================================
class CBuildingDefinition : public IUnitDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_PROPERTY(IsShop, bool)
    ENT_DEF_PROPERTY(IsBase, bool)
    ENT_DEF_PROPERTY(IsTower, bool)
    ENT_DEF_PROPERTY(IsRax, bool)

    ENT_DEF_RESOURCE_PROPERTY(LowHealthEffect, Effect)
    ENT_DEF_RESOURCE_PROPERTY(LowHealthSound, Sample)
    ENT_DEF_RESOURCE_PROPERTY(DestroyedSound, Sample)

    ENT_DEF_STRING_PROPERTY(DefaultShop)

public:
    ~CBuildingDefinition()  {}
    CBuildingDefinition() :
    IUnitDefinition(&g_allocatorBuilding)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CBuildingDefinition)(*this); }

    virtual void    Precache(EPrecacheScheme eScheme)
    {
        IUnitDefinition::Precache(eScheme);

        PRECACHE_GUARD
            PrecacheLowHealthEffect();
            PrecacheLowHealthSound();
            PrecacheDestroyedSound();
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        IUnitDefinition::GetPrecacheList(eScheme, deqPrecache);
        
        PRECACHE_GUARD
            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }
};
//=============================================================================

#endif //__C_BUILDINGDEFINITION_H__
