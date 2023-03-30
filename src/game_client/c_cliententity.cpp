// (C)2006 S2 Games
// c_cliententity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_cliententity.h"
#include "c_gameclient.h"

#include "../game_shared/c_entityclientinfo.h"
#include "../game_shared/c_entityeffect.h"
#include "../game_shared/c_teaminfo.h"
#include "../game_shared/c_teamdefinition.h"

#include "../k2/c_entitysnapshot.h"
#include "../k2/c_world.h"
#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_draw2d.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_input.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentitymodifier.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL		(cg_debugServerSnapshot,				false);
CVAR_BOOL		(cg_debugMeleeAngles,					false);
CVAR_STRING		(cg_effectTeamPlayer,					"/shared/effects/team_player.effect");
CVAR_STRING		(cg_effectEye,							"/shared/effects/eye.effect");
//=============================================================================

/*====================
  CClientEntity::~CClientEntity
  ====================*/
CClientEntity::~CClientEntity()
{
	uint uiWorldIndex(m_pNextState->GetWorldIndex());

	if (uiWorldIndex != INVALID_INDEX && GameClient.WorldEntityExists(uiWorldIndex))
	{
		GameClient.UnlinkEntity(uiWorldIndex);
		GameClient.DeleteWorldEntity(uiWorldIndex);
	}

	for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
		SAFE_DELETE(m_apEffectThread[i]);

	for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
	{
		if(m_ahSoundHandle[i] != INVALID_INDEX)
			K2SoundManager.StopHandle(m_ahSoundHandle[i]);
	}

	SAFE_DELETE(m_pSkeleton);

	// Clear states manually because clients already delete states when the entity state snapshot is deleted
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		m_pNextState->SetState(i, NULL);
		m_pPrevState->SetState(i, NULL);
		m_pCurrentState->SetState(i, NULL);
	}

	for (int i(0); i < MAX_INVENTORY; ++i)
	{
		m_pNextState->SetInventorySlot(i, NULL);
		m_pPrevState->SetInventorySlot(i, NULL);
		m_pCurrentState->SetInventorySlot(i, NULL);
	}

	SAFE_DELETE(m_pNextState);
	SAFE_DELETE(m_pPrevState);
	SAFE_DELETE(m_pCurrentState);
}


/*====================
  CClientEntity::CClientEntity
  ====================*/
CClientEntity::CClientEntity() :
m_uiIndex(INVALID_INDEX),
m_unType(0),

m_pSkeleton(NULL),
m_bStatic(false),
m_bClientEntity(false),

m_pNextState(NULL),
m_pPrevState(NULL),
m_pCurrentState(NULL),

m_iIndicatorEffectChannel(-1),
m_hIndicatorEffect(INVALID_RESOURCE),
m_iEyeEffectChannel(-1),
m_iBuildEffectChannel(-1),
m_iStatePreviewChannel(-1),

m_v3PositionLinked(FAR_AWAY, FAR_AWAY, FAR_AWAY),
m_yStatusLinked(EEntityStatus(-1)),
m_fScaleLinked(0.0f),
m_hModelLinked(INVALID_RESOURCE)
{
	for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
		m_apEffectThread[i] = NULL;

	for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
		m_ahSoundHandle[i] = INVALID_INDEX;
}


/*====================
  CClientEntity::IsValid
  ====================*/
bool	CClientEntity::IsValid() const
{
	if (m_pPrevState == NULL ||
		m_pNextState == NULL ||
		!m_pPrevState->IsValid() ||
		!m_pNextState->IsValid())
		return false;

	return true;
}


/*====================
  CClientEntity::SetIndex
  ====================*/
void	CClientEntity::SetIndex(uint uiIndex)
{
	m_uiIndex = uiIndex;

	if (m_pNextState == NULL ||
		m_pPrevState == NULL ||
		m_pCurrentState == NULL)
		return;

	m_pNextState->SetIndex(m_uiIndex);
	m_pPrevState->SetIndex(m_uiIndex);
	m_pCurrentState->SetIndex(m_uiIndex);
}


/*====================
  CClientEntity::SetType
  ====================*/
void	CClientEntity::SetType(ushort unType)
{
	m_unType = unType;

	if (m_pNextState == NULL ||
		m_pPrevState == NULL ||
		m_pCurrentState == NULL)
		return;

	m_pNextState->SetType(m_unType);
	m_pPrevState->SetType(m_unType);
	m_pCurrentState->SetType(m_unType);
}


