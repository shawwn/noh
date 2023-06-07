// (C)2006 S2 Games
// c_stategroupheal.h
//
//=============================================================================
#ifndef __C_STATEGROUPHEAL_H__
#define __C_STATEGROUPHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateGroupHeal
//=============================================================================
class CStateGroupHeal : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthPerSecond)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, GroupHeal);

    float   m_fTotalHealed;

public:
    ~CStateGroupHeal()  {}
    CStateGroupHeal();

    void    StateFrame();
    void    Expired();
};
//=============================================================================

#endif //__C_STATEGROUPHEAL_H__
