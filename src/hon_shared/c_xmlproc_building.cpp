// (C)2008 S2 Games
// c_xmlproc_building.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_buildingdefinition.h"
#include "i_buildingentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CBuildingDefinition, ENTITY_BASE_TYPE_BUILDING, Building)

START_ENTITY_DEFINITION_XML_PROCESSOR(IBuildingEntity, Building)
    IUnitDefinition::ReadSettings(pDefinition, node, bMod);

    READ_ENTITY_DEFINITION_PROPERTY_EX(AlwaysTargetable, alwaystargetable, true)
    READ_ENTITY_DEFINITION_PROPERTY(IsMobile, ismobile)
    READ_ENTITY_DEFINITION_PROPERTY(CanAttack, canattack)
    READ_ENTITY_DEFINITION_PROPERTY(CanRotate, canrotate)
    READ_ENTITY_DEFINITION_PROPERTY_EX(CombatType, combattype, structure)
    READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)

    READ_ENTITY_DEFINITION_PROPERTY(IsShop, isshop)
    READ_ENTITY_DEFINITION_PROPERTY(IsBase, isbase)
    READ_ENTITY_DEFINITION_PROPERTY(IsTower, istower)
    READ_ENTITY_DEFINITION_PROPERTY(IsRax, israx)
    READ_ENTITY_DEFINITION_PROPERTY(NoAltClickPing, noaltclickping)

    READ_ENTITY_DEFINITION_PROPERTY(LowHealthEffect, lowhealtheffect)
    READ_ENTITY_DEFINITION_PROPERTY(LowHealthSound, lowhealthsound)
    READ_ENTITY_DEFINITION_PROPERTY(DestroyedSound, destroyedsound)

    READ_ENTITY_DEFINITION_PROPERTY(DefaultShop, defaultshop)

    READ_ENTITY_DEFINITION_PROPERTY(NoHeroArmorReduction, noheroarmorreduction)
END_ENTITY_DEFINITION_XML_PROCESSOR(Building, building)

ENTITY_DEF_MERGE_START(CBuildingDefinition, IUnitDefinition)
    MERGE_PROPERTY(IsShop)
    MERGE_PROPERTY(IsBase)
    MERGE_PROPERTY(IsTower)
    MERGE_PROPERTY(IsRax)
    MERGE_PROPERTY(NoAltClickPing)

    MERGE_RESOURCE_PROPERTY(LowHealthEffect)
    MERGE_RESOURCE_PROPERTY(LowHealthSound)
    MERGE_RESOURCE_PROPERTY(DestroyedSound)

    MERGE_STRING_PROPERTY(DefaultShop)

    MERGE_ARRAY_PROPERTY(NoHeroArmorReduction)
ENTITY_DEF_MERGE_END
