// (C)2007 S2 Games
// i_attackbuildingentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_attackbuildingentity.h"
#include "c_teaminfo.h"

#include "../k2/c_model.h"
#include "../k2/c_worldentity.h"
#include "../k2/intersection.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IAttackBuildingEntity::s_pvFields;

CVAR_FLOATR(	g_towerPreferenceDistance,	0.5f,	CVAR_GAMECONFIG,	0.0f,	1.0f);
CVAR_FLOATR(	g_towerPreferenceHealth,	0.5f,	CVAR_GAMECONFIG,	0.0f,	1.0f);
CVAR_FLOATR(	g_towerPreferenceDamage,	0.5f,	CVAR_GAMECONFIG,	0.0f,	1.0f);
CVAR_FLOATR(	g_towerPreferenceType,		0.5f,	CVAR_GAMECONFIG,	0.0f,	1.0f);
//=============================================================================

/*====================
  IAttackBuildingEntity::CEntityConfig::CEntityConfig
  ====================*/
IAttackBuildingEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IBuildingEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Projectile, _T("")),
INIT_ENTITY_CVAR(AttackTime, 1000),
INIT_ENTITY_CVAR(AttackTimeVariance, 150),
INIT_ENTITY_CVAR(Range, 1000.0f),
INIT_ENTITY_CVAR(MinDamage, 40.0f),
INIT_ENTITY_CVAR(MaxDamage, 50.0f),
INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f),
INIT_ENTITY_CVAR(DamageRadius, 0.0f),
INIT_ENTITY_CVAR(SpreadX, 0.0f),
INIT_ENTITY_CVAR(SpreadY, 0.0f),
INIT_ENTITY_CVAR(AttackOffset, CVec3f(0.0f, 0.0f, 100.0f)),
INIT_ENTITY_CVAR(TargetState, _T("")),
INIT_ENTITY_CVAR(TargetStateDuration, 0),
INIT_ENTITY_CVAR(TraceEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactTerrainEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactEffectPath, _T("")),
INIT_ENTITY_CVAR(AttackEffectPath, _T("")),
INIT_ENTITY_CVAR(SiegeTargetPreference, 0.5f),
INIT_ENTITY_CVAR(AlwaysTargetNpcs, false)
{
}


/*====================
  IAttackBuildingEntity::~IAttackBuildingEntity
  ====================*/
IAttackBuildingEntity::~IAttackBuildingEntity()
{	
}


/*====================
  IAttackBuildingEntity::IAttackBuildingEntity
  ====================*/
IAttackBuildingEntity::IAttackBuildingEntity(CEntityConfig *pConfig) :
IBuildingEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiNextSightTime(INVALID_TIME)
{
}


/*====================
  IAttackBuildingEntity::GetTypeVector
  ====================*/
const vector<SDataField>&	IAttackBuildingEntity::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		const vector<SDataField> &vBase(IBuildingEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
	}

	return *s_pvFields;
}


/*====================
  IAttackBuildingEntity::Spawn
  ====================*/
void	IAttackBuildingEntity::Spawn()
{
	IBuildingEntity::Spawn();

	m_uiNextAttackTime = 0;
	m_uiNextSightTime = Game.GetGameTime() + M_Randnum(800, 1200);
}


/*====================
  IAttackBuildingEntity::Damage
  ====================*/
float	IAttackBuildingEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
	return IBuildingEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);
}


/*====================
  IAttackBuildingEntity::DamageNotification
  ====================*/
void	IAttackBuildingEntity::DamageNotification(uint uiIndex, uint uiAttacker, float fDamage)
{
	IBuildingEntity *pBuilding(Game.GetBuildingEntity(uiIndex));
	if (pBuilding == NULL)
		return;
	if (Distance(pBuilding->GetPosition(), GetPosition()) > m_pEntityConfig->GetRange())
		return;

	map<uint, float>::iterator itFind(m_mapRecentDamage.find(uiAttacker));
	if (itFind == m_mapRecentDamage.end())
		m_mapRecentDamage[uiAttacker] = fDamage;
	else
		itFind->second += fDamage;
}


