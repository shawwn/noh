// (C)2007 S2 Games
// c_statefalldown.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statefalldown.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, FallDown);
//=============================================================================


/*====================
  CStateFallDown::CEntityConfig::CEntityConfig
  ====================*/
CStateFallDown::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateFallDown::CStateFallDown
  ====================*/
CStateFallDown::CStateFallDown() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
