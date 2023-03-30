// (C)2006 S2 Games
// c_entityregistry.h
//
//=============================================================================
#ifndef __C_ENTITYREGISTRY_H__
#define __C_ENTITYREGISTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_entitysnapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGameEntity;
class IEntityAllocator;

extern GAME_SHARED_API class CEntityRegistry &g_EntityRegistry;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================

// Do not change the indexes
enum EEntityType
{
    // 0 is the delete code in snapshot
    Entity_Chest = 1,
    Entity_GameInfo,
    Entity_NpcController,
    Entity_Soul,
    Entity_TeamInfo,
    Entity_ClientInfo,
    Entity_Effect,

    Building_Academy = 100,
    Building_Armory,
    Building_ArrowTower,
    Building_CannonTower,
    Building_CharmShrine,
    Building_ChlorophilicSpire,
    Building_EntangleSpire,
    Building_Garrison,
    Building_GroveMine,
    Building_HumanHellShrine,
    Building_Lair,
    Building_Monastery,
    Building_Nexus,
    Building_PredatorDen,
    Building_Sanctuary,
    Building_ShieldTower,
    Building_SiegeWorkshop,
    Building_SteamMine,
    Building_StrataSpire,
    Building_Stronghold,
    Building_SubLair,
    
    Consumable_AmmoPack = 200,
    Consumable_AmmoSatchel,
    Consumable_Chainmail,
    Consumable_HealthMajor,
    Consumable_HealthMinor,
    Consumable_HealthReplenish,
    Consumable_HealthShrine,
    Consumable_ManaClarity,
    Consumable_ManaMajor,
    Consumable_ManaMinor,
    Consumable_ManaShrine,
    Consumable_Platemail,
    Consumable_SpeedBoost,
    Consumable_LynxFeet,
    Consumable_StaminaMajor,
    Consumable_StaminaMinor,
    Consumable_ToughSkin,
    Consumable_StoneHide,
    Consumable_ManaCrystal,
    Consumable_ManaStone,

    
    Gadget_AmmoDepot = 300,
    Gadget_DemoCharge,
    Gadget_ElectricEye,
    Gadget_EntangleTrap,
    Gadget_FireRain,
    Gadget_Hail,
    Gadget_HealingWard,
    Gadget_HealthShrine,
    Gadget_HumanOfficerSpawnFlag,
    Gadget_Immolate,
    Gadget_ManaShrine,
    Gadget_ManaWard,
    Gadget_Meteor,
    Gadget_Mole,
    Gadget_ShieldGenerator,
    Gadget_Venus,
    Gadget_Sentry,
    Gadget_BeastSpawnPortal,
    Gadget_SteamTurret,
    Gadget_Reveal,
    Gadget_ManaFountain,
    
    Gun_Blaze = 400,
    Gun_Crossbow,
    Gun_Exorcise,
    Gun_Fireball,
    Gun_FrostBolts,
    Gun_HolyBolt,
    Gun_ImpFire,
    Gun_Launcher,
    Gun_Lightning,
    Gun_Locusts,
    Gun_Mortar,
    Gun_Ravager,
    Gun_Repeater,
    Gun_Rifle,
    Gun_Scorch,
    Gun_Shotgun,
    Gun_SniperBow,
    Gun_Venom,
    Gun_StaticDischarge,
    Siege_FireBombardment,
    Siege_StoneBombardment,
    
    Melee_BatteringRam = 500,
    Melee_BattleAxe,
    Melee_BattleAxe2,
    Melee_BearlothSwipe,
    Melee_Broadswords,
    Melee_Broadswords2,
    Melee_ConjurerClaws,
    Melee_ConjurerClaws2,
    Melee_FlameSword,
    Melee_Hammer,
    Melee_Hammer2,
    Melee_HellStaff,
    Melee_PredatorClaws,
    Melee_PredatorClaws2,
    Melee_Push,
    Melee_ShifterClaws,
    Melee_ShifterClaws2,
    Melee_ShortBlades,
    Melee_ShortBlades2,
    Melee_SpiritHammer,
    Melee_SpiritHammer2,
    Melee_Staff,
    Melee_Staff2,
    Melee_SummonerStaff,
    Melee_SummonerStaff2,
    Melee_Tree,
    Melee_WorkerHammer,
    Melee_WorkerClaws,
    
