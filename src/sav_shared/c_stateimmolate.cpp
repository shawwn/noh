// (C)2006 S2 Games
// c_stateimmolate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateimmolate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Immolate);
//=============================================================================


/*====================
  CStateImmolate::CEntityConfig::CEntityConfig
  ====================*/
CStateImmolate::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateImmolate::CStateImmolate
  ====================*/
CStateImmolate::CStateImmolate() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateImmolate::StateFrame
  ====================*/
void    CStateImmolate::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

    return;
}
