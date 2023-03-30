// (C)2006 S2 Games
// c_statesteamboost.h
//
//=============================================================================
#ifndef __C_STATESTEAMBOOST_H__
#define __C_STATESTEAMBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateSteamBoost
//=============================================================================
class CStateSteamBoost : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, SteamBoost);

public:
    ~CStateSteamBoost() {}
    CStateSteamBoost();
};
//=============================================================================

#endif //__C_STATESTEAMBOOST_H__