    Pet_Bearloth = 600,
    Pet_Imp,
    Pet_BatteringRam,
    Pet_Behemoth,
    Pet_Chaplain,
    Pet_Conjurer,
    Pet_Engineer,
    Pet_Legionnaire,
    Pet_Malphas,
    Pet_Marksman,
    Pet_Predator,
    Pet_Revenant,
    Pet_Savage,
    Pet_Shaman,
    Pet_ShapeShifter,
    Pet_Steambuchet,
    Pet_Summoner,
    Pet_Tempest,
    Pet_HumanWorker,
    Pet_BeastWorker,

    Player_BatteringRam = 700,
    Player_Behemoth,
    Player_Chaplain,
    Player_Commander,
    Player_Conjurer,
    Player_Engineer,
    Player_Legionnaire,
    Player_Malphas,
    Player_Marksman,
    Player_Observer,
    Player_PossessedLegionnaire,
    Player_Predator,
    Player_Revenant,
    Player_Savage,
    Player_Shaman,
    Player_ShapeShifter,
    Player_Steambuchet,
    Player_Summoner,
    Player_Tempest,

    Projectile_Arrow = 800,
    Projectile_Bolt,
    Projectile_BuilderGrenade,
    Projectile_Fireball,
    Projectile_FrostBolt,
    Projectile_Grenade,
    Projectile_Holy,
    Projectile_ImpFireball,
    Projectile_Locust,
    Projectile_NpcShot,
    Projectile_PredFireball,
    Projectile_Rocket,
    Projectile_Scorch,
    Projectile_SiegeFire,
    Projectile_SiegeStone,
    Projectile_TowerArrow,
    Projectile_TowerCannonball,
    Projectile_VenomSpore,
    Projectile_VenusSpore,
    
    Prop_BaseBuilding = 900,
    Prop_Dynamic,
    Prop_Mine,
    Prop_Scar,
    Prop_Scenery,
    
    Skill_AmmoDepot = 1000,
    Skill_BackStab,
    Skill_Bash,
    Skill_BearlothSlam,
    Skill_BeastBuild,
    Skill_Blind,
    Skill_Build,
    Skill_Burrow,
    Skill_Carnivorous,
    Skill_Cheetah,
    Skill_CriticalStrike,
    Skill_DemoCharge,
    Skill_Disembark,
    Skill_DoubleSwing,
    Skill_ElectricEye,
    Skill_Enrage,
    Skill_Ethereal,
    Skill_Flames,
    Skill_HealingWard,
    Skill_HumanOfficerPortal,
    Skill_ImpPoisoned,
    Skill_LogSweep,
    Skill_ManaWard,
    Skill_Morale,
    Skill_Pounce,
    Skill_Rage,
    Skill_Reconstitute,
    Skill_Riposte,
    Skill_Sacrifice,
    Skill_ShieldGenerator,
    Skill_Shockwave,
    Skill_Stealth,
    Skill_SteamBoost,
    Skill_Stomp,
    Skill_SummonMole,
    Skill_TephraWave,
    Skill_TrunkSlam,
    Skill_Venomous,
    Skill_WhirlingBlade,
    Skill_Sentry,

