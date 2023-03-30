// (C)2007 S2 Games
// c_statepetheal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statepetheal.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, PetHeal)
//=============================================================================


/*====================
  CStatePetHeal::CEntityConfig::CEntityConfig
  ====================*/
CStatePetHeal::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStatePetHeal::CStatePetHeal
  ====================*/
CStatePetHeal::CStatePetHeal() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}
