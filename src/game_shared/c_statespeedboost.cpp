// (C)2006 S2 Games
// c_statespeedboost.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statespeedboost.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, SpeedBoost);
//=============================================================================


/*====================
  CStateSpeedBoost::CEntityConfig::CEntityConfig
  ====================*/
CStateSpeedBoost::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedBoost, 0.30f)
{
}


/*====================
  CStateSpeedBoost::CStateSpeedBoost
  ====================*/
CStateSpeedBoost::CStateSpeedBoost() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(1.0f + m_pEntityConfig->GetSpeedBoost());
}
