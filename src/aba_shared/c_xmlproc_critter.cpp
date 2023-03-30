// (C)2008 S2 Games
// c_xmlproc_critter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_critterdefinition.h"
#include "i_critterentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CCritterDefinition, ENTITY_BASE_TYPE_CRITTER, Critter)

START_ENTITY_DEFINITION_XML_PROCESSOR(ICritterEntity, Critter)
	IUnitDefinition::ReadSettings(pDefinition, node, bMod);
	
	READ_ENTITY_DEFINITION_PROPERTY_EX(IsMobile, ismobile, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanAttack, canattack, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanRotate, canrotate, true)
	READ_ENTITY_DEFINITION_PROPERTY(CombatType, combattype)
	READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)
END_ENTITY_DEFINITION_XML_PROCESSOR(Critter, critter)
