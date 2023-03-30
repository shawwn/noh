// (C)2007 S2 Games
// c_stateethereal.h
//
//=============================================================================
#ifndef __C_STATEETHEREAL_H__
#define __C_STATEETHEREAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CSateEthereal
//=============================================================================
class CStateEthereal : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Ethereal);

public:
    ~CStateEthereal()   {}
    CStateEthereal() :
    m_pEntityConfig(GetEntityConfig()),
    IEntityState(GetEntityConfig())
    {
        m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
    }

    float   OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker)   { return 0.0f; }
};
//=============================================================================

#endif //__C_STATEETHEREAL_H__
