// (C)2007 S2 Games
// c_gadgetsteamturret.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetsteamturret.h"

#include "../k2/intersection.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, SteamTurret)
//=============================================================================

/*====================
  CGadgetSteamTurret::CEntityConfig::CEntityConfig
  ====================*/
CGadgetSteamTurret::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Range, 500.0f),
INIT_ENTITY_CVAR(AttackTime, 1500),
INIT_ENTITY_CVAR(AttackOffset, V_ZERO),
INIT_ENTITY_CVAR(SpreadX, 0.0f),
INIT_ENTITY_CVAR(SpreadY, 0.0f),
INIT_ENTITY_CVAR(MinDamage, 50.0f),
INIT_ENTITY_CVAR(MaxDamage, 50.0f),
INIT_ENTITY_CVAR(DamageRadius, 0.0f),
INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f),
INIT_ENTITY_CVAR(FiringAnimName, _T("")),
INIT_ENTITY_CVAR(TraceEffectPath, _T("")),
INIT_ENTITY_CVAR(AttackEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactTerrainEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactEffectPath, _T("")),
INIT_ENTITY_CVAR(BurstCount, 5),
INIT_ENTITY_CVAR(BurstTime, 80)
{
}


/*====================
  CGadgetSteamTurret::Baseline
  ====================*/
void	CGadgetSteamTurret::Baseline()
{
	IGadgetEntity::Baseline();
	m_auiCounter[0] = 0;
	m_auiCounter[1] = 0;
}


/*====================
  CGadgetSteamTurret::ValidateTarget
  ====================*/
bool	CGadgetSteamTurret::ValidateTarget(uint uiTargetIndex)
{
	if (uiTargetIndex == INVALID_INDEX)
		return false;

	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	if (pTarget == NULL)
		return false;

	// Check status
	if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE && ((!pTarget->IsBuilding() && !pTarget->IsGadget()) || pTarget->GetStatus() != ENTITY_STATUS_SPAWNING))
		return false;
	if (!IsEnemy(pTarget))
		return false;
	if (!LooksLikeEnemy(pTarget) && !pTarget->HasNetFlags(ENT_NET_FLAG_REVEALED))
		return false;
	if ((pTarget->IsStealthed() && !pTarget->HasNetFlags(ENT_NET_FLAG_REVEALED)) || pTarget->IsIntangible())
		return false;
	if (!pTarget->AIShouldTarget())
		return false;

	// Check range
	CBBoxf bbBoundsWorld(pTarget->GetBounds());
	bbBoundsWorld.Transform(pTarget->GetPosition(), CAxis(pTarget->GetAngles()), pTarget->GetScale());
	if (!I_SphereBoundsIntersect(CSphere(GetPosition(), m_pEntityConfig->GetRange()), bbBoundsWorld))
		return false;

	// Check vision
	CVec3f v3Start(m_v3Position + m_pEntityConfig->GetAttackOffset());
	CVec3f v3End(pTarget->GetPosition() + pTarget->GetBounds().GetMid());
	STraceInfo trace;
	Game.TraceLine(trace, v3Start, v3End, TRACE_PROJECTILE, GetWorldIndex());

	if (trace.uiEntityIndex == INVALID_INDEX)
		return false;

	IVisualEntity *pActualTarget(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));

	if (pActualTarget == NULL || pActualTarget->GetIndex() != pTarget->GetIndex())
		return false;

	return true;
}


/*====================
  CGadgetSteamTurret::ServerFrame
  ====================*/
