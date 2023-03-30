// (C)2007 S2 Games
// c_statecommanderheal.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERHEAL_H__
#define __C_STATECOMMANDERHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderHeal
//=============================================================================
class CStateCommanderHeal : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthPerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, CommanderHeal);

public:
    ~CStateCommanderHeal()  {}
    CStateCommanderHeal();
    
    void    StateFrame();
};
//=============================================================================

#endif //__C_STATECOMMANDERHEAL_H__
