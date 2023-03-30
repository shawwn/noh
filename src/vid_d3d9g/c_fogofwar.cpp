// (C)2007 S2 Games
// c_fogofwar.cpp
//
// Fog of war render target
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

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
#include "c_fogofwar.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
#include "../k2/c_world.h"
#include "../k2/c_bitmap.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CFogofWar	g_FogofWar;

#if 1
CVAR_BOOL	(vid_fogofwarmapMultisample,		true);
CVAR_BOOL	(vid_fogofwarmapBlur,				true);
#else
CVAR_BOOL	(vid_fogofwarmapMultisample,		false);
CVAR_BOOL	(vid_fogofwarmapBlur,				false);
#endif
//=============================================================================

/*====================
  CFogofWar::CFogofWar
  ====================*/
CFogofWar::CFogofWar() :
m_bValid(false),
m_iTexturemap(-1),
m_hFogofWarTexture(INVALID_RESOURCE),
m_hFogofWarTexture0(INVALID_RESOURCE),
m_hFogofWarTexture1(INVALID_RESOURCE),
m_fWorldWidth(1.0f),
m_fWorldHeight(1.0f),
m_iNextTexturemap(0),
m_uiBMPWidth(0),
m_uiBMPHeight(0),
m_uiRTWidth(0),
m_uiRTHeight(0),
m_iSize(0)
{
}


/*====================
  CFogofWar::~CFogofWar
  ====================*/
CFogofWar::~CFogofWar()
{
}


/*====================
  CFogofWar::Initialize
  ====================*/
