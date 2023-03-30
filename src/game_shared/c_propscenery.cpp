// (C)2006 S2 Games
// c_propscenery.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propscenery.h"

#include "../k2/c_scenemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Scenery);
//=============================================================================


/*====================
  CPropScenery::Spawn
  ====================*/
void	CPropScenery::Spawn()
{
	IPropEntity::Spawn();

	m_yStatus = ENTITY_STATUS_ACTIVE;

	if (Game.IsClient())
	{
		m_cSceneEntity.Clear();
		m_cSceneEntity.scale = m_fScale;
		m_cSceneEntity.SetPosition(m_v3Position);
		m_cSceneEntity.axis.Set(m_v3Angles);
		m_cSceneEntity.objtype = OBJTYPE_MODEL;
		m_cSceneEntity.hModel = m_hModel;
		m_cSceneEntity.skeleton = m_pSkeleton;
		m_cSceneEntity.color = WHITE;

		m_cSceneEntity.flags = SCENEOBJ_SOLID_COLOR | SCENEOBJ_USE_AXIS | SCENEOBJ_FOG_OF_WAR;
	}
}


/*====================
  CPropScenery::AddToScene
  ====================*/
bool	CPropScenery::AddToScene(const CVec4f &v4Color, int iFlags)
{
	PROFILE("CPropScenery::AddToScene");

	if (m_hModel == INVALID_INDEX)
		return false;

	m_cSceneEntity.color = WHITE;

	if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
		m_cSceneEntity.color[A] *= 0.5f;

	SSceneEntityEntry &cEntry(SceneManager.AddEntity(m_cSceneEntity));

	if (!cEntry.bCull || !cEntry.bCullShadow)
		UpdateSkeleton(true);
	else
		UpdateSkeleton(false);

	return true;
}


/*====================
  CPropScenery::Copy
  ====================*/
void	CPropScenery::Copy(const IGameEntity &B)
{
	IPropEntity::Copy(B);

	const CPropScenery *pB(static_cast<const CPropScenery *>(&B));

	if (!pB)	
		return;

	const CPropScenery &C(*pB);

	m_cSceneEntity	 = C.m_cSceneEntity;
}

