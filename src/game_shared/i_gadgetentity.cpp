// (C)2006 S2 Games
// i_gadgetentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gadgetentity.h"
#include "c_teaminfo.h"
#include "c_entityclientinfo.h"

#include "../k2/c_model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_eventscript.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOATF(	g_gadgetMaxTerrainAngle,	0.5f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);

vector<SDataField>*	IGadgetEntity::s_pvFields;
//=============================================================================

/*====================
  IGadgetEntity::CEntityConfig::CEntityConfig
  ====================*/
IGadgetEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Lifetime, 30000),
INIT_ENTITY_CVAR(CorpseTime, 5000),
INIT_ENTITY_CVAR(StateName, _T("")),
INIT_ENTITY_CVAR(StateRadius, 0.f),
INIT_ENTITY_CVAR(StateDuration, 0),
INIT_ENTITY_CVAR(StateTargetEnemy, false),
INIT_ENTITY_CVAR(StateTargetAlly, false),
INIT_ENTITY_CVAR(BuildTime, 0),
INIT_ENTITY_CVAR(IsInvulnerable, false),
INIT_ENTITY_CVAR(ExperiencePerMinute, 0.0f)
{
}


/*====================
  IGadgetEntity::~IGadgetEntity
  ====================*/
IGadgetEntity::~IGadgetEntity()
{
	if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
	{
		Game.UnlinkEntity(m_uiWorldIndex);
		Game.DeleteWorldEntity(m_uiWorldIndex);
	}
}


/*====================
  IGadgetEntity::IGadgetEntity
  ====================*/
IGadgetEntity::IGadgetEntity(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_pEntityConfig(pConfig),

m_iOwnerClientNumber(-1),
m_uiOwnerIndex(INVALID_INDEX),
m_uiSpawnTime(INVALID_TIME),
m_unDamageID(INVALID_ENT_TYPE),
m_v3ViewAngles(V3_ZERO),

m_fTotalExperience(0.0f),
m_bAccessed(false)
{
	for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
		m_auiCounter[i] = -1;
}


/*====================
  IGadgetEntity::Baseline
  ====================*/
void	IGadgetEntity::Baseline()
{
	IVisualEntity::Baseline();

	m_uiSpawnTime = INVALID_TIME;

	m_v3ViewAngles = V3_ZERO;

	m_fTotalExperience = 0.0f;
	for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
		m_auiCounter[i] = -1;
}


/*====================
  IGadgetEntity::GetSnapshot
  ====================*/
void	IGadgetEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IVisualEntity::GetSnapshot(snapshot);

	snapshot.AddField(m_uiSpawnTime);
	snapshot.AddAngle16(m_v3ViewAngles[PITCH]);
	snapshot.AddAngle16(m_v3ViewAngles[ROLL]);
	snapshot.AddAngle16(m_v3ViewAngles[YAW]);
	snapshot.AddRound16(m_fTotalExperience);
	for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
		snapshot.AddField(m_auiCounter[i]);
}


/*====================
  IGadgetEntity::ReadSnapshot
  ====================*/
bool	IGadgetEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		if (!IVisualEntity::ReadSnapshot(snapshot))
			return false;

		snapshot.ReadNextField(m_uiSpawnTime);
		snapshot.ReadNextAngle16(m_v3ViewAngles[PITCH]);
		snapshot.ReadNextAngle16(m_v3ViewAngles[ROLL]);
		snapshot.ReadNextAngle16(m_v3ViewAngles[YAW]);
		snapshot.ReadNextRound16(m_fTotalExperience);
		for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
			snapshot.ReadNextField(m_auiCounter[i]);
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGadgetEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}

	return true;
}


/*====================
  IGadgetEntity::GetTypeVector
  ====================*/
