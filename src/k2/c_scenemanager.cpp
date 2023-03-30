// (C)2005 S2 Games
// c_scenemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_scenemanager.h"

#include "c_scenelight.h"
#include "c_vid.h"
#include "c_camera.h"
#include "c_model.h"
#include "c_convexhull.h"
#include "c_orthofrustum.h"
#include "c_sphere.h"
#include "c_draw2d.h"
#include "c_world.h"
#include "intersection.h"
#include "c_particlesystem.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int NUM_LIGHTNING_MODELS(12);
const int NUM_LIGHTNING_TOP_MODELS(4);
const int NUM_TIMES_OF_DAY(4);
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_FLOATF (   scene_nearClip,             100.0f,                         CONEL_DEV);

CVAR_FLOATF(    scene_farClip,              30000.0f,                       CVAR_SAVECONFIG);
CVAR_FLOATF(    scene_entityDrawDistance,   6500.0f,                        CVAR_SAVECONFIG);
CVAR_FLOATF(    scene_foliageDrawDistance,  750.0f,                         CVAR_SAVECONFIG);
CVAR_INTF(      scene_effectQuality,        0,                              CVAR_SAVECONFIG);

CVAR_BOOL(      scene_lockFrustum,      false);
CVAR_BOOL(      scene_noCull,           false);
CVAR_BOOL(      scene_noShadowCull,     false);
CVAR_BOOL(      scene_noCullEffects,    false);
CVAR_BOOL(      scene_noSecondShadowCull,   false);
CVAR_BOOL(      scene_noTerrainOcclusion,   !TERRAIN_OCCLUSION);
CVAR_BOOL(      scene_dynamicLighting,  true);
CVAR_BOOL(      scene_noOcclusion,      false);
CVAR_INTR(      scene_maxOccluders,     6,                                  0,                  0,  32);
CVAR_INT(       scene_maxOccluderList,  32);
CVAR_BOOLF(     scene_drawOccluders,    false,                              CONEL_DEV);

CVAR_FLOATF(    scene_worldFarClip,     10000.0f,                           CVAR_WORLDCONFIG);

CVAR_VEC3F(     scene_bgColor,          CVec3f(0.75f, 0.75f, 0.75f),        CVAR_WORLDCONFIG);
CVAR_VEC4F(     scene_skyColor,         CVec4f(1.0f, 1.0f, 1.0f, 1.0f),     CVAR_WORLDCONFIG);
CVAR_VEC4F(     scene_altSkyColor,      CVec4f(1.0f, 1.0f, 1.0f, 1.0f),     CVAR_WORLDCONFIG);

CVAR_STRINGF(   scene_skySkin,          "blue2",                            CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_skyOffset,        10.0f,                              CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_skyTopOffset,     0.0f,                               CVAR_WORLDCONFIG);
CVAR_STRINGF(   scene_altSkySkin,       "blue2",                            CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_altSkyOffset,     10.0f,                              CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_altSkyTopOffset,  0.0f,                               CVAR_WORLDCONFIG);

CVAR_FLOATF(    scene_windSpeed,        1.8f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_windSpeedTop,     0.9f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_windSpeedAlt,     1.8f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_windSpeedAltTop,  0.9f,                               CVAR_WORLDCONFIG);

CVAR_FLOATF(    scene_sunSize,          2000.0f,                            CVAR_WORLDCONFIG);
CVAR_VEC4F(     scene_sunColor,         CVec4f(1.0f, 1.0f, 1.0f, 1.0f),     CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_sunAltitude,      60.0f,                              CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_sunAzimuth,       0.0f,                               CVAR_WORLDCONFIG);

CVAR_VEC3F(     scene_terrainSunColor,  CVec3f(0.9f, 0.9f, 0.9f),           CVAR_WORLDCONFIG);
CVAR_VEC3F(     scene_terrainAmbientColor,  CVec3f(0.25f, 0.25f, 0.25f),    CVAR_WORLDCONFIG);

CVAR_VEC3F(     scene_entitySunColor,   CVec3f(0.9f, 0.9f, 0.9f),           CVAR_WORLDCONFIG);
CVAR_VEC3F(     scene_entityAmbientColor,   CVec3f(0.25f, 0.25f, 0.25f),    CVAR_WORLDCONFIG);

CVAR_VEC3(      scene_moonPos,          CVec3f(0.5f, 0.5f, -0.70710677f));
CVAR_VEC4F(     scene_moonColor,        CVec4f(1.0f, 1.0f, 1.0f, 1.0f),     CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_moonSize,         50.0f,                              CVAR_WORLDCONFIG);

CVAR_FLOATF(    scene_lightningRed,     1.0f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_lightningGreen,   1.0f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_lightningBlue,    1.0f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_lightningAlpha,   1.0f,                               CVAR_WORLDCONFIG);

CVAR_ARRAY_BOOLF(   scene_lightning,    NUM_LIGHTNING_MODELS,       false,  CVAR_WORLDCONFIG);
CVAR_ARRAY_BOOLF(   scene_lightningTop, NUM_LIGHTNING_TOP_MODELS,   false,  CVAR_WORLDCONFIG);

CVAR_BOOLF(     scene_drawSky,          true,                               CVAR_WORLDCONFIG);
CVAR_BOOLF(     scene_drawAltSky,       false,                              CVAR_WORLDCONFIG);
CVAR_BOOLF(     scene_drawSun,          true,                               CVAR_WORLDCONFIG);
CVAR_BOOLF(     scene_drawMoon,         false,                              CVAR_WORLDCONFIG);
CVAR_BOOLF(     scene_drawlightning,    false,                              CVAR_WORLDCONFIG);

CVAR_BOOLF(     scene_drawSkybox,       false,                              CVAR_WORLDCONFIG);
CVAR_STRINGF(   scene_skyboxMaterial,   "/world/sky/desert/desert.material",    CVAR_WORLDCONFIG);

CVAR_FLOATF(    scene_brightMin,        1.0f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_brightMax,        2.0f,                               CVAR_WORLDCONFIG);
CVAR_FLOATF(    scene_brightScale,      1.0f,                               CVAR_WORLDCONFIG);

//
// Time-of-day settings
//

