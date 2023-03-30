// (C)2008 S2 Games
// i_entitydefinition.h
//
//=============================================================================
#ifndef __I_ENTITYDEFINITION_H__
#define __I_ENTITYDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_baseentityallocator.h"

#include "../k2/i_resource.h"
#include "../k2/c_temporalproperty.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IBaseEntityAllocator;
class CCombatEvent;
class CCombatActionScript;
class CDamageEvent;

GAME_SHARED_API tstring GetActionScriptName(uint uiActionScript);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    int     m_i##name##Priority; \
public: \
    int     Get##name##Priority() const         { return m_i##name##Priority; } \
    void    Set##name##Priority(int iPriority)  { m_i##name##Priority = iPriority; }

// ENT_DEF_PROPERTY
#define ENT_DEF_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    type    m_##name; \
public: \
    void    Set##name(const tstring &s)     { AtoX(s, m_##name); } \
    void    Set##name(type value)           { m_##name = value; } \
    type    Get##name() const               { return m_##name; }

// ENT_DEF_PROPERTY_EX
#define ENT_DEF_PROPERTY_EX(name, type, function) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    type    m_##name; \
public: \
    void    Set##name(const tstring &s)     { m_##name = function(s); } \
    void    Set##name(type value)           { m_##name = value; } \
    type    Get##name() const               { return m_##name; }

// ENT_DEF_STRING_PROPERTY
#define ENT_DEF_STRING_PROPERTY(name) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    tstring m_s##name; \
public: \
    void            Set##name(const tstring &s) { m_s##name = s; } \
    const tstring&  Get##name() const           { return m_s##name; }

// ENT_DEF_LOCALIZED_STRING_PROPERTY
#define ENT_DEF_LOCALIZED_STRING_PROPERTY(name) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    uint    m_ui##name##Index; \
public: \
    void            Set##name(uint uiIndex)     { m_ui##name##Index = uiIndex; } \
    uint            Get##name##Index() const    { return m_ui##name##Index; } \
    const tstring&  Get##name() const           { return Game.GetEntityString(m_ui##name##Index); }

// ENT_DEF_RESOURCE_PROPERTY
#define ENT_DEF_RESOURCE_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    tstring     m_s##name; \
    ResHandle   m_h##name; \
public: \
    void            Set##name(const tstring &s)     { m_s##name = FileManager.SanitizePath(s); m_h##name = INVALID_RESOURCE; } \
    const tstring&  Get##name##Path() const         { return m_s##name; } \
    ResHandle       Get##name() const               { return m_h##name; } \
    void            Precache##name()                { m_h##name = Game.Register##type(m_s##name); }

// ENT_DEF_TEMPORAL_PROPERTY
#define ENT_DEF_TEMPORAL_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name##Temporal) \
protected: \
    type    m_##name##Start; \
    type    m_##name##Mid; \
    type    m_##name##End; \
    float   m_f##name##MidPos; \
    type    m_##name##Speed; \
    uint    m_ui##name##Delay; \
    uint    m_ui##name##Duration; \
public: \
    void    Set##name##Start(const tstring &s)      { AtoX(s, m_##name##Start); } \
    void    Set##name##Mid(const tstring &s)        { AtoX(s, m_##name##Mid); } \
    void    Set##name##End(const tstring &s)        { AtoX(s, m_##name##End); } \
    void    Set##name##MidPos(const tstring &s)     { AtoX(s, m_f##name##MidPos); } \
    void    Set##name##Speed(const tstring &s)      { AtoX(s, m_##name##Speed); } \
    void    Set##name##Delay(const tstring &s)      { AtoX(s, m_ui##name##Delay); } \
    void    Set##name##Duration(const tstring &s)   { AtoX(s, m_ui##name##Duration); } \
\
    type    Get##name##Start() const    { return m_##name##Start; } \
    type    Get##name##Mid() const      { return m_##name##Mid; } \
    type    Get##name##End() const      { return m_##name##End; } \
    float   Get##name##MidPos() const   { return m_f##name##MidPos; } \
    type    Get##name##Speed() const    { return m_##name##Speed; } \
    uint    Get##name##Delay() const    { return m_ui##name##Delay; } \
    uint    Get##name##Duration() const { return m_ui##name##Duration; }

// ENT_DEF_ARRAY_SET
#define ENT_DEF_ARRAY_SET(name, type) \
void    Set##name(const tstring &s) \
{ \
    m_v##name.clear(); \
    type _t; \
    const tsvector &vValues(TokenizeString(s, _T(','))); \
    for (tsvector_cit it(vValues.begin()); it != vValues.end(); ++it) \
        m_v##name.push_back(AtoX(*it, _t)); \
}

// ENT_DEF_ARRAY_SET_EX
#define ENT_DEF_ARRAY_SET_EX(name, type, function) \
void    Set##name(const tstring &s) \
{ \
    m_v##name.clear(); \
    const tsvector &vValues(TokenizeString(s, _T(','))); \
    for (tsvector_cit it(vValues.begin()); it != vValues.end(); ++it) \
        m_v##name.push_back(function(*it)); \
}

