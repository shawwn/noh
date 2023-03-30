// (C)2007 S2 Games
// c_stateblind.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateblind.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Blind)
//=============================================================================


/*====================
  CStateBlind::CEntityConfig::CEntityConfig
  ====================*/
CStateBlind::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}



/*====================
  CStateBlind::CStateBlind
  ====================*/
CStateBlind::CStateBlind() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetMult(m_pEntityConfig->GetArmorMult());
	m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}
