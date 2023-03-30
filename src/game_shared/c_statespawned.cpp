// (C)2007 S2 Games
// c_statespawned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statespawned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Spawned);
//=============================================================================


/*====================
  CStateSpawned::CEntityConfig::CEntityConfig
  ====================*/
CStateSpawned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateSpawned::CStateSpawned
  ====================*/
CStateSpawned::CStateSpawned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
