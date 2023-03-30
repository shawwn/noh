// (C)2007 S2 Games
// i_visualentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_visualentity.h"

#include "c_entityregistry.h"
#include "i_propentity.h"
#include "c_teaminfo.h"
#include "c_player.h"
#include "i_gadgetentity.h"
#include "i_entitystate.h"

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
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(IVisualEntity, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IGameEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_DELTAPOS3D, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[PITCH]"), TYPE_ANGLE8, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[ROLL]"), TYPE_ANGLE8, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Angles[YAW]"), TYPE_ANGLE8, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yStatus"), TYPE_CHAR, 5, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySequence"), TYPE_CHAR, 4, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unVisibilityFlags"), TYPE_SHORT, 16, 0));
	
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnim[") + XtoA(i) + _T("]"), TYPE_CHAR, 5, -1));
		s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnimSequence[") + XtoA(i) + _T("]"), TYPE_CHAR, 2, 0));
		s_cDesc.pFieldTypes->push_back(SDataField(_T("m_afAnimSpeed[") + XtoA(i) + _T("]"), TYPE_FLOAT, 32, 0));
	}

	// Effects
	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ahEffect[") + XtoA(i) + _T("]"), TYPE_RESHANDLE, 0, 0));
		s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayEffectSequence[") + XtoA(i) + _T("]"), TYPE_CHAR, 2, 0));
	}

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 3, TEAM_INVALID));
}
//=============================================================================

/*====================
  IVisualEntity::IVisualEntity
  ====================*/
IVisualEntity::IVisualEntity() :
IGameEntity(NULL),

m_uiWorldIndex(INVALID_INDEX),
m_uiTeamID(TEAM_PASSIVE),

m_yStatus(ENTITY_STATUS_DORMANT),
m_ySequence(0),
m_unVisibilityFlags(0),

m_v3Position(V_ZERO),
m_v3Velocity(V_ZERO),
m_v3Angles(V_ZERO),
m_fScale(1.0f),
m_bbBounds(V3_ZERO, V3_ZERO),
m_uiGroundEntityIndex(INVALID_INDEX),
m_uiBindTargetUID(INVALID_INDEX),

m_yNextEventSlot(0),
m_yLastProcessedEvent(0),

m_uiLocalFlags(0),

m_pSkeleton(NULL),

m_uiClientRenderFlags(0),
m_v4HighlightColor(WHITE),
m_aAxis(0.0f, 0.0f, 0.0f),
m_v3AxisAngles(V3_ZERO),
m_uiLastTerrainTypeUpdateTime(INVALID_TIME),
m_uiSelectFrame(uint(-1)),

m_uiMinimapFlashEndTime(0),
m_uiOrderTime(INVALID_TIME),

m_bAlwaysTransmitData(false)
{
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
	m_ySequence = 0;
	m_unVisibilityFlags = 0;

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

	m_uiTeamID = TEAM_PASSIVE;
}


/*====================
  IVisualEntity::GetSnapshot
  ====================*/
void	IVisualEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IGameEntity::GetSnapshot(snapshot, uiFlags);

	// Basic data
	snapshot.WriteDeltaPos3D(m_v3Position);
	snapshot.WriteAngle8(m_v3Angles.x);
	snapshot.WriteAngle8(m_v3Angles.y);
	snapshot.WriteAngle8(m_v3Angles.z);

	snapshot.WriteField(m_yStatus);
	snapshot.WriteField(m_ySequence);
	snapshot.WriteField(m_unVisibilityFlags);

	// Anims
	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		snapshot.WriteInteger(m_ayAnim[i]);
		snapshot.WriteField(m_ayAnimSequence[i]);
		snapshot.WriteField(m_afAnimSpeed[i]);
	}

	// Effects
	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		snapshot.WriteResHandle(m_ahEffect[i]);
		snapshot.WriteField(m_ayEffectSequence[i]);
	}

	snapshot.WriteInteger(m_uiTeamID);
}


/*====================
  IVisualEntity::ReadSnapshot
  ====================*/
