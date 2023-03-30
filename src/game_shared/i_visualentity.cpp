// (C)2007 S2 Games
// i_gameentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_visualentity.h"
#include "c_entityregistry.h"
#include "i_propentity.h"
#include "i_playerentity.h"
#include "c_teaminfo.h"
#include "c_playercommander.h"
#include "c_entityclientinfo.h"

#include "../k2/c_sceneentity.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_host.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_model.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_uitrigger.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IVisualEntity::s_pvFields;

SWorkingMovementVars	IVisualEntity::s_Move;
ResHandle				IVisualEntity::s_hSelectionIndicator(INVALID_RESOURCE);

const float SLIDE_OVERTRACE(2.0f);
const int	MAX_SLIDE_MOVES(8);

extern CCvar<bool>		p_debugSlide;
extern CCvar<float>		p_gravity;
extern CCvar<float>		p_maxWalkSlope;
extern CCvar<float>		p_landVelocity;
extern CCvar<float>		p_stepHeight;
extern CCvar<bool>		p_debugStep;
extern CCvar<float>		p_groundEpsilon;

CVAR_BOOL(		g_TeamDamage,	false);

CVAR_FLOATF(	g_expHealing,				0.25f,			CVAR_GAMECONFIG);
CVAR_FLOATF(	g_goldHealing,				0.25f,			CVAR_GAMECONFIG);
CVAR_UINTF(		g_playerDamageExpireTime,	SecToMs(10u),	CVAR_GAMECONFIG);
CVAR_UINTF(		g_buildingDamageExpireTime,	SecToMs(120u),	CVAR_GAMECONFIG);
CVAR_FLOATF(	g_expLastHitBonus,			0.2f,			CVAR_GAMECONFIG);
CVAR_FLOATF(	g_goldLastHitBonus,			0.2f,			CVAR_GAMECONFIG);

CVAR_FLOATF(	g_assistDamagePercent,		0.3f,			CVAR_GAMECONFIG);
CVAR_UINTF(		g_assistExpireTime,			SecToMs(30u),	CVAR_GAMECONFIG);

CVAR_UINT(		g_minimapFlashInterval,		500);
//=============================================================================

/*====================
  IVisualEntity::CEntityConfig::CEntityConfig
  ====================*/
IVisualEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IGameEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Name, _T("NAME")),
INIT_ENTITY_CVAR(Description, _T("DESCRIPTION")),
INIT_ENTITY_CVAR(Race, _T("")),
INIT_ENTITY_CVAR(ModelPath, _T("")),
INIT_ENTITY_CVAR(Scale, 1.0f),
INIT_ENTITY_CVAR(CommanderScale, 1.0f),
INIT_ENTITY_CVAR(MaxHealth, 0.05f),
INIT_ENTITY_CVAR(HealthRegenRate, 2.5f),
INIT_ENTITY_CVAR(Cost, 0),
INIT_ENTITY_CVAR(ExperienceValue, 100.0f),
INIT_ENTITY_CVAR(Bounty, 200),
INIT_ENTITY_CVAR(Prerequisite, _T("")),
INIT_ENTITY_CVAR(HitByMeleeEffectPath, _T("")),
INIT_ENTITY_CVAR(HitByRangedEffectPath, _T("")),
INIT_ENTITY_CVAR(IconPath, _T("")),
INIT_ENTITY_CVAR(MinimapIconPath, _T("$white")),
INIT_ENTITY_CVAR(MinimapIconSize, 4),
INIT_ENTITY_CVAR(LargeMapIconPath, _T("$white")),
INIT_ENTITY_CVAR(LargeMapIconSize, 8),
INIT_ENTITY_CVAR(SightRange, 0.0f),
INIT_ENTITY_CVAR(IsHidden, false),
INIT_ENTITY_CVAR(GameTip, _T("")),
INIT_ENTITY_CVAR(CommanderPortraitPath, _T("")),
INIT_ENTITY_CVAR(EffectScale, 1.0f),
INIT_ENTITY_CVAR(SelectionRadius, 0.0f),
INIT_ENTITY_CVAR(PushMultiplier, 1.0f),
INIT_ENTITY_CVAR(SpawnPoint, false),
INIT_ENTITY_CVAR(SiegeSpawnPoint, false)
{
}


/*====================
  IVisualEntity::~IVisualEntity
  ====================*/
IVisualEntity::~IVisualEntity()
{
	ClearStates();
	ClearInventory();
}


/*====================
  IVisualEntity::IVisualEntity
  ====================*/
IVisualEntity::IVisualEntity(CEntityConfig *pConfig) :
IGameEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiWorldIndex(INVALID_INDEX),
m_iTeam(-1),
m_ySquad(-1),

m_yStatus(ENTITY_STATUS_DORMANT),

m_uiCreationTime(0),
m_uiCorpseTime(0),

m_fHealth(0.0f),
m_bInvulnerable(false),

m_v3Position(V_ZERO),
m_v3Velocity(V_ZERO),
m_v3Angles(V_ZERO),
m_fScale(1.0f),
m_bbBounds(V3_ZERO, V3_ZERO),
m_uiGroundEntityIndex(INVALID_INDEX),
m_bOnGround(false),

m_yNextEventSlot(0),
m_yLastProcessedEvent(0),

m_uiLocalFlags(0),

m_uiRenderFlags(0),
m_hModel(INVALID_RESOURCE),
m_pSkeleton(NULL),

m_uiClientRenderFlags(0),
m_v4SelectColor(1.0f, 1.0f, 1.0f, 1.0f),
m_aAxis(0.0f, 0.0f, 0.0f),
m_v3AxisAngles(V3_ZERO),
m_bSighted(false),
m_bPrevSighted(false),
m_v3SightedPos(0.0f, 0.0f, 0.0f),
m_hMinimapIcon(INVALID_RESOURCE),
m_hLargeMapIcon(INVALID_RESOURCE),
m_uiLastTerrainTypeUpdateTime(INVALID_TIME),

m_uiMinimapFlashEndTime(0),
m_bShowEffects(false)
{
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
		m_apState[i] = NULL;

	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		m_ayAnim[i] = ENTITY_INVALID_ANIM;
		m_ayAnimSequence[i] = 0;
		m_afAnimSpeed[i] = 1.0f;
		m_auiAnimLockTime[i] = INVALID_TIME;
	}

	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		m_ahEffect[i] = INVALID_RESOURCE;
		m_ayEffectSequence[i] = 0;
	}

	for (int i(0); i < MAX_INVENTORY; ++i)
		m_apInventory[i] = NULL;
}


/*====================
  IVisualEntity::GetGunManaCost
  ====================*/
float	IVisualEntity::GetGunManaCost(float fManaCost) const
{
	FloatMod modGunManaCost;
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		modGunManaCost += m_apState[i]->GetGunManaCostMod();
	}

	return modGunManaCost.Modify(fManaCost);
}


/*====================
  IVisualEntity::MinimapFlash
  ====================*/
void	IVisualEntity::MinimapFlash(const CVec4f &v4Color, uint uiDuration)
{
	m_v4MinimapFlashColor = v4Color;
	m_uiMinimapFlashEndTime = Game.GetGameTime() + uiDuration;
}


/*====================
  IVisualEntity::GetMapIconColor
  ====================*/
CVec4f	IVisualEntity::GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
	uint uiGameTime(Game.GetGameTime());
	if (uiGameTime < m_uiMinimapFlashEndTime && (uiGameTime % g_minimapFlashInterval < (g_minimapFlashInterval / 2)))
		return m_v4MinimapFlashColor;

	if (pLocalPlayer == NULL || pLocalPlayer->GetTeam() < 1 || GetTeam() == 0)
		return ORANGE;
	else if (pLocalPlayer->LooksLikeEnemy(this))
		return RED;
	else if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return GREEN;
	else
		return LIME;
}


/*====================
  IVisualEntity::UpdateSighting
  ====================*/
void	IVisualEntity::UpdateSighting(const vector<IVisualEntity *> &vVision)
{
	bool bSighted(false);

	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
	{
		SetPrevSighted(true);
		SetSighted(true);
		SetSightedPos(GetPosition());
		return;
	}

	// Check the sight radius of all allies
	for (vector<IVisualEntity *>::const_iterator it(vVision.begin()); it != vVision.end(); ++it)
	{
		if (DistanceSq(GetPosition().xy(), (*it)->GetPosition().xy()) <= SQR((*it)->GetSightRange()))
		{
			bSighted = true;
			break;
		}
	}

	if (bSighted)
	{
		SetPrevSighted(true);
		SetSighted(true);
		SetSightedPos(GetPosition());
	}
	else
	{
		SetSighted(false);
	}
}


/*====================
  IVisualEntity::IsVisibleOnMinimap
  ====================*/
bool	IVisualEntity::IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
	int iTeam(pLocalPlayer == NULL ? -1 : pLocalPlayer->GetTeam());

	if (!GetSighted())
		return false;

	if (GetMapIcon(pLocalPlayer, bLargeMap) == INVALID_RESOURCE)
		return false;

	if (GetStatus() != ENTITY_STATUS_ACTIVE &&
		GetStatus() != ENTITY_STATUS_DEAD)
		return false;

	if (GetStatus() == ENTITY_STATUS_DEAD && GetTeam() != iTeam)
		return false;

	if (IsStealthed() && GetTeam() != iTeam)
		return false;
	if (GetIsHidden() && GetTeam() != iTeam)
		return false;

	return true;
}


/*====================
  IVisualEntity::GetMapIconSize
  ====================*/
float	IVisualEntity::GetMapIconSize(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
	if (bLargeMap)
		return float(GetLargeMapIconSize());
	else
		return float(GetMinimapIconSize());
}


/*====================
  IVisualEntity::GetMapIcon
  ====================*/
ResHandle	IVisualEntity::GetMapIcon(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
	if (bLargeMap && GetLargeMapIcon() != INVALID_RESOURCE)
		return GetLargeMapIcon();

	return GetMinimapIcon();
}


/*====================
  IVisualEntity::DrawOnMap
  ====================*/
