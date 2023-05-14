// (C)2008 S2 Games
// i_combataction.h
//
//=============================================================================
#ifndef __I_COMBATACTION_H__
#define __I_COMBATACTION_H__

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IUnitEntity;
class CCombatEvent;
class CDamageEvent;
class ICombatAction;

bool    EvaluateConditionalString(const tstring &sTest, IGameEntity *pThis, IGameEntity *pInflictor, IUnitEntity *pSource, IUnitEntity *pTarget, const ICombatAction *pAction);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// COMBAT_ACTION_PROPERTY
#define COMBAT_ACTION_PROPERTY(type, name) \
protected: \
    vector<type>    m_v##name; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        tsvector vValues(TokenizeString(sValues, _T(','))); \
        type _val; \
        for (tsvector_it it(vValues.begin()); it != vValues.end(); ++it) \
            m_v##name.push_back(AtoX(*it, _val)); \
    } \
\
    type    Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name.empty()) \
            return GetDefaultEmptyValue<type>(); \
    \
        return m_v##name[MIN(GetLevel() - 1, INT_SIZE(m_v##name.size() - 1))]; \
    } \

// COMBAT_ACTION_VALUE_PROPERTY
#define COMBAT_ACTION_VALUE_PROPERTY(name) \
protected: \
    vector<EDynamicActionValue> m_v##name##Dynamic; \
    vector<float>               m_v##name##Static; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        tsvector vValues(TokenizeString(sValues, _T(','))); \
        for (tsvector_it it(vValues.begin()); it != vValues.end(); ++it) \
        { \
            m_v##name##Dynamic.push_back(GetDynamicActionValueFromString(*it)); \
            m_v##name##Static.push_back(AtoF(*it)); \
        } \
    } \
\
    float   Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name##Dynamic.empty()) \
            return 0.0f; \