bool	IVisualEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		// Base entity info
		if (!IGameEntity::ReadSnapshot(snapshot, 1))
			return false;
		
		snapshot.ReadDeltaPos3D(m_v3Position);
		snapshot.ReadAngle8(m_v3Angles.x);
		snapshot.ReadAngle8(m_v3Angles.y);
		snapshot.ReadAngle8(m_v3Angles.z);
		snapshot.ReadField(m_yStatus);
		snapshot.ReadField(m_ySequence);
		snapshot.ReadField(m_unVisibilityFlags);

		// Anims
		for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
		{
			snapshot.ReadInteger(m_ayAnim[i]);
			snapshot.ReadField(m_ayAnimSequence[i]);
			snapshot.ReadField(m_afAnimSpeed[i]);
		}

		// Effects
		for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
		{
			snapshot.ReadResHandle(m_ahEffect[i]);
			snapshot.ReadField(m_ayEffectSequence[i]);
		}

		snapshot.ReadInteger(m_uiTeamID);

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
  IVisualEntity::MinimapFlash
  ====================*/
void	IVisualEntity::MinimapFlash(const CVec4f &v4Color, uint uiDuration)
{
	m_v4MinimapFlashColor = v4Color;
	m_uiMinimapFlashEndTime = Game.GetGameTime() + uiDuration;
}


/*====================
  IVisualEntity::DrawOnMap
  ====================*/
void	IVisualEntity::DrawOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const
{
	if (!IsVisibleOnMap(pLocalPlayer))
		return;
	if (GetMapIcon(pLocalPlayer) == INVALID_RESOURCE)
		return;

	CBufferFixed<36> buffer;
	
	buffer << GetPosition().x / Game.GetWorldWidth();
	buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
	
	buffer << GetMapIconSize(pLocalPlayer) << GetMapIconSize(pLocalPlayer);
	
	CVec4f v4Color(GetMapIconColor(pLocalPlayer));
	buffer << v4Color[R];
	buffer << v4Color[G];
	buffer << v4Color[B];
	buffer << v4Color[A];

	buffer << GetMapIcon(pLocalPlayer);

	minimap.Execute(_T("icon"), buffer);
}


/*====================
  IVisualEntity::ApplyWorldEntity
  ====================*/
void	IVisualEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
	m_sName = ent.GetName();
	m_uiWorldIndex = ent.GetIndex();
	m_v3Position = ent.GetPosition();
	m_v3Angles = ent.GetAngles();
	m_fScale = ent.GetScale();
	SetTeam(ent.GetTeam());
}


/*====================
  IVisualEntity::UpdateSkeleton
  ====================*/
void	IVisualEntity::UpdateSkeleton(bool bPose)
{
	if (m_pSkeleton == NULL)
		return;

	m_pSkeleton->SetModel(GetModel());

	if (GetModel() == INVALID_RESOURCE)
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
		FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModel())));

		const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

		for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
			EventScript.Execute(it->sCmd, it->iTimeNudge);

		m_pSkeleton->ClearEvents();

		FileManager.SetWorkingDirectory(sOldDir);
	}
}


/*====================
  IVisualEntity::StartRandomAnimation
  ====================*/
