// (C)2006 S2 Games
// c_gameevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gameevent.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_effect.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_networkresourcemanager.h"
//=============================================================================

/*====================
  CGameEvent::~CGameEvent
  ====================*/
CGameEvent::~CGameEvent()
{
}


/*====================
  CGameEvent::CGameEvent
  ====================*/
CGameEvent::CGameEvent() :
m_bValid(true),
m_uiIndex(INVALID_INDEX),
m_unFlags(0),
m_uiExpireTime(INVALID_TIME),
m_uiSourceEntityIndex(INVALID_INDEX),
m_v3SourcePosition(V3_ZERO),
m_v3SourceAngles(V3_ZERO),
m_fSourceScale(1.0f),
m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO),
m_fTargetScale(1.0f),
m_hEffect(INVALID_RESOURCE),
m_hSound(INVALID_RESOURCE),
m_pEffectThread(NULL),
m_bEffectActive(true)
{
}

CGameEvent::CGameEvent(const IBuffer &buffer) :
m_bValid(true),
m_uiIndex(INVALID_INDEX),
m_unFlags(0),
m_uiExpireTime(INVALID_TIME),
m_uiSourceEntityIndex(INVALID_INDEX),
m_v3SourcePosition(V3_ZERO),
m_v3SourceAngles(V3_ZERO),
m_fSourceScale(1.0f),
m_uiTargetEntityIndex(INVALID_INDEX),
m_v3TargetPosition(V3_ZERO),
m_v3TargetAngles(V3_ZERO),
m_fTargetScale(1.0f),
m_hEffect(INVALID_RESOURCE),
m_hSound(INVALID_RESOURCE),
m_pEffectThread(NULL),
m_bEffectActive(true)
{
	try
	{
		m_unFlags = buffer.ReadShort();
		
		if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
			m_uiExpireTime = buffer.ReadInt();
		if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
			m_uiSourceEntityIndex = buffer.ReadShort();
		if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
		{
			m_v3SourcePosition.x = ushort(buffer.ReadShort());
			m_v3SourcePosition.y = ushort(buffer.ReadShort());
			m_v3SourcePosition.z = buffer.ReadShort();
		}
		if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
		{
			m_v3SourceAngles.x = M_GetAngle(buffer.ReadByte());
			m_v3SourceAngles.y = M_GetAngle(buffer.ReadByte());
			m_v3SourceAngles.z = M_GetAngle(buffer.ReadByte());
		}
		if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
			m_fSourceScale = buffer.ReadFloat();
		if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
			m_uiSourceEntityIndex = buffer.ReadShort();
		if (m_unFlags & EVENT_HAS_TARGET_POSITION)
		{
			m_v3TargetPosition.x = ushort(buffer.ReadShort());
			m_v3TargetPosition.y = ushort(buffer.ReadShort());
			m_v3TargetPosition.z = buffer.ReadShort();
		}
		if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
		{
			m_v3TargetAngles.x = M_GetAngle(buffer.ReadByte());
			m_v3TargetAngles.y = M_GetAngle(buffer.ReadByte());
			m_v3TargetAngles.z = M_GetAngle(buffer.ReadByte());
		}
		if (m_unFlags & EVENT_HAS_TARGET_SCALE)
			m_fTargetScale = buffer.ReadFloat();
		if (m_unFlags & EVENT_HAS_EFFECT)
			m_hEffect = NetworkResourceManager.GetLocalHandle(buffer.ReadShort());
		if (m_unFlags & EVENT_HAS_SOUND)
			m_hSound = NetworkResourceManager.GetLocalHandle(buffer.ReadShort());
	}
	catch (CException &ex)
	{
		ex.Process(_T("CGameEvent::CGameEvent() - "), NO_THROW);
		m_bValid = false;
	}
}


/*====================
  CGameEvent::Clear
  ====================*/
void	CGameEvent::Clear()
{
	m_bValid = true;
	m_unFlags = 0;
	m_uiExpireTime = INVALID_TIME;
	m_uiSourceEntityIndex = INVALID_INDEX;
	m_v3SourcePosition.Clear();
	m_v3SourceAngles.Clear();
	m_fSourceScale = 1.0f;
	m_uiTargetEntityIndex = INVALID_INDEX;
	m_v3TargetPosition.Clear();
	m_v3TargetAngles.Clear();
	m_fTargetScale = 1.0f;
	m_hEffect = INVALID_RESOURCE;
	m_hSound = INVALID_RESOURCE;

	SAFE_DELETE(m_pEffectThread);
}


