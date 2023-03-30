// (C)2006 S2 Games
// c_statecommandercripple.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommandercripple.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderCripple);
//=============================================================================


/*====================
  CStateCommanderCripple::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderCripple::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedAdd, 0.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateCommanderCripple::CStateCommanderCripple
  ====================*/
CStateCommanderCripple::CStateCommanderCripple() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.Set(m_pEntityConfig->GetSpeedAdd(), m_pEntityConfig->GetSpeedMult(), 0.0f);
}
