// (C)2006 S2 Games
// c_statebleed.h
//
//=============================================================================
#ifndef __C_STATEBLEED_H__
#define __C_STATEBLEED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBleed
//=============================================================================
class CStateBleed : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
        DECLARE_ENTITY_CVAR(float, SpeedAdd)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Bleed);

public:
    ~CStateBleed()  {}
    CStateBleed();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATEBLEED_H__
