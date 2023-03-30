// (C)2008 S2 Games
// c_propwater.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propwater.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_model.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Water);
//=============================================================================


/*====================
  CPropWater::Spawn
  ====================*/
void    CPropWater::Spawn()
{
    IPropEntity::Spawn();

    m_yStatus = ENTITY_STATUS_ACTIVE;

    if (Game.IsClient())
    {
        m_aAxis.Set(m_v3Angles);

        m_cSceneEntity.Clear();
        m_cSceneEntity.scale = m_fScale;
        m_cSceneEntity.SetPosition(m_v3Position);
        m_cSceneEntity.axis = m_aAxis;
        m_cSceneEntity.objtype = OBJTYPE_MODEL;
        m_cSceneEntity.hRes = GetModel();
        m_cSceneEntity.skeleton = m_pSkeleton;
        m_cSceneEntity.color = WHITE;
        m_cSceneEntity.effectdepth = FAR_AWAY * 0.5f;

        m_cSceneEntity.flags = SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS | SCENEENT_FOG_OF_WAR;

        CModel *pModel(g_ResourceManager.GetModel(GetModel()));
        if (pModel)
        {
            CBBoxf bbBounds(pModel->GetBounds());
            bbBounds.Transform(m_v3Position, m_aAxis, m_fScale);

            m_cSceneEntity.bounds = bbBounds;
            m_cSceneEntity.flags |= SCENEENT_USE_BOUNDS;
        }
    }

    Game.AddWaterMarker(m_v3Position);
}


/*====================
  CPropWater::AddToScene
  ====================*/
bool    CPropWater::AddToScene(const CVec4f &v4Color, int iFlags)
{
    //PROFILE("CPropWater::AddToScene");

    if (GetModel() == INVALID_INDEX)
        return false;

    m_cSceneEntity.color = v4Color;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        m_cSceneEntity.color[A] *= 0.5f;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(m_cSceneEntity));

    if (!cEntry.bCull)
        Game.SetActiveReflection(true);

    if (!cEntry.bCull || !cEntry.bCullShadow)
        UpdateSkeleton(true);
    else
        UpdateSkeleton(false);

    return true;
}


/*====================
  CPropWater::Copy
  ====================*/
void    CPropWater::Copy(const IGameEntity &B)
{
    IPropEntity::Copy(B);

    const CPropWater *pB(static_cast<const CPropWater *>(&B));

    if (!pB)    
        return;

    const CPropWater &C(*pB);

    m_aAxis          = C.m_aAxis;
    m_cSceneEntity   = C.m_cSceneEntity;
}

