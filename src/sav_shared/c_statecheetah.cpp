// (C)2007 S2 Games
// c_statecheetah.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecheetah.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Cheetah)
//=============================================================================


/*====================
  CStateCheetah::CEntityConfig::CEntityConfig
  ====================*/
CStateCheetah::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateCheetah::CStateCheetah
  ====================*/
CStateCheetah::CStateCheetah() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}