\
        int iIndex(MIN(GetLevel() - 1, INT_SIZE(m_v##name##Dynamic.size() - 1))); \
        if (m_v##name##Dynamic[iIndex] == DYNAMIC_VALUE_INVALID) \
            return m_v##name##Static[iIndex]; \
        return GetDynamicValue(m_v##name##Dynamic[iIndex]); \
    } \

// COMBAT_ACTION_VALUE_PROPERTY
#define COMBAT_ACTION_VALUE_PROPERTY_EX(name, type, function) \
protected: \
    vector<EDynamicActionValue> m_v##name##Dynamic; \
    vector<float>               m_v##name##Static; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        tsvector vValues(TokenizeString(sValues, _T(','))); \
        for (tsvector_it it(vValues.begin()); it != vValues.end(); ++it) \
        { \
            m_v##name##Dynamic.push_back(GetDynamicActionValueFromString(*it)); \
            m_v##name##Static.push_back(AtoF(*it)); \
        } \
    } \
\
    type    Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name##Dynamic.empty()) \
            return GetDefaultEmptyValue<type>(); \
\
        int iIndex(MIN(GetLevel() - 1, INT_SIZE(m_v##name##Dynamic.size() - 1))); \
        if (m_v##name##Dynamic[iIndex] == DYNAMIC_VALUE_INVALID) \
            return function(m_v##name##Static[iIndex]); \
        return function(GetDynamicValue(m_v##name##Dynamic[iIndex])); \
    } \

// COMBAT_ACTION_PROPERTY_EX
#define COMBAT_ACTION_PROPERTY_EX(type, name, function) \
protected: \
    vector<type>    m_v##name; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        tsvector vValues(TokenizeString(sValues, _T(','))); \
        for (tsvector_it it(vValues.begin()); it != vValues.end(); ++it) \
            m_v##name.push_back(function(*it)); \
    } \
\
    type    Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name.empty()) \
            return GetDefaultEmptyValue<type>(); \
    \
        return m_v##name[MIN(GetLevel() - 1, INT_SIZE(m_v##name.size() - 1))]; \
    } \

// COMBAT_ACTION_STRING_PROPERTY
#define COMBAT_ACTION_STRING_PROPERTY(name) \
protected: \
    tsvector    m_v##name; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        m_v##name = TokenizeString(sValues, _T(',')); \
    } \
\
    const tstring&  Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name.empty()) \
            return TSNULL; \
    \
        return m_v##name[MIN(GetLevel() - 1, INT_SIZE(m_v##name.size() - 1))]; \
    } \
\
    uint            Get##name##Size() const         { return INT_SIZE(m_v##name.size()); } \
    const tstring&  Get##name(uint uiIndex) const   { return uiIndex < m_v##name.size() ? m_v##name[uiIndex] : TSNULL; }

// COMBAT_ACTION_RESOURCE_PROPERTY
#define COMBAT_ACTION_RESOURCE_PROPERTY(name, type) \
protected: \
    tsvector            m_v##name##Path; \
    vector<ResHandle>   m_v##name; \
\
public: \
    void    Set##name(const tstring &sValues) \
    { \
        tsvector vPaths(TokenizeString(sValues, _T(','))); \
        for (tsvector_it it(vPaths.begin()); it != vPaths.end(); ++it) \
            m_v##name##Path.push_back(FileManager.SanitizePath(*it)); \
    } \
\
    const tstring&  Get##name##Path() const \
    { \
        if (GetLevel() == 0 || m_v##name##Path.empty()) \
            return TSNULL; \
\
        return m_v##name##Path[MIN(GetLevel() - 1, INT_SIZE(m_v##name##Path.size() - 1))]; \
    } \
\
    ResHandle   Get##name() const \
    { \
        if (GetLevel() == 0 || m_v##name.empty()) \
            return INVALID_RESOURCE; \
\
        return m_v##name[MIN(GetLevel() - 1, INT_SIZE(m_v##name.size() - 1))]; \
    } \
\
    uint            Get##name##Size() const             { return INT_SIZE(m_v##name.size()); } \
    const tstring&  Get##name##Path(uint uiIndex) const { return uiIndex < m_v##name##Path.size() ? m_v##name##Path[uiIndex] : TSNULL; } \
    ResHandle       Get##name(uint uiIndex) const       { return uiIndex < m_v##name.size() ? m_v##name[uiIndex] : INVALID_RESOURCE; } \
\
    void    Precache##name() \
    { \
        m_v##name.clear(); \
\
        for (tsvector_it it(m_v##name##Path.begin()); it != m_v##name##Path.end(); ++it) \
            m_v##name.push_back(Game.Register##type(*it)); \
    }

// READ_COMBAT_ACTION_PROPERTY
#define READ_COMBAT_ACTION_PROPERTY(name, attribute) \
    pAction->Set##name(node.GetProperty(_T(#attribute)));

// READ_COMBAT_ACTION_PROPERTY_EX
#define READ_COMBAT_ACTION_PROPERTY_EX(name, attribute, def) \
    pAction->Set##name(node.GetProperty(_T(#attribute), _T(#def)));

// READ_COMBAT_ACTION_PROPERTY_EXVALUE
#define READ_COMBAT_ACTION_PROPERTY_EXVALUE(name, attribute, def) \
    pAction->Set##name(node.GetProperty(_T(#attribute), def));

// READ_COMBAT_ACTION_PROPERTY_INHERIT
#define READ_COMBAT_ACTION_PROPERTY_INHERIT(name, attribute, parent, def) \
    pAction->Set##name(node.GetProperty(_T(#attribute), node.GetProperty(_T(#parent), _T(#def))));

typedef vector<class ICombatAction*>        CombatActionScript;
typedef CombatActionScript::iterator        CombatActionScript_it;
typedef CombatActionScript::const_iterator  CombatActionScript_cit;

enum EActionOperator
{
    OPERATOR_NONE,
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MULT,
    OPERATOR_DIV,
    OPERATOR_MIN,
    OPERATOR_MAX
};

template<> inline EActionOperator   GetDefaultEmptyValue<EActionOperator>() { return OPERATOR_NONE; }

inline EActionOperator  GetActionOperatorFromString(const tstring &sActionMultiplier)
{
    if (CompareNoCase(sActionMultiplier, _CTS("none")) == 0)
        return OPERATOR_NONE;
    else if (CompareNoCase(sActionMultiplier, _CTS("add")) == 0)
        return OPERATOR_ADD;
    else if (CompareNoCase(sActionMultiplier, _CTS("sub")) == 0)
        return OPERATOR_SUB;
    else if (CompareNoCase(sActionMultiplier, _CTS("mult")) == 0)
        return OPERATOR_MULT;
    else if (CompareNoCase(sActionMultiplier, _CTS("div")) == 0)
        return OPERATOR_DIV;
    else if (CompareNoCase(sActionMultiplier, _CTS("min")) == 0)
        return OPERATOR_MIN;
    else if (CompareNoCase(sActionMultiplier, _CTS("max")) == 0)
        return OPERATOR_MAX;
    else
        return OPERATOR_NONE;
}

inline EActionOperator& AtoX(const tstring &s, EActionOperator &e)  { return e = GetActionOperatorFromString(s); }

enum EActionCmpOperator
{
    OPERATOR_CMP_NONE,
    OPERATOR_CMP_EQUALS,
    OPERATOR_CMP_NOT_EQUALS,
    OPERATOR_CMP_LESS_THAN,
    OPERATOR_CMP_LESS_THAN_OR_EQUALS,
    OPERATOR_CMP_GREATER_THAN,
    OPERATOR_CMP_GREATER_THAN_OR_EQUALS
};

template<> inline EActionCmpOperator    GetDefaultEmptyValue<EActionCmpOperator>()  { return OPERATOR_CMP_NONE; }

inline EActionCmpOperator   GetActionCmpOperatorFromString(const tstring &sOperator)
{
    if (CompareNoCase(sOperator, _CTS("none")) == 0)
        return OPERATOR_CMP_NONE;
    else if (CompareNoCase(sOperator, _CTS("eq")) == 0)
        return OPERATOR_CMP_EQUALS;
    else if (CompareNoCase(sOperator, _CTS("ne")) == 0)
        return OPERATOR_CMP_NOT_EQUALS;
    else if (CompareNoCase(sOperator, _CTS("lt")) == 0)
        return OPERATOR_CMP_LESS_THAN;
    else if (CompareNoCase(sOperator, _CTS("le")) == 0)
        return OPERATOR_CMP_LESS_THAN_OR_EQUALS;
    else if (CompareNoCase(sOperator, _CTS("gt")) == 0)
        return OPERATOR_CMP_GREATER_THAN;
    else if (CompareNoCase(sOperator, _CTS("ge")) == 0)
        return OPERATOR_CMP_GREATER_THAN_OR_EQUALS;
    else
        return OPERATOR_CMP_NONE;
}

inline EActionCmpOperator&  AtoX(const tstring &s, EActionCmpOperator &e)   { return e = GetActionCmpOperatorFromString(s); }

enum EDynamicActionValue
{
    DYNAMIC_VALUE_INVALID,
    
    DYNAMIC_VALUE_RESULT,
    DYNAMIC_VALUE_STACK,
    DYNAMIC_VALUE_VAR0,
    DYNAMIC_VALUE_VAR1,
    DYNAMIC_VALUE_VAR2,
    DYNAMIC_VALUE_VAR3,

    DYNAMIC_VALUE_PER_SECOND,

    DYNAMIC_VALUE_CHARGES,
    DYNAMIC_VALUE_LIFETIME_REMAINING,
    DYNAMIC_VALUE_HEALTH_LOST,  // Health lost since last saved
    DYNAMIC_VALUE_MOVEMENT,     // Distance moved this frame
    DYNAMIC_VALUE_ACCUMULATOR,  // Accumulator total
    DYNAMIC_VALUE_PARAM,        // Generic parameter
    DYNAMIC_VALUE_LIFETIME,
    DYNAMIC_VALUE_TEAM,
    DYNAMIC_VALUE_TIME,
    DYNAMIC_VALUE_CASTDURATION,
    DYNAMIC_VALUE_OWNER_COUNTER,
    DYNAMIC_VALUE_LEVEL,

    DYNAMIC_VALUE_SOURCE_ENTITY,
    DYNAMIC_VALUE_TARGET_ENTITY,
    DYNAMIC_VALUE_INFLICTOR_ENTITY,
    DYNAMIC_VALUE_PROXY_ENTITY,
    DYNAMIC_VALUE_OWNER_ENTITY,
    DYNAMIC_VALUE_STACK_ENTITY,
    DYNAMIC_VALUE_THIS_PROXY_ENTITY,

    // Only valid in ondamaged events
    DYNAMIC_VALUE_TARGET_DAMAGE,    
    DYNAMIC_VALUE_SOURCE_DAMAGE,
        
    DYNAMIC_VALUE_TARGET_HEALTH,
    DYNAMIC_VALUE_TARGET_HEALTH_PERCENT,
    DYNAMIC_VALUE_TARGET_TOTAL_HEALTH,
    DYNAMIC_VALUE_TARGET_MISSING_HEALTH,
    DYNAMIC_VALUE_TARGET_MISSING_HEALTH_PERCENT,
    
    DYNAMIC_VALUE_TARGET_MANA,
    DYNAMIC_VALUE_TARGET_MANA_PERCENT,
    DYNAMIC_VALUE_TARGET_TOTAL_MANA,
    DYNAMIC_VALUE_TARGET_MISSING_MANA,
    DYNAMIC_VALUE_TARGET_MISSING_MANA_PERCENT,

    DYNAMIC_VALUE_TARGET_STRENGTH,
    DYNAMIC_VALUE_TARGET_INTELLIGENCE,
    DYNAMIC_VALUE_TARGET_AGILITY,

    DYNAMIC_VALUE_TARGET_ATTACKSPEED,
    DYNAMIC_VALUE_TARGET_CASTSPEED,
    DYNAMIC_VALUE_TARGET_ATTACKACTIONTIME,
    DYNAMIC_VALUE_TARGET_ATTACKDURATION,
    DYNAMIC_VALUE_TARGET_ATTACKCOOLDOWN,
    DYNAMIC_VALUE_TARGET_ATTACKDAMAGE,

    DYNAMIC_VALUE_TARGET_ACCUMULATOR,
    DYNAMIC_VALUE_TARGET_LIFETIME,
    DYNAMIC_VALUE_TARGET_TEAM,
    DYNAMIC_VALUE_TARGET_MOVESPEED,

    DYNAMIC_VALUE_SOURCE_HEALTH,
    DYNAMIC_VALUE_SOURCE_HEALTH_PERCENT,
    DYNAMIC_VALUE_SOURCE_TOTAL_HEALTH,
    DYNAMIC_VALUE_SOURCE_MISSING_HEALTH,
    DYNAMIC_VALUE_SOURCE_MISSING_HEALTH_PERCENT,
    
    DYNAMIC_VALUE_SOURCE_MANA,
    DYNAMIC_VALUE_SOURCE_MANA_PERCENT,
    DYNAMIC_VALUE_SOURCE_TOTAL_MANA,
    DYNAMIC_VALUE_SOURCE_MISSING_MANA,
    DYNAMIC_VALUE_SOURCE_MISSING_MANA_PERCENT,

    DYNAMIC_VALUE_SOURCE_STRENGTH,
    DYNAMIC_VALUE_SOURCE_INTELLIGENCE,
    DYNAMIC_VALUE_SOURCE_AGILITY,

    DYNAMIC_VALUE_SOURCE_ATTACKSPEED,
    DYNAMIC_VALUE_SOURCE_CASTSPEED,
    DYNAMIC_VALUE_SOURCE_ATTACKACTIONTIME,
    DYNAMIC_VALUE_SOURCE_ATTACKDURATION,
    DYNAMIC_VALUE_SOURCE_ATTACKCOOLDOWN,
    DYNAMIC_VALUE_SOURCE_ATTACKDAMAGE,

    DYNAMIC_VALUE_SOURCE_ACCUMULATOR,
    DYNAMIC_VALUE_SOURCE_LIFETIME,
    DYNAMIC_VALUE_SOURCE_TEAM,
    DYNAMIC_VALUE_SOURCE_MOVESPEED,

    // Requires a combat valid event
    DYNAMIC_VALUE_TOTAL_ADJUSTED_DAMAGE,
    DYNAMIC_VALUE_APPLIED_DAMAGE,

    DYNAMIC_COMBAT_TARGET,
    DYNAMIC_COMBAT_EFFECTTYPE,
    DYNAMIC_COMBAT_DAMAGETYPE,
    DYNAMIC_COMBAT_SUPERTYPE,
    DYNAMIC_COMBAT_BASEDAMAGE,
    DYNAMIC_COMBAT_ADDITIONALDAMAGE,
    DYNAMIC_COMBAT_DAMAGEMULTIPLIER,
    DYNAMIC_COMBAT_BONUSDAMAGE,
    DYNAMIC_COMBAT_BONUSMULTIPLIER,
    DYNAMIC_COMBAT_LIFESTEAL,
    DYNAMIC_COMBAT_EVASION,
    DYNAMIC_COMBAT_MISSCHANCE,
    DYNAMIC_COMBAT_NONLETHAL,
    DYNAMIC_COMBAT_TRUESTRIKE,
    DYNAMIC_COMBAT_NEGATED,
    DYNAMIC_COMBAT_DEFLECTION,
    DYNAMIC_COMBAT_MANACOST,
    DYNAMIC_COMBAT_COOLDOWNTIME,
    DYNAMIC_COMBAT_ATTACKABILITY,
    DYNAMIC_COMBAT_ARMORPIERCE,
    DYNAMIC_COMBAT_MAGICARMORPIERCE,

    DYNAMIC_DAMAGE_SUPERTYPE,
    DYNAMIC_DAMAGE_EFFECTTYPE,
    DYNAMIC_DAMAGE_ATTEMPTED,
    DYNAMIC_DAMAGE_APPLIED,
    DYNAMIC_DAMAGE_DEFLECTION,
    DYNAMIC_DAMAGE_TARGET,
    DYNAMIC_DAMAGE_ATTACKER,
    DYNAMIC_DAMAGE_INFLICTOR,
    DYNAMIC_DAMAGE_ARMORPIERCE,
    DYNAMIC_DAMAGE_MAGICARMORPIERCE
};

template<> inline EDynamicActionValue   GetDefaultEmptyValue<EDynamicActionValue>() { return DYNAMIC_VALUE_INVALID; }

inline EDynamicActionValue  GetDynamicActionValueFromString(const tstring &sActionMultiplier)
{
    if (CompareNoCase(sActionMultiplier, _CTS("result")) == 0)
        return DYNAMIC_VALUE_RESULT;
    else if (CompareNoCase(sActionMultiplier, _CTS("stack")) == 0)
        return DYNAMIC_VALUE_STACK;
    else if (CompareNoCase(sActionMultiplier, _CTS("var0")) == 0)
        return DYNAMIC_VALUE_VAR0;
    else if (CompareNoCase(sActionMultiplier, _CTS("var1")) == 0)
        return DYNAMIC_VALUE_VAR1;
    else if (CompareNoCase(sActionMultiplier, _CTS("var2")) == 0)
        return DYNAMIC_VALUE_VAR2;
    else if (CompareNoCase(sActionMultiplier, _CTS("var3")) == 0)
        return DYNAMIC_VALUE_VAR3;

    else if (CompareNoCase(sActionMultiplier, _CTS("frametime")) == 0)
        return DYNAMIC_VALUE_PER_SECOND;
    else if (CompareNoCase(sActionMultiplier, _CTS("lifetime_remaining")) == 0)
        return DYNAMIC_VALUE_LIFETIME_REMAINING;
    else if (CompareNoCase(sActionMultiplier, _CTS("charges")) == 0)
        return DYNAMIC_VALUE_CHARGES;

    else if (CompareNoCase(sActionMultiplier, _CTS("healthlost")) == 0)
        return DYNAMIC_VALUE_HEALTH_LOST;
    else if (CompareNoCase(sActionMultiplier, _CTS("movement")) == 0)
        return DYNAMIC_VALUE_MOVEMENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("accumulator")) == 0)
        return DYNAMIC_VALUE_ACCUMULATOR;
    else if (CompareNoCase(sActionMultiplier, _CTS("param")) == 0)
        return DYNAMIC_VALUE_PARAM;
    else if (CompareNoCase(sActionMultiplier, _CTS("lifetime")) == 0)
        return DYNAMIC_VALUE_LIFETIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("team")) == 0)
        return DYNAMIC_VALUE_TEAM;
    else if (CompareNoCase(sActionMultiplier, _CTS("time")) == 0)
        return DYNAMIC_VALUE_TIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("castduration")) == 0)
        return DYNAMIC_VALUE_CASTDURATION;
    else if (CompareNoCase(sActionMultiplier, _CTS("owner_counter")) == 0)
        return DYNAMIC_VALUE_OWNER_COUNTER;
    else if (CompareNoCase(sActionMultiplier, _CTS("level")) == 0)
        return DYNAMIC_VALUE_LEVEL;

    else if (CompareNoCase(sActionMultiplier, _CTS("source_entity")) == 0)
        return DYNAMIC_VALUE_SOURCE_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_entity")) == 0)
        return DYNAMIC_VALUE_TARGET_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("inflictor_entity")) == 0)
        return DYNAMIC_VALUE_INFLICTOR_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("proxy_entity")) == 0)
        return DYNAMIC_VALUE_PROXY_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("owner_entity")) == 0)
        return DYNAMIC_VALUE_OWNER_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("stack_entity")) == 0)
        return DYNAMIC_VALUE_STACK_ENTITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("this_proxy_entity")) == 0)
        return DYNAMIC_VALUE_THIS_PROXY_ENTITY;

    else if (CompareNoCase(sActionMultiplier, _CTS("target_damage")) == 0)
        return DYNAMIC_VALUE_TARGET_DAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_damage")) == 0)
        return DYNAMIC_VALUE_SOURCE_DAMAGE;

    else if (CompareNoCase(sActionMultiplier, _CTS("target_health")) == 0)
        return DYNAMIC_VALUE_TARGET_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_health_percent")) == 0)
        return DYNAMIC_VALUE_TARGET_HEALTH_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_maxhealth")) == 0)
        return DYNAMIC_VALUE_TARGET_TOTAL_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_missinghealth")) == 0)
        return DYNAMIC_VALUE_TARGET_MISSING_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_missinghealth_percent")) == 0)
        return DYNAMIC_VALUE_TARGET_MISSING_HEALTH_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_mana")) == 0)
        return DYNAMIC_VALUE_TARGET_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_mana_percent")) == 0)
        return DYNAMIC_VALUE_TARGET_MANA_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_maxmana")) == 0)
        return DYNAMIC_VALUE_TARGET_TOTAL_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_missingmana")) == 0)
        return DYNAMIC_VALUE_TARGET_MISSING_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_missingmana_percent")) == 0)
        return DYNAMIC_VALUE_TARGET_MISSING_MANA_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_strength")) == 0)
        return DYNAMIC_VALUE_TARGET_STRENGTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_intelligence")) == 0)
        return DYNAMIC_VALUE_TARGET_INTELLIGENCE;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_agility")) == 0)
        return DYNAMIC_VALUE_TARGET_AGILITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_attackspeed")) == 0)
        return DYNAMIC_VALUE_TARGET_ATTACKSPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_castspeed")) == 0)
        return DYNAMIC_VALUE_TARGET_CASTSPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_movespeed")) == 0)
        return DYNAMIC_VALUE_TARGET_MOVESPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_attackactiontime")) == 0)
        return DYNAMIC_VALUE_TARGET_ATTACKACTIONTIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_attackduration")) == 0)
        return DYNAMIC_VALUE_TARGET_ATTACKDURATION;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_attackcooldown")) == 0)
        return DYNAMIC_VALUE_TARGET_ATTACKCOOLDOWN;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_attackdamage")) == 0)
        return DYNAMIC_VALUE_TARGET_ATTACKDAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_accumulator")) == 0)
        return DYNAMIC_VALUE_TARGET_ACCUMULATOR;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_lifetime")) == 0)
        return DYNAMIC_VALUE_TARGET_LIFETIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("target_team")) == 0)
        return DYNAMIC_VALUE_TARGET_TEAM;
    
    else if (CompareNoCase(sActionMultiplier, _CTS("source_health")) == 0)
        return DYNAMIC_VALUE_SOURCE_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_health_percent")) == 0)
        return DYNAMIC_VALUE_SOURCE_HEALTH_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_maxhealth")) == 0)
        return DYNAMIC_VALUE_SOURCE_TOTAL_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_missinghealth")) == 0)
        return DYNAMIC_VALUE_SOURCE_MISSING_HEALTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_missinghealth_percent")) == 0)
        return DYNAMIC_VALUE_SOURCE_MISSING_HEALTH_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_mana")) == 0)
        return DYNAMIC_VALUE_SOURCE_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_mana_percent")) == 0)
        return DYNAMIC_VALUE_SOURCE_MANA_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_maxmana")) == 0)
        return DYNAMIC_VALUE_SOURCE_TOTAL_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_missingmana")) == 0)
        return DYNAMIC_VALUE_SOURCE_MISSING_MANA;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_missingmana_percent")) == 0)
        return DYNAMIC_VALUE_SOURCE_MISSING_MANA_PERCENT;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_strength")) == 0)
        return DYNAMIC_VALUE_SOURCE_STRENGTH;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_intelligence")) == 0)
        return DYNAMIC_VALUE_SOURCE_INTELLIGENCE;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_agility")) == 0)
        return DYNAMIC_VALUE_SOURCE_AGILITY;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_attackspeed")) == 0)
        return DYNAMIC_VALUE_SOURCE_ATTACKSPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_castspeed")) == 0)
        return DYNAMIC_VALUE_SOURCE_CASTSPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_movespeed")) == 0)
        return DYNAMIC_VALUE_SOURCE_MOVESPEED;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_attackactiontime")) == 0)
        return DYNAMIC_VALUE_SOURCE_ATTACKACTIONTIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_attackduration")) == 0)
        return DYNAMIC_VALUE_SOURCE_ATTACKDURATION;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_attackcooldown")) == 0)
        return DYNAMIC_VALUE_SOURCE_ATTACKCOOLDOWN;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_attackdamage")) == 0)
        return DYNAMIC_VALUE_SOURCE_ATTACKDAMAGE;   
    else if (CompareNoCase(sActionMultiplier, _CTS("source_accumulator")) == 0)
        return DYNAMIC_VALUE_SOURCE_ACCUMULATOR;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_lifetime")) == 0)
        return DYNAMIC_VALUE_SOURCE_LIFETIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("source_team")) == 0)
        return DYNAMIC_VALUE_SOURCE_TEAM;

    else if (CompareNoCase(sActionMultiplier, _CTS("total_adjusted_damage")) == 0)
        return DYNAMIC_VALUE_TOTAL_ADJUSTED_DAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("applied_damage")) == 0)
        return DYNAMIC_VALUE_APPLIED_DAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_target")) == 0)
        return DYNAMIC_COMBAT_TARGET;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_effecttype")) == 0)
        return DYNAMIC_COMBAT_EFFECTTYPE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_damagetype")) == 0)
        return DYNAMIC_COMBAT_DAMAGETYPE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_supertype")) == 0)
        return DYNAMIC_COMBAT_SUPERTYPE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_basedamage")) == 0)
        return DYNAMIC_COMBAT_BASEDAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_additionaldamage")) == 0)
        return DYNAMIC_COMBAT_ADDITIONALDAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_damagemultiplier")) == 0)
        return DYNAMIC_COMBAT_DAMAGEMULTIPLIER;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_bonusdamage")) == 0)
        return DYNAMIC_COMBAT_BONUSDAMAGE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_bonusmultiplier")) == 0)
        return DYNAMIC_COMBAT_BONUSMULTIPLIER;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_lifesteal")) == 0)
        return DYNAMIC_COMBAT_LIFESTEAL;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_evasion")) == 0)
        return DYNAMIC_COMBAT_EVASION;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_misschance")) == 0)
        return DYNAMIC_COMBAT_MISSCHANCE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_nonlethal")) == 0)
        return DYNAMIC_COMBAT_NONLETHAL;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_truestrike")) == 0)
        return DYNAMIC_COMBAT_TRUESTRIKE;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_negated")) == 0)
        return DYNAMIC_COMBAT_NEGATED;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_deflection")) == 0)
        return DYNAMIC_COMBAT_DEFLECTION;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_manacost")) == 0)
        return DYNAMIC_COMBAT_MANACOST;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_cooldowntime")) == 0)
        return DYNAMIC_COMBAT_COOLDOWNTIME;
    else if (CompareNoCase(sActionMultiplier, _CTS("combat_attackability")) == 0)
        return DYNAMIC_COMBAT_ATTACKABILITY;

    else if (CompareNoCase(sActionMultiplier, _CTS("damage_supertype")) == 0)
        return DYNAMIC_DAMAGE_SUPERTYPE;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_effecttype")) == 0)
        return DYNAMIC_DAMAGE_EFFECTTYPE;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_attempted")) == 0)
        return DYNAMIC_DAMAGE_ATTEMPTED;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_applied")) == 0)
        return DYNAMIC_DAMAGE_APPLIED;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_deflection")) == 0)
        return DYNAMIC_DAMAGE_DEFLECTION;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_target")) == 0)
        return DYNAMIC_DAMAGE_TARGET;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_attacker")) == 0)
        return DYNAMIC_DAMAGE_ATTACKER;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_inflictor")) == 0)
        return DYNAMIC_DAMAGE_INFLICTOR;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_armorpierce")) == 0)
        return DYNAMIC_DAMAGE_ARMORPIERCE;
    else if (CompareNoCase(sActionMultiplier, _CTS("damage_magicarmorpierce")) == 0)
        return DYNAMIC_DAMAGE_MAGICARMORPIERCE;
    
    else
        return DYNAMIC_VALUE_INVALID;
}

