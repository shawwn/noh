// (C)2007 S2 Games
// c_triggerspawn.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_triggerspawn.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Trigger, Spawn)
//=============================================================================

/*====================
  CTriggerSpawn::RegisterEntityScripts
  ====================*/
void    CTriggerSpawn::RegisterEntityScripts(const CWorldEntity &ent)
{
    Game.RegisterEntityScript(GetIndex(), _T("spawn"), ent.GetProperty(_T("triggerspawn"), _T("")));
}

/*====================
  CTriggerSpawn::ApplyWorldEntity
  ====================*/
void    CTriggerSpawn::ApplyWorldEntity(const CWorldEntity &ent)
{
    ITriggerEntity::ApplyWorldEntity(ent);

    m_sSpawnType = ent.GetProperty(_T("spawntype"), _T(""));
    m_uiSpawnDelay = ent.GetPropertyInt(_T("spawndelay"), -1);
    m_unSpawnType = EntityRegistry.LookupID(m_sSpawnType);
}

/*====================
  CTriggerSpawn::ServerFrame
  ====================*/
bool    CTriggerSpawn::ServerFrame()
{
    ITriggerEntity::ServerFrame();

    if (GetStatus() != ENTITY_STATUS_ACTIVE || !Game.IsServer() || m_unSpawnType == INVALID_ENT_TYPE)
        return true;

    if (m_uiSpawnDelay == -1 || m_uiSpawnDelay < 1)
        return true;

    if (m_uiLastSpawn == -1)
        m_uiLastSpawn = Game.GetGameTime();

    if (m_uiLastSpawn + m_uiSpawnDelay < Game.GetGameTime())
    {
        m_uiLastSpawn = Game.GetGameTime();
        Trigger(GetWorldIndex(), _T("spawn"), true);
    }

    return true;
}

/*====================
  CTriggerSpawn::Trigger
  ====================*/
void    CTriggerSpawn::Trigger(uint uiTriggeringEntIndex, const tstring &sTriggerName, bool bPlayEffect)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE || !Game.IsServer())
        return;

    IGameEntity* pNewEnt(Game.AllocateEntity(m_unSpawnType));

    if (pNewEnt == NULL)
    {
        Console << _T("CTriggerSpawn::Frame: Object could not be allocated.") << newl;
        return;
    }

    IVisualEntity *pEnt(pNewEnt->GetAsVisualEnt());

    if (pEnt == NULL)
    {
        Console << _T("CTriggerSpawn::Frame: Improper entity type.") << newl;
        Game.DeleteEntity(pNewEnt);
        return;
    }

    uint uiWorldIndex(Game.AllocateNewWorldEntity());
    CWorldEntity *pWorldEnt(Game.GetWorldEntity(uiWorldIndex));

    if (pWorldEnt == NULL)
    {
        Console << _T("CTriggerSpawn::Frame: World entity could not be created.") << newl;
        Game.DeleteEntity(pNewEnt);
        return;
    }

    CWorldEntity *pThisEnt(Game.GetWorldEntity(GetWorldIndex()));

    if (pWorldEnt == NULL)
    {
        Console << _T("CTriggerSpawn::Frame: World entity could not be accessed.") << newl;
        Game.DeleteEntity(pNewEnt);
        Game.DeleteWorldEntity(uiWorldIndex);
        return;
    }

    // Inherit spawn point's settings
    (*pWorldEnt) = (*pThisEnt);
    pWorldEnt->SetType(m_sSpawnType);
    pWorldEnt->SetIndex(uiWorldIndex);
    pWorldEnt->SetGameIndex(pEnt->GetIndex());

    if (pEnt->GetWorldIndex() != INVALID_INDEX)
        Game.DeleteWorldEntity(pEnt->GetWorldIndex());

    pEnt->ApplyWorldEntity(*pWorldEnt);
    pEnt->Spawn();

    pEnt->Validate();

    m_uiTotalSpawned++;

    Game.RegisterTriggerParam(_T("spawnedindex"), XtoA(pEnt->GetIndex()));
    Game.RegisterTriggerParam(_T("totalspawned"), XtoA(m_uiTotalSpawned));

    ITriggerEntity::Trigger(uiTriggeringEntIndex, _T("spawn"), bPlayEffect);
}

/*====================
  CTriggerSpawn::Copy
  ====================*/
void    CTriggerSpawn::Copy(const IGameEntity &B)
{
    ITriggerEntity::Copy(B);

    const ITriggerEntity *pB(B.GetAsTrigger());

    if (!pB || !pB->IsSpawnTrigger())
        return;

    const CTriggerSpawn &C(*static_cast<const CTriggerSpawn*>(pB));

    m_sSpawnType =      C.m_sSpawnType;

    m_uiSpawnDelay =    C.m_uiSpawnDelay;
    m_uiLastSpawn =     C.m_uiLastSpawn;
    m_uiTotalSpawned =  C.m_uiTotalSpawned;

    m_unSpawnType =     C.m_unSpawnType;
}
