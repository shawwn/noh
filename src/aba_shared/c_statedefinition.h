// (C)2008 S2 Games
// c_statedefinition.h
//
//=============================================================================
#ifndef __C_STATEDEFINITION_H__
#define __C_STATEDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_slavedefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IEntityState, State, state)
//=============================================================================

//=============================================================================
// CStateDefinition
//=============================================================================
class CStateDefinition : public ISlaveDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_ARRAY_PROPERTY(IsHidden, bool)
    ENT_DEF_ARRAY_PROPERTY(DispelOnDamage, bool)
    ENT_DEF_ARRAY_PROPERTY(DispelOnAction, bool)
    ENT_DEF_ARRAY_PROPERTY(DisplayLevel, bool)
    ENT_DEF_ARRAY_PROPERTY(ImpactInterval, uint)
    ENT_DEF_ARRAY_PROPERTY(MorphPriority, uint)
    ENT_DEF_ARRAY_PROPERTY(NoRefresh, bool)
    ENT_DEF_ARRAY_PROPERTY(PropagateToIllusions, bool)
    ENT_DEF_ARRAY_PROPERTY(DeathPersist, bool)

    ENT_DEF_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY(Armor, float)
    ENT_DEF_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeedSlow, float)
    ENT_DEF_TEMPORAL_PROGRESSIVE_ARRAY_PROPERTY(MagicArmor, float)

    ENT_DEF_ARRAY_MUTATION(Strength, float)
    ENT_DEF_ARRAY_MUTATION(Agility, float)
    ENT_DEF_ARRAY_MUTATION(Intelligence, float)
    ENT_DEF_ARRAY_MUTATION(MaxHealth, float)
    ENT_DEF_ARRAY_MUTATION(HealthRegen, float)
    ENT_DEF_ARRAY_MUTATION(HealthProportionRegen, float)
    ENT_DEF_ARRAY_MUTATION(MaxMana, float)
    ENT_DEF_ARRAY_MUTATION(ManaRegen, float)
    ENT_DEF_ARRAY_MUTATION(ManaProportionRegen, float)
    ENT_DEF_ARRAY_MUTATION(MaxStamina, float)
    ENT_DEF_ARRAY_MUTATION(StaminaRegen, float)
    ENT_DEF_ARRAY_MUTATION(StaminaProportionRegen, float)
    ENT_DEF_ARRAY_MUTATION(Armor, float)
    ENT_DEF_ARRAY_MUTATION(MagicArmor, float)
    ENT_DEF_ARRAY_MUTATION(MoveSpeed, float)
    ENT_DEF_ARRAY_MUTATION(SlowResistance, float)
    ENT_DEF_ARRAY_MUTATION(AttackRange, float)
    ENT_DEF_ARRAY_MUTATION(LifeSteal, float)
    ENT_DEF_ARRAY_MUTATION(RevealRange, float)
    ENT_DEF_ARRAY_MUTATION(HealthRegenReduction, float)
    ENT_DEF_ARRAY_MUTATION(ManaRegenReduction, float)
    ENT_DEF_ARRAY_MUTATION(StaminaRegenReduction, float)
    ENT_DEF_STRING_VECTOR_ARRAY_MUTATION(UnitType)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(Icon, Icon)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(Portrait, Icon)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(Model, Model)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(MapIconProperty, Icon)
    ENT_DEF_ARRAY_MUTATION(MapIconColorProperty, CVec4f)
    ENT_DEF_ARRAY_MUTATION(MapIconSizeProperty, float)
    ENT_DEF_STRING_ARRAY_MUTATION(Skin)
    ENT_DEF_STRING_ARRAY_MUTATION(AttackProjectile)
    ENT_DEF_ARRAY_MUTATION_EX(AttackType, uint, Game.LookupAttackType)
    ENT_DEF_ARRAY_MUTATION_EX(AttackEffectType, uint, Game.LookupEffectType)
    ENT_DEF_ARRAY_MUTATION_EX(AttackDamageType, uint, Game.LookupEffectType)
    ENT_DEF_ARRAY_MUTATION(AttackCooldown, uint)
    ENT_DEF_ARRAY_MUTATION(AttackDuration, uint)
    ENT_DEF_ARRAY_MUTATION(AttackActionTime, uint)
    ENT_DEF_ARRAY_MUTATION_EX(AttackTargetScheme, uint, Game.LookupTargetScheme)
    ENT_DEF_ARRAY_MUTATION(AttackNonLethal, bool)
    ENT_DEF_ARRAY_MUTATION_EX(ThreatEffectType, uint, Game.LookupEffectType)
    ENT_DEF_ARRAY_MUTATION_EX(ThreatScheme, uint, Game.LookupTargetScheme)
    ENT_DEF_ARRAY_MUTATION(AggroRange, float)
    ENT_DEF_ARRAY_MUTATION_EX(AggroScheme, uint, Game.LookupTargetScheme)
    ENT_DEF_ARRAY_MUTATION(ProximityRange, float)
    ENT_DEF_ARRAY_MUTATION_EX(ProximityScheme, uint, Game.LookupTargetScheme)
    ENT_DEF_ARRAY_MUTATION(PreGlobalScale, float)
    ENT_DEF_ARRAY_MUTATION(EffectScale, float)
    ENT_DEF_ARRAY_MUTATION(ModelScale, float)
    ENT_DEF_ARRAY_MUTATION(InfoHeight, float)
    ENT_DEF_ARRAY_MUTATION(BoundsHeight, float)
    ENT_DEF_ARRAY_MUTATION(BoundsRadius, float)
    ENT_DEF_ARRAY_MUTATION(SelectionRadius, float)
    ENT_DEF_MUTATION(IsSelectable, bool)
    ENT_DEF_MUTATION(NoCorpse, bool)
    ENT_DEF_ARRAY_MUTATION(AttackRangeBuffer, float)
    ENT_DEF_ARRAY_MUTATION(AttackOffset, CVec3f)
    ENT_DEF_ARRAY_MUTATION(TargetOffset, CVec3f)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(AttackStartEffect, Effect)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(AttackActionEffect, Effect)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(AttackImpactEffect, Effect)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(DeathEffect, Effect)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(PassiveEffect, Effect)
    ENT_DEF_RESOURCE_ARRAY_MUTATION(SpawnEffect, Effect)
    ENT_DEF_ARRAY_MUTATION(SightRangeDay, float)
    ENT_DEF_ARRAY_MUTATION(SightRangeNight, float)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory0)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory1)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory2)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory3)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory4)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory5)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory6)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory7)
    ENT_DEF_STRING_ARRAY_MUTATION(Inventory8)
    ENT_DEF_STRING_ARRAY_MUTATION(SharedInventory0)
    ENT_DEF_STRING_ARRAY_MUTATION(SharedInventory1)
    ENT_DEF_STRING_ARRAY_MUTATION(SharedInventory2)
    ENT_DEF_ARRAY_MUTATION(Invulnerable, bool)
    ENT_DEF_ARRAY_MUTATION(ClearVision, bool)
    ENT_DEF_ARRAY_MUTATION(Deniable, bool)
    ENT_DEF_ARRAY_MUTATION(DeniablePercent, float)
    ENT_DEF_ARRAY_MUTATION(Smackable, bool)
    ENT_DEF_ARRAY_MUTATION(NoThreat, bool)
    ENT_DEF_ARRAY_MUTATION(Unitwalking, bool)
    ENT_DEF_ARRAY_MUTATION(Treewalking, bool)
    ENT_DEF_ARRAY_MUTATION(Cliffwalking, bool)
    ENT_DEF_ARRAY_MUTATION(Buildingwalking, bool)
    ENT_DEF_ARRAY_MUTATION(Antiwalking, bool)
    ENT_DEF_STRING_ARRAY_MUTATION(ShopAccess)
    ENT_DEF_STRING_ARRAY_MUTATION(RemoteShopAccess)
    ENT_DEF_STRING_ARRAY_MUTATION(SharedShopAccess)
    ENT_DEF_STRING_ARRAY_MUTATION(SharedRemoteShopAccess)
    ENT_DEF_STRING_ARRAY_MUTATION(RestrictItemAccess)
    ENT_DEF_ARRAY_MUTATION(StashAccess, bool)
    ENT_DEF_ARRAY_MUTATION(PreventAggro, bool)
    ENT_DEF_ARRAY_MUTATION(TrueStrike, bool)
    ENT_DEF_ARRAY_MUTATION(DefaultBehavior, EUnitCommand)

    ENT_DEF_ARRAY_MUTATION(Power, float)

