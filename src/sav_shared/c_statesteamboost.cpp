// (C)2006 S2 Games
// c_statesteamboost.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statesteamboost.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, SteamBoost);
//=============================================================================


/*====================
  CStateSteamBoost::CEntityConfig::CEntityConfig
  ====================*/
CStateSteamBoost::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedMult, 1.50f)
{
}


/*====================
  CStateSteamBoost::CStateSteamBoost
  ====================*/
CStateSteamBoost::CStateSteamBoost() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}
