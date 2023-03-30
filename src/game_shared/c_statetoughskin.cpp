// (C)2006 S2 Games
// c_statetoughskin.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statetoughskin.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ToughSkin);
//=============================================================================


/*====================
  CStateToughSkin::CEntityConfig::CEntityConfig
  ====================*/
CStateToughSkin::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorAdd, 0.30f)
{
}


/*====================
  CStateToughSkin::CStateToughSkin
  ====================*/
CStateToughSkin::CStateToughSkin() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
}
