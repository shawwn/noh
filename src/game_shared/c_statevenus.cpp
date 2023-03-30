// (C)2007 S2 Games
// c_statevenus.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statevenus.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Venus)
//=============================================================================


/*====================
  CStateVenus::CEntityConfig::CEntityConfig
  ====================*/
CStateVenus::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 20.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateVenus::CStateVenus
  ====================*/
CStateVenus::CStateVenus() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}


/*====================
  CStateVenus::StateFrame
  ====================*/
void    CStateVenus::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

    return;
}
