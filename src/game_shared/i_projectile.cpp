// (C)2006 S2 Games
// i_projectile.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_projectile.h"

#include "../k2/s_traceinfo.h"
#include "../k2/c_world.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarf	p_gravity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IProjectile::s_pvFields;

uint				IProjectile::s_uiStartTime;
uint				IProjectile::s_uiFrameTime;
uint				IProjectile::s_uiElapsed;
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_UINTF(		g_projectileStickTime,		SecToMs(5u),	CVAR_GAMECONFIG);
//=============================================================================

/*====================
  IProjectile::CEntityConfig::CEntityConfig
  ====================*/
IProjectile::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Speed, 500.0f),
INIT_ENTITY_CVAR(LifeTime, 5000),
INIT_ENTITY_CVAR(Gravity, 0.0f),
INIT_ENTITY_CVAR(Bounce, 0.0f),
INIT_ENTITY_CVAR(Friction, 0.0f),
INIT_ENTITY_CVAR(TrailEffectPath, _T("")),
INIT_ENTITY_CVAR(DeathEffectPath, _T("")),
INIT_ENTITY_CVAR(BounceEffectPath, _T("")),
INIT_ENTITY_CVAR(Stick, false),
INIT_ENTITY_CVAR(MinStickDistance, 0.0f),
INIT_ENTITY_CVAR(MaxStickDistance, 0.0f),
INIT_ENTITY_CVAR(Turn, false)
{
}


/*====================
  IProjectile::IProjectile
  ====================*/