void	IVisualEntity::DrawOnMap(CUITrigger &minimap, IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
	if (!IsVisibleOnMinimap(pLocalPlayer, bLargeMap))
		return;

	if (bLargeMap && CanSpawnFrom(pLocalPlayer))
	{
		CBufferFixed<64> buffer;
		
		// Position
		buffer << GetPosition().x / Game.GetWorldWidth();
		buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
		
		// Size and color
		buffer << float(GetLargeMapIconSize());
		buffer << float(GetLargeMapIconSize());
		buffer << 1.0f; // Color R
		buffer << 1.0f; // Color G
		buffer << 1.0f; // Color B
		buffer << 1.0f; // Color A

		// Mouse over size and color
		buffer << GetLargeMapIconSize() * 1.5f;
		buffer << GetLargeMapIconSize() * 1.5f;
		buffer << 1.0f; // Color R
		buffer << 1.0f; // Color G
		buffer << 1.0f; // Color B
		buffer << 1.0f; // Color A
		
		buffer << GetLargeMapIcon();
		buffer << GetIndex();

		minimap.Execute(_T("button"), buffer);
	}
	else
	{
		CBufferFixed<36> buffer;
		
		buffer << GetSightedPos().x / Game.GetWorldWidth();
		buffer << 1.0f - (GetSightedPos().y / Game.GetWorldHeight());
		
		buffer << GetMapIconSize(pLocalPlayer, bLargeMap) << GetMapIconSize(pLocalPlayer, bLargeMap);
		
		CVec4f v4Color(GetMapIconColor(pLocalPlayer, bLargeMap));
		buffer << v4Color[R];
		buffer << v4Color[G];
		buffer << v4Color[B];
		buffer << v4Color[A];

		buffer << GetMapIcon(pLocalPlayer, bLargeMap);

		minimap.Execute(_T("icon"), buffer);
	}
}


/*====================
  IVisualEntity::Baseline
  ====================*/
void	IVisualEntity::Baseline()
{
	IGameEntity::Baseline();

	// Basic data
	m_v3Position = V3_ZERO;
	m_v3Angles = V3_ZERO;
	
	m_yStatus = ENTITY_STATUS_ACTIVE;

	m_fHealth = 0.0f;

	// Anims
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		m_ayAnim[i] = ENTITY_INVALID_ANIM;
		m_ayAnimSequence[i] = 0;
		m_afAnimSpeed[i] = 1.0f;
	}

	// Effects
	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		m_ahEffect[i] = INVALID_RESOURCE;
		m_ayEffectSequence[i] = 0;
	}

	m_iTeam = -1;
	m_ySquad = -1;
}


/*====================
  IVisualEntity::GetGameTip
  ====================*/
bool	IVisualEntity::GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage)
{
	if (pPlayer == NULL)
		return false;
	if (GetTeam() != pPlayer->GetTeam())
		return false;

	sMessage = GetGameTip();
	if (sMessage.empty())
		return false;
	return true;
}


/*====================
  IVisualEntity::Heal
  ====================*/
float	IVisualEntity::Heal(float fHealth, IVisualEntity *pSource)
{
	fHealth = MIN(fHealth, GetMaxHealth() - GetHealth());
	SetHealth(CLAMP(GetHealth() + fHealth, 0.0f, GetMaxHealth()));

	int iClientID(-1);
	if (IsPlayer())
		iClientID = GetAsPlayerEnt()->GetClientID();

	if (pSource != NULL && pSource->GetAsPlayerEnt() != NULL && fHealth > 0.0f)
	{
		Game.MatchStatEvent(pSource->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_HEALED, fHealth, iClientID, pSource->GetType());
		Game.MatchStatEvent(pSource->GetAsPlayerEnt()->GetClientID(), COMMANDER_MATCH_HEALED, fHealth, iClientID, pSource->GetType());
	}

	return fHealth;
}


/*====================
  IVisualEntity::GetScale2
  ====================*/
float	IVisualEntity::GetScale2() const
{
	return (Game.IsCommander() ? GetCommanderScale() : 1.0f);
}


/*====================
  IVisualEntity::GetModelHandle
  ====================*/
ResHandle	IVisualEntity::GetModelHandle() const
{
	ResHandle hModel(m_hModel);
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;
		if (m_apState[i]->GetModelHandle() == INVALID_INDEX)
			continue;

		hModel = m_apState[i]->GetModelHandle();
	}

	return hModel;
}


/*====================
  IVisualEntity::GetSnapshot
  ====================*/
void	IVisualEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IGameEntity::GetSnapshot(snapshot);

	// Basic data
	snapshot.AddField(m_v3Position);
	snapshot.AddAngle16(m_v3Angles.x);
	snapshot.AddAngle16(m_v3Angles.y);
	snapshot.AddAngle16(m_v3Angles.z);

	snapshot.AddField(m_yStatus);

	snapshot.AddRound16(m_fHealth);

	// Anims
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		snapshot.AddField(m_ayAnim[i]);
		snapshot.AddField(m_ayAnimSequence[i]);
		snapshot.AddField(m_afAnimSpeed[i]);
	}

	// Effects
	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		snapshot.AddResHandle(m_ahEffect[i]);
		snapshot.AddField(m_ayEffectSequence[i]);
	}

	snapshot.AddField(m_iTeam);
	snapshot.AddField(m_ySquad);
}


/*====================
  IVisualEntity::ReadSnapshot
  ====================*/
bool	IVisualEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		// Base entity info
		if (!IGameEntity::ReadSnapshot(snapshot))
			return false;
		
		snapshot.ReadNextField(m_v3Position);
		snapshot.ReadNextAngle16(m_v3Angles.x);
		snapshot.ReadNextAngle16(m_v3Angles.y);
		snapshot.ReadNextAngle16(m_v3Angles.z);
		snapshot.ReadNextField(m_yStatus);

		snapshot.ReadNextRound16(m_fHealth);

		// Anims
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
		{
			snapshot.ReadNextField(m_ayAnim[i]);
			snapshot.ReadNextField(m_ayAnimSequence[i]);
			snapshot.ReadNextField(m_afAnimSpeed[i]);
		}

		// Effects
		for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
		{
			snapshot.ReadNextResHandle(m_ahEffect[i]);
			snapshot.ReadNextField(m_ayEffectSequence[i]);
		}

		snapshot.ReadNextField(m_iTeam);
		snapshot.ReadNextField(m_ySquad);

		Validate();

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IVisualEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IVisualEntity::GetTypeVector
  ====================*/