inline EDynamicActionValue& AtoX(const tstring &s, EDynamicActionValue &e)  { return e = GetDynamicActionValueFromString(s); }


enum EPositionModifier
{
    POSITION_START,
    POSITION_END,
    POSITION_POINT_ON_LINE,
    POSITION_MAX_ON_LINE,
    POSITION_MIN_ON_LINE,
    POSITION_PERCENT_ON_LINE,
    POSITION_POINT_PAST_LINE,
    POSITION_DIRECTION_OFFSET
};

template<> inline EPositionModifier GetDefaultEmptyValue<EPositionModifier>()   { return POSITION_END; }

inline EPositionModifier    GetPositionModifierFromString(const tstring &sPositionModifier)
{
    if (CompareNoCase(sPositionModifier, _CTS("start")) == 0)
        return POSITION_START;
    else if (CompareNoCase(sPositionModifier, _CTS("end")) == 0)
        return POSITION_END;
    else if (CompareNoCase(sPositionModifier, _CTS("pointonline")) == 0)
        return POSITION_POINT_ON_LINE;
    else if (CompareNoCase(sPositionModifier, _CTS("maxonline")) == 0)
        return POSITION_MAX_ON_LINE;
    else if (CompareNoCase(sPositionModifier, _CTS("minonline")) == 0)
        return POSITION_MIN_ON_LINE;
    else if (CompareNoCase(sPositionModifier, _CTS("percentonline")) == 0)
        return POSITION_PERCENT_ON_LINE;
    else if (CompareNoCase(sPositionModifier, _CTS("pointpastline")) == 0)
        return POSITION_POINT_PAST_LINE;
    else if (CompareNoCase(sPositionModifier, _CTS("directionoffset")) == 0)
        return POSITION_DIRECTION_OFFSET;
    else
        return POSITION_END;
}

