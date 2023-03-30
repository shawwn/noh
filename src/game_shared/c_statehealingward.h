// (C)2007 S2 Games
// c_satehealingward.h
//
//=============================================================================
#ifndef __C_STATEHEALINGWARD_H__
#define __C_STATEHEALINGWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateHealingWard
//=============================================================================
class CStateHealingWard : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, HealthRegenBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    float   m_fTotalHealed;

    DECLARE_ENT_ALLOCATOR2(State, HealingWard);

public:
    ~CStateHealingWard()    {}
    CStateHealingWard();

    void    StateFrame();
    void    Expired();
};
//=============================================================================

#endif //__C_STATEHEALINGWARD_H__