const vector<SDataField>&	IVisualEntity::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

		s_pvFields->push_back(SDataField(_T("m_v3Position"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[PITCH]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[ROLL]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[YAW]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_yStatus"), FIELD_PUBLIC, TYPE_CHAR));
		
		s_pvFields->push_back(SDataField(_T("m_fHealth"), FIELD_PUBLIC, TYPE_ROUND16));

		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
		{
			s_pvFields->push_back(SDataField(_T("m_ayAnim[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_CHAR));
			s_pvFields->push_back(SDataField(_T("m_ayAnimSequence[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_CHAR));
			s_pvFields->push_back(SDataField(_T("m_afAnimSpeed[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_FLOAT));
		}

		// Effects
		for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
		{
			s_pvFields->push_back(SDataField(_T("m_ahEffect[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_RESHANDLE));
			s_pvFields->push_back(SDataField(_T("m_ayEffectSequence[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_CHAR));
		}

		s_pvFields->push_back(SDataField(_T("m_iTeam"), FIELD_PUBLIC, TYPE_CHAR));
		s_pvFields->push_back(SDataField(_T("m_ySquad"), FIELD_PUBLIC, TYPE_CHAR));
	}

	return *s_pvFields;
}


/*====================
  IVisualEntity::RegisterEntityScripts
  ====================*/
void	IVisualEntity::RegisterEntityScripts(const CWorldEntity &ent)
{
	Game.RegisterEntityScript(GetIndex(), _T("death"), ent.GetProperty(_T("triggerdeath"), _T("")));
	Game.RegisterEntityScript(GetIndex(), _T("damage"), ent.GetProperty(_T("triggerdamage"), _T("")));
}


/*====================
  IVisualEntity::ApplyWorldEntity
  ====================*/
void	IVisualEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
	m_sName = ent.GetName();
	m_hModel = Game.RegisterModel(ent.GetModelPath());
	m_uiWorldIndex = ent.GetIndex();
	m_v3Position = ent.GetPosition();
	m_v3Angles = ent.GetAngles();
	m_fScale = ent.GetScale();
	SetTeam(ent.GetTeam());

	RegisterEntityScripts(ent);

	// Starting anim
	CModel* pModel(g_ResourceManager.GetModel(GetModelHandle()));
	if (pModel != NULL &&
		pModel->GetModelFile()->GetType() == MODEL_K2)
	{
		CK2Model* pK2Model(static_cast<CK2Model*>(pModel->GetModelFile()));
		if (pK2Model->GetNumAnims() > 0)
			m_yDefaultAnim = pK2Model->GetAnimIndex(ent.GetProperty(_T("anim"), _T("idle")));
	}
}


/*====================
  IVisualEntity::Spawn
  ====================*/
void	IVisualEntity::Spawn()
{
	IGameEntity::Spawn();

	if (Game.IsClient())
	{
		if (!GetMinimapIconPath().empty())
			m_hMinimapIcon = g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetMinimapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

		if (!GetLargeMapIconPath().empty())
			m_hLargeMapIcon = g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetLargeMapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

		if (s_hSelectionIndicator == INVALID_RESOURCE)
			s_hSelectionIndicator = g_ResourceManager.Register(_T("/shared/effects/materials/selection_indicator.material"), RES_MATERIAL);
	}
}


/*====================
  IVisualEntity::ServerFrame
  ====================*/
bool	IVisualEntity::ServerFrame()
{
	PROFILE("IVisualEntity::ServerFrame");

	if (!IGameEntity::ServerFrame())
		return false;

	// Check for commander selection
	for (int i(1); i <= 2; ++i)
	{
		RemoveNetFlags(ENT_NET_FLAG_COMMANDER_SELECTED(i));
		CEntityTeamInfo *pTeam(Game.GetTeam(i));
		if (pTeam != NULL)
		{
			CPlayerCommander *pCommander(pTeam->GetCommanderPlayerEntity());
			if (pCommander != NULL)
			{
				uiset setSelected(pCommander->GetSelection());
				if (setSelected.find(GetIndex()) != setSelected.end())
					SetNetFlags(ENT_NET_FLAG_COMMANDER_SELECTED(i));
			}
		}
	}

	UpdateStates();
	return true;
}


/*====================
  IVisualEntity::UpdateSkeleton
  ====================*/
void	IVisualEntity::UpdateSkeleton(bool bPose)
{
	if (m_pSkeleton == NULL)
		return;

	m_pSkeleton->SetModel(GetModelHandle());

	if (GetModelHandle() == INVALID_RESOURCE)
		return;

	// Pose skeleton
	if (bPose)
		m_pSkeleton->Pose(Game.GetGameTime());
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
  IVisualEntity::StartAnimation
  ====================*/
void	IVisualEntity::StartAnimation(const tstring &sAnimName, int iChannel, float fSpeed, uint uiLength)
{
	if (iChannel == -1)
	{
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			StartAnimation(sAnimName, i, fSpeed, uiLength);

		return;
	}

	if (iChannel < 0 || iChannel >= NUM_ANIM_CHANNELS)
	{
		Console.Warn << _T("IVisualEntity::StartAnimation() - Invalid animation channel: ") << iChannel
					<< _T(", using 0") << newl;
		iChannel = 0;
	}

	if (m_auiAnimLockTime[iChannel] != INVALID_TIME && m_auiAnimLockTime[iChannel] > Game.GetGameTime())
		return;

	m_ayAnim[iChannel] = g_ResourceManager.GetAnim(GetModelHandle(), sAnimName);

	float fLength(uiLength != 0 ? float(g_ResourceManager.GetAnimLength(GetModelHandle(), m_ayAnim[iChannel])) / uiLength : 1.0f);
	
	m_ayAnimSequence[iChannel] = (m_ayAnimSequence[iChannel] + 1) & 0xff;
	m_afAnimSpeed[iChannel] = fSpeed * fLength;
}


/*====================
  IVisualEntity::SetDefaultAnimation
  ====================*/
void	IVisualEntity::SetDefaultAnimation(const tstring &sAnimName)
{
	m_yDefaultAnim = g_ResourceManager.GetAnim(GetModelHandle(), sAnimName);
}


/*====================
  IVisualEntity::StopAnimation
  ====================*/
void	IVisualEntity::StopAnimation(int iChannel)
{
	if (iChannel == -1)
	{
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			StopAnimation(i);

		return;
	}

	if (iChannel < 0 || iChannel >= NUM_ANIM_CHANNELS)
	{
		Console.Warn << _T("IVisualEntity::StopAnimation() - Invalid animation channel: ") << iChannel << newl;
		return;
	}

	m_ayAnim[iChannel] = ENTITY_STOP_ANIM;
	m_ayAnimSequence[iChannel] = (m_ayAnimSequence[iChannel] + 1) & 0xff;
	m_afAnimSpeed[iChannel] = 1.0f;
}


/*====================
  IVisualEntity::StopAnimation

  Conditional stop
  ====================*/
void	IVisualEntity::StopAnimation(const tstring &sAnimName, int iChannel)
{
	if (iChannel == -1)
	{
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			StopAnimation(sAnimName, i);

		return;
	}

	if (iChannel < 0 || iChannel >= NUM_ANIM_CHANNELS)
	{
		Console.Warn << _T("IVisualEntity::StopAnimation() - Invalid animation channel: ") << iChannel << newl;
		return;
	}

	if (m_ayAnim[iChannel] == g_ResourceManager.GetAnim(GetModelHandle(), sAnimName))
	{
		m_ayAnim[iChannel] = ENTITY_STOP_ANIM;
		m_ayAnimSequence[iChannel] = (m_ayAnimSequence[iChannel] + 1) & 0xff;
		m_afAnimSpeed[iChannel] = 1.0f;
	}
}


/*====================
  IVisualEntity::LockAnimation
  ====================*/
void	IVisualEntity::LockAnimation(int iChannel, uint uiTime)
{
	if (iChannel == -1)
	{
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
			LockAnimation(i, uiTime);

		return;
	}

	if (iChannel < 0 || iChannel >= NUM_ANIM_CHANNELS)
	{
		Console.Warn << _T("IVisualEntity::LockAnimation() - Invalid animation channel: ") << iChannel << newl;
		return;
	}

	m_auiAnimLockTime[iChannel] = uiTime;
}


/*====================
  IVisualEntity::GetAnimIndex
  ====================*/
int		IVisualEntity::GetAnimIndex(const tstring &sAnimName)
{
	return g_ResourceManager.GetAnim(GetModelHandle(), sAnimName);
}


/*====================
  IVisualEntity::AddToScene
  ====================*/
bool	IVisualEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
	PROFILE("IVisualEntity::AddToScene");

	if (GetModelHandle() == INVALID_INDEX)
		return false;

	CVec4f v4TintedColor(v4Color);

	if (Game.IsCommander() && !m_bSighted)
	{
		//v4TintedColor[R] *= 0.333f;
		//v4TintedColor[G] *= 0.333f;
		//v4TintedColor[B] *= 0.333f;
	}

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


/*====================
  IVisualEntity::AddSelectionRingToScene
  ====================*/
void	IVisualEntity::AddSelectionRingToScene()
{
	CEntityClientInfo *pLocalClient(Game.GetClientInfo(Game.GetLocalClientNum()));
	if (pLocalClient == NULL)
		return;

	float fSize(GetSelectionRadius());
	if (fSize <= 0.0f)
		return;

	if (!HasNetFlags(ENT_NET_FLAG_COMMANDER_SELECTED(pLocalClient->GetTeam())) &&
		!Game.IsEntityHoverSelected(m_uiIndex))
		return;

	if (!Game.IsCommander())
		fSize /= GetCommanderScale();

	CSceneEntity sceneEntity;
	sceneEntity.Clear();

	sceneEntity.width = fSize * 2.0f;
	sceneEntity.height = fSize * 2.0f;
	sceneEntity.scale = m_fScale;
	sceneEntity.SetPosition(m_v3Position);
	sceneEntity.objtype = OBJTYPE_GROUNDSPRITE;
	sceneEntity.hMaterial = s_hSelectionIndicator;
	sceneEntity.flags = SCENEOBJ_SOLID_COLOR;

	if (HasNetFlags(ENT_NET_FLAG_COMMANDER_SELECTED(pLocalClient->GetTeam())))
	{
		sceneEntity.color = IsNeutral() ? WHITE : Game.LooksLikeEnemy(m_uiIndex) ? RED : LIME;
		SceneManager.AddEntity(sceneEntity);
	}
	else if (Game.IsEntityHoverSelected(m_uiIndex))
	{
		sceneEntity.color = IsNeutral() ? CVec4f(WHITE.xyz(), 0.5f) : Game.LooksLikeEnemy(m_uiIndex) ? CVec4f(RED.xyz(), 0.5f) : CVec4f(LIME.xyz(), 0.5f);
		SceneManager.AddEntity(sceneEntity);
	}
}



/*====================
  IVisualEntity::ClearStates
  ====================*/
void	IVisualEntity::ClearStates()
{
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
		RemoveState(i);
}


/*====================
  IVisualEntity::UpdateStates
  ====================*/
void	IVisualEntity::UpdateStates()
{
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (!m_apState[i])
			continue;

		// See if a state should be applied
		if (!m_apState[i]->IsValid() || (m_apState[i]->GetExpireTime() != INVALID_TIME && m_apState[i]->GetExpireTime() <= Game.GetGameTime()))
		{
			RemoveState(i);
			continue;
			break; // Leave this here
		}
	}

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (!m_apState[i])
			continue;
		
		m_apState[i]->StateFrame();

		// Damage and such can remove states, so check again...
		if (!m_apState[i])
			continue;
		
		if (!m_apState[i]->IsValid())
			RemoveState(i);
	}
}


/*====================
  IVisualEntity::ApplyState
  ====================*/
int		IVisualEntity::ApplyState(ushort unID, uint uiStartTime, uint uiDuration, uint uiInflictorIndex)
{
	try
	{
		if (!Game.IsServer())
			return -1;

		IEntityState **ppPrevState = NULL;

		for (int iSlot(0); iSlot < MAX_ACTIVE_ENTITY_STATES; ++iSlot)
		{
			if (m_apState[iSlot] && m_apState[iSlot]->IsMatch(unID))
			{
				ppPrevState = &m_apState[iSlot];
				break;
			}
		}

		// Determine slot for state
		if (ppPrevState && !(*ppPrevState)->GetStack())
		{
			(*ppPrevState)->SetStartTime(uiStartTime);
			(*ppPrevState)->SetDuration(uiDuration);
			(*ppPrevState)->SetInflictor(uiInflictorIndex);
			
			//(*ppPrevState)->Activated();
		}
		else
		{
			// Allocate the state
			IGameEntity *pNew(Game.AllocateEntity(unID, GetIndex()));
			if (pNew == NULL)
				EX_ERROR(_T("Invalid state"));

			IEntityState *pNewState(pNew->GetAsState());
			if (pNewState == NULL)
				EX_ERROR(_T("Invalid state"));

			pNewState->SetStartTime(uiStartTime);
			pNewState->SetDuration(uiDuration);
			pNewState->SetOwner(GetIndex());
			pNewState->SetInflictor(uiInflictorIndex);

			int i(0);
			for (; i < MAX_ACTIVE_ENTITY_STATES; ++i)
			{
				if (m_apState[i] == NULL)
					break;
			}
			if (i == MAX_ACTIVE_ENTITY_STATES)
				return -1;
				
			m_apState[i] = pNewState;
			m_apState[i]->Activated();

			return i;
		}

		return -1;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IVisualEntity::ApplyState() - "), NO_THROW);
		return -1;
	}
}


/*====================
  IVisualEntity::RemoveState
  ====================*/
void	IVisualEntity::RemoveState(int iSlot)
{
	if (iSlot < 0 || iSlot > MAX_ACTIVE_ENTITY_STATES)
		EX_ERROR(_T("Invalid slot specified"));

	if (m_apState[iSlot] == NULL)
		return;

	m_apState[iSlot]->Expired();
	Game.DeleteEntity(m_apState[iSlot]);
	m_apState[iSlot] = NULL;
}


/*====================
  IVisualEntity::AddState
  ====================*/
void	IVisualEntity::AddState(IEntityState *pState)
{
	for (int iSlot(0); iSlot < MAX_ACTIVE_ENTITY_STATES; ++iSlot)
	{
		if (m_apState[iSlot] && m_apState[iSlot] == pState)
			return;
	}

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i])
			continue;

		m_apState[i] = pState;
		return;
	}
}


/*====================
  IVisualEntity::ClearState
  ====================*/
void	IVisualEntity::ClearState(IEntityState *pState)
{
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (!m_apState[i] || m_apState[i] != pState)
			continue;

		m_apState[i] = NULL;
	}
}


/*====================
  IVisualEntity::GetStateExpireTime
  ====================*/
uint	IVisualEntity::GetStateExpireTime(int iSlot)
{
	if (iSlot < 0 || iSlot >= MAX_ACTIVE_ENTITY_STATES)
		return INVALID_TIME;
	return m_apState[iSlot]->GetExpireTime();
}


/*====================
  IVisualEntity::AddEvent
  ====================*/
void	IVisualEntity::AddEvent(EEntityEvent eEvent)
{
	m_aEvents[m_yNextEventSlot].Reset(eEvent);
	++m_yNextEventSlot;
	m_yNextEventSlot %= ENTITY_EVENT_BUFFER_LENGTH;
}


/*====================
  IVisualEntity::Hit
  ====================*/
void	IVisualEntity::Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy)
{
	ResHandle hHitEffectPath(INVALID_RESOURCE);

	switch (eHitBy)
	{
		case ENTITY_HIT_BY_MELEE:
			if (!GetHitByMeleeEffectPath().empty())
				hHitEffectPath = Game.RegisterEffect(GetHitByMeleeEffectPath());
		break;
		case ENTITY_HIT_BY_RANGED:
			if (!GetHitByRangedEffectPath().empty())
				hHitEffectPath = Game.RegisterEffect(GetHitByRangedEffectPath());
		break;
	}

	if (hHitEffectPath == INVALID_RESOURCE)
		return;

	//TODO: Set up impact angle properly
	CGameEvent evImpact;
	if (eHitBy == ENTITY_HIT_BY_MELEE)
		evImpact.SetSourcePosition(v3Pos + CVec3f(0,0,40.0-M_Randnum(0.0f,20.0f)));
	else
		evImpact.SetSourcePosition(v3Pos);
	evImpact.SetSourceAngles(v3Angle);
	evImpact.SetEffect(hHitEffectPath);
	Game.AddEvent(evImpact);
}


/*====================
  IVisualEntity::KillReward
  ====================*/
void	IVisualEntity::KillReward(IGameEntity *pKiller)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	if (pKiller != NULL)
	{
		if (pKiller->IsPet())
		{
			IGameEntity *pPetOwner(Game.GetEntityFromUniqueID(pKiller->GetAsPet()->GetOwnerUID()));
			if (pPetOwner != NULL)
				pKiller = pPetOwner;
		}
		if (pKiller->IsGadget())
		{
			IGameEntity *pGadgetOwner(Game.GetEntity(pKiller->GetAsGadget()->GetOwnerIndex()));
			if (pGadgetOwner != NULL)
				pKiller = pGadgetOwner;
		}
	}

	// Distribute gold and experience
	float fBonusExperience(GetExperienceValue() * g_expLastHitBonus);
	float fExperience(GetExperienceValue() - fBonusExperience);
	ushort unBonusGold(INT_ROUND(GetBounty() * g_goldLastHitBonus));
	ushort unGold(GetBounty() - unBonusGold);

	uint uiDamageExpireTime(g_playerDamageExpireTime);
	if (IsBuilding())
		uiDamageExpireTime = g_buildingDamageExpireTime;

	float fTotalDamage(0.0f);
	for (map<uint, SDamageRecord>::iterator it(m_mapDamage.begin()); it != m_mapDamage.end(); ++it)
	{
		if (Game.GetGameTime() - it->second.uiTime > uiDamageExpireTime)
			continue;
		fTotalDamage += it->second.fDamage;
	}

	if (fTotalDamage > 0.0f)
	{
		CVec3f v3Pos(m_v3Position + m_bbBounds.GetMid());
		for (map<uint, SDamageRecord>::iterator it(m_mapDamage.begin()); it != m_mapDamage.end(); ++it)
		{
			IVisualEntity *pVis(Game.GetVisualEntity(it->first));
			
			if (pVis == NULL)
				continue;

			if (pVis->IsPet())
			{
				IGameEntity *pPetOwner(Game.GetEntityFromUniqueID(pVis->GetAsPet()->GetOwnerUID()));
				if (pPetOwner != NULL && pPetOwner->IsVisual())
					pVis = pPetOwner->GetAsVisualEnt();
			}

			if (pVis->IsGadget())
			{
				IVisualEntity *pGadgetOwner(Game.GetVisualEntity(pVis->GetAsGadget()->GetOwnerIndex()));
				if (pGadgetOwner != NULL)
					pVis = pGadgetOwner;
			}

			IPlayerEntity *pPlayer(pVis->GetAsPlayerEnt());

			if (pPlayer != NULL && IsPlayer() && (pKiller == NULL || pPlayer->GetIndex() != pKiller->GetIndex()))
			{
				if (it->second.fDamage / fTotalDamage >= g_assistDamagePercent)
				{
					if (Game.GetGameTime() - it->second.uiTime <= g_assistExpireTime)
					{
						Game.MatchStatEvent(pPlayer->GetClientID(), PLAYER_MATCH_ASSISTS, 1, GetAsPlayerEnt()->GetClientID(), INVALID_ENT_TYPE, INVALID_ENT_TYPE, Game.GetGameTime());
						
						CBufferFixed<4> buffer;
						buffer << GAME_CMD_ASSIST_NOTIFICATION;

						Game.SendGameData(pPlayer->GetClientID(), buffer, true);
					}
				}
			}

			if (Game.GetGameTime() - it->second.uiTime > uiDamageExpireTime)
				continue;

			float fExperienceReward(fExperience * (it->second.fDamage / fTotalDamage));
			ushort unGoldReward(INT_ROUND(unGold * (it->second.fDamage / fTotalDamage)));
			if (pKiller != NULL && pKiller->GetIndex() == pVis->GetIndex())
			{
				fExperienceReward += fBonusExperience;
				unGoldReward += unBonusGold;
			}

			if (pPlayer != NULL)
			{
				pPlayer->GiveExperience(fExperienceReward, v3Pos);
				pPlayer->GiveGold(unGoldReward, v3Pos, true, true);

				if (IsPlayer())
					Game.MatchStatEvent(pPlayer->GetClientID(), PLAYER_MATCH_GOLD_EARNED, int(unGoldReward), GetAsPlayerEnt()->GetClientID(), (pKiller != NULL ? pKiller->GetType() : INVALID_ENT_TYPE));
				else
					Game.MatchStatEvent(pPlayer->GetClientID(), PLAYER_MATCH_GOLD_EARNED, int(unGoldReward), -1, (pKiller != NULL ? pKiller->GetType() : INVALID_ENT_TYPE), GetType());
			}
			else
			{
				CEntityTeamInfo *pTeam(Game.GetTeam(pVis->GetTeam()));

				if (pTeam == NULL)
					continue;

				pTeam->GiveGold(unGoldReward);

				CEntityClientInfo *pComm(pTeam->GetCommanderClient());

				if (pComm != NULL)
				{
					pComm->GiveExperience(fExperienceReward, v3Pos);

					if (pVis != NULL && pKiller != NULL && pKiller->GetIndex() == pVis->GetIndex())
						Game.MatchStatEvent(pComm->GetClientNumber(), COMMANDER_MATCH_KILLS, 1, (IsPlayer() ? GetAsPlayerEnt()->GetClientID() : -1), pVis->GetType(), GetType());
				}
			}
		}
	}
}


/*====================
  IVisualEntity::RecordDamageCredit
  ====================*/
void	IVisualEntity::RecordDamageCredit(uint uiIndex, float fDamage)
{
	if (uiIndex == INVALID_INDEX)
		return;

	map<uint, SDamageRecord>::iterator it(m_mapDamage.find(uiIndex));
	if (it == m_mapDamage.end())
	{
		m_mapDamage[uiIndex] = SDamageRecord(fDamage, Game.GetGameTime());
	}
	else
	{
		it->second.fDamage += fDamage;
		it->second.uiTime = Game.GetGameTime();
	}
}


/*====================
  IVisualEntity::Damage
  ====================*/
float	IVisualEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
	if (IsInvulnerable())
		return 0.0f;

	// Ignore meaningless damage
	if (fDamage <= 0.0f)
		return 0.0f;
	if (m_fHealth <= 0.0f)
		return 0.0f;

	uint uiAttackingIndex(INVALID_INDEX);

	if (pAttacker != NULL)
	{
		if (pAttacker->IsPet())
		{
			IGameEntity *pPetOwner(Game.GetEntityFromUniqueID(pAttacker->GetAsPet()->GetOwnerUID()));
			if (pPetOwner != NULL && pPetOwner->GetAsVisualEnt() != NULL)
				pAttacker = pPetOwner->GetAsVisualEnt();
		}
		if (pAttacker->IsGadget())
		{
			IGameEntity *pGadgetOwner(Game.GetEntity(pAttacker->GetAsGadget()->GetOwnerIndex()));
			if (pGadgetOwner != NULL && pGadgetOwner->GetAsVisualEnt() != NULL)
				pAttacker = pGadgetOwner->GetAsVisualEnt();
		}

		uiAttackingIndex = pAttacker->GetIndex();
	}

	if (!(iFlags & DAMAGE_FLAG_DIRECT) && !g_TeamDamage && pAttacker != NULL)
	{
		// Check for same team or allied team
		if (!IsEnemy(pAttacker))
			return 0.0f;
	}

	// Allow inventory items to react to damage
	// FIXME: Doing damage adjustment this way could cause unpredicatable results
	// due to interactions between differnt items
	for (int i(0); i < MAX_INVENTORY; ++i)
	{
		if (GetItem(i) != NULL)
			fDamage = GetItem(i)->OwnerDamaged(fDamage, iFlags, pAttacker);
	}

	// Allow states to react to damage
	// FIXME: Doing damage adjustment this way could cause unpredicatable results
	// due to interactions between differnt states
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		fDamage = m_apState[i]->OwnerDamaged(fDamage, iFlags, pAttacker);
		if (!m_apState[i]->IsValid())
			RemoveState(i);
	}

	// Recheck damage since it could have been reduced to or below 0
	if (fDamage <= 0.0f)
		return 0.0f;

	if (fDamage >= m_fHealth)
		fDamage = m_fHealth;

	tstring sMethod(_T("Unknown"));
	if (unDamagingObjectID != INVALID_ENT_TYPE)
	{
		ICvar *pCvar(EntityRegistry.GetGameSetting(unDamagingObjectID, _T("Name")));

		if (pCvar != NULL)
			sMethod = pCvar->GetString();
	}

	Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
	Game.RegisterTriggerParam(_T("attackingindex"), XtoA(uiAttackingIndex));
	Game.RegisterTriggerParam(_T("damage"), XtoA(fDamage));
	Game.RegisterTriggerParam(_T("method"), sMethod);
	Game.RegisterTriggerParam(_T("blockable"), XtoA((iFlags & DAMAGE_FLAG_BLOCKABLE) && (iFlags & DAMAGE_FLAG_MELEE) && !(iFlags & DAMAGE_FLAG_DIRECT)));
	Game.TriggerEntityScript(GetIndex(), _T("damage"));

	if (pAttacker != NULL && pAttacker->IsPlayer())
	{
		if (IsPlayer())
		{
			Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_PLAYER_DAMAGE, fDamage, GetAsPlayerEnt()->GetClientID(), unDamagingObjectID);
			Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), COMMANDER_MATCH_PLAYER_DAMAGE, fDamage, GetAsPlayerEnt()->GetClientID(), unDamagingObjectID);
		}
		else if (IsBuilding())
			Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_BUILDING_DAMAGE, fDamage, -1, unDamagingObjectID, GetType());
	}

	// Hit notification
	if (bFeedback && pAttacker != NULL &&
		pAttacker->IsPlayer() &&
		(iFlags & DAMAGE_FLAG_NOTIFY) &&
		!(iFlags & DAMAGE_FLAG_SILENT))
	{
		if (IsPlayer() && (iFlags & DAMAGE_FLAG_MELEE || iFlags & DAMAGE_FLAG_MELEE_NOTIFY))
		{
			if (GetAsPlayerEnt()->GetIsVehicle())
			{
				CBufferFixed<4> buffer;
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_GOT_HITBYMELEE_NOKICK) << ushort(m_uiIndex);
				Game.SendGameData(GetAsPlayerEnt()->GetClientID(), buffer, false);
			}
			else
			{
				CBufferFixed<4> buffer;
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_GOT_HITBYMELEE) << ushort(m_uiIndex);
				Game.SendGameData(GetAsPlayerEnt()->GetClientID(), buffer, false);
			}
		}

		CBufferFixed<4>	buffer;

		if (iFlags & DAMAGE_FLAG_MELEE || iFlags & DAMAGE_FLAG_MELEE_NOTIFY)
		{
			if (IsPlayer() || IsPet() || IsNpc())
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_MELEEHIT) << ushort(m_uiIndex);
			else if (IsBuilding())
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_BUILDING_MELEE) << ushort(m_uiIndex);
			else
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_MELEE) << ushort(m_uiIndex);
		}
		else if (iFlags & (DAMAGE_FLAG_GUN | DAMAGE_FLAG_SIEGE))
		{
			if (IsPlayer() || IsPet() || IsNpc())
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_UNIT_RANGED) << ushort(m_uiIndex);
			else if (IsBuilding())
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_BUILDING_RANGED) << ushort(m_uiIndex);
			else
				buffer << GAME_CMD_HITFEEDBACK << byte(HIT_RANGED) << ushort(m_uiIndex);
		}
		else
		{
			buffer << GAME_CMD_HITFEEDBACK << byte(HIT_GENERIC) << ushort(m_uiIndex);
		}
		
		Game.SendGameData(pAttacker->GetAsPlayerEnt()->GetClientID(), buffer, false);
	}

	// Check for total credit claimed by attacker buffs and target debuffs
	float fTotalStateCredit(0.0f);
	float fAdjustedStateCredit(0.0f);
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		IEntityState *pTargetState(GetState(i));
		if (pTargetState != NULL && pTargetState->IsDebuff() && pTargetState->GetInflictor() != INVALID_INDEX)
		{
			fTotalStateCredit += pTargetState->GetAssistCredit();
			fAdjustedStateCredit += (1.0f - fAdjustedStateCredit) * pTargetState->GetAssistCredit();
		}

		if (pAttacker == NULL)
			continue;

		IEntityState *pAttackerState(pAttacker->GetState(i));
		if (pAttackerState != NULL && pAttackerState->IsBuff() && pAttackerState->GetInflictor() != INVALID_INDEX)
		{
			fTotalStateCredit += pAttackerState->GetAssistCredit();
			fAdjustedStateCredit += (1.0f - fAdjustedStateCredit) * pAttackerState->GetAssistCredit();
		}
	}

	// Credit state inflictors
	if (fTotalStateCredit > 0.0f)
	{
		for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
		{
			IEntityState *pTargetState(GetState(i));
			if (pTargetState != NULL && pTargetState->IsDebuff() && pTargetState->GetInflictor() != INVALID_INDEX)
			{
				float fCredit((pTargetState->GetAssistCredit() / fTotalStateCredit) * fAdjustedStateCredit * fDamage);
				RecordDamageCredit(pTargetState->GetInflictor(), fCredit);
			}

			if (pAttacker == NULL)
				continue;

			IEntityState *pAttackerState(pAttacker->GetState(i));
			if (pAttackerState != NULL && pAttackerState->IsBuff() && pAttackerState->GetInflictor() != INVALID_INDEX)
			{
				float fCredit((pAttackerState->GetAssistCredit() / fTotalStateCredit) * fAdjustedStateCredit * fDamage);
				RecordDamageCredit(pAttackerState->GetInflictor(), fCredit);
			}
		}
	}

	// Credit attacker
	if (pAttacker != NULL)
		RecordDamageCredit(pAttacker->GetIndex(), fDamage * (1.0f - fAdjustedStateCredit));

	// Adjust health and check for a kill
	if (fDamage >= m_fHealth)
	{
		m_fHealth = 0.0f;

		if (GetStatus() != ENTITY_STATUS_DEAD && GetStatus() != ENTITY_STATUS_CORPSE)
		{
			if (pAttacker != NULL && IsPlayer())
			{
				CBufferFixed<12> bufferMessage;
				bufferMessage << GAME_CMD_DEATH_MESSAGE << pAttacker->GetIndex() << GetIndex() << unDamagingObjectID;
				Game.BroadcastGameData(bufferMessage, true);
			}

			if (pAttacker != NULL && pAttacker->IsPlayer())
			{
				if (IsPlayer())
				{
					Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_KILLS, 1, GetAsPlayerEnt()->GetClientID(), unDamagingObjectID);
					Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), COMMANDER_MATCH_KILLS, 1, GetAsPlayerEnt()->GetClientID(), unDamagingObjectID);
					
					CBufferFixed<1> buffer;
					buffer << GAME_CMD_KILL_NOTIFICATION;

					Game.SendGameData(pAttacker->GetAsPlayerEnt()->GetClientID(), buffer, true);
				}
				else if (IsBuilding())
				{
					CEntityTeamInfo *pTeam(Game.GetTeam(pAttacker->GetTeam()));

					Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_RAZED, 1, -1, unDamagingObjectID, GetType());

					if (pTeam->GetLastCommanderClientID() != -1)
						Game.MatchStatEvent(pTeam->GetLastCommanderClientID(), COMMANDER_MATCH_RAZED, 1, -1, unDamagingObjectID, GetType());

					CBufferFixed<1> buffer;
					buffer << GAME_CMD_RAZED_NOTIFICATION;

					Game.SendGameData(pAttacker->GetAsPlayerEnt()->GetClientID(), buffer, true);
				}
				else if (IsNpc())
					Game.MatchStatEvent(pAttacker->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_NPC_KILLS, 1, -1, unDamagingObjectID, GetType());
			}

			Kill(pAttacker, unDamagingObjectID);
		}
	}
	else
	{
		m_fHealth -= fDamage;
	}

	return fDamage;
}


