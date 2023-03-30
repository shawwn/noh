// (C)2008 S2 Games
// c_critterdefinition.h
//
//=============================================================================
#ifndef __C_CRITTERDEFINITION_H__
#define __C_CRITTERDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(ICritterEntity, Critter, critter)
//=============================================================================

//=============================================================================
// CCritterDefinition
//=============================================================================
class CCritterDefinition : public IUnitDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

public:
    ~CCritterDefinition()   {}
    CCritterDefinition() :
    IUnitDefinition(&g_allocatorCritter)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CCritterDefinition)(*this); }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        IUnitDefinition::GetPrecacheList(eScheme, deqPrecache);

        PRECACHE_GUARD
            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }
};
//=============================================================================

#endif //__C_CRITTERDEFINITION_H__
