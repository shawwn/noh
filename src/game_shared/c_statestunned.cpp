// (C)2006 S2 Games
// c_statestunned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statestunned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Stunned);
//=============================================================================


/*====================
  CStateStunned::CEntityConfig::CEntityConfig
  ====================*/
CStateStunned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateStunned::CStateStunned
  ====================*/
CStateStunned::CStateStunned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateStunned::Activated
  ====================*/
void	CStateStunned::Activated()
{
	IEntityState::Activated();
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL && pOwner->IsCombat())
		pOwner->GetAsCombatEnt()->Stun(GetExpireTime());
}
