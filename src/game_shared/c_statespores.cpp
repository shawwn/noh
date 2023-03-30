// (C)2007 S2 Games
// c_statespores.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statespores.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Spores)
//=============================================================================


/*====================
  CStateSpores::CEntityConfig::CEntityConfig
  ====================*/
CStateSpores::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 20.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateSpores::CStateSpores
  ====================*/
CStateSpores::CStateSpores() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}


/*====================
  CStateSpores::StateFrame
  ====================*/
void	CStateSpores::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

	return;
}
