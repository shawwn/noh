// (C)2007 S2 Games
// c_statecommanderspeedboost.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERSPEEDBOOST_H__
#define __C_STATECOMMANDERSPEEDBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderSpeedBoost
//=============================================================================
class CStateCommanderSpeedBoost : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedAdd)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, CommanderSpeedBoost);

public:
    ~CStateCommanderSpeedBoost()    {}
    CStateCommanderSpeedBoost();
};
//=============================================================================

#endif //__C_STATECOMMANDERSPEEDBOOST_H__
