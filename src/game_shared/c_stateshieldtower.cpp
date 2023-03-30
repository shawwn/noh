// (C)2007 S2 Games
// c_stateshieldtower.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateshieldtower.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ShieldTower);
//=============================================================================


/*====================
  CStateShieldTower::CEntityConfig::CEntityConfig
  ====================*/
CStateShieldTower::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorBuff, 1.0f),
INIT_ENTITY_CVAR(DamageMultiplier, .01f)
{
}


/*====================
  CStateShieldTower::CStateShieldTower
  ====================*/
CStateShieldTower::CStateShieldTower() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetAdd(m_pEntityConfig->GetArmorBuff());
}