CVAR_ARRAY_VEC3F(   tod_terrainSunColor,        NUM_TIMES_OF_DAY,   CVec3f(0.9f, 0.9f, 0.9f),       CVAR_WORLDCONFIG);
CVAR_ARRAY_VEC3F(   tod_terrainAmbientColor,    NUM_TIMES_OF_DAY,   CVec3f(0.25f, 0.25f, 0.25f),    CVAR_WORLDCONFIG);
CVAR_ARRAY_VEC3F(   tod_entitySunColor,         NUM_TIMES_OF_DAY,   CVec3f(0.9f, 0.9f, 0.9f),       CVAR_WORLDCONFIG);
CVAR_ARRAY_VEC3F(   tod_entityAmbientColor,     NUM_TIMES_OF_DAY,   CVec3f(0.25f, 0.25f, 0.25f),    CVAR_WORLDCONFIG);
CVAR_ARRAY_FLOATF(  tod_brightMin,              NUM_TIMES_OF_DAY,   1.0f,                           CVAR_WORLDCONFIG);
CVAR_ARRAY_FLOATF(  tod_brightMax,              NUM_TIMES_OF_DAY,   2.0f,                           CVAR_WORLDCONFIG);
CVAR_ARRAY_FLOATF(  tod_brightScale,            NUM_TIMES_OF_DAY,   1.0f,                           CVAR_WORLDCONFIG);

CSceneManager *g_pSceneManager(CSceneManager::GetInstance());

SINGLETON_INIT(CSceneManager)
//=============================================================================


/*====================
  EntityListSort

  Return whether first element should be draw before the second
  ====================*/
static CVec2f s_v2CameraForward;

static bool EntityListSort(SSceneEntityEntry *a, SSceneEntityEntry *b)
{
#if 1
    float fDotA(DotProduct(a->cEntity.GetPosition().xy(), s_v2CameraForward));
    float fDotB(DotProduct(b->cEntity.GetPosition().xy(), s_v2CameraForward));

    if (fDotA < fDotB)
        return true;
    else if (fDotA > fDotB)
        return false;

    return a < b;
#else
    float fDotA(DotProduct(a->cEntity.GetPosition(), s_v3CameraForward));
    float fDotB(DotProduct(b->cEntity.GetPosition(), s_v3CameraForward));

    return fDotA < fDotB;
#endif
}


/*====================
  CSceneManager::~CSceneManager
  ====================*/
CSceneManager::~CSceneManager()
{
}


/*====================
  CSceneManager::CSceneManager
  ====================*/
CSceneManager::CSceneManager() :
m_fSkyAngle(0.0f),
m_fSkyTopAngle(0.0f),
m_fAltSkyAngle(0.0f),
m_fAltSkyTopAngle(0.0f),
m_hSkyModel(INVALID_RESOURCE),
m_hSkyTopModel(INVALID_RESOURCE),
m_hSunMaterial(INVALID_RESOURCE)
{
}


/*====================
  CSceneManager::AABBInFrustum
  ====================*/
bool    CSceneManager::AABBInFrustum(const CBBoxf &bbBox, bool bIgnoreNearFar)
{
    int iNumPlanes(bIgnoreNearFar ? 4 : 5);

    for (int n(0); n < iNumPlanes; ++n)
    {
        if (M_AABBOnPlaneSide(bbBox, m_FrustumPlanes[n]) == PLANE_NEGATIVE)
            return false;
    }

    return true;
}


/*====================
  CSceneManager::AABBIsVisible

  determines if an AABB is inside the camera view and not occluded
  ====================*/
bool    CSceneManager::AABBIsVisible(const CBBoxf &bbBox)
{
    if (!AABBInFrustum(bbBox, false))
    {
        ++m_iNumOutsideFrustum;
        return false;
    }

    // Test occlusion
    for (OccluderVolumeList::iterator it(m_lOccluderVolumes.begin()); it != m_lOccluderVolumes.end(); ++it)
    {
        size_t z;
        for (z = 0; z < it->zNumPlanes; ++z)
        {
            if (M_AABBOnPlaneSide(bbBox, it->planes[z]) != PLANE_POSITIVE)  // if the AABB isn't completely inside the plane
                break;
        }
        if (z == it->zNumPlanes)
        {
            // AABB is occluded
            ++m_iNumOccluded;
            return false;
        }
    }

    return true;
}

bool CSceneManager::PointIsVisible(const CVec3f &v3pos)
{
    return PointInFrustum(v3pos, true);
}


/*====================
  CSceneManager::OBBInFrustum
  ====================*/
bool    CSceneManager::OBBInFrustum(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis, bool bIgnoreNearFar)
{
    int iNumPlanes(bIgnoreNearFar ? 4 : 5);

    for (int n(0); n < iNumPlanes; ++n)
    {
        if (M_OBBOnPlaneSide(bb, v3Pos, axis, m_FrustumPlanes[n]) == PLANE_NEGATIVE)
            return false;
    }

    return true;
}


/*====================
  CSceneManager::OBBIsVisible
  ====================*/
bool    CSceneManager::OBBIsVisible(const CBBoxf &bb, const CVec3f &v3Pos, const CAxis &axis)
{
    if (!OBBInFrustum(bb, v3Pos, axis, false))
    {
        ++m_iNumOutsideFrustum;
        return false;
    }

    // Test occlusion
    for (OccluderVolumeList::iterator it(m_lOccluderVolumes.begin()); it != m_lOccluderVolumes.end(); ++it)
    {
        size_t z;
        for (z = 0; z < it->zNumPlanes; ++z)
        {
            if (M_OBBOnPlaneSide(bb, v3Pos, axis, it->planes[z]) != PLANE_POSITIVE)     // If the OBB isn't completely inside the plane
                break;
        }
        if (z == it->zNumPlanes)
        {
            // OBB is occluded
            ++m_iNumOccluded;
            return false;
        }
    }

    return true;
}


/*====================
  CSceneManager::PointInFrustum
  ====================*/
bool    CSceneManager::PointInFrustum(const CVec3f &v3Point, bool ignoreNearFar)
{
    int numplanes = ignoreNearFar ? 4 : 5;

    for (int n = 0; n < numplanes; ++n)
    {
        if (DotProduct(v3Point, m_FrustumPlanes[n].v3Normal) - m_FrustumPlanes[n].fDist < 0)
            return false;
    }

    return true;
}


/*====================
  CSceneManager::BoxInSelectionRect

  test left, right, top, and bottom planes only
  ====================*/
