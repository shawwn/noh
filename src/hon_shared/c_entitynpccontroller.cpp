// (C)2007 S2 Games
// c_entitynpccontroller.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitynpccontroller.h"
#include "i_npcentity.h"
#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, NpcController)
//=============================================================================

/*====================
  CEntityNpcController::CEntityConfig::CEntityConfig
  ====================*/
CEntityNpcController::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(MinimapColor, CVec3f(1.0f, 1.0f, 1.0f))
{
}


/*====================
  CEntityNpcController::CEntityNpcController
  ====================*/
CEntityNpcController::CEntityNpcController() :
IVisualEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_uiNextRespawnCheckTime(INVALID_TIME),
m_bDistrubed(false)
{
}


/*====================
  CEntityNpcController::ApplyWorldEntity
  ====================*/
void    CEntityNpcController::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);

    m_uiSpawnPeriod = ent.GetPropertyInt(_T("period"), 60000);
    m_fSpawnRadius = ent.GetPropertyFloat(_T("radius"), 0.0f);
}


/*====================
  CEntityNpcController::Spawn
  ====================*/
void    CEntityNpcController::Spawn()
{
    IVisualEntity::Spawn();

    m_yStatus = ENTITY_STATUS_ACTIVE;
    m_hModel = INVALID_INDEX;
    m_uiNextRespawnCheckTime = Game.GetGameTime();
    m_bDistrubed = false;
}


/*====================
  CEntityNpcController::DeleteChildren
  ====================*/
void    CEntityNpcController::DeleteChildren()
{
    for (uivector::iterator it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
    {
        IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
        if (!pEntity)
            continue;

        INpcEntity *pNpc(pEntity->GetAsNpc());
        if (!pNpc)
            continue;

        Game.DeleteEntity(pNpc);
    }

    m_vChildren.clear();
}


/*====================
  CEntityNpcController::SpawnChildren
  ====================*/
void    CEntityNpcController::SpawnChildren()
{
    for (vector<SNpcSpawnState>::iterator it(m_vNpcSpawnStates.begin()); it != m_vNpcSpawnStates.end(); ++it)
    {
        // Spawn a npc
        IGameEntity *pNewEnt(Game.AllocateEntity(it->unType));
        if (pNewEnt == NULL || !pNewEnt->IsNpc())
        {
            Console.Warn << _T("Failed to spawn npc: ") << EntityRegistry.LookupName(it->unType) << newl;
            continue;
        }

        INpcEntity *pNpc(pNewEnt->GetAsNpc());

        // Set properties
        pNpc->SetPosition(it->v3Pos);
        pNpc->SetAngles(it->v3Angles);
        pNpc->SetDefinition(it->hDefinition);
        pNpc->SetTeam(0);
        pNpc->SetControllerUID(m_uiUniqueID);
        
        pNpc->Spawn();
        
        m_vChildren.push_back(pNpc->GetUniqueID());
    }
}


/*====================
  CEntityNpcController::ServerFrame
  ====================*/
bool    CEntityNpcController::ServerFrame()
{
    if (Game.GetGameTime() < m_uiNextRespawnCheckTime)
        return true;

    // Only attempt to respawn if this ground has been disturbed in any way
    if (m_bDistrubed)
    {
        bool bRespawn(true);

        // Only respawn if no children are aggro'd
        bool bAggro(false);
        for (uivector::iterator it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
        {
            IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
            if (!pEntity)
                continue;

            INpcEntity *pNpc(pEntity->GetAsNpc());
            if (!pNpc)
                continue;

            if (pNpc->GetMaxAggro() != INVALID_INDEX)
            {
                bAggro = true;
                break;
            }
        }

        if (bAggro)
            bRespawn = false;

        if (bRespawn)
        {
            DeleteChildren();
            SpawnChildren();

            m_bDistrubed = false;
        }
    }

    m_uiNextRespawnCheckTime = Game.GetGameTime() + m_uiSpawnPeriod;
    
    return true;
}


/*====================
  CEntityNpcController::AddNpc
  ====================*/
void    CEntityNpcController::AddNpc(INpcEntity *pNpc)
{
    SNpcSpawnState  sSpawnState;
    sSpawnState.unType = pNpc->GetType();
    sSpawnState.v3Pos = pNpc->GetPosition();
    sSpawnState.v3Angles = pNpc->GetAngles();
    sSpawnState.hDefinition = pNpc->GetDefinition();

    m_vNpcSpawnStates.push_back(sSpawnState);
    m_vChildren.push_back(pNpc->GetUniqueID());
}
