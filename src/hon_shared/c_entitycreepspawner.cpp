// (C)2008 S2 Games
// c_entitycreepspawner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_entitycreepspawner.h"

#include "c_entitylanenode.h"

#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, CreepSpawner)
//=============================================================================

/*====================
  CEntityCreepSpawner::ApplyWorldEntity
  ====================*/
CEntityCreepSpawner::CEntityCreepSpawner()
{
    for (uint ui(0); ui < NUM_CREEP_SPAWNER_TARGETS; ++ui)
        m_auiTargetUIDs[ui] = INVALID_INDEX;
}


/*====================
  CEntityCreepSpawner::ApplyWorldEntity
  ====================*/
void    CEntityCreepSpawner::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    for (uint ui(0); ui < NUM_CREEP_SPAWNER_TARGETS; ++ui)
        m_asTargets[ui] = ent.GetProperty(_T("target") + XtoA(ui), TSNULL);
}


/*====================
  CEntityCreepSpawner::Spawn
  ====================*/
void    CEntityCreepSpawner::Spawn()
{
    IVisualEntity::Spawn();

    CEntityLaneNode *pWaypoint(Game.GetEntityFromNameAs<CEntityLaneNode>(m_asTargets[0]));
    while (pWaypoint != nullptr)
    {
        m_cLane.AddWaypoint(pWaypoint->GetPosition().xy());

        pWaypoint = Game.GetEntityFromNameAs<CEntityLaneNode>(pWaypoint->GetTarget());
    }
}


/*====================
  CEntityCreepSpawner::GameStart
  ====================*/
void    CEntityCreepSpawner::GameStart()
{
    // Register targets
    for (uint ui(0); ui < NUM_CREEP_SPAWNER_TARGETS; ++ui)
    {
        if (m_asTargets[ui].empty())
            m_auiTargetUIDs[ui] = INVALID_INDEX;
        else
        {
            IVisualEntity *pTarget(Game.GetEntityFromName(m_asTargets[ui]));
            if (pTarget)
                m_auiTargetUIDs[ui] = pTarget->GetUniqueID();
            else
            {
                Console.Warn << _T("Target ") << SingleQuoteStr(m_asTargets[ui]) << _T(" not found") << newl;
                m_auiTargetUIDs[ui] = INVALID_INDEX;
            }
        }
    }
}