bool    CSceneManager::BoxInSelectionRect(const CBBoxf &bb)
{
    for (int n(0); n < 4; ++n)
    {
        if (M_AABBOnPlaneSide(bb, m_FrustumPlanes[n]) == PLANE_NEGATIVE)
            return false;
    }

    return true;
}


/*====================
  CSceneManager::SetFrustum
  ====================*/
void    CSceneManager::SetFrustum(const CCamera &camera)
{
    if (scene_lockFrustum)
        return;

    m_Frustum.Update(camera);

    for (uint ui(0); ui < NUM_FRUSTUM_PLANES; ++ui)
        m_FrustumPlanes[ui] = m_Frustum.GetPlane(ui);
}


/*====================
  CSceneManager::AddEntity
  ====================*/
SSceneEntityEntry&  CSceneManager::AddEntity(const CSceneEntity &cSceneEntity, bool bCull)
{
    PROFILE("CSceneManager::AddEntity");

    SSceneEntityEntry *pNewEntry(K2_NEW(ctx_Renderer, SSceneEntityEntry)(cSceneEntity));

    if (!(pNewEntry->cEntity.flags & SCENEENT_USE_AXIS))
        pNewEntry->cEntity.axis.Set(pNewEntry->cEntity.angle);

    ++m_iNumEntities;

    SSceneEntityEntry &cEntry(*pNewEntry);

    if (bCull)
    {
        PROFILE("Cull");

        CSceneEntity &cEntity(cEntry.cEntity);
        
        if (cEntity.objtype == OBJTYPE_BILLBOARD ||
            cEntity.objtype == OBJTYPE_BEAM ||
            cEntity.objtype == OBJTYPE_GROUNDSPRITE)
        {
            m_lEntities.push_back(pNewEntry);

            cEntry.bCull = false;
            cEntry.bCullShadow = true;
            return cEntry;
        }

        if (scene_noCull ||
            cEntity.flags & SCENEENT_NEVER_CULL)
        {
            m_lEntities.push_back(pNewEntry);

            cEntry.bCull = false;
            cEntry.bCullShadow = false;
            return cEntry;
        }

        if (cEntity.flags & SCENEENT_USE_BOUNDS)
        {
            const CBBoxf &bbBounds(cEntity.bounds);

            if (AABBIsVisible(bbBounds))
            {
                cEntry.bCull = false;
                ++m_iNumRenderedObjects;

                if (scene_noSecondShadowCull)
                {
                    cEntry.bCullShadow = cEntry.bCull;
                }
                else
                {
                    float fFraction(1.0f);

                    if (!scene_noTerrainOcclusion)
                    {
                        CWorld *pWorld(m_Camera.GetWorld());

                        if (pWorld)
                        {
                            CWorldTree &cWorldTree(pWorld->GetWorldTree());

                            if (!cWorldTree.TestBoundsVisibilty(m_Camera.GetOrigin(), bbBounds))
                            {
                                cEntry.bCull = true;
                                cEntry.bCullShadow = true;
                            }
                            else
                            {
                                cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbBounds, m_cSceneHull, fFraction);
                            }
                        }
                        else
                        {
                            cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbBounds, m_cSceneHull, fFraction);
                        }
                    }
                    else
                    {
                        cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbBounds, m_cSceneHull, fFraction);
                    }
                }
            }
            else
            {
                cEntry.bCull = true;
                ++m_iNumCulledObjects;

                if (scene_noSecondShadowCull)
                {
                    cEntry.bCullShadow = cEntry.bCull;
                }
                else
                {
                    float fFraction(1.0f);

                    cEntry.bCullShadow = !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbBounds, m_cSceneHull, fFraction);
                }
            }
        }
        else
        {
            CModel *pModel(g_ResourceManager.GetModel(cEntity.hRes));

            if (!pModel)
            {
                cEntry.bCull = true;
                cEntry.bCullShadow = true;
                return cEntry;
            }

            CBBoxf bbModel(pModel->GetBounds());
            if (cEntity.scale != 1.0f)
                bbModel *= cEntity.scale;

            float fFraction(1.0f);

            if (OBBIsVisible(bbModel, cEntity.GetPosition(), cEntity.axis))
            {
                cEntry.bCull = false;
                ++m_iNumRenderedObjects;

                bbModel.Transform(cEntity.GetPosition(), cEntity.axis, 1.0f);

                if (!scene_noTerrainOcclusion)
                {
                    CWorld *pWorld(m_Camera.GetWorld());

                    if (pWorld)
                    {
                        CWorldTree &cWorldTree(pWorld->GetWorldTree());

                        if (!cWorldTree.TestBoundsVisibilty(m_Camera.GetOrigin(), bbModel))
                        {
                            cEntry.bCull = true;
                            cEntry.bCullShadow = true;
                        }
                        else
                        {
                            cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbModel, m_cSceneHull, fFraction);
                        }
                    }
                    else
                    {
                        cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbModel, m_cSceneHull, fFraction);
                    }
                }
                else
                {
                    cEntry.bCullShadow = m_bShadowFalloff && !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbModel, m_cSceneHull, fFraction);
                }
            }
            else
            {
                cEntry.bCull = true;
                ++m_iNumCulledObjects;

                bbModel.Transform(cEntity.GetPosition(), cEntity.axis, 1.0f);

                cEntry.bCullShadow = !I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), m_v3SunPos * -100000.0f, bbModel, m_cSceneHull, fFraction);
            }
        }
    }

    if (!cEntry.bCull || !cEntry.bCullShadow)
        m_lEntities.push_back(pNewEntry);

    return cEntry;
}


/*====================
  CSceneManager::AddSkyEntity

  Sky objects are always rendered before any other object, and in the order
  they are passed
  ====================*/
void    CSceneManager::AddSkyEntity(const CSceneEntity &cSceneEntity)
{
    PROFILE("CSceneManager::AddSkyEntity");

    SSceneEntityEntry *pNewEntry(K2_NEW(ctx_Renderer,  SSceneEntityEntry)(cSceneEntity));

    if (!(pNewEntry->cEntity.flags & SCENEENT_USE_AXIS))
        pNewEntry->cEntity.axis.Set(pNewEntry->cEntity.angle);

    pNewEntry->cEntity.flags |= SCENEENT_SKYBOX | SCENEENT_NO_ZWRITE | SCENEENT_NEVER_CULL;
    pNewEntry->bCull = false;
    pNewEntry->bCullShadow = false;

    m_lSkyEntities.push_back(pNewEntry);
    ++m_iNumEntities;
}


