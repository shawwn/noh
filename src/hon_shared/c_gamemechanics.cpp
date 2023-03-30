// (C)2009 S2 Games
// c_gamemechanics.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gamemechanics.h"
#include "c_gamemechanicsresource.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

/*====================
  GetGlobalConditionFromString
  ====================*/
EGlobalCondition    GetGlobalConditionFromString(const tstring &sName)
{
    if (TStringCompare(sName, _T("day")) == 0)
        return GLOBAL_CONDITION_DAY;
    else if (TStringCompare(sName, _T("night")) == 0)
        return GLOBAL_CONDITION_NIGHT;

    return INVALID_GLOBAL_CONDITION;
}


/*====================
  GetTargetTraitFromString
  ====================*/
ETargetTrait    GetTargetTraitFromString(const tstring &sName)
{
    if (TStringCompare(sName, _T("all")) == 0)
        return TARGET_TRAIT_ALL;
    else if (TStringCompare(sName, _T("none")) == 0)
        return TARGET_TRAIT_NONE;
    else if (TStringCompare(sName, _T("self")) == 0)
        return TARGET_TRAIT_SELF;
    else if (TStringCompare(sName, _T("other")) == 0)
        return TARGET_TRAIT_OTHER;
    else if (TStringCompare(sName, _T("ally")) == 0)
        return TARGET_TRAIT_ALLY;
    else if (TStringCompare(sName, _T("friendly")) == 0)
        return TARGET_TRAIT_FRIENDLY;
    else if (TStringCompare(sName, _T("enemy")) == 0)
        return TARGET_TRAIT_ENEMY;
    else if (TStringCompare(sName, _T("neutral")) == 0)
        return TARGET_TRAIT_NEUTRAL;
    else if (TStringCompare(sName, _T("passive")) == 0)
        return TARGET_TRAIT_PASSIVE;
    else if (TStringCompare(sName, _T("alive")) == 0)
        return TARGET_TRAIT_ALIVE;
    else if (TStringCompare(sName, _T("corpse")) == 0)
        return TARGET_TRAIT_CORPSE;
    else if (TStringCompare(sName, _T("dead")) == 0)
        return TARGET_TRAIT_DEAD;
    else if (TStringCompare(sName, _T("unit")) == 0)
        return TARGET_TRAIT_UNIT;
    else if (TStringCompare(sName, _T("hero")) == 0)
        return TARGET_TRAIT_HERO;
    else if (TStringCompare(sName, _T("creep")) == 0)
        return TARGET_TRAIT_CREEP;
    else if (TStringCompare(sName, _T("building")) == 0)
        return TARGET_TRAIT_BUILDING;
    else if (TStringCompare(sName, _T("pet")) == 0)
        return TARGET_TRAIT_PET;
    else if (TStringCompare(sName, _T("gadget")) == 0)
        return TARGET_TRAIT_GADGET;
    else if (TStringCompare(sName, _T("powerup")) == 0)
        return TARGET_TRAIT_POWERUP;
    else if (TStringCompare(sName, _T("tree")) == 0)
        return TARGET_TRAIT_TREE;
    else if (TStringCompare(sName, _T("chest")) == 0)
        return TARGET_TRAIT_CHEST;
    else if (TStringCompare(sName, _T("illusion")) == 0)
        return TARGET_TRAIT_ILLUSION;
    else if (TStringCompare(sName, _T("mine")) == 0)
        return TARGET_TRAIT_MINE;
    else if (TStringCompare(sName, _T("player_controlled")) == 0)
        return TARGET_TRAIT_PLAYER_CONTROLLED;
    else if (TStringCompare(sName, _T("owner")) == 0)
        return TARGET_TRAIT_OWNER;
    else if (TStringCompare(sName, _T("owned")) == 0)
        return TARGET_TRAIT_OWNED;
    else if (TStringCompare(sName, _T("deniable")) == 0)
        return TARGET_TRAIT_DENIABLE;
    else if (TStringCompare(sName, _T("smackable")) == 0)
        return TARGET_TRAIT_SMACKABLE;
    else if (TStringCompare(sName, _T("nohelp")) == 0)
        return TARGET_TRAIT_NOHELP;
    else if (TStringCompare(sName, _T("visible")) == 0)
        return TARGET_TRAIT_VISIBLE;
    else if (TStringCompare(sName, _T("full")) == 0)
        return TARGET_TRAIT_FULL;
    else if (TStringCompare(sName, _T("perks")) == 0)
        return TARGET_TRAIT_PERKS;
    else if (TStringCompare(sName, _T("immobilized")) == 0)
        return TARGET_TRAIT_IMMOBILIZED;
    else if (TStringCompare(sName, _T("restrained")) == 0)
        return TARGET_TRAIT_RESTRAINED;
    else if (TStringCompare(sName, _T("disarmed")) == 0)
        return TARGET_TRAIT_DISARMED;
    else if (TStringCompare(sName, _T("silenced")) == 0)
        return TARGET_TRAIT_SILENCED;
    else if (TStringCompare(sName, _T("perplexed")) == 0)
        return TARGET_TRAIT_PERPLEXED;
    else if (TStringCompare(sName, _T("stunned")) == 0)
        return TARGET_TRAIT_STUNNED;
    else if (TStringCompare(sName, _T("stealth")) == 0)
        return TARGET_TRAIT_STEALTH;
    else if (TStringCompare(sName, _T("moving")) == 0)
        return TARGET_TRAIT_MOVING;
    else if (TStringCompare(sName, _T("idle")) == 0)
        return TARGET_TRAIT_IDLE;
    else if (TStringCompare(sName, _T("attacking")) == 0)
        return TARGET_TRAIT_ATTACKING;
    else if (TStringCompare(sName, _T("casting")) == 0)
        return TARGET_TRAIT_CASTING;
    else if (TStringCompare(sName, _T("manapool")) == 0)
        return TARGET_TRAIT_MANAPOOL;
    else if (TStringCompare(sName, _T("deleted")) == 0)
        return TARGET_TRAIT_DELETED;

    return INVALID_TARGET_TYPE;
}


