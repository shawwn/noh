// (C)2008 S2 Games
// c_xmlproc_projectile.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_projectiledefinition.h"
#include "i_projectile.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CProjectileDefinition, ENTITY_BASE_TYPE_PROJECTILE, Projectile)

START_ENTITY_DEFINITION_XML_PROCESSOR(IProjectile, Projectile)
	READ_ENTITY_DEFINITION_PROPERTY(Speed, speed)
	READ_ENTITY_DEFINITION_PROPERTY(Gravity, gravity)
	READ_ENTITY_DEFINITION_PROPERTY(Arc, arc)
	READ_ENTITY_DEFINITION_PROPERTY(Lifetime, lifetime)
	READ_ENTITY_DEFINITION_PROPERTY(Model, model)
	READ_ENTITY_DEFINITION_PROPERTY(TrailEffect, traileffect)
	READ_ENTITY_DEFINITION_PROPERTY(DeathEffect, deatheffect)
	READ_ENTITY_DEFINITION_PROPERTY(ImpactEffect, impacteffect)
	READ_ENTITY_DEFINITION_PROPERTY(InvalidEffect, invalideffect)
	READ_ENTITY_DEFINITION_PROPERTY_EX(ModelScale, modelscale, 1.0)
	READ_ENTITY_DEFINITION_PROPERTY_EX(EffectScale, effectscale, 1.0)
	READ_ENTITY_DEFINITION_PROPERTY_EX(CanTurn, canturn, true)
	READ_ENTITY_DEFINITION_PROPERTY(UseExactLifetime, useexactlifetime)
	READ_ENTITY_DEFINITION_PROPERTY(Flying, flying)
	READ_ENTITY_DEFINITION_PROPERTY(Pathing, pathing)
	READ_ENTITY_DEFINITION_PROPERTY(FlyHeight, flyheight)
	READ_ENTITY_DEFINITION_PROPERTY(TouchRadius, touchradius)
	READ_ENTITY_DEFINITION_PROPERTY(TouchRadiusDirAdjust, touchradiusdiradjust)
	READ_ENTITY_DEFINITION_PROPERTY(TouchTargetScheme, touchtargetscheme)
	READ_ENTITY_DEFINITION_PROPERTY(TouchEffectType, toucheffecttype)
	READ_ENTITY_DEFINITION_PROPERTY(TouchIgnoreInvulnerable, touchignoreinvulnerable)
	READ_ENTITY_DEFINITION_PROPERTY(MaxTouches, maxtouches)
	READ_ENTITY_DEFINITION_PROPERTY(MaxTouchesPerTarget, maxtouchespertarget)
	READ_ENTITY_DEFINITION_PROPERTY(TouchCliffs, touchcliffs)

	READ_ENTITY_DEFINITION_PROPERTY(InitialCharges, initialcharges)
	READ_ENTITY_DEFINITION_PROPERTY(MaxCharges, maxcharges)
	READ_ENTITY_DEFINITION_PROPERTY(ImpactDistance, impactdistance)
	READ_ENTITY_DEFINITION_PROPERTY(ImpactStealth, impactstealth)
	READ_ENTITY_DEFINITION_PROPERTY(Curve, curve)

	READ_ENTITY_DEFINITION_PROPERTY_EX(Unitwalking, unitwalking, true)
	READ_ENTITY_DEFINITION_PROPERTY(Treewalking, treewalking)
	READ_ENTITY_DEFINITION_PROPERTY(Cliffwalking, cliffwalking)
	READ_ENTITY_DEFINITION_PROPERTY(Buildingwalking, buildingwalking)
END_ENTITY_DEFINITION_XML_PROCESSOR(Projectile, projectile)