void	CFogofWar::Initialize(uint uiWidth, uint uiHeight, int iSize)
{
	m_uiBMPWidth = uiWidth;
	m_uiBMPHeight = uiHeight;
	m_uiRTWidth = uiWidth;
	m_uiRTHeight = uiHeight;
	m_uiFilterWidth = uiWidth;
	m_uiFilterHeight = uiHeight;
	m_iSize = iSize;

	if (uiWidth == 0 || uiHeight == 0)
	{
		Release();
		return;
	}

	if (vid_fogofwarmapMultisample)
	{
		int iMultisample(CLAMP(1 - iSize, -1, 1));

		if (iMultisample > 0)
		{
			m_uiRTWidth >>= iMultisample;
			m_uiRTHeight >>= iMultisample;
		}
		else if (iMultisample < 0)
		{
			m_uiRTWidth <<= -iMultisample;
			m_uiRTHeight <<= -iMultisample;
		}
	}

	m_iTexturemap = D3D_RegisterRenderTargetTexture(_T("$fogofwar"), m_uiRTWidth, m_uiRTHeight, D3DFMT_A8R8G8B8, false);
	m_pTexturemap = g_pTextures2D[m_iTexturemap];
	m_pTexturemap->GetSurfaceLevel(0, &m_pTexturemapSurface);

	ETextureFormat eFmt(TEXFMT_A8R8G8B8);

	if (g_DeviceCaps.bA8)
		eFmt = TEXFMT_A8;
	else if (g_DeviceCaps.bA8L8)
		eFmt = TEXFMT_A8L8;

	CBitmap bmp(m_uiBMPWidth, m_uiBMPHeight, BITMAP_ALPHA);

	MemManager.Set(bmp.GetBuffer(), 255, bmp.GetSize());

	m_iDynamicTexturemap0 = D3D_RegisterDynamicTexture(_T("$fogofwar0"), bmp, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt);
	if (m_iDynamicTexturemap0 == -1)
	{
		Release();
		return;
	}

	m_iDynamicTexturemap1 = D3D_RegisterDynamicTexture(_T("$fogofwar1"), bmp, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt);
	if (m_iDynamicTexturemap1 == -1)
	{
		Release();
		return;
	}

	m_hFogofWarTexture = g_ResourceManager.Register(new CTexture(_T("$fogofwar"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
	m_hFogofWarTexture0 = g_ResourceManager.Register(new CTexture(_T("$fogofwar0"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
	m_hFogofWarTexture1 = g_ResourceManager.Register(new CTexture(_T("$fogofwar1"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);

	m_iNextTexturemap = 0;
	m_bValid = true;
}


/*====================
  CFogofWar::Release
  ====================*/
void	CFogofWar::Release()
{
	SAFE_RELEASE(m_pTexturemapSurface);

	D3D_Unregister2DTexture(_T("$fogofwar"));
	D3D_Unregister2DTexture(_T("$fogofwar0"));
	D3D_Unregister2DTexture(_T("$fogofwar1"));

	m_pTexturemap = NULL;
	m_iTexturemap = -1;
	m_iDynamicTexturemap0 = -1;
	m_iDynamicTexturemap1 = -1;
	m_iNextTexturemap = 0;

	m_bValid = false;
}


/*====================
  CFogofWar::Render
  ====================*/
void	CFogofWar::Render(float fClear, bool bTexture, float fLerp)
{
	CTexture *pFogofWarTexture(g_ResourceManager.GetTexture(m_hFogofWarTexture));
	if (pFogofWarTexture)
		pFogofWarTexture->SetIndex(-1);

	if (!m_bValid || !g_bValidScene)
	{
		//Console.Err << _T("CFogofWar::Render: Called before initialization") << newl;
		return;
	}

	// Set the new as the render target
	if (FAILED(g_pd3dDevice->SetRenderTarget(0, m_pTexturemapSurface)))
	{
		Console.Err << _T("CFogofWar::Render: SetRenderTarget failed") << newl;
		return;
	}

	if (FAILED(g_pd3dDevice->SetDepthStencilSurface(NULL)))
	{
		Console.Err << _T("CFogofWar::Render: SetDepthStencilSurface failed") << newl;
		return;
	}

	g_Viewport.X = 0;
	g_Viewport.Y = 0;
	g_Viewport.Width  = m_uiRTWidth;
	g_Viewport.Height = m_uiRTHeight;
	g_Viewport.MinZ = 0.0f;
	g_Viewport.MaxZ = 1.0f;

	g_uiImageWidth = m_uiRTWidth;
	g_uiImageHeight = m_uiRTHeight;

	g_pd3dDevice->SetViewport(&g_Viewport);

	// Clear
	g_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, BYTE_ROUND(fClear * 255.0f)), 1.0f, 0);

	if (terrain.pWorld)
	{
		m_fWorldWidth = terrain.pWorld->GetWorldWidth();
		m_fWorldHeight = terrain.pWorld->GetWorldHeight();
	}
	else
	{
		m_fWorldWidth = 1.0f;
		m_fWorldHeight = 1.0f;
	}

	float fNudgeX(0.5f * m_fWorldWidth / m_uiRTWidth);
	float fNudgeY(0.5f * m_fWorldHeight / m_uiRTHeight);

	// Projection matrix
	D3DXMATRIXA16 mProj;
	D3DXMatrixOrthoOffCenterRH(&mProj, fNudgeX, m_fWorldWidth + fNudgeX, m_fWorldHeight + fNudgeY, fNudgeY, 0.0f, 1.0f);

	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);
	g_bInvertedProjection = false;

	// View matrix
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &g_mIdentity);
	g_mView = g_mIdentity;

	// World matrix
	g_pd3dDevice->SetTransform(D3DTS_WORLDMATRIX(0), &g_mIdentity);
	g_mWorld = g_mIdentity;
	g_mWorldViewProj = mProj;

	D3D_SetRenderState(D3DRS_ZENABLE, FALSE);
	D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	D3D_SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	D3D_SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	D3D_SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

	g_bLighting = false;
	g_iNumActiveBones = 0;
	g_bShadows = false;
	g_bFogofWar = false;
	g_bFog = false;
	g_iNumActivePointLights = 0;
	g_iTexcoords = 1;
	g_bTexkill = false;

	if (bTexture)
	{
		CTexture *pFogofWarTexture0(g_ResourceManager.GetTexture(m_hFogofWarTexture0));
		if (pFogofWarTexture0)
			pFogofWarTexture0->SetIndex(m_iDynamicTexturemap0);

		CTexture *pFogofWarTexture1(g_ResourceManager.GetTexture(m_hFogofWarTexture1));
		if (pFogofWarTexture1)
			pFogofWarTexture1->SetIndex(m_iDynamicTexturemap1);

		DrawTexture(fLerp);
	}

	// Restore the old render target
	g_pd3dDevice->SetRenderTarget(0, g_pBackBuffer);

	// Restore the old DepthStencilSurface
	if (g_pDepthBuffer)
	{
		if (FAILED(g_pd3dDevice->SetDepthStencilSurface(g_pDepthBuffer)))
			Console.Warn << _T("CFogofWar::Render: Failed to restore old DepthStencilSurface") << newl;
	}

	if (pFogofWarTexture)
		pFogofWarTexture->SetIndex(m_iTexturemap);
}


/*====================
  CFogofWar::Update
  ====================*/
void	CFogofWar::Update(const CBitmap &cBmp)
{
	if (!m_bValid)
		return;

	ETextureFormat eFmt(TEXFMT_A8R8G8B8);

	if (g_DeviceCaps.bA8)
		eFmt = TEXFMT_A8;
	else if (g_DeviceCaps.bA8L8)
		eFmt = TEXFMT_A8L8;

	if (m_iNextTexturemap == 0)
		D3D_UpdateDynamicTexture(m_iDynamicTexturemap0, cBmp, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt);
	else
		D3D_UpdateDynamicTexture(m_iDynamicTexturemap1, cBmp, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt);

	m_iNextTexturemap = TOGGLE(m_iNextTexturemap);
}


/*====================
  CFogofWar::DrawTexture

  Draw dynamic fog of war texture
  ====================*/
void	CFogofWar::DrawTexture(float fLerp)
{
	if (!m_bValid)
	{
		//Console.Err << _T("CFogofWar::DrawTexture: Called before initialization") << newl;
		return;
	}

	PROFILE("CFogofWar::DrawTexture");

	SGuiVertex* pVertices;
			
	if (g_dwFowQuadBase + 2 >= MAX_FOWQUADS)
		g_dwFowQuadBase = 0;

	if (FAILED(g_pVBFowQuad->Lock(g_dwFowQuadBase * 4 * sizeof(SGuiVertex), 2 * 4 * sizeof(SGuiVertex), (void**)&pVertices, g_dwFowQuadBase ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)))
		return;

	dword dwColor0(D3DCOLOR_ARGB(BYTE_ROUND((1.0f - fLerp) * 255.0f), 255, 255, 255));
	dword dwColor1(D3DCOLOR_ARGB(BYTE_ROUND(fLerp * 255.0f), 255, 255, 255));

	// Bottom texture
	pVertices->x = 0.0f;
	pVertices->y = 0.0f;
	pVertices->color = dwColor0;
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	pVertices->x = 0.0f;
	pVertices->y = m_fWorldHeight;
	pVertices->color = dwColor0;
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = m_fWorldHeight;
	pVertices->color = dwColor0;
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = 0.0f;
	pVertices->color = dwColor0;
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	// Top texture
	pVertices->x = 0.0f;
	pVertices->y = 0.0f;
	pVertices->color = dwColor1;
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	pVertices->x = 0.0f;
	pVertices->y = m_fWorldHeight;
	pVertices->color = dwColor1;
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = m_fWorldHeight;
	pVertices->color = dwColor1;
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = 0.0f;
	pVertices->color = dwColor1;
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	g_pVBFowQuad->Unlock();

	D3D_SetStreamSource(0, g_pVBFowQuad, 0, sizeof(SGuiVertex));
	D3D_SetIndices(g_pIBFowQuad);

	CMaterial &material(vid_fogofwarmapBlur ? g_MaterialGUIBlur : g_MaterialGUI);

	ResHandle hBottom(m_iNextTexturemap ? m_hFogofWarTexture1 : m_hFogofWarTexture0);
	ResHandle hTop(m_iNextTexturemap ? m_hFogofWarTexture0 : m_hFogofWarTexture1);

	material.GetPhase(PHASE_COLOR).GetSampler(0).SetTexture(hBottom);
	material.GetPhase(PHASE_COLOR).SetSrcBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetDstBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetCullMode(CULL_NONE);

	D3D_SelectMaterial(material, PHASE_COLOR, VERTEX_GUI, SceneManager.GetShaderTime(), false);

	D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (g_dwFowQuadBase) * 4, 4, (g_dwFowQuadBase) * 6, 2);

	D3D_UpdateShaderTexture(_T("image"), hTop);

	D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, (g_dwFowQuadBase + 1) * 4, 4, (g_dwFowQuadBase + 1) * 6, 2);

	g_dwFowQuadBase += 2;
}

