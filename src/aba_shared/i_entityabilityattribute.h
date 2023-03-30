// (C)2008 S2 Games
// i_entityabiliyattribute.h
//
//=============================================================================
#ifndef __I_ENTITYABILITYATTRIBUTE_H__
#define __I_ENTITYABILITYATTRIBUTE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entityability.h"
#include "c_abilityattributedefinition.h"
//=============================================================================

//=============================================================================
// IEntityAbilityAttribute
//=============================================================================
class IEntityAbilityAttribute : public IEntityAbility
{
    DECLARE_ENTITY_DESC

public:
    typedef CAbilityDefinition TDefinition;

protected:
    float           m_fStrength;
    float           m_fAgility;
    float           m_fIntelligence;

public:
    virtual ~IEntityAbilityAttribute()  {}
    IEntityAbilityAttribute();

    SUB_ENTITY_ACCESSOR(IEntityAbilityAttribute, AbilityAttribute)

    // Network
    GAME_SHARED_API virtual void    Baseline();
    GAME_SHARED_API virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    GAME_SHARED_API virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual void    Spawn();

    virtual float   GetStrength() const         { return (IsActive() ? floor(m_fStrength) : 0.0f) + IEntityAbility::GetStrength(); }
    virtual float   GetAgility() const          { return (IsActive() ? floor(m_fAgility) : 0.0f) + IEntityAbility::GetAgility(); }
    virtual float   GetIntelligence() const     { return (IsActive() ? floor(m_fIntelligence) : 0.0f) + IEntityAbility::GetIntelligence(); }

    void            AdjustStrength(float fValue)        { m_fStrength += fValue; }
    void            AdjustAgility(float fValue)         { m_fAgility += fValue; }
    void            AdjustIntelligence(float fValue)    { m_fIntelligence += fValue; }
};
//=============================================================================

#endif //__I_ENTITYABILITYATTRIBUTE_H__
