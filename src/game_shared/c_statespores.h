// (C)2007 S2 Games
// c_statespores.h
//
//=============================================================================
#ifndef __C_STATESPORES_H__
#define __C_STATESPORES_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateSpores
//=============================================================================
class CStateSpores : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, DamagePerSecond)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Spores)

public:
    ~CStateSpores() {}
    CStateSpores();

    void    StateFrame();
};
//=============================================================================

#endif //__C_STATESPORES_H__
