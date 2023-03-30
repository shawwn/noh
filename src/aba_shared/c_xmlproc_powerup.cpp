// (C)2008 S2 Games
// c_xmlproc_powerup.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_powerupdefinition.h"
#include "i_powerupentity.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CPowerupDefinition, ENTITY_BASE_TYPE_POWERUP, Powerup)

START_ENTITY_DEFINITION_XML_PROCESSOR(IPowerupEntity, Powerup)
    IUnitDefinition::ReadSettings(pDefinition, node, bMod);

    READ_ENTITY_DEFINITION_PROPERTY(IsMobile, ismobile)
    READ_ENTITY_DEFINITION_PROPERTY(CanAttack, canattack)
    READ_ENTITY_DEFINITION_PROPERTY(CanRotate, canrotate)

    READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)

    READ_ENTITY_DEFINITION_PROPERTY(TouchSound, touchsound)
    READ_ENTITY_DEFINITION_PROPERTY(TouchTargetScheme, touchtargetscheme)
END_ENTITY_DEFINITION_XML_PROCESSOR(Powerup, powerup)