/*====================
  CClientEntity::SetSkeleton
  ====================*/
void	CClientEntity::SetSkeleton(CSkeleton *pSkeleton)
{
	m_pSkeleton = pSkeleton;

	if (m_pNextState == NULL ||
		m_pPrevState == NULL ||
		m_pCurrentState == NULL)
		return;

	m_pNextState->SetSkeleton(m_pSkeleton);
	m_pPrevState->SetSkeleton(m_pSkeleton);
	m_pCurrentState->SetSkeleton(m_pSkeleton);
}


/*====================
  CClientEntity::GetWorldIndex
  ====================*/
uint	CClientEntity::GetWorldIndex() const
{
	return m_pNextState->GetWorldIndex();
}


/*====================
  CClientEntity::GetClientID
  ====================*/
int		CClientEntity::GetClientID() const
{
	if (m_pNextState != NULL &&	m_pNextState->IsPlayer() &&	m_pNextState->GetAsPlayerEnt()->GetClientID() != -1)
		return m_pNextState->GetAsPlayerEnt()->GetClientID();
	else if (m_pPrevState != NULL && m_pPrevState->IsPlayer() && m_pPrevState->GetAsPlayerEnt()->GetClientID() != -1)
		return m_pPrevState->GetAsPlayerEnt()->GetClientID();

	return -1;
}


/*====================
  CClientEntity::Initialize
  ====================*/
void	CClientEntity::Initialize(IVisualEntity *pEntity)
{
	try
	{
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
		{
			m_aiActiveAnim[i] = -1;
			m_ayActiveAnimSequence[i] = 0;
		}

		for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
		{
			m_ahActiveEffect[i] = INVALID_RESOURCE;
			m_ayActiveEffectSequence[i] = 0;
			SAFE_DELETE(m_apEffectThread[i]);
		}

		for (int i(NUM_EFFECT_CHANNELS); i < NUM_CLIENT_EFFECT_THREADS; ++i)
		{
			SAFE_DELETE(m_apEffectThread[i]);
		}

		for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
		{
			if (m_ahSoundHandle[i] != INVALID_INDEX)
			{
				K2SoundManager.StopHandle(m_ahSoundHandle[i]);
				m_ahSoundHandle[i] = INVALID_INDEX;
			}
		}

		for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
		{
			if (m_pNextState) m_pNextState->SetState(i, NULL);
			if (m_pPrevState) m_pPrevState->SetState(i, NULL);
			if (m_pCurrentState) m_pCurrentState->SetState(i, NULL);
		}

		for (int i(0); i < MAX_INVENTORY; ++i)
		{
			if (m_pNextState) m_pNextState->SetInventorySlot(i, NULL);
			if (m_pPrevState) m_pPrevState->SetInventorySlot(i, NULL);
			if (m_pCurrentState) m_pCurrentState->SetInventorySlot(i, NULL);
		}

		m_v3PositionLinked = CVec3f(FAR_AWAY, FAR_AWAY, FAR_AWAY);
		m_yStatusLinked = EEntityStatus(-1);
		m_fScaleLinked = 0.0f;
		m_hModelLinked = INVALID_RESOURCE;
		m_hNoBuildEffect = INVALID_RESOURCE;
		m_hStatePreviewEffect = INVALID_RESOURCE;

		if (m_pNextState)
			m_pNextState->Unlink();

		SAFE_DELETE(m_pSkeleton);

		m_hIndicatorEffect = INVALID_RESOURCE;
		if (m_iIndicatorEffectChannel != -1)
		{
			StopEffect(m_iIndicatorEffectChannel);
			m_iIndicatorEffectChannel = -1;
		}

		if (m_iEyeEffectChannel != -1)
		{
			StopEffect(m_iEyeEffectChannel);
			m_iEyeEffectChannel = -1;
		}

		if (m_iBuildEffectChannel != -1)
		{
			StopEffect(m_iBuildEffectChannel);
			m_iBuildEffectChannel = -1;
		}

		if (m_iStatePreviewChannel != -1)
		{
			StopEffect(m_iStatePreviewChannel);
			m_iStatePreviewChannel = -1;
		}

		IGameEntity *pNewEnt(pEntity);

		SAFE_DELETE(m_pNextState);
		m_pNextState = pNewEnt->GetAsVisualEnt();
		if (m_pNextState == NULL)
			EX_ERROR(_T("Allocation failed for NextState"));

		SAFE_DELETE(m_pPrevState);
		pNewEnt = EntityRegistry.Allocate(pEntity->GetType());
		if (pNewEnt == NULL)
			EX_ERROR(_T("Allocation failed for PrevState"));
		m_pPrevState = pNewEnt->GetAsVisualEnt();
		if (m_pPrevState == NULL)
			EX_ERROR(_T("Allocation failed for PrevState"));

		SAFE_DELETE(m_pCurrentState);
		pNewEnt = EntityRegistry.Allocate(pEntity->GetType());
		if (pNewEnt == NULL)
			EX_ERROR(_T("Allocation failed for CurrentState"));
		m_pCurrentState = pNewEnt->GetAsVisualEnt();
		if (m_pCurrentState == NULL)
			EX_ERROR(_T("Allocation failed for CurrentState"));

		SetIndex(pEntity->GetIndex());
		SetType(pEntity->GetType());
		SetSkeleton(pEntity->AllocateSkeleton());

		m_bStatic = pEntity->IsStatic();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CClientEntity::Initialize() - "));
	}
}


