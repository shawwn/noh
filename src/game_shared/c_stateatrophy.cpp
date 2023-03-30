// (C)2006 S2 Games
// c_stateatrophy.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateatrophy.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Atrophy);
//=============================================================================


/*====================
  CStateAtrophy::CEntityConfig::CEntityConfig
  ====================*/
CStateAtrophy::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}



/*====================
  CStateAtrophy::CStateAtrophy
  ====================*/
CStateAtrophy::CStateAtrophy() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
	m_modArmor.SetMult(m_pEntityConfig->GetArmorMult());
}
