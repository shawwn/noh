// (C)2007 S2 Games
// c_statedash.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statedash.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Dash);
//=============================================================================


/*====================
  CStateDash::CEntityConfig::CEntityConfig
  ====================*/
CStateDash::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateDash::CStateDash
  ====================*/
CStateDash::CStateDash() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
