// (C)2008 S2 Games
// i_temporalstate.h
//
//=============================================================================
#ifndef __I_TEMPORALSTATE_H__
#define __I_TEMPORALSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define DECLARE_TEMPORAL_CVAR(type, name) \
DECLARE_ENTITY_CVAR(type, name##Start) \
DECLARE_ENTITY_CVAR(type, name##Mid) \
DECLARE_ENTITY_CVAR(type, name##End) \
DECLARE_ENTITY_CVAR(float, name##MidPos)

#define INIT_TEMPORAL_CVAR(name, def) \
INIT_ENTITY_CVAR(name##Start, def), \
INIT_ENTITY_CVAR(name##Mid, def), \
INIT_ENTITY_CVAR(name##End, def), \
INIT_ENTITY_CVAR(name##MidPos, 0.5f)

#define TEMPORAL_CVAR_ACCESSOR(type, name) \
virtual type    Get##name() const \
{ \
    if (m_pEntityConfig == nullptr) \
        return GetDefaultEmptyValue<type>(); \
\
    uint uiGameTime(Game.GetGameTime()); \
    float fLerp(CLAMP(float(uiGameTime - m_uiCreationTime) / m_uiLifetime, 0.0f, 1.0f)); \
    float fMidPos(m_pEntityConfig->Get##name##MidPos().GetValue()); \
\
    if (fLerp < fMidPos) \
        return LERP(fLerp / fMidPos, \
            m_pEntityConfig->Get##name##Start().GetValue(), \
            m_pEntityConfig->Get##name##Mid().GetValue()) + IEntityState::Get##name(); \
    else \
        return LERP((fLerp - fMidPos) / (1.0f - fMidPos), \
            m_pEntityConfig->Get##name##Mid().GetValue(), \
            m_pEntityConfig->Get##name##End().GetValue()) + IEntityState::Get##name(); \
}
//=============================================================================

//=============================================================================
// ITemporalState
//=============================================================================
class ITemporalState : public IEntityState
{
private:
    static vector<SDataField>*  s_pvFields;

    ITemporalState();

protected:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_TEMPORAL_CVAR(float, MoveSpeed)
        DECLARE_TEMPORAL_CVAR(float, Armor)
        DECLARE_TEMPORAL_CVAR(float, MagicArmor)
        DECLARE_TEMPORAL_CVAR(float, MaxHealth)
        DECLARE_TEMPORAL_CVAR(float, MaxMana)
        DECLARE_TEMPORAL_CVAR(float, HealthRegen)
        DECLARE_TEMPORAL_CVAR(float, ManaRegen)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

public:
    ~ITemporalState();
    ITemporalState(CEntityConfig *pConfig);

    SUB_ENTITY_ACCESSOR(ITemporalState, TemporalState)

    TEMPORAL_CVAR_ACCESSOR(float, MoveSpeed)
    TEMPORAL_CVAR_ACCESSOR(float, Armor)
    TEMPORAL_CVAR_ACCESSOR(float, MagicArmor)
    TEMPORAL_CVAR_ACCESSOR(float, MaxHealth)
    TEMPORAL_CVAR_ACCESSOR(float, MaxMana)
    TEMPORAL_CVAR_ACCESSOR(float, HealthRegen)
    TEMPORAL_CVAR_ACCESSOR(float, ManaRegen)
};
//=============================================================================

#endif //__I_TEMPORALSTATE_H__