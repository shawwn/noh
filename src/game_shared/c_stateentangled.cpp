// (C)2006 S2 Games
// c_stateentangled.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateentangled.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Entangled);
//=============================================================================


/*====================
  CStateEntangled::CEntityConfig::CEntityConfig
  ====================*/
CStateEntangled::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateEntangled::CStateEntangled
  ====================*/
CStateEntangled::CStateEntangled() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(0.0f);
}
