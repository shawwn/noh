// (C)2008 S2 Games
// c_reflectionmap.cpp
//
// Reflection wrapper
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_reflectionmap.h"

#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "C_renderlist.h"
#include "d3d9f_texture.h"
#include "d3d9f_shader.h"
#include "d3d9f_scene.h"
#include "d3d9f_terrain.h"
#include "d3d9f_state.h"
#include "d3d9f_util.h"
#include "d3d9f_material.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CReflectionMap  g_ReflectionMap;

CVAR_BOOLF  (vid_reflections,               false,                  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CReflectionMap::CReflectionMap
  ====================*/
CReflectionMap::CReflectionMap() :
m_iReflectionMap(g_iInvis),
m_pReflectionMap(NULL),
m_pReflectionMapSurface(NULL),
m_pDepthStencil(NULL),
m_bActive(false),
m_uiWidth(0),
m_uiHeight(0)
{
}


/*====================
  CReflectionMap::~CReflectionMap
  ====================*/
CReflectionMap::~CReflectionMap()
{
}


/*====================
  CReflectionMap::Initialize
  ====================*/
void    CReflectionMap::Initialize(int iWidth, int iHeight)
{
    PROFILE("CReflectionMap::Initialize");

    return;

    vid_reflections.SetModified(false);

    if (!vid_reflections)
    {
        m_iReflectionMap = g_iInvis;
        return;
    }

    m_uiWidth = iWidth >> 1;
    m_uiHeight = iHeight >> 1;

    m_iReflectionMap = D3D_RegisterRenderTargetTexture(_T("$reflection"), m_uiWidth, m_uiHeight, D3DFMT_A8R8G8B8, REFLECTIONS_MIPMAP);

    if (m_iReflectionMap == -1)
    {
        m_bActive = false;
        return;
    }

    m_pReflectionMap = g_pTextures2D[m_iReflectionMap];
    m_pReflectionMap->GetSurfaceLevel(0, &m_pReflectionMapSurface);

    if (FAILED(g_pd3dDevice->CreateDepthStencilSurface(m_uiWidth, m_uiHeight, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencil, NULL)))
        K2System.Error(_T("CreateDepthStencilSurface failed for reflection map render target"));

    int iTextureFlags(TEX_FULL_QUALITY | TEX_NO_COMPRESS);
    if (!REFLECTIONS_MIPMAP)
        iTextureFlags |= TEX_NO_MIPMAPS;

    m_hReflectionTexture = g_ResourceManager.Register(new CTexture(_T("$reflection"), TEXTURE_2D, iTextureFlags, TEXFMT_A8R8G8B8), RES_TEXTURE);

    CTexture *pReflectionMapTexture(g_ResourceManager.GetTexture(m_hReflectionTexture));
    if (pReflectionMapTexture != NULL)
        pReflectionMapTexture->SetIndex(m_iReflectionMap);

    m_bActive = true;
}


/*====================
  CReflectionMap::Release
  ====================*/
void    CReflectionMap::Release()
{
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pReflectionMapSurface);

    D3D_Unregister2DTexture(_T("$reflection"));

    m_iReflectionMap = g_iInvis;
    m_pReflectionMap = NULL;
    m_bActive = false;

    CTexture *pReflectionMapTexture(g_ResourceManager.GetTexture(m_hReflectionTexture));
    if (pReflectionMapTexture != NULL)
        pReflectionMapTexture->SetIndex(m_iReflectionMap);
}


/*====================
  CReflectionMap::Render
  ====================*/