void	IVisualEntity::StartRandomAnimation(const tstring &sAnimName, int iNumAnims, int iChannel, float fSpeed, uint uiLength)
{
	size_t zPos(sAnimName.find(_T('%')));
	if (zPos != tstring::npos)
	{
		const tstring &sFirstPart(sAnimName.substr(0, zPos));
		const tstring &sLastPart(sAnimName.substr(zPos + 1));

		tstring sRandomAnimName(sFirstPart + XtoA(M_Randnum(1, iNumAnims)) + sLastPart);

		StartAnimation(sRandomAnimName, iChannel, fSpeed, uiLength);
	}
	else
	{
		StartAnimation(sAnimName, iChannel, fSpeed, uiLength);
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

	m_asAnim[iChannel] = sAnimName;
	m_ayAnim[iChannel] = g_ResourceManager.GetAnim(GetModel(), sAnimName);

	// Why is this #ifdef'd out?  -- Shawn
#if 0
	if (m_ayAnim[iChannel] == byte(-1))
		Console.Warn << _T("IVisualEntity::StartAnimation() - Animation not found: ") << sAnimName << newl;
#endif

	float fLength(uiLength != 0 ? float(g_ResourceManager.GetAnimLength(GetModel(), m_ayAnim[iChannel])) / uiLength : 1.0f);
	
	m_ayAnimSequence[iChannel] = (m_ayAnimSequence[iChannel] + 1) & 0xff;
	m_afAnimSpeed[iChannel] = fSpeed * fLength;
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
		Console.Warn << _T("IVisualEntity::StartAnimation() - Invalid animation channel: ") << iChannel
					<< _T(", using 0") << newl;
		return;
	}

	m_asAnim[iChannel] = TSNULL;
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

	if (m_asAnim[iChannel] == sAnimName)
	{
		m_asAnim[iChannel] = TSNULL;
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
	return g_ResourceManager.GetAnim(GetModel(), sAnimName);
}


/*====================
  IVisualEntity::AddToScene
  ====================*/
bool	IVisualEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
	//PROFILE("IVisualEntity::AddToScene");

	if (GetModel() == INVALID_INDEX)
		return false;

	CVec4f v4TintedColor(v4Color);

	CPlayer *pLocalPlayer(Game.GetLocalPlayer());
	if (pLocalPlayer == NULL)
		return false;

	if (!pLocalPlayer->CanSee(this))
		return false;

	if (m_v3AxisAngles != m_v3Angles)
	{
		m_aAxis.Set(m_v3Angles);
		m_v3AxisAngles = m_v3Angles;
	}

	static CSceneEntity sceneEntity;

	sceneEntity.Clear();
	sceneEntity.scale = GetBaseScale() * GetScale();
	sceneEntity.SetPosition(m_v3Position);
	sceneEntity.axis = m_aAxis;
	sceneEntity.objtype = OBJTYPE_MODEL;
	sceneEntity.hRes = GetModel();
	sceneEntity.skeleton = m_pSkeleton;
	sceneEntity.color = v4TintedColor;
	sceneEntity.flags = iFlags | SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

	if (IsHighlighted())
		sceneEntity.color *= m_v4HighlightColor;

	if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
		sceneEntity.color[A] *= 0.5f;

	SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

	if (!cEntry.bCull || !cEntry.bCullShadow)
		UpdateSkeleton(true);
	else
		UpdateSkeleton(false);

	return true;
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
  IVisualEntity::Copy
  ====================*/
void	IVisualEntity::Copy(const IGameEntity &B)
{
	IGameEntity::Copy(B);

	const IVisualEntity *pB(B.GetAsVisual());

	if (!pB)	
		return;

	const IVisualEntity &C(*pB);

	m_bValid =				C.m_bValid;
	m_sName =				C.m_sName;
	m_uiIndex =				C.m_uiIndex;
	m_uiWorldIndex =		C.m_uiWorldIndex;

	SetTeam(C.m_uiTeamID);

	m_yStatus =					C.m_yStatus;
	m_ySequence =				C.m_ySequence;
	m_unVisibilityFlags =		C.m_unVisibilityFlags;

	m_v3Position =			C.m_v3Position;
	m_v3Velocity =			C.m_v3Velocity;
	m_v3Angles =			C.m_v3Angles;
	m_fScale =				C.m_fScale;
	m_bbBounds =			C.m_bbBounds;

	m_pSkeleton =			C.m_pSkeleton;

	m_yNextEventSlot =		C.m_yNextEventSlot;
	m_yLastProcessedEvent =	C.m_yLastProcessedEvent;

	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
	{
		m_ayAnim[i] =			C.m_ayAnim[i];
		m_ayAnimSequence[i] =	C.m_ayAnimSequence[i];
		m_afAnimSpeed[i] =		C.m_afAnimSpeed[i];
		m_auiAnimLockTime[i] =	C.m_auiAnimLockTime[i];
	}

	for (int i(0); i < NUM_EFFECT_CHANNELS; ++i)
	{
		m_ahEffect[i] =			C.m_ahEffect[i];
		m_ayEffectSequence[i] =	C.m_ayEffectSequence[i];
	}
}


/*====================
  IVisualEntity::IsPositionValid

  Determines whether or not a position is valid
  ====================*/
bool	IVisualEntity::IsPositionValid(const CVec3f &v3Position)
{
	STraceInfo trace;

	Game.TraceBox(trace, v3Position, v3Position + CVec3f(0.0, 0.0, CONTACT_EPSILON * 1.25f), m_bbBounds, TRACE_UNIT_MOVEMENT, m_uiWorldIndex);

	return !trace.bStartedInSurface;
}


/*====================
  IVisualEntity::ClientPrecache
  ====================*/
void	IVisualEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
	IGameEntity::ClientPrecache(pConfig, eScheme);
}