    Spell_Atrophy = 1100,
    Spell_Attrition,
    Spell_BeastHeal,
    Spell_Combustion,
    Spell_CommanderCenturionArmor,
    Spell_CommanderCripple,
    Spell_CommanderDispel,
    Spell_CommanderFear,
    Spell_CommanderHeal,
    Spell_CommanderLifeLeak,
    Spell_CommanderResurrect,
    Spell_CommanderSpeedBoost,
    Spell_Defile,
    Spell_Enchant,
    Spell_Entangle,
    Spell_EntangleTrap,
    Spell_Eruption,
    Spell_Fear,
    Spell_Grenade,
    Spell_GroupHeal,
    Spell_HailStorm,
    Spell_Heal,
    Spell_HealPet,
    Spell_Immolate,
    Spell_Meteor,
    Spell_MindWarp,
    Spell_Mortification,
    Spell_Polymorph,
    Spell_RainOfFire,
    Spell_Reincarnate,
    Spell_Resurrect,
    Spell_SummonBearloth,
    Spell_SummonImp,
    Spell_Venus,
    Spell_CommanderRecall,
    Spell_BeastOfficerPortal,
    Spell_SummonWorker,
    Spell_SteamTurret,
    Spell_CommanderReveal,
    Spell_ManaFountain,
    Spell_StormShield,

    State_Afraid = 1200,
    State_Atrophy,
    State_Attrition,
    State_BearlothStunned,
    State_BeastHeal,
    State_BehemothStunned,
    State_Bleed,
    State_Blind,
    State_Burn,
    State_Burrowed,
    State_Carnivorous,
    State_Chainmail,
    State_Cheetah,
    State_Combustion,
    State_CommanderCenturionArmor,
    State_CommanderCripple,
    State_CommanderFear,
    State_CommanderHeal,
    State_CommanderLifeLeak,
    State_CommanderSpeedBoost,
    State_Confused,
    State_Defile,
    State_Enchanted,
    State_Enraged,
    State_Entangled,
    State_Ethereal,
    State_FallDown,
    State_Flames,
    State_FlamesBurn,
    State_Frost,
    State_GrenadeBlast,
    State_GroupHeal,
    State_Heal,
    State_HealingWard,
    State_HealthReplenish,
    State_HealthShrine,
    State_Immolate,
    State_ImpPoisoned,
    State_Locust,
    State_ManaClarity,
    State_ManaShrine,
    State_ManaWard,
    State_Morale,
    State_Morphed,
    State_Mortification,
    State_NpcAbility,
    State_Officer,
    State_OfficerAura,
    State_PersistantItem,
    State_PersistantReplenish,
    State_PetHeal,
    State_Platemail,
    State_Poisoned,
    State_Rage,
    State_RainOfFire,
    State_Reconstitute,
    State_Riposte,
    State_Sacrifice,
    State_ShieldTower,
    State_Spawned,
    State_SpeedBoost,
    State_Stealth,
    State_SteamBoost,
    State_Stunned,
    State_Targeted,
    State_Blaze,
    State_Venus,
    State_Venomous,
    State_Spores,
    State_Chlorophilic,
    State_Electrified,
    State_ManaCrystal,
    State_ManaStone,
    State_CommanderRecall,
    State_StoneHide,
    State_ToughSkin,
    State_Dash,
    State_LynxFeet,
    State_StormShield,
    State_WillOfGod,
    //Make sure once we hit 100 states that trigger_proximity is set to a higher number, we currently have 78 states as of StormShield
    
    Trigger_Proximity = 1300,
    Trigger_Spawn,
    
    Light_Static = 1400,
    Npc_Critter,
    OfficerCommand_Attack2,
    OfficerCommand_Follow2,
    OfficerCommand_Move2,
    OfficerCommand_Defend,
    Persistant_Item,
    PetCommand_Attack,
    PetCommand_Follow,
    PetCommand_Move,
};


#ifdef GAME_SHARED_EXPORTS
#define EntityRegistry (*CEntityRegistry::GetInstance())
#else
#define EntityRegistry g_EntityRegistry
#endif

#define DECLARE_ENT_ALLOCATOR(type, name)   static  CEntityAllocator<C##type##name> s_Allocator;
#define DEFINE_ENT_ALLOCATOR(type, name)    CEntityAllocator<C##type##name> C##type##name::s_Allocator(_T(#type"_"#name), type##_##name);

#define DECLARE_ENT_ALLOCATOR2(type, name) \
    friend class CEntityAllocator2<C##type##name>; \
    static CEntityAllocator2<C##type##name> s_Allocator; \
    static CEntityConfig s_EntityConfig; \
    static CEntityConfig *GetEntityConfig() { return &s_EntityConfig; }