/*====================
  IVisualEntity::IsEnemy
  ====================*/
bool	IVisualEntity::IsEnemy(IVisualEntity *pOther) const
{
	if (pOther == NULL)
		return true;

	if (pOther == this)
		return false;

	if (pOther->IsPlayer() && pOther->GetAsPlayerEnt()->GetPetIndex() == GetIndex())
		return false;

	if (IsPlayer() && GetAsPlayerEnt()->GetPetIndex() == pOther->GetIndex())
		return false;

	if (pOther->IsGadget() && IsPlayer() && pOther->GetAsGadget()->GetOwnerClientNumber() == GetAsPlayerEnt()->GetClientID())
		return false;

	if (IsGadget() && pOther->IsPlayer() && GetAsGadget()->GetOwnerClientNumber() == pOther->GetAsPlayerEnt()->GetClientID())
		return false;

	if (pOther->GetTeam() == m_iTeam && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;

	if (Game.GetTeam(GetTeam()) != NULL && Game.GetTeam(GetTeam())->IsAlliedTeam(pOther->GetTeam()) && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;

	if (pOther->GetTeam() == 0 && (GetTeam() == 0 || (pOther->IsPlayer() && pOther->GetAsPlayerEnt()->IsObserver())))
		return false;

	return true;
}


/*====================
  IVisualEntity::LooksLikeEnemy
  ====================*/
bool	IVisualEntity::LooksLikeEnemy(IVisualEntity *pOther) const
{
	if (pOther == NULL)
		return true;

	int iTeam(pOther->GetTeam());
	if (pOther->IsDisguised())
		iTeam = pOther->GetDisguiseTeam();

	if (pOther == this)
		return false;

	if (pOther->IsPlayer() && pOther->GetAsPlayerEnt()->GetPetIndex() == GetIndex())
		return false;

	if (IsPlayer() && GetAsPlayerEnt()->GetPetIndex() == pOther->GetIndex())
		return false;

	if (pOther->IsGadget() && IsPlayer() && pOther->GetAsGadget()->GetOwnerClientNumber() == GetAsPlayerEnt()->GetClientID())
		return false;

	if (IsGadget() && pOther->IsPlayer() && GetAsGadget()->GetOwnerClientNumber() == pOther->GetAsPlayerEnt()->GetClientID())
		return false;

	if ((pOther->GetTeam() == m_iTeam || iTeam == m_iTeam) && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;

	if (Game.GetTeam(iTeam) != NULL && Game.GetTeam(iTeam)->IsAlliedTeam(GetTeam()) && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;

	if (pOther->GetTeam() == 0 && (GetTeam() == 0 || (pOther->IsPlayer() && pOther->GetAsPlayerEnt()->IsObserver())))
		return false;

	return true;
}


/*====================
  IVisualEntity::Copy
  ====================*/
void	IVisualEntity::Copy(const IGameEntity &B)
{
	IGameEntity::Copy(B);

	const IVisualEntity *pB(B.GetAsVisualEnt());

	if (!pB)	
		return;

	const IVisualEntity &C(*pB);

	m_bValid =			C.m_bValid;
	m_sName =			C.m_sName;
	m_uiIndex =			C.m_uiIndex;
	m_uiWorldIndex =	C.m_uiWorldIndex;

	SetTeam(C.m_iTeam);

	m_ySquad =			C.m_ySquad;
	m_yStatus =		C.m_yStatus;
	m_uiCreationTime =	C.m_uiCreationTime;

	m_uiLocalFlags =	C.m_uiLocalFlags;

	m_fHealth =		C.m_fHealth;
	m_bInvulnerable = C.m_bInvulnerable;

	m_v3Position =	C.m_v3Position;
	m_v3Velocity =	C.m_v3Velocity;
	m_v3Angles =	C.m_v3Angles;
	m_fScale =		C.m_fScale;
	m_bbBounds =	C.m_bbBounds;

	m_hModel =			C.m_hModel;
	m_pSkeleton =		C.m_pSkeleton;

	m_yNextEventSlot =		C.m_yNextEventSlot;
	m_yLastProcessedEvent =	C.m_yLastProcessedEvent;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
		m_apState[i] = C.m_apState[i];

	m_yDefaultAnim = C.m_yDefaultAnim;
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		m_ayAnim[i] = C.m_ayAnim[i];
		m_ayAnimSequence[i] = C.m_ayAnimSequence[i];
		m_afAnimSpeed[i] = C.m_afAnimSpeed[i];
		m_auiAnimLockTime[i] = C.m_auiAnimLockTime[i];
	}

	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		m_ahEffect[i] = C.m_ahEffect[i];
		m_ayEffectSequence[i] = C.m_ayEffectSequence[i];
	}
	
	for (int i(0); i < MAX_INVENTORY; ++i)
		m_apInventory[i] = C.m_apInventory[i];

	m_hMinimapIcon = C.m_hMinimapIcon;
	m_hLargeMapIcon = C.m_hLargeMapIcon;
}


/*====================
  IVisualEntity::Slide

  Applies the velocity.
  Returns whether or not we touched anything
  ====================*/
bool	IVisualEntity::Slide(bool bGravity)
{
	STraceInfo trace;

	if (bGravity)
		s_Move.v3Velocity.z -= p_gravity * s_Move.fFrameTime;

	if (s_Move.iMoveFlags & MOVE_ON_GROUND)
		s_Move.v3Velocity.Clip(s_Move.plGround.v3Normal);
	
	if (s_Move.v3Velocity == V3_ZERO)
		return false;

	if (p_debugSlide && Game.IsServer())
		Console << _T("Slide: ") << bGravity << _T(" ");

	float fTimeLeft(s_Move.fFrameTime);

	int iMoves(0);
	int iZeroCount(0);
	CPlane plZeroPlanes[3];
	bool bDoubleHit[3] = { false, false, false };

	while (fTimeLeft > 0.0001f && iZeroCount < 3 && iMoves < MAX_SLIDE_MOVES && s_Move.v3Velocity != V3_ZERO)
	{
		CVec3f v3OldPosition(s_Move.v3Position);
		CVec3f v3NewPosition(M_PointOnLine(s_Move.v3Position, s_Move.v3Velocity, fTimeLeft * SLIDE_OVERTRACE));

		Game.TraceBox(trace, v3OldPosition, v3NewPosition, m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);

		float fFraction(trace.fFraction * SLIDE_OVERTRACE);

		CVec3f v3EndPos;

		if (fFraction > 1.0f)
		{
			v3EndPos = LERP(1.0f / fFraction, v3OldPosition, trace.v3EndPos);
			fFraction = 1.0f;
		}
		else
			v3EndPos = trace.v3EndPos;

		float fZeroFraction(CONTACT_EPSILON / Length(v3NewPosition - v3OldPosition));

		if (!IsPositionValid(v3EndPos))
		{
			if (p_debugSlide && Game.IsServer())
				Console << _T("STUCK ");
		}
		else
			s_Move.v3Position = v3EndPos;
  
		if (fFraction < 1.0f)
		{
			if (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope)
			{
				if (-DotProduct(trace.plPlane.v3Normal, s_Move.v3Velocity) > p_landVelocity)
					s_Move.bLanded = true;
			}

			if (!trace.plPlane.v3Normal.IsValid())
				break;

			if (fFraction > fZeroFraction)
				iZeroCount = 0;

			// Check if we already hit this plane once
			bool bBreak(false);
			bool bContinue(false);

			for (int i(0); iMoves == iZeroCount ? i < iZeroCount : i <= iZeroCount; ++i)
			{
				if (trace.plPlane == plZeroPlanes[i])
				{
					if (bDoubleHit[i]) // Hit this plane twice, so just give up
					{
						if (p_debugSlide && Game.IsServer())
							Console << _T("Triple Hit ");

						s_Move.v3Velocity.Clear();
						bBreak = true;
						break;
					}

					if (p_debugSlide && Game.IsServer())
						Console << _T("Double Hit ");

					// Nudge our velocity away from plane and try again
					bDoubleHit[i] = true;
					s_Move.v3Velocity += trace.plPlane.v3Normal;
					bContinue = true;
					break;
				}
			}

			if (bContinue)
				continue;
			else if (bBreak)
				break;

			bool bNoPrev;

			if (iMoves == iZeroCount) // We don't have any previous successful moves
			{
				bNoPrev = true;
				plZeroPlanes[iZeroCount] = trace.plPlane;
			}
			else
			{
				bNoPrev = false;

				if (fFraction <= fZeroFraction)
					++iZeroCount;

				plZeroPlanes[iZeroCount] = trace.plPlane;
			}

			switch (iZeroCount)
			{
			case 0: // Single plane intersection, slide along plane
				{
					s_Move.v3Velocity.Clip(plZeroPlanes[0].v3Normal);
				}
				break;
			case 1: // Double plane intersection, slide along line
				{
					if (p_debugSlide && Game.IsServer())
						Console << _T("Double Zero ");

					CVec3f v3Line(Normalize(CrossProduct(plZeroPlanes[0].v3Normal, plZeroPlanes[1].v3Normal)));

					if (fabs(1.0f - v3Line.Length()) > 0.1f) // Check for malformed normal
						s_Move.v3Velocity.Clear();
					else
						s_Move.v3Velocity.Project(v3Line);
				}
				break;
			case 2: // Triple plane intersection, clear velocity
				{
					if (p_debugSlide && Game.IsServer())
						Console << _T("Triple Zero ");
					
					s_Move.v3Velocity.Clear();
				}
				break;
			}

			if (bNoPrev && fFraction <= fZeroFraction)
				++iZeroCount;
			
			if (!s_Move.v3Velocity.IsValid())
				s_Move.v3Velocity.Clear();
			
			if (s_Move.v3Velocity.Length() < 0.001f)
				s_Move.v3Velocity.Clear();
		}

		if (p_debugSlide && Game.IsServer())
			Console << (fTimeLeft * fFraction) / s_Move.fFrameTime << _T(" ");
	
		fTimeLeft -= fTimeLeft * fFraction;
		++iMoves;
	}

	if (p_debugSlide && Game.IsServer())
		Console << newl;

	if (iMoves == MAX_SLIDE_MOVES)
		Console.Warn << _T("IPlayerEntity::Slide Moves == MAX_SLIDE_MOVES") << newl;

	return iMoves != 1;
}


/*====================
  IVisualEntity::StepSlide
  ====================*/
bool	IVisualEntity::StepSlide(bool bGravity)
{
	CVec3f v3StartPos(s_Move.v3Position);
	CVec3f v3StartVel(s_Move.v3Velocity);

	if (!Slide(bGravity))
	{
		// we completed the move without hitting anything

		// Try to lower if we have ground control
		if (s_Move.bGroundControl && p_stepDown && (bGravity || v3StartVel != V3_ZERO))
		{
			CVec3f v3OldPos(s_Move.v3Position);
			CVec3f v3OldVel(s_Move.v3Velocity);

			STraceInfo trace;

			// Lower ourselves down slope changes
			if (Game.TraceBox(trace, s_Move.v3Position, s_Move.v3Position - CVec3f(0.0f, 0.0f, MAX<float>(p_stepHeight, v3StartVel.Length() * s_Move.fFrameTime / (1.0f - p_maxWalkSlope))), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex))
			{
				if (trace.plPlane.v3Normal.z >= 1.0f - p_maxWalkSlope)
				{					
					if (s_Move.v3Velocity.x != 0.0f && s_Move.v3Velocity.y != 0.0f && s_Move.v3Velocity.z != 0.0f)
					{
						// Project velocity onto the ground plane top-down
						const CVec3f &v3Normal(trace.plPlane.v3Normal);
						float fSpeed(s_Move.v3Velocity.Length());
						
						// Ax + By + Cz = 0 --> Cz = -Ax - By --> z = (-Ax - By)/C
						CVec3f v3NewVelocity(s_Move.v3Velocity);

						v3NewVelocity.z = (-v3Normal.x * v3NewVelocity.x - v3Normal.y * v3NewVelocity.y) / v3Normal.z;
						v3NewVelocity.SetLength(fSpeed);

						// Never turn more than 90 degrees
						if (DotProduct(s_Move.v3Velocity, v3NewVelocity) > 0.0f)
						{
							s_Move.v3Position = trace.v3EndPos;
							s_Move.v3Velocity = v3NewVelocity;
						}
					}
				}
			}
		}
		
		return false; 
	}

	// Don't try stepping if we don't have ground control
	if (!(s_Move.bGroundControl))
		return true;
	
	CVec3f v3OldPos(s_Move.v3Position);
	CVec3f v3OldVel(s_Move.v3Velocity);
	
	// Raise ourselves up p_stepHeight and try again
	STraceInfo trace;

	Game.TraceBox(trace, v3StartPos, v3StartPos + CVec3f(0.0f, 0.0f, p_stepHeight), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);

	if (trace.bStartedInSurface)
		return false;

	float fStepHeight(trace.v3EndPos.z - s_Move.v3Position.z);

	s_Move.v3Position = trace.v3EndPos;
	s_Move.v3Velocity = v3StartVel;

	// Reslide from this new position
	Slide(bGravity);

	// Lower ourselves back down
	Game.TraceBox(trace, s_Move.v3Position, s_Move.v3Position - CVec3f(0.0f, 0.0f, fStepHeight + p_groundEpsilon), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);

	// Undo the step if we got stuck
	if (trace.bStartedInSurface)
	{
		if (p_debugStep && Game.IsServer())
			Console << _T("StepSlide STUCK") << newl;

		s_Move.v3Position = v3OldPos;
		s_Move.v3Velocity = v3OldVel;
		return false;
	}

	s_Move.v3Position = trace.v3EndPos;

	// Reclip our velocity if we hit the ground again
	if (trace.fFraction < 1.0f && trace.plPlane.v3Normal.z >= 1.0f - p_maxWalkSlope)
	{
		s_Move.v3Velocity.Clip(trace.plPlane.v3Normal);
	}
	else
	{
		s_Move.v3Position = v3OldPos;
		s_Move.v3Velocity = v3OldVel;

		// Lower ourselves down slope changes
		if (p_stepDown && Game.TraceBox(trace, s_Move.v3Position, s_Move.v3Position - CVec3f(0.0f, 0.0f, MAX<float>(p_stepHeight, v3StartVel.Length() * s_Move.fFrameTime / (1.0f - p_maxWalkSlope))), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex))
		{
			if (trace.plPlane.v3Normal.z >= 1.0f - p_maxWalkSlope && DotProduct(s_Move.v3Velocity, trace.plPlane.v3Normal) > 0.0f)
			{
				if (s_Move.v3Velocity.x != 0.0f && s_Move.v3Velocity.y != 0.0f && s_Move.v3Velocity.z != 0.0f)
				{
					// Project velocity onto the ground plane top-down
					const CVec3f &v3Normal(trace.plPlane.v3Normal);
					float fSpeed(s_Move.v3Velocity.Length());
					
					// Ax + By + Cz = 0 --> Cz = -Ax - By --> z = (-Ax - By)/C
					CVec3f v3NewVelocity(s_Move.v3Velocity);

					v3NewVelocity.z = (-v3Normal.x * v3NewVelocity.x - v3Normal.y * v3NewVelocity.y) / v3Normal.z;
					v3NewVelocity.SetLength(fSpeed);

					// Never turn more than 90 degrees
					if (DotProduct(s_Move.v3Velocity, v3NewVelocity) > 0.0f)
					{
						s_Move.v3Position = trace.v3EndPos;
						s_Move.v3Velocity = v3NewVelocity;
					}
				}
			}
		}
	}

	return true;
}


/*====================
  IVisualEntity::IsPositionValid

  Determines whether or not a position is valid
  ====================*/
bool	IVisualEntity::IsPositionValid(const CVec3f &v3Position)
{
	STraceInfo trace;

	Game.TraceBox(trace, v3Position, v3Position + CVec3f(0.0, 0.0, CONTACT_EPSILON * 1.25f), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);

	return !trace.bStartedInSurface;
}


/*====================
  IVisualEntity::CheckGround

  Determines whether or not we are sitting on the ground.
  If we are it sets the ground plane data up.
  ====================*/
void	IVisualEntity::CheckGround()
{
	STraceInfo trace;

	// Trace down a certain number of units to determine whats beneath us
	Game.TraceBox(trace, s_Move.v3Position, s_Move.v3Position - CVec3f(0.0, 0.0, p_groundEpsilon), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);

	// We'll be in air if the trace didn't hit anything or we're
	// traveling away from the ground plane at more than 10 units/sec

	if (trace.fFraction == 1.0f || DotProduct(trace.plPlane.v3Normal, s_Move.v3Velocity) > 10.0f)
	{
		//
		// In Air
		//

		s_Move.bGroundControl = false;

		s_Move.iMoveFlags &= ~MOVE_ON_GROUND;
		s_Move.plGround = CPlane(0.0f, 0.0f, 1.0f, 0.0f);
	}
	else
	{
		if (trace.bStartedInSurface)
			return;

		//
		// On Ground
		//

		if (trace.plPlane.v3Normal.z < 1.0f - p_maxWalkSlope)
			s_Move.bGroundControl = false;
		else
			s_Move.bGroundControl = true;

		s_Move.iMoveFlags |= MOVE_ON_GROUND;
		s_Move.iMoveFlags &= ~MOVE_JUMPING;
		s_Move.plGround = trace.plPlane;

		// Re-snap us to the ground
		s_Move.v3Position = trace.v3EndPos;

		m_uiGroundEntityIndex = trace.uiEntityIndex;
	}

	m_bOnGround = s_Move.bGroundControl;
}


/*====================
  IVisualEntity::ClientPrecache
  ====================*/
void	IVisualEntity::ClientPrecache(CEntityConfig *pConfig)
{
	g_ResourceManager.Register(_T("/shared/effects/materials/selection_indicator.material"), RES_MATERIAL);

	IGameEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetModelPath().empty())
	{
		ResHandle hModel(g_ResourceManager.Register(pConfig->GetModelPath(), RES_MODEL));

		if (hModel != INVALID_RESOURCE)
			g_ResourceManager.PrecacheSkin(hModel, -1);
	}

	if (!pConfig->GetHitByMeleeEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetHitByMeleeEffectPath(), RES_EFFECT);

	if (!pConfig->GetHitByRangedEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetHitByRangedEffectPath(), RES_EFFECT);

	if (!pConfig->GetIconPath().empty())
		g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

	if (!pConfig->GetMinimapIconPath().empty())
		g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetMinimapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
	if (!pConfig->GetLargeMapIconPath().empty())
		g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetLargeMapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

	if (!pConfig->GetCommanderPortraitPath().empty())
		g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetCommanderPortraitPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
}


/*====================
  IVisualEntity::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IVisualEntity::ServerPrecache(CEntityConfig *pConfig)
{
	IGameEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetModelPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetModelPath(), RES_MODEL, RES_MODEL_SERVER));

	if (!pConfig->GetHitByMeleeEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetHitByMeleeEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetHitByRangedEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetHitByRangedEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IVisualEntity::GetStateExpirePercent
  ====================*/
float	IVisualEntity::GetStateExpirePercent(int iSlot)
{
	if (iSlot < 0 || iSlot >= MAX_ACTIVE_ENTITY_STATES)
		return 0.0f;

	if (m_apState[iSlot]->GetStartTime() == INVALID_TIME || m_apState[iSlot]->GetDuration() == INVALID_TIME)
		return 0.0f;

	return float(Game.GetGameTime() - m_apState[iSlot]->GetStartTime()) / float(m_apState[iSlot]->GetDuration());
}


/*====================
  IVisualEntity::IsStealthed
  ====================*/
bool	IVisualEntity::IsStealthed()
{
	if (HasNetFlags(ENT_NET_FLAG_REVEALED))
		return false;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->GetIsStealth())
			return true;
	}

	return false;
}


