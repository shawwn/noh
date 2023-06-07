// (C)2006 S2 Games
// c_statemorale.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemorale.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Morale);
//=============================================================================


/*====================
  CStateMorale::CEntityConfig::CEntityConfig
  ====================*/
CStateMorale::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenMult, 1.10f),
INIT_ENTITY_CVAR(ManaRegenMult, 1.10f),
INIT_ENTITY_CVAR(StaminaRegenMult, 1.10f)
{
}


/*====================
  CStateMorale::CStateMorale
  ====================*/
CStateMorale::CStateMorale() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modHealthRegen.SetMult(m_pEntityConfig->GetHealthRegenMult());
    m_modManaRegen.SetMult(m_pEntityConfig->GetManaRegenMult());
    m_modStaminaRegen.SetMult(m_pEntityConfig->GetStaminaRegenMult());
}
