// (C)2007 S2 Games
// c_statelocust.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statelocust.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Locust)
//=============================================================================


/*====================
  CStateLocust::CEntityConfig::CEntityConfig
  ====================*/
CStateLocust::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}



/*====================
  CStateBlind::CStateLocust
  ====================*/
CStateLocust::CStateLocust() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