/*====================
  CGameEvent::GetBuffer
  ====================*/
void	CGameEvent::GetBuffer(IBuffer &buffer)
{
	buffer.Clear();
	buffer.WriteShort(m_unFlags);
	if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
		buffer.WriteInt(m_uiExpireTime);
	if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
		buffer.WriteShort(m_uiSourceEntityIndex);
	if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
	{
		buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3SourcePosition.x, 0.0f, float(USHRT_MAX)))));
		buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3SourcePosition.y, 0.0f, float(USHRT_MAX)))));
		buffer.WriteShort(INT_ROUND(CLAMP(m_v3SourcePosition.z, float(SHRT_MIN), float(SHRT_MAX))));
	}
	if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
	{
		buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.x));
		buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.y));
		buffer.WriteByte(M_GetAngle8(m_v3SourceAngles.z));
	}
	if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
		buffer.WriteFloat(m_fSourceScale);
	if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
		buffer.WriteShort(m_uiTargetEntityIndex);
	if (m_unFlags & EVENT_HAS_TARGET_POSITION)
	{
		buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3TargetPosition.x, 0.0f, float(USHRT_MAX)))));
		buffer.WriteShort(ushort(INT_ROUND(CLAMP(m_v3TargetPosition.y, 0.0f, float(USHRT_MAX)))));
		buffer.WriteShort(INT_ROUND(CLAMP(m_v3TargetPosition.z, float(SHRT_MIN), float(SHRT_MAX))));
	}
	if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
	{
		buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.x));
		buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.y));
		buffer.WriteByte(M_GetAngle8(m_v3TargetAngles.z));
	}
	if (m_unFlags & EVENT_HAS_TARGET_SCALE)
		buffer.WriteFloat(m_fTargetScale);
	if (m_unFlags & EVENT_HAS_EFFECT)
		buffer.WriteShort(NetworkResourceManager.GetNetIndex(m_hEffect));
	if (m_unFlags & EVENT_HAS_SOUND)
		buffer.WriteShort(NetworkResourceManager.GetNetIndex(m_hSound));
}


/*====================
  CGameEvent::Translate
  ====================*/
void	 CGameEvent::Translate(const IBuffer &bufferIn, IBuffer &bufferOut)
{
	ushort unFlags(bufferIn.ReadShort());

	bufferOut.Clear();
	bufferOut.WriteShort(unFlags);

	uint uiSize(0);
	
	if (unFlags & EVENT_HAS_EXPIRE_TIME)
		uiSize += 4;
	if (unFlags & EVENT_HAS_SOURCE_ENTITY)
		uiSize += 2;
	if (unFlags & EVENT_HAS_SOURCE_POSITION)
		uiSize += 6;
	if (unFlags & EVENT_HAS_SOURCE_ANGLES)
		uiSize += 3;
	if (unFlags & EVENT_HAS_SOURCE_SCALE)
		uiSize += 4;
	if (unFlags & EVENT_HAS_TARGET_ENTITY)
		uiSize += 2;
	if (unFlags & EVENT_HAS_TARGET_POSITION)
		uiSize += 6;
	if (unFlags & EVENT_HAS_TARGET_ANGLES)
		uiSize += 3;
	if (unFlags & EVENT_HAS_TARGET_SCALE)
		uiSize += 4;
	if (unFlags & EVENT_HAS_EFFECT)
		uiSize += 2;
	if (unFlags & EVENT_HAS_SOUND)
		uiSize += 2;

	bufferOut.Append(bufferIn.Get(bufferIn.GetReadPos()), uiSize);
	bufferIn.Advance(uiSize);
}


/*====================
  CGameEvent::AdvanceBuffer
  ====================*/
void	 CGameEvent::AdvanceBuffer(const IBuffer &buffer)
{
	ushort unFlags(buffer.ReadShort());

	int uiSize(0);
	
	if (unFlags & EVENT_HAS_EXPIRE_TIME)
		uiSize += 4;
	if (unFlags & EVENT_HAS_SOURCE_ENTITY)
		uiSize += 2;
	if (unFlags & EVENT_HAS_SOURCE_POSITION)
		uiSize += 6;
	if (unFlags & EVENT_HAS_SOURCE_ANGLES)
		uiSize += 3;
	if (unFlags & EVENT_HAS_SOURCE_SCALE)
		uiSize += 4;
	if (unFlags & EVENT_HAS_TARGET_ENTITY)
		uiSize += 2;
	if (unFlags & EVENT_HAS_TARGET_POSITION)
		uiSize += 6;
	if (unFlags & EVENT_HAS_TARGET_ANGLES)
		uiSize += 3;
	if (unFlags & EVENT_HAS_TARGET_SCALE)
		uiSize += 4;
	if (unFlags & EVENT_HAS_EFFECT)
		uiSize += 2;
	if (unFlags & EVENT_HAS_SOUND)
		uiSize += 2;

	buffer.Advance(uiSize);
}