/*====================
  CSceneManager::AddPoly
  ====================*/
void    CSceneManager::AddPoly(uint zNumVerts, SSceneFaceVert *verts, ResHandle hMaterial, int flags)
{
    SSceneFaceEntry newFaceEntry;

    newFaceEntry.verts = K2_NEW_ARRAY(ctx_Renderer, SSceneFaceVert, zNumVerts);
    if (newFaceEntry.verts == NULL)
    {
        Console.Warn << _T("Failed to allocate memmory in CSceneManager::AddPoly()") << newl;
        return;
    }

    MemManager.Copy(newFaceEntry.verts, verts, zNumVerts * sizeof(SSceneFaceVert));
    newFaceEntry.zNumVerts = zNumVerts;
    newFaceEntry.hMaterial = hMaterial;
    newFaceEntry.flags = flags;
    newFaceEntry.cull = false;

    m_lFaces.push_front(newFaceEntry);
}


/*====================
  CSceneManager::AddOccluder
  ====================*/
void    CSceneManager::AddOccluder(const COccluder &occluder)
{
    m_lOccluders.push_front(occluder);
    m_lOccluders.front().SetCulling(false);
}


/*====================
  CSceneManager::AddLight
  ====================*/
void    CSceneManager::AddLight(const CSceneLight &cLight)
{
    if (cLight.GetFalloffStart() < 0.0f || cLight.GetFalloffEnd() <= 0.0f)
        return;

    m_lLights.push_back(K2_NEW(ctx_Renderer,  SSceneLightEntry)(cLight));
}


/*====================
  CSceneManager::AddParticleSystem
  ====================*/
void    CSceneManager::AddParticleSystem(const CParticleSystem *pParticleSystem, bool bCull)
{
    SSceneParticleSystemEntry *pNewEntry(K2_NEW(ctx_Renderer,  SSceneParticleSystemEntry)(pParticleSystem));

    m_lParticleSystems.push_back(pNewEntry);
    //++m_iNumParticleSystems;

    SSceneParticleSystemEntry &cEntry(*m_lParticleSystems.back());

    if (!scene_noCullEffects && bCull)
    {
        PROFILE("Cull");

        const CBBoxf &bbBounds(pParticleSystem->GetBounds());

        if (AABBIsVisible(bbBounds))
        {
            cEntry.bCull = false;
            //++m_iNumRenderedParticleSystems;

#if 0
            CWorld *pWorld(m_Camera.GetWorld());

            if (!scene_noTerrainOcclusion)
            {           
                if (pWorld)
                {
                    CWorldTree &cWorldTree(pWorld->GetWorldTree());

                    if (!cWorldTree.TestBoundsVisibilty(m_Camera.GetOrigin(), bbBounds))
                    {
                        cEntry.bCull = true;
                    }
                }
            }
#endif
        }
        else
        {
            cEntry.bCull = true;
            //++m_iNumCulledParticleSystems;
        }
    }

    //return cEntry;
}


/*====================
  CSceneManager::AddModifier
  ====================*/
void    CSceneManager::AddModifier(const CSceneEntityModifier *pModifier, uint uiSourceEntityIndex, uint uiTargetEntityIndex)
{
    SSceneModifierEntry *pNewEntry(K2_NEW(ctx_Renderer,  SSceneModifierEntry)(pModifier));

    if (uiSourceEntityIndex != INVALID_INDEX)
        m_mapModifiers[uiSourceEntityIndex] = pNewEntry;
    if (uiTargetEntityIndex != INVALID_INDEX)
        m_mapModifiers[uiTargetEntityIndex] = pNewEntry;
}


/*====================
  CSceneManager::GetModifiers
  ====================*/
SSceneModifierEntry*    CSceneManager::GetModifiers(uint uiEntityIndex)
{
    SceneEntityModifierMap::iterator itFind(m_mapModifiers.find(uiEntityIndex));
    if (itFind != m_mapModifiers.end())
        return itFind->second;
    else
        return NULL;
}


/*====================
  CSceneManager::CullEntities
  ====================*/
void    CSceneManager::CullEntities()
{
    PROFILE("CSceneManager::CullEntities");

    for (SceneEntityList::iterator it(m_lEntities.begin()); it != m_lEntities.end(); ++it)
    {
        SSceneEntityEntry &cEntry(**it);
        CSceneEntity &cEntity(cEntry.cEntity);

        if (scene_noCull ||
            cEntity.objtype == OBJTYPE_BILLBOARD ||
            cEntity.objtype == OBJTYPE_BEAM ||
            cEntity.objtype == OBJTYPE_GROUNDSPRITE ||
            (cEntity.flags & SCENEENT_NEVER_CULL))
        {
            cEntry.bCull = false;
            continue;
        }

        CModel *pModel(g_ResourceManager.GetModel(cEntity.hRes));

        if (!pModel)
        {
            cEntry.bCull = true;
            continue;
        }

        CBBoxf bbModel(pModel->GetBounds());
        if (cEntity.scale != 1.0f)
            bbModel *= cEntity.scale;

        if (OBBIsVisible(bbModel, cEntity.GetPosition(), cEntity.axis))
        {
            PROFILE("Rendered Objects");
            cEntry.bCull = false;
            ++m_iNumRenderedObjects;

            if (scene_noTerrainOcclusion)
                continue;

            bbModel.Transform(cEntity.GetPosition(), cEntity.axis, 1.0f);

            CWorld *pWorld(m_Camera.GetWorld());
            if (pWorld)
            {
                CWorldTree &cWorldTree(pWorld->GetWorldTree());

                if (!cWorldTree.TestBoundsVisibilty(m_Camera.GetOrigin(), bbModel))
                {
                    cEntry.bCull = true;
                    cEntry.bCullShadow = true;
                }
            }
        }
        else
        {
            cEntry.bCull = true;
            ++m_iNumCulledObjects;
        }
    }
}


/*====================
  CSceneManager::CullEntitiesShadow
  ====================*/
