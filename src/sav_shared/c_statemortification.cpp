// (C)2006 S2 Games
// c_statemortification.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemortification.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Mortification);
//=============================================================================


/*====================
  CStateMortification::CEntityConfig::CEntityConfig
  ====================*/
CStateMortification::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedReduction, 0.15f)
{
}


/*====================
  CStateMortification::CStateMortification
  ====================*/
CStateMortification::CStateMortification() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(1.0f - m_pEntityConfig->GetSpeedReduction());
}
