// (C)2009 S2 Games
// c_orderdefinition.h
//
//=============================================================================
#ifndef __C_ORDERDEFINITION_H__
#define __C_ORDERDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IOrderEntity, Order, order)
//=============================================================================

//=============================================================================
// COrderDefinition
//=============================================================================
class COrderDefinition : public IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_ARRAY_PROPERTY(TriggerRange, float)

public:
    ~COrderDefinition() {}
    COrderDefinition() :
    IEntityDefinition(&g_allocatorOrder)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    COrderDefinition)(*this); }

    virtual void    Precache(EPrecacheScheme eScheme)
    {
        IEntityDefinition::Precache(eScheme);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        IEntityDefinition::GetPrecacheList(eScheme, deqPrecache);

        PRECACHE_GUARD
            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }

    virtual void    ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_ORDERDEFINITION_H__
