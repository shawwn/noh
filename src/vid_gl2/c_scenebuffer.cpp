// (C)2009 S2 Games
// c_scenebuffer.cpp
//
// Scene buffer for refraction and other post processing effects
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_scenebuffer.h"

#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"
#include "c_renderlist.h"
#include "c_gfxtextures.h"

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
CSceneBuffer    g_SceneBuffer;

CVAR_BOOLF  (vid_sceneBuffer,           true,               CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_sceneBufferMipmap,     true,               CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CSceneBuffer::CSceneBuffer
  ====================*/
CSceneBuffer::CSceneBuffer() :
m_bActive(false),
m_uiWidth(0),
m_uiHeight(0)
{
    m_uiSceneBuffer = 0;
}


/*====================
  CSceneBuffer::~CSceneBuffer
  ====================*/
CSceneBuffer::~CSceneBuffer()
{
}


/*====================
  CSceneBuffer::Initialize
  ====================*/
void    CSceneBuffer::Initialize(int iWidth, int iHeight)
{
    PROFILE("CSceneBuffer::Initialize");

    if (!vid_sceneBuffer)
    {
        m_uiSceneBuffer = GfxTextures->GetWhiteTexture();
        return;
    }

#if 0
    m_uiWidth = 800;
    m_uiHeight = 800 * g_CurrentVidMode.iHeight / g_CurrentVidMode.iWidth;
#else
    m_uiWidth = iWidth;
    m_uiHeight = iHeight;
#endif

    Console.Video << _T("Using A8R8G8B8 Scene Buffer ") << m_uiWidth << _T(" x ") << m_uiHeight << newl;

    m_uiSceneBuffer = GfxTextures->RegisterRenderTargetTexture(_T("$scene"), m_uiWidth, m_uiHeight, GL_RGBA8, vid_sceneBufferMipmap);

    if (m_uiSceneBuffer == 0)
    {
        m_bActive = false;
        return;
    }

    int iTextureFlags(TEX_FULL_QUALITY | TEX_NO_COMPRESS);
    if (!vid_sceneBufferMipmap)
        iTextureFlags |= TEX_NO_MIPMAPS;

    m_hSceneBufferTexture = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(_T("$scene"), TEXTURE_2D, iTextureFlags, TEXFMT_A8R8G8B8), RES_TEXTURE);

    // Update reference texture
    CTexture *pSceneBufferTexture(g_ResourceManager.GetTexture(m_hSceneBufferTexture));
    if (pSceneBufferTexture != NULL)
        pSceneBufferTexture->SetIndex(m_uiSceneBuffer);

    m_bActive = true;
}


/*====================
  CSceneBuffer::Release
  ====================*/
void    CSceneBuffer::Release()
{
    GfxTextures->UnregisterTexture(_T("$scene"));

    // Update reference texture
    CTexture *pSceneBufferTexture(g_ResourceManager.GetTexture(m_hSceneBufferTexture));
    if (pSceneBufferTexture != NULL)
        pSceneBufferTexture->SetIndex(m_uiSceneBuffer);
    
    m_bActive = false;
}


/*====================
  CSceneBuffer::Render
  ====================*/
void    CSceneBuffer::Render()
{
    PROFILE("CSceneBuffer::Render");

    if (!vid_sceneBuffer)
        return;

    if (!m_bActive)
        return;

    const CCamera &cCamera(*g_pCam);

    int iSceneX(INT_ROUND(cCamera.GetX()));
    int iSceneY(INT_ROUND(cCamera.GetY()));
    int iSceneWidth(INT_ROUND(cCamera.GetWidth()));
    int iSceneHeight(INT_ROUND(cCamera.GetHeight()));

    int iViewportX(iSceneX);
    int iViewportY(g_iScreenHeight - (iSceneY + iSceneHeight));

    if (m_uiWidth != iSceneWidth || m_uiHeight != iSceneHeight)
    {
        Release();
        Initialize(iSceneWidth, iSceneHeight);

        if (!m_bActive)
            return;
    }

    vid_sceneBuffer.SetModified(false);

    g_uiImageWidth = m_uiWidth;
    g_uiImageHeight = m_uiHeight;

    g_fSceneScaleX = 1.0f;
    g_fSceneScaleY = 1.0f;

    // Copy back buffer to scene buffer
    glBindTexture(GL_TEXTURE_2D, m_uiSceneBuffer);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iViewportX, iViewportY, m_uiWidth, m_uiHeight);
    glBindTexture(GL_TEXTURE_2D, 0);
}
