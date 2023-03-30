// (C)2007 S2 Games
// c_entitysoul.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitysoul.h"
#include "c_entityclientinfo.h"

#include "../k2/c_texture.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	CEntitySoul::s_pvFields;

DEFINE_ENT_ALLOCATOR2(Entity, Soul)

CVAR_FLOATF(	g_hellbourneHealthPerSoul,	50.0f,	CVAR_GAMECONFIG);
//=============================================================================


/*====================
  CEntitySoul::CEntityConfig::CEntityConfig
  ====================*/
CEntitySoul::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpawnDelay, 1000),
INIT_ENTITY_CVAR(InitialSpeed, 300.0f),
INIT_ENTITY_CVAR(InitialSpeedDistanceFactor, 0.1f),
INIT_ENTITY_CVAR(LerpTime, 3000),
INIT_ENTITY_CVAR(TrailEffectPath, _T("")),
INIT_ENTITY_CVAR(EndEffectPath, _T(""))
{
}


/*====================
  CEntitySoul::CEntitySoul
  ====================*/
CEntitySoul::CEntitySoul() :
IVisualEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_uiTarget(INVALID_INDEX)
{
}


/*====================
  CEntitySoul::Baseline
  ====================*/
void	CEntitySoul::Baseline()
{
	m_v3Position = V3_ZERO;
	m_v3Angles = V3_ZERO;
	m_hModel = INVALID_RESOURCE;

	// Effects
	for (int i(0); i < 1; ++i)
	{
		m_ahEffect[i] = INVALID_INDEX;
		m_ayEffectSequence[i] = 0;
	}
}


/*====================
  CEntitySoul::GetSnapshot
  ====================*/
void	CEntitySoul::GetSnapshot(CEntitySnapshot &snapshot) const
{
	snapshot.AddField(m_v3Position);
	snapshot.AddField(m_v3Angles);
	snapshot.AddResHandle(m_hModel);

	// Effects
	for (int i(0); i < 1; ++i)
	{
		snapshot.AddResHandle(m_ahEffect[i]);
		snapshot.AddField(m_ayEffectSequence[i]);
	}
}


/*====================
  CEntitySoul::ReadSnapshot
  ====================*/
bool	CEntitySoul::ReadSnapshot(CEntitySnapshot &snapshot)
{
	snapshot.ReadNextField(m_v3Position);
	snapshot.ReadNextField(m_v3Angles);
	snapshot.ReadNextResHandle(m_hModel);

	// Effects
	for (int i(0); i < 1; ++i)
	{
		snapshot.ReadNextResHandle(m_ahEffect[i]);
		snapshot.ReadNextField(m_ayEffectSequence[i]);
	}

	Validate();
	
	return true;
}


/*====================
  CEntitySoul::GetTypeVector
  ====================*/
