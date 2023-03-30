// (C)2008 S2 Games
// game_shared_entities.h
//
//=============================================================================
#ifndef __GAME_SHARED_ENTITIES_H__
#define __GAME_SHARED_ENTITIES_H__

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IVisualEntity;
class IPropEntity;

class IUnitEntity;
class IHeroEntity;
class ICreepEntity;
class IBuildingEntity;
class IGadgetEntity;
class IBitEntity;

class ISlaveEntity;
class IEntityTool;
class IEntityAbility;
class IEntityItem;
class IEntityState;

class IProjectile;
class IAffector;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EEntityType
{
    // 0 is the delete code in snapshot
    Player = 1,
    Info_Team,
    Info_Stats,

    // Marker for entities that editor actually cares about
    Entity_Tangible,

    Entity_CreepSpawner,
    Entity_NeutralCampController,
    Entity_NeutralCampSpawner,
    Entity_PowerupSpawner,
    Entity_BossController,
    Entity_BossSpawner,
    Entity_Chest,
    Entity_Effect,
    Entity_LaneNode,
    Entity_CritterSpawner,

    Prop_Dynamic,
    Prop_Static,
    Prop_Scenery,
    Prop_Tree,
    Prop_Water,
    Prop_Cliff,
    Prop_Cliff2,
    
    Trigger_Proximity,
    Trigger_Spawn,
    Trigger_RefPoint,
    Trigger_SpawnPoint,
    
    Light_Static,

    Shop_Info,
    Shop_ItemInfo,

    Trigger_Marker,
    Entity_Camera,

    // This needs to be last, it's just a place holder
    Entity_Dynamic = 0x100
};
//=============================================================================

#endif //__GAME_SHARED_ENTITIES_H__