/*====================
  IVisualEntity::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IVisualEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
	IGameEntity::ServerPrecache(pConfig, eScheme);
}


/*====================
  IVisualEntity::Interpolate
  ====================*/
void	IVisualEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
	if (pPrevState->GetNoInterpolateSequence() != pNextState->GetNoInterpolateSequence())
		return;

	m_v3Angles = M_LerpAngles(fLerp, pPrevState->m_v3Angles, pNextState->m_v3Angles);
	m_v3Position = LERP(fLerp, pPrevState->m_v3Position, pNextState->m_v3Position);
	m_fScale = LERP(fLerp, pPrevState->m_fScale, pNextState->m_fScale);
	m_unVisibilityFlags = pPrevState->m_unVisibilityFlags & pNextState->m_unVisibilityFlags;
}


/*====================
  IVisualEntity::UpdateEffectThreadSource
  ====================*/
void	IVisualEntity::UpdateEffectThreadSource(CEffectThread *pEffectThread)
{
	pEffectThread->SetSourceModel(g_ResourceManager.GetModel(GetModel()));
	pEffectThread->SetSourceSkeleton(m_pSkeleton);

	pEffectThread->SetSourcePos(m_v3Position);
	pEffectThread->SetSourceAxis(m_aAxis);

	pEffectThread->SetSourceScale(GetBaseScale() * GetScale());

	if (pEffectThread->GetUseEntityEffectScale())
		pEffectThread->SetSourceEffectScale(GetEffectScale() / (GetBaseScale() * GetScale()));
	else
		pEffectThread->SetSourceEffectScale(1.0f);

	pEffectThread->SetSourceVisibility(Game.GetLocalPlayer() == NULL || Game.GetLocalPlayer()->CanSee(this));
}


/*====================
  IVisualEntity::UpdateEffectThreadTarget
  ====================*/
void	IVisualEntity::UpdateEffectThreadTarget(CEffectThread *pEffectThread)
{
	pEffectThread->SetTargetModel(g_ResourceManager.GetModel(GetModel()));
	pEffectThread->SetTargetSkeleton(m_pSkeleton);

	pEffectThread->SetTargetPos(m_v3Position);
	pEffectThread->SetTargetAxis(m_aAxis);

	pEffectThread->SetTargetScale(GetBaseScale() * GetScale());

	if (pEffectThread->GetUseEntityEffectScale())
		pEffectThread->SetTargetEffectScale(GetEffectScale() / (GetBaseScale() * GetScale()));
	else
		pEffectThread->SetTargetEffectScale(1.0f);

	pEffectThread->SetTargetVisibility(Game.GetLocalPlayer() == NULL || Game.GetLocalPlayer()->CanSee(this));
}


/*====================
  IVisualEntity::GetTerrainType
  ====================*/
