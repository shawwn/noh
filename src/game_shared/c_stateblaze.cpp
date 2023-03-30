// (C)2007 S2 Games
// c_stateblaze.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateblaze.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Blaze);
//=============================================================================


/*====================
  CStateBlaze::CEntityConfig::CEntityConfig
  ====================*/
CStateBlaze::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateBlaze::CStateBlaze
  ====================*/
CStateBlaze::CStateBlaze() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateBlaze::StateFrame
  ====================*/
void	CStateBlaze::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

	return;
}
