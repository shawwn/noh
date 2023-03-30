// (C)2008 S2 Games
// c_projectiledefinition.h
//
//=============================================================================
#ifndef __C_PROJECTILEDEFINITION_H__
#define __C_PROJECTILEDEFINITION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitydefinition.h"

#include "../k2/i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
DECLARE_ENTITY_DEFINITION_XML_PROCESSOR(IProjectile, Projectile, projectile)
//=============================================================================

//=============================================================================
// CProjectileDefinition
//=============================================================================
class CProjectileDefinition : public IEntityDefinition
{
	DECLARE_DEFINITION_TYPE_INFO

	ENT_DEF_ARRAY_PROPERTY(Speed, float)
	ENT_DEF_ARRAY_PROPERTY(Gravity, float)
	ENT_DEF_ARRAY_PROPERTY(Arc, float)
	ENT_DEF_ARRAY_PROPERTY(Lifetime, uint)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(Model, Model)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(TrailEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(DeathEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(ImpactEffect, Effect)
	ENT_DEF_RESOURCE_ARRAY_PROPERTY(InvalidEffect, Effect)
	ENT_DEF_ARRAY_PROPERTY(ModelScale, float)
	ENT_DEF_ARRAY_PROPERTY(EffectScale, float)
	ENT_DEF_ARRAY_PROPERTY(CanTurn, bool)
	ENT_DEF_ARRAY_PROPERTY(UseExactLifetime, bool)
	ENT_DEF_ARRAY_PROPERTY(Flying, bool)
	ENT_DEF_ARRAY_PROPERTY(Pathing, bool)
	ENT_DEF_ARRAY_PROPERTY(FlyHeight, float)
	ENT_DEF_ARRAY_PROPERTY(TouchRadius, float)
	ENT_DEF_ARRAY_PROPERTY(TouchRadiusDirAdjust, bool)
	ENT_DEF_ARRAY_PROPERTY_EX(TouchTargetScheme, uint, Game.LookupTargetScheme)
	ENT_DEF_ARRAY_PROPERTY_EX(TouchEffectType, uint, Game.LookupEffectType)
	ENT_DEF_ARRAY_PROPERTY(TouchIgnoreInvulnerable, bool)
	ENT_DEF_ARRAY_PROPERTY(MaxTouches, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxTouchesPerTarget, uint)
	ENT_DEF_ARRAY_PROPERTY(TouchCliffs, bool)
	ENT_DEF_ARRAY_PROPERTY(InitialCharges, uint)
	ENT_DEF_ARRAY_PROPERTY(MaxCharges, uint)
	ENT_DEF_ARRAY_PROPERTY(ImpactDistance, float)
	ENT_DEF_ARRAY_PROPERTY(ImpactStealth, bool)
	ENT_DEF_ARRAY_PROPERTY(Curve, float)
	ENT_DEF_ARRAY_PROPERTY(Unitwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Treewalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Cliffwalking, bool)
	ENT_DEF_ARRAY_PROPERTY(Buildingwalking, bool)

protected:
	virtual void	PrecacheV(EPrecacheScheme eScheme, const tstring &sModifier)
	{
		IEntityDefinition::PrecacheV(eScheme, sModifier);

		PRECACHE_GUARD
			PrecacheModel();
			PrecacheTrailEffect();
			PrecacheDeathEffect();
			PrecacheImpactEffect();
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
	~CProjectileDefinition()	{}
	CProjectileDefinition() :
	IEntityDefinition(&g_allocatorProjectile)
	{}

	IEntityDefinition*	GetCopy() const	{ return K2_NEW(ctx_Game,    CProjectileDefinition)(*this); }
};
//=============================================================================

#endif //__C_PROJECTILEDEFINITION_H__