const tstring&	IVisualEntity::GetTerrainType()
{
	if (m_uiLastTerrainTypeUpdateTime == Game.GetGameTime())
		return m_sTerrainType;

	CBBoxf bbWorldBounds(m_bbBounds);
	bbWorldBounds.Transform(m_v3Position, m_aAxis, GetBaseScale() * GetScale());

	static WorldEntVector vEntities;
	vEntities.clear();

	Game.GetEntityHandlesInRegion(vEntities, bbWorldBounds, SURF_STATIC | SURF_DYNAMIC | SURF_TREE);
	WorldEntVector_cit cit(vEntities.begin()), citEnd(vEntities.end());
	CWorldEntity *pWorldEnt(NULL);
	for (; cit != citEnd; ++cit)
	{
		pWorldEnt = Game.GetWorldPointer()->GetEntityByHandle(*cit);
		if (pWorldEnt->GetSurfFlags() & SURF_WATER)
			break;
	}

	if (cit != citEnd && m_v3Position.z + 8.0f < pWorldEnt->GetPosition().z)
	{
		m_sTerrainType = pWorldEnt->GetProperty(_T("effecttype"));
		return m_sTerrainType;
	}
	else
	{
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
}


/*====================
  IVisualEntity::Bind
  ====================*/
void	IVisualEntity::Bind(IUnitEntity *pTarget, const CVec3f &v3Offset, uint uiFlags)
{
	if (pTarget == NULL || pTarget->IsBit())
		return;

	pTarget->m_uiBindTargetUID = GetUniqueID();
	pTarget->SetUnitFlags(UNIT_FLAG_BOUND);

	SEntityBind cBind;
	cBind.uiEntityUID = pTarget->GetUniqueID();
	cBind.v3Offset = v3Offset;
	cBind.uiFlags = uiFlags;

	CVec3f v3OldPosition(pTarget->GetPosition());

	if (cBind.uiFlags & ENT_BIND_TURN)
	{
		CVec3f v3BindOffset(cBind.v3Offset);
		v3BindOffset.xy().Rotate(GetAngles()[YAW]);

		pTarget->SetPosition(GetPosition() + v3BindOffset);
	}
	else
	{
		pTarget->SetPosition(GetPosition() + cBind.v3Offset);
	}

	if (pTarget->GetPosition() != v3OldPosition)
	{
		Unlink();
		Link();
	}

	if (cBind.uiFlags & ENT_BIND_TURN)
	{
		pTarget->SetAngles(GetAngles());
		pTarget->SetAttentionYaw(GetAngles()[YAW]);
	}

	pTarget->Moved();

	m_vBinds.push_back(cBind);
}


/*====================
  IVisualEntity::ReleaseBinds
  ====================*/
void	IVisualEntity::ReleaseBinds()
{
	for (vector<SEntityBind>::iterator it(m_vBinds.begin()); it != m_vBinds.end(); ++it)
	{
		IGameEntity *pGameEntity(Game.GetEntityFromUniqueID(it->uiEntityUID));
		IUnitEntity *pEntity(pGameEntity ? pGameEntity->GetAsUnit() : NULL);
		if (pEntity == NULL)
			continue;

		pEntity->m_uiBindTargetUID = INVALID_INDEX;
		pEntity->RemoveUnitFlags(UNIT_FLAG_BOUND);
		pEntity->ValidatePosition(TRACE_UNIT_SPAWN);
		pEntity->Action(ACTION_SCRIPT_RELEASE, GetAsUnit(), this);
	}
	m_vBinds.clear();

	for (uivector_it it(m_vBindStateUIDs.begin()); it != m_vBindStateUIDs.end(); ++it)
	{
		IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
		if (pEntity == NULL)
			continue;
		IEntityState *pState(pEntity->GetAsState());
		if (pState == NULL)
			continue;
		IUnitEntity *pOwner(pState->GetOwner());
		if (pOwner == NULL)
			continue;

		pState->SetExpireNextFrame(true);
	}
	m_vBindStateUIDs.clear();
}


/*====================
  IVisualEntity::ReleaseBind
  ====================*/
void	IVisualEntity::ReleaseBind(uint uiUID)
{
	for (vector<SEntityBind>::iterator it(m_vBinds.begin()); it != m_vBinds.end(); ++it)
	{
		if (it->uiEntityUID != uiUID)
			continue;

		IGameEntity *pGameEntity(Game.GetEntityFromUniqueID(it->uiEntityUID));
		IUnitEntity *pEntity(pGameEntity ? pGameEntity->GetAsUnit() : NULL);
		if (pEntity == NULL)
			continue;

		pEntity->m_uiBindTargetUID = INVALID_INDEX;
		pEntity->RemoveUnitFlags(UNIT_FLAG_BOUND);
		pEntity->ValidatePosition(TRACE_UNIT_SPAWN);
		pEntity->Action(ACTION_SCRIPT_RELEASE, GetAsUnit(), this);

		it->uiEntityUID = INVALID_INDEX;
	}

	for (uivector_it it(m_vBindStateUIDs.begin()); it != m_vBindStateUIDs.end(); ++it)
	{
		IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
		if (pEntity == NULL)
			continue;
		IEntityState *pState(pEntity->GetAsState());
		if (pState == NULL)
			continue;
		IUnitEntity *pOwner(pState->GetOwner());
		if (pOwner == NULL)
			continue;

		if (pOwner->GetUniqueID() != uiUID)
			continue;

		pState->SetExpireNextFrame(true);

		*it = INVALID_INDEX;
	}
}


/*====================
  IVisualEntity::GetBindFlags
  ====================*/
uint	IVisualEntity::GetBindFlags(uint uiUID)
{
	for (vector<SEntityBind>::iterator it(m_vBinds.begin()); it != m_vBinds.end(); ++it)
	{
		if (it->uiEntityUID != uiUID)
			continue;

		return it->uiFlags;
	}

	return 0;
}


/*====================
  IVisualEntity::HasBinds
  ====================*/
bool	IVisualEntity::HasBinds()
{
	return !m_vBinds.empty();
}


/*====================
  IVisualEntity::Unbind
  ====================*/
void	IVisualEntity::Unbind()
{
	if (m_uiBindTargetUID == INVALID_INDEX)
		return;
	
	IGameEntity *pBindTarget(Game.GetEntityFromUniqueID(m_uiBindTargetUID));
	if (pBindTarget == NULL || !pBindTarget->IsVisual())
		return;

	pBindTarget->GetAsVisual()->ReleaseBind(m_uiUniqueID);
}


/*====================
  IVisualEntity::GetBindFlags
  ====================*/
uint	IVisualEntity::GetBindFlags()
{
	if (m_uiBindTargetUID == INVALID_INDEX)
		return 0;
	
	IGameEntity *pBindTarget(Game.GetEntityFromUniqueID(m_uiBindTargetUID));
	if (pBindTarget == NULL || !pBindTarget->IsVisual())
		return 0;

	return pBindTarget->GetAsVisual()->GetBindFlags(m_uiUniqueID);
}


/*====================
  IVisualEntity::ServerFrameMovementEnd
  ====================*/
bool	IVisualEntity::ServerFrameMovementEnd()
{
	for (vector<SEntityBind>::iterator it(m_vBinds.begin()); it != m_vBinds.end(); ++it)
	{
		IGameEntity *pGameEntity(Game.GetEntityFromUniqueID(it->uiEntityUID));
		IVisualEntity *pEntity(pGameEntity ? pGameEntity->GetAsVisual() : NULL);
		if (pEntity == NULL)
		{
			it->uiEntityUID = INVALID_INDEX;
			continue;
		}

		CVec3f v3OldPosition(pEntity->GetPosition());
		CVec2f v3OldAngles(pEntity->GetAngles());

		if (it->uiFlags & ENT_BIND_TURN)
		{
			CVec3f v3BindOffset(it->v3Offset);
			v3BindOffset.xy().Rotate(GetAngles()[YAW]);

			pEntity->SetPosition(GetPosition() + v3BindOffset);
			pEntity->SetAngles(GetAngles());
			if (pEntity->IsUnit())
				pEntity->GetAsUnit()->SetAttentionYaw(GetAngles()[YAW]);
		}
		else
		{
			pEntity->SetPosition(GetPosition() + it->v3Offset);
		}

		if (pEntity->GetPosition() != v3OldPosition ||
			pEntity->GetAngles() != v3OldAngles)
		{
			pEntity->Unlink();
			pEntity->Link();
		}

		IUnitEntity *pUnit(pEntity->GetAsUnit());
		if (pUnit != NULL && !(it->uiFlags & ENT_BIND_NO_PUSH))
			pUnit->Moved();

		if (m_vBinds.empty())
			break;
	}

	return true;
}