const vector<SDataField>&	IGadgetEntity::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IVisualEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
		
		s_pvFields->push_back(SDataField(_T("m_uiSpawnTime"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_v3ViewAngles[PITCH]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3ViewAngles[ROLL]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3ViewAngles[YAW]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_fTotalExperience"), FIELD_PRIVATE, TYPE_ROUND16));
		for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
			s_pvFields->push_back(SDataField(_T("m_auiCounter[") + XtoA(i) + _T("]"), FIELD_PRIVATE, TYPE_FLOAT));
	}

	return *s_pvFields;
}


/*====================
  IGadgetEntity::AllocateSkeleton
  ====================*/
CSkeleton*	IGadgetEntity::AllocateSkeleton()
{
	return m_pSkeleton = K2_NEW(global,   CSkeleton);
}


/*====================
  IGadgetEntity::Link
  ====================*/
void	IGadgetEntity::Link()
{
	if (m_uiWorldIndex != INVALID_INDEX)
	{
		CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
		
		if (pWorldEnt != NULL)
		{
			pWorldEnt->SetPosition(GetPosition());
			pWorldEnt->SetScale(GetScale());
			pWorldEnt->SetScale2(GetScale2());
			pWorldEnt->SetAngles(GetAngles());
			pWorldEnt->SetModelHandle(GetModelHandle());
			pWorldEnt->SetGameIndex(GetIndex());

			uint uiLinkFlags(SURF_GADGET);
			if (IsIntangible())
				uiLinkFlags |= SURF_INTANGIBLE;

			if (GetStatus() != ENTITY_STATUS_DEAD)
				Game.LinkEntity(m_uiWorldIndex, LINK_SURFACE | LINK_MODEL, uiLinkFlags);
		}
	}
}


/*====================
  IGadgetEntity::Unlink
  ====================*/
void	IGadgetEntity::Unlink()
{
	if (m_uiWorldIndex != INVALID_INDEX)
		Game.UnlinkEntity(m_uiWorldIndex);
}


/*====================
  IGadgetEntity::Spawn
  ====================*/
void	IGadgetEntity::Spawn()
{
	IVisualEntity::Spawn();

	CEntityClientInfo *pClient(Game.GetClientInfo(GetOwnerClientNumber()));
	if (pClient != NULL)
		pClient->AddGadget(GetIndex());

	// Anims
	m_yDefaultAnim = 0;
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		m_ayAnim[i] = ENTITY_INVALID_ANIM;
		m_ayAnimSequence[i] = 0;
		m_afAnimSpeed[i] = 1.0f;
	}

	m_uiSpawnTime = Game.GetServerTime() + m_pEntityConfig->GetBuildTime();
	SetStatus(ENTITY_STATUS_ACTIVE);

	SetModelHandle(Game.RegisterModel(GetModelPath()));
	m_bbBounds = g_ResourceManager.GetModelSurfaceBounds(GetModelHandle());
	m_fScale = m_pEntityConfig->GetScale();
	m_bbBounds *= m_fScale;

	if (m_pSkeleton != NULL)
		m_pSkeleton->SetModel(GetModelHandle());

	m_fHealth = GetMaxHealth();

	if (m_uiWorldIndex == INVALID_INDEX)
		m_uiWorldIndex = Game.AllocateNewWorldEntity();

	StartAnimation(_T("idle"), 0);
	m_yDefaultAnim = m_ayAnim[0];

	if (m_pEntityConfig->GetBuildTime())
	{
		StartAnimation(_T("build"), 0, 1.0f, m_pEntityConfig->GetBuildTime());
		SetStatus(ENTITY_STATUS_SPAWNING);
	}

	CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
	if (pTeamInfo != NULL)
	{
		if (IsSpawnLocation())
			pTeamInfo->AddSpawnBuildingIndex(m_uiIndex);
	}

	Link();
}


/*====================
  IGadgetEntity::Damage
  ====================*/
float	IGadgetEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
	if (GetIsInvulnerable())
		return 0.0f;

	return IVisualEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);
}


/*====================
  IGadgetEntity::CanSpawn
  ====================*/