inline EPositionModifier&   AtoX(const tstring &s, EPositionModifier &e)    { return e = GetPositionModifierFromString(s); }

inline CVec3f   ApplyPositionModifier(const EPositionModifier &e, const CVec3f &v3Origin, const CVec3f &v3Target, float fValue, const CVec3f &v3Offset)
{
    CVec3f v3NewTarget;

    if (DistanceSq(v3Origin, v3Target) < SQR(0.001f))
        return v3Origin;

    switch (e)
    {
        case POSITION_POINT_ON_LINE:
            v3NewTarget = M_PointOnLine(v3Origin, Normalize(v3Target - v3Origin), fValue);
            break;

        case POSITION_MAX_ON_LINE:
            v3NewTarget = M_PointOnLine(v3Origin, Normalize(v3Target - v3Origin), MAX(fValue, Distance(v3Origin, v3Target)));
            break;

        case POSITION_MIN_ON_LINE:
            v3NewTarget = M_PointOnLine(v3Origin, Normalize(v3Target - v3Origin), MIN(fValue, Distance(v3Origin, v3Target)));
            break;

        case POSITION_PERCENT_ON_LINE:
            v3NewTarget = M_PointOnLine(v3Origin, Normalize(v3Target - v3Origin), Distance(v3Origin, v3Target) * fValue);
            break;

        case POSITION_POINT_PAST_LINE:
            v3NewTarget = M_PointOnLine(v3Target, Normalize(v3Target - v3Origin), fValue);
            break;

        case POSITION_DIRECTION_OFFSET:
            {
                CAxis axis(M_GetAnglesFromForwardVec(Normalize(v3Target - v3Origin)));

                v3NewTarget = v3Origin + TransformPoint(v3Offset, axis);
            }
            break;

        case POSITION_START:
            v3NewTarget = v3Origin;
            break;

        case POSITION_END:
        default:
            v3NewTarget = v3Target;
            break;
    }

    return v3NewTarget;
}