void    CSceneManager::CullEntitiesShadow(const CConvexPolyhedron &cScene)
{
    PROFILE("CSceneManager::CullEntitiesShadow");

    CVec3f v3Start(0.0f, 0.0f, 0.0f);
    CVec3f v3End(m_v3SunPos * -100000.0f);

    SceneEntityList::iterator itEnd(m_lEntities.end());
    for (SceneEntityList::iterator it(m_lEntities.begin()); it != itEnd; ++it)
    {
        SSceneEntityEntry &cEntry(**it);

        if (cEntry.bCullShadow)
            continue;

        CSceneEntity &cEntity(cEntry.cEntity);

        if (scene_noShadowCull ||
            cEntity.flags & SCENEENT_NEVER_CULL)
        {
            cEntry.bCullShadow = false;
            continue;
        }

        if (cEntity.objtype == OBJTYPE_BILLBOARD ||
            cEntity.objtype == OBJTYPE_BEAM ||
            cEntity.objtype == OBJTYPE_GROUNDSPRITE)
        {
            cEntry.bCullShadow = true;
            continue;
        }

        if (!m_bShadowFalloff && !cEntry.bCull)
        {
            cEntry.bCullShadow = false;
            continue;
        }

        if (cEntity.flags & SCENEENT_USE_BOUNDS)
        {
            float fFraction(1.0f);
            cEntry.bCullShadow = !I_MovingBoundsSurfaceIntersect(v3Start, v3End, cEntity.bounds, cScene, fFraction);
        }
        else
        {
            CModel *pModel(g_ResourceManager.GetModel(cEntity.hRes));

            if (!pModel)
            {
                cEntry.bCullShadow = true;
                continue;
            }

            CBBoxf bbModel(pModel->GetBounds());
            bbModel.Transform(cEntity.GetPosition(), cEntity.axis, cEntity.scale);

            float fFraction(1.0f);
            cEntry.bCullShadow = !I_MovingBoundsSurfaceIntersect(v3Start, v3End, bbModel, cScene, fFraction);
        }
    }
}


/*====================
  CSceneManager::CullLights
  ====================*/
void    CSceneManager::CullLights()
{
    PROFILE("CSceneManager::CullLights");

    // All lights are culled by default
    if (!scene_dynamicLighting)
        return;

    for (SceneLightList::iterator it(m_lLights.begin()); it != m_lLights.end(); ++it)
    {
        SSceneLightEntry &cEntry(**it);
        CSceneLight &cLight(cEntry.cLight);

        if (m_Frustum.Touches(CSphere(cLight.GetPosition(), cLight.GetFalloffEnd())))
            cEntry.bCull = false;
    }
}


/*====================
  CSceneManager::BuildVolume

  create a polyhedron (uncapped) represented as planes from a point and a polygon
  assumes numpoints is >= 3
  planes[] must have capacity for numpoints + 1 entries
  the poly array must be coplanar, convex, and specified in counterclockwise order
  ====================*/
void    CSceneManager::BuildVolume(const CVec3f &v3Point, const COccluder &occluder, CPlane planes[])
{
    int iLastPoint(occluder.GetNumPoints());

    planes[iLastPoint].CalcPlaneNormalized(occluder.GetPoint(0), occluder.GetPoint(1), occluder.GetPoint(2));
    CVec3f  v3Forward(occluder.GetPoint(0) - v3Point);

    // Determine corner winding
    if (M_DotProduct(planes[iLastPoint].v3Normal, v3Forward) > 0.0f)
    {
        for (uint ui(0); ui < occluder.GetNumPoints(); ++ui)
            planes[ui].CalcPlaneNormalized(v3Point, occluder.GetPoint(ui), occluder.GetPoint((ui + 1) % occluder.GetNumPoints()));
    }
    else
    {
        planes[iLastPoint].v3Normal = -planes[iLastPoint].v3Normal;
        planes[iLastPoint].fDist = -planes[iLastPoint].fDist;

        for (uint ui(0); ui < occluder.GetNumPoints(); ++ui)
            planes[ui].CalcPlaneNormalized(occluder.GetPoint((ui + 1) % occluder.GetNumPoints()), occluder.GetPoint(ui), v3Point);
    }
}


/*====================
  CSceneManager::DrawOccluders
  ====================*/
void    CSceneManager::DrawOccluders()
{
    for (OccluderVolumeList::iterator it(m_lOccluderVolumes.begin()); it != m_lOccluderVolumes.end(); ++it)
    {
        COccluder *occluder(it->pOccluder);
        SSceneFaceVert poly[64];

        MemManager.Set(poly, 0, sizeof(poly));

        for (uint ui(0); ui < occluder->GetNumPoints(); ++ui)
        {
            M_CopyVec3(vec3_cast(occluder->GetPoint(ui)), poly[ui].vtx);
            SET_VEC4(poly[ui].col, 255, 255, 255, 255);
        }
        //AddPoly(occluder->GetNumPoints(), poly, m_hOccluderMaterial, POLY_DOUBLESIDED | POLY_WIREFRAME | POLY_NO_DEPTH_TEST);
    }
}

/*====================
  CSceneManager::BuildOccluderList
  ====================*/
void    CSceneManager::BuildOccluderList(const CCamera &camera)
{
    if (camera.HasFlags(CAM_NO_OCCLUDERS) || scene_noOcclusion)
        return;

    // Step through each possible occluder
    for (OccluderList::iterator it(m_lOccluders.begin()); it != m_lOccluders.end(); ++it)
    {
        if (it->GetNumPoints() < 3)
            continue;

        // Determine if the occluder polygon is inside the camera frustum
        for (uint uiPoint(0); uiPoint < it->GetNumPoints(); ++uiPoint)
        {
            CVec3f v3A(it->GetPoint(uiPoint));
            CVec3f v3B(it->GetPoint((uiPoint + 1) % it->GetNumPoints()));

            // Check each frustum plane
            int i;
            for (i = 0; i < 6; ++i)
            {
                float fDotA(DotProduct(v3A, m_FrustumPlanes[i].v3Normal) - m_FrustumPlanes[i].fDist);
                float fDotB(DotProduct(v3B, m_FrustumPlanes[i].v3Normal) - m_FrustumPlanes[i].fDist);

                // Does polygon intersects camera frustum?
                if (fDotA < 0.0f && fDotB < 0.0f)
                    continue;

                // Compute a distance approximation to decide if we should use this occluder
                float fDist(0.0f);
                for (uint ui(0); ui < it->GetNumPoints(); ++ui)
                    fDist += M_GetDistanceSq(camera.GetOrigin(), it->GetPoint(ui));
                fDist /= it->GetNumPoints();

                // Add an occluder volume to the list
                SOccluderVolume newOccluderVolume;
                newOccluderVolume.influence = 1.0f / fDist;
                BuildVolume(camera.GetOrigin(), *it, newOccluderVolume.planes);
                newOccluderVolume.zNumPlanes = it->GetNumPoints() + 1;
                newOccluderVolume.pOccluder = &(*it);
                m_lOccluderVolumes.push_back(newOccluderVolume);
                break;
            }

            // If the loop didn't finish, this occluder has been added and can stop being tested
            if (i < 6)
                break;
        }

        if (m_lOccluderVolumes.size() >= uint(scene_maxOccluderList))
            break;
    }

    // Sort the occluders
    //sort(m_lOccluderVolumes.begin(), m_lOccluderVolumes.end());
}