/*====================
  CGameEvent::Print
  ====================*/
void	CGameEvent::Print() const
{
	Console.Dev << _T("GameEvent:");

	if (m_unFlags & EVENT_HAS_EXPIRE_TIME)
		Console.Dev << _T(" Expire: ") << m_uiExpireTime;
	if (m_unFlags & EVENT_HAS_SOURCE_ENTITY)
		Console.Dev << _T(" Ent: ") << m_uiSourceEntityIndex;
	if (m_unFlags & EVENT_HAS_SOURCE_POSITION)
		Console.Dev << _T(" Pos: ") << m_v3SourcePosition;
	if (m_unFlags & EVENT_HAS_SOURCE_ANGLES)
		Console.Dev << _T(" Angles: ") << m_v3SourceAngles;
	if (m_unFlags & EVENT_HAS_SOURCE_SCALE)
		Console.Dev << _T(" Scale: ") << m_fSourceScale;
	if (m_unFlags & EVENT_HAS_TARGET_ENTITY)
		Console.Dev << _T(" Ent: ") << m_uiTargetEntityIndex;
	if (m_unFlags & EVENT_HAS_TARGET_POSITION)
		Console.Dev << _T(" Pos: ") << m_v3TargetPosition;
	if (m_unFlags & EVENT_HAS_TARGET_ANGLES)
		Console.Dev << _T(" Angles: ") << m_v3TargetAngles;
	if (m_unFlags & EVENT_HAS_TARGET_SCALE)
		Console.Dev << _T(" Scale: ") << m_fTargetScale;
	if (m_unFlags & EVENT_HAS_EFFECT)
		Console.Dev << _T(" Effect: ") << INT_HEX_STR(m_hEffect);
	if (m_unFlags & EVENT_HAS_SOUND)
		Console.Dev << _T(" Sound: ") << INT_HEX_STR(m_hSound);

	Console.Dev << newl;
}


/*====================
  CGameEvent::GetVisualEntityIndex

  returns state owner if the index is a state, otherwise just returns the index
  ====================*/
uint	CGameEvent::GetVisualEntityIndex(uint uiIndex)
{
	IGameEntity *pEntity(Game.GetEntity(uiIndex));
	if (pEntity != NULL && pEntity->IsState())
		uiIndex = pEntity->GetAsState()->GetOwner();

	return uiIndex;
}


/*====================
  CGameEvent::SynchWithEntity
  ====================*/
bool	CGameEvent::SynchWithEntity()
{
	// If this event references an entity, update from the
	// entities information
	IVisualEntity *pVisSourceEntity(NULL);
	if (m_uiSourceEntityIndex != INVALID_INDEX)
	{
		uint uiSourceIndex(GetVisualEntityIndex(m_uiSourceEntityIndex));

		pVisSourceEntity = Game.GetVisualEntity(uiSourceIndex);
		if (pVisSourceEntity == NULL)
		{
			if (!(m_unFlags & EVENT_SPAWNED_THIS_FRAME))
			{
				m_uiSourceEntityIndex = INVALID_INDEX;
				return false;
			}
		}
		else
		{
			m_v3SourcePosition = pVisSourceEntity->GetPosition();

			if (pVisSourceEntity->IsPlayer())
				m_v3SourceAngles = CVec3f(0.0f, 0.0f, pVisSourceEntity->GetAsPlayerEnt()->GetCurrentYaw());
			else if (pVisSourceEntity->IsPet())
				m_v3SourceAngles = CVec3f(0.0f, 0.0f, pVisSourceEntity->GetAsPet()->GetCurrentYaw());
			else
				m_v3SourceAngles = pVisSourceEntity->GetAngles();
			
			m_fSourceScale = pVisSourceEntity->GetScale() * pVisSourceEntity->GetScale2();
		}
	}

	IVisualEntity *pVisTargetEntity(NULL);
	if (m_uiTargetEntityIndex != INVALID_INDEX)
	{
		uint uiTargetIndex(GetVisualEntityIndex(m_uiTargetEntityIndex));

		pVisTargetEntity = Game.GetVisualEntity(uiTargetIndex);
		if (pVisTargetEntity == NULL)
		{
			if (!(m_unFlags & EVENT_SPAWNED_THIS_FRAME))
			{
				m_uiTargetEntityIndex = INVALID_INDEX;
				return false;
			}
		}
		else
		{
			m_v3TargetPosition = pVisTargetEntity->GetPosition();
			m_v3TargetAngles = pVisTargetEntity->GetAngles();
			m_fTargetScale = pVisTargetEntity->GetScale() * pVisTargetEntity->GetScale2();
		}
	}

	if (m_pEffectThread == NULL)
		return true;

	if (pVisSourceEntity != NULL)
	{
		m_pEffectThread->SetSourceSkeleton(pVisSourceEntity->GetSkeleton());
		m_pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pVisSourceEntity->GetModelHandle()));
	}
	else
	{
		m_pEffectThread->SetSourceSkeleton(NULL);
		m_pEffectThread->SetSourceModel(NULL);
	}
	
	if (pVisTargetEntity != NULL)
	{
		m_pEffectThread->SetTargetSkeleton(pVisTargetEntity->GetSkeleton());
		m_pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pVisTargetEntity->GetModelHandle()));
	}
	else
	{
		m_pEffectThread->SetTargetSkeleton(NULL);
		m_pEffectThread->SetTargetModel(NULL);
	}

	return true;
}


