// (C)2006 S2 Games
// i_light.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_light.h"

#include "../k2/s_traceinfo.h"
#include "../k2/c_world.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_worldlight.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(ILight, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position"), TYPE_V3F, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Color"), TYPE_V3F, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fFalloffStart"), TYPE_FLOAT, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fFalloffEnd"), TYPE_FLOAT, 0, 0));
}
//=============================================================================

/*====================
  ILight::ILight
  ====================*/
ILight::ILight() :
m_v3Color(1.0f, 1.0f, 1.0f),
m_fFalloffStart(0.0f),
m_fFalloffEnd(0.0f)
{
}


/*====================
  ILight::Baseline
  ====================*/
void	ILight::Baseline()
{
	m_v3Position = V3_ZERO;
	m_v3Color = CVec3f(1.0f, 1.0f, 1.0f);
	m_fFalloffStart = 0.0f;
	m_fFalloffEnd = 0.0f;
}


/*====================
  ILight::GetSnapshot
  ====================*/
void	ILight::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	snapshot.WriteField(m_v3Position);
	snapshot.WriteField(m_v3Color);
	snapshot.WriteField(m_fFalloffStart);
	snapshot.WriteField(m_fFalloffEnd);
}


/*====================
  ILight::ReadSnapshot
  ====================*/
bool	ILight::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		snapshot.ReadField(m_v3Position);
		snapshot.ReadField(m_v3Color);
		snapshot.ReadField(m_fFalloffStart);
		snapshot.ReadField(m_fFalloffEnd);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("ILight::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  ILight::Spawn
  ====================*/
void	ILight::Spawn()
{
	try
	{
		IVisualEntity::Spawn();

		if (m_uiWorldIndex == INVALID_INDEX)
			EX_WARN(_T("Light has no world index"));

		CWorldLight *pLight(Game.GetWorldLight(m_uiWorldIndex));
		if (pLight == NULL)
			EX_WARN(_T("World entity not found: #") + XtoA(m_uiWorldIndex));

		m_v3Position = pLight->GetPosition();
		m_v3Color = pLight->GetColor();
		m_fFalloffStart = pLight->GetFalloffStart();
		m_fFalloffEnd = pLight->GetFalloffEnd();
	}
	catch (CException &ex)
	{
		ex.Process(_T("ILight::Spawn() - "));
	}
}


/*====================
  ILight::AddToScene
  ====================*/
bool	ILight::AddToScene(const CVec4f &v4Color, int iFlags)
{
	SceneManager.AddLight(CSceneLight(m_v3Position, m_v3Color, m_fFalloffStart, m_fFalloffEnd));
	return true;
}


/*====================
  ILight::Copy
  ====================*/
void	ILight::Copy(const IGameEntity &B)
{
	const ILight *pB(B.GetAsLight());

	if (!pB)
		return;

	const ILight &C(*pB);

	m_unType =			C.m_unType;
	m_sName =			C.m_sName;
	m_uiIndex =			C.m_uiIndex;
	m_uiWorldIndex =	C.m_uiWorldIndex;

	m_v3Position =		C.m_v3Position;
	m_v3Color =			C.m_v3Color;
	m_fFalloffStart =	C.m_fFalloffStart;
	m_fFalloffEnd =		C.m_fFalloffEnd;
}
