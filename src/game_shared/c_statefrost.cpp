// (C)2007 S2 Games
// c_statefrost.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statefrost.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Frost)
//=============================================================================


/*====================
  CStateFrost::CEntityConfig::CEntityConfig
  ====================*/
CStateFrost::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(FrostSlow, 0.50f)
{
}


/*====================
  CStateFrost::CStateFrost
  ====================*/
CStateFrost::CStateFrost() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(m_pEntityConfig->GetFrostSlow());
}
