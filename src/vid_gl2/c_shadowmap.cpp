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

#include "c_gfxutils.h"
#include "c_gfxshaders.h"
#include "c_gfx3d.h"
#include "c_gfxterrain.h"
#include "c_renderlist.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"

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

CVAR_INTF   (vid_shadowmapSize,                 1024,               CVAR_SAVECONFIG);
CVAR_INTF   (vid_shadowmapType,                 SHADOWMAP_DEPTH,    CVAR_SAVECONFIG);
CVAR_BOOL   (vid_shadowmapMagFilter,            false);
CVAR_INTF   (vid_shadowmapFilterWidth,          1,                  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CShadowmap::CShadowmap
  ====================*/
CShadowmap::CShadowmap() :
m_bActive(false),
m_eShadowmapType(SHADOWMAP_DEPTH),

m_uiFrameBufferObject(0),
m_uiShadowTexture(0),
m_uiDepthRenderBuffer(0)
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
void    CShadowmap::Initialize(EShadowmapType eType)
{
    m_bActive = vid_shadows;
    m_eShadowmapType = eType;

    bool bReloadShaders(false);

    {
        {
            const string &sOldDef(g_ShaderPreprocessor.GetDefinition("SHADOWMAP_TYPE"));
            string sNewDef(TStringToString(XtoA(m_eShadowmapType)));

            if (sOldDef != sNewDef)
            {
                g_ShaderPreprocessor.Define("SHADOWMAP_TYPE", sNewDef);
                bReloadShaders = true;
            }
        }

        {
            const string &sOldDef(g_ShaderPreprocessor.GetDefinition("SHADOWMAP_FILTER_WIDTH"));
            string sNewDef(TStringToString(XtoA(vid_shadowmapFilterWidth)));

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

    glPushAttrib(GL_COLOR_BUFFER_BIT);

    // Initialize frame buffer object
    glGenFramebuffersEXT(1, &m_uiFrameBufferObject);
    glUseProgramObjectARB(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_uiFrameBufferObject);

    switch (m_eShadowmapType)
    {
    case SHADOWMAP_R32F:
        glGenTextures(1, &m_uiShadowTexture);
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, m_uiShadowTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_FLOAT32_ATI, vid_shadowmapSize, vid_shadowmapSize, 0, GL_LUMINANCE, GL_FLOAT, nullptr);

        PRINT_GLERROR_BREAK();

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_uiShadowTexture, 0);

        PRINT_GLERROR_BREAK();

        // Depth buffer
        glGenRenderbuffersEXT(1, &m_uiDepthRenderBuffer);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_uiDepthRenderBuffer);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, vid_shadowmapSize, vid_shadowmapSize);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_uiDepthRenderBuffer);

        break;

    case SHADOWMAP_DEPTH:
        glGenTextures(1, &m_uiShadowTexture);

        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, m_uiShadowTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, vid_shadowmapSize, vid_shadowmapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

        PRINT_GLERROR_BREAK();

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_uiShadowTexture, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        PRINT_GLERROR_BREAK();

        break;
    }

    bool bFailed(!GL_CheckFrameBufferStatus(_T("Shadowmap")));

    // Restore old frame buffer
    glUseProgramObjectARB(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    glPopAttrib();

    PRINT_GLERROR_BREAK();

    // Fallback to R32F if depth shadows aren't supported
    if (bFailed)
    {
        Release();

        if (eType == SHADOWMAP_DEPTH)
        {
            Initialize(SHADOWMAP_R32F);
        }
        else if (eType == SHADOWMAP_R32F)
        {
            // Fail and disable shadows
            m_bActive = false;
            return; 
        }
    }
    else
    {
        if (m_eShadowmapType == SHADOWMAP_R32F)
            Console.Video << _T("Using R32F Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;
        else if (m_eShadowmapType == SHADOWMAP_DEPTH)
            Console.Video << _T("Using Hardware Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;
        else
            Console.Video << _T("Using Unknown Shadow Map ") << vid_shadowmapSize << _T(" x ") << vid_shadowmapSize << newl;
    }
}


/*====================
  CShadowmap::Release
  ====================*/
void    CShadowmap::Release()
{
    GL_SAFE_DELETE(glDeleteFramebuffersEXT, m_uiFrameBufferObject);
    GL_SAFE_DELETE(glDeleteTextures, m_uiShadowTexture);
    GL_SAFE_DELETE(glDeleteRenderbuffersEXT, m_uiDepthRenderBuffer);

    m_bActive = false;
}


/*====================
  CShadowmap::Render
  ====================*/
void    CShadowmap::Render(const CCamera &cCamera)
{
    PROFILE("CShadowmap::Render");

    if (m_uiFrameBufferObject == 0 || m_uiShadowTexture == 0)
    {
        Console.Warn << _T("CShadowmap::Render() - Invalid shadowmap") << newl;
        return;
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
        }
    }

    // Initialize the scene: setup the camera, set some global vars
    Gfx3D->SetupCamera(virtualCamera);

    if (!cCamera.HasFlags(CAM_NO_TERRAIN))
        GfxTerrain->FlagVisibleTerrainChunks();

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

    // Save draw and read buffer
    glPushAttrib(GL_COLOR_BUFFER_BIT);

    // Set the shadowmap as the Render Target
    glUseProgramObjectARB(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_uiFrameBufferObject);

    // Clear the shadowmap
    switch (m_eShadowmapType)
    {
    case SHADOWMAP_R32F:
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        break;
    case SHADOWMAP_DEPTH:
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        break;
    }

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

        if (vid_shadowChunkBounds && GfxTerrain->pWorld && !cCamera.HasFlags(CAM_NO_TERRAIN) && !cCamera.HasFlags(CAM_SHADOW_SCENE_BOUNDS))
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

            for (int iX(0); iX < GfxTerrain->iNumChunksX; ++iX)
            {
                for (int iY(0); iY < GfxTerrain->iNumChunksY; ++iY)
                {
                    STerrainChunk *chunk = &GfxTerrain->chunks[iY][iX];

                    if (chunk->bVisible)
                    {
                        CConvexHull     sceneHull(chunk->bbBounds);

                        for (vector<CPlane>::iterator it(vPlanes.begin()); it != vPlanes.end(); ++it)
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
                GfxTerrain->TerrainBounds(bbScene);

            Gfx3D->ObjectBounds(bbScene);

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

            vPointsUntransformed = vPoints;
        }
        else
        {
            GfxUtils->TransformPoints(vPoints, &mViewProj);
            
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
            GfxUtils->TransformPoints(vPoints, &mViewProj);

            for (vector<CVec3f>::iterator it(vPoints.begin()); it != vPoints.end(); ++it)
                it->z = CLAMP<float>(it->z, vid_shadowNearZ, vid_shadowFarZ);
        }
    }

    float fNear(FAR_AWAY);
    for (vector<CVec3f>::iterator it(vPointsUntransformed.begin()); it != vPointsUntransformed.end(); ++it)
    {
        float fDot(DotProduct(cCamera.GetViewAxis(FORWARD), *it - cCamera.GetOrigin()));

        if (fDot < fNear)
            fNear = fDot;
    }

    float fFar(-FAR_AWAY);
    for (vector<CVec3f>::iterator it(vPointsUntransformed.begin()); it != vPointsUntransformed.end(); ++it)
    {
        float fDot(DotProduct(cCamera.GetViewAxis(FORWARD), *it - cCamera.GetOrigin()));

        if (fDot > fFar)
            fFar = fDot;
    }

    CConvexPolyhedron cScene(cCamera.GetOrigin(), cCamera.GetViewAxis(), cCamera.GetFovX(), cCamera.GetFovY(),
        bShadowFalloff ? MIN(float(vid_shadowDrawDistance), MIN(cCamera.GetZFar(), fFar)) : MIN(cCamera.GetZFar(), fFar));

    SceneManager.CullEntitiesShadow(cScene);
    
    if (!cCamera.HasFlags(CAM_NO_TERRAIN))
        GfxTerrain->FlagVisibleTerrainChunksShadow(cScene);

    CVec3f          vOrigin(vLight.x, vLight.y, vLight.z);
    CFrustum        shadowFrustum(vPoints, vOrigin);

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

    int iOldScreenWidth(g_iScreenWidth);
    int iOldScreenHeight(g_iScreenHeight);

    g_iScreenWidth = vid_shadowmapSize;
    g_iScreenHeight = vid_shadowmapSize;

    // Initialize the scene: setup the camera, set some global vars
    Gfx3D->SetupCamera(lightCamera);

    g_mView = mView;
    g_mProj = mProj * g_mViewProj;
    g_mViewProj = g_mView * g_mProj;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((float *)&g_mProj);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((float *)&g_mView);

    // Save Light transformation matrix for use in the color pass
    float fOffsetX, fOffsetY, fOffsetZ;

    D3DXMATRIXA16 mTexShadowScale(0.5f, 0.0f, 0.0f, 0.0f,
                                  0.0f, 0.5f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 0.5f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f);

    fOffsetX = 0.5f;
    fOffsetY = 0.5f;
    fOffsetZ = 0.5f;

    D3DXMATRIXA16 mTexShadowOffset1(1.0f,     0.0f,     0.0f,     0.0f,
                                    0.0f,     1.0f,     0.0f,     0.0f,
                                    0.0f,     0.0f,     1.0f,     0.0f,
                                    fOffsetX, fOffsetY, fOffsetZ, 1.0f);

    g_mLightViewProjTex = g_mViewProj * (mTexShadowScale * mTexShadowOffset1);

    Gfx3D->AddWorld(PHASE_SHADOW);

    g_RenderList.Setup(PHASE_SHADOW);
    g_RenderList.Sort();
    g_RenderList.Render(PHASE_SHADOW);
    g_RenderList.Clear();

    glUseProgramObjectARB(0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // Restore draw and read buffer
    glPopAttrib();

    g_iScreenWidth = iOldScreenWidth;
    g_iScreenHeight = iOldScreenHeight;
}