const vector<SDataField>&	CEntitySoul::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->push_back(SDataField(_T("m_v3Position"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_v3Angles"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_hModel"), FIELD_PUBLIC, TYPE_RESHANDLE));

		// Effects
		for (int i(0); i < 1; ++i)
		{
			s_pvFields->push_back(SDataField(_T("m_ahEffect[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_RESHANDLE));
			s_pvFields->push_back(SDataField(_T("m_ayEffectSequence[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_CHAR));
		}
	}

	return *s_pvFields;
}


/*====================
  CEntitySoul::Spawn
  ====================*/
void	CEntitySoul::Spawn()
{
	m_yStatus = ENTITY_STATUS_ACTIVE;
	m_hModel = INVALID_INDEX;
	m_fInitialSpeed = m_pEntityConfig->GetInitialSpeed();

	if (m_uiTarget != INVALID_INDEX && Game.GetVisualEntity(m_uiTarget))
		m_fInitialSpeed += Distance(Game.GetVisualEntity(m_uiTarget)->GetPosition(), m_v3Position) * m_pEntityConfig->GetInitialSpeedDistanceFactor();
	
	m_v3Velocity = CVec3f(0.0f, 0.0f, m_fInitialSpeed);
	m_uiLastUpdateTime = m_uiSpawnTime = Game.GetGameTime();
	m_v3Origin = m_v3Position;
	m_bStarted = false;

	if (Game.IsClient())
	{
		if (GetMinimapIconPath().empty())
			m_hMinimapIcon = INVALID_RESOURCE;
		else
			m_hMinimapIcon = g_ResourceManager.Register(K2_NEW(global,   CTexture)(GetMinimapIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
	}
}


/*====================
  CEntitySoul::ServerFrame
  ====================*/
bool	CEntitySoul::ServerFrame()
{
	if (Game.IsClient())
		return true;

	uint uiStartTime(m_uiSpawnTime + 1000);

	if (uiStartTime <= Game.GetGameTime())
	{
		if (!m_bStarted)
		{
			m_bStarted = true;
			m_hModel = Game.RegisterModel(m_pEntityConfig->GetModelPath());

			if (!m_pEntityConfig->GetTrailEffectPath().empty())
			{
				SetEffect(0, Game.RegisterEffect(m_pEntityConfig->GetTrailEffectPath()));
				IncEffectSequence(0);
			}
		}

		float fTime(MsToSec(Game.GetGameTime() - uiStartTime));
		float fLerpTime(MsToSec(m_pEntityConfig->GetLerpTime()));
		float fLerpAcceleration(2.0f / (fLerpTime * fLerpTime));
		float fDeltaTime(MsToSec(Game.GetGameTime() - m_uiLastUpdateTime));

		CVec3f v3Accel;
		
		if (fTime < fLerpTime * 0.667f)
			v3Accel = CVec3f(0.0f, 0.0f, m_fInitialSpeed) * -1.0f;
		else
			v3Accel = CVec3f(0.0f, 0.0f, m_fInitialSpeed) * 2.5f;

		m_v3Origin += m_v3Velocity * fDeltaTime + v3Accel * (0.5f * fDeltaTime * fDeltaTime);
		m_v3Velocity += v3Accel * fDeltaTime;

		if (m_uiTarget != INVALID_INDEX && Game.GetVisualEntity(m_uiTarget))
		{			
			float fLerp(0.5f * fTime * fTime * fLerpAcceleration);

			IVisualEntity *pEnt(Game.GetVisualEntity(m_uiTarget));
			m_v3Position = LERP(fLerp, m_v3Origin, pEnt->GetPosition() + pEnt->GetBounds().GetMid());
		}
		else
			m_v3Position = m_v3Origin;
	}

	uint uiEndTime(uiStartTime + m_pEntityConfig->GetLerpTime());

	if (uiEndTime <= Game.GetGameTime())
	{
		// End event
		if (!m_pEntityConfig->GetEndEffectPath().empty())
		{
			CGameEvent evEnd;
			evEnd.SetSourcePosition(m_v3Position);
			evEnd.SetSourceAngles(m_v3Angles);
			evEnd.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetEndEffectPath()));
			Game.AddEvent(evEnd);
		}
		
		IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiTarget));
		if (pPlayer != NULL && pPlayer->GetStatus() == ENTITY_STATUS_ACTIVE)
		{
			if (pPlayer->GetIsHellbourne())
				pPlayer->Heal(g_hellbourneHealthPerSoul, NULL);
			else
			{
				CEntityClientInfo *pClient(Game.GetClientInfo(pPlayer->GetClientID()));
				if (pClient != NULL)
					pClient->AddSoul();
			}
		}
		return false;
	}

	m_uiLastUpdateTime = Game.GetGameTime();
	return true;
}


/*====================
  CEntitySoul::ClientPrecache
  ====================*/
void	CEntitySoul::ClientPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetTrailEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetTrailEffectPath(), RES_EFFECT);

	if (!pConfig->GetEndEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetEndEffectPath(), RES_EFFECT);
}


/*====================
  CEntitySoul::ServerPrecache
  ====================*/
void	CEntitySoul::ServerPrecache(CEntityConfig *pConfig)
{
	IVisualEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTrailEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTrailEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetEndEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetEndEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
