// (C)2006 S2 Games
// c_statecommanderheal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommanderheal.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderHeal);
//=============================================================================


/*====================
  CStateCommanderHeal::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderHeal::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthPerSecond, 0.0f)
{
}


/*====================
  CStateCommanderHeal::CStateCommanderHeal
  ====================*/
CStateCommanderHeal::CStateCommanderHeal() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateCommanderHeal::StateFrame
  ====================*/
void	CStateCommanderHeal::StateFrame()
{
	IEntityState::StateFrame();

	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	pOwner->Heal(m_pEntityConfig->GetHealthPerSecond() * MsToSec(Game.GetFrameLength()), Game.GetVisualEntity(m_uiInflictorIndex));
}