/*====================
  IVisualEntity::IsDisguised
  ====================*/
bool	IVisualEntity::IsDisguised() const
{
	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->IsDisguised())
			return true;
	}

	return false;
}


/*====================
  IVisualEntity::GetDisguiseTeam
  ====================*/
int	IVisualEntity::GetDisguiseTeam() const
{
	if (HasNetFlags(ENT_NET_FLAG_REVEALED))
		return GetTeam();

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->IsDisguised())
			return m_apState[i]->GetDisguiseTeam();
	}

	return GetTeam();
}


/*====================
  IVisualEntity::GetDisguiseClient
  ====================*/
int	IVisualEntity::GetDisguiseClient() const
{
	if (!IsPlayer())
		return -1;

	if (HasNetFlags(ENT_NET_FLAG_REVEALED))
		return GetAsPlayerEnt()->GetClientID();

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->IsDisguised())
			return m_apState[i]->GetDisguiseClient();
	}

	return GetAsPlayerEnt()->GetClientID();
}


/*====================
  IVisualEntity::GetDisguiseItem
  ====================*/
ushort	IVisualEntity::GetDisguiseItem() const
{
	if (!IsPlayer())
		return -1;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->IsDisguised())
			return m_apState[i]->GetDisguiseItem();
	}

	return INVALID_ENT_TYPE;
}


