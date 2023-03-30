// (C)2008 S2 Games
// c_affectordefinition.h
//
//=============================================================================
#ifndef __C_AFFECTORDEFINITION_H__
#define __C_AFFECTORDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IAffector, Affector, affector)
//=============================================================================

//=============================================================================
// CAffectorDefinition
//=============================================================================
class CAffectorDefinition : public IEntityDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_ARRAY_PROPERTY(Lifetime, uint)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Radius, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(InnerRadiusOffset, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Arc, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Angle, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Speed, float)
	ENT_DEF_ARRAY_PROPERTY(ImpactDelay, uint)
	ENT_DEF_ARRAY_PROPERTY(ImpactInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxIntervals, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxTotalImpacts, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerTarget, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerTargetPerInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(TargetSelection, ETargetSelection)
	ENT_DEF_ARRAY_PROPERTY_EX(TargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(EffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(IgnoreInvulnerable, bool)
	ENT_DEF_ARRAY_PROPERTY(Persist, bool)
	ENT_DEF_ARRAY_PROPERTY(DestroyTrees, bool)
	ENT_DEF_ARRAY_PROPERTY(CanTurn, bool)

	ENT_DEF_RESOURCE_ARRAY_PROPERTY(Effect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(ImpactEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(BridgeEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(LinkEffect, Effect)

public:
	~CAffectorDefinition()	{}
	CAffectorDefinition() :
	IEntityDefinition(&g_allocatorAffector)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(g_heapResources,    CAffectorDefinition)(*this); }

	virtual void	Precache(EPrecacheScheme eScheme)
	{
		IEntityDefinition::Precache(eScheme);

		PRECACHE_GUARD

			PrecacheEffect();
			PrecacheImpactEffect();
			PrecacheBridgeEffect();
			PrecacheLinkEffect();

		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
	{
		IEntityDefinition::GetPrecacheList(eScheme, deqPrecache);

		PRECACHE_GUARD
			deqPrecache.push_back(SHeroPrecache(GetName(), eScheme));
		PRECACHE_GUARD_END
	}

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_AFFECTORDEFINITION_H__