/*====================
  CSceneManager::UpdateSunPosition
  ====================*/
void    CSceneManager::UpdateSunPosition()
{
    float phi = DEG2RAD(scene_sunAltitude - 90.0f);
    float theta = DEG2RAD(-scene_sunAzimuth);

    m_v3SunPos.Set(sin(phi) * cos(theta), sin(phi) * sin(theta), cos(phi));
    m_v3SunPos.Normalize();
}


/*====================
  CSceneManager::DrawSun
  ====================*/
void    CSceneManager::DrawSun(const CCamera &cam)
{
    if (!scene_drawSun)
        return;

    if (m_hSunMaterial == INVALID_RESOURCE)
        m_hSunMaterial = g_ResourceManager.Register(_T("/world/sky/sun/sun1.material"), RES_MATERIAL);

    CVec4f vecSunColor(scene_sunColor);

    CSceneEntity sun;
    sun.hRes = m_hSunMaterial;
    sun.objtype = OBJTYPE_BILLBOARD;
    sun.width = 1000.0f;
    sun.height = 1000.0f;
    sun.scale = 1.0f;
    sun.flags = SCENEENT_BILLBOARD_ALL_AXES | SCENEENT_SOLID_COLOR;     // always rotate towards the viewer
    sun.color = vecSunColor;

    sun.angle.Set(0.0f, 0.0f, 0.0f);
    CVec3f v3SunPos(cam.GetOrigin() + (m_v3SunPos * scene_sunSize));
    sun.SetPosition(v3SunPos);

    AddSkyEntity(sun);
}


/*====================
  CSceneManager::DrawMoon
  ====================*/
void    CSceneManager::DrawMoon(const CCamera &cam)
{
    if (!scene_drawSky)
        return;
    if (!scene_drawMoon)
        return;

    //tstring sMoonModel(RES_ENVIRONMENT_PATH + _T("moon/") + tl_moonmodel + _T(".model"));
    //tstring sMoonSkin(RES_ENVIRONMENT_PATH + _T("moon/") + tl_moonskin + _T(".skin"));

    CSceneEntity moon;
    //moon.model = Res_LoadModel(sMoonModel);
    //moon.skin = Res_LoadSkin(moon.model, sMoonSkin);
    moon.objtype = OBJTYPE_MODEL;
    moon.scale = 1;
    moon.flags = 0;
    CVec4f vecMoonColor(scene_moonColor);
    moon.color = vecMoonColor;

    moon.angle.Set(0, 0, 0);
    CVec3f v3MoonPos(cam.GetOrigin() + (CVec3f(scene_moonPos) * (1.0f / scene_moonSize)));
    moon.SetPosition(v3MoonPos);

    AddSkyEntity(moon);
}


/*====================
  CSceneManager::DrawLightning
  ====================*/
void    CSceneManager::DrawLightning(const CCamera &cam)
{
    if (!scene_drawlightning)
        return;

    // Lightning top models
    for (int n(0); n < NUM_LIGHTNING_TOP_MODELS; ++n)
    {
        if (!scene_lightningTop[n])
            continue;

        CSceneEntity lightning_top;
        CVec4f vecLightningColor(scene_lightningRed, scene_lightningGreen, scene_lightningBlue, scene_lightningAlpha);

        lightning_top.hRes = g_ResourceManager.Register(_T("/world/sky/lightning_top") + XtoA(n + 1) + _T(".mdf"), RES_MODEL);
        lightning_top.hSkin = g_ResourceManager.GetSkin(lightning_top.hRes, _T("thunderstorm"));
        lightning_top.objtype = OBJTYPE_MODEL;
        lightning_top.scale = 1;
        lightning_top.flags = SCENEENT_SOLID_COLOR;
        lightning_top.color = vecLightningColor;

        lightning_top.angle.Set(0, 0, m_fSkyTopAngle);
        lightning_top.SetPosition(cam.GetOrigin());
        lightning_top.Translate(0.0f, 0.0f, scene_skyOffset + scene_skyTopOffset);

        SceneManager.AddSkyEntity(lightning_top);
    }

    // Lightning models
    for (int n(0); n < NUM_LIGHTNING_MODELS; ++n)
    {
        if (!scene_lightning[n])
            continue;

        CSceneEntity lightning;
        CVec4f vecLightningColor(scene_lightningRed, scene_lightningGreen, scene_lightningBlue, scene_lightningAlpha);

        lightning.hRes = g_ResourceManager.Register(_T("/world/sky/lightning") + XtoA(n + 1) + _T(".mdf"), RES_MODEL);
        lightning.hSkin = g_ResourceManager.GetSkin(lightning.hRes, _T("thunderstorm"));
        lightning.objtype = OBJTYPE_MODEL;
        lightning.scale = 1;
        lightning.flags = SCENEENT_SOLID_COLOR;
        lightning.color = vecLightningColor;

        lightning.angle.Set(0, 0, m_fSkyAngle);
        lightning.SetPosition(cam.GetOrigin());
        lightning.Translate(0.0f, 0.0f, scene_skyOffset);

        SceneManager.AddSkyEntity(lightning);
    }
}


/*====================
  CSceneManager::DrawSky
  ====================*/