bool	IGadgetEntity::CanSpawn()
{
	try
	{
		SetModelHandle(Game.RegisterModel(GetModelPath()));

		// Collision
		uivector vEntities;
		vEntities.clear();
		const SurfVector &vSurfs(g_ResourceManager.GetModelSurfaces(GetModelHandle()));
		for (SurfVector::const_iterator it(vSurfs.begin()); it != vSurfs.end(); ++it)
		{
			CConvexPolyhedron cSurf(*it);
			cSurf.Transform(m_v3Position, CAxis(m_v3Angles), m_fScale);

			Game.GetEntitiesInSurface(vEntities, cSurf, SURF_MODEL | SURF_CORPSE | SURF_SHIELD | SURF_PROJECTILE);
			for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
			{
				IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
				if (pEntity == NULL)
					continue;
					
				EX_WARN(_T("Collision with entity #") + XtoA(pEntity->GetIndex()) + SPACE + ParenStr(pEntity->GetTypeName()));
			}
		}

		// Terrain
		CVec3f v3Normal(Game.GetTerrainNormal(m_v3Position.x, m_v3Position.y));
		if (DotProduct(v3Normal, V_UP) < g_gadgetMaxTerrainAngle)
			EX_WARN(_T("Max terrain variance"));

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGadgetEntity::CanSpawn() - "), NO_THROW);
		return false;
	}
}


/*====================
  IGadgetEntity::TestSpawn
  ====================*/
bool	IGadgetEntity::TestSpawn(ResHandle hModel, const CVec3f &v3Position, const CVec3f &v3Angles, float fScale)
{
	try
	{
		// Collision
		uivector vEntities;
		vEntities.clear();
		const SurfVector &vSurfs(g_ResourceManager.GetModelSurfaces(hModel));
		for (SurfVector::const_iterator it(vSurfs.begin()); it != vSurfs.end(); ++it)
		{
			CConvexPolyhedron cSurf(*it);
			cSurf.Transform(v3Position, CAxis(v3Angles), fScale);

			Game.GetEntitiesInSurface(vEntities, cSurf, SURF_MODEL | SURF_CORPSE | SURF_SHIELD | SURF_PROJECTILE);
			for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
			{
				IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
				if (pEntity == NULL)
					continue;
					
				EX_WARN(_T("Collision with entity #") + XtoA(pEntity->GetIndex()) + SPACE + ParenStr(pEntity->GetTypeName()));
			}
		}

		// Terrain
		CVec3f v3Normal(Game.GetTerrainNormal(v3Position.x, v3Position.y));
		if (DotProduct(v3Normal, V_UP) < g_gadgetMaxTerrainAngle)
			EX_WARN(_T("Max terrain variance"));

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGadgetEntity::TestSpawn() - "), NO_THROW);
		return false;
	}
}


/*====================
  IGadgetEntity::SpawnPreview
  ====================*/
void	IGadgetEntity::SpawnPreview()
{
	SetModelHandle(Game.RegisterModel(GetModelPath()));
	m_bbBounds = g_ResourceManager.GetModelSurfaceBounds(GetModelHandle());
	m_fScale = m_pEntityConfig->GetScale();
	m_bbBounds *= m_fScale;

	if (GetSkeleton() != NULL)
	{
		GetSkeleton()->SetModel(GetModelHandle());
		GetSkeleton()->SetDefaultAnim(_T("place"));
		GetSkeleton()->StartAnim(_T("place"), Game.GetGameTime(), 0);
		GetSkeleton()->Pose(Game.GetGameTime());
	}

	SetStatus(ENTITY_STATUS_ACTIVE);

	StartAnimation(_T("place"), 0);
	m_yDefaultAnim = m_ayAnim[0];
}


/*====================
  IGadgetEntity::GiveDeploymentExperience
  ====================*/
float	IGadgetEntity::GiveDeploymentExperience()
{
	if (Game.GetGamePhase() != GAME_PHASE_ACTIVE)
		return 0.0f;
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return 0.0f;

	float fExpReward(MsToMin(Game.GetFrameLength()) * m_pEntityConfig->GetExperiencePerMinute());
	if (fExpReward > 0.0f)
	{
		IPlayerEntity *pOwner(Game.GetPlayerEntityFromClientID(GetOwnerClientNumber()));
		if (pOwner != NULL)
			pOwner->GiveExperience(fExpReward);
		m_fTotalExperience += fExpReward;
	}

	return fExpReward;
}


