// (C)2006 S2 Games
// c_statecriticalstrike.h
//
//=============================================================================
#ifndef __C_STATECRITICALSTRIKE_H__
#define __C_STATECRITICALSTRIKE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCriticalStrike
//=============================================================================
class CStateCriticalStrike : public IEntityState
{
private:
    DECLARE_STATE_ALLOCATOR(CriticalStrike);
    DECLARE_ENTITY_STATE_CVARS;
    static CCvarui  s_cvarAttackTime;
    static CCvarf   s_cvarAttackDamage;
    static CCvarui  s_cvarBleedDuration;

public:
    ~CStateCriticalStrike();
    CStateCriticalStrike();

    void    DoAttack(CMeleeAttackEvent &attack);
};
//=============================================================================

#endif //__C_STATECRITICALSTRIKE_H__
