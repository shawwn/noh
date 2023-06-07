// (C)2006 S2 Games
// c_staterainoffire.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_staterainoffire.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, RainOfFire);
//=============================================================================


/*====================
  CStateRainOfFire::CEntityConfig::CEntityConfig
  ====================*/
CStateRainOfFire::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 25.0f)
{
}


/*====================
  CStateRainOfFire::CStateRainOfFire
  ====================*/
CStateRainOfFire::CStateRainOfFire() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateRainOfFire::StateFrame
  ====================*/
void    CStateRainOfFire::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);
}
