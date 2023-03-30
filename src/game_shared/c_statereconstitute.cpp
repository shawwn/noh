// (C)2006 S2 Games
// c_statereconstitute.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statereconstitute.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Reconstitute);
//=============================================================================


/*====================
  CStateReconstitute::CEntityConfig::CEntityConfig
  ====================*/
CStateReconstitute::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenMult, 1.50)
{
}


/*====================
  CStateReconstitute::CStateReconstitute
  ====================*/
CStateReconstitute::CStateReconstitute() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modHealthRegen.SetMult(m_pEntityConfig->GetHealthRegenMult());
}