/*====================
  CClientEntity::CopyNextToPrev
  ====================*/
void	CClientEntity::CopyNextToPrev()
{
	m_pPrevState->Copy(*m_pNextState);
}


/*====================
  CClientEntity::CopyNextToCurrent
  ====================*/
void	CClientEntity::CopyNextToCurrent()
{
	m_pCurrentState->Copy(*m_pNextState);
}


/*====================
  CClientEntity::Interpolate
  ====================*/
void	CClientEntity::Interpolate(float fLerp)
{
	if (!IsValid())
		return;

	m_pCurrentState->Interpolate(fLerp, m_pPrevState, m_pNextState);

	// Check for things that might require a re-link
	bool bRelink(false);
	if (m_v3PositionLinked != m_pCurrentState->GetPosition())
		bRelink = true;

	if (m_yStatusLinked != m_pCurrentState->GetStatus())
		bRelink = true;

	if (m_fScaleLinked != m_pCurrentState->GetScale() * m_pCurrentState->GetScale2())
		bRelink = true;

	if (m_hModelLinked != m_pCurrentState->GetModelHandle())
		bRelink = true;

	if (bRelink ||
		(m_pCurrentState->GetStatus() == ENTITY_STATUS_DEAD && m_pCurrentState->HasNetFlags(ENT_NET_FLAG_NO_CORPSE)))
		m_pCurrentState->Unlink();

	if (bRelink &&
		(m_pCurrentState->GetStatus() != ENTITY_STATUS_SPAWNING || m_pCurrentState->IsBuilding()) &&
		m_pCurrentState->GetStatus() != ENTITY_STATUS_DORMANT &&
		(m_pCurrentState->GetStatus() != ENTITY_STATUS_DEAD || !m_pCurrentState->HasNetFlags(ENT_NET_FLAG_NO_CORPSE)))
	{
		m_pCurrentState->Link();
		m_v3PositionLinked = m_pCurrentState->GetPosition();
		m_yStatusLinked = m_pCurrentState->GetStatus();
		m_fScaleLinked = m_pCurrentState->GetScale() * m_pCurrentState->GetScale2();
		m_hModelLinked = m_pCurrentState->GetModelHandle();
	}
}


/*====================
  CClientEntity::Frame
  ====================*/
void	CClientEntity::Frame()
{
	m_pCurrentState->UpdateSighting(GameClient.GetVision());
}


/*====================
  CClientEntity::AddToScene
  ====================*/
