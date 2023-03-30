// (C)2008 S2 Games
// i_unitdefinition.h
//
//=============================================================================
#ifndef __I_UNITDEFINITION_H__
#define __I_UNITDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"
//=============================================================================

//=============================================================================
// IUnitDefinition
//=============================================================================
class IUnitDefinition : public IEntityDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_STRING_VECTOR_ARRAY_PROPERTY(UnitType)

	ENT_DEF_LOCALIZED_STRING_PROPERTY(DisplayName)
	ENT_DEF_LOCALIZED_STRING_PROPERTY(Description)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(Icon, Icon)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(Portrait, Icon)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(Model, Model)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(MapIconProperty, Icon)
	ENT_DEF_ARRAY_PROPERTY(MapIconColorProperty, CVec4f)
	ENT_DEF_ARRAY_PROPERTY(MapIconSizeProperty, float)
	ENT_DEF_STRING_ARRAY_PROPERTY(Skin)
	ENT_DEF_PROPERTY(DrawOnMap, bool)
	ENT_DEF_PROPERTY(PartialControlShare, bool)
	ENT_DEF_STRING_PROPERTY(IdleAnim)
	ENT_DEF_STRING_PROPERTY(WalkAnim)
	ENT_DEF_STRING_PROPERTY(DeniedAnim)
	ENT_DEF_STRING_PROPERTY(GibAnim)
	ENT_DEF_STRING_PROPERTY(SprintAnim)
	
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(PassiveEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(SpawnEffect, Effect)
	ENT_DEF_RESOURCE_PROPERTY(SelectedSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(SelectedFlavorSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(ConfirmMoveSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(ConfirmAttackSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(TauntedSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(TauntKillSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(NoManaSound, Sample)
	ENT_DEF_RESOURCE_PROPERTY(CooldownSound, Sample)

	ENT_DEF_ARRAY_PROPERTY(PreGlobalScale, float)
	ENT_DEF_ARRAY_PROPERTY(ModelScale, float)
	ENT_DEF_ARRAY_PROPERTY(EffectScale, float)
	ENT_DEF_ARRAY_PROPERTY(InfoHeight, float)
	ENT_DEF_ARRAY_PROPERTY(BoundsRadius, float)
	ENT_DEF_ARRAY_PROPERTY(BoundsHeight, float)
	ENT_DEF_PROPERTY(IsSelectable, bool)
	ENT_DEF_PROPERTY(NoCorpse, bool)
	ENT_DEF_PROPERTY(IsControllable, bool)
	ENT_DEF_PROPERTY(IsUnit, bool)
	ENT_DEF_PROPERTY(NoGlobalSelect, bool)
	ENT_DEF_PROPERTY(NoBlockNeutralSpawn, bool)
	ENT_DEF_ARRAY_PROPERTY(SelectionRadius, float)
	ENT_DEF_ARRAY_PROPERTY(TargetOffset, CVec3f)

	ENT_DEF_ARRAY_PROPERTY(IsMobile, bool)
	ENT_DEF_ARRAY_PROPERTY(CanAttack, bool)
	ENT_DEF_ARRAY_PROPERTY(CanRotate, bool)
	ENT_DEF_ARRAY_PROPERTY(MoveSpeed, float)
	ENT_DEF_ARRAY_PROPERTY(SlowResistance, float)
	ENT_DEF_PROPERTY(TurnRate, float)
	ENT_DEF_PROPERTY(TurnSmoothing, float)
	ENT_DEF_ARRAY_PROPERTY(Blocking, bool)
	ENT_DEF_ARRAY_PROPERTY(AntiBlocking, bool)

	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxHealth, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthRegen, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(HealthProportionRegen, float)
	
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxMana, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaRegen, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(ManaProportionRegen, float)

	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MaxStamina, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(StaminaRegen, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(StaminaProportionRegen, float)
	
	ENT_DEF_ARRAY_PROPERTY_EX(ArmorType, uint, Game.LookupArmorType)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Armor, float)
	ENT_DEF_ARRAY_PROPERTY_EX(MagicArmorType, uint, Game.LookupArmorType)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(MagicArmor, float)

	ENT_DEF_ARRAY_PROPERTY(LifeSteal, float)

	ENT_DEF_ARRAY_PROPERTY(HealthRegenReduction, float)
	ENT_DEF_ARRAY_PROPERTY(ManaRegenReduction, float)
	ENT_DEF_ARRAY_PROPERTY(StaminaRegenReduction, float)
	
	ENT_DEF_ARRAY_PROPERTY(CanCarryItems, bool)
	ENT_DEF_ARRAY_PROPERTY(DropItemsOnDeath, bool)
	ENT_DEF_ARRAY_PROPERTY(PassiveInventory, bool)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory0)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory1)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory2)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory3)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory4)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory5)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory6)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory7)
	ENT_DEF_STRING_ARRAY_PROPERTY(Inventory8)
	ENT_DEF_STRING_ARRAY_PROPERTY(SharedInventory0)
	ENT_DEF_STRING_ARRAY_PROPERTY(SharedInventory1)
	ENT_DEF_STRING_ARRAY_PROPERTY(SharedInventory2)

	ENT_DEF_ARRAY_PROPERTY_EX(AttackType, uint, Game.LookupAttackType)
	ENT_DEF_ARRAY_PROPERTY_EX(AttackEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(AttackRange, float)
	ENT_DEF_STRING_ARRAY_PROPERTY(AttackProjectile)
	ENT_DEF_ARRAY_PROPERTY(AttackRangeBuffer, float)
	ENT_DEF_ARRAY_PROPERTY(AttackOffset, CVec3f)
	ENT_DEF_ARRAY_PROPERTY_EX(AttackDamageType, uint, Game.LookupEffectType)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackDamageMin, float)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(AttackDamageMax, float)
	ENT_DEF_ARRAY_PROPERTY(AttackCooldown, uint)
	ENT_DEF_ARRAY_PROPERTY(AttackDuration, uint)
	ENT_DEF_ARRAY_PROPERTY(AttackActionTime, uint)
	ENT_DEF_ARRAY_PROPERTY_EX(AttackTargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(AttackNonLethal, bool)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(AttackStartEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(AttackActionEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(AttackImpactEffect, Effect)
	
	ENT_DEF_STRING_ARRAY_PROPERTY(CombatType)
	ENT_DEF_ARRAY_PROPERTY(RevealRange, float)
	ENT_DEF_ARRAY_PROPERTY_EX(RevealType, uint, Game.LookupRevealType)
	ENT_DEF_ARRAY_PROPERTY_EX(StealthType, uint, Game.LookupStealthType)
	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(FadeTime, uint)
	ENT_DEF_ARRAY_PROPERTY(StealthProximity, float)
	ENT_DEF_ARRAY_PROPERTY_EX(ImmunityType, uint, Game.LookupImmunityType)

	ENT_DEF_ARRAY_PROPERTY_EX(ThreatScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(ThreatEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(AggroRange, float)
	ENT_DEF_ARRAY_PROPERTY_EX(AggroScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(ProximityRange, float)
	ENT_DEF_ARRAY_PROPERTY_EX(ProximityScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(SightRangeDay, float)
	ENT_DEF_ARRAY_PROPERTY(SightRangeNight, float)
	ENT_DEF_ARRAY_PROPERTY(WanderRange, float)
	ENT_DEF_ARRAY_PROPERTY(DefaultBehavior, EUnitCommand)

	ENT_DEF_ARRAY_PROPERTY(GoldBountyMin, uint)
	ENT_DEF_ARRAY_PROPERTY(GoldBountyMax, uint)
	ENT_DEF_ARRAY_PROPERTY(GoldBountyTeam, uint)
	ENT_DEF_ARRAY_PROPERTY(GoldBountyConsolation, uint)
	ENT_DEF_ARRAY_PROPERTY(ExperienceBounty, float)
	ENT_DEF_ARRAY_PROPERTY(GlobalExperience, bool)
	ENT_DEF_ARRAY_PROPERTY(DeadExperience, bool)

	ENT_DEF_PROPERTY(CorpseTime, uint)
	ENT_DEF_PROPERTY(CorpseFadeTime, uint)
	ENT_DEF_RESOURCE_PROPERTY(CorpseFadeEffect, Effect)
	
	ENT_DEF_STRING_PROPERTY(AttackAnim)
	ENT_DEF_PROPERTY(AttackNumAnims, uint)
	ENT_DEF_STRING_PROPERTY(DeathAnim)
	ENT_DEF_PROPERTY(DeathNumAnims, uint)
	ENT_DEF_STRING_PROPERTY(AltDeathAnim)
	ENT_DEF_PROPERTY(AltDeathNumAnims, uint)
	ENT_DEF_PROPERTY(DeathTime, uint)
	ENT_DEF_STRING_PROPERTY(ExpireAnim)
	ENT_DEF_PROPERTY(ExpireNumAnims, uint)

	ENT_DEF_PROPERTY(TiltFactor, float)
	ENT_DEF_PROPERTY(TiltSpeed, float)
	ENT_DEF_PROPERTY(CorpseTiltFactor, float)
	ENT_DEF_PROPERTY(CorpseTiltSpeed, float)

	ENT_DEF_ARRAY_PROPERTY(Invulnerable, bool)
	ENT_DEF_PROPERTY(AlwaysVisible, bool)
	ENT_DEF_PROPERTY(Hidden, bool)
	ENT_DEF_PROPERTY(AlwaysTargetable, bool)
	
	ENT_DEF_ARRAY_PROPERTY(Flying, bool)
	ENT_DEF_ARRAY_PROPERTY(FlyHeight, float)
	ENT_DEF_ARRAY_PROPERTY(GroundOffset, float)

	ENT_DEF_ARRAY_PROPERTY(Unitwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Treewalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Cliffwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Buildingwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Antiwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(ClearVision, bool)
	ENT_DEF_ARRAY_PROPERTY(Deniable, bool)
	ENT_DEF_ARRAY_PROPERTY(DeniablePercent, float)
	ENT_DEF_ARRAY_PROPERTY(Smackable, bool)
	ENT_DEF_ARRAY_PROPERTY(NoThreat, bool)
	ENT_DEF_ARRAY_PROPERTY(TrueStrike, bool)

	ENT_DEF_STRING_ARRAY_PROPERTY(ShopAccess)
	ENT_DEF_STRING_ARRAY_PROPERTY(RemoteShopAccess)
	ENT_DEF_STRING_ARRAY_PROPERTY(SharedShopAccess)
	ENT_DEF_STRING_ARRAY_PROPERTY(SharedRemoteShopAccess)
	ENT_DEF_STRING_ARRAY_PROPERTY(RestrictItemAccess)
	ENT_DEF_ARRAY_PROPERTY(StashAccess, bool)

	ENT_DEF_ARRAY_PROPERTY(DieWithOwner, bool)
	ENT_DEF_ARRAY_PROPERTY(RelayExperience, bool)
	ENT_DEF_ARRAY_PROPERTY(MaxDistanceFromOwner, float)
	ENT_DEF_ARRAY_PROPERTY(PreferTouch, bool)

	ENT_DEF_ARRAY_PROPERTY(SprintMoveSpeedBonus, float)
	ENT_DEF_ARRAY_PROPERTY(SprintMoveSpeedMultiplier, float)
	ENT_DEF_ARRAY_PROPERTY(SprintStaminaCost, float)

	ENT_DEF_PROGRESSIVE_ARRAY_PROPERTY(Power, float)

private:
	IUnitDefinition();

public:
	~IUnitDefinition()	{}
	IUnitDefinition(IBaseEntityAllocator *pAllocator) :
	IEntityDefinition(pAllocator)
	{}

	static void		ReadSettings(IUnitDefinition *pDefinition, const class CXMLNode &node, bool bMod);

	virtual void	Precache(EPrecacheScheme eScheme)
	{
		IEntityDefinition::Precache(eScheme);

		PRECACHE_GUARD
			PrecacheIcon();
			PrecachePortrait();
			PrecacheModel();
			PrecacheMapIconProperty();
			PrecachePassiveEffect();
			PrecacheSpawnEffect();
			PrecacheTauntedSound();
			PrecacheTauntKillSound();
			PrecacheAttackStartEffect();
			PrecacheAttackActionEffect();
			PrecacheAttackImpactEffect();
			PrecacheCorpseFadeEffect();

			//if (eScheme != PRECACHE_OTHER)
			{
				PrecacheSelectedSound();
				PrecacheSelectedFlavorSound();
				PrecacheConfirmMoveSound();
				PrecacheConfirmAttackSound();
				PrecacheNoManaSound();
				PrecacheCooldownSound();
			}

			PRECACHE_ENTITY_ARRAY(Inventory0, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory1, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory2, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory3, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory4, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory5, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory6, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory7, eScheme)
			PRECACHE_ENTITY_ARRAY(Inventory8, eScheme)
			PRECACHE_ENTITY_ARRAY(SharedInventory0, eScheme)
			PRECACHE_ENTITY_ARRAY(SharedInventory1, eScheme)
			PRECACHE_ENTITY_ARRAY(SharedInventory2, eScheme)
			PRECACHE_ENTITY_ARRAY(AttackProjectile, eScheme)
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		IEntityDefinition::GetPrecacheList(eScheme, deqPrecache);

		PRECACHE_GUARD
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory0, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory1, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory2, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory3, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory4, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory5, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory6, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory7, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(Inventory8, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(SharedInventory0, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(SharedInventory1, eScheme, deqPrecache)
			GET_ENTITY_ARRAY_PRECACHE_LIST(SharedInventory2, eScheme, deqPrecache)
		PRECACHE_GUARD_END
	}

	virtual void	PostProcess()
	{
		if (m_bPostProcessing)
			return;

		IEntityDefinition::PostProcess();

		m_bPostProcessing = true;

		PRECACHE_LOCALIZED_STRING(DisplayName, name);
		PRECACHE_LOCALIZED_STRING(Description, description);

		m_bPostProcessing = false;
	}

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__I_UNITDEFINITION_H__
