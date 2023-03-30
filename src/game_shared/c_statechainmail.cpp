// (C)2006 S2 Games
// c_statechainmail.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statechainmail.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Chainmail);
//=============================================================================


/*====================
  CStateChainmail::CEntityConfig::CEntityConfig
  ====================*/
CStateChainmail::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorAdd, 0.15f)
{
}



/*====================
  CStateChainmail::CStateChainmail
  ====================*/
CStateChainmail::CStateChainmail() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
}
