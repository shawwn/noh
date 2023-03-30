// (C)2008 S2 Games
// c_creepdefinition.h
//
//=============================================================================
#ifndef __C_CREEPDEFINITION_H__
#define __C_CREEPDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitdefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(ICreepEntity, Creep, creep)
//=============================================================================

//=============================================================================
// CCreepDefinition
//=============================================================================
class CCreepDefinition : public IUnitDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

public:
    ~CCreepDefinition() {}
    CCreepDefinition() :
    IUnitDefinition(&g_allocatorCreep)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CCreepDefinition)(*this); }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        IUnitDefinition::GetPrecacheList(eScheme, deqPrecache);

        PRECACHE_GUARD
            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }
};
//=============================================================================

#endif //__C_CREEPDEFINITION_H__