/*====================
  IAttackBuildingEntity::UpdateTargetPreference
  ====================*/
void	IAttackBuildingEntity::UpdateTargetPreference()
{
	static uivector s_vTargets;

	// First, find out what is in range
	s_vTargets.clear();
	map<uint, float> mapTargets;
	Game.GetEntitiesInRadius(s_vTargets, CSphere(GetPosition(), m_pEntityConfig->GetRange()), 0);

	// Add new potential targets to the map
	for (uivector_it it(s_vTargets.begin()); it != s_vTargets.end(); ++it)
	{
		uint uiGameIndex(Game.GetGameIndexFromWorldIndex(*it));
		IVisualEntity *pVisualTarget(Game.GetVisualEntity(uiGameIndex));
		if (pVisualTarget == NULL)
			continue;

		if (!ShouldTarget(pVisualTarget))
			continue;

		float fTargetWeight(1.0f);

		// Prefer targets that are closer
		CVec3f v3Start(GetPosition() + m_pEntityConfig->GetAttackOffset());
		CVec3f v3Target(pVisualTarget->GetPosition() + pVisualTarget->GetBounds().GetMid());
		float fDistance(Distance(v3Start, v3Target));
		if (fDistance > m_pEntityConfig->GetRange())
			continue;
		fTargetWeight *= (1.0f - g_towerPreferenceDistance) +  g_towerPreferenceDistance * (1.0f - CLAMP((fDistance / m_pEntityConfig->GetRange()), 0.0f, 1.0f));

		// Filter targets that are obstructed
		STraceInfo trace;
		Game.TraceLine(trace, v3Start, v3Target, 0, GetWorldIndex());
		if (trace.uiEntityIndex != pVisualTarget->GetWorldIndex())
			continue;

		// Prefer targets that are easier to kill
		ICombatEntity *pCombatTarget(pVisualTarget->GetAsCombatEnt());
		float fTargetHealth(pVisualTarget->GetHealth());
		float fAverageDamage((m_pEntityConfig->GetMinDamage() + m_pEntityConfig->GetMaxDamage()) / 2.0f);
		if (pVisualTarget->IsBuilding())
			fAverageDamage *= m_pEntityConfig->GetPierceBuilding();
		else if (pCombatTarget != NULL && pCombatTarget->GetIsHellbourne())
			fAverageDamage *= m_pEntityConfig->GetPierceHellbourne();
		else if (pCombatTarget != NULL && pCombatTarget->GetIsSiege())
			fAverageDamage *= m_pEntityConfig->GetPierceSiege();
		else
			fAverageDamage *= m_pEntityConfig->GetPierceUnit();
		
		if (pCombatTarget != NULL)
			fAverageDamage -= (pCombatTarget->GetArmorDamageReduction(pCombatTarget->GetArmor()) * fAverageDamage);
		else if (pVisualTarget->IsBuilding())
			fAverageDamage -= (pVisualTarget->GetAsBuilding()->GetArmorDamageReduction() * fAverageDamage);

		float fNumHits(fTargetHealth / fAverageDamage);
		fTargetWeight *= (1.0f - g_towerPreferenceHealth) +  g_towerPreferenceHealth * CLAMP(1.0f / fNumHits, 0.0f, 1.0f);

		// Prefer targets that have done the most damage
		float fDamage(0.0f);
		map<uint, float>::iterator itFind(m_mapRecentDamage.find(pVisualTarget->GetIndex()));
		if (itFind != m_mapRecentDamage.end())
			fDamage = itFind->second;
		fTargetWeight *= (1.0f - g_towerPreferenceDamage) +  g_towerPreferenceDamage * (1.0f - CLAMP(1.0f / fDamage, 0.0f, 1.0f));

		// Prefer either siege or non-siege
		if (pCombatTarget != NULL && pCombatTarget->GetIsSiege())
			fTargetWeight *= (1.0f - g_towerPreferenceType) +  g_towerPreferenceType * CLAMP(m_pEntityConfig->GetSiegeTargetPreference().GetFloat(), 0.0f, 1.0f);
		else
			fTargetWeight *= (1.0f - g_towerPreferenceType) +  g_towerPreferenceType * (1.0f - CLAMP(m_pEntityConfig->GetSiegeTargetPreference().GetFloat(), 0.0f, 1.0f));
		
		mapTargets[uiGameIndex] = fTargetWeight;
	}

	// Select target
	float fBestTargetWeight(-1.0f);
	uint uiBestTargetIndex(INVALID_INDEX);
	for (map<uint, float>::iterator it(mapTargets.begin()); it != mapTargets.end(); ++it)
	{
		if (it->second > fBestTargetWeight)
		{
			uiBestTargetIndex = it->first;
			fBestTargetWeight = it->second;
		}
	}

	m_uiTarget = uiBestTargetIndex;
}


