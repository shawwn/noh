// (C)2008 S2 Games
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
#include "../k2/c_posteffect.h"
#include "../k2/c_materialparameter.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CSceneBuffer	g_SceneBuffer;

CVAR_BOOLF	(vid_sceneBuffer,			true,				CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_sceneBufferMipmap,		true,				CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CSceneBuffer::CSceneBuffer
  ====================*/
CSceneBuffer::CSceneBuffer() :
m_bActive(false),
m_uiWidth(0),
m_uiHeight(0)
{
	m_iSceneBuffer = -1;
	m_pSceneBuffer = NULL;
	m_pSceneBufferSurface = NULL;
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
void	CSceneBuffer::Initialize(int iWidth, int iHeight)
{
	PROFILE("CSceneBuffer::Initialize");

	if (!vid_sceneBuffer || true)
	{
		m_iSceneBuffer = g_iWhite;
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

	m_iSceneBuffer = D3D_RegisterRenderTargetTexture(_T("$scene"), m_uiWidth, m_uiHeight, D3DFMT_A8R8G8B8, vid_sceneBufferMipmap);

	if (m_iSceneBuffer == -1)
	{
		m_bActive = false;
		return;
	}

	m_pSceneBuffer = g_pTextures2D[m_iSceneBuffer];
	m_pSceneBuffer->GetSurfaceLevel(0, &m_pSceneBufferSurface);

	int iTextureFlags(TEX_FULL_QUALITY | TEX_NO_COMPRESS);
	if (!vid_sceneBufferMipmap)
		iTextureFlags |= TEX_NO_MIPMAPS;

	m_hSceneBufferTexture = g_ResourceManager.Register(new CTexture(_T("$scene"), TEXTURE_2D, iTextureFlags, TEXFMT_A8R8G8B8), RES_TEXTURE);

	// Update reference texture
	CTexture *pSceneBufferTexture(g_ResourceManager.GetTexture(m_hSceneBufferTexture));
	if (pSceneBufferTexture != NULL)
		pSceneBufferTexture->SetIndex(m_iSceneBuffer);

	m_bActive = true;
}


/*====================
  CSceneBuffer::Release
  ====================*/
void	CSceneBuffer::Release()
{
	SAFE_RELEASE(m_pSceneBufferSurface);
	D3D_Unregister2DTexture(_T("$scene"));
	m_iSceneBuffer = g_iWhite;
	m_pSceneBuffer = NULL;

	// Update reference texture
	CTexture *pSceneBufferTexture(g_ResourceManager.GetTexture(m_hSceneBufferTexture));
	if (pSceneBufferTexture != NULL)
		pSceneBufferTexture->SetIndex(m_iSceneBuffer);
	
	m_bActive = false;
}


/*====================
  CSceneBuffer::Render
  ====================*/
void	CSceneBuffer::Render()
{
	PROFILE("CSceneBuffer::Render");

	if (!vid_sceneBuffer)
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

	vid_sceneBuffer.SetModified(false);
	vid_sceneBufferMipmap.SetModified(false);

	g_uiImageWidth = m_uiWidth;
	g_uiImageHeight = m_uiHeight;

	g_fSceneScaleX = 1.0f;
	g_fSceneScaleY = g_Viewport.Width / g_Viewport.Height;

	D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

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

	// Copy back buffer to scene buffer
	g_pd3dDevice->StretchRect(g_pBackBuffer, &cSrc, m_pSceneBufferSurface, &cDst, D3DTEXF_POINT);
}
