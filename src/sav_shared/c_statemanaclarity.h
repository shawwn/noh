// (C)2006 S2 Games
// c_statemanaclarity.h
//
//=============================================================================
#ifndef __C_STATEMANACLARITY_H__
#define __C_STATEMANACLARITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateManaClarity
//=============================================================================
class CStateManaClarity : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ManaRegenBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, ManaClarity);

public:
    ~CStateManaClarity()    {}
    CStateManaClarity();
};
//=============================================================================

#endif //__C_STATEMANACLARITY_H__