// ENT_DEF_ARRAY_GET
#define ENT_DEF_ARRAY_GET(name, type) \
type    Get##name(uint uiIndex) const \
{ \
    if (m_v##name.empty()) \
        return GetDefaultEmptyValue<type>(); \
    return m_v##name[MIN(uiIndex, INT_SIZE(m_v##name.size()) - 1)]; \
} \
\
uint    Get##name##Size() const     { return INT_SIZE(m_v##name.size()); }

// ENT_DEF_ARRAY_PROPERTY
#define ENT_DEF_ARRAY_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    vector<type>    m_v##name; \
public: \
    ENT_DEF_ARRAY_SET(name, type) \
    ENT_DEF_ARRAY_GET(name, type)

// ENT_DEF_ARRAY_PROPERTY_EX
#define ENT_DEF_ARRAY_PROPERTY_EX(name, type, function) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    vector<type>    m_v##name; \
public: \
    ENT_DEF_ARRAY_SET_EX(name, type, function) \
    ENT_DEF_ARRAY_GET(name, type)

// ENT_DEF_STRING_ARRAY_PROPERTY
#define ENT_DEF_STRING_ARRAY_PROPERTY(name) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    tsvector    m_vs##name; \
public: \
    void    Set##name(const tstring &s) \
    { \
        m_vs##name = TokenizeString(s, _T(',')); \
    } \
\
    const tstring&  Get##name(uint uiIndex) const \
    { \
        if (m_vs##name.empty()) \
            return TSNULL; \
        return m_vs##name[MIN(uiIndex, INT_SIZE(m_vs##name.size()) - 1)]; \
    } \
\
    uint    Get##name##Size() const     { return INT_SIZE(m_vs##name.size()); }

// ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY
#define ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY(name) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    vector<tsvector>    m_vvs##name; \
public: \
    void    Set##name(const tstring &s) \
    { \
        m_vvs##name.clear(); \
        tsvector vValues(TokenizeString(s, _T(','))); \
        for (tsvector_it it(vValues.begin()); it != vValues.end(); ++it) \
            m_vvs##name.push_back(TokenizeString(*it, _T(' '))); \
    } \
\
    const tsvector& Get##name(uint uiIndex) const \
    { \
        if (m_vvs##name.empty()) \
            return VSNULL; \
        return m_vvs##name[MIN(uiIndex, INT_SIZE(m_vvs##name.size()) - 1)]; \
    } \
\
    uint    Get##name##Size() const     { return INT_SIZE(m_vvs##name.size()); }

// ENT_DEF_RESOURCE_ARRAY_PROPERTY
#define ENT_DEF_RESOURCE_ARRAY_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name) \
protected: \
    tsvector            m_vs##name; \
    vector<ResHandle>   m_v##name; \
public: \
    void            Set##name(const tstring &s) \
    { \
        m_vs##name = TokenizeString(s, _T(',')); \
        for (uint ui(0); ui < m_vs##name.size(); ++ui) \
            m_vs##name[ui] = FileManager.SanitizePath(m_vs##name[ui]); \
    } \
\
    const tstring&  Get##name##Path(uint uiIndex) const \
    { \
        if (m_vs##name.empty()) \
            return TSNULL; \
        return m_vs##name[MIN(uiIndex, INT_SIZE(m_vs##name.size()) - 1)]; \
    } \
\
    ResHandle   Get##name(uint uiIndex) const \
    { \
        if (m_v##name.empty()) \
            return INVALID_RESOURCE; \
        return m_v##name[MIN(uiIndex, INT_SIZE(m_v##name.size()) - 1)]; \
    } \
\
    uint    Get##name##Size() const     { return INT_SIZE(m_vs##name.size()); } \
\
    void    Precache##name() \
    { \
        m_v##name.clear(); \
        for (uint ui(0); ui < Get##name##Size(); ++ui) \
            m_v##name.push_back(Game.Register##type(m_vs##name[ui])); \
    }

// ENT_DEF_TEMPORAL_ARRAY_PROPERTY
#define ENT_DEF_TEMPORAL_ARRAY_PROPERTY(name, type) \
ENT_DEF_MOD_PRIORITY_TRACKER(name##Temporal) \
protected: \
    vector<type>    m_v##name##Start; \
    vector<type>    m_v##name##Mid; \
    vector<type>    m_v##name##End; \
    vector<float>   m_v##name##MidPos; \
    vector<type>    m_v##name##Speed; \
    vector<uint>    m_v##name##Delay; \
    vector<uint>    m_v##name##Duration; \
public: \
    ENT_DEF_ARRAY_SET(name##Start, type) \
    ENT_DEF_ARRAY_SET(name##Mid, type) \
    ENT_DEF_ARRAY_SET(name##End, type) \
    ENT_DEF_ARRAY_SET(name##MidPos, float) \
    ENT_DEF_ARRAY_SET(name##Speed, type) \
    ENT_DEF_ARRAY_SET(name##Delay, uint) \
    ENT_DEF_ARRAY_SET(name##Duration, uint) \
\
    ENT_DEF_ARRAY_GET(name##Start, type) \
    ENT_DEF_ARRAY_GET(name##Mid, type) \
    ENT_DEF_ARRAY_GET(name##End, type) \
    ENT_DEF_ARRAY_GET(name##MidPos, float) \
    ENT_DEF_ARRAY_GET(name##Speed, type) \
    ENT_DEF_ARRAY_GET(name##Delay, uint) \
    ENT_DEF_ARRAY_GET(name##Duration, uint)

// ENT_DEF_MUTABLE_TEMPORAL_ARRAY_PROPERTY
#define ENT_DEF_MUTABLE_TEMPORAL_ARRAY_PROPERTY(name, type) \
ENT_DEF_TEMPORAL_ARRAY_PROPERTY(name, type) \
ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Morph##name, type) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_MUTATION
#define ENT_DEF_MUTATION(name, type) \
ENT_DEF_PROPERTY(Morph##name, type) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_ARRAY_MUTATION
#define ENT_DEF_ARRAY_MUTATION(name, type) \
ENT_DEF_ARRAY_PROPERTY(Morph##name, type) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_ARRAY_MUTATION_EX
#define ENT_DEF_ARRAY_MUTATION_EX(name, type, function) \
ENT_DEF_ARRAY_PROPERTY_EX(Morph##name, type, function) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_STRING_ARRAY_MUTATION
#define ENT_DEF_STRING_ARRAY_MUTATION(name) \
ENT_DEF_STRING_ARRAY_PROPERTY(Morph##name) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_RESOURCE_ARRAY_MUTATION
#define ENT_DEF_RESOURCE_ARRAY_MUTATION(name, type) \
ENT_DEF_RESOURCE_ARRAY_PROPERTY(Morph##name, type) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_STRING_VECTOR_ARRAY_MUTATION
#define ENT_DEF_STRING_VECTOR_ARRAY_MUTATION(name) \
ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY(Morph##name) \
ENT_DEF_PROPERTY(Apply##name##Morph, bool)

// ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY
#define ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(name, type) \
ENT_DEF_ARRAY_PROPERTY(name, type) \
ENT_DEF_PROPERTY(name##PerLevel, type) \
ENT_DEF_ARRAY_PROPERTY(name##PerCharge, type)

// ENT_DEF_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY
#define ENT_DEF_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY(name, type) \
ENT_DEF_TEMPORAL_ARRAY_PROPERTY(name, type) \
ENT_DEF_TEMPORAL_PROPERTY(name##PerLevel, type) \
ENT_DEF_TEMPORAL_ARRAY_PROPERTY(name##PerCharge, type)

// PRECACHE_LOCALIZED_STRING
#define PRECACHE_LOCALIZED_STRING(name, tag) \
Set##name(INVALID_INDEX); \
if (GetModifierID() != INVALID_INDEX) \
    Set##name(Game.GetEntityStringIndex(GetName() + _CWS("_" _T(#tag) _T(":")) + EntityRegistry.LookupModifierKey(GetModifierID()))); \
if (Get##name##Index() == INVALID_INDEX) \
    Set##name(Game.GetEntityStringIndex(GetName() + _CWS("_" _T(#tag)))); \

// PRECACHE_ENTITY_ARRAY
#define PRECACHE_ENTITY_ARRAY(n, e) \
for (uint ui(0); ui < Get##n##Size(); ++ui) \
{ \
    if (Get##n##Priority() != GetPriority()) \
        continue; \
    Game.Precache(Get##n(ui), e); \
}

// PRECACHE_ENTITY_ARRAY2
#define PRECACHE_ENTITY_ARRAY2(n, e) \
for (uint ui(0); ui < Get##n##Size(); ++ui) \
    Game.Precache(Get##n(ui), e);

// GET_ENTITY_ARRAY_PRECACHE_LIST
#define GET_ENTITY_ARRAY_PRECACHE_LIST(n, e, c) \
for (uint ui(0); ui < Get##n##Size(); ++ui) \
{ \
    if (Get##n##Priority() != GetPriority()) \
        continue; \
    Game.GetPrecacheList(Get##n(ui), e, c); \
}

// GET_ENTITY_ARRAY_PRECACHE_LIST2
#define GET_ENTITY_ARRAY_PRECACHE_LIST2(n, e, c) \
for (uint ui(0); ui < Get##n##Size(); ++ui) \
    Game.GetPrecacheList(Get##n(ui), e, c);

// PRECACHE_ENTITY
#define PRECACHE_ENTITY(n) Game.Precache(Get##n());

// DECLARE_ENTITY_DEFINITION_XML_PROCESSOR
#define DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(entity, type, name) \
extern CBaseEntityAllocator<class entity>   g_allocator##type; \
\
namespace XML##type \
{ \
    DECLARE_XML_PROCESSOR(name) \
    DECLARE_XML_PROCESSOR(modifier) \
}

// START_ENTITY_DEFINITION_XML_PROCESSOR
#define START_ENTITY_DEFINITION_XML_PROCESSOR(baseclass, type) \
CBaseEntityAllocator<baseclass> g_allocator##type; \
\
namespace XML##type \
{ \
    void    Read##type##Settings(C##type##Definition *pDefinition, const CXMLNode &node, bool bMod) \
    {

// END_ENTITY_DEFINITION_XML_PROCESSOR
#define END_ENTITY_DEFINITION_XML_PROCESSOR(type, name) \
    } \
\
    BEGIN_XML_REGISTRATION(name) \
        REGISTER_XML_PROCESSOR(root) \
    END_XML_REGISTRATION \
\
    BEGIN_XML_PROCESSOR(name, CEntityDefinitionResource) \
        C##type##Definition *pDefinition(K2_NEW(g_heapResources,   C##type##Definition)); \
\
        pObject->SetName(node.GetProperty(_T("name"), _T("INVALID_ENTITY"))); \
        pObject->SetDefinition(pDefinition); \
\
        Read##type##Settings(pDefinition, node, false); \
    END_XML_PROCESSOR(pDefinition) \
\
    BEGIN_XML_REGISTRATION(modifier) \
        REGISTER_XML_PROCESSOR(name) \
    END_XML_REGISTRATION \
\
    BEGIN_XML_PROCESSOR(modifier, C##type##Definition) \
        if (!node.HasProperty(_T("key"))) \
            return true; \
\
        C##type##Definition *pModifier(K2_NEW(g_heapResources,   C##type##Definition)(*pObject)); \
        pModifier->SetPriority(node.GetPropertyInt(_T("modpriority"), 0)); \
        pModifier->SetExclusive(node.GetPropertyBool(_T("exclusive"), false)); \
        pModifier->SetCondition(node.GetProperty(_T("condition"))); \
        pObject->AddModifier(node.GetProperty(_T("key")), pModifier); \
        Read##type##Settings(pModifier, node, true); \
    END_XML_PROCESSOR(pModifier) \
}

// READ_ENTITY_DEFINITION_PROPERTY
#define READ_ENTITY_DEFINITION_PROPERTY(name, attribute) \
if (!bMod || node.HasProperty(_CWS(#attribute))) \
{ \
    pDefinition->Set##name(node.GetProperty(_CWS(#attribute))); \
    pDefinition->Set##name##Priority(pDefinition->GetPriority()); \
} \
else \
    pDefinition->Set##name##Priority(0);

// READ_SPLIT_ENTITY_DEFINITION_PROPERTY
#define READ_SPLIT_ENTITY_DEFINITION_PROPERTY(base, A, B, attribute, a, b) \
if (!bMod || node.HasProperty(_CWS(#attribute))) \
{ \
    pDefinition->Set##base##A(node.GetProperty(_CWS(#attribute))); \
    pDefinition->Set##base##A##Priority(pDefinition->GetPriority()); \
    pDefinition->Set##base##B(node.GetProperty(_CWS(#attribute))); \
    pDefinition->Set##base##B##Priority(pDefinition->GetPriority()); \
} \
else \
{ \
    pDefinition->Set##base##A##Priority(0); \
    pDefinition->Set##base##B##Priority(0); \
} \
\
if (node.HasProperty(_CWS(#attribute _T(#a)))) \
{ \
    pDefinition->Set##base##A(node.GetProperty(_CWS(#attribute _T(#a)))); \
    pDefinition->Set##base##A##Priority(pDefinition->GetPriority()); \
} \
\
if (node.HasProperty(_CWS(#attribute _T(#b)))) \
{ \
    pDefinition->Set##base##B(node.GetProperty(_CWS(#attribute _T(#b)))); \
    pDefinition->Set##base##B##Priority(pDefinition->GetPriority()); \
}

// READ_SPLIT_ENTITY_DEFINITION_PROPERTY_SUFFIXED
#define READ_SPLIT_ENTITY_DEFINITION_PROPERTY_SUFFIXED(base, A, B, C, attribute, a, b, c) \
if (!bMod || node.HasProperty(_CWS(#attribute _T(#c)))) \
{ \
    pDefinition->Set##base##A##C(node.GetProperty(_CWS(#attribute _T(#c)))); \
    pDefinition->Set##base##A##C##Priority(pDefinition->GetPriority()); \
    pDefinition->Set##base##B##C(node.GetProperty(_CWS(#attribute _T(#c)))); \
    pDefinition->Set##base##B##C##Priority(pDefinition->GetPriority()); \
} \
else \
{ \
    pDefinition->Set##base##A##C##Priority(0); \
    pDefinition->Set##base##B##C##Priority(0); \
} \
\
if (node.HasProperty(_CWS(#attribute _T(#a) _T(#c)))) \
{ \
    pDefinition->Set##base##A##C(node.GetProperty(_CWS(#attribute _T(#a) _T(#c)))); \
    pDefinition->Set##base##A##C##Priority(pDefinition->GetPriority()); \
} \
\
if (node.HasProperty(_CWS(#attribute _T(#b) _T(#c)))) \
{ \
    pDefinition->Set##base##B##C(node.GetProperty(_CWS(#attribute _T(#b) _T(#c)))); \
    pDefinition->Set##base##B##C##Priority(pDefinition->GetPriority()); \
}

// READ_ENTITY_DEFINITION_PROPERTY_EX
#define READ_ENTITY_DEFINITION_PROPERTY_EX(name, attribute, def) \
if (!bMod || node.HasProperty(_CWS(#attribute))) \
{ \
    pDefinition->Set##name(node.GetProperty(_CWS(#attribute), _CWS(#def))); \
    pDefinition->Set##name##Priority(pDefinition->GetPriority()); \
} \
else \
    pDefinition->Set##name##Priority(0);

// READ_ENTITY_DEFINITION_PROPERTY_PREFIX
#define READ_ENTITY_DEFINITION_PROPERTY_PREFIX(name, prefix, attribute) \
if (!bMod || node.HasProperty(_T(#prefix) _T(#attribute))) \
{ \
    pDefinition->Set##name(node.GetProperty(_T(#prefix) _T(#attribute))); \
    pDefinition->Set##name##Priority(pDefinition->GetPriority()); \
} \
else \
    pDefinition->Set##name##Priority(0);

// READ_ENTITY_DEFINITION_PROPERTY_SUFFIX
#define READ_ENTITY_DEFINITION_PROPERTY_SUFFIX(name, attribute, suffix) \
if (!bMod || node.HasProperty(_T(#attribute) _T(#suffix))) \
{ \
    pDefinition->Set##name(node.GetProperty(_T(#attribute) _T(#suffix))); \
    pDefinition->Set##name##Priority(pDefinition->GetPriority()); \
} \
else \
    pDefinition->Set##name##Priority(0);

// READ_APPLY_MORPH
#define READ_APPLY_MORPH(name, attribute, suffix) \
if (!bMod || node.HasProperty(_T(#attribute) _T(#suffix))) \
{ \
    if (node.HasProperty(_T(#attribute) _T(#suffix))) \
        pDefinition->SetApply##name##Morph(true); \
    else \
        pDefinition->SetApply##name##Morph(false); \
    pDefinition->SetApply##name##MorphPriority(pDefinition->GetPriority()); \
} \
else \
{ \
    pDefinition->SetApply##name##Morph(false); \
    pDefinition->SetApply##name##MorphPriority(0); \
}

// READ_ENTITY_DEFINITION_MUTATION
#define READ_ENTITY_DEFINITION_MUTATION(name, attribute) \
READ_ENTITY_DEFINITION_PROPERTY_PREFIX(Morph##name, morph, attribute) \
READ_APPLY_MORPH(name, morph, attribute)

// READ_ENTITY_DEFINITION_TEMPORAL_PROPERTY
#define READ_ENTITY_DEFINITION_TEMPORAL_PROPERTY(name, attribute) \
if (!bMod || node.HasProperty(_T(#attribute)) || node.HasProperty(_T(#attribute) _T("start"))) \
    pDefinition->Set##name##Start(node.GetProperty(_T(#attribute) _T("start"), node.GetProperty(_T(#attribute)))); \
if (!bMod || node.HasProperty(_T(#attribute)) || node.HasProperty(_T(#attribute) _T("start")) || node.HasProperty(_T(#attribute) _T("end"))) \
    pDefinition->Set##name##End(node.GetProperty(_T(#attribute) _T("end"), node.GetProperty(_T(#attribute) _T("start"), node.GetProperty(_T(#attribute))))); \
if (node.HasProperty(_T(#attribute) _T("mid"))) \
{ \
    pDefinition->Set##name##Mid(node.GetProperty(_T(#attribute) _T("mid"))); \
} \
else \
{ \
    tstring sValue; \
    sValue = XtoA(pDefinition->Get##name##Start() + ((pDefinition->Get##name##End() - pDefinition->Get##name##Start()) / 2.0f)); \
\
    pDefinition->Set##name##Mid(sValue); \
} \
if (!bMod || node.HasProperty(_T(#attribute) _T("midpos"))) \
    pDefinition->Set##name##MidPos(node.GetProperty(_T(#attribute) _T("midpos"), _T("0.5"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("speed"))) \
    pDefinition->Set##name##Speed(node.GetProperty(_T(#attribute) _T("speed"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("delay"))) \
    pDefinition->Set##name##Delay(node.GetProperty(_T(#attribute) _T("delay"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("duration"))) \
    pDefinition->Set##name##Duration(node.GetProperty(_T(#attribute) _T("duration"))); \
if (!bMod || \
    node.HasProperty(_T(#attribute)) || \
    node.HasProperty(_T(#attribute) _T("start")) || \
    node.HasProperty(_T(#attribute) _T("mid")) || \
    node.HasProperty(_T(#attribute) _T("end"))) \
{ \
    pDefinition->Set##name##TemporalPriority(pDefinition->GetPriority()); \
} \
else \
{ \
    pDefinition->Set##name##TemporalPriority(0); \
}

// READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY
#define READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(name, attribute) \
if (!bMod || node.HasProperty(_T(#attribute)) || node.HasProperty(_T(#attribute) _T("start"))) \
    pDefinition->Set##name##Start(node.GetProperty(_T(#attribute) _T("start"), node.GetProperty(_T(#attribute)))); \
if (!bMod || node.HasProperty(_T(#attribute)) || node.HasProperty(_T(#attribute) _T("start")) || node.HasProperty(_T(#attribute) _T("end"))) \
    pDefinition->Set##name##End(node.GetProperty(_T(#attribute) _T("end"), node.GetProperty(_T(#attribute) _T("start"), node.GetProperty(_T(#attribute))))); \
if (node.HasProperty(_T(#attribute) _T("mid"))) \
    pDefinition->Set##name##Mid(node.GetProperty(_T(#attribute) _T("mid"))); \
else \
{ \
    tstring sValues; \
    for (uint ui(0); ui < pDefinition->Get##name##StartSize(); ++ui) \
    { \
        if (ui > 0) \
            sValues += _T(","); \
        sValues += XtoA(pDefinition->Get##name##Start(ui) + ((pDefinition->Get##name##End(ui) - pDefinition->Get##name##Start(ui)) / 2.0f)); \
    } \
    pDefinition->Set##name##Mid(sValues); \
} \
if (!bMod || node.HasProperty(_T(#attribute) _T("midpos"))) \
    pDefinition->Set##name##MidPos(node.GetProperty(_T(#attribute) _T("midpos"), _T("0.5"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("speed"))) \
    pDefinition->Set##name##Speed(node.GetProperty(_T(#attribute) _T("speed"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("delay"))) \
    pDefinition->Set##name##Delay(node.GetProperty(_T(#attribute) _T("delay"))); \
if (!bMod || node.HasProperty(_T(#attribute) _T("duration"))) \
    pDefinition->Set##name##Duration(node.GetProperty(_T(#attribute) _T("duration"))); \
if (!bMod || \
    node.HasProperty(_T(#attribute)) || \
    node.HasProperty(_T(#attribute) _T("start")) || \
    node.HasProperty(_T(#attribute) _T("mid")) || \
    node.HasProperty(_T(#attribute) _T("end"))) \
{ \
    pDefinition->Set##name##TemporalPriority(pDefinition->GetPriority()); \
} \
else \
{ \
    pDefinition->Set##name##TemporalPriority(0); \
}

// READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY
#define READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(name, attribute) \
READ_ENTITY_DEFINITION_PROPERTY(name, attribute) \
READ_ENTITY_DEFINITION_PROPERTY(name##PerLevel, attribute##perlevel) \
READ_ENTITY_DEFINITION_PROPERTY(name##PerCharge, attribute##percharge)

// READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY_EX
#define READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY_EX(name, attribute, def, defperlevel, defpercharge) \
READ_ENTITY_DEFINITION_PROPERTY_EX(name, attribute, def) \
READ_ENTITY_DEFINITION_PROPERTY_EX(name##PerLevel, attribute##perlevel, defperlevel) \
READ_ENTITY_DEFINITION_PROPERTY_EX(name##PerCharge, attribute##percharge, defpercharge)

// READ_SPLIT_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY
#define READ_SPLIT_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(base, A, B, attribute, a, b) \
READ_SPLIT_ENTITY_DEFINITION_PROPERTY(base, A, B, attribute, a, b) \
READ_SPLIT_ENTITY_DEFINITION_PROPERTY_SUFFIXED(base, A, B, PerLevel, attribute, a, b, perlevel) \
READ_SPLIT_ENTITY_DEFINITION_PROPERTY_SUFFIXED(base, A, B, PerCharge, attribute, a, b, percharge)

// READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROGRESSIVE_PROPERTY
#define READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROGRESSIVE_PROPERTY(name, attribute) \
READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(name, attribute) \
READ_ENTITY_DEFINITION_TEMPORAL_PROPERTY(name##PerLevel, attribute##perlevel) \
READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(name##PerCharge, attribute##percharge)

// ENTITY_DEF_MERGE_START
#define ENTITY_DEF_MERGE_START(def, base) \
void    def::ImportDefinition(IEntityDefinition *pOtherDefinition) \
{ \
    base::ImportDefinition(pOtherDefinition); \
    if (pOtherDefinition == NULL) \
        return; \
\
    if (GetAllocator()->GetBaseType() != pOtherDefinition->GetAllocator()->GetBaseType()) \
        return; \
\
    def *pDefinition(static_cast<def*>(pOtherDefinition));


// ENTITY_DEF_MERGE_END
#define ENTITY_DEF_MERGE_END \
}

// MERGE_PROPERTY
#define MERGE_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_##name = pDefinition->Get##name(); \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_STRING_PROPERTY
#define MERGE_STRING_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_s##name = pDefinition->Get##name(); \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_LOCALIZED_STRING_PROPERTY
#define MERGE_LOCALIZED_STRING_PROPERTY(name) \
if (pDefinition->GetDisplayNamePriority() > GetDisplayNamePriority()) \
{ \
    Set##name(pDefinition->Get##name##Index()); \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_ARRAY_PROPERTY
#define MERGE_ARRAY_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_v##name = pDefinition->m_v##name; \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_PROGRESSIVE_ARRAY_PROPERTY
#define MERGE_PROGRESSIVE_ARRAY_PROPERTY(name) \
MERGE_ARRAY_PROPERTY(name) \
MERGE_PROPERTY(name##PerLevel) \
MERGE_ARRAY_PROPERTY(name##PerCharge)

// MERGE_STRING_ARRAY_PROPERTY
#define MERGE_STRING_ARRAY_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_vs##name = pDefinition->m_vs##name; \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_STRING_VECTOR_ARRAY_PROPERTY
#define MERGE_STRING_VECTOR_ARRAY_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_vvs##name = pDefinition->m_vvs##name; \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_RESOURCE_PROPERTY
#define MERGE_RESOURCE_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_s##name = pDefinition->m_s##name; \
    m_h##name = pDefinition->m_h##name; \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_RESOURCE_ARRAY_PROPERTY
#define MERGE_RESOURCE_ARRAY_PROPERTY(name) \
if (pDefinition->Get##name##Priority() > Get##name##Priority()) \
{ \
    m_vs##name = pDefinition->m_vs##name; \
    m_v##name = pDefinition->m_v##name; \
    m_i##name##Priority = pDefinition->Get##name##Priority(); \
}

// MERGE_TEMPORAL_PROPERTY
#define MERGE_TEMPORAL_PROPERTY(name) \
if (pDefinition->Get##name##TemporalPriority() > Get##name##TemporalPriority()) \
{ \
    m_##name##Start = pDefinition->m_##name##Start; \
    m_##name##Mid = pDefinition->m_##name##Mid; \
    m_##name##End = pDefinition->m_##name##End; \
    m_f##name##MidPos = pDefinition->m_f##name##MidPos; \
    m_##name##Speed = pDefinition->m_##name##Speed; \
    m_ui##name##Delay = pDefinition->m_ui##name##Delay; \
    m_ui##name##Duration = pDefinition->m_ui##name##Duration; \
    m_i##name##TemporalPriority = pDefinition->Get##name##TemporalPriority(); \
}

// MERGE_TEMPORAL_ARRAY_PROPERTY
#define MERGE_TEMPORAL_ARRAY_PROPERTY(name) \
if (pDefinition->Get##name##TemporalPriority() > Get##name##TemporalPriority()) \
{ \
    m_v##name##Start = pDefinition->m_v##name##Start; \
    m_v##name##Mid = pDefinition->m_v##name##Mid; \
    m_v##name##End = pDefinition->m_v##name##End; \
    m_v##name##MidPos = pDefinition->m_v##name##MidPos; \
    m_v##name##Speed = pDefinition->m_v##name##Speed; \
    m_v##name##Delay = pDefinition->m_v##name##Delay; \
    m_v##name##Duration = pDefinition->m_v##name##Duration; \
    m_i##name##TemporalPriority = pDefinition->Get##name##TemporalPriority(); \
}

// MERGE_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY
#define MERGE_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY(name) \
MERGE_TEMPORAL_ARRAY_PROPERTY(name) \
MERGE_TEMPORAL_PROPERTY(name##PerLevel) \
MERGE_TEMPORAL_ARRAY_PROPERTY(name##PerCharge)

// MERGE_MUTATION
#define MERGE_MUTATION(name) \
MERGE_PROPERTY(Morph##name) \
MERGE_PROPERTY(Apply##name##Morph)

// MERGE_ARRAY_MUTATION
#define MERGE_ARRAY_MUTATION(name) \
MERGE_ARRAY_PROPERTY(Morph##name) \
MERGE_PROPERTY(Apply##name##Morph)

// MERGE_RESOURCE_MUTATION
#define MERGE_RESOURCE_ARRAY_MUTATION(name) \
MERGE_RESOURCE_ARRAY_PROPERTY(Morph##name) \
MERGE_PROPERTY(Apply##name##Morph)

// MERGE_STRING_ARRAY_MUTATION
#define MERGE_STRING_ARRAY_MUTATION(name) \
MERGE_STRING_ARRAY_PROPERTY(Morph##name) \
MERGE_PROPERTY(Apply##name##Morph)

// MERGE_STRING_VECTOR_ARRAY_MUTATION
#define MERGE_STRING_VECTOR_ARRAY_MUTATION(name) \
MERGE_STRING_VECTOR_ARRAY_PROPERTY(Morph##name) \
MERGE_PROPERTY(Apply##name##Morph)

// DECLARE_DEFINITION_TYPE_INFO
#define DECLARE_DEFINITION_TYPE_INFO \
private: \
    GAME_SHARED_API static tstring  s_sBaseTypeName; \
    GAME_SHARED_API static uint     s_uiBaseType; \
public: \
    static const tstring&   GetBaseTypeName()   { return s_sBaseTypeName; } \
    static uint             GetBaseType()       { return s_uiBaseType; }

// DEFINE_DEFINITION_TYPE_INFO
#define DEFINE_DEFINITION_TYPE_INFO(def, type, name) \
tstring def::s_sBaseTypeName(_T(#name)); \
uint    def::s_uiBaseType(type);

// REGISTER_WITH_ENTITY_PROCESSORS
#define REGISTER_WITH_ENTITY_PROCESSORS \
REGISTER_XML_PROCESSOR_EX(XMLHero, hero) \
REGISTER_XML_PROCESSOR_EX(XMLHero, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLBuilding, building) \
REGISTER_XML_PROCESSOR_EX(XMLBuilding, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLGadget, gadget) \
REGISTER_XML_PROCESSOR_EX(XMLGadget, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLPet, pet) \
REGISTER_XML_PROCESSOR_EX(XMLPet, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLCreep, creep) \
REGISTER_XML_PROCESSOR_EX(XMLCreep, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLNeutral, neutral) \
REGISTER_XML_PROCESSOR_EX(XMLNeutral, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLCritter, critter) \
REGISTER_XML_PROCESSOR_EX(XMLCritter, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLAffector, affector) \
REGISTER_XML_PROCESSOR_EX(XMLAffector, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLLinearAffector, linearaffector) \
REGISTER_XML_PROCESSOR_EX(XMLLinearAffector, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLState, state) \
REGISTER_XML_PROCESSOR_EX(XMLState, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLAbility, ability) \
REGISTER_XML_PROCESSOR_EX(XMLAbility, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLAbilityAttribute, abilityattribute) \
REGISTER_XML_PROCESSOR_EX(XMLAbilityAttribute, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLItem, item) \
REGISTER_XML_PROCESSOR_EX(XMLItem, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLProjectile, projectile) \
REGISTER_XML_PROCESSOR_EX(XMLProjectile, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLPowerup, powerup) \
REGISTER_XML_PROCESSOR_EX(XMLPowerup, modifier) \
REGISTER_XML_PROCESSOR_EX(XMLOrder, order) \
REGISTER_XML_PROCESSOR_EX(XMLOrder, modifier)\
REGISTER_XML_PROCESSOR_EX(XMLGameInfo, game) \

#define PRECACHE_GUARD \
if (!m_bPrecaching) \
{ \
    m_bPrecaching = true;

#define PRECACHE_GUARD_END \
    m_bPrecaching = false; \
}

struct SHeroPrecache
{
    tstring sName;
    EPrecacheScheme eScheme;

    SHeroPrecache() :
    eScheme(PRECACHE_ALL)
    {
    }
    
    SHeroPrecache(const tstring &_sName, EPrecacheScheme _eScheme) :
    sName(_sName),
    eScheme(_eScheme)
    {
    }

    bool operator==(const SHeroPrecache &b) const
    {
        return sName == b.sName && eScheme == b.eScheme;
    }
};

typedef deque<SHeroPrecache>    HeroPrecacheList;
//=============================================================================

//=============================================================================
// Post Headers
//=============================================================================
#include "c_auradefinition.h"
//=============================================================================

//=============================================================================
// IEntityDefinition
//=============================================================================
class IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

private:
    IEntityDefinition();

protected:
    ushort                          m_unTypeID;
    ushort                          m_unModifierBits;
    bool                            m_bHasBaseModifier;

    tstring                         m_sName;

    IBaseEntityAllocator*           m_pAllocator;
    vector<CCombatActionScript*>    m_vActionScripts;
    AuraList                        m_vAuras;
    
    ushort                          m_unModifierMask;
    map<ushort, IEntityDefinition*> m_mapModifiers;

    uint                            m_uiModifierCount;
    map<uint, ushort>               m_mapModifierIDs; // Maps unique modifier IDs to modifier bits

    // Modifier properties
    uint                            m_uiModifierID; // ID of this modifier
    tstring                         m_sCondition;
    int                             m_iPriority;
    bool                            m_bExclusive;

    bool                            m_bPrecaching;
    bool                            m_bPostProcessing;

public:
    virtual ~IEntityDefinition()    {}
    IEntityDefinition(IBaseEntityAllocator *pAllocator);

    virtual IEntityDefinition*  GetCopy() const = 0;

    void                    SetName(const tstring &sName);
    const tstring&          GetName() const                 { return m_sName; }
    void                    SetTypeID(ushort unTypeID);
    ushort                  GetTypeID() const               { return m_unTypeID; }

    IBaseEntityAllocator*   GetAllocator() const                                    { return m_pAllocator; }
    IBaseEntityAllocator*   GetAllocator(const tstring &sSubType) const;
    CCombatActionScript*    GetActionScript(EEntityActionScript eScript)            { return m_vActionScripts[eScript]; }
    CCombatActionScript*    NewActionScript(EEntityActionScript eScript, int iPriority, bool bPropagateToIllusions, bool bActivateOnBounces);
    float                   ExecuteActionScript(EEntityActionScript eScript, IGameEntity *pEntity, IGameEntity *pInitiator, IGameEntity *pInflictor, IGameEntity *pTarget, const CVec3f &v3Target, IGameEntity *pProxy, uint uiLevel, CCombatEvent *pCombatEvent = NULL, CDamageEvent *pDamageEvent = NULL, const CVec3f &v3Delta = V3_ZERO, float fDefault = 0.0f);

    void                    ApplyAuras(IGameEntity *pSource, uint uiLevel);
    const AuraList&         GetAuraList() const                             { return m_vAuras; }
    void    AddAura(
        const tstring &sStateName,
        const tstring &sGadgetName,
        const tstring &sRadius,
        const tstring &sDuration,
        const tstring &sTargetScheme,
        const tstring &sEffectType,
        const tstring &sIgnoreInvulnerable,
        const tstring &sCondition,
        const tstring &sReflexiveState,
        const tstring &sPropagateCondition,
        const tstring &sStack,
        bool bNoTooltip)
    {
        m_vAuras.push_back(CAuraDefinition(sStateName, sGadgetName, sRadius, sDuration, sTargetScheme, sEffectType, sIgnoreInvulnerable, sCondition, sReflexiveState, sPropagateCondition, sStack, bNoTooltip)); 
    }

    virtual void            Precache(EPrecacheScheme eScheme);
    virtual void            GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache);
    virtual void            PostProcess();

    void                    AddModifier(const tstring &sModifierKey, IEntityDefinition *pDef);
    ushort                  RegisterModifier(uint uiModifierID);
    GAME_SHARED_API ushort  GetModifierBit(uint uiModifierID);

    void                    SetModifierID(uint uiModifierID)                { m_uiModifierID = uiModifierID; }
    uint                    GetModifierID() const                           { return m_uiModifierID; }

    void                    SetCondition(const tstring &sCondition)         { m_sCondition = sCondition; }
    const tstring&          GetCondition() const                            { return m_sCondition; }

    void                    SetPriority(int iPriority)                      { m_iPriority = iPriority; }
    int                     GetPriority() const                             { return m_iPriority; }

    void                    SetExclusive(bool bExclusive)                   { m_bExclusive = bExclusive; }
    bool                    GetExclusive() const                            { return m_bExclusive; }

    const map<ushort, IEntityDefinition*>&  GetModifiers() const            { return m_mapModifiers; }
    const map<uint, ushort>&                GetModifierIDMap() const        { return m_mapModifierIDs; }

    GAME_SHARED_API ushort  GetModifierBits(const uivector &vModifiers);

    virtual void        ImportDefinition(IEntityDefinition *pDefinition);
    IEntityDefinition*  GenerateMergedModifier(ushort unModifierBits);

    GAME_SHARED_API CCombatActionScript*    GetActionScript(EEntityActionScript eScript, ushort unModifierBits);

    GAME_SHARED_API IEntityDefinition*  GetModifiedDefinition(ushort unModifierBits);
    GAME_SHARED_API IEntityDefinition*  GetModifiedDefinition(const uivector &vModifiers);
    GAME_SHARED_API const tstring&      GetEffectDescription(EEntityActionScript eScript);
    GAME_SHARED_API const tstring&      GetEffectDescription(EEntityActionScript eScript, ushort unModifierBits);
    GAME_SHARED_API uint                GetEffectDescriptionIndex(EEntityActionScript eScript);
    GAME_SHARED_API int                 GetActionScriptPriority(EEntityActionScript eScript);
    GAME_SHARED_API int                 GetActionScriptPriority(EEntityActionScript eScript, ushort unModifierBits);
};
//=============================================================================

#endif //__I_ENTITYDEFINITION_H__
