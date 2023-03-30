// (C)2006 S2 Games
// c_statespeedboost.h
//
//=============================================================================
#ifndef __C_STATESPEEDBOOST_H__
#define __C_STATESPEEDBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateSpeedBoost
//=============================================================================
class CStateSpeedBoost : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedBoost)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, SpeedBoost);

public:
    ~CStateSpeedBoost() {}
    CStateSpeedBoost();
};
//=============================================================================

#endif //__C_STATESPEEDBOOST_H__