void    CSceneManager::DrawSky(const CCamera &cam, float fFrameTime)
{
    m_fSkyAngle += scene_windSpeed * fFrameTime;
    if (m_fSkyAngle > 360.0f)
        m_fSkyAngle -= 360.0f;

    m_fSkyTopAngle += scene_windSpeedTop * fFrameTime;
    if (m_fSkyTopAngle > 360.0f)
        m_fSkyTopAngle -= 360.0f;

    m_fAltSkyAngle += scene_windSpeedAlt * fFrameTime;
    if (m_fAltSkyAngle > 360.0f)
        m_fAltSkyAngle -= 360.0f;

    m_fAltSkyTopAngle += scene_windSpeedAltTop * fFrameTime;
    if (m_fAltSkyTopAngle > 360.0f)
        m_fAltSkyTopAngle -= 360.0f;

    if (!scene_drawSky)
        return;

    if (m_hSkyModel == INVALID_RESOURCE)
        m_hSkyModel = g_ResourceManager.Register(_T("/world/sky/sky.mdf"), RES_MODEL);

    if (m_hSkyTopModel == INVALID_RESOURCE)
        m_hSkyTopModel = g_ResourceManager.Register(_T("/world/sky/sky_top.mdf"), RES_MODEL);

    // Alt Sky
    if (scene_drawAltSky)
    {
        CSceneEntity altsky, altskytop;

        altsky.hRes = m_hSkyModel;
        altsky.hSkin = g_ResourceManager.GetSkin(altsky.hRes, scene_altSkySkin);
        altsky.objtype = OBJTYPE_MODEL;
        altsky.scale = 1;
        altsky.flags = SCENEENT_SOLID_COLOR;
        altsky.color = scene_altSkyColor;
        altsky.angle.Set(0, 0, m_fAltSkyAngle);
        altsky.SetPosition(cam.GetOrigin());
        altsky.Translate(0.0f, 0.0f, -scene_altSkyOffset);

        altskytop.hRes = m_hSkyTopModel;
        altskytop.hSkin = g_ResourceManager.GetSkin(altsky.hRes, scene_altSkySkin);
        altskytop.objtype = OBJTYPE_MODEL;
        altskytop.scale = 1;
        altskytop.flags = SCENEENT_SOLID_COLOR;
        altskytop.color = scene_altSkyColor;
        altskytop.angle.Set(0, 0, m_fAltSkyTopAngle);
        altskytop.SetPosition(cam.GetOrigin());
        altskytop.Translate(0.0f, 0.0f, -scene_altSkyTopOffset);

        AddSkyEntity(altskytop);
        AddSkyEntity(altsky);
    }

    // Main sky
    CSceneEntity sky, skytop;

    sky.Clear();
    sky.scale = 1.0f;
    sky.angle.Set(0, 0, m_fSkyAngle);
    sky.SetPosition(cam.GetOrigin());
    sky.Translate(0.0f, 0.0f, scene_skyOffset);
    sky.hRes = m_hSkyModel;
    sky.hSkin = g_ResourceManager.GetSkin(sky.hRes, scene_skySkin);
    sky.objtype = OBJTYPE_MODEL;
    sky.flags = SCENEENT_SOLID_COLOR | SCENEENT_NO_LIGHTING;
    sky.color = scene_skyColor;

    skytop.Clear();
    skytop.scale = 1.0f;
    skytop.angle.Set(0, 0, m_fSkyTopAngle);
    skytop.SetPosition(cam.GetOrigin());
    skytop.Translate(0.0f, 0.0f, scene_skyOffset + scene_skyTopOffset);
    skytop.hRes = m_hSkyTopModel;
    skytop.hSkin = g_ResourceManager.GetSkin(sky.hRes, scene_skySkin);
    skytop.objtype = OBJTYPE_MODEL;
    skytop.flags = SCENEENT_SOLID_COLOR | SCENEENT_NO_LIGHTING;
    skytop.color = scene_skyColor;

    AddSkyEntity(sky);
    AddSkyEntity(skytop);
    
    DrawMoon(cam);
    DrawSun(cam);
    DrawLightning(cam);
}


/*====================
  CSceneManager::PrepCamera
  ====================*/
void    CSceneManager::PrepCamera(const CCamera &camera)
{
    PROFILE("PrepCamera");
    m_Camera = camera;

    m_fTime = Host.GetTime() * SEC_PER_MS;

    if (m_Camera.GetWidth() <= 0.0f || m_Camera.GetHeight() <= 0.0f)
    {
        m_Camera.SetX(0.0f);
        m_Camera.SetY(0.0f);
        m_Camera.SetWidth(float(Vid.GetScreenW()));
        m_Camera.SetHeight(float(Vid.GetScreenH()));
    }

    if (m_Camera.GetX() < 0.0f || m_Camera.GetY() < 0.0f || m_Camera.GetX() + m_Camera.GetWidth() > Vid.GetScreenW() || m_Camera.GetY() + m_Camera.GetHeight() > Vid.GetScreenH())
    {
        float fOldWidth(m_Camera.GetWidth());
        float fOldHeight(m_Camera.GetHeight());
        float fOldFovX(m_Camera.GetFovX());
        float fOldFovY(m_Camera.GetFovY());

        m_Camera.SetX(MAX(m_Camera.GetX(), 0.0f));
        m_Camera.SetY(MAX(m_Camera.GetY(), 0.0f));
        m_Camera.SetWidth(MIN(m_Camera.GetWidth(), Vid.GetScreenW() - m_Camera.GetX()));
        m_Camera.SetHeight(MIN(m_Camera.GetHeight(), Vid.GetScreenH() - m_Camera.GetY()));

        if (fOldWidth != m_Camera.GetWidth())
        {
            float x(fOldWidth / tan(DEG2RAD(fOldFovX * 0.5f)));
            float a(atan(m_Camera.GetWidth() / x));
            a = RAD2DEG(a) * 0.5f;

            m_Camera.SetFovX(a);
            m_Camera.SetAspect(m_Camera.GetWidth() / m_Camera.GetHeight());
        }

        if (fOldHeight != m_Camera.GetHeight())
        {
            float y(fOldHeight / tan(DEG2RAD(fOldFovY * 0.5f)));
            float a(atan(m_Camera.GetHeight() / y));
            a = RAD2DEG(a) * 0.5f;

            m_Camera.SetFovY(a);
            m_Camera.SetAspect(m_Camera.GetWidth() / m_Camera.GetHeight());
        }
    }

    if (camera.GetZNear())
        m_Camera.SetZNear(camera.GetZNear());
    else
        m_Camera.SetZNear(scene_nearClip);

    if (camera.GetZFar())
        m_Camera.SetZFar(camera.GetZFar());
    else
        m_Camera.SetZFar(MIN(scene_farClip, scene_worldFarClip));

    SetFrustum(m_Camera);

    m_cSceneHull = CConvexPolyhedron(m_Camera.GetOrigin(), m_Camera.GetViewAxis(), m_Camera.GetFovX(), m_Camera.GetFovY(), m_Camera.GetZFar());

    UpdateSunPosition();

    BuildOccluderList(m_Camera);
    if (scene_drawOccluders)
        DrawOccluders();

    m_bShadowFalloff = !m_Camera.HasFlags(CAM_SHADOW_NO_FALLOFF) && ICvar::GetBool(_T("vid_shadowFalloff"));
}


