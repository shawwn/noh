// (C)2006 S2 Games
// c_statestonehide.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statestonehide.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, StoneHide);
//=============================================================================


/*====================
  CStateStoneHide::CEntityConfig::CEntityConfig
  ====================*/
CStateStoneHide::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorAdd, 0.30f)
{
}


/*====================
  CStateStoneHide::CStateStoneHide
  ====================*/
CStateStoneHide::CStateStoneHide() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
}