inline CVec2f   ApplyPositionModifier(const EPositionModifier &e, const CVec2f &v2Origin, const CVec2f &v2Target, float fValue, const CVec2f &v2Offset)
{
    CVec2f v2NewTarget;

    if (DistanceSq(v2Origin, v2Target) < SQR(0.001f))
        return v2Origin;

    switch (e)
    {
        case POSITION_POINT_ON_LINE:
            v2NewTarget = M_PointOnLine(v2Origin, Normalize(v2Target - v2Origin), fValue);
            break;

        case POSITION_MAX_ON_LINE:
            v2NewTarget = M_PointOnLine(v2Origin, Normalize(v2Target - v2Origin), MAX(fValue, Distance(v2Origin, v2Target)));
            break;

        case POSITION_MIN_ON_LINE:
            v2NewTarget = M_PointOnLine(v2Origin, Normalize(v2Target - v2Origin), MIN(fValue, Distance(v2Origin, v2Target)));
            break;

        case POSITION_PERCENT_ON_LINE:
            v2NewTarget = M_PointOnLine(v2Origin, Normalize(v2Target - v2Origin), Distance(v2Origin, v2Target) * fValue);
            break;

        case POSITION_POINT_PAST_LINE:
            v2NewTarget = M_PointOnLine(v2Target, Normalize(v2Target - v2Origin), fValue);
            break;

        case POSITION_DIRECTION_OFFSET:
            {
                float fYaw(M_YawToPosition(v2Origin, v2Target));

                CVec2f v2RotatedOffset(v2Offset);
                v2RotatedOffset.Rotate(fYaw);

                v2NewTarget = v2Origin + v2RotatedOffset;
            }
            break;

        case POSITION_START:
            v2NewTarget = v2Origin;
            break;

        case POSITION_END:
        default:
            v2NewTarget = v2Target;
            break;
    }

    return v2NewTarget;
}
//=============================================================================

