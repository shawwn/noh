// (C)2006 S2 Games
// c_stateburn.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateburn.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Burn);
//=============================================================================


/*====================
  CStateBurn::CEntityConfig::CEntityConfig
  ====================*/
CStateBurn::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateBurn::CStateBurn
  ====================*/
CStateBurn::CStateBurn() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateBurn::StateFrame
  ====================*/
void    CStateBurn::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

    return;
}
