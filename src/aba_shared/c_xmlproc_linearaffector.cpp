// (C)2008 S2 Games
// c_xmlproc_linearaffector.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_linearaffectordefinition.h"
#include "c_linearaffector.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CLinearAffectorDefinition, ENTITY_BASE_TYPE_LINEAR_AFFECTOR, LinearAffector)

START_ENTITY_DEFINITION_XML_PROCESSOR(CLinearAffector, LinearAffector)
    READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(Radius, radius)
    READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(MinLength, minlength)
    READ_ENTITY_DEFINITION_MULTI_LEVEL_TEMPORAL_PROPERTY(MaxLength, maxlength)

    READ_ENTITY_DEFINITION_PROPERTY(Lifetime, lifetime)
    READ_ENTITY_DEFINITION_PROPERTY(ImpactDelay, impactdelay)
    READ_ENTITY_DEFINITION_PROPERTY(ImpactInterval, impactinterval)
    READ_ENTITY_DEFINITION_PROPERTY(MaxIntervals, maxintervals)
    READ_ENTITY_DEFINITION_PROPERTY(MaxTotalImpacts, maxtotalimpacts)
    READ_ENTITY_DEFINITION_PROPERTY(MaxImpactsPerInterval, maximpactsperinterval)
    READ_ENTITY_DEFINITION_PROPERTY(MaxImpactsPerTarget, maximpactspertarget)
    READ_ENTITY_DEFINITION_PROPERTY_EX(MaxImpactsPerTargetPerInterval, maximpactspertargetperinterval, 1)
    READ_ENTITY_DEFINITION_PROPERTY(SubSegmentOffset, subsegmentoffset)
    READ_ENTITY_DEFINITION_PROPERTY(SubSegmentLength, subsegmentlength)
    READ_ENTITY_DEFINITION_PROPERTY(TargetSelection, targetselection)
    READ_ENTITY_DEFINITION_PROPERTY(TargetScheme, targetscheme)
    READ_ENTITY_DEFINITION_PROPERTY(EffectType, effecttype)
    READ_ENTITY_DEFINITION_PROPERTY(Persist, persist)
    READ_ENTITY_DEFINITION_PROPERTY(DestroyTrees, destroytrees)

    READ_ENTITY_DEFINITION_PROPERTY(Effect, effect)
    READ_ENTITY_DEFINITION_PROPERTY(ImpactEffect, impacteffect)
    READ_ENTITY_DEFINITION_PROPERTY(BridgeEffect, bridgeeffect)
    READ_ENTITY_DEFINITION_PROPERTY(LinkEffect, linkeffect)
END_ENTITY_DEFINITION_XML_PROCESSOR(LinearAffector, linearaffector)

ENTITY_DEF_MERGE_START(CLinearAffectorDefinition, IEntityDefinition)
    MERGE_TEMPORAL_ARRAY_PROPERTY(Radius)
    MERGE_TEMPORAL_ARRAY_PROPERTY(MinLength)
    MERGE_TEMPORAL_ARRAY_PROPERTY(MaxLength)

    MERGE_ARRAY_PROPERTY(Lifetime)
    MERGE_ARRAY_PROPERTY(ImpactDelay)
    MERGE_ARRAY_PROPERTY(ImpactInterval)
    MERGE_ARRAY_PROPERTY(MaxIntervals)
    MERGE_ARRAY_PROPERTY(MaxTotalImpacts)
    MERGE_ARRAY_PROPERTY(MaxImpactsPerInterval)
    MERGE_ARRAY_PROPERTY(MaxImpactsPerTarget)
    MERGE_ARRAY_PROPERTY(MaxImpactsPerTargetPerInterval)
    MERGE_ARRAY_PROPERTY(SubSegmentOffset)
    MERGE_ARRAY_PROPERTY(SubSegmentLength)
    MERGE_ARRAY_PROPERTY(TargetSelection)
    MERGE_ARRAY_PROPERTY(TargetScheme)
    MERGE_ARRAY_PROPERTY(EffectType)
    MERGE_ARRAY_PROPERTY(Persist)
    MERGE_ARRAY_PROPERTY(DestroyTrees)

    MERGE_RESOURCE_PROPERTY(Effect)
    MERGE_RESOURCE_PROPERTY(ImpactEffect)
    MERGE_RESOURCE_PROPERTY(BridgeEffect)
    MERGE_RESOURCE_PROPERTY(LinkEffect)
ENTITY_DEF_MERGE_END
