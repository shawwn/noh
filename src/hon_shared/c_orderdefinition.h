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

protected:
    virtual void    PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        IEntityDefinition::PrecacheV(eScheme, sModifier);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
    {
        IEntityDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);

        PRECACHE_GUARD
            // ...
        PRECACHE_GUARD_END
    }

public:
    ~COrderDefinition() {}
    COrderDefinition() :
    IEntityDefinition(&g_allocatorOrder)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(ctx_Game,    COrderDefinition)(*this); }

    virtual void    ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_ORDERDEFINITION_H__
