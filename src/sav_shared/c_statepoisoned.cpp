// (C)2007 S2 Games
// c_statepoisoned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statepoisoned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Poisoned)
//=============================================================================


/*====================
  CStatePoisoned::CEntityConfig::CEntityConfig
  ====================*/
CStatePoisoned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 20.0f),
INIT_ENTITY_CVAR(SpeedMult, 1.0f)
{
}


/*====================
  CStatePoisoned::CStatePoisoned
  ====================*/
CStatePoisoned::CStatePoisoned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}


/*====================
  CStatePoisoned::StateFrame
  ====================*/
void    CStatePoisoned::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

    return;
}
