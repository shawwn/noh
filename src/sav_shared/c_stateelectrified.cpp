// (C)2007 S2 Games
// c_stateelectrified.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateelectrified.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Electrified)
//=============================================================================


/*====================
  CStateElectrified::CEntityConfig::CEntityConfig
  ====================*/
CStateElectrified::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 20.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStateElectrified::CStateElectrified
  ====================*/
CStateElectrified::CStateElectrified() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}


/*====================
  CStateElectrified::StateFrame
  ====================*/
void    CStateElectrified::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

    return;
}