#define DEFINE_ENT_ALLOCATOR2(type, name) \
    C##type##name::CEntityConfig        C##type##name::s_EntityConfig(_T(#type"_"#name)); \
    CEntityAllocator2<C##type##name>    C##type##name::s_Allocator(_T(#type"_"#name), type##_##name);

typedef map<ushort, IEntityAllocator*>      EntAllocatorIDMap;
typedef pair<ushort, IEntityAllocator*>     EntAllocatorIDEntry;
typedef EntAllocatorIDMap::iterator         EntAllocatorIDMap_it;
typedef EntAllocatorIDMap::const_iterator   EntAllocatorIDMap_cit;

typedef map<tstring, IEntityAllocator*>     EntAllocatorNameMap;
typedef pair<tstring, IEntityAllocator*>    EntAllocatorNameEntry;
typedef EntAllocatorNameMap::iterator       EntAllocatorNameMap_it;
//=============================================================================

//=============================================================================
// IEntityAllocator
//=============================================================================
class IEntityAllocator
{
protected:
    tstring             m_sName;
    ushort              m_unID;

    CEntitySnapshot*    m_pBaseline;

    IEntityAllocator();

public:
    virtual ~IEntityAllocator();
    IEntityAllocator(const tstring &sName, ushort unID);

    ushort                  GetID() const           { return m_unID; }
    const tstring&          GetName() const         { return m_sName; }
    const CEntitySnapshot*  GetBaseline() const     { return m_pBaseline; }

    virtual IGameEntity*                Allocate() const = 0;
    virtual const vector<SDataField>*   GetTypeVector() const = 0;
    virtual void                        ClientPrecache() const = 0;
    virtual void                        ServerPrecache() const = 0;
    virtual IGameEntity::CEntityConfig* GetEntityConfig() const = 0;
        
    virtual ICvar*                      GetGameSetting(const tstring &sSetting) const
    {
        return ICvar::GetCvar(m_sName + _T("_") + sSetting);
    }
};
//=============================================================================

//=============================================================================
// CEntityAllocator<T>
//=============================================================================
template <class T>
class CEntityAllocator : public IEntityAllocator
{
private:
    CEntityAllocator();

public:
    ~CEntityAllocator() {}
    CEntityAllocator(const tstring &sName, ushort unID) : IEntityAllocator(sName, unID)
    {
        IGameEntity *pEntBaseline(Allocate());

        m_pBaseline = K2_NEW(global,   CEntitySnapshot)();
        m_pBaseline->SetIndex(INVALID_INDEX);
        m_pBaseline->SetType(pEntBaseline->GetType());
        m_pBaseline->SetFieldTypes(GetTypeVector());
        m_pBaseline->SetAllFields();
        m_pBaseline->SetUniqueID(-1);
        m_pBaseline->SetPublicSequence(-1);
        m_pBaseline->SetPrivateSequence(-1);
        pEntBaseline->GetSnapshot(*m_pBaseline);

        K2_DELETE(pEntBaseline);
    }

    virtual IGameEntity*                Allocate() const;
    virtual const vector<SDataField>*   GetTypeVector() const   { return &T::GetTypeVector(); }
    virtual void                        ClientPrecache() const  { T::ClientPrecache(NULL); }
    virtual void                        ServerPrecache() const  { T::ServerPrecache(NULL); }
    virtual IGameEntity::CEntityConfig* GetEntityConfig() const { return NULL; }
};
//=============================================================================


/*====================
  CEntityAllocator::Allocate
  ====================*/
template <class T>
IGameEntity*    CEntityAllocator<T>::Allocate() const
{
    try
    {
        T* pNewEnt(K2_NEW(global,   T)());
        if (pNewEnt == NULL)
            EX_ERROR(_T("Allocation failed"));

        pNewEnt->SetTypeName(m_sName);
        pNewEnt->SetType(m_unID);
        pNewEnt->Baseline();
        return pNewEnt;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityAllocator::Allocate() - "), NO_THROW);
        return NULL;
    }
}


