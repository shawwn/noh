// (C)2009 S2 Games
// c_auradefinition.h
//
//=============================================================================
#ifndef __C_AURADEFINITION_H__
#define __C_AURADEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gamemechanics.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CAuraDefinition;

enum EPrecacheScheme;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<CAuraDefinition>     AuraList;
typedef AuraList::iterator          AuraList_it;
typedef AuraList::const_iterator    AuraList_cit;
//=============================================================================

//=============================================================================
// CAuraDefinition
//=============================================================================
class CAuraDefinition
{
private:
    bool            m_bPrecaching;

    tsvector        m_vStateName;
    tsvector        m_vGadgetName;
    fvector         m_vRadius;
    uivector        m_vDuration;
    uivector        m_vTargetScheme;
    uivector        m_vEffectType;
    vector<bool>    m_vIgnoreInvulnerable;
    tsvector        m_vCondition;
    tsvector        m_vReflexiveStateName;
    tsvector        m_vPropagateCondition;
    vector<bool>    m_vStack;
    bool            m_bNoTooltip;

    // Working values
    bool            m_bValid;
    ushort          m_unStateID;
    ushort          m_unGadgetID;
    uint            m_uiDuration;
    uint            m_uiTargetScheme;
    uint            m_uiEffectType;
    bool            m_bIgnoreInvulnerable;
    IUnitEntity*    m_pGadgetOwner;
    tstring         m_sCondition;
    ushort          m_unReflexiveStateID;
    tstring         m_sPropagateCondition;
    bool            m_bStack;

    CAuraDefinition();

public:
    ~CAuraDefinition()  {}
    CAuraDefinition(
        const tstring &sStateName,
        const tstring &sGadgetName,
        const tstring &sRadius,
        const tstring &sDuration,
        const tstring &sTargetScheme,
        const tstring &sEffectType,
        const tstring &sIgnoreInvulnerable,
        const tstring &sCondition,
        const tstring &sReflexiveStateName,
        const tstring &sPropogateCondition,
        const tstring &sStack,
        bool bNoTooltip);

    const CAuraDefinition&  operator=(const CAuraDefinition &aura)
    {
        m_vStateName = aura.m_vStateName;
        m_vGadgetName = aura.m_vGadgetName;
        m_vRadius = aura.m_vRadius;
        m_vDuration = aura.m_vDuration;
        m_vTargetScheme = aura.m_vTargetScheme;
        m_vEffectType = aura.m_vEffectType;
        m_vIgnoreInvulnerable = aura.m_vIgnoreInvulnerable;
        m_vCondition = aura.m_vCondition;
        m_vReflexiveStateName = aura.m_vReflexiveStateName;
        m_vPropagateCondition = aura.m_vPropagateCondition;
        m_vStack = aura.m_vStack;
        m_bNoTooltip = aura.m_bNoTooltip;

        return *this;
    }

    bool    IsValid() const                 { return m_bValid; }
    
    bool    CanApply(IUnitEntity *pSource, IUnitEntity *pTarget) const;
    bool    CanPropagate(IUnitEntity *pSource) const;
    bool    ApplyState(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const;
    bool    BindGadget(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const;
    bool    ApplyReflexiveState(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const;

    void    FetchWorkingValues(IGameEntity *pSource, uint uiLevel);

    bool    GetNoTooltip() const            { return m_bNoTooltip; }

#define AURA_ATTRIBUTE_ACCESSOR(name, type) \
type    Get##name(uint uiLevel) const   { if (m_v##name.empty()) return GetDefaultEmptyValue<type>(); return m_v##name[MIN(uiLevel > 0 ? uiLevel - 1 : 0, INT_SIZE(m_v##name.size()) - 1)]; }

    AURA_ATTRIBUTE_ACCESSOR(StateName, const tstring&)
    AURA_ATTRIBUTE_ACCESSOR(GadgetName, const tstring&)
    AURA_ATTRIBUTE_ACCESSOR(Radius, float)
    AURA_ATTRIBUTE_ACCESSOR(Duration, uint)
    AURA_ATTRIBUTE_ACCESSOR(TargetScheme, uint)
    AURA_ATTRIBUTE_ACCESSOR(EffectType, uint)
    AURA_ATTRIBUTE_ACCESSOR(IgnoreInvulnerable, bool)
    AURA_ATTRIBUTE_ACCESSOR(Condition, const tstring&)
    AURA_ATTRIBUTE_ACCESSOR(ReflexiveStateName, const tstring&)
    AURA_ATTRIBUTE_ACCESSOR(PropagateCondition, const tstring&)
    AURA_ATTRIBUTE_ACCESSOR(Stack, bool)
    
    virtual void    Precache(EPrecacheScheme eScheme, const tstring &sModifier);
    virtual void    GetPrecacheList(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache);
};
//=============================================================================

#endif //__C_AURADEFINITION_H__
