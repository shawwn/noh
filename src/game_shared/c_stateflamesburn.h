// (C)2007 S2 Games
// c_stateflamesburn.h
//
//=============================================================================
#ifndef __C_STATEFLAMESBURN_H__
#define __C_STATEFLAMESBURN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateFlamesBurn
//=============================================================================
class CStateFlamesBurn : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, FlamesBurn);

public:
    ~CStateFlamesBurn() {}
    CStateFlamesBurn();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEFLAMESBURN_H__
