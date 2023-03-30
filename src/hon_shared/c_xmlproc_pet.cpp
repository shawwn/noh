// (C)2008 S2 Games
// c_xmlproc_pet.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_petdefinition.h"
#include "i_petentity.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CPetDefinition, ENTITY_BASE_TYPE_PET, Pet)

START_ENTITY_DEFINITION_XML_PROCESSOR(IPetEntity, Pet)
	IUnitDefinition::ReadSettings(pDefinition, node, bMod);
	
	READ_ENTITY_DEFINITION_PROPERTY_EX(IsMobile, ismobile, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanAttack, canattack, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanRotate, canrotate, true)
	READ_ENTITY_DEFINITION_PROPERTY(CombatType, combattype)
	READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)

	READ_ENTITY_DEFINITION_PROPERTY(IsPersistent, ispersistent)
	READ_ENTITY_DEFINITION_PROPERTY(Lifetime, lifetime)

	READ_ENTITY_DEFINITION_PROPERTY_EX(DefaultBehavior, defaultbehavior, guard)
END_ENTITY_DEFINITION_XML_PROCESSOR(Pet, pet)
