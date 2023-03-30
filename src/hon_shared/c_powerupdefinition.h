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

protected:
    virtual void    PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        IUnitDefinition::PrecacheV(eScheme, sModifier);

        PRECACHE_GUARD
            PrecacheTouchSound();
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
    {
        IUnitDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

public:
    ~CPowerupDefinition()   {}
    CPowerupDefinition() :
    IUnitDefinition(&g_allocatorPowerup)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(ctx_Game,    CPowerupDefinition)(*this); }
};
//=============================================================================

#endif //__C_POWERUPDEFINITION_H__
