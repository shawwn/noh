// (C)2007 S2 Games
// c_stateenchanted.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateenchanted.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Enchanted)
//=============================================================================


/*====================
  CStateEnchanted::CEntityConfig::CEntityConfig
  ====================*/
CStateEnchanted::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateEnchanted::CStateEnchanted
  ====================*/
CStateEnchanted::CStateEnchanted() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