/*====================
  GetStateStackTypeFromString
  ====================*/
EStateStackType GetStateStackTypeFromString(const tstring &sName)
{
    if (TStringCompare(sName, _T("none")) == 0)
        return STATE_STACK_NONE;
    else if (TStringCompare(sName, _T("noself")) == 0)
        return STATE_STACK_NOSELF;
    else if (TStringCompare(sName, _T("full")) == 0)
        return STATE_STACK_FULL;
    
    return INVALID_STATE_STACK_TYPE;
}


/*====================
  CTargetScheme::CTargetScheme
  ====================*/
CTargetScheme::CTargetScheme(const tstring &sName, const tstring &sAllow, const tstring &sRestrict, const tstring &sAllow2, const tstring &sRestrict2, const CGameMechanics *pMechanics) :
m_sName(sName),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("TargetScheme_") + sName))
{
    tsvector vAllow(TokenizeString(sAllow, _T(',')));
    for (tsvector_it it(vAllow.begin()); it != vAllow.end(); ++it)
    {
        if (it->empty())
            continue;
        
        bool bNot((*it)[0] == _T('!'));
        const tstring &sValue(bNot ? it->substr(1) : *it);

        EGlobalCondition eGlobal(GetGlobalConditionFromString(sValue));
        if (eGlobal != INVALID_GLOBAL_CONDITION)
        {
            m_vAllow.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_GLOBAL : TARGET_SCHEME_TEST_GLOBAL, eGlobal));
            continue;
        }

        EAttribute eAttribute(GetAttributeFromString(sValue));
        if (eAttribute != ATTRIBUTE_INVALID)
        {
            m_vAllow.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTRIBUTE : TARGET_SCHEME_TEST_ATTRIBUTE, eAttribute));
            continue;
        }

        ETargetTrait eTrait(GetTargetTraitFromString(sValue));
        if (eTrait != INVALID_TARGET_TYPE)
        {
            m_vAllow.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_TRAIT : TARGET_SCHEME_TEST_TRAIT, eTrait));
            continue;
        }

        uint uiAttackType(pMechanics->LookupAttackType(sValue));
        if (uiAttackType != INVALID_ATTACK_TYPE)
        {
            m_vAllow.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTACK : TARGET_SCHEME_TEST_ATTACK, uiAttackType));
            continue;
        }
        
        m_vAllow.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_STRING : TARGET_SCHEME_TEST_STRING, sValue));
    }

    tsvector vRestrict(TokenizeString(sRestrict, _T(',')));
    for (tsvector_it it(vRestrict.begin()); it != vRestrict.end(); ++it)
    {
        if (it->empty())
            continue;
        
        bool bNot((*it)[0] == _T('!'));
        const tstring &sValue(bNot ? it->substr(1) : *it);

        EGlobalCondition eGlobal(GetGlobalConditionFromString(sValue));
        if (eGlobal != INVALID_GLOBAL_CONDITION)
        {
            m_vRestrict.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_GLOBAL : TARGET_SCHEME_TEST_GLOBAL, eGlobal));
            continue;
        }

        EAttribute eAttribute(GetAttributeFromString(sValue));
        if (eAttribute != ATTRIBUTE_INVALID)
        {
            m_vRestrict.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTRIBUTE : TARGET_SCHEME_TEST_ATTRIBUTE, eAttribute));
            continue;
        }

        ETargetTrait eTrait(GetTargetTraitFromString(sValue));
        if (eTrait != INVALID_TARGET_TYPE)
        {
            m_vRestrict.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_TRAIT : TARGET_SCHEME_TEST_TRAIT, eTrait));
            continue;
        }

        uint uiAttackType(pMechanics->LookupAttackType(sValue));
        if (uiAttackType != INVALID_ATTACK_TYPE)
        {
            m_vRestrict.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTACK : TARGET_SCHEME_TEST_ATTACK, uiAttackType));
            continue;
        }
        
        m_vRestrict.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_STRING : TARGET_SCHEME_TEST_STRING, sValue));
    }

    //
    // 2
    //

    tsvector vAllow2(TokenizeString(sAllow2, _T(',')));
    for (tsvector_it it(vAllow2.begin()); it != vAllow2.end(); ++it)
    {
        if (it->empty())
            continue;
        
        bool bNot((*it)[0] == _T('!'));
        const tstring &sValue(bNot ? it->substr(1) : *it);

        EGlobalCondition eGlobal(GetGlobalConditionFromString(sValue));
        if (eGlobal != INVALID_GLOBAL_CONDITION)
        {
            m_vAllow2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_GLOBAL : TARGET_SCHEME_TEST_GLOBAL, eGlobal));
            continue;
        }

        EAttribute eAttribute(GetAttributeFromString(sValue));
        if (eAttribute != ATTRIBUTE_INVALID)
        {
            m_vAllow2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTRIBUTE : TARGET_SCHEME_TEST_ATTRIBUTE, eAttribute));
            continue;
        }

        ETargetTrait eTrait(GetTargetTraitFromString(sValue));
        if (eTrait != INVALID_TARGET_TYPE)
        {
            m_vAllow2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_TRAIT : TARGET_SCHEME_TEST_TRAIT, eTrait));
            continue;
        }

        uint uiAttackType(pMechanics->LookupAttackType(sValue));
        if (uiAttackType != INVALID_ATTACK_TYPE)
        {
            m_vAllow2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTACK : TARGET_SCHEME_TEST_ATTACK, uiAttackType));
            continue;
        }
        
        m_vAllow2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_STRING : TARGET_SCHEME_TEST_STRING, sValue));
    }

    tsvector vRestrict2(TokenizeString(sRestrict2, _T(',')));
    for (tsvector_it it(vRestrict2.begin()); it != vRestrict2.end(); ++it)
    {
        if (it->empty())
            continue;
        
        bool bNot((*it)[0] == _T('!'));
        const tstring &sValue(bNot ? it->substr(1) : *it);

        EGlobalCondition eGlobal(GetGlobalConditionFromString(sValue));
        if (eGlobal != INVALID_GLOBAL_CONDITION)
        {
            m_vRestrict2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_GLOBAL : TARGET_SCHEME_TEST_GLOBAL, eGlobal));
            continue;
        }

        EAttribute eAttribute(GetAttributeFromString(sValue));
        if (eAttribute != ATTRIBUTE_INVALID)
        {
            m_vRestrict2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTRIBUTE : TARGET_SCHEME_TEST_ATTRIBUTE, eAttribute));
            continue;
        }

        ETargetTrait eTrait(GetTargetTraitFromString(sValue));
        if (eTrait != INVALID_TARGET_TYPE)
        {
            m_vRestrict2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_TRAIT : TARGET_SCHEME_TEST_TRAIT, eTrait));
            continue;
        }

        uint uiAttackType(pMechanics->LookupAttackType(sValue));
        if (uiAttackType != INVALID_ATTACK_TYPE)
        {
            m_vRestrict2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_ATTACK : TARGET_SCHEME_TEST_ATTACK, uiAttackType));
            continue;
        }
        
        m_vRestrict2.push_back(STestRecord(bNot ? TARGET_SCHEME_TEST_NOT_STRING : TARGET_SCHEME_TEST_STRING, sValue));
    }
}


