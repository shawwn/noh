// (C)2007 S2 Games
// c_stateblocked.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateblocked.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Blocked)
//=============================================================================


/*====================
  CStateBlocked::CEntityConfig::CEntityConfig
  ====================*/
CStateBlocked::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateBlocked::CStateBlocked
  ====================*/
CStateBlocked::CStateBlocked() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}