/*====================
  CGameEvent::Spawn
  ====================*/
void	CGameEvent::Spawn()
{
	m_unFlags |= EVENT_SPAWNED_THIS_FRAME;

	if (!SynchWithEntity())
		SetExpireNextFrame();

	// Start requested effect
	if (m_pEffectThread != NULL)
		return;

	if (m_hEffect == INVALID_RESOURCE)
		return;

	CEffect	*pEffect(g_ResourceManager.GetEffect(m_hEffect));
	if (pEffect == NULL)
		return;

	m_pEffectThread = pEffect->SpawnThread(Game.GetGameTime());
	if (m_pEffectThread == NULL)
		return;

	m_pEffectThread->SetCamera(Game.GetCamera());
	m_pEffectThread->SetWorld(Game.GetWorldPointer());

	IVisualEntity *pSourceEntity(Game.GetVisualEntity(m_uiSourceEntityIndex));
	if (pSourceEntity != NULL)
	{
		m_pEffectThread->SetSourceSkeleton(pSourceEntity->GetSkeleton());
		m_pEffectThread->SetSourceModel(g_ResourceManager.GetModel(pSourceEntity->GetModelHandle()));
	}
	else
	{
		m_pEffectThread->SetSourceSkeleton(NULL);
		m_pEffectThread->SetSourceModel(NULL);
	}
	
	IVisualEntity *pTargetEntity(Game.GetVisualEntity(m_uiTargetEntityIndex));
	if (pTargetEntity != NULL)
	{
		m_pEffectThread->SetTargetSkeleton(pTargetEntity->GetSkeleton());
		m_pEffectThread->SetTargetModel(g_ResourceManager.GetModel(pTargetEntity->GetModelHandle()));
	}
	else
	{
		m_pEffectThread->SetTargetSkeleton(NULL);
		m_pEffectThread->SetTargetModel(NULL);
	}
}


/*====================
  CGameEvent::Frame
  ====================*/
bool	CGameEvent::Frame()
{
	if ((m_unFlags & EVENT_NO_FIRST_PERSON) && (Game.GetCamera() && Game.GetCamera()->HasFlags(CAM_FIRST_PERSON)))
		return false;

	if ((m_unFlags & EVENT_ONLY_FIRST_PERSON) && (Game.GetCamera() && !Game.GetCamera()->HasFlags(CAM_FIRST_PERSON)))
		return false;

	// Check to see if the event has expired
	if (m_uiExpireTime < Game.GetGameTime() || !m_pEffectThread || (m_unFlags & EVENT_EXPIRE_NEXT_FRAME))
	{
		//Console << _T("Effect finished: ") << m_hEffect << newl;
		return false;
	}

	return SynchWithEntity();
}


/*====================
  CGameEvent::AddToScene
  ====================*/
