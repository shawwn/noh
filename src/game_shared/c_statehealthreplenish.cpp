// (C)2006 S2 Games
// c_statehealthreplenish.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statehealthreplenish.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, HealthReplenish);
//=============================================================================


/*====================
  CStateHealthReplenish::CEntityConfig::CEntityConfig
  ====================*/
CStateHealthReplenish::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenBoost, 10.0f)
{
}


/*====================
  CStateHealthReplenish::CStateHealthReplenish
  ====================*/
CStateHealthReplenish::CStateHealthReplenish() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modHealthRegen.SetAdd(m_pEntityConfig->GetHealthRegenBoost());
}
