// (C)2008 S2 Games
// c_powerupdefinition.h
//
//=============================================================================
#ifndef __C_POWERUPDEFINITION_H__
#define __C_POWERUPDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IPowerupEntity, Powerup, powerup)
//=============================================================================

//=============================================================================
// CPowerupDefinition
//=============================================================================
class CPowerupDefinition : public IUnitDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_RESOURCE_PROPERTY(TouchSound, Sample)
    ENT_DEF_ARRAY_PROPERTY_EX(TouchTargetScheme, uint, Game.LookupTargetScheme)

public:
    ~CPowerupDefinition()   {}
    CPowerupDefinition() :
    IUnitDefinition(&g_allocatorPowerup)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CPowerupDefinition)(*this); }

    virtual void    Precache(EPrecacheScheme eScheme)
    {
        IUnitDefinition::Precache(eScheme);

        PRECACHE_GUARD
            PrecacheTouchSound();
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

#endif //__C_POWERUPDEFINITION_H__
