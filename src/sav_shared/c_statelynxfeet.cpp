// (C)2006 S2 Games
// c_statelynxfeet.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statelynxfeet.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, LynxFeet);
//=============================================================================


/*====================
  CStateLynxFeet::CEntityConfig::CEntityConfig
  ====================*/
CStateLynxFeet::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedBoost, 0.30f)
{
}


/*====================
  CStateLynxFeet::CStateLynxFeet
  ====================*/
CStateLynxFeet::CStateLynxFeet() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(1.0f + m_pEntityConfig->GetSpeedBoost());
}
