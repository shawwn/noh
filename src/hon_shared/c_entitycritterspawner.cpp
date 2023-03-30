// (C)2008 S2 Games
// c_entitycritterspawner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitycritterspawner.h"

#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, CritterSpawner)
//=============================================================================

/*====================
  CEntityCritterSpawner::CEntityCritterSpawner
  ====================*/
CEntityCritterSpawner::CEntityCritterSpawner()
{
}


/*====================
  CEntityCritterSpawner::ApplyWorldEntity
  ====================*/
void    CEntityCritterSpawner::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);
}


/*====================
  CEntityCritterSpawner::Spawn
  ====================*/
void    CEntityCritterSpawner::Spawn()
{
    IVisualEntity::Spawn();
}
