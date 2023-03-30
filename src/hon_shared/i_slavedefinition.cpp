// (C)2008 S2 Games
// i_slavedefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_slavedefinition.h"

#include "../k2/c_xmlnode.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(ISlaveDefinition, ENTITY_BASE_TYPE_SLAVE, Slave)

/*====================
  ISlaveDefinition::ReadSettings
  ====================*/
void    ISlaveDefinition::ReadSettings(ISlaveDefinition *pDefinition, const CXMLNode &node, bool bMod)
{
    READ_ENTITY_DEFINITION_PROPERTY(UnitType, unittype)

    pDefinition->SetDisplayNamePriority(pDefinition->GetPriority());
    pDefinition->SetDescriptionPriority(pDefinition->GetPriority());
    pDefinition->SetDescription2Priority(pDefinition->GetPriority());

    READ_ENTITY_DEFINITION_PROPERTY(Icon, icon)
    READ_ENTITY_DEFINITION_PROPERTY(MapIcon, mapicon)
    READ_ENTITY_DEFINITION_PROPERTY(MapIconColor, mapiconcolor)
    READ_ENTITY_DEFINITION_PROPERTY(PassiveEffect, passiveeffect)
    READ_ENTITY_DEFINITION_PROPERTY(ForceAnim, forceanim)
    READ_ENTITY_DEFINITION_PROPERTY(Singleton, Singleton)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Strength, strength)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Agility, agility)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Intelligence, intelligence)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxHealth, maxhealth)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxHealthMultiplier, maxhealthmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(HealthRegen, healthregen)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(HealthRegenMultiplier, healthregenmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(HealthRegenPercent, healthregenpercent)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxMana, maxmana)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxManaMultiplier, maxmanamultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(ManaRegen, manaregen)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(ManaRegenMultiplier, manaregenmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(ManaRegenPercent, manaregenpercent)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Armor, armor)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MagicArmor, magicarmor)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Deflection, deflection)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(DeflectionChance, deflectionchance)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(IncomingDamageMultiplier, incomingdamagemultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(DebuffDurationMultiplier, debuffdurationmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(HealMultiplier, healmultiplier)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackSpeed, attackspeed)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackSpeedMultiplier, attackspeedmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackSpeedSlow, attackspeedslow)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(CastSpeed, castspeed)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(CooldownSpeed, cooldownspeed)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(ReducedCooldowns, reducedcooldowns)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(IncreasedCooldowns, increasedcooldowns)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MoveSpeed, movespeed)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MoveSpeedMultiplier, movespeedmultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(SlowResistance, slowresistance)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MoveSpeedSlow, movespeedslow)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Damage, damage)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(BaseDamageMultiplier, basedamagemultiplier)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(TotalDamageMultiplier, totaldamagemultiplier)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackRange, attackrange)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackRangeMultiplier, attackrangemultiplier)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(LifeSteal, lifesteal)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(CriticalChance, criticalchance)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(CriticalMultiplier, criticalmultiplier)
    READ_SPLIT_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Evasion, Ranged, Melee, evasion, ranged, melee)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MissChance, misschance)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(HealthRegenReduction, healthregenreduction)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(ManaRegenReduction, manaregenreduction)

    READ_ENTITY_DEFINITION_PROPERTY(Stunned, stunned)
    READ_ENTITY_DEFINITION_PROPERTY(Silenced, silenced)
    READ_ENTITY_DEFINITION_PROPERTY(Perplexed, perplexed)
    READ_ENTITY_DEFINITION_PROPERTY(Disarmed, disarmed)
    READ_ENTITY_DEFINITION_PROPERTY(Immobilized, immobilized)
    READ_ENTITY_DEFINITION_PROPERTY(Immobilized2, immobilized2)
    READ_ENTITY_DEFINITION_PROPERTY(Restrained, restrained)
    READ_ENTITY_DEFINITION_PROPERTY(Invulnerable, invulnerable)
    READ_ENTITY_DEFINITION_PROPERTY(Revealed, revealed)
    READ_ENTITY_DEFINITION_PROPERTY(Frozen, frozen)
    READ_ENTITY_DEFINITION_PROPERTY(Isolated, isolated)
    READ_ENTITY_DEFINITION_PROPERTY(FreeCast, freecast)
    READ_ENTITY_DEFINITION_PROPERTY(ClearVision, clearvision)
    READ_ENTITY_DEFINITION_PROPERTY(Deniable, deniable)
    READ_ENTITY_DEFINITION_PROPERTY(DeniablePercent, deniablepercent)
    READ_ENTITY_DEFINITION_PROPERTY(Smackable, smackable)
    READ_ENTITY_DEFINITION_PROPERTY(NoThreat, nothreat)
    READ_ENTITY_DEFINITION_PROPERTY(TrueStrike, truestrike)
    READ_ENTITY_DEFINITION_PROPERTY(Sighted, sighted)
    READ_ENTITY_DEFINITION_PROPERTY(AlwaysTransmitData, alwaystransmitdata)

    READ_ENTITY_DEFINITION_PROPERTY(RevealType, revealtype)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(RevealRange, revealrange)
    READ_ENTITY_DEFINITION_PROPERTY(StealthType, stealthtype)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(FadeTime, fadetime)
    READ_ENTITY_DEFINITION_PROPERTY(StealthProximity, stealthproximity)

    READ_ENTITY_DEFINITION_PROPERTY(ImmunityType, immunitytype)

    READ_ENTITY_DEFINITION_PROPERTY(EffectType, effecttype)

    READ_ENTITY_DEFINITION_PROPERTY(Invulnerable, invulnerable)

    READ_ENTITY_DEFINITION_PROPERTY(Unitwalking, unitwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Treewalking, treewalking)
    READ_ENTITY_DEFINITION_PROPERTY(Cliffwalking, cliffwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Buildingwalking, buildingwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Antiwalking, antiwalking)

    READ_ENTITY_DEFINITION_PROPERTY(ShopAccess, shopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(RemoteShopAccess, remoteshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(SharedShopAccess, sharedshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(SharedRemoteShopAccess, sharedremoteshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(RestrictItemAccess, restrictitemaccess)
    READ_ENTITY_DEFINITION_PROPERTY(StashAccess, stashaccess)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY_EX(MaxCharges, maxcharges, -1, 0, 0)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Counter, counter)

    READ_ENTITY_DEFINITION_PROPERTY(Shield, shield)
    READ_ENTITY_DEFINITION_PROPERTY(MaxAccumulator, maxaccumulator)

    READ_ENTITY_DEFINITION_PROPERTY(DefaultActiveModifierKey, defaultactivemodifierkey)
    READ_ENTITY_DEFINITION_PROPERTY(ModifierKey, modifierkey)
    READ_ENTITY_DEFINITION_PROPERTY(ModifierKey2, modifierkey2)
}

ENTITY_DEF_MERGE_START(ISlaveDefinition, IEntityDefinition)
    MERGE_STRING_VECTOR_ARRAY_PROPERTY(UnitType)

    MERGE_LOCALIZED_STRING_PROPERTY(DisplayName)
    MERGE_LOCALIZED_STRING_PROPERTY(Description)
    MERGE_LOCALIZED_STRING_PROPERTY(Description2)

    MERGE_RESOURCE_ARRAY_PROPERTY(Icon)
    MERGE_RESOURCE_ARRAY_PROPERTY(MapIcon)
    MERGE_STRING_ARRAY_PROPERTY(MapIconColor)
    MERGE_RESOURCE_ARRAY_PROPERTY(PassiveEffect)
    MERGE_STRING_ARRAY_PROPERTY(ForceAnim)
    MERGE_PROPERTY(Singleton)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Strength)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Agility)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Intelligence)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxHealth)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxHealthMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(HealthRegen)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenPercent)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxMana)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxManaMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(ManaRegen)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenPercent)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Armor)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MagicArmor)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Deflection)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(DeflectionChance)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(IncomingDamageMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(DebuffDurationMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(HealMultiplier)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeed)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeedMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackSpeedSlow)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(CastSpeed)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(CooldownSpeed)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(ReducedCooldowns)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(IncreasedCooldowns)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeed)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeedMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(SlowResistance)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MoveSpeedSlow)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Damage)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(BaseDamageMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(TotalDamageMultiplier)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackRange)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackRangeMultiplier)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(LifeSteal)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(CriticalChance)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(CriticalMultiplier)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(EvasionMelee)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(EvasionRanged)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MissChance)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(HealthRegenReduction)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(ManaRegenReduction)

    MERGE_ARRAY_PROPERTY(Stunned)
    MERGE_ARRAY_PROPERTY(Silenced)
    MERGE_ARRAY_PROPERTY(Perplexed)
    MERGE_ARRAY_PROPERTY(Disarmed)
    MERGE_ARRAY_PROPERTY(Immobilized)
    MERGE_ARRAY_PROPERTY(Immobilized2)
    MERGE_ARRAY_PROPERTY(Restrained)
    MERGE_ARRAY_PROPERTY(Invulnerable)
    MERGE_ARRAY_PROPERTY(Revealed)
    MERGE_ARRAY_PROPERTY(Frozen)
    MERGE_ARRAY_PROPERTY(Isolated)
    MERGE_ARRAY_PROPERTY(FreeCast)
    MERGE_ARRAY_PROPERTY(ClearVision)
    MERGE_ARRAY_PROPERTY(Deniable)
    MERGE_ARRAY_PROPERTY(DeniablePercent)
    MERGE_ARRAY_PROPERTY(Smackable)
    MERGE_ARRAY_PROPERTY(NoThreat)
    MERGE_ARRAY_PROPERTY(TrueStrike)
    MERGE_ARRAY_PROPERTY(Sighted)
    MERGE_ARRAY_PROPERTY(AlwaysTransmitData)

    MERGE_ARRAY_PROPERTY(RevealType)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(RevealRange)
    MERGE_ARRAY_PROPERTY(StealthType)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(FadeTime)

    MERGE_ARRAY_PROPERTY(ImmunityType)

    MERGE_ARRAY_PROPERTY(EffectType)

    MERGE_ARRAY_PROPERTY(Invulnerable)

    MERGE_ARRAY_PROPERTY(Unitwalking)
    MERGE_ARRAY_PROPERTY(Treewalking)
    MERGE_ARRAY_PROPERTY(Cliffwalking)
    MERGE_ARRAY_PROPERTY(Buildingwalking)
    MERGE_ARRAY_PROPERTY(Antiwalking)

    MERGE_STRING_ARRAY_PROPERTY(ShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(RemoteShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(SharedShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(SharedRemoteShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(RestrictItemAccess)
    MERGE_ARRAY_PROPERTY(StashAccess)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxCharges)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Counter)

    MERGE_STRING_ARRAY_PROPERTY(DefaultActiveModifierKey)
    MERGE_ARRAY_PROPERTY(ModifierKey)
    MERGE_ARRAY_PROPERTY(ModifierKey2)
ENTITY_DEF_MERGE_END
