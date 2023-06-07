// (C)2006 S2 Games
// c_statecommanderlifeleak.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommanderlifeleak.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderLifeLeak);
//=============================================================================


/*====================
  CStateCommanderLifeLeak::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderLifeLeak::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 0.0f)
{
}


/*====================
  CStateCommanderLifeLeak::CStateCommanderLifeLeak
  ====================*/
CStateCommanderLifeLeak::CStateCommanderLifeLeak() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateCommanderLifeLeak::StateFrame
  ====================*/
void    CStateCommanderLifeLeak::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiOwnerIndex));
    if (pPlayer != NULL)
        pPlayer->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);
}
