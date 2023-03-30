// (C)2006 S2 Games
// c_stateattrition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateattrition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Attrition);
//=============================================================================


/*====================
  CStateAttrition::CEntityConfig::CEntityConfig
  ====================*/
CStateAttrition::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorFactor, 0.0f)
{
}


/*====================
  CStateAttrition::CStateAttrition
  ====================*/
CStateAttrition::CStateAttrition() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.SetMult(m_pEntityConfig->GetArmorFactor());
}