/*====================
  IAttackBuildingEntity::FireProjectile
  ====================*/
void	IAttackBuildingEntity::FireProjectile(const tstring &sName, const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v3Dir, const CVec3f &v3TargetVelocity)
{
	// Spawn a projectile
	IGameEntity *pNewEnt(Game.AllocateEntity(sName));
	if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
		Console.Warn << _T("Failed to spawn projectile: ") << sName << newl;

	IProjectile *pProjectile(pNewEnt->GetAsProjectile());

	float fTime(Distance(v3Start, v3End) / pProjectile->GetSpeed());

	pProjectile->SetOwner(GetIndex());
	pProjectile->SetOrigin(v3Start);
	pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Dir));
	pProjectile->SetVelocity(v3Dir * pProjectile->GetSpeed() + CVec3f(0.0f, 0.0f, (p_gravity * pProjectile->GetGravity()) * (fTime / 2.0f)) + v3TargetVelocity);
	pProjectile->SetOriginTime(Game.GetServerTime() + Game.GetServerFrameLength());
	pProjectile->SetDamage(m_pEntityConfig->GetMinDamage(), m_pEntityConfig->GetMaxDamage(), m_pEntityConfig->GetDamageRadius(),
						m_pEntityConfig->GetPierceUnit(), m_pEntityConfig->GetPierceHellbourne(), m_pEntityConfig->GetPierceSiege(), m_pEntityConfig->GetPierceBuilding());
	pProjectile->SetTargetState(EntityRegistry.LookupID(m_pEntityConfig->GetTargetState()));
	pProjectile->SetTargetStateDuration(m_pEntityConfig->GetTargetStateDuration());
	pProjectile->Spawn();
}


/*====================
  IAttackBuildingEntity::FireTrace
  ====================*/
