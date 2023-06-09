// (C)2008 S2 Games
// i_unitdefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_unitdefinition.h"

#include "../k2/c_xmlnode.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(IUnitDefinition, ENTITY_BASE_TYPE_UNIT, Unit)

/*====================
  IUnitDefinition::ReadSettings
  ====================*/
void    IUnitDefinition::ReadSettings(IUnitDefinition *pDefinition, const CXMLNode &node, bool bMod)
{
    READ_ENTITY_DEFINITION_PROPERTY(UnitType, unittype)

    pDefinition->SetDisplayNamePriority(pDefinition->GetPriority());
    pDefinition->SetDescriptionPriority(pDefinition->GetPriority());

    READ_ENTITY_DEFINITION_PROPERTY(Icon, icon)
    READ_ENTITY_DEFINITION_PROPERTY(Portrait, portrait)

    READ_ENTITY_DEFINITION_PROPERTY(MapIconProperty, mapicon)
    READ_ENTITY_DEFINITION_PROPERTY(MapIconColorProperty, mapiconcolor)
    READ_ENTITY_DEFINITION_PROPERTY(MapIconSizeProperty, mapiconsize)
    
    READ_ENTITY_DEFINITION_PROPERTY(Model, model)
    READ_ENTITY_DEFINITION_PROPERTY(Skin, skin)
    READ_ENTITY_DEFINITION_PROPERTY_EX(DrawOnMap, drawonmap, true)
    READ_ENTITY_DEFINITION_PROPERTY_EX(HoverOnMap, hoveronmap, true)
    READ_ENTITY_DEFINITION_PROPERTY(PartialControlShare, partialcontrolshare)
    READ_ENTITY_DEFINITION_PROPERTY_EX(IdleAnim, idleanim, idle)
    READ_ENTITY_DEFINITION_PROPERTY_EX(WalkAnim, walkanim, walk_1)
    READ_ENTITY_DEFINITION_PROPERTY(DeniedAnim, deniedanim)
    READ_ENTITY_DEFINITION_PROPERTY(GibAnim, gibanim)

    READ_ENTITY_DEFINITION_PROPERTY(PassiveEffect, passiveeffect)
    READ_ENTITY_DEFINITION_PROPERTY(SpawnEffect, spawneffect)
    READ_ENTITY_DEFINITION_PROPERTY(SelectedSound, selectedsound)
    READ_ENTITY_DEFINITION_PROPERTY(SelectedFlavorSound, selectedflavorsound)
    READ_ENTITY_DEFINITION_PROPERTY(ConfirmMoveSound, confirmmovesound)
    READ_ENTITY_DEFINITION_PROPERTY(ConfirmAttackSound, confirmattacksound)
    READ_ENTITY_DEFINITION_PROPERTY(TauntedSound, tauntedsound)
    READ_ENTITY_DEFINITION_PROPERTY(TauntKillSound, tauntkillsound)
    READ_ENTITY_DEFINITION_PROPERTY(NoManaSound, nomanasound)
    READ_ENTITY_DEFINITION_PROPERTY(CooldownSound, cooldownsound)

    READ_ENTITY_DEFINITION_PROPERTY_EX(PreGlobalScale, preglobalscale, 1.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(EffectScale, effectscale, 1.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ModelScale, modelscale, 1.0)
    READ_ENTITY_DEFINITION_PROPERTY(InfoHeight, infoheight)

    READ_ENTITY_DEFINITION_PROPERTY(BoundsRadius, boundsradius)
    READ_ENTITY_DEFINITION_PROPERTY(BoundsHeight, boundsheight)
    READ_ENTITY_DEFINITION_PROPERTY(SelectionRadius, selectionradius)
    READ_ENTITY_DEFINITION_PROPERTY_EX(IsSelectable, isselectable, true)
    READ_ENTITY_DEFINITION_PROPERTY(NoCorpse, nocorpse)
    READ_ENTITY_DEFINITION_PROPERTY_EX(IsControllable, iscontrollable, true)
    READ_ENTITY_DEFINITION_PROPERTY_EX(IsUnit, isunit, true)
    READ_ENTITY_DEFINITION_PROPERTY(NoGlobalSelect, noglobalselect)
    READ_ENTITY_DEFINITION_PROPERTY(NoBlockNeutralSpawn, noblockneutralspawn)
    READ_ENTITY_DEFINITION_PROPERTY(TargetOffset, targetoffset)

    //READ_ENTITY_DEFINITION_PROPERTY(IsMobile, ismobile)
    //READ_ENTITY_DEFINITION_PROPERTY(CanATtack, canattack)
    //READ_ENTITY_DEFINITION_PROPERTY(CanRotate, canrotate)
    READ_ENTITY_DEFINITION_PROPERTY(MoveSpeed, movespeed)
    READ_ENTITY_DEFINITION_PROPERTY(SlowResistance, slowresistance)
    READ_ENTITY_DEFINITION_PROPERTY(TurnRate, turnrate)
    READ_ENTITY_DEFINITION_PROPERTY(TurnSmoothing, turnsmoothing)
    READ_ENTITY_DEFINITION_PROPERTY(Blocking, blocking)
    READ_ENTITY_DEFINITION_PROPERTY(AntiBlocking, antiblocking)

    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxHealth, maxhealth)
    READ_ENTITY_DEFINITION_PROPERTY(HealthRegen, healthregen)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(MaxMana, maxmana)
    READ_ENTITY_DEFINITION_PROPERTY(ManaRegen, manaregen)
    
    //READ_ENTITY_DEFINITION_PROPERTY(CombatType, combattype)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ArmorType, armortype, normal)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(Armor, armor)
    READ_ENTITY_DEFINITION_PROPERTY_EX(MagicArmorType, magicarmortype, magic)
    READ_ENTITY_DEFINITION_PROPERTY(MagicArmor, magicarmor)
    READ_ENTITY_DEFINITION_PROPERTY(ImmunityType, immunity)

    READ_ENTITY_DEFINITION_PROPERTY(LifeSteal, lifesteal)
    READ_ENTITY_DEFINITION_PROPERTY(HealthRegenReduction, healthregenreduction)
    READ_ENTITY_DEFINITION_PROPERTY(ManaRegenReduction, manaregenreduction)

    //READ_ENTITY_DEFINITION_PROPERTY(CanCarryItems, cancarryitems)
    READ_ENTITY_DEFINITION_PROPERTY(DropItemsOnDeath, dropitemsondeath)
    READ_ENTITY_DEFINITION_PROPERTY(PassiveInventory, passiveinventory)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory0, inventory0)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory1, inventory1)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory2, inventory2)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory3, inventory3)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory4, inventory4)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory5, inventory5)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory6, inventory6)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory7, inventory7)
    READ_ENTITY_DEFINITION_PROPERTY(Inventory8, inventory8)
    READ_ENTITY_DEFINITION_PROPERTY(SharedInventory0, sharedinventory0)
    READ_ENTITY_DEFINITION_PROPERTY(SharedInventory1, sharedinventory1)
    READ_ENTITY_DEFINITION_PROPERTY(SharedInventory2, sharedinventory2)
    
    READ_ENTITY_DEFINITION_PROPERTY(AttackType, attacktype)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackEffectType, attackeffecttype, Attack Physical)
    READ_ENTITY_DEFINITION_PROPERTY(AttackRange, attackrange)
    READ_ENTITY_DEFINITION_PROPERTY(AttackProjectile, attackprojectile)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackRangeBuffer, attackrangebuffer, 250.0)
    READ_ENTITY_DEFINITION_PROPERTY(AttackOffset, attackoffset)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackDamageType, attackdamagetype, Physical)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackDamageMin, attackdamagemin)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(AttackDamageMax, attackdamagemax)
    READ_ENTITY_DEFINITION_PROPERTY(AttackCooldown, attackcooldown)
    READ_ENTITY_DEFINITION_PROPERTY(AttackDuration, attackduration)
    READ_ENTITY_DEFINITION_PROPERTY(AttackActionTime, attackactiontime)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackTargetScheme, attacktargetscheme, attack)
    READ_ENTITY_DEFINITION_PROPERTY(AttackNonLethal, attacknonlethal)
    READ_ENTITY_DEFINITION_PROPERTY(AttackStartEffect, attackstarteffect)
    READ_ENTITY_DEFINITION_PROPERTY(AttackActionEffect, attackactioneffect)
    READ_ENTITY_DEFINITION_PROPERTY(AttackImpactEffect, attackimpacteffect)
    
    READ_ENTITY_DEFINITION_PROPERTY(ThreatScheme, threatscheme)
    READ_ENTITY_DEFINITION_PROPERTY(ThreatEffectType, threateffecttype)
    READ_ENTITY_DEFINITION_PROPERTY(AggroRange, aggrorange)
    READ_ENTITY_DEFINITION_PROPERTY(AggroScheme, aggroscheme)
    READ_ENTITY_DEFINITION_PROPERTY(ProximityRange, proximityrange)
    READ_ENTITY_DEFINITION_PROPERTY(ProximityScheme, proximityscheme)
    READ_ENTITY_DEFINITION_PROPERTY(SightRangeDay, sightrangeday)
    READ_ENTITY_DEFINITION_PROPERTY(SightRangeNight, sightrangenight)
    READ_ENTITY_DEFINITION_PROPERTY(WanderRange, wanderrange)
    READ_ENTITY_DEFINITION_PROPERTY(RevealRange, revealrange)
    READ_ENTITY_DEFINITION_PROPERTY(RevealType, revealtype)
    READ_ENTITY_DEFINITION_PROPERTY(StealthType, stealthtype)
    READ_ENTITY_DEFINITION_PROGRESSIVE_PROPERTY(FadeTime, fadetime)
    READ_ENTITY_DEFINITION_PROPERTY(StealthProximity, stealthproximity)
    READ_ENTITY_DEFINITION_PROPERTY(DefaultBehavior, defaultbehavior)

    READ_ENTITY_DEFINITION_PROPERTY(GoldBountyMin, goldbountymin)
    READ_ENTITY_DEFINITION_PROPERTY(GoldBountyMax, goldbountymax)
    READ_ENTITY_DEFINITION_PROPERTY(GoldBountyTeam, goldbountyteam)
    READ_ENTITY_DEFINITION_PROPERTY(GoldBountyConsolation, goldbountyconsolation)
    READ_ENTITY_DEFINITION_PROPERTY(ExperienceBounty, experiencebounty)
    READ_ENTITY_DEFINITION_PROPERTY(GlobalExperience, globalexperience)
    READ_ENTITY_DEFINITION_PROPERTY(DeadExperience, deadexperience)
    READ_ENTITY_DEFINITION_PROPERTY(UnsharedExperienceBounty, unsharedexperiencebounty)

    READ_ENTITY_DEFINITION_PROPERTY_EX(CorpseTime, corpsetime, 10000)
    READ_ENTITY_DEFINITION_PROPERTY_EX(CorpseFadeTime, corpsefadetime, 0)
    READ_ENTITY_DEFINITION_PROPERTY(CorpseFadeEffect, corpsefadeeffect)

    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackAnim, attackanim, attack_%)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackNumAnims, attacknumanims, 1)
    READ_ENTITY_DEFINITION_PROPERTY_EX(DeathAnim, deathanim, death_%)
    READ_ENTITY_DEFINITION_PROPERTY_EX(DeathNumAnims, deathnumanims, 1)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AltDeathAnim, altdeathanim, alt_death_%)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AltDeathNumAnims, altdeathnumanims, 1)
    READ_ENTITY_DEFINITION_PROPERTY_EX(DeathTime, deathtime, 2000)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ExpireAnim, expireanim, expire_%)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ExpireNumAnims, expirenumanims, 0)

    READ_ENTITY_DEFINITION_PROPERTY_EX(TiltFactor, tiltfactor, 0.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(TiltSpeed, tiltspeed, 90.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(CorpseTiltFactor, corpsetiltfactor, 1.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(CorpseTiltSpeed, corpsetiltspeed, 30.0)

    READ_ENTITY_DEFINITION_PROPERTY_EX(Invulnerable, invulnerable, false)

    READ_ENTITY_DEFINITION_PROPERTY_EX(AlwaysVisible, alwaysvisible, false)
    READ_ENTITY_DEFINITION_PROPERTY_EX(Hidden, hidden, false)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AlwaysTargetable, alwaystargetable, false)

    READ_ENTITY_DEFINITION_PROPERTY_EX(Flying, flying, false)
    READ_ENTITY_DEFINITION_PROPERTY_EX(FlyHeight, flyheight, 300.0)
    READ_ENTITY_DEFINITION_PROPERTY_EX(GroundOffset, groundoffset, 0.0)

    READ_ENTITY_DEFINITION_PROPERTY(Unitwalking, unitwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Treewalking, treewalking)
    READ_ENTITY_DEFINITION_PROPERTY(Cliffwalking, cliffwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Buildingwalking, buildingwalking)
    READ_ENTITY_DEFINITION_PROPERTY(Antiwalking, antiwalking)
    READ_ENTITY_DEFINITION_PROPERTY(ClearVision, clearvision)
    READ_ENTITY_DEFINITION_PROPERTY(Deniable, deniable)
    READ_ENTITY_DEFINITION_PROPERTY(DeniablePercent, deniablepercent)
    READ_ENTITY_DEFINITION_PROPERTY(Smackable, smackable)
    READ_ENTITY_DEFINITION_PROPERTY(NoThreat, nothreat)
    READ_ENTITY_DEFINITION_PROPERTY(TrueStrike, truestrike)
    READ_ENTITY_DEFINITION_PROPERTY(AlwaysTransmitData, alwaystransmitdata)

    READ_ENTITY_DEFINITION_PROPERTY(ShopAccess, shopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(RemoteShopAccess, remoteshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(SharedShopAccess, sharedshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(SharedRemoteShopAccess, sharedremoteshopaccess)
    READ_ENTITY_DEFINITION_PROPERTY(RestrictItemAccess, restrictitemaccess)
    READ_ENTITY_DEFINITION_PROPERTY(StashAccess, stashaccess)

    READ_ENTITY_DEFINITION_PROPERTY(DieWithOwner, diewithowner)
    READ_ENTITY_DEFINITION_PROPERTY(RelayExperience, relayexperience)
    READ_ENTITY_DEFINITION_PROPERTY(MaxDistanceFromOwner, maxdistancefromowner)
    READ_ENTITY_DEFINITION_PROPERTY(PreferTouch, prefertouch)
    READ_ENTITY_DEFINITION_PROPERTY(LargeUnit, largeunit)
}

ENTITY_DEF_MERGE_START(IUnitDefinition, IEntityDefinition)
    MERGE_STRING_VECTOR_ARRAY_PROPERTY(UnitType)

    MERGE_LOCALIZED_STRING_PROPERTY(DisplayName)
    MERGE_LOCALIZED_STRING_PROPERTY(Description)
    MERGE_RESOURCE_ARRAY_PROPERTY(Icon)
    MERGE_RESOURCE_ARRAY_PROPERTY(MapIconProperty)
    MERGE_ARRAY_PROPERTY(MapIconColorProperty)
    MERGE_ARRAY_PROPERTY(MapIconSizeProperty)
    MERGE_RESOURCE_ARRAY_PROPERTY(Portrait)
    MERGE_RESOURCE_ARRAY_PROPERTY(Model)
    MERGE_STRING_ARRAY_PROPERTY(Skin)
    MERGE_PROPERTY(DrawOnMap)
    MERGE_PROPERTY(HoverOnMap)
    MERGE_PROPERTY(PartialControlShare)
    MERGE_STRING_PROPERTY(IdleAnim)
    MERGE_STRING_PROPERTY(WalkAnim)
    MERGE_STRING_PROPERTY(DeniedAnim)
    MERGE_STRING_PROPERTY(GibAnim)
    
    MERGE_RESOURCE_ARRAY_PROPERTY(PassiveEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(SpawnEffect)
    MERGE_RESOURCE_PROPERTY(SelectedSound)
    MERGE_RESOURCE_PROPERTY(SelectedFlavorSound)
    MERGE_RESOURCE_PROPERTY(ConfirmMoveSound)
    MERGE_RESOURCE_PROPERTY(ConfirmAttackSound)
    MERGE_RESOURCE_PROPERTY(TauntedSound)
    MERGE_RESOURCE_PROPERTY(TauntKillSound)
    MERGE_RESOURCE_PROPERTY(NoManaSound)
    MERGE_RESOURCE_PROPERTY(CooldownSound)

    MERGE_ARRAY_PROPERTY(PreGlobalScale)
    MERGE_ARRAY_PROPERTY(ModelScale)
    MERGE_ARRAY_PROPERTY(EffectScale)
    MERGE_ARRAY_PROPERTY(InfoHeight)
    MERGE_ARRAY_PROPERTY(BoundsRadius)
    MERGE_ARRAY_PROPERTY(BoundsHeight)
    MERGE_PROPERTY(IsSelectable)
    MERGE_PROPERTY(NoCorpse)
    MERGE_PROPERTY(IsControllable)
    MERGE_PROPERTY(IsUnit)
    MERGE_PROPERTY(NoGlobalSelect)
    MERGE_PROPERTY(NoBlockNeutralSpawn)
    MERGE_ARRAY_PROPERTY(SelectionRadius)
    MERGE_ARRAY_PROPERTY(TargetOffset)

    MERGE_ARRAY_PROPERTY(IsMobile)
    MERGE_ARRAY_PROPERTY(CanAttack)
    MERGE_ARRAY_PROPERTY(CanRotate)
    MERGE_ARRAY_PROPERTY(MoveSpeed)
    MERGE_ARRAY_PROPERTY(SlowResistance)
    MERGE_PROPERTY(TurnRate)
    MERGE_PROPERTY(TurnSmoothing)
    MERGE_ARRAY_PROPERTY(Blocking)
    MERGE_ARRAY_PROPERTY(AntiBlocking)

    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxHealth)
    MERGE_ARRAY_PROPERTY(HealthRegen)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(MaxMana)
    MERGE_ARRAY_PROPERTY(ManaRegen)
    
    MERGE_ARRAY_PROPERTY(ArmorType)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(Armor)
    MERGE_ARRAY_PROPERTY(MagicArmorType)
    MERGE_ARRAY_PROPERTY(MagicArmor)

    MERGE_ARRAY_PROPERTY(LifeSteal)
    MERGE_ARRAY_PROPERTY(HealthRegenReduction)
    MERGE_ARRAY_PROPERTY(ManaRegenReduction)
    
    MERGE_ARRAY_PROPERTY(CanCarryItems)
    MERGE_ARRAY_PROPERTY(DropItemsOnDeath)
    MERGE_ARRAY_PROPERTY(PassiveInventory)
    MERGE_STRING_ARRAY_PROPERTY(Inventory0)
    MERGE_STRING_ARRAY_PROPERTY(Inventory1)
    MERGE_STRING_ARRAY_PROPERTY(Inventory2)
    MERGE_STRING_ARRAY_PROPERTY(Inventory3)
    MERGE_STRING_ARRAY_PROPERTY(Inventory4)
    MERGE_STRING_ARRAY_PROPERTY(Inventory5)
    MERGE_STRING_ARRAY_PROPERTY(Inventory6)
    MERGE_STRING_ARRAY_PROPERTY(Inventory7)
    MERGE_STRING_ARRAY_PROPERTY(Inventory8)
    MERGE_STRING_ARRAY_PROPERTY(SharedInventory0)
    MERGE_STRING_ARRAY_PROPERTY(SharedInventory1)
    MERGE_STRING_ARRAY_PROPERTY(SharedInventory2)

    MERGE_ARRAY_PROPERTY(AttackType)
    MERGE_ARRAY_PROPERTY(AttackEffectType)
    MERGE_ARRAY_PROPERTY(AttackRange)
    MERGE_STRING_ARRAY_PROPERTY(AttackProjectile)
    MERGE_ARRAY_PROPERTY(AttackRangeBuffer)
    MERGE_ARRAY_PROPERTY(AttackOffset)
    MERGE_ARRAY_PROPERTY(AttackDamageType)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackDamageMin)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(AttackDamageMax)
    MERGE_ARRAY_PROPERTY(AttackCooldown)
    MERGE_ARRAY_PROPERTY(AttackDuration)
    MERGE_ARRAY_PROPERTY(AttackActionTime)
    MERGE_ARRAY_PROPERTY(AttackTargetScheme)
    MERGE_ARRAY_PROPERTY(AttackNonLethal)
    MERGE_RESOURCE_ARRAY_PROPERTY(AttackStartEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(AttackActionEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(AttackImpactEffect)
    
    MERGE_STRING_ARRAY_PROPERTY(CombatType)
    MERGE_ARRAY_PROPERTY(RevealRange)
    MERGE_ARRAY_PROPERTY(RevealType)
    MERGE_ARRAY_PROPERTY(StealthType)
    MERGE_PROGRESSIVE_ARRAY_PROPERTY(FadeTime)
    MERGE_ARRAY_PROPERTY(StealthProximity)
    MERGE_ARRAY_PROPERTY(ImmunityType)

    MERGE_ARRAY_PROPERTY(ThreatScheme)
    MERGE_ARRAY_PROPERTY(ThreatEffectType)
    MERGE_ARRAY_PROPERTY(AggroRange)
    MERGE_ARRAY_PROPERTY(AggroScheme)
    MERGE_ARRAY_PROPERTY(ProximityRange)
    MERGE_ARRAY_PROPERTY(ProximityScheme)
    MERGE_ARRAY_PROPERTY(SightRangeDay)
    MERGE_ARRAY_PROPERTY(SightRangeNight)
    MERGE_ARRAY_PROPERTY(WanderRange)
    MERGE_ARRAY_PROPERTY(DefaultBehavior)

    MERGE_ARRAY_PROPERTY(GoldBountyMin)
    MERGE_ARRAY_PROPERTY(GoldBountyMax)
    MERGE_ARRAY_PROPERTY(GoldBountyTeam)
    MERGE_ARRAY_PROPERTY(GoldBountyConsolation)
    MERGE_ARRAY_PROPERTY(ExperienceBounty)
    MERGE_ARRAY_PROPERTY(GlobalExperience)
    MERGE_ARRAY_PROPERTY(DeadExperience)
    MERGE_ARRAY_PROPERTY(UnsharedExperienceBounty)

    MERGE_PROPERTY(CorpseTime)
    MERGE_PROPERTY(CorpseFadeTime)
    MERGE_RESOURCE_PROPERTY(CorpseFadeEffect)
    
    MERGE_STRING_PROPERTY(AttackAnim)
    MERGE_PROPERTY(AttackNumAnims)
    MERGE_STRING_PROPERTY(DeathAnim)
    MERGE_PROPERTY(DeathNumAnims)
    MERGE_STRING_PROPERTY(AltDeathAnim)
    MERGE_PROPERTY(AltDeathNumAnims)
    MERGE_PROPERTY(DeathTime)
    MERGE_STRING_PROPERTY(ExpireAnim)
    MERGE_PROPERTY(ExpireNumAnims)

    MERGE_PROPERTY(TiltFactor)
    MERGE_PROPERTY(TiltSpeed)
    MERGE_PROPERTY(CorpseTiltFactor)
    MERGE_PROPERTY(CorpseTiltSpeed)

    MERGE_ARRAY_PROPERTY(Invulnerable)
    MERGE_PROPERTY(AlwaysVisible)
    MERGE_PROPERTY(Hidden)
    MERGE_PROPERTY(AlwaysTargetable)
    
    MERGE_ARRAY_PROPERTY(Flying)
    MERGE_ARRAY_PROPERTY(FlyHeight)
    MERGE_ARRAY_PROPERTY(GroundOffset)

    MERGE_ARRAY_PROPERTY(Unitwalking)
    MERGE_ARRAY_PROPERTY(Treewalking)
    MERGE_ARRAY_PROPERTY(Cliffwalking)
    MERGE_ARRAY_PROPERTY(Buildingwalking)
    MERGE_ARRAY_PROPERTY(Antiwalking)
    MERGE_ARRAY_PROPERTY(ClearVision)
    MERGE_ARRAY_PROPERTY(Deniable)
    MERGE_ARRAY_PROPERTY(DeniablePercent)
    MERGE_ARRAY_PROPERTY(Smackable)
    MERGE_ARRAY_PROPERTY(NoThreat)
    MERGE_ARRAY_PROPERTY(TrueStrike)
    MERGE_ARRAY_PROPERTY(AlwaysTransmitData)

    MERGE_STRING_ARRAY_PROPERTY(ShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(RemoteShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(SharedShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(SharedRemoteShopAccess)
    MERGE_STRING_ARRAY_PROPERTY(RestrictItemAccess)
    MERGE_ARRAY_PROPERTY(StashAccess)

    MERGE_ARRAY_PROPERTY(DieWithOwner)
    MERGE_ARRAY_PROPERTY(RelayExperience)
    MERGE_ARRAY_PROPERTY(MaxDistanceFromOwner)
    MERGE_ARRAY_PROPERTY(PreferTouch)
    MERGE_ARRAY_PROPERTY(LargeUnit)
ENTITY_DEF_MERGE_END
