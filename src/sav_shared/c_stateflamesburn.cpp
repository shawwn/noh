// (C)2007 S2 Games
// c_stateflamesburn.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateflamesburn.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, FlamesBurn);
//=============================================================================


/*====================
  CStateFlamesBurn::CEntityConfig::CEntityConfig
  ====================*/
CStateFlamesBurn::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateFlamesBurn::CStateFlamesBurn
  ====================*/
CStateFlamesBurn::CStateFlamesBurn() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateFlamesBurn::StateFrame
  ====================*/
void    CStateFlamesBurn::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IGameEntity *pOwner(Game.GetEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

    return;
}