public:
    ~CStateDefinition() {}
    CStateDefinition() :
    ISlaveDefinition(&g_allocatorState)
    {}

    IEntityDefinition*  GetCopy() const { return K2_NEW(g_heapResources,    CStateDefinition)(*this); }

    virtual void    Precache(EPrecacheScheme eScheme)
    {
        ISlaveDefinition::Precache(eScheme);

        PRECACHE_GUARD
            PrecacheMorphModel();
            PrecacheMorphAttackStartEffect();
            PrecacheMorphAttackActionEffect();
            PrecacheMorphAttackImpactEffect();
            PrecacheMorphDeathEffect();
            PrecacheMorphPassiveEffect();
            PrecacheMorphSpawnEffect();
            PRECACHE_ENTITY_ARRAY(MorphAttackProjectile, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory0, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory1, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory2, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory3, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory4, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory5, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory6, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory7, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphInventory8, eScheme)
            PRECACHE_ENTITY_ARRAY(MorphSharedInventory0, eScheme);
            PRECACHE_ENTITY_ARRAY(MorphSharedInventory1, eScheme);
            PRECACHE_ENTITY_ARRAY(MorphSharedInventory2, eScheme);
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
    {
        ISlaveDefinition::GetPrecacheList(eScheme, deqPrecache);

        PRECACHE_GUARD
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphAttackProjectile, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory0, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory1, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory2, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory3, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory4, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory5, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory6, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory7, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphInventory8, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphSharedInventory0, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphSharedInventory1, eScheme, deqPrecache)
            GET_ENTITY_ARRAY_PRECACHE_LIST(MorphSharedInventory2, eScheme, deqPrecache)

            deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
        PRECACHE_GUARD_END
    }

    virtual void    ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_STATEDEFINITION_H__
