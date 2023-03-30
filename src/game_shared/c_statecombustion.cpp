// (C)2006 S2 Games
// c_statecombustion.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecombustion.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Combustion);
//=============================================================================


/*====================
  CStateCombustion::CEntityConfig::CEntityConfig
  ====================*/
CStateCombustion::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateCombustion::CStateCombustion
  ====================*/
CStateCombustion::CStateCombustion() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateCombustion::StateFrame
  ====================*/
void	CStateCombustion::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

	return;
}
