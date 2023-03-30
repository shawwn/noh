// (C)2007 S2 Games
// c_stateenraged.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateenraged.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Enraged)
//=============================================================================


/*====================
  CStateEnraged::CEntityConfig::CEntityConfig
  ====================*/
CStateEnraged::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(AttackSpeedMult, 1.0f),
INIT_ENTITY_CVAR(MoveSpeedMult, 1.0f),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(DamageMult, 1.0f)
{
}


/*====================
  CStateEnraged::CStateEnraged
  ====================*/
CStateEnraged::CStateEnraged() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.Set(0.0f, m_pEntityConfig->GetMoveSpeedMult(), 0.0f);
    m_modAttackSpeed.Set(0.0f, m_pEntityConfig->GetAttackSpeedMult(), 0.0f);
    m_modArmor.Set(0.0f, m_pEntityConfig->GetArmorMult(), 0.0f);
    m_modDamage.Set(0.0f, m_pEntityConfig->GetDamageMult(), 0.0f);
}