bool	CGadgetSteamTurret::ServerFrame()
{
	if (!IGadgetEntity::ServerFrame())
		return false;

	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return true;

	// Select target
	float fShortestDistance(FAR_AWAY);
	if (!ValidateTarget(m_uiTarget))
	{
		m_uiTarget = INVALID_INDEX;

		uivector vSight;
		Game.GetEntitiesInRadius(vSight, CSphere(GetPosition(), m_pEntityConfig->GetRange()), 0);
		for (uivector_it it(vSight.begin()); it != vSight.end(); ++it)
		{
			IVisualEntity *pTarget(Game.GetEntityFromWorldIndex(*it));
			if (pTarget == NULL)
				continue;
			if (!ValidateTarget(pTarget->GetIndex()))
				continue;

			CVec3f v3Dist(pTarget->GetPosition() - GetPosition());
			float fDistance(v3Dist.Length());
			if (fDistance < fShortestDistance)
			{
				m_uiTarget = pTarget->GetIndex();
				fShortestDistance = fDistance;
			}
		}
	}

	IVisualEntity *pTarget(Game.GetVisualEntity(m_uiTarget));
	if (pTarget == NULL)
		return true;

	// Face current target
	CVec3f v3Start(m_v3Position + m_pEntityConfig->GetAttackOffset());
	CVec3f v3Aim(pTarget->GetPosition() + pTarget->GetBounds().GetMid());
	CVec3f v3Forward(Normalize(v3Aim - v3Start));
	CVec3f v3Angles(M_GetAnglesFromForwardVec(v3Forward));
	v3Angles[YAW] -= GetAngles()[YAW];
	v3Angles[YAW] += 180.0f;
	SetViewAngles(v3Angles);

	if (m_uiNextAttackTime > Game.GetGameTime())
		return true;

	// Attack the target
	CAxis axis(GetAxisFromForwardVec(v3Forward));
	const CVec3f &v3Right(axis.Right());
	const CVec3f &v3Up(axis.Up());

	CVec3f v3Rand;
	do
	{
		v3Rand.x = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
		v3Rand.y = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
		v3Rand.z = v3Rand.x * v3Rand.x + v3Rand.y * v3Rand.y;
	}
	while (v3Rand.z > 1.0f);

	CVec2f v2Spread(m_pEntityConfig->GetSpreadX(), m_pEntityConfig->GetSpreadY());
	CVec3f v3Dir(Normalize(v3Forward + v3Right * (v3Rand.x * v2Spread.x) + v3Up * (v3Rand.y * v2Spread.y)));
	CVec3f v3End(v3Start + v3Dir * (m_pEntityConfig->GetRange()));

	FireTrace(v3Start, v3End);

	// Firing Effect
	if (!m_pEntityConfig->GetAttackEffectPath().empty() && Game.IsServer())
	{
		CGameEvent evFire;
		evFire.SetSourcePosition(v3Start);
		evFire.SetSourceAngles(M_GetAnglesFromForwardVec(v3Forward));
		evFire.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetAttackEffectPath()));
		Game.AddEvent(evFire);
	}

	if (m_uiCurrentBurstCount <= m_pEntityConfig->GetBurstCount() - 1)
	{
		m_uiNextAttackTime = Game.GetGameTime() + m_pEntityConfig->GetBurstTime();
		++m_uiCurrentBurstCount;
	}
	else
	{
		m_uiNextAttackTime = Game.GetGameTime() + m_pEntityConfig->GetAttackTime();
		m_uiCurrentBurstCount = 0;
	}
	
	StartAnimation(m_pEntityConfig->GetFiringAnimName(), 0);
	return true;
}


/*====================
  CGadgetSteamTurret::FireTrace
  ====================*/
void	CGadgetSteamTurret::FireTrace(const CVec3f &v3Start, const CVec3f &v3End)
{
	++m_auiCounter[TURRET_COUNTER_SHOTS];

	// Do a trace
	STraceInfo trace;
	Game.TraceLine(trace, v3Start, v3End, SURF_HULL, GetWorldIndex());
	if (trace.uiEntityIndex != INVALID_INDEX)
	{
		IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));
		if (pEntity != NULL)
		{
			float fDamage(M_Randnum(m_pEntityConfig->GetMinDamage(), m_pEntityConfig->GetMaxDamage()));
			ICombatEntity *pCombatEnt(pEntity->GetAsCombatEnt());

			if (pEntity->IsBuilding())
				fDamage *= m_pEntityConfig->GetPierceBuilding();
			else if (pCombatEnt != NULL && pCombatEnt->GetIsSiege())
				fDamage *= m_pEntityConfig->GetPierceSiege();
			else if (pCombatEnt != NULL && pCombatEnt->GetIsHellbourne())
				fDamage *= m_pEntityConfig->GetPierceHellbourne();
			else if (pCombatEnt != NULL)
				fDamage *= m_pEntityConfig->GetPierceUnit();

			m_fDamageCounter += pEntity->Damage(fDamage, 0, this);
			m_auiCounter[TURRET_COUNTER_DAMAGE] = INT_ROUND(m_fDamageCounter);
			pEntity->Hit(trace.v3EndPos, trace.plPlane.v3Normal);	// FIXME: Put something meaningful here
		}
	}

	if (!m_pEntityConfig->GetTraceEffectPath().empty())
	{
		CGameEvent evTrace;
		evTrace.SetSourcePosition(v3Start);
		evTrace.SetTargetPosition(trace.v3EndPos);
		evTrace.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetTraceEffectPath()));
		Game.AddEvent(evTrace);
	}

	// Impact effect
	if (trace.fFraction >= 1.0f)
		return;

	ResHandle hImpactEffect(INVALID_RESOURCE);
	if (trace.uiSurfFlags & SURF_TERRAIN && !m_pEntityConfig->GetImpactTerrainEffectPath().empty())
		hImpactEffect = Game.RegisterEffect(m_pEntityConfig->GetImpactTerrainEffectPath());
	else if (!m_pEntityConfig->GetImpactEffectPath().empty())
		hImpactEffect = Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath());
	if (hImpactEffect == INVALID_RESOURCE)
		return;

	CGameEvent evDeath;
	evDeath.SetSourcePosition(trace.v3EndPos);
	evDeath.SetEffect(hImpactEffect);
	Game.AddEvent(evDeath);
}
