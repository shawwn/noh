// (C)2006 S2 Games
// c_propspawnpoint.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propspawnpoint.h"
#include "c_teaminfo.h"

#include "../k2/c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, SpawnPoint)
//=============================================================================

/*====================
  CPropSpawnPoint::Spawn
  ====================*/
void    CPropSpawnPoint::Spawn()
{
    SetStatus(ENTITY_STATUS_DORMANT);

    CTeamInfo *pTeam(Game.GetTeam(m_iTeam));
    if (pTeam == NULL)
    {
        Console.Warn << _T("CPropSpawnPoint::Spawn() - Team does not exist: ") << m_iTeam << newl;
        return;
    }

    Console << _T("Adding SpawnPoint #") << m_uiWorldIndex << _T(" as entity #") << m_uiIndex << _T(" to team ") << m_iTeam << newl;
    pTeam->AddSpawnPointIndex(m_uiIndex);
}
