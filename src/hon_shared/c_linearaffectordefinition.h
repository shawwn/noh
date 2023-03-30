// (C)2009 S2 Games
// c_linearaffectordefinition.h
//
//=============================================================================
#ifndef __C_LINEARAFFECTORDEFINITION_H__
#define __C_LINEARAFFECTORDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(CLinearAffector, LinearAffector, linearaffector)
//=============================================================================

//=============================================================================
// CLinearAffectorDefinition
//=============================================================================
class CLinearAffectorDefinition : public IEntityDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_ARRAY_PROPERTY(Lifetime, uint)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(Radius, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(MinLength, float)
	ENT_DEF_TEMPORAL_ARRAY_PROPERTY(MaxLength, float)
	ENT_DEF_ARRAY_PROPERTY(ImpactDelay, uint)
	ENT_DEF_ARRAY_PROPERTY(ImpactInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxIntervals, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxTotalImpacts, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerTarget, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxImpactsPerTargetPerInterval, uint)
	ENT_DEF_ARRAY_PROPERTY(SubSegmentLength, float)
	ENT_DEF_ARRAY_PROPERTY(SubSegmentOffset, float)
	ENT_DEF_ARRAY_PROPERTY(TargetSelection, ETargetSelection)
	ENT_DEF_ARRAY_PROPERTY_EX(TargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(EffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(Persist, bool)
	ENT_DEF_ARRAY_PROPERTY(DestroyTrees, bool)

	ENT_DEF_RESOURCE_PROPERTY(Effect, Effect)
	ENT_DEF_RESOURCE_PROPERTY(ImpactEffect, Effect)
	ENT_DEF_RESOURCE_PROPERTY(BridgeEffect, Effect)
	ENT_DEF_RESOURCE_PROPERTY(LinkEffect, Effect)

protected:
	virtual void	PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
	{
		IEntityDefinition::PrecacheV(eScheme, sModifier);

		PRECACHE_GUARD
			PrecacheEffect();
			PrecacheImpactEffect();
			PrecacheBridgeEffect();
			PrecacheLinkEffect();
		PRECACHE_GUARD_END
	}

	virtual void	GetPrecacheListV(EPrecacheScheme eScheme, const tstring &sModifier, HeroPrecacheList &deqPrecache)
	{
		IEntityDefinition::GetPrecacheListV(eScheme, sModifier, deqPrecache);

		PRECACHE_GUARD
			// ...
		PRECACHE_GUARD_END
	}

public:
	~CLinearAffectorDefinition()	{}
	CLinearAffectorDefinition() :
	IEntityDefinition(&g_allocatorLinearAffector)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(ctx_Game,    CLinearAffectorDefinition)(*this); }

	virtual void	ImportDefinition(IEntityDefinition *pOtherDefinition);
};
//=============================================================================

#endif //__C_LINEARAFFECTORDEFINITION_H__
