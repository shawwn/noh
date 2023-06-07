// (C)2008 S2 Games
// i_tooldefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_tooldefinition.h"

#include "../k2/c_xmlnode.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(IToolDefinition, ENTITY_BASE_TYPE_TOOL, Tool)

/*====================
  IToolDefinition::ReadSettings
  ====================*/
void    IToolDefinition::ReadSettings(IToolDefinition *pDefinition, const CXMLNode &node, bool bMod)
{
    ISlaveDefinition::ReadSettings(pDefinition, node, bMod);

    pDefinition->SetStatusEffectHeaderPriority(pDefinition->GetPriority());
    pDefinition->SetStatusEffectHeader2Priority(pDefinition->GetPriority());
    pDefinition->SetTooltipFlavorTextPriority(pDefinition->GetPriority());

    READ_ENTITY_DEFINITION_PROPERTY(TargetMaterial, targetmaterial)
    READ_ENTITY_DEFINITION_PROPERTY(TargetRadius, targetradius)
    READ_ENTITY_DEFINITION_PROPERTY(ActionEffect, actioneffect)
    READ_ENTITY_DEFINITION_PROPERTY(CastEffect, casteffect)
    READ_ENTITY_DEFINITION_PROPERTY(ImpactEffect, impacteffect)
    READ_ENTITY_DEFINITION_PROPERTY(BridgeEffect, bridgeeffect)

    READ_ENTITY_DEFINITION_PROPERTY(ActionType, actiontype)
    READ_ENTITY_DEFINITION_PROPERTY(MaxLevel, maxlevel)
    READ_ENTITY_DEFINITION_PROPERTY(BaseLevel, baselevel)

    READ_ENTITY_DEFINITION_PROPERTY(Anim, anim)
    READ_ENTITY_DEFINITION_PROPERTY(AnimChannel, animchannel)
    READ_ENTITY_DEFINITION_PROPERTY(CastTime, casttime)
    READ_ENTITY_DEFINITION_PROPERTY(CastActionTime, castactiontime)
    READ_ENTITY_DEFINITION_PROPERTY(IsChanneling, ischanneling)
    READ_ENTITY_DEFINITION_PROPERTY(ChannelTime, channeltime)
    READ_ENTITY_DEFINITION_PROPERTY(ManaCost, manacost)
    READ_ENTITY_DEFINITION_PROPERTY(ToggleOffManaCost, toggleoffmanacost)
    READ_ENTITY_DEFINITION_PROPERTY(ActiveManaCost, activemanacost)
    READ_ENTITY_DEFINITION_PROPERTY(TriggeredManaCost, triggeredmanacost)
    READ_ENTITY_DEFINITION_PROPERTY(CooldownTime, cooldowntime)
    READ_ENTITY_DEFINITION_PROPERTY(ToggleOffCooldownTime, toggleoffcooldowntime)
    READ_ENTITY_DEFINITION_PROPERTY(CooldownOnDamage, cooldownondamage)
    READ_ENTITY_DEFINITION_PROPERTY(Range, range)
    READ_ENTITY_DEFINITION_PROPERTY(ForceRange, forcerange)
    READ_ENTITY_DEFINITION_PROPERTY(MinRange, minrange)
    READ_ENTITY_DEFINITION_PROPERTY(ForceDelta, forcedelta)
    READ_ENTITY_DEFINITION_PROPERTY(MaxDelta, maxdelta)
    READ_ENTITY_DEFINITION_PROPERTY_EX(RangeBuffer, rangebuffer, 300.0)
    READ_ENTITY_DEFINITION_PROPERTY(AllowOutOfRangeCast, allowoutofrangecast)
    READ_ENTITY_DEFINITION_PROPERTY(AllowOutOfBoundsCast, allowoutofboundscast)
    READ_ENTITY_DEFINITION_PROPERTY(AllowAutoCast, allowautocast)
    READ_ENTITY_DEFINITION_PROPERTY(UsePathForRange, usepathforrange)
    READ_ENTITY_DEFINITION_PROPERTY(CastEffectType, casteffecttype)
    READ_ENTITY_DEFINITION_PROPERTY(TargetScheme, targetscheme)
    READ_ENTITY_DEFINITION_PROPERTY(IgnoreInvulnerable, ignoreinvulnerable)
    READ_ENTITY_DEFINITION_PROPERTY(Projectile, projectile)
    READ_ENTITY_DEFINITION_PROPERTY_EX(FrontQueue, frontqueue, false)
    READ_ENTITY_DEFINITION_PROPERTY_EX(InheritMovement, inheritmovement, false)
    READ_ENTITY_DEFINITION_PROPERTY(StatusEffectTooltip, statuseffecttooltip)
    READ_ENTITY_DEFINITION_PROPERTY(StatusEffectTooltip2, statuseffecttooltip2)

    READ_ENTITY_DEFINITION_PROPERTY(UseProxy, useproxy)
    READ_ENTITY_DEFINITION_PROPERTY(ProxyTargetScheme, proxytargetscheme)
    READ_ENTITY_DEFINITION_PROPERTY(ProxyEffectType, proxyeffecttype)
    READ_ENTITY_DEFINITION_PROPERTY(ProxySelectionRadius, proxyselectionradius)
    READ_ENTITY_DEFINITION_PROPERTY_EX(ProxySelectionMethod, proxyselectionmethod, TARGET_SELECT_RANDOM)
    READ_ENTITY_DEFINITION_PROPERTY(ProxyAllowInvulnerable, proxyallowinvulnerable)
    READ_ENTITY_DEFINITION_PROPERTY(ProxyTargetMaterial, proxytargetmaterial)
    
    READ_ENTITY_DEFINITION_PROPERTY(SearchRadius, searchradius)
    READ_ENTITY_DEFINITION_PROPERTY(Disabled, disabled)

    READ_ENTITY_DEFINITION_PROPERTY(NoStun, nostun)

    READ_ENTITY_DEFINITION_PROPERTY(NoTargetRadius, notargetradius)
    READ_ENTITY_DEFINITION_PROPERTY(NoTargetMaterial, notargetmaterial)
    READ_ENTITY_DEFINITION_PROPERTY(NoCastEffectType, nocasteffecttype)
    READ_ENTITY_DEFINITION_PROPERTY(NoTargetScheme, notargetscheme)
    READ_ENTITY_DEFINITION_PROPERTY(NoTargetIgnoreInvulnerable, notargetignoreinvulnerable)
    READ_ENTITY_DEFINITION_PROPERTY(DeferChannelCost, deferchannelcost)
    READ_ENTITY_DEFINITION_PROPERTY(DeferChannelImpact, deferchannelimpact)
    READ_ENTITY_DEFINITION_PROPERTY(ChannelRange, channelrange)

    READ_ENTITY_DEFINITION_PROPERTY(CooldownType, cooldowntype)

    READ_ENTITY_DEFINITION_PROPERTY(NonInterrupting, noninterrupting)
    READ_ENTITY_DEFINITION_PROPERTY(IgnoreCooldown, ignorecooldown)
    READ_ENTITY_DEFINITION_PROPERTY(AutoToggleOffWhenDisabled, autotoggleoffwhendisabled)
    READ_ENTITY_DEFINITION_PROPERTY(AutoToggleOffWithTriggeredManaCost, autotoggleoffwithtriggeredmanacost)
    READ_ENTITY_DEFINITION_PROPERTY(NoStopAnim, nostopanim)
    READ_ENTITY_DEFINITION_PROPERTY(NoResponse, noresponse)
    READ_ENTITY_DEFINITION_PROPERTY(NoVoiceResponse, novoiceresponse)
    READ_ENTITY_DEFINITION_PROPERTY(NeedVision, needvision)
    READ_ENTITY_DEFINITION_PROPERTY(NoTurnToTarget, noturntotarget)

    READ_ENTITY_DEFINITION_PROPERTY(ActivateScheme, activatescheme)
    READ_ENTITY_DEFINITION_PROPERTY(CarryScheme, carryscheme)
    READ_ENTITY_DEFINITION_PROPERTY(CloneScheme, clonescheme)
    READ_ENTITY_DEFINITION_PROPERTY(ChargeCost, chargecost)

    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackEffectType, attackeffecttype, Attack Physical)
    READ_ENTITY_DEFINITION_PROPERTY_EX(AttackDamageType, attackdamagetype, Physical)

    READ_ENTITY_DEFINITION_PROPERTY(DoubleActivate, doubleactivate)
}


