// (C)2007 S2 Games
// c_statetarget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statetargeted.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Targeted);
//=============================================================================


/*====================
  CStateTargeted::CEntityConfig::CEntityConfig
  ====================*/
CStateTargeted::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorAdjustment, -2.0f)
{
}


/*====================
  CStateTargeted::CStateTargeted
  ====================*/
CStateTargeted::CStateTargeted() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdjustment());
}
