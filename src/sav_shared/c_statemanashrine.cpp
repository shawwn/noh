// (C)2006 S2 Games
// c_statemanashrine.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemanashrine.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ManaShrine);
//=============================================================================


/*====================
  CStateManaShrine::CEntityConfig::CEntityConfig
  ====================*/
CStateManaShrine::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaRegenBoost, 10.0f)
{
}


/*====================
  CStateManaShrine::CStateManaShrine
  ====================*/
CStateManaShrine::CStateManaShrine() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modManaRegen.SetAdd(m_pEntityConfig->GetManaRegenBoost());
}