ENTITY_DEF_MERGE_START(IToolDefinition, ISlaveDefinition)
    MERGE_RESOURCE_ARRAY_PROPERTY(TargetMaterial)
    MERGE_ARRAY_PROPERTY(TargetRadius)
    MERGE_RESOURCE_ARRAY_PROPERTY(CastEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(ActionEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(ImpactEffect)
    MERGE_RESOURCE_ARRAY_PROPERTY(BridgeEffect)

    MERGE_PROPERTY(ActionType)
    MERGE_PROPERTY(MaxLevel)
    MERGE_PROPERTY(BaseLevel)

    MERGE_STRING_ARRAY_PROPERTY(Anim)
    MERGE_ARRAY_PROPERTY(AnimChannel)
    MERGE_ARRAY_PROPERTY(CastTime)
    MERGE_ARRAY_PROPERTY(CastActionTime)
    MERGE_PROPERTY(IsChanneling)
    MERGE_ARRAY_PROPERTY(ChannelTime)
    MERGE_ARRAY_PROPERTY(ManaCost)
    MERGE_ARRAY_PROPERTY(ToggleOffManaCost)
    MERGE_ARRAY_PROPERTY(ActiveManaCost)
    MERGE_ARRAY_PROPERTY(TriggeredManaCost)
    MERGE_ARRAY_PROPERTY(CooldownTime)
    MERGE_ARRAY_PROPERTY(ToggleOffCooldownTime)
    MERGE_ARRAY_PROPERTY(CooldownOnDamage)
    MERGE_ARRAY_PROPERTY(Range)
    MERGE_ARRAY_PROPERTY(ForceRange)
    MERGE_ARRAY_PROPERTY(MinRange)
    MERGE_ARRAY_PROPERTY(RangeBuffer)
    MERGE_ARRAY_PROPERTY(MaxDelta)
    MERGE_ARRAY_PROPERTY(ForceDelta)
    MERGE_ARRAY_PROPERTY(AllowOutOfRangeCast)
    MERGE_ARRAY_PROPERTY(AllowOutOfBoundsCast)
    MERGE_ARRAY_PROPERTY(AllowAutoCast)
    MERGE_ARRAY_PROPERTY(UsePathForRange)
    MERGE_ARRAY_PROPERTY(CastEffectType)
    MERGE_ARRAY_PROPERTY(TargetScheme)
    MERGE_ARRAY_PROPERTY(IgnoreInvulnerable)
    MERGE_STRING_ARRAY_PROPERTY(Projectile)
    MERGE_PROPERTY(FrontQueue)
    MERGE_PROPERTY(InheritMovement)
    MERGE_STRING_ARRAY_PROPERTY(StatusEffectTooltip)
    MERGE_STRING_ARRAY_PROPERTY(StatusEffectTooltip2)

    MERGE_LOCALIZED_STRING_PROPERTY(StatusEffectHeader)
    MERGE_LOCALIZED_STRING_PROPERTY(StatusEffectHeader2)
    MERGE_LOCALIZED_STRING_PROPERTY(TooltipFlavorText)

    MERGE_ARRAY_PROPERTY(UseProxy)
    MERGE_ARRAY_PROPERTY(ProxyTargetScheme)
    MERGE_ARRAY_PROPERTY(ProxyEffectType)
    MERGE_ARRAY_PROPERTY(ProxySelectionRadius)
    MERGE_ARRAY_PROPERTY(ProxySelectionMethod)
    MERGE_ARRAY_PROPERTY(ProxyAllowInvulnerable)
    MERGE_RESOURCE_ARRAY_PROPERTY(ProxyTargetMaterial)
    
    MERGE_ARRAY_PROPERTY(SearchRadius)
    MERGE_ARRAY_PROPERTY(Disabled)

    MERGE_ARRAY_PROPERTY(NoStun)

    MERGE_ARRAY_PROPERTY(NoTargetRadius)
    MERGE_ARRAY_PROPERTY(NoTargetMaterial)
    MERGE_ARRAY_PROPERTY(NoCastEffectType)
    MERGE_ARRAY_PROPERTY(NoTargetScheme)
    MERGE_ARRAY_PROPERTY(NoTargetIgnoreInvulnerable)
    MERGE_ARRAY_PROPERTY(DeferChannelCost)
    MERGE_ARRAY_PROPERTY(DeferChannelImpact)
    MERGE_ARRAY_PROPERTY(ChannelRange)

    MERGE_ARRAY_PROPERTY(CooldownType)

    MERGE_ARRAY_PROPERTY(NonInterrupting)
    MERGE_ARRAY_PROPERTY(IgnoreCooldown)
    MERGE_ARRAY_PROPERTY(AutoToggleOffWhenDisabled)
    MERGE_ARRAY_PROPERTY(AutoToggleOffWithTriggeredManaCost)
    MERGE_ARRAY_PROPERTY(NoStopAnim)
    MERGE_ARRAY_PROPERTY(NoResponse)
    MERGE_ARRAY_PROPERTY(NoVoiceResponse)
    MERGE_ARRAY_PROPERTY(NeedVision)
    MERGE_ARRAY_PROPERTY(NoTurnToTarget)

    MERGE_ARRAY_PROPERTY(ActivateScheme)
    MERGE_ARRAY_PROPERTY(CarryScheme)
    MERGE_ARRAY_PROPERTY(CloneScheme)
    MERGE_ARRAY_PROPERTY(ChargeCost)

    MERGE_ARRAY_PROPERTY(AttackEffectType)
    MERGE_ARRAY_PROPERTY(AttackDamageType)

    MERGE_ARRAY_PROPERTY(DoubleActivate)
ENTITY_DEF_MERGE_END