IProjectile::IProjectile(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_unWeaponOriginID(INVALID_ENT_TYPE),
m_uiDirectDamageEntityIndex(INVALID_INDEX),

m_pEntityConfig(pConfig),

m_uiLastUpdateTime(INVALID_TIME),
m_uiOwnerIndex(INVALID_INDEX),
m_pImpactEntity(NULL),
m_fMinDamage(0.0f),
m_fMaxDamage(0.0f),
m_fUnitPierce(1.0f),
m_fHellbournePierce(1.0f),
m_fSiegePierce(1.0f),
m_fBuildingPierce(1.0f),
m_fDamageRadius(0.0f),
m_unTargetState(INVALID_ENT_TYPE),
m_uiTargetStateDuration(0),
m_bStuck(false),
m_uiStickTime(INVALID_TIME),
m_iDamageFlags(0),
m_uiLastBounceTime(INVALID_TIME)
{
}


/*====================
  IProjectile::Baseline
  ====================*/
void	IProjectile::Baseline()
{
	m_yStatus = ENTITY_STATUS_ACTIVE;
	m_v3Origin = V3_ZERO;
	m_v3Angles = V3_ZERO;
	m_v3Velocity = V3_ZERO;
	m_uiOriginTime = 0;
}


/*====================
  IProjectile::GetSnapshot
  ====================*/
void	IProjectile::GetSnapshot(CEntitySnapshot &snapshot) const
{
	snapshot.AddField(m_yStatus);
	snapshot.AddRoundPosition(m_v3Origin);
	snapshot.AddField(m_v3Angles);
	snapshot.AddField(m_v3Velocity);
	snapshot.AddField(m_uiOriginTime);
}


/*====================
  IProjectile::ReadSnapshot
  ====================*/
bool	IProjectile::ReadSnapshot(CEntitySnapshot &snapshot)
{
	snapshot.ReadNextField(m_yStatus);
	snapshot.ReadNextRoundPosition(m_v3Origin);
	snapshot.ReadNextField(m_v3Angles);
	snapshot.ReadNextField(m_v3Velocity);
	snapshot.ReadNextField(m_uiOriginTime);

	Validate();
	
	return true;
}


/*====================
  IProjectile::GetTypeVector
  ====================*/
const vector<SDataField>&	IProjectile::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->push_back(SDataField(_T("m_yStatus"), FIELD_PUBLIC, TYPE_CHAR));
		s_pvFields->push_back(SDataField(_T("m_v3Origin"), FIELD_PUBLIC, TYPE_ROUNDPOSITION));
		s_pvFields->push_back(SDataField(_T("m_v3Angles"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_v3Velocity"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_uiOriginTime"), FIELD_PUBLIC, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  IProjectile::CalculatePosition
  ====================*/
void	IProjectile::CalculatePosition(uint uiTime)
{
	if (m_yStatus == ENTITY_STATUS_ACTIVE)
	{
		float fDeltaTime(uiTime > m_uiOriginTime ? MsToSec(uiTime - m_uiOriginTime) : 0.0f);
		CVec3f v3Gravity(-V_UP * p_gravity * GetGravity());

		CVec3f v3VelPos(m_v3Velocity * fDeltaTime);
		CVec3f v3GravPos(v3Gravity * fDeltaTime * fDeltaTime * 0.5f);
		m_v3Position = m_v3Origin + v3VelPos + v3GravPos;
		
		if (GetTurn())
		{
			CVec3f v3Velocity(m_v3Velocity + v3Gravity * fDeltaTime);
			m_v3Angles = M_GetAnglesFromForwardVec(Normalize(v3Velocity));
		}
	}
	else if (m_yStatus == ENTITY_STATUS_DORMANT)
	{
		float fDeltaTime(uiTime > m_uiOriginTime ? MsToSec(uiTime - m_uiOriginTime) : 0.0f);
		CVec3f v3Gravity(-V_UP * p_gravity * GetGravity());

		m_v3Position = m_v3Origin;
		
		if (GetTurn())
		{
			CVec3f v3Velocity(m_v3Velocity + v3Gravity * fDeltaTime);
			m_v3Angles = M_GetAnglesFromForwardVec(Normalize(v3Velocity));
		}
	}
	else
	{
		m_v3Position = m_v3Origin;
	}
}


/*====================
  IProjectile::AllocateSkeleton
  ====================*/
CSkeleton*	IProjectile::AllocateSkeleton()
{
	return m_pSkeleton = K2_NEW(global,   CSkeleton);
}


/*====================
  IProjectile::Bounce
  ====================*/
bool	IProjectile::Bounce(const STraceInfo &trace, bool bCheckDamage)
{
	IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));

	if (bCheckDamage && 
		pEntity != NULL &&
		pEntity->CanTakeDamage(0, Game.GetVisualEntity(m_uiOwnerIndex)))
		return false;

	uint uiElapsed(MAX(1u, uint(s_uiFrameTime * trace.fFraction)));

	// Reset the origin to the point of impact, slightly offset from the surface
	m_v3Origin = trace.v3EndPos + trace.plPlane.v3Normal;

	// Check for little to no progress
	if (trace.fFraction < 0.01f)
	{
		m_v3Velocity.Clear();

		s_uiElapsed += s_uiFrameTime;
		m_uiLastUpdateTime += s_uiFrameTime;
		s_uiFrameTime = 0;

		return false;
	}

	// Calculate the velocity at the instant of impact
	CVec3f v3Gravity(-V_UP * p_gravity * GetGravity());
	float fStartTime(MsToSec(s_uiStartTime - m_uiOriginTime + uiElapsed));
	CVec3f v3Velocity(m_v3Velocity + v3Gravity * fStartTime);

	// Reflect the velocicty
	m_v3Velocity = Reflect(v3Velocity, trace.plPlane.v3Normal, GetBounce());

	// Apply friction
	m_v3Velocity -= m_v3Velocity * GetFriction()/* * (1.0f - DotProduct(m_v3Velocity.Direction(), trace.plPlane.v3Normal))*/;

	// Reset the origin time to the instant of impact and track the sub frame timing
	m_uiOriginTime = s_uiStartTime + uiElapsed;

	if (s_uiFrameTime <= uiElapsed)
		return true;

	s_uiFrameTime -= uiElapsed;
	s_uiElapsed += uiElapsed;
	m_uiLastUpdateTime += uiElapsed;

	// Don't bounce more than once per frame
	if (m_uiLastBounceTime != Game.GetGameTime())
	{
		// Bounce effect
		tstring sEffectPath(m_pEntityConfig->GetBounceEffectPath());
		if (!sEffectPath.empty())
		{
			CGameEvent evImpact;
			evImpact.SetSourcePosition(m_v3Origin);
			evImpact.SetEffect(Game.RegisterEffect(sEffectPath));
			Game.AddEvent(evImpact);
		}

		m_uiLastBounceTime = Game.GetGameTime();
	}
	
	return EvaluateTrajectory();
}


/*====================
  IProjectile::Spawn
  ====================*/
void	IProjectile::Spawn()
{
	IVisualEntity::Spawn();

	m_yStatus = ENTITY_STATUS_ACTIVE;

	SetModelHandle(Game.RegisterModel(GetModelPath()));

	m_fScale = 1.0f;

	tstring sEffectPath(m_pEntityConfig->GetTrailEffectPath());
	if (!sEffectPath.empty())
	{
		SetEffect(EFFECT_CHANNEL_PROJECTILE_TRAIL, Game.RegisterEffect(sEffectPath));
		IncEffectSequence(EFFECT_CHANNEL_PROJECTILE_TRAIL);
	}

	StartAnimation(_T("idle"), -1);
	m_yDefaultAnim = m_ayAnim[0];
	m_pImpactEntity = NULL;
	m_uiCreationTime = m_uiOriginTime;
	m_uiLastUpdateTime = m_uiOriginTime;
	m_v3Position = m_v3Origin;
	m_bStuck = false;
	m_uiStickTime = INVALID_TIME;

	if (GetTurn())
		m_v3Angles = V3_ZERO;
}


/*====================
  IProjectile::DamageRadius
  ====================*/
void	IProjectile::DamageRadius()
{
	uint uiTeam(-1);

	if (Game.GetVisualEntity(m_uiOwnerIndex) != NULL)
		uiTeam = Game.GetVisualEntity(m_uiOwnerIndex)->GetTeam();

	uivector vResult;
	Game.GetEntitiesInRadius(vResult, CSphere(m_v3Position, m_fDamageRadius), TRACE_MELEE);	// FIXME: Add TRACE_RADIUSDAMAGE
	for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
	{
		IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
		if (pEnt == NULL)
			continue;

		if (m_uiDirectDamageEntityIndex != INVALID_INDEX && pEnt->GetIndex() == m_uiDirectDamageEntityIndex)
			continue;

		// Shields protect from radius damage
		// TODO: Check for obstruction between damage source and target?
		STraceInfo trace;
		Game.TraceLine(trace, GetPosition(), pEnt->GetPosition(), ~SURF_SHIELD);
		if (trace.fFraction < 1.0f)
			continue;

		float fFalloff(1.0f - CLAMP(Distance(pEnt->GetPosition(), GetPosition()) / m_fDamageRadius, 0.0f, 1.0f));

		pEnt->Damage(M_Randnum(m_fMinDamage, m_fMaxDamage) * fFalloff, GetDamageFlags() | DAMAGE_FLAG_SPLASH, Game.GetVisualEntity(m_uiOwnerIndex), m_unWeaponOriginID);
		if (m_unTargetState != INVALID_ENT_TYPE && pEnt->GetTeam() != uiTeam)
			pEnt->ApplyState(m_unTargetState, Game.GetGameTime(), m_uiTargetStateDuration, m_uiOwnerIndex);
	}
}


/*====================
  IProjectile::EvaluateTrajectory
  ====================*/
bool	IProjectile::EvaluateTrajectory()
{
	if (m_uiLastUpdateTime > Game.GetGameTime())
		return true;

	s_uiStartTime = MAX(m_uiLastUpdateTime, m_uiOriginTime);
	uint uiEndTime(MIN(Game.GetGameTime(), m_uiCreationTime + m_pEntityConfig->GetLifeTime()));

	float fStartTime(MsToSec(s_uiStartTime - m_uiOriginTime));
	float fEndTime(MsToSec(uiEndTime - m_uiOriginTime));

	// Calculate current position
	CVec3f v3Gravity(-V_UP * p_gravity * GetGravity());
	CVec3f v3Start(m_v3Origin + m_v3Velocity * fStartTime + v3Gravity * fStartTime * fStartTime * 0.5f);
	CVec3f v3End(m_v3Origin + m_v3Velocity * fEndTime + v3Gravity * fEndTime * fEndTime * 0.5f);

	uint uiOwnerWorldIndex(INVALID_INDEX);
	IGameEntity *pOwner(Game.GetEntity(m_uiOwnerIndex));
	if (pOwner != NULL && pOwner->IsVisual())
		uiOwnerWorldIndex = pOwner->GetAsVisualEnt()->GetWorldIndex();

	STraceInfo result;

	if (v3Start != v3End)
	{
		Game.TraceLine(result, v3Start, v3End, TRACE_PROJECTILE, uiOwnerWorldIndex);
		m_v3Position = result.v3EndPos;
	}
	else
	{
		result.fFraction = 1.0f;
		m_v3Position = v3End;
	}

	// Check for expired timer
	if (int(Game.GetGameTime() - m_uiCreationTime) > m_pEntityConfig->GetLifeTime())
	{
		m_v3Origin = m_v3Position;
		m_yStatus = ENTITY_STATUS_DORMANT;

		Kill();
		SetLocalFlags(ENT_LOCAL_FLAG_DELETE_NEXT_FRAME);
		return true;
	}

	// Check for impact
	if (result.fFraction < 1.0f)
	{
		bool bProcessDamage(true);

		IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
		if (pEntity != NULL)
			bProcessDamage = pEntity->Impact(result, this);

		// Bounce
		if (GetBounce() > 0.0f)
		{			
			if (Bounce(result, bProcessDamage))
			{
				m_uiLastUpdateTime = Game.GetGameTime();
				return true;
			}
		}

		if (!(result.uiSurfFlags & SURF_TERRAIN))
			m_pImpactEntity = Game.GetEntityFromWorldIndex(result.uiEntityIndex);

		if (result.uiSurfFlags & SURF_TERRAIN)
		{
			if (GetStick())
			{
				CVec3f v3Velocity(m_v3Velocity + v3Gravity * fEndTime);
 
				m_yStatus = ENTITY_STATUS_CORPSE;
				m_uiStickTime = Game.GetGameTime() + g_projectileStickTime;
				m_v3Origin = m_v3Position - (Normalize(v3Velocity) * M_Randnum(GetMinStickDistance(), GetMaxStickDistance()));

				if (GetTurn())
				{
					CVec3f v3Velocity(m_v3Velocity + v3Gravity * fEndTime);
					m_v3Angles = M_GetAnglesFromForwardVec(Normalize(v3Velocity));
				}

				m_v3Velocity = V3_ZERO;
			}
			else
			{
				m_v3Origin = m_v3Position;
				m_yStatus = ENTITY_STATUS_DORMANT;
				
				Kill();
				SetLocalFlags(ENT_LOCAL_FLAG_DELETE_NEXT_FRAME);
			}
			return true;
		}

		// Direct damage
		if (pEntity != NULL && pEntity->GetIndex() != m_uiOwnerIndex && bProcessDamage)
		{
			float fDamage = M_Randnum(m_fMinDamage, m_fMaxDamage);
			ICombatEntity *pCombat(pEntity->GetAsCombatEnt());

			if (pEntity->IsBuilding())
				fDamage *= m_fBuildingPierce;
			else if (pCombat != NULL && pCombat->GetIsSiege())
				fDamage *= m_fSiegePierce;
			else if (pCombat != NULL && pCombat->GetIsHellbourne())
				fDamage *= m_fHellbournePierce;
			else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
				fDamage *= m_fUnitPierce;

			pEntity->Damage(fDamage, GetDamageFlags(), Game.GetVisualEntity(m_uiOwnerIndex), m_unWeaponOriginID);
			pEntity->Hit(result.v3EndPos, m_v3Angles);
			m_uiDirectDamageEntityIndex = pEntity->GetIndex();

			uint uiTeam(-1);

			if (Game.GetVisualEntity(m_uiOwnerIndex) != NULL)
				uiTeam = Game.GetVisualEntity(m_uiOwnerIndex)->GetTeam();

			if (m_unTargetState != INVALID_ENT_TYPE && (pOwner == NULL || !pOwner->IsVisual() || pOwner->GetAsVisualEnt()->IsEnemy(pEntity)))
				pEntity->ApplyState(m_unTargetState, Game.GetGameTime(), m_uiTargetStateDuration, m_uiOwnerIndex);
		}

		m_v3Origin = m_v3Position;
		m_yStatus = ENTITY_STATUS_DORMANT;

		Kill();
		SetLocalFlags(ENT_LOCAL_FLAG_DELETE_NEXT_FRAME);
		return true;
	}

	m_uiLastUpdateTime = Game.GetGameTime();
	return true;
}


/*====================
  IProjectile::ServerFrame
  ====================*/
bool	IProjectile::ServerFrame()
{
	if (HasLocalFlags(ENT_LOCAL_FLAG_DELETE_NEXT_FRAME))
		return false;

	if (m_yStatus == ENTITY_STATUS_CORPSE)
	{
		if (m_uiStickTime <= Game.GetGameTime())
		{
			Kill();
			SetLocalFlags(ENT_LOCAL_FLAG_DELETE_NEXT_FRAME);
		}

		return true;
	}

	// Reset trajectory variables
	s_uiElapsed = 0;
	s_uiFrameTime = MAX(int(Game.GetGameTime() - m_uiLastUpdateTime), 0);

	return EvaluateTrajectory();
}


/*====================
  IProjectile::Kill
  ====================*/
void	IProjectile::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
	// Death event
	tstring sEffectPath(m_pEntityConfig->GetDeathEffectPath());
	if (!sEffectPath.empty())
	{
		CGameEvent evDeath;
		evDeath.SetSourcePosition(m_v3Position);
		evDeath.SetEffect(Game.RegisterEffect(sEffectPath));
		Game.AddEvent(evDeath);
	}

	// Splash damage
	if (m_fDamageRadius != 0.0f)
		DamageRadius();

	Killed();
}


