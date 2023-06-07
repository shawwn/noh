// (C)2006 S2 Games
// c_statedecay.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statedecay.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Decay);
//=============================================================================


/*====================
  CStateDecay::CEntityConfig::CEntityConfig
  ====================*/
CStateDecay::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 15.0f)
{
}


/*====================
  CStateDecay::CStateDecay
  ====================*/
CStateDecay::CStateDecay() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateDecay::StateFrame
  ====================*/
void    CStateDecay::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));

    if (pOwner != NULL && pOwner != pInflictor)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);
    else if (pOwner != NULL && pOwner == pInflictor)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

    return;
}
