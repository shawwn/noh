// (C)2008 S2 Games
// i_tooldefinition.h
//
//=============================================================================
#ifndef __I_TOOLDEFINITION_H__
#define __I_TOOLDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_slavedefinition.h"
//=============================================================================

//=============================================================================
// IToolDefinition
//=============================================================================
class IToolDefinition : public ISlaveDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_PROPERTY(ActionType, EEntityToolAction)
	ENT_DEF_PROPERTY(MaxLevel, uint)
	ENT_DEF_PROPERTY(BaseLevel, uint)

	ENT_DEF_RESOURCE_ARRAY_PROPERTY(CastEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(ActionEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(ImpactEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(BridgeEffect, Effect)

	ENT_DEF_PROPERTY(IsChanneling, bool)
	ENT_DEF_STRING_ARRAY_PROPERTY(Anim)
	ENT_DEF_ARRAY_PROPERTY(AnimChannel, int)
	ENT_DEF_ARRAY_PROPERTY(CastTime, uint)
	ENT_DEF_ARRAY_PROPERTY(CastActionTime, uint)
	ENT_DEF_ARRAY_PROPERTY(ChannelTime, uint)
	ENT_DEF_ARRAY_PROPERTY(ManaCost, float)
	ENT_DEF_ARRAY_PROPERTY(ToggleOffManaCost, float)
	ENT_DEF_ARRAY_PROPERTY(ActiveManaCost, float)
	ENT_DEF_ARRAY_PROPERTY(TriggeredManaCost, float)
	ENT_DEF_ARRAY_PROPERTY(CooldownTime, uint)
	ENT_DEF_ARRAY_PROPERTY(ToggleOffCooldownTime, uint)
	ENT_DEF_ARRAY_PROPERTY(CooldownOnDamage, uint)
	ENT_DEF_ARRAY_PROPERTY(Range, float)
	ENT_DEF_ARRAY_PROPERTY(ForceRange, float)
	ENT_DEF_ARRAY_PROPERTY(MinRange, float)
	ENT_DEF_ARRAY_PROPERTY(RangeBuffer, float)
	ENT_DEF_ARRAY_PROPERTY(TargetRadius, float)
	ENT_DEF_ARRAY_PROPERTY(MaxDelta, float)
	ENT_DEF_ARRAY_PROPERTY(ForceDelta, float)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(TargetMaterial, Material)
	ENT_DEF_ARRAY_PROPERTY(AllowOutOfRangeCast, bool)
	ENT_DEF_ARRAY_PROPERTY(AllowOutOfBoundsCast, bool)
	ENT_DEF_ARRAY_PROPERTY(AllowAutoCast, bool)
	ENT_DEF_ARRAY_PROPERTY(UsePathForRange, bool)
	ENT_DEF_ARRAY_PROPERTY_EX(CastEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY_EX(TargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(IgnoreInvulnerable, bool)
	ENT_DEF_STRING_ARRAY_PROPERTY(Projectile)
	ENT_DEF_PROPERTY(FrontQueue, bool)
	ENT_DEF_PROPERTY(InheritMovement, bool)
	ENT_DEF_STRING_ARRAY_PROPERTY(StatusEffectTooltip)
	ENT_DEF_STRING_ARRAY_PROPERTY(StatusEffectTooltip2)
	ENT_DEF_LOCALIZED_STRING_PROPERTY(StatusEffectHeader)
	ENT_DEF_LOCALIZED_STRING_PROPERTY(StatusEffectHeader2)
	ENT_DEF_LOCALIZED_STRING_PROPERTY(TooltipFlavorText)

	ENT_DEF_ARRAY_PROPERTY(UseProxy, bool)
	ENT_DEF_ARRAY_PROPERTY_EX(ProxyTargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(ProxyEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(ProxySelectionRadius, float)
	ENT_DEF_ARRAY_PROPERTY(ProxySelectionMethod, ETargetSelection)
	ENT_DEF_ARRAY_PROPERTY(ProxyAllowInvulnerable, bool)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(ProxyTargetMaterial, Material)

	ENT_DEF_ARRAY_PROPERTY(SearchRadius, float)
	ENT_DEF_ARRAY_PROPERTY(Disabled, bool)

	ENT_DEF_ARRAY_PROPERTY(NoTargetRadius, float)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(NoTargetMaterial, Material)
	ENT_DEF_ARRAY_PROPERTY_EX(NoCastEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY_EX(NoTargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(NoTargetIgnoreInvulnerable, bool)
	ENT_DEF_ARRAY_PROPERTY(DeferChannelCost, bool)
	ENT_DEF_ARRAY_PROPERTY(DeferChannelImpact, bool)
	ENT_DEF_ARRAY_PROPERTY(ChannelRange, float)

	ENT_DEF_ARRAY_PROPERTY_EX(CooldownType, uint, Game.LookupCooldownType)

	ENT_DEF_ARRAY_PROPERTY(NonInterrupting, bool)
	ENT_DEF_ARRAY_PROPERTY(IgnoreCooldown, bool)
	ENT_DEF_ARRAY_PROPERTY(AutoToggleOffWhenDisabled, bool)
	ENT_DEF_ARRAY_PROPERTY(AutoToggleOffWithTriggeredManaCost, bool)
	ENT_DEF_ARRAY_PROPERTY(NoStopAnim, bool)
	ENT_DEF_ARRAY_PROPERTY(NoResponse, bool)
	ENT_DEF_ARRAY_PROPERTY(NoVoiceResponse, bool)
	ENT_DEF_ARRAY_PROPERTY(NeedVision, bool)

	ENT_DEF_ARRAY_PROPERTY_EX(ActivateScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(CarryScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(CloneScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY(ChargeCost, uint)
	
	ENT_DEF_ARRAY_PROPERTY_EX(AttackEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY_EX(AttackDamageType, uint, Game.LookupEffectType)

	ENT_DEF_ARRAY_PROPERTY(DoubleActivate, bool)

private:
	IToolDefinition();

public:
	~IToolDefinition()	{}
	IToolDefinition(IBaseEntityAllocator *pAllocator) :
	ISlaveDefinition(pAllocator)
	{}

	static void		ReadSettings(IToolDefinition *pDefinition, const class CXMLNode &node, bool bMod);

	virtual void	Precache(EPrecacheScheme eScheme)
	{
		ISlaveDefinition::Precache(eScheme);

		PRECACHE_GUARD
			PRECACHE_ENTITY_ARRAY(Projectile, eScheme)
			PrecacheCastEffect();
			PrecacheImpactEffect();
			PrecacheBridgeEffect();

			PrecacheTargetMaterial();
			PrecacheProxyTargetMaterial();
			PrecacheNoTargetMaterial();
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		ISlaveDefinition::GetPrecacheList(eScheme, deqPrecache);

		PRECACHE_GUARD
			GET_ENTITY_ARRAY_PRECACHE_LIST(Projectile, eScheme, deqPrecache)
		PRECACHE_GUARD_END
	}

	virtual void	PostProcess()
	{
		if (m_bPostProcessing)
			return;

		ISlaveDefinition::PostProcess();

		m_bPostProcessing = true;

		PRECACHE_LOCALIZED_STRING(StatusEffectHeader, effect_header);
		PRECACHE_LOCALIZED_STRING(StatusEffectHeader2, effect_header2);
		PRECACHE_LOCALIZED_STRING(TooltipFlavorText, tooltip_flavor);

		m_bPostProcessing = false;
	}

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__I_TOOLDEFINITION_H__