//=============================================================================
// SCombatActionEnv
//=============================================================================
struct SCombatActionEnv
{
    IGameEntity*            pThis;
    uint                    uiLevel;
    IGameEntity*            pInitiator;
    IGameEntity*            pInflictor;
    IGameEntity*            pTarget;
    IGameEntity*            pProxy;
    CVec3f                  v3Target;
    CCombatEvent*           pCombatEvent;
    CDamageEvent*           pDamageEvent;
    CVec3f                  v3Delta;
    CScriptThread*          pScriptThread;

    CombatActionScript_cit  citAct;
    fvector                 vStack;
    uivector                vEntityStack;
    float                   fResult;
    float                   fVar0;
    float                   fVar1;
    float                   fVar2;
    float                   fVar3;
    CVec3f                  v3Pos0;
    CVec3f                  v3Pos1;
    CVec3f                  v3Pos2;
    CVec3f                  v3Pos3;
    IGameEntity*            pEnt0;
    IGameEntity*            pEnt1;
    IGameEntity*            pEnt2;
    IGameEntity*            pEnt3;

    bool                    bStall;
    bool                    bTerminate;
    uint                    uiRepeated;
    uint                    uiTracker;

    SCombatActionEnv*       pNext;
};
//=============================================================================

//=============================================================================
// ICombatAction
//=============================================================================
class ICombatAction
{
protected:
    SCombatActionEnv*   m_pEnv;

