// (C)2007 S2 Games
// c_statecommanderrecall.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERRECALL_H__
#define __C_STATECOMMANDERRECALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderRecall
//=============================================================================
class CStateCommanderRecall : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(uint, ActivationDelay)
        DECLARE_ENTITY_CVAR(tstring, ActivationEffect)
        DECLARE_ENTITY_CVAR(tstring, TerminationEffect)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, CommanderRecall)

    uint            m_uiActivationTime;

public:
    ~CStateCommanderRecall()    {}
    CStateCommanderRecall() :
    IEntityState(GetEntityConfig()),
    m_pEntityConfig(GetEntityConfig()),
    m_uiActivationTime(0)
    {}

    void    StateFrame();
    bool    Use(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_STATECOMMANDERRECALL_H__