/*====================
  CSceneManager::SceneCull
  ====================*/
void    CSceneManager::SceneCull()
{
    PROFILE("CSceneManager::SceneCull");

    CullEntities();
    CullLights();   
}


/*====================
  CSceneManager::Render
  ====================*/
void    CSceneManager::Render()
{
    PROFILE("CSceneManager::Render");

    Vid.RenderScene(m_Camera);
    ++m_iNumRenderCalls;
}


/*====================
  CSceneManager::ClearBackground
  ====================*/
void    CSceneManager::ClearBackground()
{
    Draw2D.SetColor(scene_bgColor);
    Draw2D.Clear();
}


/*====================
  CSceneManager::Clear

  Clear all objects from memory
  ====================*/
void    CSceneManager::Clear()
{
    PROFILE("CSceneManager::Clear");

    SSceneEntityEntry::s_Pool.Reset();
    SSceneParticleSystemEntry::s_Pool.Reset();
    SSceneModifierEntry::s_Pool.Reset();
    SSceneLightEntry::s_Pool.Reset();

    m_lEntities.clear();
    m_lSkyEntities.clear();
    m_lParticleSystems.clear();
    m_lLights.clear();
    m_mapModifiers.clear();

    for (SceneFaceList::iterator it(m_lFaces.begin()); it != m_lFaces.end(); ++it)
    {
        if (it->zNumVerts > 0)
            K2_DELETE_ARRAY(it->verts);
    }
    m_lFaces.clear();

    m_lOccluders.clear();
    m_lOccluderVolumes.clear();

    Draw2D.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
}


/*====================
  CSceneManager::DrawStats
  ====================*/
void    CSceneManager::DrawStats()
{
}


/*====================
  CSceneManager::GetTerrainSunColor
  ====================*/
CVec3f  CSceneManager::GetTerrainSunColor()
{
    return scene_terrainSunColor;
}


/*====================
  CSceneManager::GetTerrainAmbientColor
  ====================*/
CVec3f  CSceneManager::GetTerrainAmbientColor()
{
    return scene_terrainAmbientColor;
}


/*====================
  CSceneManager::GetEntitySunColor
  ====================*/
CVec3f  CSceneManager::GetEntitySunColor()
{
    return scene_entitySunColor;
}


/*====================
  CSceneManager::GetEntityAmbientColor
  ====================*/
CVec3f  CSceneManager::GetEntityAmbientColor()
{
    return scene_entityAmbientColor;
}


/*====================
  CSceneManager::GetDrawSkybox
  ====================*/
bool    CSceneManager::GetDrawSkybox()
{
    return scene_drawSkybox;
}


/*====================
  CSceneManager::GetSkyboxMaterial
  ====================*/
tstring     CSceneManager::GetSkyboxMaterial()
{
    return scene_skyboxMaterial;
}


/*====================
  CSceneManager::GetSkyColor
  ====================*/
CVec4f  CSceneManager::GetSkyColor()
{
    return scene_skyColor;
}


/*====================
  CSceneManager::GetFoliageDrawDistance
  ====================*/
float   CSceneManager::GetFoliageDrawDistance()
{
    return scene_foliageDrawDistance;
}


/*====================
  CSceneManager::GetEntityDrawDistance
  ====================*/
float   CSceneManager::GetEntityDrawDistance()
{
    return scene_foliageDrawDistance;
}


/*====================
  CSceneManager::GetSceneBgColor
  ====================*/
CVec3f  CSceneManager::GetSceneBgColor()
{
    return scene_bgColor;
}


/*====================
  CSceneManager::SetTimeOfDay
  ====================*/
void    CSceneManager::SetTimeOfDay(float fTimeOfDay)
{
    int iLo(INT_FLOOR(fTimeOfDay) % NUM_TIMES_OF_DAY);
    int iHi((iLo + 1) % NUM_TIMES_OF_DAY);
    float fLerp(FRAC(fTimeOfDay));

    scene_terrainSunColor = LERP<CVec3f>(fLerp, tod_terrainSunColor[iLo], tod_terrainSunColor[iHi]);
    scene_terrainAmbientColor = LERP<CVec3f>(fLerp, tod_terrainAmbientColor[iLo], tod_terrainAmbientColor[iHi]);
    scene_entitySunColor = LERP<CVec3f>(fLerp, tod_entitySunColor[iLo], tod_entitySunColor[iHi]);
    scene_entityAmbientColor = LERP<CVec3f>(fLerp, tod_entityAmbientColor[iLo], tod_entityAmbientColor[iHi]);
    scene_brightMin = LERP<float>(fLerp, tod_brightMin[iLo], tod_brightMin[iHi]);
    scene_brightMax = LERP<float>(fLerp, tod_brightMax[iLo], tod_brightMax[iHi]);
    scene_brightScale = LERP<float>(fLerp, tod_brightScale[iLo], tod_brightScale[iHi]);
}


CPool<SSceneEntityEntry> SSceneEntityEntry::s_Pool(1, uint(-1));

/*====================
  SSceneEntityEntry::operator new
  ====================*/
void*   SSceneEntityEntry::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


CPool<SSceneParticleSystemEntry> SSceneParticleSystemEntry::s_Pool(1, uint(-1));

/*====================
  SSceneParticleSystemEntry::operator new
  ====================*/
void*   SSceneParticleSystemEntry::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


CPool<SSceneLightEntry> SSceneLightEntry::s_Pool(1, uint(-1));

/*====================
  SSceneLightEntry::operator new
  ====================*/
void*   SSceneLightEntry::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


CPool<SSceneModifierEntry> SSceneModifierEntry::s_Pool(1, uint(-1));

/*====================
  SSceneModifierEntry::operator new
  ====================*/
void*   SSceneModifierEntry::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}