    COMBAT_ACTION_PROPERTY(EActionTarget, Source)
    COMBAT_ACTION_PROPERTY(EActionTarget, Target)

public:
    virtual ~ICombatAction()    {}
    ICombatAction() :
    m_pEnv(NULL)
    {}

    void            SetEnv(SCombatActionEnv *pEnv)                      { m_pEnv = pEnv; }

    void            PushStack(float fValue) const           { m_pEnv->vStack.push_back(fValue); }
    void            PopStack() const                        { m_pEnv->vStack.pop_back(); }
    float           PeekStack() const                       { return (m_pEnv->vStack.empty()) ? 0.0f : m_pEnv->vStack.back(); }

    void            PushEntity(uint uiEntityUID) const      { m_pEnv->vEntityStack.push_back(uiEntityUID); }
    void            PopEntity() const                       { m_pEnv->vEntityStack.pop_back(); }
    uint            PeekEntity() const                      { return (m_pEnv->vEntityStack.empty()) ? INVALID_INDEX : m_pEnv->vEntityStack.back(); }

    IGameEntity*    GetThis() const                         { return m_pEnv->pThis; }
    uint            GetLevel() const                        { return m_pEnv->uiLevel; }

    IGameEntity*    GetEntityFromActionTarget(EActionTarget eTarget) const;
    IUnitEntity*    GetUnitFromActionTarget(EActionTarget eTarget) const;

    template <class T> T*   GetEntityFromActionTargetAs(EActionTarget eTarget) const
    {
        IGameEntity *pEntity(GetEntityFromActionTarget(eTarget));
        if (pEntity == NULL)
            return NULL;
        else
            return pEntity->GetAs<T>();
    }

