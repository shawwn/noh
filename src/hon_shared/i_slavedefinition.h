// (C)2008 S2 Games
// i_slavedefinition.h
//
//=============================================================================
#ifndef __I_SLAVEDEFINITION_H__
#define __I_SLAVEDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"
//=============================================================================

//=============================================================================
// ISlaveDefinition
//=============================================================================
class ISlaveDefinition : public IEntityDefinition
{
    DECLARE_DEFINITION_TYPE_INFO

    ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY(UnitType)

    ENT_DEF_LOCALIZED_STRING_PROPERTY(DisplayName)
    ENT_DEF_LOCALIZED_STRING_PROPERTY(Description)
    ENT_DEF_LOCALIZED_STRING_PROPERTY(Description2)
    ENT_DEF_RESOURCE_ARRAY_PROPERTY(Icon, Icon)
    ENT_DEF_RESOURCE_ARRAY_PROPERTY(MapIcon, Icon)
    ENT_DEF_STRING_ARRAY_PROPERTY(MapIconColor)
    ENT_DEF_RESOURCE_ARRAY_PROPERTY(PassiveEffect, Effect)
    ENT_DEF_STRING_ARRAY_PROPERTY(ForceAnim)
    ENT_DEF_PROPERTY(Singleton, bool)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Strength, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Agility, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Intelligence, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxHealth, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxHealthMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthRegen, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenPercent, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxMana, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxManaMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaRegen, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenPercent, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Armor, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MagicArmor, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Deflection, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(DeflectionChance, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(IncomingDamageMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(DebuffDurationMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealMultiplier, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeed, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeedMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeedSlow, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(CastSpeed, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(CooldownSpeed, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ReducedCooldowns, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(IncreasedCooldowns, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Damage, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(BaseDamageMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(TotalDamageMultiplier, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeed, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(SlowResistance, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeedMultiplier, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeedSlow, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackRange, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackRangeMultiplier, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(LifeSteal, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(CriticalChance, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(CriticalMultiplier, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(EvasionRanged, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(EvasionMelee, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MissChance, float)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenReduction, float)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenReduction, float)
    
    ENT_DEF_ARRAY_PROPERTY(Stunned, bool)
    ENT_DEF_ARRAY_PROPERTY(Silenced, bool)
    ENT_DEF_ARRAY_PROPERTY(Perplexed, bool)
    ENT_DEF_ARRAY_PROPERTY(Disarmed, bool)
    ENT_DEF_ARRAY_PROPERTY(Immobilized, bool)
    ENT_DEF_ARRAY_PROPERTY(Immobilized2, bool)
    ENT_DEF_ARRAY_PROPERTY(Restrained, bool)
    ENT_DEF_ARRAY_PROPERTY(Invulnerable, bool)
    ENT_DEF_ARRAY_PROPERTY(Revealed, bool)
    ENT_DEF_ARRAY_PROPERTY(Frozen, bool)
    ENT_DEF_ARRAY_PROPERTY(Isolated, bool)
    ENT_DEF_ARRAY_PROPERTY(FreeCast, bool)
    ENT_DEF_ARRAY_PROPERTY(ClearVision, bool)
    ENT_DEF_ARRAY_PROPERTY(Deniable, bool)
    ENT_DEF_ARRAY_PROPERTY(DeniablePercent, float)
    ENT_DEF_ARRAY_PROPERTY(Smackable, bool)
    ENT_DEF_ARRAY_PROPERTY(NoThreat, bool)
    ENT_DEF_ARRAY_PROPERTY(TrueStrike, bool)
    ENT_DEF_ARRAY_PROPERTY(Sighted, bool)
    ENT_DEF_ARRAY_PROPERTY(AlwaysTransmitData, bool)
    
    ENT_DEF_ARRAY_PROPERTY_EX(RevealType, uint, Game.LookupRevealType)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(RevealRange, float)
    ENT_DEF_ARRAY_PROPERTY_EX(StealthType, uint, Game.LookupStealthType)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(FadeTime, uint)
    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(StealthProximity, float)

    ENT_DEF_ARRAY_PROPERTY_EX(ImmunityType, uint, Game.LookupImmunityType)
    
    ENT_DEF_ARRAY_PROPERTY_EX(EffectType, uint, Game.LookupEffectType)

    ENT_DEF_ARRAY_PROPERTY(Unitwalking, bool)
    ENT_DEF_ARRAY_PROPERTY(Treewalking, bool)
    ENT_DEF_ARRAY_PROPERTY(Cliffwalking, bool)
    ENT_DEF_ARRAY_PROPERTY(Buildingwalking, bool)
    ENT_DEF_ARRAY_PROPERTY(Antiwalking, bool)

    ENT_DEF_STRING_ARRAY_PROPERTY(ShopAccess)
    ENT_DEF_STRING_ARRAY_PROPERTY(RemoteShopAccess)
    ENT_DEF_STRING_ARRAY_PROPERTY(SharedShopAccess)
    ENT_DEF_STRING_ARRAY_PROPERTY(SharedRemoteShopAccess)
    ENT_DEF_STRING_ARRAY_PROPERTY(RestrictItemAccess)
    ENT_DEF_ARRAY_PROPERTY(StashAccess, bool)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxCharges, int)

    ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Counter, float)

    ENT_DEF_ARRAY_PROPERTY(Shield, bool)
    ENT_DEF_ARRAY_PROPERTY(MaxAccumulator, float)

    ENT_DEF_STRING_ARRAY_PROPERTY(DefaultActiveModifierKey)
    ENT_DEF_ARRAY_PROPERTY_EX(ModifierKey, uint, EntityRegistry.RegisterModifier)
    ENT_DEF_ARRAY_PROPERTY_EX(ModifierKey2, uint, EntityRegistry.RegisterModifier)

private:
    ISlaveDefinition();

protected:
    virtual void    PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
    {
        IEntityDefinition::PrecacheV(eScheme, sModifier);

        PRECACHE_GUARD
            PrecacheIcon();
            PrecachePassiveEffect();
        PRECACHE_GUARD_END
    }

    virtual void    GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
    {
        IEntityDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);
    }

public:
    ~ISlaveDefinition() {}
    ISlaveDefinition(IBaseEntityAllocator *pAllocator) :
    IEntityDefinition(pAllocator)
    {}

    static void     ReadSettings(ISlaveDefinition *pDefinition, const class CXMLNode &node, bool bMod);

    virtual void    PostProcess()
    {
        if (m_bPostProcessing)
            return;

        m_bPostProcessing = true;

        PRECACHE_LOCALIZED_STRING(DisplayName, name);
        PRECACHE_LOCALIZED_STRING(Description, description);
        PRECACHE_LOCALIZED_STRING(Description2, description2);

        m_bPostProcessing = false;

        IEntityDefinition::PostProcess();
    }

    virtual void    ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__I_SLAVEDEFINITION_H__
