// (C)2006 S2 Games
// c_stateinterrupted.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateinterrupted.h"

#include "../k2/c_camera.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Interrupted);
//=============================================================================

/*====================
  CStateInterrupted::CEntityConfig::CEntityConfig
  ====================*/
CStateInterrupted::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateInterrupted::~CStateInterrupted
  ====================*/
CStateInterrupted::~CStateInterrupted()
{
}


/*====================
  CStateInterrupted::CStateInterrupted
  ====================*/
CStateInterrupted::CStateInterrupted() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}
