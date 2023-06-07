// (C)2007 S2 Games
// c_statecommanderrecall.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecommanderrecall.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, CommanderRecall)
//=============================================================================

/*====================
  CStateCommanderRecall::CEntityConfig::CEntityConfig
  ====================*/
CStateCommanderRecall::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ActivationDelay, 3000),
INIT_ENTITY_CVAR(ActivationEffect, _T("")),
INIT_ENTITY_CVAR(TerminationEffect, _T(""))
{
}


/*====================
  CStateCommanderRecall::StateFrame
  ====================*/
void    CStateCommanderRecall::StateFrame()
{
    IEntityState::StateFrame();

    if (m_uiActivationTime > 0 && Game.GetGameTime() >= m_uiActivationTime)
    {
        IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiOwnerIndex));
        if (pPlayer != NULL)
        {
            CGameEvent evPoof;
            evPoof.SetSourcePosition(pPlayer->GetPosition());
            evPoof.SetSourceAngles(pPlayer->GetAngles());
            evPoof.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetTerminationEffect()));
            Game.AddEvent(evPoof);

            pPlayer->Spawn2();
        }

        m_uiActivationTime = -1;
    }
}


/*====================
  CStateCommanderRecall::Use
  ====================*/
bool    CStateCommanderRecall::Use(IGameEntity *pActivator)
{
    if (m_uiActivationTime != 0)
        return false;

    m_uiActivationTime = Game.GetGameTime() + m_pEntityConfig->GetActivationDelay();
    m_uiDuration = -1;
    
    ICombatEntity *pOwner(Game.GetCombatEntity(m_uiOwnerIndex));
    if (pOwner == NULL)
        return false;

    ResHandle hEffect(Game.RegisterEffect(m_pEntityConfig->GetActivationEffect()));
    if (hEffect != INVALID_RESOURCE)
    {
        CGameEvent evEffect;
        evEffect.SetSourceEntity(pOwner->GetIndex());
        evEffect.SetEffect(hEffect);
        Game.AddEvent(evEffect);
    }

    pOwner->SetAction(PLAYER_ACTION_IMMOBILE, m_uiActivationTime);
    return true;
}
