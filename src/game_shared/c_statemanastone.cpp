// (C)2006 S2 Games
// c_statemanastone.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemanastone.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ManaStone);
//=============================================================================


/*====================
  CStateManaStone::CEntityConfig::CEntityConfig
  ====================*/
CStateManaStone::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaCostMultiplier, 0.75f)
{
}


/*====================
  CStateManaStone::CStateManaStone
  ====================*/
CStateManaStone::CStateManaStone() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modGunManaCost.SetMult(m_pEntityConfig->GetManaCostMultiplier());
}