void	CClientEntity::AddToScene()
{
	PROFILE("CClientEntity::AddToScene");

	m_pCurrentState->SetShowEffects(false);

	if (!IsValid())
		return;

	m_pSkeleton = m_pCurrentState->GetSkeleton();

	if (!IsStatic())
	{
		// Update skeleton model
		if (m_pSkeleton && m_pSkeleton->GetModel() != m_pCurrentState->GetModelHandle())
		{
			if (m_pSkeleton->GetModel() == INVALID_RESOURCE ||
				m_pCurrentState->GetModelHandle() == INVALID_RESOURCE)
			{
				m_pSkeleton->SetModel(m_pCurrentState->GetModelHandle());
			}
			else
			{
				uint auiAnimStartTime[NUM_ANIM_CHANNELS];
				float afAnimSpeed[NUM_ANIM_CHANNELS];
				tstring asAnim[NUM_ANIM_CHANNELS];
				for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
				{
					auiAnimStartTime[i] = m_pSkeleton->GetCurrentAnimStartTime(i);
					asAnim[i] = m_pSkeleton->GetCurrentAnimName(i);
					afAnimSpeed[i] = m_pSkeleton->GetCurrentAnimSpeed(i);
				}

				m_pSkeleton->SetModel(m_pCurrentState->GetModelHandle());

				for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
					m_pSkeleton->StartAnim(asAnim[i], auiAnimStartTime[i], i, 0, afAnimSpeed[i]);
			}
		}

		// Start any new animations on the skeleton
		if (m_pSkeleton && m_pSkeleton->GetModel() != INVALID_RESOURCE)
		{
			m_pSkeleton->SetDefaultAnim(g_ResourceManager.GetAnimName(m_pCurrentState->GetModelHandle(), m_pCurrentState->GetDefaultAnim()));

			// Process StopAnim's
			for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			{
				if (m_pCurrentState->GetAnim(i) == ENTITY_STOP_ANIM &&
					(m_pCurrentState->GetAnim(i) != m_aiActiveAnim[i] || 
					m_pCurrentState->GetAnimSequence(i) != m_ayActiveAnimSequence[i]))
				{
					m_aiActiveAnim[i] = m_pCurrentState->GetAnim(i);
					m_ayActiveAnimSequence[i] = m_pCurrentState->GetAnimSequence(i);

					m_pSkeleton->StopAnim(i, Game.GetGameTime());
				}
			}

			// Start new animations
			for (int i(NUM_ANIM_CHANNELS - 1); i >= 0; --i)
			{
				if (m_pCurrentState->GetAnim(i) != m_aiActiveAnim[i] || 
					m_pCurrentState->GetAnimSequence(i) != m_ayActiveAnimSequence[i])
				{
					m_aiActiveAnim[i] = m_pCurrentState->GetAnim(i);
					m_ayActiveAnimSequence[i] = m_pCurrentState->GetAnimSequence(i);

					if (m_pCurrentState->GetAnim(i) == ENTITY_INVALID_ANIM)
						continue;

					m_pSkeleton->StartAnim(
						g_ResourceManager.GetAnimName(m_pCurrentState->GetModelHandle(), m_pCurrentState->GetAnim(i)),
						Game.GetGameTime(), i, -1,
						m_pCurrentState->GetAnimSpeed(i), 0);
				}
			}

			// Set animation speeds
			for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			{
				m_pSkeleton->SetAnimSpeed(m_pCurrentState->GetAnimSpeed(i), i);
			}

		}
	}

	if (/*!Game.IsCommander() &&*/GetClientID() != GameClient.GetLocalClientNum() && m_unType != Entity_Effect)
	{
		if (DistanceSq(m_pCurrentState->GetPosition(), GameClient.GetCamera()->GetOrigin()) > SQR(MIN<float>(scene_farClip, scene_worldFarClip)))
			return;
	}

	// Start any new effects on the entity
	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		if (m_pCurrentState->GetEffect(i) != m_ahActiveEffect[i] || 
			m_pCurrentState->GetEffectSequence(i) != m_ayActiveEffectSequence[i])
		{
			SAFE_DELETE(m_apEffectThread[i]);

			m_ahActiveEffect[i] = m_pCurrentState->GetEffect(i);
			m_ayActiveEffectSequence[i] = m_pCurrentState->GetEffectSequence(i);

			CEffect	*pEffect(g_ResourceManager.GetEffect(m_ahActiveEffect[i]));

			if (pEffect)
			{
				if (m_pCurrentState->IsProjectile() && m_pPrevState->GetAsProjectile()->GetOriginTime() == Game.GetPrevServerTime())
				{
					// Hack to fix projectile trail start
					IProjectile *pProjectile(m_pCurrentState->GetAsProjectile());

					uint uiTime(MIN(pProjectile->GetOriginTime(), Game.GetGameTime()));

					m_apEffectThread[i] = pEffect->SpawnThread(uiTime);

					if (m_apEffectThread[i] != NULL)
					{
						m_apEffectThread[i]->SetCamera(GameClient.GetCamera());
						m_apEffectThread[i]->SetWorld(GameClient.GetWorldPointer());

						m_apEffectThread[i]->SetSourceSkeleton(m_pSkeleton);
						m_apEffectThread[i]->SetSourceModel(g_ResourceManager.GetModel(m_pCurrentState->GetModelHandle()));
						m_apEffectThread[i]->SetTargetSkeleton(NULL);
						m_apEffectThread[i]->SetTargetModel(NULL);

						m_apEffectThread[i]->SetActive(true);

						pProjectile->CalculatePosition(uiTime);

						pProjectile->UpdateEffectThread(m_apEffectThread[i]);

						// Execute effect
						if (m_apEffectThread[i]->Execute(uiTime))
						{
							// Effect finished, so delete it
							K2_DELETE(m_apEffectThread[i]);
							m_apEffectThread[i] = NULL;
						}
					}
				}
				else
				{
					m_apEffectThread[i] = pEffect->SpawnThread(GameClient.GetGameTime());

					if (m_apEffectThread[i] != NULL)
					{
						m_apEffectThread[i]->SetCamera(GameClient.GetCamera());
						m_apEffectThread[i]->SetWorld(GameClient.GetWorldPointer());

						m_apEffectThread[i]->SetSourceSkeleton(m_pSkeleton);
						m_apEffectThread[i]->SetSourceModel(g_ResourceManager.GetModel(m_pCurrentState->GetModelHandle()));
						m_apEffectThread[i]->SetTargetSkeleton(NULL);
						m_apEffectThread[i]->SetTargetModel(NULL);
					}
				}
			}
			else
			{
				m_apEffectThread[i] = NULL;
			}
		}
	}

	// Update existing sounds
	{
		PROFILE("Sounds");

		for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
		{
			if (m_ahSoundHandle[i] == INVALID_INDEX)
				continue;

			if (!K2SoundManager.UpdateHandle(m_ahSoundHandle[i], m_pCurrentState->GetPosition(), m_pCurrentState->GetVelocity()))
			{
				m_ahSoundHandle[i] = INVALID_INDEX;
			}
		}
	}

	CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
	IPlayerEntity *pLocalPlayer(NULL);

	if (pLocalClient != NULL)
		pLocalPlayer = pLocalClient->GetPlayerEntity();

	// Team indicators
	if ((m_pCurrentState->IsPlayer() || m_pCurrentState->IsPet()) &&
		pLocalClient != NULL && pLocalPlayer != NULL &&
		pLocalPlayer->GetIndex() != m_uiIndex &&
		!pLocalPlayer->LooksLikeEnemy(m_pCurrentState) &&
		(pLocalPlayer->GetStatus() == ENTITY_STATUS_ACTIVE || pLocalPlayer->GetStatus() == ENTITY_STATUS_DEAD))
	{
		CEntityTeamInfo *pTeam(GameClient.GetTeam(pLocalPlayer->GetTeam()));

		ResHandle hIndicatorEffect(INVALID_RESOURCE);
		if (m_pCurrentState->GetStatus() == ENTITY_STATUS_ACTIVE)
			hIndicatorEffect = g_ResourceManager.Register(cg_effectTeamPlayer, RES_EFFECT);
		else if (m_pCurrentState->GetStatus() == ENTITY_STATUS_DEAD && pLocalPlayer->GetShowResurrectable() && pTeam != NULL && !m_pCurrentState->HasNetFlags(ENT_NET_FLAG_NO_RESURRECT))
			hIndicatorEffect = g_ResourceManager.Register(pTeam->GetDefinition()->GetResableEffect(), RES_EFFECT);

		if (m_hIndicatorEffect != hIndicatorEffect)
		{
			if (m_iIndicatorEffectChannel != -1)
				StopEffect(m_iIndicatorEffectChannel);
			m_iIndicatorEffectChannel = StartEffect(hIndicatorEffect, -1, 0);
			m_hIndicatorEffect = hIndicatorEffect;
		}
	}
	else
	{
		if (m_iIndicatorEffectChannel != -1)
		{
			StopEffect(m_iIndicatorEffectChannel);
			m_iIndicatorEffectChannel = -1;
		}
	}

	// Plays building effects during build preview 
	IBuildingEntity *pBuilding(m_pCurrentState->GetAsBuilding());
	if (pBuilding != NULL)
	{
		if (GameClient.IsBuildModeActive())
		{
			if (m_hNoBuildEffect != INVALID_RESOURCE && m_iBuildEffectChannel == -1)
				m_iBuildEffectChannel = StartEffect(m_hNoBuildEffect, -1, 0);

			if (m_hStatePreviewEffect != INVALID_RESOURCE && m_iStatePreviewChannel == -1)
				m_iStatePreviewChannel = StartEffect(m_hStatePreviewEffect, -1, 0);
		}
		else
		{
			StopNoBuildEffect();
			StopStatePreviewEffect();
		}
	}

	// Electric eye indicators
	if (m_pCurrentState->HasNetFlags(ENT_NET_FLAG_REVEALED) &&
		m_pCurrentState->GetTeam() != pLocalClient->GetTeam())
	{
		if (m_iEyeEffectChannel == -1)
			m_iEyeEffectChannel = StartEffect(cg_effectEye, -1, 0);
	}
	else
	{
		if (m_iEyeEffectChannel != -1)
		{
			StopEffect(m_iEyeEffectChannel);
			m_iEyeEffectChannel = -1;
		}
	}

	if (cg_debugServerSnapshot)
	{
		m_pNextState->AddToScene(GREEN, SCENEOBJ_SOLID_COLOR);
		m_pPrevState->AddToScene(RED, SCENEOBJ_SOLID_COLOR);
	}

	uint uiOldSize(uint(SceneManager.GetEntityList().size()));

	CVec4f v4Color(WHITE);
	int iFlags(0);

	// Melee attack angle debugging
	if (cg_debugMeleeAngles && m_pCurrentState != GameClient.GetLocalPlayerCurrent() && m_pCurrentState->IsCombat() && GameClient.GetLocalPlayer() != NULL)
	{
		CVec2f	v2AttackerDir(GameClient.GetLocalPlayer()->GetPosition().xy() - m_pCurrentState->GetAsCombatEnt()->GetPosition().xy());
		v2AttackerDir.Normalize();
		CAxis	axisDefender(m_pCurrentState->GetAsCombatEnt()->GetAngles());
		CVec2f	v2DefenderFwd(axisDefender.Forward2d());
		float fAngle = DotProduct(-v2AttackerDir, v2DefenderFwd);

		if (fAngle >= DEGCOS(ICvar::GetFloat(_T("p_rearAttackAngle")) / 2.0f))
		{
			v4Color = RED;
			iFlags = SCENEOBJ_SOLID_COLOR;
		}

		if (fAngle <= DEGCOS(180.0f - (ICvar::GetFloat(_T("p_blockAngle")) / 2.0f)))
		{
			v4Color = CYAN;
			iFlags = SCENEOBJ_SOLID_COLOR;
		}
	}

	// Generate scene entities
	{
		PROFILE("AddToScene");

		bool bShowEffects(m_pCurrentState->AddToScene(v4Color, iFlags) && !m_pCurrentState->IsStealthed());

		m_pCurrentState->SetShowEffects(bShowEffects);
	}

	SceneEntityList &lSceneEntities(SceneManager.GetEntityList());
	SceneEntityList::iterator itBegin(lSceneEntities.begin() + uiOldSize);

	{
		PROFILE("Effects");
	
		// Process effect threads for this entity
		for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
		{
			if (!m_apEffectThread[i])
				continue;

			// Setup effect parameters
			m_apEffectThread[i]->SetActive(true);

			m_pCurrentState->UpdateEffectThread(m_apEffectThread[i]);

			// Pass off first person entity effects
			bool bUpdate(true);

			if (m_unType == Entity_Effect)
			{
				uint uiLocalPlayerIndex(GameClient.GetLocalPlayerIndex());
				bool bFirstPerson(GameClient.GetCamera()->HasFlags(CAM_FIRST_PERSON));

				if (i == 0)
				{
					if (static_cast<CEntityEffect *>(m_pCurrentState)->GetSourceEntityIndex() == uiLocalPlayerIndex && bFirstPerson)
						bUpdate = false;
				}
				else if (i == 1)
				{
					if (static_cast<CEntityEffect *>(m_pCurrentState)->GetSourceEntityIndex() == uiLocalPlayerIndex && bFirstPerson)
					{
						GameClient.PushFirstPersonEffect(m_apEffectThread[i]);
						continue;
					}
					else
					{
						bUpdate = false;
					}
				}
			}
			
			// Execute effect
			if (m_apEffectThread[i]->Execute(GameClient.GetGameTime()))
			{
				// Effect finished, so delete it
				K2_DELETE(m_apEffectThread[i]);
				m_apEffectThread[i] = NULL;
			}
			else
			{
				if (!bUpdate)
				{
					// Cleanup
					m_apEffectThread[i]->Cleanup();
					continue;
				}

				// Camera movement
				GameClient.AddCameraEffectOffset(m_apEffectThread[i]->GetCameraOffset());
				GameClient.AddCameraEffectAngleOffset(m_apEffectThread[i]->GetCameraAngleOffset());

				// Overlays
				if (m_apEffectThread[i]->HasActiveOverlay() &&
					pLocalPlayer != NULL && m_uiIndex == pLocalPlayer->GetIndex())
					GameClient.AddOverlay(m_apEffectThread[i]->GetOverlayColor(), m_apEffectThread[i]->GetOverlayMaterial());

				// Update and render all particles systems associated with this effect thread
				const InstanceMap &mapInstances(m_apEffectThread[i]->GetInstances());
				for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
				{
					IEffectInstance *pParticleSystem(it->second);

					pParticleSystem->Update(GameClient.GetGameTime());

					if (!pParticleSystem->IsDead() && m_pCurrentState->GetShowEffects())
					{
						if (pParticleSystem->IsParticleSystem())
							SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
						else if (pParticleSystem->IsModifier())
						{
							for (SceneEntityList::iterator itEnts(itBegin); itEnts != lSceneEntities.end(); ++itEnts)
								static_cast<CSceneEntityModifier *>(pParticleSystem)->Modify((*itEnts)->cEntity);
						}
					}
				}

				// Cleanup
				m_apEffectThread[i]->Cleanup();
			}
		}

	} // PROFILE Effects
}


