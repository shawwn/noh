// (C)2006 S2 Games
// c_statemanaclarity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemanaclarity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ManaClarity);
//=============================================================================


/*====================
  CStateManaClarity::CEntityConfig::CEntityConfig
  ====================*/
CStateManaClarity::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaRegenBoost, 3.0f)
{
}


/*====================
  CStateManaClarity::CStateManaClarity
  ====================*/
CStateManaClarity::CStateManaClarity() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modManaRegen.SetAdd(m_pEntityConfig->GetManaRegenBoost());
}
