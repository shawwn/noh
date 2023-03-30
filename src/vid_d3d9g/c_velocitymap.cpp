// (C)2008 S2 Games
// c_velocitymap.cpp
//
// Velocity wrapper
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_velocitymap.h"

#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "C_renderlist.h"
#include "d3d9g_texture.h"
#include "d3d9g_shader.h"
#include "d3d9g_scene.h"
#include "d3d9g_terrain.h"
#include "d3d9g_state.h"
#include "d3d9g_util.h"
#include "d3d9g_material.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVelocityMap    g_VelocityMap;

CVAR_BOOLF  (vid_motionBlur,                false,                  CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CVelocityMap::CVelocityMap
  ====================*/
CVelocityMap::CVelocityMap() :
m_iVelocityMap(g_iBlack),
m_pVelocityMap(NULL),
m_pVelocityMapSurface(NULL),
m_pDepthStencil(NULL),
m_bActive(false),
m_uiWidth(0),
m_uiHeight(0),
m_hVelocityTexture(INVALID_RESOURCE),
m_hVelocityReference(INVALID_RESOURCE)
{
}


/*====================
  CVelocityMap::~CVelocityMap
  ====================*/
CVelocityMap::~CVelocityMap()
{
}


/*====================
  CVelocityMap::Initialize
  ====================*/
void    CVelocityMap::Initialize()
{
    PROFILE("CVelocityMap::Initialize");

    vid_motionBlur.SetModified(false);

    if (!vid_motionBlur)
    {
        m_iVelocityMap = g_iBlack;
        return;
    }

    m_uiWidth = g_CurrentVidMode.iWidth;
    m_uiHeight = g_CurrentVidMode.iHeight;

    m_iVelocityMap = D3D_RegisterRenderTargetTexture(_T("$velocity"), m_uiWidth, m_uiHeight, D3DFMT_A8R8G8B8, true);
    m_pVelocityMap = g_pTextures2D[m_iVelocityMap];
    m_pVelocityMap->GetSurfaceLevel(0, &m_pVelocityMapSurface);

    if (FAILED(g_pd3dDevice->CreateDepthStencilSurface(m_uiWidth, m_uiHeight, D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, FALSE, &m_pDepthStencil, NULL)))
        K2System.Error(_T("CreateDepthStencilSurface failed for velocity map render target"));

    m_hVelocityTexture = g_ResourceManager.Register(new CTexture(_T("$velocity"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_hVelocityReference = g_ResourceManager.Register(_T("!velocity"), RES_REFERENCE);
    g_ResourceManager.UpdateReference(m_hVelocityReference, m_hVelocityTexture);

    CTexture *pVelocityMapTexture(g_ResourceManager.GetTexture(m_hVelocityTexture));
    if (pVelocityMapTexture != NULL)
        pVelocityMapTexture->SetIndex(m_iVelocityMap);

    m_bActive = true;
}


/*====================
  CVelocityMap::Release
  ====================*/
void    CVelocityMap::Release()
{
    SAFE_RELEASE(m_pDepthStencil);
    SAFE_RELEASE(m_pVelocityMapSurface);

    D3D_Unregister2DTexture(_T("$velocity"));

    m_iVelocityMap = g_iBlack;
    m_pVelocityMap = NULL;
    m_bActive = false;

    CTexture *pVelocityMapTexture(g_ResourceManager.GetTexture(m_hVelocityTexture));
    if (pVelocityMapTexture != NULL)
        pVelocityMapTexture->SetIndex(m_iVelocityMap);
}


/*====================
  CVelocityMap::Render
  ====================*/
void    CVelocityMap::Render(const CCamera &cCamera)
{
    PROFILE("CVelocityMap::Render");

    if (!vid_motionBlur)
        return;

    CTexture *pVelocityMapTexture(g_ResourceManager.GetTexture(m_hVelocityTexture));
    if (pVelocityMapTexture != NULL)
        pVelocityMapTexture->SetIndex(g_iBlack);

    if (!m_bActive || !g_bValidScene)
        return;

    // Set the new as the render target
    if (FAILED(g_pd3dDevice->SetRenderTarget(0, m_pVelocityMapSurface)))
    {
        Console.Err << _T("CVelocityMap::Render: SetRenderTarget failed") << newl;
        return;
    }

    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(m_pDepthStencil)))
    {
        Console.Err << _T("CVelocityMap::Render: SetDepthStencilSurface failed") << newl;
        return;
    }

    D3D_SetupCamera(cCamera);

    //g_Viewport.X = g_Viewport.X * m_uiWidth / g_CurrentVidMode.iWidth;
    //g_Viewport.Y = g_Viewport.Y * m_uiHeight / g_CurrentVidMode.iHeight;
    g_Viewport.X = 0;
    g_Viewport.Y = 0;
    g_Viewport.Width  = g_Viewport.Width * m_uiWidth / g_CurrentVidMode.iWidth;
    g_Viewport.Height = g_Viewport.Height * m_uiHeight / g_CurrentVidMode.iHeight;
    g_Viewport.MinZ = g_Viewport.MinZ;
    g_Viewport.MaxZ = g_Viewport.MaxZ;

    g_uiImageWidth = m_uiWidth;
    g_uiImageHeight = m_uiHeight;

    g_pd3dDevice->SetViewport(&g_Viewport);

    CVec4f v4ClearColor(0.5f, 0.5f, 1.0f, 0.0f);

    // Clear
    g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, v4ClearColor.GetAsDWord(), 1.0f, 0L);

    // Render reflected scene
    g_RenderList.Setup(PHASE_VELOCITY);
    g_RenderList.Sort();
    g_RenderList.Render(PHASE_VELOCITY);

    D3D_SelectMaterial(g_MaterialGUI, PHASE_COLOR, VERTEX_GUI, 0.0f, false);

    // Restore the old Render Target
    g_pd3dDevice->SetRenderTarget(0, g_pBackBuffer);

    // Restore the old DepthStencilSurface
    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(g_pDepthBuffer)))
        Console.Warn << _T("D3D_RenderScene: Failed to restore old DepthStencilSurface") << newl;

    if (pVelocityMapTexture != NULL)
        pVelocityMapTexture->SetIndex(m_iVelocityMap);
}
