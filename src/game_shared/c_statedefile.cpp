// (C)2006 S2 Games
// c_statedefile.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statedefile.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Defile);
//=============================================================================


/*====================
  CStateDefile::CEntityConfig::CEntityConfig
  ====================*/
CStateDefile::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateDefile::CStateDefile
  ====================*/
CStateDefile::CStateDefile() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateDefile::StateFrame
  ====================*/
void	CStateDefile::StateFrame()
{
	IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);
}
