// (C)2007 S2 Games
// c_stateflames.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateflames.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Flames);
//=============================================================================


/*====================
  CStateFlames::CEntityConfig::CEntityConfig
  ====================*/
CStateFlames::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(Radius, 200.0f),
INIT_ENTITY_CVAR(State, _T("")),
INIT_ENTITY_CVAR(StateDuration, 1000u)
{
}


/*====================
  CStateFlames::CStateFlames
  ====================*/
CStateFlames::CStateFlames() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateFlames::StateFrame
  ====================*/
void    CStateFlames::StateFrame()
{
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));

    if (!pOwner)
        return;

    CSphere sphRegion(pOwner->GetPosition(), m_pEntityConfig->GetRadius());

    uivector vSetResult;
    Game.GetEntitiesInRadius(vSetResult, sphRegion, 0);
    for (uivector_it it(vSetResult.begin()); it != vSetResult.end(); ++it)
    {
        IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
        if (pEnt == NULL)
            continue;
            
        if (pEnt->GetStatus() == ENTITY_STATUS_ACTIVE && (pEnt->IsCombat() || pEnt->IsBuilding()) && pOwner->IsEnemy(pEnt))
            pEnt->ApplyState(EntityRegistry.LookupID(m_pEntityConfig->GetState()), Game.GetGameTime(), m_pEntityConfig->GetStateDuration(), m_uiOwnerIndex);
    }

    return;
}
