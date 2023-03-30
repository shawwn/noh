// (C)2008 S2 Games
// c_abilityattributedefinition.h
//
//=============================================================================
#ifndef __C_ABILITYATTRIBUTEDEFINITION_H__
#define __C_ABILITYATTRIBUTEDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_abilitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IEntityAbilityAttribute, AbilityAttribute, abilityattribute)
//=============================================================================

//=============================================================================
// CAbilityAttributeDefinition
//=============================================================================
class CAbilityAttributeDefinition : public CAbilityDefinition
{
public:
    ~CAbilityAttributeDefinition()  {}
    CAbilityAttributeDefinition() :
    CAbilityDefinition(&g_allocatorAbilityAttribute)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(ctx_Game,    CAbilityAttributeDefinition)(*this); }
};
//=============================================================================

#endif //__C_ABILITYATTRIBUTEDEFINITION_H__