    CVec3f          GetPositionFromActionTarget(EActionTarget eTarget) const;
    float           GetDynamicValue(EDynamicActionValue eMultiplier) const;
    void            SetDynamicValue(EDynamicActionValue eMultiplier, float fValue);
    void            SetDynamicEffectType(EDynamicActionValue eMultiplier, uint uiEffectType);
    float           Evaluate(float fA, float fB, EActionOperator eOp) const;
    bool            Compare(float fA, float fB, EActionCmpOperator eOp) const;

    IGameEntity*    GetSourceEntity() const     { return GetEntityFromActionTarget(GetSource()); }
    IGameEntity*    GetTargetEntity() const     { return GetEntityFromActionTarget(GetTarget()); }
    IUnitEntity*    GetSourceUnit() const;
    IUnitEntity*    GetTargetUnit() const;
    CVec3f          GetSourcePosition() const   { return GetPositionFromActionTarget(GetSource()); }
    CVec3f          GetTargetPosition() const   { return GetPositionFromActionTarget(GetTarget()); }
    CVec3f          GetDeltaPosition() const    { return GetPositionFromActionTarget(ACTION_TARGET_DELTA_POSITION); }

    virtual float   Execute() = 0;
    virtual void    Precache(EPrecacheScheme eScheme, const tstring &sModifier)     {}
    virtual void    GetPrecacheList(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)   {}
};
//=============================================================================

//=============================================================================
// IActionScript
//=============================================================================
class IActionScript
{
public:
    virtual void    AddAction(ICombatAction *pAction) = 0;
};
//=============================================================================

//=============================================================================
// CCombatActionScript
//=============================================================================
class CCombatActionScript : public IActionScript
{
private:
    EEntityActionScript m_eAction;
    uint                m_uiModifierID;
    uint                m_uiEffectDescriptionIndex;
    int                 m_iPriority;
    int                 m_iScriptPriority;
    CombatActionScript  m_vActions;

    uint                m_uiThisUID;
    uint                m_uiLevel;

    bool                m_bPropagateToIllusions;
    bool                m_bActivateOnBounces;
    
public:
    ~CCombatActionScript()  {}

    CCombatActionScript() :
    m_uiEffectDescriptionIndex(INVALID_INDEX),
    m_uiModifierID(INVALID_INDEX),
    m_iPriority(0),
    m_uiThisUID(INVALID_INDEX),
    m_uiLevel(0),
    m_bPropagateToIllusions(false),
    m_bActivateOnBounces(false),
    m_iScriptPriority(0)
    {}

    CCombatActionScript(EEntityActionScript eAction, int iScriptPriority, bool bPropagateToIllusions, bool bActivateOnBounces, uint uiModifierID) :
    m_eAction(eAction),
    m_uiModifierID(uiModifierID),
    m_uiEffectDescriptionIndex(INVALID_INDEX),
    m_iPriority(-1),
    m_uiThisUID(INVALID_INDEX),
    m_uiLevel(0),
    m_bPropagateToIllusions(bPropagateToIllusions),
    m_bActivateOnBounces(bActivateOnBounces),
    m_iScriptPriority(iScriptPriority)
    {}

    virtual void    AddAction(ICombatAction *pAction)       { m_vActions.push_back(pAction); }
    
    GAME_SHARED_API void            FetchEffectDescription(const tstring &sName);
    GAME_SHARED_API const tstring&  GetEffectDescription() const;
    GAME_SHARED_API uint            GetEffectDescriptionIndex() const;
    int                             GetPriority() const             { return m_iPriority; }
    uint                            GetModifierID() const           { return m_uiModifierID; }
    int                             GetScriptPriority() const       { return m_iScriptPriority; }
    CombatActionScript&             GetActions()                    { return m_vActions; }

    void    SetLevel(uint uiLevel)                          { m_uiLevel = uiLevel; }
    void    SetThisUID(uint uiIndex)                        { m_uiThisUID = uiIndex; }

    bool    GetPropagateToIllusions()                       { return m_bPropagateToIllusions; }
    bool    GetActivateOnBounces()                          { return m_bActivateOnBounces; }

    float   Execute(IGameEntity *pInflictor, IGameEntity *pInitiator, IGameEntity *pTarget, const CVec3f &v3Target, IGameEntity *pProxy, CCombatEvent *pCombatEvent, CDamageEvent *pDamageEvent, CScriptThread *pScriptThread, const CVec3f &v3Delta, float fDefault);
    void    ExecuteActions(SCombatActionEnv &cReg);
    
    void    Precache(EPrecacheScheme eScheme, const tstring &sModifier) const
    {
        for (CombatActionScript_cit it(m_vActions.begin()); it != m_vActions.end(); ++it)
            (*it)->Precache(eScheme, sModifier);
    }

    void    GetPrecacheList(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache) const
    {
        for (CombatActionScript_cit it(m_vActions.begin()); it != m_vActions.end(); ++it)
            (*it)->GetPrecacheList(eScheme, sModifier, deqPrecache);
    }
};
//=============================================================================

//=============================================================================
// ICombatActionBranch
//=============================================================================
class ICombatActionBranch : public ICombatAction
{
private:
    CCombatActionScript     m_script;

public:
    ~ICombatActionBranch()  {}
    ICombatActionBranch()   {}

    void    AddAction(ICombatAction *pAction)
    {
        m_script.AddAction(pAction);
    }

    CCombatActionScript*    GetActionScript()
    {
        return &m_script;
    }

    void    Precache(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        m_script.Precache(eScheme, sModifier);
    }

    void    GetPrecacheList(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
    {
        m_script.GetPrecacheList(eScheme, sModifier, deqPrecache);
    }

    void    ExecuteActions();
};
//=============================================================================

#endif //__I_COMBATACTION_H__