/*====================
  CTargetScheme::GetDisplayName
  ====================*/
const tstring&  CTargetScheme::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CCombatTable::CCombatTable
  ====================*/
CCombatTable::CCombatTable(const tstring &sName, uint uiNumCombatTypes, const CGameMechanics *pMechanics) :
m_sName(sName),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("CombatType_") + sName)),
m_pGameMechanics(pMechanics)
{
    m_vAttackMultiplier.resize(uiNumCombatTypes);
    m_vSpellMultiplier.resize(uiNumCombatTypes);
    m_vAggroPriority.resize(uiNumCombatTypes);
    m_vAttackPriority.resize(uiNumCombatTypes);
    m_vProximityPriority.resize(uiNumCombatTypes);
    m_vTargetPriority.resize(uiNumCombatTypes);
}


/*====================
  CCombatTable::GetDisplayName
  ====================*/
const tstring&  CCombatTable::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CEffectType::CEffectType
  ====================*/
CEffectType::CEffectType(const tstring &sName, uint uiEffectBit, bool bAssist) :
m_sName(sName),
m_uiEffectBit(uiEffectBit),
m_bAssist(bAssist),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("EffectType_") + sName))
{
}


/*====================
  CEffectType::GetDisplayName
  ====================*/
const tstring&  CEffectType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CImmunityType::CImmunityType
  ====================*/
CImmunityType::CImmunityType(const tstring &sName, uint uiImmunityBits) :
m_sName(sName),
m_uiImmunityBits(uiImmunityBits),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("ImmunityType_") + sName))
{
}


/*====================
  CImmunityType::GetDisplayName
  ====================*/
const tstring&  CImmunityType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CStealthType::CStealthType
  ====================*/
CStealthType::CStealthType(const tstring &sName, uint uiStealthBit) :
m_sName(sName),
m_uiStealthBit(uiStealthBit),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("StealthType_") + sName))
{
}


/*====================
  CStealthType::GetDisplayName
  ====================*/
const tstring&  CStealthType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CRevealType::CRevealType
  ====================*/
CRevealType::CRevealType(const tstring &sName, uint uiRevealBits) :
m_sName(sName),
m_uiRevealBits(uiRevealBits),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("RevealType_") + sName))
{
}


/*====================
  CRevealType::GetDisplayName
  ====================*/
const tstring&  CRevealType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CArmorType::CArmorType
  ====================*/
CArmorType::CArmorType(const tstring &sName, uint uiEffectType, float fFactor) :
m_sName(sName),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("ArmorType_") + sName)),
m_uiEffectType(uiEffectType),
m_fFactor(fFactor)
{
}


/*====================
  CArmorType::GetDisplayName
  ====================*/
const tstring&  CArmorType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CAttackType::CAttackType
  ====================*/
CAttackType::CAttackType(const tstring &sName, float fDeniedExpMultiplier, float fUphillMissChance) :
m_sName(sName),
m_uiDisplayNameIndex(Game.GetEntityStringIndex(_CWS("AttackType_") + sName)),
m_fDeniedExpMultiplier(fDeniedExpMultiplier),
m_fUphillMissChance(fUphillMissChance)
{
}


/*====================
  CAttackType::CAttackType
  ====================*/
const tstring&  CAttackType::GetDisplayName() const
{
    return Game.GetEntityString(m_uiDisplayNameIndex);
}


/*====================
  CPopup::CPopup
  ====================*/
CPopup::CPopup(const tstring &sName, byte yType, const CXMLNode &node) :
m_sName(sName),
m_yType(yType),
m_uiMessageIndex(Game.GetEntityStringIndex(_CWS("Popup_") + sName)),
m_bShowValue(node.GetPropertyBool(_CWS("value"))),
m_bSelfOnly(node.GetProperty(_CWS("visibility")) == _CWS("self")),
m_bTeamOnly(node.GetProperty(_CWS("visibility")) == _CWS("team")),
m_bSpectatorOnly(node.GetProperty(_CWS("visibility")) == _CWS("spectator")),
m_bUsePlayerColor(node.GetProperty(_CWS("color")) == _CWS("*")),
m_v4Color(GetColorFromString(node.GetProperty(_CWS("color")))),
m_fStartX(node.GetPropertyFloat(_CWS("startx"), 0.0f)),
m_fStartY(node.GetPropertyFloat(_CWS("starty"), 0.0f)),
m_fSpeedX(node.GetPropertyFloat(_CWS("speedx"), 0.0f)),
m_fSpeedY(node.GetPropertyFloat(_CWS("speedy"), 2.0f)),
m_uiDuration(node.GetPropertyInt(_CWS("duration"))),
m_uiFadeTime(node.GetPropertyInt(_CWS("fadetime")))
{
}


/*====================
  CPopup::GetMessage
  ====================*/
