// (C)2008 S2 Games
// c_xmlproc_ability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_abilitydefinition.h"
#include "i_entityability.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CAbilityDefinition, ENTITY_BASE_TYPE_ABILITY, Ability)

/*====================
  CAbilityDefinition::ReadSettings
  ====================*/
void	CAbilityDefinition::ReadSettings(CAbilityDefinition *pDefinition, const CXMLNode &node, bool bMod)
{
	IToolDefinition::ReadSettings(pDefinition, node, bMod);

	READ_ENTITY_DEFINITION_PROPERTY(RequiredLevel, requiredlevel)
	READ_ENTITY_DEFINITION_PROPERTY(Interface, interface)
	READ_ENTITY_DEFINITION_PROPERTY_EX(SubSlot, subslot, -1)
	READ_ENTITY_DEFINITION_PROPERTY_EX(KeySlot, keyslot, -1)
	READ_ENTITY_DEFINITION_PROPERTY(NoSilence, nosilence)
}

ENTITY_DEF_MERGE_START(CAbilityDefinition, IToolDefinition)
	MERGE_ARRAY_PROPERTY(RequiredLevel)
	MERGE_STRING_ARRAY_PROPERTY(Interface)
	MERGE_PROPERTY(SubSlot)
	MERGE_PROPERTY(KeySlot)
	MERGE_ARRAY_PROPERTY(NoSilence)
ENTITY_DEF_MERGE_END

START_ENTITY_DEFINITION_XML_PROCESSOR(IEntityAbility, Ability)
	CAbilityDefinition::ReadSettings(pDefinition, node, bMod);
END_ENTITY_DEFINITION_XML_PROCESSOR(Ability, ability)
