// (C)2006 S2 Games
// c_propscar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propscar.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Scar)
//=============================================================================

/*====================
  CPropScar::Spawn
  ====================*/
void    CPropScar::Spawn()
{
    SetStatus(ENTITY_STATUS_ACTIVE);

    IPropEntity::Spawn();

    if (m_iTeam == -1)
    {
        for (int i(0); i < Game.GetNumTeams(); ++i)
            AssignToTeam(i);
    }
    else
    {
        AssignToTeam(m_iTeam);
    }
}


/*====================
  CPropScar::AddToScene
  ====================*/
bool    CPropScar::AddToScene(const CVec4f &v4Color, int iFlags)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    if (!IPropEntity::AddToScene(v4Color, iFlags))
        return false;
    return IPropFoundation::AddToScene(v4Color, iFlags);
}


/*====================
  CPropScar::Link
  ====================*/
void    CPropScar::Link()
{
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
        IPropFoundation::Link();
}