/*====================
  CClientEntity::StartEffect
  ====================*/
int		CClientEntity::StartEffect(ResHandle hEffect, int iChannel, int iTimeNudge)
{
	PROFILE("CClientEntity::StartEffect");

	// Search from an unused effect slot
	if (iChannel == -1)
	{
		for (int i(NUM_CLIENT_EFFECT_THREADS - 1); i >= NUM_EFFECT_CHANNELS; --i)
		{
			if (!m_apEffectThread[i])
			{
				iChannel = i;
				break;
			}
		}

		if (iChannel == -1)
			return -1;
	}
	else
	{
		if (iChannel >= int(NUM_CLIENT_EFFECTS))
			return -1;

		iChannel += NUM_EFFECT_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS
	}

	SAFE_DELETE(m_apEffectThread[iChannel])

	CEffect	*pEffect(g_ResourceManager.GetEffect(hEffect));

	if (pEffect)
	{
		m_apEffectThread[iChannel] = pEffect->SpawnThread(GameClient.GetGameTime() + iTimeNudge);

		if (m_apEffectThread[iChannel] == NULL)
			return -1;
		
		m_apEffectThread[iChannel]->SetCamera(GameClient.GetCamera());
		m_apEffectThread[iChannel]->SetWorld(GameClient.GetWorldPointer());

		m_apEffectThread[iChannel]->SetSourceSkeleton(m_pSkeleton);
		m_apEffectThread[iChannel]->SetSourceModel(g_ResourceManager.GetModel(m_pNextState->GetModelHandle()));
		m_apEffectThread[iChannel]->SetTargetSkeleton(NULL);
		m_apEffectThread[iChannel]->SetTargetModel(NULL);

		m_apEffectThread[iChannel]->SetActive(true);
		
		// TODO: we should use a timenudged lerped position instead of the previous frame's CurrentState
		m_apEffectThread[iChannel]->SetSourcePos(m_pCurrentState->GetPosition());

		if (m_pCurrentState->IsPlayer())
		{
			IPlayerEntity *pPlayer(m_pCurrentState->GetAsPlayerEnt());

			CVec3f v3Angles(0.0f, 0.0f, pPlayer->GetCurrentYaw());
			m_apEffectThread[iChannel]->SetSourceAxis(CAxis(v3Angles));
		}
		else
		{
			m_apEffectThread[iChannel]->SetSourceAxis(CAxis(m_pCurrentState->GetAngles()));
		}
		
		m_apEffectThread[iChannel]->SetSourceScale(m_pCurrentState->GetScale() * m_pCurrentState->GetScale2());
		
		if (m_apEffectThread[iChannel]->Execute(GameClient.GetGameTime() + iTimeNudge))
			SAFE_DELETE(m_apEffectThread[iChannel])
	}

	return iChannel;
}


