// (C)2006 S2 Games
// c_statecommandercenturionarmor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommandercenturionarmor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderCenturionArmor);
//=============================================================================


/*====================
  CStateCommanderCenturionArmor::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderCenturionArmor::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}


/*====================
  CStateCommanderCenturionArmor::CStateCommanderCenturionArmor
  ====================*/
CStateCommanderCenturionArmor::CStateCommanderCenturionArmor() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
}
