// (C)2006 S2 Games
// c_statemanacrystal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemanacrystal.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ManaCrystal);
//=============================================================================


/*====================
  CStateManaCrystal::CEntityConfig::CEntityConfig
  ====================*/
CStateManaCrystal::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaCostMultiplier, 0.60f)
{
}


/*====================
  CStateManaCrystal::CStateManaCrystal
  ====================*/
CStateManaCrystal::CStateManaCrystal() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modGunManaCost.SetMult(m_pEntityConfig->GetManaCostMultiplier());
}