/*====================
  IProjectile::Copy
  ====================*/
void	IProjectile::Copy(const IGameEntity &B)
{
	IVisualEntity::Copy(B);

	const IProjectile *pB(B.GetAsProjectile());
	if (!pB)
		return;
	const IProjectile &C(*pB);
	
	m_v3Origin = C.m_v3Origin;
	m_uiOriginTime = C.m_uiOriginTime;
	m_uiOwnerIndex = C.m_uiOwnerIndex;
}


/*====================
  IProjectile::ClientPrecache
  ====================*/
void	IProjectile::ClientPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetTrailEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetTrailEffectPath(), RES_EFFECT);

	if (!pConfig->GetDeathEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetDeathEffectPath(), RES_EFFECT);

	if (!pConfig->GetBounceEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetBounceEffectPath(), RES_EFFECT);
}


/*====================
  IProjectile::ServerPrecache
  ====================*/
void	IProjectile::ServerPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTrailEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTrailEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetDeathEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetDeathEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetBounceEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetBounceEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IProjectile::Interpolate
  ====================*/
void	IProjectile::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
	static_cast<IProjectile *>(pPrevState)->CalculatePosition(Game.GetPrevServerTime());
	static_cast<IProjectile *>(pNextState)->CalculatePosition(Game.GetServerTime());

	m_v3Position = LERP(fLerp, pPrevState->GetPosition(), pNextState->GetPosition());
	m_v3Angles = M_LerpAngles(fLerp, pPrevState->GetAngles(), pNextState->GetAngles());
	m_fScale = LERP(fLerp, pPrevState->GetScale(), pNextState->GetScale());
}

