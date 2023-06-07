// (C)2006 S2 Games
// c_staterage.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_staterage.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Rage);
//=============================================================================


/*====================
  CStateRage::CEntityConfig::CEntityConfig
  ====================*/
CStateRage::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(AttackSpeedMult, 2.0f),
INIT_ENTITY_CVAR(MoveSpeedMult, 1.0f),
INIT_ENTITY_CVAR(ArmorMult, 0.5f)
{
}


/*====================
  CStateRage::CStateRage
  ====================*/
CStateRage::CStateRage() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.Set(0.0f, m_pEntityConfig->GetMoveSpeedMult(), 0.0f);
    m_modAttackSpeed.Set(0.0f, m_pEntityConfig->GetAttackSpeedMult(), 0.0f);
    m_modArmor.Set(0.0f, m_pEntityConfig->GetArmorMult(), 0.0f);
}
