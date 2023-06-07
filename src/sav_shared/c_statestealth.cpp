// (C)2006 S2 Games
// c_statestealth.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statestealth.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Stealth);
//=============================================================================


/*====================
  CStateStealth::CEntityConfig::CEntityConfig
  ====================*/
CStateStealth::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateStealth::CStateStealth
  ====================*/
CStateStealth::CStateStealth() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
