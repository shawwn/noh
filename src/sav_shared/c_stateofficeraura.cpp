// (C)2007 S2 Games
// c_stateofficeraura.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateofficeraura.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, OfficerAura);
//=============================================================================


/*====================
  CStateOfficerAura::CEntityConfig::CEntityConfig
  ====================*/
CStateOfficerAura::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenMult, 1.50f),
INIT_ENTITY_CVAR(ManaRegenMult, 1.50f),
INIT_ENTITY_CVAR(StaminaRegenMult, 1.50f)
{
}


/*====================
  CStateOfficerAura::CStateOfficerAura
  ====================*/
CStateOfficerAura::CStateOfficerAura() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modHealthRegen.SetMult(m_pEntityConfig->GetHealthRegenMult());
    m_modManaRegen.SetMult(m_pEntityConfig->GetManaRegenMult());
    m_modStaminaRegen.SetMult(m_pEntityConfig->GetStaminaRegenMult());
}