void    CReflectionMap::Render(const CCamera &cCamera)
{
    PROFILE("CReflectionMap::Render");

    if (!vid_reflections)
        return;

    CTexture *pReflectionMapTexture(g_ResourceManager.GetTexture(m_hReflectionTexture));
    if (pReflectionMapTexture != NULL)
        pReflectionMapTexture->SetIndex(g_iInvis);

    if (!m_bActive || !g_bValidScene)
        return;

    if (m_uiWidth != (g_Viewport.Width >> 1) || m_uiHeight != (g_Viewport.Height >> 1))
    {
        Release();
        Initialize(g_Viewport.Width, g_Viewport.Height);

        if (!m_bActive)
            return;
    }

    // Set the new as the render target
    if (FAILED(g_pd3dDevice->SetRenderTarget(0, m_pReflectionMapSurface)))
    {
        Console.Err << _T("CReflectionMap::Render: SetRenderTarget failed") << newl;
        return;
    }

    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(m_pDepthStencil)))
    {
        Console.Err << _T("CReflectionMap::Render: SetDepthStencilSurface failed") << newl;
        return;
    }

    CCamera cReflectionCamera(cCamera);

    // Initialize the scene: setup the camera, set some global vars
    D3D_SetupCamera(cReflectionCamera);

    const CPlane &plReflectionPlane(cReflectionCamera.GetReflect());

    D3DXPLANE d3dPlane
    (
        plReflectionPlane.v3Normal.x,
        plReflectionPlane.v3Normal.y,
        plReflectionPlane.v3Normal.z,
        -plReflectionPlane.fDist
    );

    // Reflect the camera
    D3DXMATRIXA16 mReflect;
    D3DXMatrixReflect(&mReflect, (const D3DXPLANE *)&d3dPlane);

    g_mView = mReflect * g_mView;
    g_mViewRotate = mReflect * g_mViewRotate;
    g_mViewProj = g_mView * g_mProj;

    cReflectionCamera.SetOrigin(Reflect(cReflectionCamera.GetOrigin(), plReflectionPlane));

    g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_mView);

    g_bInvertedProjection = true;

    D3DXMATRIXA16 mViewProjInverse;
    D3DXMATRIXA16 mViewProjIT;
    D3DXMatrixInverse(&mViewProjInverse, NULL, &g_mViewProj);
    D3DXMatrixTranspose(&mViewProjIT, &mViewProjInverse); 

    D3DXPLANE d3dHPlane;
    D3DXPlaneTransform(&d3dHPlane, &d3dPlane, &mViewProjIT);

    D3D_SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
    g_pd3dDevice->SetClipPlane(0, static_cast<float *>(d3dHPlane));

    g_fSceneScaleX = 1.0f;
    g_fSceneScaleY = 1.0f;

    g_Viewport.X = 0;
    g_Viewport.Y = 0;
    g_Viewport.Width = m_uiWidth;
    g_Viewport.Height = m_uiHeight;
    g_Viewport.MinZ = g_Viewport.MinZ;
    g_Viewport.MaxZ = g_Viewport.MaxZ;

    g_uiImageWidth = m_uiWidth;
    g_uiImageHeight = m_uiHeight;

    g_pd3dDevice->SetViewport(&g_Viewport);

    CVec4f v4ClearColor(SceneManager.GetSceneBgColor(), 0.0f);

    // Clear
    g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, v4ClearColor.GetAsDWord(), 1.0f, 0L);

    g_bReflectPass = true;
    bool bOldCamShadows(g_bCamShadows);
    bool bOldFogofWar(g_bFogofWar);

    g_bCamShadows = false;
    g_bFogofWar = false;

    // Render reflected scene
    g_RenderList.Setup(PHASE_COLOR);
    g_RenderList.Sort();
    g_RenderList.Render(PHASE_COLOR);

    g_bReflectPass = false;
    g_bInvertedProjection = false;
    g_bCamShadows = bOldCamShadows;
    g_bFogofWar = bOldFogofWar;

    D3D_SelectMaterial(g_MaterialGUI, PHASE_COLOR, VERTEX_GUI, 0.0f, false);

    D3D_SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

    // Restore the old Render Target
    g_pd3dDevice->SetRenderTarget(0, g_pBackBuffer);

    // Restore the old DepthStencilSurface
    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(g_pDepthBuffer)))
        Console.Warn << _T("D3D_RenderScene: Failed to restore old DepthStencilSurface") << newl;

    if (pReflectionMapTexture != NULL)
        pReflectionMapTexture->SetIndex(m_iReflectionMap);
}