/*====================
  IVisualEntity::IsIntangible
  ====================*/
bool	IVisualEntity::IsIntangible() const
{
	if (m_apState == NULL)
		return false;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->GetIsIntangible())
			return true;
	}

	return false;
}


/*====================
  IVisualEntity::IsSilenced
  ====================*/
bool	IVisualEntity::IsSilenced() const
{
	if (m_apState == NULL)
		return false;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		if (m_apState[i] == NULL)
			continue;

		if (m_apState[i]->GetDisableSkills())
			return true;
	}

	return false;
}


/*====================
  IVisualEntity::CanTakeDamage
  ====================*/
bool	IVisualEntity::CanTakeDamage(int iFlags, IVisualEntity *pAttacker)
{
	IVisualEntity *pVisual(pAttacker->GetAsVisualEnt());

	if (pVisual == NULL)
		return false;
	
	if (!g_TeamDamage && pVisual->GetTeam() == GetTeam() && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;

	if (Game.GetTeam(pVisual->GetTeam()) != NULL && Game.GetTeam(pVisual->GetTeam())->IsAlliedTeam(GetTeam()) && Game.GetGamePhase() != GAME_PHASE_WARMUP)
		return false;
	
	if	(m_fHealth > 0.0f &&
		!IsInvulnerable() &&
		m_yStatus != ENTITY_STATUS_DORMANT &&
		m_yStatus != ENTITY_STATUS_CORPSE)
		return true;

	return false;
}


/*====================
  IVisualEntity::CanSee
  ====================*/
bool	IVisualEntity::CanSee(IVisualEntity *pOther)
{
	if (pOther == NULL)
		return false;

	if (DistanceSq(GetPosition().xy(), pOther->GetPosition().xy()) <= SQR(GetSightRange()))
		return true;

	return false;
}


/*====================
  IVisualEntity::CanCarryItem
  ====================*/
bool	IVisualEntity::CanCarryItem(ushort unItem)
{
	if (IsPlayer() && !GetAsPlayerEnt()->GetCanPurchase())
		return false;

	ICvar *pUniqueCategory(EntityRegistry.GetGameSetting(unItem, _T("UniqueCategory")));
	tstring sUniqueCategory(pUniqueCategory ? pUniqueCategory->GetString() : _T(""));
	ICvar *pMaxStacks(EntityRegistry.GetGameSetting(unItem, _T("MaxStacks")));
	uint uiNumStacks(0);

	for (int i = INVENTORY_START_BACKPACK; i < INVENTORY_END_BACKPACK; i++)
	{
		if (!m_apInventory[i])
			continue;

		if (!m_apInventory[i]->IsConsumable())
			continue;
		
		if (!sUniqueCategory.empty() &&
			CompareNoCase(m_apInventory[i]->GetAsConsumable()->GetUniqueCategory(), sUniqueCategory) == 0)
		{
			// Player already has an item of this category
			return false;
		}

		if (m_apInventory[i]->GetType() == unItem)
		{
			if (m_apInventory[i]->GetAmmo() >= int(m_apInventory[i]->GetAsConsumable()->GetMaxPerStack()))
				++uiNumStacks;
		}
	}

	if (pMaxStacks && uiNumStacks >= pMaxStacks->GetUnsignedInteger())
	{
		// MaxStacks exceeded
		return false;
	}

	// Check prerequisites
	ICvar *pPrerequisite(EntityRegistry.GetGameSetting(unItem, _T("Prerequisite")));
	if (m_iTeam > 0 && pPrerequisite && !pPrerequisite->GetString().empty())
	{
		CEntityTeamInfo *pTeamInfo(Game.GetTeam(int(m_iTeam)));
		if (pTeamInfo)
		{
			if (!pTeamInfo->HasBuilding(pPrerequisite->GetString()))
			{
				return false;
			}
		}
	}

	return true;
}


/*====================
  IVisualEntity::GiveItem
  ====================*/
int		IVisualEntity::GiveItem(int iSlot, ushort unID, bool bEnabled)
{
	try
	{
		if (Game.IsClient())
			return -1;

		if (unID != INVALID_ENT_TYPE)
		{
			ICvar *pMaxStacks(EntityRegistry.GetGameSetting(unID, _T("MaxStacks")));

			if (iSlot == INVENTORY_AUTO_BACKPACK)
			{
				uint uiNumStacks(0);
				for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK; ++i)
				{
					if (m_apInventory[i] == NULL && iSlot == INVENTORY_AUTO_BACKPACK)
					{
						iSlot = i;
					}
					else if (m_apInventory[i] != NULL && m_apInventory[i]->GetType() == unID && m_apInventory[i]->IsConsumable())
					{
						if (m_apInventory[i]->GetAmmo() < int(m_apInventory[i]->GetAsConsumable()->GetMaxPerStack()))
						{
							m_apInventory[i]->AdjustAmmo(m_apInventory[i]->GetAmmoCount());
							iSlot = i;
							return i;
						}
						else
						{
							++uiNumStacks;
						}
					}
				}

				if (iSlot == INVENTORY_AUTO_BACKPACK)
				{
					//EX_ERROR(_T("No free backpack slots");
					return -1;
				}
				
				if (pMaxStacks && uiNumStacks >= pMaxStacks->GetUnsignedInteger())
				{
					//EX_ERROR(_T("MaxStacks exceeded");
					return -1;
				}
			}
		}

		if (iSlot < 0 || iSlot >= MAX_INVENTORY)
			EX_ERROR(_T("Invalid inventory slot: ") + XtoA(iSlot));

		if (unID == INVALID_ENT_TYPE)
		{
			RemoveItem(iSlot);
			return -1;
		}

		// Check prerequisites
		bool bDisableItem(!bEnabled);
		ICvar *pCvar(EntityRegistry.GetGameSetting(unID, _T("Prerequisite")));
		if (m_iTeam > 0 && pCvar && !pCvar->GetString().empty())
		{
			CEntityTeamInfo *pTeamInfo(Game.GetTeam(int(m_iTeam)));
			if (pTeamInfo == NULL)
				EX_WARN(_T("Entity's team is invalid: ") + XtoA(int(m_iTeam)));

			if (!pTeamInfo->HasBuilding(pCvar->GetString()))
				bDisableItem = true;
		}

		if (m_apInventory[iSlot] != NULL && m_apInventory[iSlot]->GetType() == unID)
		{
			if (m_apInventory[iSlot]->IsDisabled() && !bDisableItem && GetStatus() == ENTITY_STATUS_DORMANT)
				m_apInventory[iSlot]->Enable();

			return iSlot;
		}

		RemoveItem(iSlot);
		m_apInventory[iSlot] = static_cast<IInventoryItem *>(Game.AllocateEntity(unID, m_uiIndex));
		if (m_apInventory[iSlot] == NULL)
			EX_ERROR(_T("Item allocation failed for: ") + EntityRegistry.LookupName(unID));

		m_apInventory[iSlot]->SetOwner(m_uiIndex);
		m_apInventory[iSlot]->SetSlot(byte(iSlot));

		if (bDisableItem)
		{
			m_apInventory[iSlot]->Disable();
			m_apInventory[iSlot]->SetAmmo(0);
		}
		else
		{
			m_apInventory[iSlot]->Enable();
			m_apInventory[iSlot]->SetAmmo(m_apInventory[iSlot]->GetAdjustedAmmoCount());
		}

		m_apInventory[iSlot]->ActivatePassive();
		
		return iSlot;
	}
	catch (CException &ex)
	{
		ex.Process(_T("ICombatEntity::GiveItem() - "), NO_THROW);
		return -1;
	}
}


