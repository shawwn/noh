// (C)2006 S2 Games
// c_statedecay.h
//
//=============================================================================
#ifndef __C_STATEDECAY_H__
#define __C_STATEDECAY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateDecay
//=============================================================================
class CStateDecay : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Decay);

public:
    ~CStateDecay()  {}
    CStateDecay();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEDECAY_H__
