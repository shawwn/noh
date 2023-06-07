// (C)2007 S2 Games
// c_satemanaward.h
//
//=============================================================================
#ifndef __C_STATEMANAWARD_H__
#define __C_STATEMANAWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateManaWard
//=============================================================================
class CStateManaWard : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, ManaRegenBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, ManaWard);

    float   m_fTotalRecovered;

public:
    ~CStateManaWard()   {}
    CStateManaWard();

    void    StateFrame();
    void    Expired();
};
//=============================================================================

#endif //__C_STATEMANAWARD_H__