/*====================
  CClientEntity::StartEffect
  ====================*/
int		CClientEntity::StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge)
{	
	return StartEffect(g_ResourceManager.Register(sEffect, RES_EFFECT), iChannel, iTimeNudge);
}


/*====================
  CClientEntity::StopEffect
  ====================*/
void	CClientEntity::StopEffect(int iChannel)
{
	if (m_apEffectThread[iChannel])
	{
		K2_DELETE(m_apEffectThread[iChannel]);
		m_apEffectThread[iChannel] = NULL;
	}
}


/*====================
  CClientEntity::PassEffects

  Give control of persistant effects to game events (for lingering rocket trails, etc)
  ====================*/
void	CClientEntity::PassEffects()
{
	for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
	{
		if (!m_apEffectThread[i])
			continue;

		if (!m_apEffectThread[i]->IsDeferred() && !m_apEffectThread[i]->IsPersistent())
			continue;

		m_apEffectThread[i]->SetCamera(NULL);

		m_apEffectThread[i]->SetSourceSkeleton(NULL);
		m_apEffectThread[i]->SetSourceModel(NULL);
		m_apEffectThread[i]->SetExpire(true);
		
		CGameEvent ev;
		ev.SetSourcePosition(m_pCurrentState->GetPosition());
		ev.SetSourceAngles(m_pCurrentState->GetAngles());
		ev.SetSourceScale(m_pCurrentState->GetScale() * m_pCurrentState->GetScale2());
		ev.SetEffect(m_apEffectThread[i]);
		ev.SetEffectActive(m_apEffectThread[i]->IsPersistent());
		ev.Spawn();
		Game.AddEvent(ev);

		m_apEffectThread[i] = NULL;
	}
}


