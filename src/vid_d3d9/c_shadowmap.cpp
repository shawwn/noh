// (C)2006 S2 Games
// c_shadowmap.cpp
//
// Shadowmap wrapper
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_shadowmap.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "C_renderlist.h"
#include "d3d9_texture.h"
#include "d3d9_shader.h"
#include "d3d9_scene.h"
#include "d3d9_terrain.h"
#include "d3d9_state.h"
#include "d3d9_util.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CShadowmap  g_Shadowmap;

CVAR_BOOLF  (vid_shadows,                       true,                       CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowDebug,                   false);
CVAR_FLOATR (vid_shadowLeak,                    0.1f,                       CVAR_SAVECONFIG, 0.0f, 1.0f);
CVAR_FLOAT  (vid_shadowDepthBias,               4.0f);
CVAR_FLOAT  (vid_shadowSlopeBias,               2.0f);
CVAR_FLOAT  (vid_shadowNearZ,                   0.0f);
CVAR_FLOAT  (vid_shadowFarZ,                    1.0f);
CVAR_FLOAT  (vid_shadowMaxFov,                  /*179.99f*/175.0f);
CVAR_FLOAT  (vid_shadowMinFov,                  0.025f);
CVAR_FLOAT  (vid_shadowParallelEpsilon,         0.2f);
CVAR_FLOAT  (vid_shadowParallelPullback,        100.0f);
CVAR_FLOAT  (vid_shadowZScale,                  4.0f);
CVAR_BOOL   (vid_shadowUnitCubeClipping,        true);
CVAR_FLOAT  (vid_shadowFrustumSlideback,        500.0f);
CVAR_BOOL   (vid_shadowFrustumInfinite,         false);
CVAR_BOOL   (vid_shadowFrustumCube,             false);
CVAR_FLOAT  (vid_shadowFrustumScale,            1.0f);
CVAR_FLOAT  (vid_shadowMaxLightDistance,        200.0f);
CVAR_BOOL   (vid_shadowBackface,                false);
CVAR_BOOL   (vid_shadowUniform,                 false);
CVAR_FLOATF (vid_shadowDrawDistance,            4000.0f,            CVAR_SAVECONFIG);
CVAR_FLOATF (vid_shadowFalloffDistance,         1000.0f,            CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowChunkBounds,             true);
CVAR_BOOL   (vid_shadowSecondCull,              true);

