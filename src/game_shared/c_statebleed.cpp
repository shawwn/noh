// (C)2006 S2 Games
// c_statebleed.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebleed.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Bleed);
//=============================================================================


/*====================
  CStateBleed::CEntityConfig::CEntityConfig
  ====================*/
CStateBleed::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 30.0f),
INIT_ENTITY_CVAR(SpeedAdd, 0.0f),
INIT_ENTITY_CVAR(SpeedMult, 0.0f)
{
}


/*====================
  CStateBleed::CStateBleed
  ====================*/
CStateBleed::CStateBleed() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.Set(m_pEntityConfig->GetSpeedAdd(), m_pEntityConfig->GetSpeedMult(), 0.0f);
}


/*====================
  CStateBleed::StateFrame
  ====================*/
void	CStateBleed::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

	return;
}