/*====================
  IGadgetEntity::ServerFrame
  ====================*/
bool	IGadgetEntity::ServerFrame()
{
	IVisualEntity::ServerFrame();

	// Building
	if (GetStatus() == ENTITY_STATUS_SPAWNING)
	{
		if (Game.GetGameTime() >= m_uiSpawnTime)
		{
			SetStatus(ENTITY_STATUS_ACTIVE);
			StartAnimation(_T("idle"), 0);
		}
		return true;
	}

	// Corpse
	if (GetStatus() == ENTITY_STATUS_DEAD)
	{
		if (Game.GetGameTime() >= m_uiCorpseTime)
			return false;
		return true;
	}

	// Expire
	if (m_pEntityConfig->GetLifetime() > 0 &&
		Game.GetGameTime() >= m_uiSpawnTime + m_pEntityConfig->GetLifetime())
	{
		Kill();
		return true;
	}

	GiveDeploymentExperience();

	// Apply radius state
	ushort unStateID(0);
	if (!m_pEntityConfig->GetStateName().empty())
		unStateID = EntityRegistry.LookupID(m_pEntityConfig->GetStateName());
	if (unStateID != 0 && GetStatus() != ENTITY_STATUS_DEAD)
	{
		static uivector vResult;
		vResult.clear();
		Game.GetEntitiesInRadius(vResult, CSphere(GetPosition(), m_pEntityConfig->GetStateRadius()), 0);

		for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
		{
			IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL)
				continue;				
			if (pEnt->GetStatus() == ENTITY_STATUS_DEAD)
				continue;
			if (!pEnt->IsPlayer())
				continue;
			if (pEnt->GetTeam() == GetTeam() && !m_pEntityConfig->GetStateTargetAlly())
				continue;
			if (pEnt->GetTeam() != GetTeam() && !m_pEntityConfig->GetStateTargetEnemy())
				continue;
			if (pEnt->GetAsPlayerEnt()->IsObserver())
				continue;

			pEnt->ApplyState(unStateID, Game.GetGameTime(), m_pEntityConfig->GetStateDuration(), m_uiOwnerIndex);
		}
	}

	return true;
}


/*====================
  IGadgetEntity::Kill
  ====================*/
void	IGadgetEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
	CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
	if (pTeamInfo != NULL && IsSpawnLocation())
		pTeamInfo->RemoveSpawnBuildingIndex(m_uiIndex);

	CEntityClientInfo *pOwner(Game.GetClientInfo(GetOwnerClientNumber()));
	if (pOwner != NULL)
		pOwner->RemoveGadget(GetIndex());

	SetStatus(ENTITY_STATUS_DEAD);
	m_uiCorpseTime = Game.GetGameTime() + m_pEntityConfig->GetCorpseTime();
	StartAnimation(_T("death"), 0);
	Unlink();
	SetViewAngles(V_ZERO);

	KillReward(pAttacker);

	IGameEntity *pEnt(Game.GetFirstEntity());
	while (pEnt)
	{
		if (pEnt->IsPlayer())
		{
			IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

			// Clear this order for all players
			for (byte ySeq(0); ySeq < pPlayer->GetNumOrders(); ySeq++)
				if (pPlayer->GetOrderEntIndex(ySeq) == m_uiIndex)
					pPlayer->DeleteOrder(ySeq);
			
			if (pPlayer->GetOfficerOrderEntIndex() == m_uiIndex)
				pPlayer->SetOfficerOrder(OFFICERCMD_INVALID);
		}

		pEnt = Game.GetNextEntity(pEnt);
	}

	tstring sMethod(_T("Unknown"));
	if (unKillingObjectID != INVALID_ENT_TYPE)
	{
		ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

		if (pCvar != NULL)
			sMethod = pCvar->GetString();
	}

	Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
	Game.RegisterTriggerParam(_T("attackingindex"), XtoA(pAttacker != NULL ? pAttacker->GetIndex() : INVALID_INDEX));
	Game.RegisterTriggerParam(_T("method"), sMethod);
	Game.TriggerEntityScript(GetIndex(), _T("death"));
}