/*====================
  IVisualEntity::ClearInventory
  ====================*/
void	IVisualEntity::ClearInventory()
{
	for (int i(0); i < MAX_INVENTORY; ++i)
		RemoveItem(i);
}


/*====================
  IVisualEntity::RemoveItem
  ====================*/
void	IVisualEntity::RemoveItem(int iSlot)
{
	if (iSlot < 0 || iSlot > MAX_INVENTORY)
		EX_ERROR(_T("Invalid slot specified"));

	if (m_apInventory[iSlot] == NULL)
		return;

	Game.DeleteEntity(m_apInventory[iSlot]);
	m_apInventory[iSlot] = NULL;
}


/*====================
  IVisualEntity::SetInventorySlot
  ====================*/
void	IVisualEntity::SetInventorySlot(int iSlot, IInventoryItem *pItem)
{
	if (iSlot < 0 || iSlot >= MAX_INVENTORY)
		return;

	m_apInventory[iSlot] = pItem;
}


/*====================
  IVisualEntity::SwapItem
  ====================*/
void	IVisualEntity::SwapItem(int iSlot1, int iSlot2)
{
	if (iSlot1 == iSlot2)
		return;

	if (iSlot1 < 1 || iSlot1 >= INVENTORY_END_BACKPACK)
		return;

	if (iSlot2 < 1 || iSlot2 >= INVENTORY_END_BACKPACK)
		return;

	if ((iSlot1 < INVENTORY_START_BACKPACK && iSlot2 >= INVENTORY_START_BACKPACK) ||
		(iSlot2 < INVENTORY_START_BACKPACK && iSlot1 >= INVENTORY_START_BACKPACK))
		return;

	SWAP(m_apInventory[iSlot1], m_apInventory[iSlot2]);

	if (m_apInventory[iSlot1])
		m_apInventory[iSlot1]->SetSlot(iSlot1);
	if (m_apInventory[iSlot2])
		m_apInventory[iSlot2]->SetSlot(iSlot2);
}


