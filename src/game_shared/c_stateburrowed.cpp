// (C)2007 S2 Games
// c_stateburrowed.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateburrowed.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Burrowed)
//=============================================================================


/*====================
  CStateBurrowed::CEntityConfig::CEntityConfig
  ====================*/
CStateBurrowed::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateBurrowed::CStateBurrowed
  ====================*/
CStateBurrowed::CStateBurrowed() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modSpeed.SetMult(0.0f);
}
