// (C)2006 S2 Games
// c_stateheal.h
//
//=============================================================================
#ifndef __C_STATEHEAL_H__
#define __C_STATEHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateHeal
//=============================================================================
class CStateHeal : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthPerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Heal);

    float   m_fTotalHealed;

public:
    ~CStateHeal()   {}
    CStateHeal();

    void    StateFrame();
    void    Expired();
};
//=============================================================================

#endif //__C_STATEHEAL_H__
