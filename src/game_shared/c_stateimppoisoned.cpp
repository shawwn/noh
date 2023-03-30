// (C)2006 S2 Games
// c_stateimppoisoned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateimppoisoned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ImpPoisoned);
//=============================================================================


/*====================
  CStateImpPoisoned::CEntityConfig::CEntityConfig
  ====================*/
CStateImpPoisoned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 30.0f)
{
}



/*====================
  CStateImpPoisoned::CStateImpPoisoned
  ====================*/
CStateImpPoisoned::CStateImpPoisoned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateImpPoisoned::StateFrame
  ====================*/
void	CStateImpPoisoned::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiOwnerIndex));
	if (pPlayer != NULL)
		pPlayer->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

	return;
}
