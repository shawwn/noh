// (C)2006 S2 Games
// c_statehealthshrine.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statehealthshrine.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, HealthShrine);
//=============================================================================


/*====================
  CStateHealthShrine::CEntityConfig::CEntityConfig
  ====================*/
CStateHealthShrine::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenBoost, 15.0f)
{
}


/*====================
  CStateHealthShrine::CStateHealthShrine
  ====================*/
CStateHealthShrine::CStateHealthShrine() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modHealthRegen.SetAdd(m_pEntityConfig->GetHealthRegenBoost());
}
