// (C)2007 S2 Games
// c_statecountered.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecountered.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Countered);
//=============================================================================


/*====================
  CStateCountered::CEntityConfig::CEntityConfig
  ====================*/
CStateCountered::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateCountered::CStateCountered
  ====================*/
CStateCountered::CStateCountered() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