tstring CPopup::GetMessage(ushort unValue) const
{
    static tsmapts s_mapTokens;

    const tstring &sMessage(Game.GetEntityString(m_uiMessageIndex));

    s_mapTokens[_CWS("value")] = XtoA(unValue);
    return ReplaceTokens(sMessage, s_mapTokens);
}


/*====================
  CPopup::GetRawMessage
  ====================*/
const tstring&  CPopup::GetRawMessage() const
{
    return Game.GetEntityString(m_uiMessageIndex);
}


/*====================
  CPing::CPing
  ====================*/
CPing::CPing(const tstring &sName, byte yType, const CXMLNode &node) :
m_sName(sName),
m_yType(yType),
m_bSelfOnly(node.GetProperty(_CWS("visibility")) == _CWS("self")),
m_bTargetOnly(node.GetProperty(_CWS("visibility")) == _CWS("target")),
m_bTeamOnly(node.GetProperty(_CWS("visibility")) == _CWS("team")),
m_bUsePlayerColor(node.GetProperty(_CWS("color")) == _CWS("*")),
m_v4Color(GetColorFromString(node.GetProperty(_CWS("color")))),
m_sEffectPath(node.GetProperty(_CWS("effect")))
{
}


namespace GameMechanics
{
// <gamemechanics>
DECLARE_XML_PROCESSOR(gamemechanics)
BEGIN_XML_REGISTRATION(gamemechanics)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(gamemechanics, CGameMechanicsResource)
    CGameMechanics *pGameMechanics(K2_NEW(ctx_Game,    CGameMechanics));
    if (pGameMechanics == NULL)
        return false;

