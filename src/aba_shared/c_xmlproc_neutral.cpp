// (C)2008 S2 Games
// c_xmlproc_neutral.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_neutraldefinition.h"
#include "i_neutralentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CNeutralDefinition, ENTITY_BASE_TYPE_NEUTRAL, Neutral)

START_ENTITY_DEFINITION_XML_PROCESSOR(INeutralEntity, Neutral)
	IUnitDefinition::ReadSettings(pDefinition, node, bMod);
	
	READ_ENTITY_DEFINITION_PROPERTY_EX(IsMobile, ismobile, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanAttack, canattack, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanRotate, canrotate, true)
	READ_ENTITY_DEFINITION_PROPERTY(CombatType, combattype)
	READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)
END_ENTITY_DEFINITION_XML_PROCESSOR(Neutral, neutral)