/*====================
  CClientEntity::PlaySound
  ====================*/
void	CClientEntity::PlaySound(ResHandle hSample, float fVolume, float fFalloff, int iChannel, int iPriority, bool bLoop, int iFadeIn, int iFadeOutStartTime, int iFadeOut, bool bOverride, int iSpeedUpTime, float fSpeed1, float fSpeed2, int iSlowDownTime)
{
	PROFILE("CClientEntity::PlaySound");

	// Search from an unused sound slot
	if (iChannel == -1)
	{
		for (int i(NUM_CLIENT_SOUND_HANDLES - 1); i >= 0; --i)
		{
			if (m_ahSoundHandle[i] == INVALID_INDEX)
			{
				iChannel = i;
				break;
			}
		}

		if (iChannel == -1)
			return;
	}
	else
	{
		//iChannel += NUM_SOUND_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS
	}

	if (m_ahSoundHandle[iChannel] != INVALID_INDEX)
	{
		if (!bOverride)
			return;
		K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
		m_ahSoundHandle[iChannel] = INVALID_INDEX;
	}

	m_ahSoundHandle[iChannel] = K2SoundManager.PlaySFXSound(hSample, &m_pCurrentState->GetPosition(), &m_pCurrentState->GetVelocity(), fVolume, fFalloff, -1, iPriority, bLoop, iFadeIn, iFadeOutStartTime, iFadeOut, iSpeedUpTime, fSpeed1, fSpeed2, iSlowDownTime);
}


/*====================
  CClientEntity::StopSound
  ====================*/
void	CClientEntity::StopSound(int iChannel)
{
	if (iChannel == -1 || iChannel >= NUM_CLIENT_SOUND_HANDLES)
		return;
	
	//iChannel += NUM_SOUND_CHANNELS; // Offset to NUM_CLIENT_EFFECT_CHANNELS

	if (m_ahSoundHandle[iChannel] != INVALID_INDEX)
		K2SoundManager.StopHandle(m_ahSoundHandle[iChannel]);
}


/*====================
  CClientEntity::StopAllSounds
  ====================*/
void	CClientEntity::StopAllSounds()
{
	for (int i(0); i < NUM_CLIENT_SOUND_HANDLES; ++i)
	{
		if (m_ahSoundHandle[i] != INVALID_INDEX)
			K2SoundManager.StopHandle(m_ahSoundHandle[i]);
	}
}
