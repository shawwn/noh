// (C)2006 S2 Games
// c_statecommanderfear.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommanderfear.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderFear);
//=============================================================================


/*====================
  CStateCommanderFear::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderFear::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}


/*====================
  CStateCommanderFear::CStateCommanderFear
  ====================*/
CStateCommanderFear::CStateCommanderFear() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
}