    pObject->SetMechanics(pGameMechanics);
END_XML_PROCESSOR(pGameMechanics)


// <attacktype>
DECLARE_XML_PROCESSOR(attacktype)
BEGIN_XML_REGISTRATION(attacktype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(attacktype, CGameMechanics)
    pObject->RegisterAttackType(node);
END_XML_PROCESSOR_NO_CHILDREN


// <combattypes>
DECLARE_XML_PROCESSOR(combattypes)
BEGIN_XML_REGISTRATION(combattypes)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(combattypes, CGameMechanics)
    tsvector vTypes(TokenizeString(node.GetProperty(_CWS("list")), _T(',')));
    for (tsvector_it it(vTypes.begin()); it != vTypes.end(); ++it)
        pObject->RegisterCombatType(*it, INT_SIZE(vTypes.size()));
END_XML_PROCESSOR(pObject)

// <combattable>
DECLARE_XML_PROCESSOR(combattable)
BEGIN_XML_REGISTRATION(combattable)
    REGISTER_XML_PROCESSOR(combattypes)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(combattable, CGameMechanics)
    const tstring &sName(node.GetProperty(_CWS("name")));
    CCombatTable *pCombatTable(pObject->GetCombatTable(sName));
    if (pCombatTable == NULL)
    {
        Console.Err << _T("CombatTable declared for unknown combat type: ") << sName << newl;
        return false;
    }
END_XML_PROCESSOR(pCombatTable)

// <attackmultiplier>
DECLARE_XML_PROCESSOR(attackmultiplier)
BEGIN_XML_REGISTRATION(attackmultiplier)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(attackmultiplier, CCombatTable)
    const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetAttackMultiplier(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyFloat(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN

// <spellmultiplier>
DECLARE_XML_PROCESSOR(spellmultiplier)
BEGIN_XML_REGISTRATION(spellmultiplier)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(spellmultiplier, CCombatTable)
    const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetSpellMultiplier(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyFloat(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN

// <aggropriority>
DECLARE_XML_PROCESSOR(aggropriority)
BEGIN_XML_REGISTRATION(aggropriority)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(aggropriority, CCombatTable)
    const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetAggroPriority(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyInt(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN

// <attackpriority>
DECLARE_XML_PROCESSOR(attackpriority)
BEGIN_XML_REGISTRATION(attackpriority)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(attackpriority, CCombatTable)
const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetAttackPriority(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyInt(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN

// <proximitypriority>
DECLARE_XML_PROCESSOR(proximitypriority)
BEGIN_XML_REGISTRATION(proximitypriority)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(proximitypriority, CCombatTable)
    const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetProximityPriority(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyInt(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN

// <targetpriority>
DECLARE_XML_PROCESSOR(targetpriority)
BEGIN_XML_REGISTRATION(targetpriority)
    REGISTER_XML_PROCESSOR(combattable)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(targetpriority, CCombatTable)
    const CGameMechanics *pGameMechanics(pObject->GetGameMechanics());
    pObject->SetTargetPriority(pGameMechanics->LookupCombatType(node.GetProperty(_CWS("target"))), node.GetPropertyInt(_CWS("value")));
END_XML_PROCESSOR_NO_CHILDREN


// <stealthtype>
DECLARE_XML_PROCESSOR(stealthtype)
BEGIN_XML_REGISTRATION(stealthtype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(stealthtype, CGameMechanics)
    pObject->RegisterStealthType(node.GetProperty(_CWS("name")));
END_XML_PROCESSOR_NO_CHILDREN

// <revealtype>
DECLARE_XML_PROCESSOR(revealtype)
BEGIN_XML_REGISTRATION(revealtype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(revealtype, CGameMechanics)
    pObject->RegisterRevealType(node.GetProperty(_CWS("name")), node.GetProperty(_CWS("reveal")));
END_XML_PROCESSOR_NO_CHILDREN


// <effecttype>
DECLARE_XML_PROCESSOR(effecttype)
BEGIN_XML_REGISTRATION(effecttype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(effecttype, CGameMechanics)
    pObject->RegisterEffectType(node.GetProperty(_CWS("name")), node.GetPropertyBool(_CWS("assist")));
END_XML_PROCESSOR_NO_CHILDREN

// <immunitytype>
DECLARE_XML_PROCESSOR(immunitytype)
BEGIN_XML_REGISTRATION(immunitytype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(immunitytype, CGameMechanics)
    pObject->RegisterImmunityType(node.GetProperty(_CWS("name")), node.GetProperty(_CWS("immune")));
END_XML_PROCESSOR_NO_CHILDREN

// <armortype>
DECLARE_XML_PROCESSOR(armortype)
BEGIN_XML_REGISTRATION(armortype)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(armortype, CGameMechanics)
    pObject->RegisterArmorType(node.GetProperty(_CWS("name")), node.GetProperty(_CWS("effects")), node.GetPropertyFloat(_CWS("factor")));
END_XML_PROCESSOR_NO_CHILDREN


// <targetscheme>
DECLARE_XML_PROCESSOR(targetscheme)
BEGIN_XML_REGISTRATION(targetscheme)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(targetscheme, CGameMechanics)
    pObject->RegisterTargetScheme(node.GetProperty(_CWS("name")),
        node.GetProperty(_CWS("allow")),
        node.GetProperty(_CWS("restrict")),
        node.GetProperty(_CWS("allow2")),
        node.GetProperty(_CWS("restrict2"))
    );
END_XML_PROCESSOR_NO_CHILDREN


// <popup>
DECLARE_XML_PROCESSOR(popup)
BEGIN_XML_REGISTRATION(popup)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(popup, CGameMechanics)
    pObject->RegisterPopup(node);
END_XML_PROCESSOR_NO_CHILDREN

// <ping>
DECLARE_XML_PROCESSOR(ping)
BEGIN_XML_REGISTRATION(ping)
    REGISTER_XML_PROCESSOR(gamemechanics)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(ping, CGameMechanics)
    pObject->RegisterPing(node);
END_XML_PROCESSOR_NO_CHILDREN
}

/*====================
  CGameMechanics::CGameMechanics
  ====================*/
CGameMechanics::CGameMechanics() :
m_uiStealthBitMarker(BIT(0)),
m_uiEffectTypeBitMarker(BIT(0)),
m_uiDebuffEffectType(0),
m_uiBuffEffectType(0),
m_uiDisableEffectType(0)
{
    MemManager.Set(m_apPopups, 0, sizeof(CPopup*) * NUM_RESERVED_POPUPS);
    MemManager.Set(m_apPings, 0, sizeof(CPing*) * NUM_RESERVED_PINGS);
}


/*====================
  CGameMechanics::Clear
  ====================*/
void    CGameMechanics::Clear()
{
    m_mapAttackTypes.clear();
    m_vAttackTypes.clear();

    m_uiStealthBitMarker = BIT(0);
    m_mapStealthTypes.clear();
    m_mapRevealTypes.clear();

    m_uiEffectTypeBitMarker = BIT(0);
    m_mapEffectTypes.clear();
    m_mapImmunityTypes.clear();

    m_mapTargetSchemes.clear();
    m_vTargetSchemes.clear();

    m_mapPopups.clear();
    m_vPopups.clear();
    MemManager.Set(m_apPopups, 0, sizeof(CPopup*) * NUM_RESERVED_POPUPS);
    
    m_mapPings.clear();
    m_vPings.clear();
    MemManager.Set(m_apPings, 0, sizeof(CPing*) * NUM_RESERVED_PINGS);
}


/*====================
  CGameMechanics::PostLoad
  ====================*/
void    CGameMechanics::PostLoad()
{
    m_apPopups[POPUP_GOLD] = GetPopup(LookupPopup(_CWS("gold")));
    m_apPopups[POPUP_EXPERIENCE] = GetPopup(LookupPopup(_CWS("experience")));
    m_apPopups[POPUP_CRITICAL] = GetPopup(LookupPopup(_CWS("critical")));
    m_apPopups[POPUP_DEFLECTION] = GetPopup(LookupPopup(_CWS("deflection")));
    m_apPopups[POPUP_DENY] = GetPopup(LookupPopup(_CWS("deny")));
    m_apPopups[POPUP_MISS] = GetPopup(LookupPopup(_CWS("miss")));
    m_apPopups[POPUP_TOOFAR] = GetPopup(LookupPopup(_CWS("toofar")));
    m_apPopups[POPUP_CREEP_KILL] = GetPopup(LookupPopup(_CWS("creep_kill")));

    m_apPings[PING_ALERT] = GetPing(LookupPing(_CWS("alert")));
    m_apPings[PING_BUILDING_ATTACK] = GetPing(LookupPing(_CWS("building_attack")));
    m_apPings[PING_KILL_HERO] = GetPing(LookupPing(_CWS("kill_hero")));
    m_apPings[PING_ALLY_BUILDING_KILL] = GetPing(LookupPing(_CWS("ally_building_kill")));
    m_apPings[PING_ENEMY_BUILDING_KILL] = GetPing(LookupPing(_CWS("enemy_building_kill")));

    for (vector<CAttackType>::iterator it(m_vAttackTypes.begin()); it != m_vAttackTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("AttackType_") + it->GetName()));

    for (vector<CEffectType>::iterator it(m_vEffectTypes.begin()); it != m_vEffectTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("EffectType_") + it->GetName()));

    for (vector<CImmunityType>::iterator it(m_vImmunityTypes.begin()); it != m_vImmunityTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("ImmunityType_") + it->GetName()));

    for (vector<CStealthType>::iterator it(m_vStealthTypes.begin()); it != m_vStealthTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("StealthType_") + it->GetName()));

    for (vector<CRevealType>::iterator it(m_vRevealTypes.begin()); it != m_vRevealTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("RevealType_") + it->GetName()));

    for (vector<CArmorType>::iterator it(m_vArmorTypes.begin()); it != m_vArmorTypes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("ArmorType_") + it->GetName()));

    for (vector<CTargetScheme>::iterator it(m_vTargetSchemes.begin()); it != m_vTargetSchemes.end(); ++it)
        it->SetDisplayNameIndex(Game.GetEntityStringIndex(_CWS("TargetScheme_") + it->GetName()));

    for (vector<CPopup>::iterator it(m_vPopups.begin()); it != m_vPopups.end(); ++it)
        it->SetMessageIndex(Game.GetEntityStringIndex(_CWS("Popup_") + it->GetName()));
    
    if (Game.IsClient())
    {
        K2_WITH_GAME_RESOURCE_SCOPE()
        {
            for (vector<CPing>::iterator it(m_vPings.begin()); it != m_vPings.end(); ++it)
                g_ResourceManager.Register(it->GetEffectPath(), RES_EFFECT);
        }
    }

    m_uiDebuffEffectType = LookupEffectType(_CWS("StatusDebuff"));
    m_uiBuffEffectType = LookupEffectType(_CWS("StatusBuff"));
    m_uiDisableEffectType = LookupEffectType(_CWS("StatusDisable"));
}


/*====================
  CGameMechanics::RegisterAttackType
  ====================*/
uint    CGameMechanics::RegisterAttackType(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CWS("name")));
    if (sName.empty())
        return INVALID_ATTACK_TYPE;
    
    map<tstring, uint>::iterator itFind(m_mapAttackTypes.find(sName));
    if (itFind != m_mapAttackTypes.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vAttackTypes.size()));
    m_vAttackTypes.push_back(CAttackType(sName, node.GetPropertyFloat(_CWS("deniedexpmultiplier")), node.GetPropertyFloat(_CWS("uphillmisschance"))));
    m_mapAttackTypes[sName] = uiNewID;
    return uiNewID;
}


/*====================
  CGameMechanics::LookupAttackType
  ====================*/
uint    CGameMechanics::LookupAttackType(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_ATTACK_TYPE;
    
    map<tstring, uint>::const_iterator itFind(m_mapAttackTypes.find(sName));
    if (itFind == m_mapAttackTypes.end())
        return INVALID_ATTACK_TYPE;

    return itFind->second;
}


/*====================
  CGameMechanics::RegisterStealthType
  ====================*/
uint    CGameMechanics::RegisterStealthType(const tstring &sName)
{
    if (sName.empty())
        return 0;
    
    if (m_uiStealthBitMarker == 0)
    {
        Console.Err << _CWS("Stealth bit overflow: ") << sName << newl;
        return 0;
    }

    map<tstring, uint>::iterator itFind(m_mapStealthTypes.find(sName));
    if (itFind != m_mapStealthTypes.end())
        return m_vStealthTypes[itFind->second].GetStealthBit();

    uint uiNewBit(m_uiStealthBitMarker);
    m_uiStealthBitMarker <<= 1;
    m_vStealthTypes.push_back(CStealthType(sName, uiNewBit));
    m_mapStealthTypes[sName] = INT_SIZE(m_vStealthTypes.size() - 1);
    return uiNewBit;
}


/*====================
  CGameMechanics::LookupStealthType
  ====================*/
uint    CGameMechanics::LookupStealthType(const tstring &sName) const
{
    if (sName.empty())
        return 0;
    
    map<tstring, uint>::const_iterator itFind(m_mapStealthTypes.find(sName));
    if (itFind == m_mapStealthTypes.end())
        return 0;

    return m_vStealthTypes[itFind->second].GetStealthBit();
}


/*====================
  CGameMechanics::RegisterRevealType
  ====================*/
uint    CGameMechanics::RegisterRevealType(const tstring &sName, const tstring &sRevealList)
{
    if (sName.empty() || sRevealList.empty())
        return 0;
    
    map<tstring, uint>::iterator itFind(m_mapRevealTypes.find(sName));
    if (itFind != m_mapRevealTypes.end())
    {
        Console.Warn << _CWS("Reveal type registered twice: ") << sName << newl;
        return itFind->second;
    }

    tsvector vRevealList(TokenizeString(sRevealList, _T(',')));
    if (vRevealList.empty())
        return 0;

    uint uiRevealBits(0);
    for (tsvector_it it(vRevealList.begin()); it != vRevealList.end(); ++it)
        uiRevealBits |= LookupStealthType(*it);

    m_vRevealTypes.push_back(CRevealType(sName, uiRevealBits));
    m_mapRevealTypes[sName] = INT_SIZE(m_vRevealTypes.size() - 1);
    return uiRevealBits;
}


/*====================
  CGameMechanics::LookupRevealType
  ====================*/
uint    CGameMechanics::LookupRevealType(const tstring &sName) const
{
    if (sName.empty())
        return 0;
    
    map<tstring, uint>::const_iterator itFind(m_mapRevealTypes.find(sName));
    if (itFind == m_mapRevealTypes.end())
        return 0;

    return m_vRevealTypes[itFind->second].GetRevealBits();
}


/*====================
  CGameMechanics::RegisterCombatType
  ====================*/
uint    CGameMechanics::RegisterCombatType(const tstring &sName, uint uiNumCombatTypes)
{
    if (sName.empty())
        return INVALID_COMBAT_TYPE;
    
    map<tstring, uint>::iterator itFind(m_mapCombatTypes.find(sName));
    if (itFind != m_mapCombatTypes.end())
        return itFind->second;

    uint uiIndex(INT_SIZE(m_vCombatTables.size()));
    m_mapCombatTypes[sName] = uiIndex;
    m_vCombatTables.push_back(CCombatTable(sName, uiNumCombatTypes, this));
    return uiIndex;
}


/*====================
  CGameMechanics::GetAttackMultiplier
  ====================*/
float   CGameMechanics::GetAttackMultiplier(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 1.0f;

    return pTable->GetAttackMultiplier(uiCombatTypeB);
}


/*====================
  CGameMechanics::GetSpellMultiplier
  ====================*/
float   CGameMechanics::GetSpellMultiplier(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 1.0f;

    return pTable->GetSpellMultiplier(uiCombatTypeB);
}


/*====================
  CGameMechanics::GetAggroPriority
  ====================*/
int CGameMechanics::GetAggroPriority(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 0;

    return pTable->GetAggroPriority(uiCombatTypeB);
}


/*====================
  CGameMechanics::GetTargetPriority
  ====================*/
int CGameMechanics::GetTargetPriority(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 0;

    return pTable->GetTargetPriority(uiCombatTypeB);
}


/*====================
  CGameMechanics::GetAttackPriority
  ====================*/
int CGameMechanics::GetAttackPriority(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 0;

    return pTable->GetAttackPriority(uiCombatTypeB);
}


/*====================
  CGameMechanics::GetProximityPriority
  ====================*/
int CGameMechanics::GetProximityPriority(uint uiCombatTypeA, uint uiCombatTypeB)
{
    CCombatTable *pTable(GetCombatTable(uiCombatTypeA));
    if (pTable == NULL)
        return 0;

    return pTable->GetProximityPriority(uiCombatTypeB);
}


/*====================
  CGameMechanics::RegisterEffectType
  ====================*/
uint    CGameMechanics::RegisterEffectType(const tstring &sName, bool bAssist)
{
    if (sName.empty())
        return 0;
    
    if (m_uiEffectTypeBitMarker == 0)
    {
        Console.Err << _CWS("Effect type bit overflow: ") << sName << newl;
        return 0;
    }

    map<tstring, uint>::iterator itFind(m_mapEffectTypes.find(sName));
    if (itFind != m_mapEffectTypes.end())
        return m_vEffectTypes[itFind->second].GetEffectBit();

    uint uiNewBit(m_uiEffectTypeBitMarker);
    m_uiEffectTypeBitMarker <<= 1;
    m_vEffectTypes.push_back(CEffectType(sName, uiNewBit, bAssist));
    m_mapEffectTypes[sName] = INT_SIZE(m_vEffectTypes.size() - 1);
    return uiNewBit;
}


/*====================
  CGameMechanics::LookupEffectType
  ====================*/
uint    CGameMechanics::LookupEffectType(const tstring &sName) const
{
    if (sName.empty())
        return 0;
    
    uint uiEffectType(0);
    tsvector vEffect(TokenizeString(sName, _T(' ')));
    for (tsvector_it it(vEffect.begin()); it != vEffect.end(); ++it)
    {
        map<tstring, uint>::const_iterator itFind(m_mapEffectTypes.find(*it));
        if (itFind == m_mapEffectTypes.end())
            continue;

        uiEffectType |= m_vEffectTypes[itFind->second].GetEffectBit();
    }

    return uiEffectType;
}


/*====================
  CGameMechanics::IsDebuff
  ====================*/
bool    CGameMechanics::IsDebuff(uint uiEffectType) const
{
    if (uiEffectType == 0)
        return false;

    if ((uiEffectType & m_uiDebuffEffectType) != 0)
        return true;
    else
        return false;
}


/*====================
  CGameMechanics::IsBuff
  ====================*/
bool    CGameMechanics::IsBuff(uint uiEffectType) const
{
    if (uiEffectType == 0)
        return false;

    if ((uiEffectType & m_uiBuffEffectType) != 0)
        return true;
    else
        return false;
}


/*====================
  CGameMechanics::IsDisable
  ====================*/
bool    CGameMechanics::IsDisable(uint uiEffectType) const
{
    if (uiEffectType == 0)
        return false;

    if ((uiEffectType & m_uiDisableEffectType) != 0)
        return true;
    else
        return false;
}


/*====================
  CGameMechanics::IsAssist
  ====================*/
bool    CGameMechanics::IsAssist(uint uiEffectType) const
{
    if (uiEffectType == 0)
        return false;

    for (vector<CEffectType>::const_iterator it(m_vEffectTypes.begin()); it != m_vEffectTypes.end(); ++it)
    {
        if ((it->GetEffectBit() & uiEffectType) == 0)
            continue;

        if (it->GetAssist())
            return true;
    }

    return false;
}


/*====================
  CGameMechanics::RegisterImmunityType
  ====================*/
uint    CGameMechanics::RegisterImmunityType(const tstring &sName, const tstring &sImmunityList)
{
    if (sName.empty() || sImmunityList.empty())
        return 0;
    
    map<tstring, uint>::iterator itFind(m_mapImmunityTypes.find(sName));
    if (itFind != m_mapImmunityTypes.end())
    {
        Console.Warn << _CWS("Immunity type registered twice: ") << sName << newl;
        return m_vImmunityTypes[itFind->second].GetImmunityBits();
    }

    uint uiImmunityBits(LookupEffectType(sImmunityList));
    m_vImmunityTypes.push_back(CImmunityType(sName, uiImmunityBits));
    m_mapImmunityTypes[sName] = INT_SIZE(m_vImmunityTypes.size() - 1);
    return uiImmunityBits;
}


/*====================
  CGameMechanics::LookupImmunityType
  ====================*/
uint    CGameMechanics::LookupImmunityType(const tstring &sName) const
{
    if (sName.empty())
        return 0;
    
    map<tstring, uint>::const_iterator itFind(m_mapImmunityTypes.find(sName));
    if (itFind == m_mapImmunityTypes.end())
        return 0;

    return m_vImmunityTypes[itFind->second].GetImmunityBits();
}


/*====================
  CGameMechanics::GetEffectTypeString
  ====================*/
tstring CGameMechanics::GetEffectTypeString(uint uiEffectType) const
{
    tstring sResult;

    if (uiEffectType == 0)
        return TSNULL;

    for (vector<CEffectType>::const_iterator it(m_vEffectTypes.begin()); it != m_vEffectTypes.end(); ++it)
    {
        if ((it->GetEffectBit() & uiEffectType) == 0)
            continue;

        if (!sResult.empty())
            sResult += SPACE;

        sResult += it->GetDisplayName();
    }

    return sResult;
}


/*====================
  CGameMechanics::RegisterArmorType
  ====================*/
uint    CGameMechanics::RegisterArmorType(const tstring &sName, const tstring &sEffects, float fFactor)
{
    if (sName.empty())
        return INVALID_ARMOR_TYPE;

    uint uiEffectType(LookupEffectType(sEffects));
    if (uiEffectType == 0)
        return INVALID_ARMOR_TYPE;

    map<tstring, uint>::iterator itFind(m_mapArmorTypes.find(sName));
    if (itFind != m_mapArmorTypes.end())
        return itFind->second;

    uint uiNewType(INT_SIZE(m_vArmorTypes.size()));
    m_vArmorTypes.push_back(CArmorType(sName, uiEffectType, fFactor));
    m_mapArmorTypes[sName] = uiNewType;
    return uiNewType;
}


/*====================
  CGameMechanics::LookupArmorType
  ====================*/
uint    CGameMechanics::LookupArmorType(const tstring &sName) const
{
    map<tstring, uint>::const_iterator itFind(m_mapArmorTypes.find(sName));
    if (itFind == m_mapArmorTypes.end())
        return INVALID_ARMOR_TYPE;

    return itFind->second;
}


/*====================
  CGameMechanics::IsArmorEffective
  ====================*/
bool    CGameMechanics::IsArmorEffective(uint uiArmorType, uint uiEffectType) const
{
    const CArmorType *pArmor(GetArmorType(uiArmorType));
    if (pArmor == NULL)
        return false;

    return pArmor->IsEffective(uiEffectType);
}


/*====================
  CGameMechanics::GetArmorDamageAdjustment
  ====================*/
float   CGameMechanics::GetArmorDamageAdjustment(uint uiArmorType, float fArmor) const
{
    const CArmorType *pArmor(GetArmorType(uiArmorType));
    if (pArmor == NULL)
        return 0.0f;

    return pArmor->GetDamageAdjustment(fArmor);
}


/*====================
  CGameMechanics::RegisterTargetScheme
  ====================*/
uint    CGameMechanics::RegisterTargetScheme(const tstring &sName, const tstring &sAllow, const tstring &sRestrict, const tstring &sAllow2, const tstring &sRestrict2)
{
    if (sName.empty())
        return INVALID_TARGET_SCHEME;

    map<tstring, uint>::iterator itFind(m_mapTargetSchemes.find(sName));
    if (itFind != m_mapTargetSchemes.end())
        return itFind->second;

    uint uiNewID(INT_SIZE(m_vTargetSchemes.size()));
    m_mapTargetSchemes[sName] = uiNewID;
    m_vTargetSchemes.push_back(CTargetScheme(sName, sAllow, sRestrict, sAllow2, sRestrict2, this));
    return uiNewID;
}


/*====================
  CGameMechanics::LookupTargetScheme
  ====================*/
uint    CGameMechanics::LookupTargetScheme(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_TARGET_SCHEME;
    
    map<tstring, uint>::const_iterator itFind(m_mapTargetSchemes.find(sName));
    if (itFind == m_mapTargetSchemes.end())
        return INVALID_TARGET_SCHEME;

    return itFind->second;
}


/*====================
  CGameMechanics::RegisterPopup
  ====================*/
byte    CGameMechanics::RegisterPopup(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CWS("name")));
    if (sName.empty())
        return INVALID_POPUP;

    map<tstring, byte>::iterator itFind(m_mapPopups.find(sName));
    if (itFind != m_mapPopups.end())
        return itFind->second;

    byte yNewID(byte(m_vPopups.size() & 0xff));
    m_mapPopups[sName] = yNewID;
    m_vPopups.push_back(CPopup(sName, yNewID, node));
    return yNewID;
}


/*====================
  CGameMechanics::LookupPopup
  ====================*/
byte    CGameMechanics::LookupPopup(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_POPUP;
    
    map<tstring, byte>::const_iterator itFind(m_mapPopups.find(sName));
    if (itFind == m_mapPopups.end())
        return INVALID_POPUP;

    return itFind->second;
}


/*====================
  CGameMechanics::RegisterPing
  ====================*/
byte    CGameMechanics::RegisterPing(const CXMLNode &node)
{
    const tstring &sName(node.GetProperty(_CWS("name")));
    if (sName.empty())
        return INVALID_PING;

    map<tstring, byte>::iterator itFind(m_mapPings.find(sName));
    if (itFind != m_mapPings.end())
        return itFind->second;

    byte yNewID(byte(m_vPings.size() & 0xff));
    m_mapPings[sName] = yNewID;
    m_vPings.push_back(CPing(sName, yNewID, node));
    return yNewID;
}


/*====================
  CGameMechanics::LookupPing
  ====================*/
byte    CGameMechanics::LookupPing(const tstring &sName) const
{
    if (sName.empty())
        return INVALID_PING;
    
    map<tstring, byte>::const_iterator itFind(m_mapPings.find(sName));
    if (itFind == m_mapPings.end())
        return INVALID_PING;

    return itFind->second;
}


/*====================
  CGameMechanics::WriteStringTable
  ====================*/
void    CGameMechanics::WriteStringTable(CFileHandle &hFile, size_t zTabStop, size_t zColumnOffset)
{
    hFile << _CWS("// Game Mechanics") << newl;

    for (vector<CAttackType>::iterator it(m_vAttackTypes.begin()); it != m_vAttackTypes.end(); ++it)
        hFile << TabPad(_CWS("AttackType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CCombatTable>::iterator it(m_vCombatTables.begin()); it != m_vCombatTables.end(); ++it)
        hFile << TabPad(_CWS("CombatType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CEffectType>::iterator it(m_vEffectTypes.begin()); it != m_vEffectTypes.end(); ++it)
        hFile << TabPad(_CWS("EffectType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CImmunityType>::iterator it(m_vImmunityTypes.begin()); it != m_vImmunityTypes.end(); ++it)
        hFile << TabPad(_CWS("ImmunityType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CStealthType>::iterator it(m_vStealthTypes.begin()); it != m_vStealthTypes.end(); ++it)
        hFile << TabPad(_CWS("StealthType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CRevealType>::iterator it(m_vRevealTypes.begin()); it != m_vRevealTypes.end(); ++it)
        hFile << TabPad(_CWS("RevealType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CArmorType>::iterator it(m_vArmorTypes.begin()); it != m_vArmorTypes.end(); ++it)
        hFile << TabPad(_CWS("ArmorType_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CTargetScheme>::iterator it(m_vTargetSchemes.begin()); it != m_vTargetSchemes.end(); ++it)
        hFile << TabPad(_CWS("TargetScheme_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetDisplayName()) << newl;

    hFile << newl;

    for (vector<CPopup>::iterator it(m_vPopups.begin()); it != m_vPopups.end(); ++it)
        hFile << TabPad(_CWS("Popup_") + it->GetName(), zTabStop, zColumnOffset) << EscapeWhiteSpace(it->GetRawMessage()) << newl;

    hFile << newl;
}
