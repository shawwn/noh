// (C)2006 S2 Games
// c_statewillofgod.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statewillofgod.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, WillOfGod);
//=============================================================================


/*====================
  CStateWillOfGod::CEntityConfig::CEntityConfig
  ====================*/
CStateWillOfGod::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}



/*====================
  CStateWillOfGod::CStateWillOfGod
  ====================*/
CStateWillOfGod::CStateWillOfGod() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
	m_modArmor.SetMult(m_pEntityConfig->GetArmorMult());
}
