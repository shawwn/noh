// (C)2007 S2 Games
// c_propdynamic.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_propdynamic.h"

#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Dynamic);
//=============================================================================

/*====================
  CPropDynamic::CPropDynamic
  ====================*/
CPropDynamic::CPropDynamic() :
IPropEntity(GetEntityConfig()),
m_uiCorpseTime(INVALID_TIME)
{
}


/*====================
  CPropDynamic::Spawn
  ====================*/
void    CPropDynamic::Spawn()
{
    IPropEntity::Spawn();
}


/*====================
  CPropDynamic::ServerFrameMovement
  ====================*/
bool    CPropDynamic::ServerFrameMovement()
{
    if (!IPropEntity::ServerFrameMovement())
        return false;

    SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));

    return true;
}


/*====================
  CPropDynamic::ServerFrameCleanup
  ====================*/
bool    CPropDynamic::ServerFrameCleanup()
{
    // Corpse
    if (GetStatus() == ENTITY_STATUS_CORPSE && Game.GetGameTime() >= m_uiCorpseTime)
        return false;

    return IPropEntity::ServerFrameCleanup();
}
