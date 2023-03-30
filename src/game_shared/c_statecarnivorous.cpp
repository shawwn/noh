// (C)2007 S2 Games
// c_statecarnivorous.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecarnivorous.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Carnivorous)
//=============================================================================


/*====================
  CStateCarnivorous::CEntityConfig::CEntityConfig
  ====================*/
CStateCarnivorous::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(LeachAmount, 0.5f)
{
}


/*====================
  CStateCarnivorous::CStateCarnivorous
  ====================*/
CStateCarnivorous::CStateCarnivorous() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateCarnivorous::DoAttack
  ====================*/
void	CStateCarnivorous::DoAttack(CMeleeAttackEvent &attack)
{
	attack.SetHealthLeach(m_pEntityConfig->GetLeachAmount());
}