//=============================================================================
// CEntityAllocator2<T>
//=============================================================================
template <class T>
class CEntityAllocator2 : public IEntityAllocator
{
private:
    CEntityAllocator2();

public:
    ~CEntityAllocator2()    {}
    CEntityAllocator2(const tstring &sName, ushort unID) : IEntityAllocator(sName, unID)
    {
        IGameEntity *pEntBaseline(Allocate());

        m_pBaseline = K2_NEW(global,   CEntitySnapshot)();
        m_pBaseline->SetIndex(INVALID_INDEX);
        m_pBaseline->SetType(pEntBaseline->GetType());
        m_pBaseline->SetFieldTypes(GetTypeVector());
        m_pBaseline->SetAllFields();
        m_pBaseline->SetUniqueID(-1);
        m_pBaseline->SetPublicSequence(-1);
        m_pBaseline->SetPrivateSequence(-1);
        pEntBaseline->GetSnapshot(*m_pBaseline);

        K2_DELETE(pEntBaseline);
    }

    virtual IGameEntity*                Allocate() const;
    virtual const vector<SDataField>*   GetTypeVector() const   { return &T::GetTypeVector(); }
    virtual void                        ClientPrecache() const  { T::ClientPrecache(T::GetEntityConfig()); }
    virtual void                        ServerPrecache() const  { T::ServerPrecache(T::GetEntityConfig()); }
    virtual IGameEntity::CEntityConfig* GetEntityConfig() const { return T::GetEntityConfig(); }
};
//=============================================================================


/*====================
  CEntityAllocator2::Allocate
  ====================*/
template <class T>
IGameEntity*    CEntityAllocator2<T>::Allocate() const
{
    try
    {
        T* pNewEnt(K2_NEW(global,   T)());
        if (pNewEnt == NULL)
            EX_ERROR(_T("Allocation failed"));

        pNewEnt->SetTypeName(m_sName);
        pNewEnt->SetType(m_unID);
        pNewEnt->Baseline();
        return pNewEnt;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityAllocator::Allocate() - "), NO_THROW);
        return NULL;
    }
}


//=============================================================================
// CEntityRegistry
//=============================================================================
class CEntityRegistry
{
    SINGLETON_DEF(CEntityRegistry);

private:
    EntAllocatorIDMap   m_mapAllocatorIDs;
    EntAllocatorNameMap m_mapAllocatorNames;

    void    RegisterEntityType(const tstring &sName);

public:
    ~CEntityRegistry()  {}

    void                                Register(IEntityAllocator* pAllocator);

    GAME_SHARED_API ushort              LookupID(const tstring &sName);
    GAME_SHARED_API tstring             LookupName(ushort unID);
    
    GAME_SHARED_API const IEntityAllocator*     GetAllocator(ushort unID);
    uint                                GetNumTypes()   { return uint(m_mapAllocatorIDs.size()); }

    GAME_SHARED_API IGameEntity*        Allocate(ushort unID);
    GAME_SHARED_API IGameEntity*        Allocate(const tstring &sName);

    GAME_SHARED_API const vector<SDataField>*   GetTypeVector(ushort unType) const;
    GAME_SHARED_API const CEntitySnapshot*      GetBaseline(ushort unType) const;
    GAME_SHARED_API void                        ServerPrecache(ushort unType) const;
    GAME_SHARED_API void                        ClientPrecache(ushort unType) const;
    
    GAME_SHARED_API ICvar*                      GetGameSetting(ushort unType, const tstring &sSetting) const;
    GAME_SHARED_API float                       GetGameSettingFloat(ushort unType, const tstring &sSetting, float fDefault = 0.0f) const;
    GAME_SHARED_API tstring                     GetGameSettingString(ushort unType, const tstring &sSetting, const tstring &sDefault = SNULL) const;

    const EntAllocatorNameMap&  GetAllocatorNames()     { return m_mapAllocatorNames; }
};
//=============================================================================

#endif //__C_ENTITYREGISTRY_H__
