// (C)2008 S2 Games
// c_xmlproc_gadget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetdefinition.h"
#include "i_gadgetentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CGadgetDefinition, ENTITY_BASE_TYPE_GADGET, Gadget)

START_ENTITY_DEFINITION_XML_PROCESSOR(IGadgetEntity, Gadget)
	IUnitDefinition::ReadSettings(pDefinition, node, bMod);

	READ_ENTITY_DEFINITION_PROPERTY(IsMobile, ismobile)
	READ_ENTITY_DEFINITION_PROPERTY(CanAttack, canattack)
	READ_ENTITY_DEFINITION_PROPERTY(CanRotate, canrotate)
	READ_ENTITY_DEFINITION_PROPERTY(CombatType, combattype)
	READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)

	READ_ENTITY_DEFINITION_PROPERTY(Lifetime, lifetime)
	READ_ENTITY_DEFINITION_PROPERTY(InitialCharges, initialcharges)
	READ_ENTITY_DEFINITION_PROPERTY(ShowLifetime, showlifetime)
END_ENTITY_DEFINITION_XML_PROCESSOR(Gadget, gadget)