/*====================
  IVisualEntity::CanSpawnFrom
  ====================*/
bool	IVisualEntity::CanSpawnFrom(IPlayerEntity *pPlayer)
{
	if (pPlayer == NULL)
		return false;

	if (!IsSpawnLocation() && (GetAsProp() == NULL || GetAsProp()->GetAsFoundation() == NULL || Game.GetGamePhase() != GAME_PHASE_WARMUP))
		return false;

	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return true;

	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return false;

	if (GetTeam() != pPlayer->GetTeam())
		return false;

	if (GetSquad() != INVALID_SQUAD && GetSquad() != pPlayer->GetSquad())
		return false;

	if (pPlayer->GetIsSiege() && !CanSpawnSiege())
		return false;

	return true;
}


/*====================
  IVisualEntity::Interpolate
  ====================*/
void	IVisualEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
	m_v3Angles = M_LerpAngles(fLerp, pPrevState->GetAngles(), pNextState->GetAngles());
	m_v3Position = LERP(fLerp, pPrevState->GetPosition(), pNextState->GetPosition());
	m_fScale = LERP(fLerp, pPrevState->GetScale(), pNextState->GetScale());
}


/*====================
  IVisualEntity::UpdateEffectThread
  ====================*/
void	IVisualEntity::UpdateEffectThread(CEffectThread *pEffectThread)
{
	pEffectThread->SetSourceModel(g_ResourceManager.GetModel(GetModelHandle()));
	pEffectThread->SetSourceSkeleton(m_pSkeleton);

	pEffectThread->SetSourcePos(m_v3Position);
	pEffectThread->SetSourceAxis(m_aAxis);

	if (pEffectThread->GetUseEntityEffectScale())
		pEffectThread->SetSourceScale(m_fScale * GetScale2() * GetEffectScale());
	else
		pEffectThread->SetSourceScale(m_fScale * GetScale2());
}


/*====================
  IVisualEntity::GetTerrainType
  ====================*/
const tstring&	IVisualEntity::GetTerrainType()
{
	if (m_uiLastTerrainTypeUpdateTime == Game.GetGameTime())
		return m_sTerrainType;

	m_uiLastTerrainTypeUpdateTime = Game.GetGameTime();

	STraceInfo result;
	CVec3f v3Start(GetPosition() + GetBounds().GetMid());
	CVec3f v3End(M_PointOnLine(v3Start, CVec3f(0.0f, 0.0f, -1.0f), FAR_AWAY));
	Game.TraceLine(result, v3Start, v3End, TRACE_TERRAIN & ~SURF_HULL, m_uiWorldIndex);
	if (result.uiEntityIndex != INVALID_INDEX)
	{
		IGameEntity *pEnt(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
		if (pEnt->IsProp())
		{
			m_sTerrainType = pEnt->GetAsProp()->GetSurfaceType();
			if (!m_sTerrainType.empty())
				return m_sTerrainType;
		}
	}
	
	m_sTerrainType = Game.GetWorldPointer()->GetTerrainType(m_v3Position.x, m_v3Position.y);
	return m_sTerrainType;
}
