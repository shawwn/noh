// (C)2008 S2 Games
// c_entitykongorspawner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitykongorspawner.h"

#include "c_entitykongorcontroller.h"
#include "i_unitentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, BossSpawner)
//=============================================================================

/*====================
  CEntityBossSpawner::ApplyWorldEntity
  ====================*/
void    CEntityBossSpawner::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);
    m_sSpawnName = ent.GetProperty(_T("target0"));
    Game.Precache(m_sSpawnName, PRECACHE_ALL);
}


/*====================
  CEntityBossSpawner::Trigger
  ====================*/
void    CEntityBossSpawner::Trigger(IGameEntity *pActivator)
{
    IUnitEntity *pUnit(Game.AllocateDynamicEntity<IUnitEntity>(m_sSpawnName));
    if (pUnit == NULL)
        return;

    if (pActivator != NULL && pActivator->IsType<CEntityBossController>())
    {
        CEntityBossController *pController(pActivator->GetAs<CEntityBossController>());
        if (pController != NULL)
        {
            pController->AddActiveBossUID(pUnit->GetUniqueID());
            pUnit->SetLevel(pController->GetLevel());
        }
    }
    else
    {
        pUnit->SetLevel(1);
    }
    
    pUnit->SetTeam(TEAM_NEUTRAL);
    pUnit->SetPosition(GetPosition());
    pUnit->SetAngles(GetAngles());
    pUnit->Spawn();
    pUnit->ValidatePosition(TRACE_UNIT_SPAWN);

    pUnit->GetBrain().AddCommand(UNITCMD_GUARD, false, GetPosition().xy(), INVALID_INDEX, uint(-1), true);
}