CVAR_INTF   (vid_shadowmapSize,                 1024,               CVAR_SAVECONFIG);
CVAR_INTF   (vid_shadowmapType,                 SHADOWMAP_DEPTH,    CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowmapMagFilter,            false);
CVAR_INTF   (vid_shadowmapFilterWidth,          1,                  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CShadowmap::CShadowmap
  ====================*/
CShadowmap::CShadowmap() :
m_pShadowmap(NULL),
m_pShadowmapSurface(NULL),
m_pDSShadow(NULL),
m_bActive(false)
{
}


/*====================
  CShadowmap::~CShadowmap
  ====================*/
CShadowmap::~CShadowmap()
{
}


/*====================
  CShadowmap::Initialize
  ====================*/
void    CShadowmap::Initialize()
{
    PROFILE("CShadowmap::Initialize");

    m_bActive = vid_shadows;
    m_iShadowmapType = vid_shadowmapType;

    if (!g_DeviceCaps.bDepthStencil)
        m_iShadowmapType = SHADOWMAP_R32F;
    else if (!g_DeviceCaps.bR32FRenderTarget)
        m_iShadowmapType = SHADOWMAP_DEPTH;

    if (m_iShadowmapType == SHADOWMAP_R32F && !g_DeviceCaps.bR32FRenderTarget)
        m_bActive = false;

    bool bReloadShaders(false);

    {
        {
            const string &sOldDef(g_ShaderPreprocessor.GetDefinition("SHADOWMAP_TYPE"));
            string sNewDef(XtoS(m_iShadowmapType));

            if (sOldDef != sNewDef)
            {
                g_ShaderPreprocessor.Define("SHADOWMAP_TYPE", sNewDef);
                bReloadShaders = true;
            }
        }

        {
            const string &sOldDef(g_ShaderPreprocessor.GetDefinition("SHADOWMAP_FILTER_WIDTH"));
            string sNewDef(TStringToString(XtoA((m_iShadowmapType != SHADOWMAP_R32F || g_DeviceCaps.bR32FShadowFilter) ? vid_shadowmapFilterWidth : 0)));

            if (sOldDef != sNewDef)
            {
                g_ShaderPreprocessor.Define("SHADOWMAP_FILTER_WIDTH", sNewDef);
                bReloadShaders = true;
            }
        }
    }

    if (bReloadShaders)
        g_ShaderRegistry.ReloadShaders();

    vid_shadows.SetModified(false);
    vid_shadowmapType.SetModified(false);
    vid_shadowmapSize.SetModified(false);

    if (!m_bActive)
        return;
    
    switch (m_iShadowmapType)
    {
    case SHADOWMAP_R32F:
        Console.Video << _T("Using R32F Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;
        m_iShadowmap = D3D_RegisterRenderTargetTexture(_T("$shadowmap"), vid_shadowmapSize, vid_shadowmapSize, D3DFMT_R32F, false);
        m_pShadowmap = g_pTextures2D[m_iShadowmap];
        
        if (m_pShadowmap == NULL)
        {
            Console.Warn << _T("CShadowmap::Initialize() - Invalid shadow map!") << newl;
            break;
        }

        m_pShadowmap->GetSurfaceLevel(0, &m_pShadowmapSurface);
        if (FAILED(g_pd3dDevice->CreateDepthStencilSurface(vid_shadowmapSize, vid_shadowmapSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDSShadow, NULL)))
            K2System.Error(_T("CreateDepthStencilSurface failed for shadowmap render target"));
        break;
    case SHADOWMAP_DEPTH:
        Console.Video << _T("Using Hardware Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;

        if (g_DeviceCaps.bNullRenderTarget)
        {
            if (FAILED(g_pd3dDevice->CreateRenderTarget(vid_shadowmapSize, vid_shadowmapSize, NV_D3DFMT_NULL, D3DMULTISAMPLE_NONE, 0,
                FALSE, &m_pShadowmapSurface, NULL)))
            {
                Console.Warn << _T("CShadowmap::Initialize: CreateRenderTarget failed") << newl;
                break;
            }
        }
        else
        {
#if 1
            int i(D3D_RegisterRenderTargetTexture(_T("$shadowmap_color"), vid_shadowmapSize, vid_shadowmapSize, D3DFMT_A8R8G8B8, false));

            if (i == -1 || g_pTextures2D[i] == NULL)
            {
                Console.Warn << _T("CShadowmap::Initialize() - Invalid shadow map!") << newl;
                break;
            }

            m_pShadowmap = g_pTextures2D[i];
            m_pShadowmap->GetSurfaceLevel(0, &m_pShadowmapSurface);
#else
            if (FAILED(g_pd3dDevice->CreateRenderTarget(vid_shadowmapSize, vid_shadowmapSize, D3DFMT_A8R8G8B8, D3DMULTISAMPLE_NONE, 0,
                FALSE, &m_pShadowmapSurface, NULL)))
            {
                Console.Warn << _T("CShadowmap::Initialize: CreateRenderTarget failed") << newl;
                break;
            }
#endif
        }

        m_iShadowmap = D3D_RegisterDepthTexture(_T("$shadowmap"), vid_shadowmapSize, vid_shadowmapSize, D3DFMT_D24X8);
        m_pDepthShadowmap = g_pTextures2D[m_iShadowmap];

        if (m_pDepthShadowmap == NULL)
        {
            Console.Warn << _T("CShadowmap::Initialize() - Invalid depth shadow map!") << newl;
            break;
        }

        m_pDepthShadowmap->GetSurfaceLevel(0, &m_pDSShadow);
        break;
    case SHADOWMAP_VARIANCE:
        Console.Video << _T("Using Variance Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;
        m_iShadowmap = D3D_RegisterRenderTargetTexture(_T("$shadowmap"), vid_shadowmapSize, vid_shadowmapSize, D3DFMT_G32R32F, false);
        m_pShadowmap = g_pTextures2D[m_iShadowmap];
        
        if (m_pShadowmap == NULL)
        {
            Console.Warn << _T("CShadowmap::Initialize() - Invalid shadow map!") << newl;
            break;
        }

        m_pShadowmap->GetSurfaceLevel(0, &m_pShadowmapSurface);
        if (FAILED(g_pd3dDevice->CreateDepthStencilSurface(vid_shadowmapSize, vid_shadowmapSize, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDSShadow, NULL)))
            K2System.Error(_T("CreateDepthStencilSurface failed for shadowmap render target"));
        break;
    }
}


/*====================
  CShadowmap::Release
  ====================*/
void    CShadowmap::Release()
{
    SAFE_RELEASE(m_pDSShadow);
    SAFE_RELEASE(m_pShadowmapSurface);

    D3D_Unregister2DTexture(_T("$shadowmap_color"));
    D3D_Unregister2DTexture(_T("$shadowmap"));

    m_pShadowmap = NULL;
    m_pDepthShadowmap = NULL;
    m_bActive = false;
}


/*====================
  CShadowmap::Setup
  ====================*/
void    CShadowmap::Setup(const CCamera &cCamera)
{
    PROFILE("CShadowmap::Setup");

    if (m_pShadowmapSurface == NULL || m_pDSShadow == NULL)
    {
        Console.Warn << _T("CShadowmap::Render() - Invalid shadowmap") << newl;
        return;
    }

    // Set the shadowmap as the Render Target
    if (FAILED(g_pd3dDevice->SetRenderTarget(0, m_pShadowmapSurface)))
        Console.Warn << _T("D3D_RenderScene: SetRenderTarget failed") << newl;

    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(m_pDSShadow)))
        Console.Warn << _T("D3D_RenderScene: SetDepthStencilSurface failed") << newl;

    // Clear the shadowmap
    switch (m_iShadowmapType)
    {
    case SHADOWMAP_R32F:
        g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255, 255, 255), 1.0f, 0L);
        break;
    case SHADOWMAP_DEPTH:
        g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255, 255, 255), 1.0f, 0L);
        D3D_SetRenderState(D3DRS_COLORWRITEENABLE, 0);
        break;
    case SHADOWMAP_VARIANCE:
        g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 255, 255, 255), 1.0f, 0L);
        break;
    }

    CCamera virtualCamera(cCamera);

    bool bShadowFalloff(vid_shadowFalloff && !cCamera.HasFlags(CAM_SHADOW_NO_FALLOFF));
    float fShadowFrustumSlideback(vid_shadowFrustumSlideback + cCamera.GetShadowBias());

    if (bShadowFalloff)
        virtualCamera.SetZFar(MIN(float(vid_shadowDrawDistance), virtualCamera.GetZFar()));

    if (vid_shadowUniform || cCamera.HasFlags(CAM_SHADOW_UNIFORM))
    {
        virtualCamera.AddFlags(CAM_ORTHO);

        float fFarPlaneWidth(tan(DEG2RAD(virtualCamera.GetFovX() / 2.0f)) * virtualCamera.GetZFar() * 2.0f);
        virtualCamera.SetOrthoWidth(fFarPlaneWidth);

        float fFarPlaneHeight(tan(DEG2RAD(virtualCamera.GetFovY() / 2.0f)) * virtualCamera.GetZFar() * 2.0f);
        virtualCamera.SetOrthoHeight(fFarPlaneHeight);
    }
    else
    {
        if (vid_shadowFrustumInfinite)
            virtualCamera.AddFlags(CAM_INFINITE);

        if (vid_shadowFrustumCube)
            virtualCamera.AddFlags(CAM_CUBEPROJECTION);

        if (fShadowFrustumSlideback && !cCamera.HasFlags(CAM_SHADOW_NOSLIDEBACK))
        {
            float fOldZFar = virtualCamera.GetZFar();

            // figure out how much we'll need to slide the camera back
            float fSlideBackDistance = MAX(0.0f, float(fShadowFrustumSlideback));

            virtualCamera.SetOrigin(virtualCamera.GetOrigin() + virtualCamera.GetViewAxis(FORWARD) * -fSlideBackDistance);

            virtualCamera.SetZBounds(virtualCamera.GetZNear() + fSlideBackDistance, virtualCamera.GetZFar() + fSlideBackDistance);

            float fNewZFar = virtualCamera.GetZFar();

            // Re-focus the FOV on the visible part of the far clip plane
            virtualCamera.SetFovXCalc(RAD2DEG(atan(tan(DEG2RAD(virtualCamera.GetFovX()) / 2.0f) * fOldZFar / fNewZFar) * 2.0f));

            virtualCamera.SetZBounds(virtualCamera.GetZNear(), virtualCamera.GetZFar() * vid_shadowFrustumScale);

#if 0
            CPlane plFar(virtualCamera.GetViewAxis(FORWARD), virtualCamera.GetZFar());
            CVec3f v3SunFar(SceneManager.GetSunPos() * (plFar.Distance(SceneManager.GetSunPos()) / DotProduct(SceneManager.GetSunPos(), plFar.v3Normal)));
            
            float fSunDistX(ABS(DotProduct(virtualCamera.GetViewAxis(RIGHT), v3SunFar)));
            float fSunDistY(ABS(DotProduct(virtualCamera.GetViewAxis(UP), v3SunFar)));

            float fDistX(tan(DEG2RAD(virtualCamera.GetFovX() / 2.0f)) * virtualCamera.GetZFar());
            float fDistY(tan(DEG2RAD(virtualCamera.GetFovY() / 2.0f)) * virtualCamera.GetZFar());

            Console << fDistX / fSunDistX << _T(" ") << fDistY / fSunDistY << newl;
#endif
        }
    }

    D3D_SetupCamera(virtualCamera);

    // Cache the current transformation matrices from virtualCamera
    D3DXMATRIXA16   mView = g_mView;
    D3DXMATRIXA16   mProj = g_mProj;
    D3DXMATRIXA16   mViewProj = g_mViewProj;

    // Transform light into post-projective space
    CVec3f v3SunPos(SceneManager.GetSunPos());
    D3DXVECTOR3     vWorldLight(v3SunPos.x, v3SunPos.y, v3SunPos.z);
    D3DXVECTOR3     vEyeLight;

    D3DXVec3TransformNormal(&vEyeLight, &vWorldLight, &mView);
    D3DXVECTOR4     vLight(vEyeLight.x, vEyeLight.y, vEyeLight.z, 0.0f);
    D3DXVec4Transform(&vLight, &vLight, &mProj);

    D3DXVECTOR3     vForward(0.0f, 0.0f, 1.0f);

    float fLightAngle(fabs(90.0f - RAD2DEG(acos(D3DXVec3Dot(&vEyeLight, &vForward)))));

    bool bParallel = fLightAngle < (virtualCamera.HasFlags(CAM_SHADOW_PARALLEL) ? 2.0f : vid_shadowParallelEpsilon);

    CCamera lightCamera;

    bool    bBackLight = false;

    if (bParallel)
    {
        g_bInvertedProjection = true;
    }
    else
    {
        if (vLight.w < 0.0f)
        {
            g_bInvertedProjection = false;
            bBackLight = true;
        }
        else
        {
            g_bInvertedProjection = true;
        }

        if (vLight.w != 0.0f)
        {
            // Put the light back onto the infinity plane
            vLight.x /= vLight.w;
            vLight.y /= vLight.w;
            vLight.z /= vLight.w;
            vLight.w = 0.0f;

            float fLightDistance(D3DXVec3Length((D3DXVECTOR3*)&vLight));

            if (fLightDistance > vid_shadowMaxLightDistance)
            {
                vLight.x *= vid_shadowMaxLightDistance / fLightDistance;
                vLight.y *= vid_shadowMaxLightDistance / fLightDistance;
                vLight.z *= vid_shadowMaxLightDistance / fLightDistance;
            }
        }
        else
        {
            D3DXVec3Normalize((D3DXVECTOR3*)&vLight, (D3DXVECTOR3*)&vLight);
            bParallel = true;
            g_bInvertedProjection = true;
        }
    }

    static vector<CVec3f>   vPoints;
    static vector<CVec3f>   vPointsUntransformed;
    vPoints.clear();
    vPointsUntransformed.clear();

    if (vid_shadowUnitCubeClipping)
    {
        PROFILE("Unit Cube Clipping");

        if (vid_shadowChunkBounds && terrain.pWorld && !cCamera.HasFlags(CAM_NO_TERRAIN) && !cCamera.HasFlags(CAM_SHADOW_SCENE_BOUNDS))
        {
            CPlane *planes(SceneManager.GetFrustumPlanes());

            // Add the frustum planes to the scene hull
            static vector<CPlane>   vPlanes;
            vPlanes.clear();
            vPlanes.push_back(CPlane(-planes[0].v3Normal[0], -planes[0].v3Normal[1], -planes[0].v3Normal[2], -planes[0].fDist));
            vPlanes.push_back(CPlane(-planes[1].v3Normal[0], -planes[1].v3Normal[1], -planes[1].v3Normal[2], -planes[1].fDist));
            vPlanes.push_back(CPlane(-planes[2].v3Normal[0], -planes[2].v3Normal[1], -planes[2].v3Normal[2], -planes[2].fDist));
            vPlanes.push_back(CPlane(-planes[3].v3Normal[0], -planes[3].v3Normal[1], -planes[3].v3Normal[2], -planes[3].fDist));
            vPlanes.push_back(CPlane(-planes[4].v3Normal[0], -planes[4].v3Normal[1], -planes[4].v3Normal[2], -planes[4].fDist));
            vPlanes.push_back(CPlane(-planes[5].v3Normal[0], -planes[5].v3Normal[1], -planes[5].v3Normal[2], -planes[5].fDist));
            
            vector<CPlane>::iterator itEnd(vPlanes.end());

            if (bShadowFalloff)
                vPlanes[4].Transform(vPlanes[4].v3Normal * -(MAX(cCamera.GetZFar() - vid_shadowDrawDistance, 0.0f)));

            for (int iY(0); iY < terrain.iNumChunksY; ++iY)
            {
                for (int iX(0); iX < terrain.iNumChunksX; ++iX)
                {
                    STerrainChunk *chunk = &terrain.chunks[iY][iX];

                    if (chunk->bVisible)
                    {
                        CConvexHull     sceneHull(chunk->bbBounds);

                        for (vector<CPlane>::iterator it(vPlanes.begin()); it != itEnd; ++it)
                        {
                            if (M_AABBOnPlaneSide(chunk->bbBounds, *it) == PLANE_INTERSECTS)
                                sceneHull.AddPlane(*it);
                        }

                        // build the list of extreme points of the new convex hull
                        sceneHull.GetPoints(vPoints);
                    }
                }
            }
        }
        else
        {
            // Build the scene bounding box
            CBBoxf  bbScene;

            if (!g_pCam->HasFlags(CAM_NO_TERRAIN))
                D3D_TerrainBounds(bbScene);

            D3D_ObjectBounds(bbScene);

            if (bbScene.GetDim(X) > 0.0f ||
                bbScene.GetDim(Y) > 0.0f ||
                bbScene.GetDim(Z) > 0.0f)
            {
                CConvexHull     sceneHull(bbScene);

                CPlane *planes(SceneManager.GetFrustumPlanes());

                // Add the frustum planes to the scene hull
                static vector<CPlane>   vPlanes;
                vPlanes.clear();
                vPlanes.push_back(CPlane(-planes[0].v3Normal[0], -planes[0].v3Normal[1], -planes[0].v3Normal[2], -planes[0].fDist));
                vPlanes.push_back(CPlane(-planes[1].v3Normal[0], -planes[1].v3Normal[1], -planes[1].v3Normal[2], -planes[1].fDist));
                vPlanes.push_back(CPlane(-planes[2].v3Normal[0], -planes[2].v3Normal[1], -planes[2].v3Normal[2], -planes[2].fDist));
                vPlanes.push_back(CPlane(-planes[3].v3Normal[0], -planes[3].v3Normal[1], -planes[3].v3Normal[2], -planes[3].fDist));
                vPlanes.push_back(CPlane(-planes[4].v3Normal[0], -planes[4].v3Normal[1], -planes[4].v3Normal[2], -planes[4].fDist));
                vPlanes.push_back(CPlane(-planes[5].v3Normal[0], -planes[5].v3Normal[1], -planes[5].v3Normal[2], -planes[5].fDist));

                if (bShadowFalloff)
                    vPlanes[4].Transform(vPlanes[4].v3Normal * -(MAX(cCamera.GetZFar() - vid_shadowDrawDistance, 0.0f)));

                sceneHull.AddPlanes(vPlanes);

                // build the list of extreme points of the new convex hull
                sceneHull.GetPoints(vPoints);
            }
            else
            {
                CPlane *planes(SceneManager.GetFrustumPlanes());

                // Add the frustum planes to the scene hull
                static vector<CPlane>   vPlanes;
                vPlanes.clear();
                vPlanes.push_back(CPlane(-planes[0].v3Normal[0], -planes[0].v3Normal[1], -planes[0].v3Normal[2], -planes[0].fDist));
                vPlanes.push_back(CPlane(-planes[1].v3Normal[0], -planes[1].v3Normal[1], -planes[1].v3Normal[2], -planes[1].fDist));
                vPlanes.push_back(CPlane(-planes[2].v3Normal[0], -planes[2].v3Normal[1], -planes[2].v3Normal[2], -planes[2].fDist));
                vPlanes.push_back(CPlane(-planes[3].v3Normal[0], -planes[3].v3Normal[1], -planes[3].v3Normal[2], -planes[3].fDist));
                vPlanes.push_back(CPlane(-planes[4].v3Normal[0], -planes[4].v3Normal[1], -planes[4].v3Normal[2], -planes[4].fDist));
                vPlanes.push_back(CPlane(-planes[5].v3Normal[0], -planes[5].v3Normal[1], -planes[5].v3Normal[2], -planes[5].fDist));

                if (bShadowFalloff)
                    vPlanes[4].Transform(vPlanes[4].v3Normal * -(MAX(cCamera.GetZFar() - vid_shadowDrawDistance, 0.0f)));

                CConvexHull     sceneHull(vPlanes);

                sceneHull.GetPoints(vPoints);
            }
        }

        vPointsUntransformed = vPoints;

        if (vPoints.empty())
        {
            vPoints.push_back(CVec3f(-1.0f, -1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f(-1.0f,  1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f( 1.0f, -1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f( 1.0f,  1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f(-1.0f, -1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f(-1.0f,  1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f( 1.0f, -1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f( 1.0f,  1.0f, vid_shadowFarZ));
        }
        else
        {
            D3D_TransformPoints(vPoints, &mViewProj);
            
            //for (vector<CVec3f>::iterator it(vPoints.begin()); it != vPoints.end(); ++it)
            //  it->z = CLAMP<float>(it->z, vid_shadowNearZ, vid_shadowFarZ);
        }
    }
    else
    {
        CPlane *planes(SceneManager.GetFrustumPlanes());

        // Add the frustum planes to the scene hull
        static vector<CPlane>   vPlanes;
        vPlanes.clear();
        vPlanes.push_back(CPlane(-planes[0].v3Normal[0], -planes[0].v3Normal[1], -planes[0].v3Normal[2], -planes[0].fDist));
        vPlanes.push_back(CPlane(-planes[1].v3Normal[0], -planes[1].v3Normal[1], -planes[1].v3Normal[2], -planes[1].fDist));
        vPlanes.push_back(CPlane(-planes[2].v3Normal[0], -planes[2].v3Normal[1], -planes[2].v3Normal[2], -planes[2].fDist));
        vPlanes.push_back(CPlane(-planes[3].v3Normal[0], -planes[3].v3Normal[1], -planes[3].v3Normal[2], -planes[3].fDist));
        vPlanes.push_back(CPlane(-planes[4].v3Normal[0], -planes[4].v3Normal[1], -planes[4].v3Normal[2], -planes[4].fDist));
        vPlanes.push_back(CPlane(-planes[5].v3Normal[0], -planes[5].v3Normal[1], -planes[5].v3Normal[2], -planes[5].fDist));

        if (bShadowFalloff)
            vPlanes[4].Transform(vPlanes[4].v3Normal * -(MAX(cCamera.GetZFar() - vid_shadowDrawDistance, 0.0f)));

        CConvexHull     sceneHull(vPlanes);

        sceneHull.GetPoints(vPoints);

        vPointsUntransformed = vPoints;

        if (vPoints.empty())
        {
            vPoints.push_back(CVec3f(-1.0f, -1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f(-1.0f,  1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f( 1.0f, -1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f( 1.0f,  1.0f, vid_shadowNearZ));
            vPoints.push_back(CVec3f(-1.0f, -1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f(-1.0f,  1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f( 1.0f, -1.0f, vid_shadowFarZ));
            vPoints.push_back(CVec3f( 1.0f,  1.0f, vid_shadowFarZ));
        }
        else
        {
            D3D_TransformPoints(vPoints, &mViewProj);

            for (vector<CVec3f>::iterator it(vPoints.begin()); it != vPoints.end(); ++it)
                it->z = CLAMP<float>(it->z, vid_shadowNearZ, vid_shadowFarZ);
        }
    }

    float fNear(FAR_AWAY);
    float fFar(-FAR_AWAY);
    
    vector<CVec3f>::iterator itEnd(vPointsUntransformed.end());
    for (vector<CVec3f>::iterator it(vPointsUntransformed.begin()); it != itEnd; ++it)
    {
        float fDot(DotProduct(cCamera.GetViewAxis(FORWARD), *it - cCamera.GetOrigin()));

        if (fDot < fNear)
            fNear = fDot;
        if (fDot > fFar)
            fFar = fDot;
    }

    CConvexPolyhedron cScene(cCamera.GetOrigin(), cCamera.GetViewAxis(), cCamera.GetFovX(), cCamera.GetFovY(),
        bShadowFalloff ? MIN(float(vid_shadowDrawDistance), MIN(cCamera.GetZFar(), fFar)) : MIN(cCamera.GetZFar(), fFar));

    if (vid_shadowSecondCull)
        SceneManager.CullEntitiesShadow(cScene);
    
    if (!cCamera.HasFlags(CAM_NO_TERRAIN))
        D3D_FlagVisibleTerrainChunksShadow(cScene);

    CVec3f          vOrigin(vLight.x, vLight.y, vLight.z);
    CFrustum        shadowFrustum(vPoints, vOrigin);

    g_bSplitLightProjection = false;

    if (bParallel)
    {
        CVec3f          vDir(Normalize(CVec3f(-vLight.x, -vLight.y, -vLight.z)));

        COrthoFrustum   orthoFrustum(vPoints, vDir, vid_shadowParallelPullback);

        lightCamera.SetFlags(CAM_ORTHO);
        lightCamera.SetX(0);
        lightCamera.SetY(0);
        lightCamera.SetWidth(float(vid_shadowmapSize));
        lightCamera.SetHeight(float(vid_shadowmapSize));
        lightCamera.SetOrthoWidth(orthoFrustum.GetWidth());
        lightCamera.SetOrthoHeight(orthoFrustum.GetHeight());
        lightCamera.SetFovX(0.0f);
        lightCamera.SetFovY(0.0f);

        lightCamera.SetZBounds(0.0f, orthoFrustum.GetFar());
        lightCamera.SetAspect(1.0f);
        lightCamera.SetOrigin(orthoFrustum.GetOrigin());

        lightCamera.SetViewAxis(orthoFrustum.GetAxis());
    }
    else
    {
        lightCamera.SetFlags(CAM_INVERSEPROJECTION);
        lightCamera.SetWidth(float(vid_shadowmapSize));
        lightCamera.SetHeight(float(vid_shadowmapSize));
        lightCamera.SetX(0);
        lightCamera.SetY(0);
        lightCamera.SetFovX(CLAMP(shadowFrustum.GetFovX(), float(vid_shadowMinFov), MIN(float(vid_shadowMaxFov), cCamera.GetShadowMaxFov())));
        lightCamera.SetFovY(CLAMP(shadowFrustum.GetFovY(), float(vid_shadowMinFov), MIN(float(vid_shadowMaxFov), cCamera.GetShadowMaxFov())));
        lightCamera.SetZBounds(bBackLight ? (-shadowFrustum.GetNear() / vid_shadowZScale) : (shadowFrustum.GetNear() / vid_shadowZScale), shadowFrustum.GetFar());
        lightCamera.SetAspect(tan(DEG2RAD(lightCamera.GetFovX()) / 2.0f) / tan(DEG2RAD(lightCamera.GetFovY()) / 2.0f));

        lightCamera.SetOrigin(vOrigin);
        lightCamera.SetViewAxis(shadowFrustum.GetAxis());
    }

    if (vid_shadowDebug)
    {
        Console << _T("Angle: ") << fabs(RAD2DEG(acos(DotProduct(virtualCamera.GetViewAxis(FORWARD), SceneManager.GetSunPos())))) << _T(" ");
        Console << _T("vLight: ") << D3DXVec3Length((D3DXVECTOR3*)&vLight) << _T(" ");
        Console << _T("Fov: ") << _T("(") << lightCamera.GetFovX() << _T(", ") << lightCamera.GetFovY() << _T(")") << _T(" ");
        Console << newl;
    }

    // Initialize the scene: setup the camera, set some global vars
    D3D_SetupCamera(lightCamera);

    g_uiImageWidth = vid_shadowmapSize;
    g_uiImageHeight = vid_shadowmapSize;

    g_mView = mView;
    g_mProj = mProj;
    g_mViewProj = mViewProj * g_mViewProj;

    // Save Light transformation matrix for use in the color pass
    float fOffsetX, fOffsetY;

    D3DXMATRIXA16 mTexShadowScale(0.5f,  0.0f, 0.0f, 0.0f,
                                  0.0f, -0.5f, 0.0f, 0.0f,
                                  0.0f,  0.0f, 1.0f, 0.0f,
                                  0.0f,  0.0f, 0.0f, 1.0f);

    fOffsetX = 0.5f;
    fOffsetY = 0.5f;

    D3DXMATRIXA16 mTexShadowOffset1(1.0f,     0.0f,      0.0f, 0.0f,
                                    0.0f,     1.0f,      0.0f, 0.0f,
                                    0.0f,     0.0f,      1.0f, 0.0f,
                                    fOffsetX, fOffsetY,  0.0f, 1.0f);

    fOffsetX = (0.5f / vid_shadowmapSize);
    fOffsetY = (0.5f / vid_shadowmapSize);

    D3DXMATRIXA16 mTexShadowOffset2(1.0f,     0.0f,      0.0f, 0.0f,
                                    0.0f,     1.0f,      0.0f, 0.0f,
                                    0.0f,     0.0f,      1.0f, 0.0f,
                                    fOffsetX, fOffsetY,  0.0f, 1.0f);

    g_mLightViewProjTex = g_mViewProj * (mTexShadowScale * mTexShadowOffset1 * mTexShadowOffset2);
}


/*====================
  CShadowmap::Render
  ====================*/
void    CShadowmap::Render()
{
    PROFILE("CShadowmap::Render");

    // Render shadow Scene
    g_RenderList.Setup(PHASE_SHADOW);
    g_RenderList.Sort();
    g_RenderList.Render(PHASE_SHADOW);

    // Restore the old Render Target
    g_pd3dDevice->SetRenderTarget(0, g_pBackBuffer);

    // Restore the old DepthStencilSurface
    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(g_pDepthBuffer)))
        Console.Warn << _T("D3D_RenderScene: Failed to restore old DepthStencilSurface") << newl;

    D3D_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
}
