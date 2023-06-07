// (C)2007 S2 Games
// c_stateafraid.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateafraid.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Afraid)
//=============================================================================


/*====================
  CStateAfraid::CEntityConfig::CEntityConfig
  ====================*/
CStateAfraid::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}


/*====================
  CStateAfraid::CStateAfraid
  ====================*/
CStateAfraid::CStateAfraid() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
}
