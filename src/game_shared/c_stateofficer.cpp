// (C)2007 S2 Games
// c_stateofficer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateofficer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Officer);
//=============================================================================


/*====================
  CStateOfficer::CEntityConfig::CEntityConfig
  ====================*/
CStateOfficer::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenMult, 1.75f),
INIT_ENTITY_CVAR(ManaRegenMult, 1.75f),
INIT_ENTITY_CVAR(StaminaRegenMult, 1.75f)
{
}


/*====================
  CStateOfficer::CStateOfficer
  ====================*/
CStateOfficer::CStateOfficer() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modHealthRegen.SetMult(m_pEntityConfig->GetHealthRegenMult());
	m_modManaRegen.SetMult(m_pEntityConfig->GetManaRegenMult());
	m_modStaminaRegen.SetMult(m_pEntityConfig->GetStaminaRegenMult());
}
