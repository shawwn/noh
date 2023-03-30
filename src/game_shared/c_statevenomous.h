// (C)2007 S2 Games
// c_statevenomous.h
//
//=============================================================================
#ifndef __C_STATEVENOMOUS_H__
#define __C_STATEVENOMOUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateVenomous
//=============================================================================
class CStateVenomous : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Venomous)

public:
    ~CStateVenomous()   {}
    CStateVenomous();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEVENOMOUS_H__