void	IAttackBuildingEntity::FireTrace(const CVec3f &v3Start, const CVec3f &v3End)
{
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

			pEntity->Damage(fDamage, 0, this);
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


/*====================
  IAttackBuildingEntity::ServerFrame
  ====================*/
bool	IAttackBuildingEntity::ServerFrame()
{
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return IBuildingEntity::ServerFrame();

	if (Game.GetGameTime() - m_uiLastDamageTime > SecToMs(30u))
		m_mapRecentDamage.clear();

	// Check attack time
	if (Game.GetGameTime() < m_uiNextAttackTime)
		return IBuildingEntity::ServerFrame();

	// Find a target
	if (Game.GetGameTime() >= m_uiNextSightTime)
	{
		UpdateTargetPreference();
		m_uiNextSightTime = Game.GetGameTime() + M_Randnum(500, 1000);
	}
	IVisualEntity *pTarget(Game.GetVisualEntity(m_uiTarget));
	if (pTarget == NULL)
		return IBuildingEntity::ServerFrame();

	// Determine attack vector
	CVec3f v3Start(m_v3Position + m_pEntityConfig->GetAttackOffset());
	CVec3f v3Aim(pTarget->GetPosition() + pTarget->GetBounds().GetMid());
	CVec3f v3Forward(Normalize(v3Aim - v3Start));
	
	CAxis axis(GetAxisFromForwardVec(v3Forward));
	const CVec3f &v3Right(axis.Right());
	const CVec3f &v3Up(axis.Up());

	CVec3f v3Rand;
	do
	{
		v3Rand.x = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
		v3Rand.y = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
		v3Rand.z = v3Rand.x * v3Rand.x + v3Rand.y * v3Rand.y;
	} while (v3Rand.z > 1.0f);

	CVec2f v2Spread(m_pEntityConfig->GetSpreadX(), m_pEntityConfig->GetSpreadY());
	CVec3f v3Dir(Normalize(v3Forward + v3Right * (v3Rand.x * v2Spread.x) + v3Up * (v3Rand.y * v2Spread.y)));
	CVec3f v3End(v3Start + v3Dir * m_pEntityConfig->GetRange());

	// Execute the attack
	tstring sProjectileName(m_pEntityConfig->GetProjectile());
	if (!sProjectileName.empty())
		FireProjectile(sProjectileName, v3Start, v3End, v3Dir, pTarget->GetVelocity());
	else
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

	// Set next attack time
	int iVariance(m_pEntityConfig->GetAttackTimeVariance() / 2);
	m_uiNextAttackTime = Game.GetGameTime() + m_pEntityConfig->GetAttackTime() + M_Randnum(-iVariance, iVariance);

	return IBuildingEntity::ServerFrame();
}


/*====================
  IAttackBuildingEntity::ShouldTarget
  ====================*/
bool	IAttackBuildingEntity::ShouldTarget(IGameEntity *pOther)
{
	if (!pOther->IsVisual())
		return false;

	IVisualEntity *pVisual(pOther->GetAsVisualEnt());

	if (pVisual->IsNpc() && !m_pEntityConfig->GetAlwaysTargetNpcs())
	{
		INpcEntity *pNpc(pOther->GetAsNpc());
		IVisualEntity *pTarget(Game.GetVisualEntity(pNpc->GetTargetIndex()));

		if (pTarget == NULL || pTarget->GetTeam() != GetTeam())
			return false;
	}

	// Allow polymorphed players that have damaged this tower within 10 seconds to be viable targets
	if (!LooksLikeEnemy(pVisual) && !pVisual->HasNetFlags(ENT_NET_FLAG_REVEALED))
	{
		map<uint, SDamageRecord>::iterator findIt(m_mapDamage.find(pVisual->GetIndex()));

		if (findIt == m_mapDamage.end() || Game.GetGameTime() - findIt->second.uiTime > 10000)
			return false;
	}

	if (pOther->IsCombat() &&
		IsEnemy(pVisual) &&
		pVisual->GetStatus() == ENTITY_STATUS_ACTIVE &&
		(!pVisual->IsStealthed() || pVisual->HasNetFlags(ENT_NET_FLAG_REVEALED)) &&
		pVisual->AIShouldTarget())
		return true;

	return false;
}


/*====================
  IAttackBuildingEntity::ClientPrecache
  ====================*/
void	IAttackBuildingEntity::ClientPrecache(CEntityConfig *pConfig)
{
	IBuildingEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTraceEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetTraceEffectPath(), RES_EFFECT);

	if (!pConfig->GetImpactTerrainEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetImpactTerrainEffectPath(), RES_EFFECT);

	if (!pConfig->GetImpactEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT);

	if (!pConfig->GetAttackEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetAttackEffectPath(), RES_EFFECT);

	if (!pConfig->GetProjectile().empty())
		EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetProjectile()));
}


/*====================
  IAttackBuildingEntity::ServerPrecache
  ====================*/
void	IAttackBuildingEntity::ServerPrecache(CEntityConfig *pConfig)
{
	IBuildingEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTraceEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTraceEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetImpactTerrainEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactTerrainEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetImpactEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetAttackEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetAttackEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetProjectile().empty())
		EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetProjectile()));

}
