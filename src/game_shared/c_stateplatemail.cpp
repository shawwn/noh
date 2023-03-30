// (C)2006 S2 Games
// c_stateplatemail.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateplatemail.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Platemail);
//=============================================================================


/*====================
  CStatePlatemail::CEntityConfig::CEntityConfig
  ====================*/
CStatePlatemail::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorAdd, 0.30f)
{
}


/*====================
  CStatePlatemail::CStatePlatemail
  ====================*/
CStatePlatemail::CStatePlatemail() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.SetAdd(m_pEntityConfig->GetArmorAdd());
}
