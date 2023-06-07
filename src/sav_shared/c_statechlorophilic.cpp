// (C)2006 S2 Games
// c_statechlorophilic.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statechlorophilic.h"
//=============================================================================

//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Chlorophilic);
//=============================================================================


/*====================
  CStateChlorophilic::CEntityConfig::CEntityConfig
  ====================*/
CStateChlorophilic::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthPerSecond, 50.0f)
{
}


/*====================
  CStateChlorophilic::CStateChlorophilic
  ====================*/
CStateChlorophilic::CStateChlorophilic() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateChlorophilic::StateFrame
  ====================*/
void    CStateChlorophilic::StateFrame()
{
    IEntityState::StateFrame();
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    IBuildingEntity *pBuilding(Game.GetBuildingEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
    {
        if (pBuilding->GetHealth() < pBuilding->GetHealLimit())
            pOwner->Heal(m_pEntityConfig->GetHealthPerSecond() * MsToSec(Game.GetFrameLength()), pInflictor);
    }


    return;
}

/*====================
  CStateChlorophilic::Activated
  ====================*/
void    CStateChlorophilic::Activated()
{
    IEntityState::Activated();
    IBuildingEntity *pOwner(Game.GetBuildingEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->SetHealLimit(pOwner->GetHealth());

}
