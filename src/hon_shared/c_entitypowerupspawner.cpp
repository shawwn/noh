// (C)2008 S2 Games
// c_entitypowerupspawner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_entitypowerupspawner.h"

#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, PowerupSpawner)
//=============================================================================

/*====================
  CEntityPowerupSpawner::CEntityPowerupSpawner
  ====================*/
CEntityPowerupSpawner::CEntityPowerupSpawner()
{
}


/*====================
  CEntityPowerupSpawner::ApplyWorldEntity
  ====================*/
void    CEntityPowerupSpawner::ApplyWorldEntity(const CWorldEntity &ent)
{
    IVisualEntity::ApplyWorldEntity(ent);
}


/*====================
  CEntityPowerupSpawner::Spawn
  ====================*/
void    CEntityPowerupSpawner::Spawn()
{
    IVisualEntity::Spawn();
}
