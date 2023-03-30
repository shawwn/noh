// (C)2008 S2 Games
// c_xmlproc_hero.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_herodefinition.h"
#include "i_heroentity.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CHeroDefinition, ENTITY_BASE_TYPE_HERO, Hero)

START_ENTITY_DEFINITION_XML_PROCESSOR(IHeroEntity, Hero)
	IUnitDefinition::ReadSettings(pDefinition, node, bMod);

	READ_ENTITY_DEFINITION_PROPERTY_EX(IsMobile, ismobile, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanAttack, canattack, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanRotate, canrotate, true)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CombatType, combattype, Hero)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanCarryItems, cancarryitems, true)

	READ_ENTITY_DEFINITION_PROPERTY(Team, team)
	READ_ENTITY_DEFINITION_PROPERTY(PrimaryAttribute, primaryattribute)
	READ_ENTITY_DEFINITION_PROPERTY(Strength, strength)
	READ_ENTITY_DEFINITION_PROPERTY(StrengthPerLevel, strengthperlevel)
	READ_ENTITY_DEFINITION_PROPERTY(Agility, agility)
	READ_ENTITY_DEFINITION_PROPERTY(AgilityPerLevel, agilityperlevel)
	READ_ENTITY_DEFINITION_PROPERTY(Intelligence, intelligence)
	READ_ENTITY_DEFINITION_PROPERTY(IntelligencePerLevel, intelligenceperlevel)

	READ_ENTITY_DEFINITION_PROPERTY(RespawnEffect, respawneffect)
	READ_ENTITY_DEFINITION_PROPERTY(AnnouncerSound, announcersound)

	READ_ENTITY_DEFINITION_PROPERTY_EX(DefaultBehavior, defaultbehavior, aggro)
END_ENTITY_DEFINITION_XML_PROCESSOR(Hero, hero)

// <gooditem>
DECLARE_XML_PROCESSOR(gooditem)
BEGIN_XML_REGISTRATION(gooditem)
	REGISTER_XML_PROCESSOR_EX(XMLHero, hero)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(gooditem, CHeroDefinition)
	const tstring &sName(node.GetProperty(_T("name")));
	if (sName.empty())
		return false;
	pObject->AddGoodItem(sName);
END_XML_PROCESSOR_NO_CHILDREN

// <item>
DECLARE_XML_PROCESSOR(recommendeditem)
BEGIN_XML_REGISTRATION(recommendeditem)
	REGISTER_XML_PROCESSOR_EX(XMLHero, hero)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(recommendeditem, CHeroDefinition)
	const tstring &sName(node.GetProperty(_T("name")));
	if (sName.empty())
		return false;
	pObject->AddRecommendedItem(sName);
END_XML_PROCESSOR_NO_CHILDREN