/*====================
  IGadgetEntity::GetRemainingLifetime
  ====================*/
uint	IGadgetEntity::GetRemainingLifetime() const
{
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return 0;

	return m_pEntityConfig->GetLifetime() - (Game.GetGameTime() - m_uiSpawnTime);
}


/*====================
  IGadgetEntity::Copy
  ====================*/
void	IGadgetEntity::Copy(const IGameEntity &B)
{
	IVisualEntity::Copy(B);

	const IGadgetEntity *pB(B.GetAsGadget());

	if (pB == NULL)	
		return;

	const IGadgetEntity &C(*pB);

	m_uiSpawnTime =			C.m_uiSpawnTime;
	m_v3ViewAngles =		C.m_v3ViewAngles;
	m_uiOwnerIndex =		C.m_uiOwnerIndex;
	m_iOwnerClientNumber =	C.m_iOwnerClientNumber;

	m_fTotalExperience =	C.m_fTotalExperience;
	for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
		m_auiCounter[i] = C.m_auiCounter[i];

	m_bAccessed =			C.m_bAccessed;
}


/*====================
  IGadgetEntity::ClientPrecache
  ====================*/
void	IGadgetEntity::ClientPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetStateName().empty())
		EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetStateName()));
}


/*====================
  IGadgetEntity::ServerPrecache
  ====================*/
void	IGadgetEntity::ServerPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetStateName().empty())
		EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetStateName()));
}


/*====================
  IGadgetEntity::UpdateSkeleton
  ====================*/
void	IGadgetEntity::UpdateSkeleton(bool bPose)
{
	if (m_pSkeleton == NULL)
		return;

	m_pSkeleton->SetModel(GetModelHandle());

	if (GetModelHandle() == INVALID_RESOURCE)
		return;

	// Pose skeleton
	if (bPose)
		m_pSkeleton->Pose(Game.GetGameTime(), GetViewAngle(PITCH), GetViewAngle(YAW));
	else
		m_pSkeleton->PoseLite(Game.GetGameTime());
	
	// Process animation events
	if (m_pSkeleton->CheckEvents())
	{
		tstring sOldDir(FileManager.GetWorkingDirectory());
		FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModelHandle())));

		const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

		for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
			EventScript.Execute(it->sCmd, it->iTimeNudge);

		m_pSkeleton->ClearEvents();

		FileManager.SetWorkingDirectory(sOldDir);
	}
}


/*====================
  IGadgetEntity::AddToScene
  ====================*/
bool	IGadgetEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
	PROFILE("IGadgetEntity::AddToScene");

	if (GetModelHandle() == INVALID_INDEX)
		return false;

	if (Game.IsCommander() && !m_bSighted)
		return false;

	CVec4f v4TintedColor(v4Color);

	if (m_v3AxisAngles != m_v3Angles)
	{
		m_aAxis.Set(m_v3Angles);
		m_v3AxisAngles = m_v3Angles;
	}

	static CSceneEntity sceneEntity;

	sceneEntity.Clear();
	sceneEntity.scale = GetScale() * GetScale2();
	sceneEntity.SetPosition(m_v3Position);
	sceneEntity.axis = m_aAxis;
	sceneEntity.objtype = OBJTYPE_MODEL;
	sceneEntity.hModel = GetModelHandle();
	sceneEntity.skeleton = m_pSkeleton;
	sceneEntity.color = v4TintedColor;
	sceneEntity.flags = iFlags | SCENEOBJ_SOLID_COLOR | SCENEOBJ_USE_AXIS;

	if (Game.LooksLikeEnemy(m_uiIndex))
		sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("red"));

	if (m_uiClientRenderFlags & ECRF_SNAPSELECTED)
		sceneEntity.color *= m_v4SelectColor;

	if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
		sceneEntity.color[A] *= 0.5f;

	SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

	if (!cEntry.bCull || !cEntry.bCullShadow)
	{
		AddSelectionRingToScene();
		UpdateSkeleton(true);
	}
	else
	{
		UpdateSkeleton(false);
	}

	return true;
}