void	CGameEvent::AddToScene()
{
	// Draw effects
	if (!m_pEffectThread)
		return;

	uint uiSourceIndex(GetVisualEntityIndex(m_uiSourceEntityIndex));
	uint uiTargetIndex(GetVisualEntityIndex(m_uiTargetEntityIndex));

	// Culling
	bool bCull(false);
	if (Game.IsCommander())
	{
		if (uiSourceIndex != INVALID_INDEX && uiTargetIndex != INVALID_INDEX)
		{
			IVisualEntity *pSourceEntity(Game.GetVisualEntity(uiSourceIndex));
			IVisualEntity *pTargetEntity(Game.GetVisualEntity(uiTargetIndex));
			if ((pSourceEntity && (!pSourceEntity->GetSighted() || !pSourceEntity->GetShowEffects())) &&
				(pTargetEntity && (!pTargetEntity->GetSighted() || !pTargetEntity->GetShowEffects())))
				bCull = true;
		}
		else if (uiSourceIndex != INVALID_INDEX)
		{
			IVisualEntity *pEntity(Game.GetVisualEntity(uiSourceIndex));
			if (pEntity && (!pEntity->GetSighted() || !pEntity->GetShowEffects()))
				bCull = true;
		}
		else if (uiTargetIndex != INVALID_INDEX)
		{
			IVisualEntity *pEntity(Game.GetVisualEntity(uiTargetIndex));
			if (pEntity && (!pEntity->GetSighted() || !pEntity->GetShowEffects()))
				bCull = true;
		}
	}
	else
	{
		if (uiSourceIndex != INVALID_INDEX && uiTargetIndex != INVALID_INDEX)
		{
			IVisualEntity *pSourceEntity(Game.GetVisualEntity(uiSourceIndex));
			IVisualEntity *pTargetEntity(Game.GetVisualEntity(uiTargetIndex));
			if ((pSourceEntity && !pSourceEntity->GetShowEffects()) &&
				(pTargetEntity && !pTargetEntity->GetShowEffects()))
				bCull = true;
		}
		else if (uiSourceIndex != INVALID_INDEX)
		{
			IVisualEntity *pEntity(Game.GetVisualEntity(uiSourceIndex));
			if (pEntity && !pEntity->GetShowEffects())
				bCull = true;
		}
		else if (uiTargetIndex != INVALID_INDEX)
		{
			IVisualEntity *pEntity(Game.GetVisualEntity(uiTargetIndex));
			if (pEntity && !pEntity->GetShowEffects())
				bCull = true;
		}
		else
		{
			if (DistanceSq(m_v3SourcePosition, Game.GetCamera()->GetOrigin()) > SQR(MIN<float>(scene_farClip, scene_worldFarClip)))
				bCull = true;
		}
	}

	// Set effect parameters
	m_pEffectThread->SetActive(m_bEffectActive);

	m_pEffectThread->SetSourcePos(m_v3SourcePosition);
	m_pEffectThread->SetSourceAxis(CAxis(m_v3SourceAngles));
	m_pEffectThread->SetSourceScale(m_fSourceScale);
	m_pEffectThread->SetTargetPos(m_v3TargetPosition);
	m_pEffectThread->SetTargetAxis(CAxis(m_v3TargetAngles));
	m_pEffectThread->SetTargetScale(m_fTargetScale);
	
	if (m_pEffectThread->Execute(Game.GetGameTime()))
	{
		SAFE_DELETE(m_pEffectThread);
		return;
	}

	// Camera effects
	Game.AddCameraEffectOffset(m_pEffectThread->GetCameraOffset());
	Game.AddCameraEffectAngleOffset(m_pEffectThread->GetCameraAngleOffset());

	// Overlays
	IGameEntity *pEntity(Game.GetEntity(m_uiSourceEntityIndex));
	if (pEntity != NULL)
	{
		IPlayerEntity *pPlayer(pEntity->GetAsPlayerEnt());
		IEntityState *pState(pEntity->GetAsState());
		if (pPlayer == NULL && pState != NULL)
			pPlayer = Game.GetPlayerEntity(pState->GetOwner());

		if (pPlayer != NULL &&
			pPlayer->GetClientID() == Game.GetLocalClientNum() &&
			m_pEffectThread->HasActiveOverlay())
			Game.AddOverlay(m_pEffectThread->GetOverlayColor(), m_pEffectThread->GetOverlayMaterial());
	}

	// Update and render all particle systems associated with this effect thread
	const InstanceMap &mapInstances(m_pEffectThread->GetInstances());
	for (InstanceMap::const_iterator it(mapInstances.begin()); it != mapInstances.end(); ++it)
	{
		IEffectInstance *pParticleSystem(it->second);
		pParticleSystem->Update(Game.GetGameTime());

		if (!pParticleSystem->IsDead() && pParticleSystem->IsParticleSystem() && !bCull)
			SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), true);
	}

	m_pEffectThread->Cleanup();
}
