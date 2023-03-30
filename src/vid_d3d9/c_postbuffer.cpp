// (C)2008 S2 Games
// c_postbuffer.cpp
//
// Post processing framework
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_postbuffer.h"

#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "C_renderlist.h"
#include "d3d9_texture.h"
#include "d3d9_shader.h"
#include "d3d9_scene.h"
#include "d3d9_terrain.h"
#include "d3d9_state.h"
#include "d3d9_util.h"
#include "d3d9_material.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
#include "../k2/c_posteffect.h"
#include "../k2/c_materialparameter.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CPostBuffer g_PostBuffer;

CVAR_BOOLF  (vid_postEffects,       true,       CVAR_SAVECONFIG);
CVAR_STRING (vid_postEffectPath,    "/core/post/bloom.posteffect");
CVAR_BOOLF  (vid_postEffectMipmaps, true,       CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CPostBuffer::CPostBuffer
  ====================*/
CPostBuffer::CPostBuffer() :
m_bActive(false),
m_hNullPostMaterial(INVALID_RESOURCE),
m_hPostColorReference(INVALID_RESOURCE),
m_iActiveBuffer(0),
m_uiWidth(0),
m_uiHeight(0)
{
    for (int i(0); i < 2; ++i)
    {
        m_aiPostBuffer[i] = -1;
        m_apPostBuffer[i] = NULL;
        m_apPostBufferSurface[i] = NULL;
        m_ahPostBufferHandle[i] = INVALID_RESOURCE;
    }
}


/*====================
  CPostBuffer::~CPostBuffer
  ====================*/
CPostBuffer::~CPostBuffer()
{
}


/*====================
  CPostBuffer::Initialize
  ====================*/
void    CPostBuffer::Initialize(int iWidth, int iHeight)
{
    PROFILE("CPostBuffer::Initialize");

    if (!vid_postEffects)
        return;

    vid_postEffects.SetModified(false);

#if 0
    m_uiWidth = 800;
    m_uiHeight = 800 * g_CurrentVidMode.iHeight / g_CurrentVidMode.iWidth;
#else
    m_uiWidth = iWidth;
    m_uiHeight = iHeight;
#endif

    Console.Video << _T("Using A8R8G8B8 Post Buffer ") << m_uiWidth << _T(" x ") << m_uiHeight << newl;

    for (int i(0); i < 2; ++i)
    {
        if (POST_EFFECTS_MIPMAP)
        {
            m_aiPostBuffer[i] = D3D_RegisterRenderTargetTexture(_T("$postbuffer") + XtoA(i), m_uiWidth, m_uiHeight, D3DFMT_A8R8G8B8, true);

            if (m_aiPostBuffer[i] == -1)
                continue;

            m_apPostBuffer[i] = g_pTextures2D[m_aiPostBuffer[i]];
            m_apPostBuffer[i]->GetSurfaceLevel(0, &m_apPostBufferSurface[i]);

            m_ahPostBufferHandle[i] = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CTexture)(_T("$postbuffer") + XtoA(i), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS, TEXFMT_A8R8G8B8), RES_TEXTURE);
        }
        else
        {
            m_aiPostBuffer[i] = D3D_RegisterRenderTargetTexture(_T("$postbuffer") + XtoA(i), m_uiWidth, m_uiHeight, D3DFMT_A8R8G8B8, false);

            if (m_aiPostBuffer[i] == -1)
                continue;

            m_apPostBuffer[i] = g_pTextures2D[m_aiPostBuffer[i]];
            m_apPostBuffer[i]->GetSurfaceLevel(0, &m_apPostBufferSurface[i]);

            m_ahPostBufferHandle[i] = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CTexture)(_T("$postbuffer") + XtoA(i), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
        }

        CTexture *pPostBufferTexture(g_ResourceManager.GetTexture(m_ahPostBufferHandle[i]));
        if (pPostBufferTexture != NULL)
            pPostBufferTexture->SetIndex(m_aiPostBuffer[i]);
    }

    for (int i(0); i < 2; ++i)
    {
        if (m_aiPostBuffer[i] == -1)
        {
            Release();
            return;
        }
    }

    m_hNullPostMaterial = g_ResourceManager.Register(_T("/core/null/post.material"), RES_MATERIAL);
    m_hPostColorReference = g_ResourceManager.Register(_T("!post_color"), RES_REFERENCE);

    m_bActive = true;

    // Precache post effect
    g_ResourceManager.Register(vid_postEffectPath, RES_POST_EFFECT);
}


