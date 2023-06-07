// (C)2007 S2 Games
// c_gadgetofficerspawnflag.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetofficerspawnflag.h"
#include "c_teaminfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR(Gadget, OfficerSpawnFlag);

CGadgetOfficerSpawnFlag::CEntityConfig  CGadgetOfficerSpawnFlag::s_EntityConfig(_T("OfficerSpawnFlag"));
//=============================================================================

/*====================
  CGadgetOfficerSpawnFlag::CGadgetOfficerSpawnFlag
  ====================*/
CGadgetOfficerSpawnFlag::CGadgetOfficerSpawnFlag() :
IGadgetEntity(&s_EntityConfig)
{
}


/*====================
  CGadgetOfficerSpawnFlag::Spawn
  ====================*/
void    CGadgetOfficerSpawnFlag::Spawn()
{
    CTeamInfo *pTeam(Game.GetTeam(m_iTeam));
    if (pTeam == NULL)
        return;

    //pTeam->AddSpawnPointIndex(m_uiIndex);
}


/*====================
  CGadgetOfficerSpawnFlag::Kill
  ====================*/
void    CGadgetOfficerSpawnFlag::Kill(IGameEntity *pAttacker)
{
    CTeamInfo *pTeam(Game.GetTeam(m_iTeam));
    if (pTeam == NULL)
        return;

    //pTeam->RemoveSpawnPoint();
}