/*====================
  CPostBuffer::Release
  ====================*/
void    CPostBuffer::Release()
{
    for (int i(0); i < 2; ++i)
    {
        SAFE_RELEASE(m_apPostBufferSurface[i]);
        D3D_Unregister2DTexture(_T("$postbuffer") + XtoA(i));
        m_aiPostBuffer[i] = -1;
        m_apPostBuffer[i] = NULL;
    }
    
    m_bActive = false;
}


/*====================
  CPostBuffer::Render
  ====================*/
void    CPostBuffer::Render()
{
    PROFILE("CPostBuffer::Render");

    if (!vid_postEffects)
        return;

    if (!m_bActive)
        return;

    if (m_uiWidth != g_Viewport.Width || m_uiHeight != g_Viewport.Height)
    {
        Release();
        Initialize(g_Viewport.Width, g_Viewport.Height);

        if (!m_bActive)
            return;
    }

    g_bInvertedProjection = false;
    
    g_bLighting = false;
    g_iNumActiveBones = 0;
    g_bShadows = false;
    g_bFogofWar = false;
    g_bFog = false;
    g_iNumActivePointLights = 0;
    g_bTexkill = false;

    m_iActiveBuffer = 0;

    // View matrix
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_mIdentity);
    g_mView = g_mIdentity;

    // World matrix
    g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mIdentity);
    g_mWorld = g_mIdentity;
    
    g_uiImageWidth = m_uiWidth;
    g_uiImageHeight = m_uiHeight;

    RECT cSrc;
    cSrc.left = g_Viewport.X;
    cSrc.top = g_Viewport.Y;
    cSrc.right = g_Viewport.X + g_Viewport.Width;
    cSrc.bottom = g_Viewport.Y + g_Viewport.Height;

    D3DVIEWPORT9 PostViewport;

    PostViewport.X = 0;
    PostViewport.Y = 0;
    PostViewport.Width = m_uiWidth;
    PostViewport.Height = m_uiHeight;
    PostViewport.MinZ = g_Viewport.MinZ;
    PostViewport.MaxZ = g_Viewport.MaxZ;

    RECT cDst;
    cDst.left = PostViewport.X;
    cDst.top = PostViewport.Y;
    cDst.right = PostViewport.X + PostViewport.Width;
    cDst.bottom = PostViewport.Y + PostViewport.Height;

    ResHandle hPostEffect(g_ResourceManager.Register(vid_postEffectPath, RES_POST_EFFECT));
    if (hPostEffect == INVALID_RESOURCE)
        return;

    CPostEffect *pPostEffect(g_ResourceManager.GetPostEffect(hPostEffect));
    if (pPostEffect == NULL)
        return;

    const vector<CPostFilter> &vFilters(pPostEffect->GetFilters());

    if (vFilters.size() == 0)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    D3D_SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

    // Copy back buffer to first post buffer
    g_pd3dDevice->StretchRect(g_pBackBuffer, &cSrc, GetCurrentSurface(), &cDst, D3DTEXF_POINT);

    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(NULL)))
        Console.Warn << _T("CPostBuffer::Render: SetDepthStencilSurface failed") << newl;

    float fImageWidth(1.0f);
    float fImageHeight(1.0f);

    // Perform post processing steps
    vector<CPostFilter>::const_iterator itEnd(vFilters.end());
    vector<CPostFilter>::const_iterator itBack(vFilters.end() - 1);

    for (vector<CPostFilter>::const_iterator it(vFilters.begin()); it != itEnd; ++it)
    {
        g_ResourceManager.UpdateReference(m_hPostColorReference, GetCurrentHandle());

        CMaterial &cMaterial(D3D_GetMaterial(it->GetMaterial()));

        if (it != itBack)
        {
            float fNudgeX(0.5f / m_uiWidth);
            float fNudgeY(0.5f / m_uiHeight);

            // Projection matrix
            D3DXMATRIXA16 mProj;
            D3DXMatrixOrthoOffCenterRH(&mProj, fNudgeX, 1.0f + fNudgeX, 1.0f + fNudgeY, fNudgeY, 0.0f, 1.0f);

            g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
            g_bInvertedProjection = false;

            g_mWorldViewProj = mProj;
        }
        else
        {
            float fNudgeX(0.5f / g_CurrentVidMode.iWidth);
            float fNudgeY(0.5f / g_CurrentVidMode.iHeight);

            // Projection matrix
            D3DXMATRIXA16 mProj;
            D3DXMatrixOrthoOffCenterRH(&mProj, fNudgeX, 1.0f + fNudgeX, 1.0f + fNudgeY, fNudgeY, 0.0f, 1.0f);

            g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
            g_bInvertedProjection = false;

            g_mWorldViewProj = mProj;
        }

        D3D_SelectMaterial(cMaterial, PHASE_COLOR, VERTEX_GUI, SceneManager.GetShaderTime(), false);

        if (it != itBack)
        {
            g_pd3dDevice->SetRenderTarget(0, GetNextSurface());

            g_pd3dDevice->SetViewport(&PostViewport);
        }
        else if (it != vFilters.begin())
        {
            g_pd3dDevice->SetRenderTarget(0, g_pBackBuffer);

            g_pd3dDevice->SetViewport(&g_Viewport);
        }

        // Scale rendered quad by fScaleU and fScaleV of the current material
        const IMaterialParameter *pScaleU(cMaterial.GetParameter(_T("fScaleU")));
        if (pScaleU != NULL)
            fImageWidth *= pScaleU->GetFloat(SceneManager.GetShaderTime());

        const IMaterialParameter *pScaleV(cMaterial.GetParameter(_T("fScaleV")));
        if (pScaleV != NULL)
            fImageHeight *= pScaleV->GetFloat(SceneManager.GetShaderTime());

        SGuiVertex *pVertices;
        if (SUCCEEDED(g_pVBPostBuffer->Lock(0, 4 * sizeof(SGuiVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
        {
            if (pVertices != NULL)
            {
                float fPaddedImageWidth(MIN(fImageWidth + 32.0f / g_uiImageWidth, 1.0f));
                float fPaddedImageHeight(MIN(fImageHeight + 32.0f / g_uiImageHeight, 1.0f));
                
                pVertices[0].x = 0.0f;
                pVertices[0].y = 0.0f;
                pVertices[0].tu = 0.0f;
                pVertices[0].tv = 0.0f;
                pVertices[0].color = 0xffffffff;

                pVertices[1].x = 0.0f;
                pVertices[1].y = fPaddedImageHeight;
                pVertices[1].tu = 0.0f;
                pVertices[1].tv = fPaddedImageHeight;
                pVertices[1].color = 0xffffffff;

                pVertices[2].x = fPaddedImageWidth;
                pVertices[2].y = fPaddedImageHeight;
                pVertices[2].tu = fPaddedImageWidth;
                pVertices[2].tv = fPaddedImageHeight;
                pVertices[2].color = 0xffffffff;

                pVertices[3].x = fPaddedImageWidth;
                pVertices[3].y = 0.0f;
                pVertices[3].tu = fPaddedImageWidth;
                pVertices[3].tv = 0.0f;
                pVertices[3].color = 0xffffffff;
            }

            g_pVBPostBuffer->Unlock();
        }

        D3D_SetStreamSource(0, g_pVBPostBuffer, 0, sizeof(SGuiVertex));
        D3D_DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

        ToggleBuffer();
    }

    if (FAILED(g_pd3dDevice->SetDepthStencilSurface(g_pDepthBuffer)))
        Console.Warn << _T("CPostBuffer::Render: Failed to restore old DepthStencilSurface") << newl;
}
