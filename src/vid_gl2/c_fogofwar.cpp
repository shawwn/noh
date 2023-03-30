// (C)2008 S2 Games
// c_fogofwar.cpp
//
// Fog of war render target
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_fogofwar.h"

#include "c_gfxutils.h"
#include "c_gfxterrain.h"
#include "c_gfxmaterials.h"
#include "c_gfxtextures.h"
#include "c_shaderpreprocessor.h"
#include "c_shaderregistry.h"

#include "../k2/c_camera.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_convexhull.h"
#include "../k2/c_orthofrustum.h"
#include "../k2/c_material.h"
#include "../k2/c_world.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CFogofWar	g_FogofWar;

#if 1
CVAR_INT	(vid_fogofwarmapSize,				512);
CVAR_INT	(vid_fogofwarmapMultisample,		-1);
CVAR_BOOL	(vid_fogofwarmapBlur,				true);
#else
CVAR_INT	(vid_fogofwarmapSize,				256);
CVAR_INT	(vid_fogofwarmapMultisample,		0);
CVAR_BOOL	(vid_fogofwarmapBlur,				false);
#endif
//=============================================================================

/*====================
  CFogofWar::CFogofWar
  ====================*/
CFogofWar::CFogofWar() :
m_uiFrameBufferObject(0),
m_uiColorTexture(0),
m_uiPixelBufferObject(0),
m_uiDynamicTexture0(0),
m_uiDynamicTexture1(0),
m_bValid(false),
m_hFogofWarTexture(INVALID_RESOURCE),
m_hFogofWarTexture0(INVALID_RESOURCE),
m_hFogofWarTexture1(INVALID_RESOURCE),
m_fWorldWidth(1.0f),
m_fWorldHeight(1.0f),
m_iNextTexturemap(0)
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
void	CFogofWar::Initialize()
{
	// Fail if we don't support framebuffer objects
	if (!GLEW_EXT_framebuffer_object)
	{
		Release();
		return;
	}

	// Vertex buffer
	glGenBuffersARB(1, &VBFowQuad);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBFowQuad);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_FOWQUADS * 4 * sizeof(SGuiVertex), NULL, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	//
	// Render target
	//

	// Initialize frame buffer object
	glGenFramebuffersEXT(1, &m_uiFrameBufferObject);
	glUseProgramObjectARB(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_uiFrameBufferObject);

	// Texture
	m_uiColorTexture = GfxTextures->RegisterRenderTargetTexture(_T("$fogofwar"), vid_fogofwarmapSize, vid_fogofwarmapSize, GL_RGBA8, false);

	PRINT_GLERROR_BREAK();

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, m_uiColorTexture);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_uiColorTexture, 0);

	PRINT_GLERROR_BREAK();

	bool bFailed(!GL_CheckFrameBufferStatus(_T("FogofWar")));

	// Finish initializing frame buffer object
	glUseProgramObjectARB(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (bFailed)
	{
		Release();
		m_bValid = false;
	}
	else
	{
		m_bValid = true;
	}

	PRINT_GLERROR_BREAK();

	//
	// Dynamic textures
	//

	int iSize(vid_fogofwarmapSize);

	if (vid_fogofwarmapMultisample > 0)
		iSize <<= vid_fogofwarmapMultisample;
	else if (vid_fogofwarmapMultisample < 0)
		iSize >>= -vid_fogofwarmapMultisample;

	m_uiDynamicTexture0 = GfxTextures->RegisterDynamicTexture(_T("$fogofwar0"), iSize, iSize, GL_ALPHA8, false);
	m_uiDynamicTexture1 = GfxTextures->RegisterDynamicTexture(_T("$fogofwar1"), iSize, iSize, GL_ALPHA8, false);

	glGenBuffersARB(1, &m_uiPixelBufferObject);

	CBitmap bmp(iSize, iSize, BITMAP_ALPHA);
	MemManager.Set(bmp.GetBuffer(), 255, bmp.GetSize());

	m_iNextTexturemap = 0;

	Update(bmp);
	Update(bmp);

	PRINT_GLERROR_BREAK();

	//
	// Register resource entries
	//

	m_hFogofWarTexture = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(_T("$fogofwar"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
	m_hFogofWarTexture0 = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(_T("$fogofwar0"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
	m_hFogofWarTexture1 = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(_T("$fogofwar1"), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);
}


/*====================
  CFogofWar::Release
  ====================*/
void	CFogofWar::Release()
{
	GL_SAFE_DELETE(glDeleteBuffersARB, VBFowQuad);
	GL_SAFE_DELETE(glDeleteFramebuffersEXT, m_uiFrameBufferObject);
	GfxTextures->UnregisterTexture(_T("$fogofwar"));
	GL_SAFE_DELETE(glDeleteBuffersARB, m_uiPixelBufferObject);
	GfxTextures->UnregisterTexture(_T("$fogofwar0"));
	GfxTextures->UnregisterTexture(_T("$fogofwar1"));

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

	if (!m_bValid)
	{
		//Console.Err << _T("CFogofWar::Render: Called before initialization") << newl;
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

	// Set the render target
	glUseProgramObjectARB(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_uiFrameBufferObject);

	PRINT_GLERROR_BREAK();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (GfxTerrain->pWorld)
	{
		m_fWorldWidth = GfxTerrain->pWorld->GetWorldWidth();
		m_fWorldHeight = GfxTerrain->pWorld->GetWorldHeight();

		glOrtho(0.0f, m_fWorldWidth, 0.0f, m_fWorldHeight, 0.0f, 1.0f);
	}
	else
	{
		glOrtho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

		m_fWorldWidth = 1.0f;
		m_fWorldHeight = 1.0f;
	}

	glViewport(0, 0, vid_fogofwarmapSize, vid_fogofwarmapSize);

	g_uiImageWidth = vid_fogofwarmapSize;
	g_uiImageHeight = vid_fogofwarmapSize;

	// Clear
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	PRINT_GLERROR_BREAK();

	g_bLighting = false;
	g_iNumActiveBones = 0;
	g_bShadows = false;
	g_bFogofWar = false;
	g_bFog = false;
	g_iNumActivePointLights = 0;

	// Render
	DrawTexture(fLerp);

	// Restore the old render target
	glUseProgramObjectARB(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	if (pFogofWarTexture)
		pFogofWarTexture->SetIndex(m_uiColorTexture);
}


/*====================
  CFogofWar::Update
  ====================*/
void	CFogofWar::Update(const CBitmap &cBmp)
{
	if (!m_bValid)
	{
		//Console.Err << _T("CFogofWar::Update: Called before initialization") << newl;
		return;
	}
#if 1
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_uiPixelBufferObject);

	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, cBmp.GetSize(), NULL, GL_STREAM_DRAW_ARB);
	GLubyte* pData((GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB));
	if (pData != NULL)
	{
		MemManager.Copy(pData, cBmp.GetBuffer(), cBmp.GetSize());
		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	}

	glActiveTextureARB(GL_TEXTURE0_ARB);
	if (m_iNextTexturemap == 0)
		glBindTexture(GL_TEXTURE_2D, m_uiDynamicTexture0);
	else
		glBindTexture(GL_TEXTURE_2D, m_uiDynamicTexture1);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cBmp.GetWidth(), cBmp.GetHeight(), GL_ALPHA, GL_UNSIGNED_BYTE, NULL);

	PRINT_GLERROR_BREAK();

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
#else
	glActiveTextureARB(GL_TEXTURE0_ARB);
	if (m_iNextTexturemap == 0)
		glBindTexture(GL_TEXTURE_2D, m_uiDynamicTexture0);
	else
		glBindTexture(GL_TEXTURE_2D, m_uiDynamicTexture1);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, cBmp.GetWidth(), cBmp.GetHeight(), GL_ALPHA, GL_UNSIGNED_BYTE, cBmp.GetBuffer());
#endif

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
		Console.Err << _T("CFogofWar::DrawTexture() - Called before initialization") << newl;
		return;
	}

	PROFILE("CFogofWar::DrawTexture");

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBFowQuad);
	
	SGuiVertex cVertices[MAX_FOWQUADS * 4];
	SGuiVertex* pVertices(cVertices);
	
	CVec4f v4Color0(1.0f, 1.0f, 1.0f, 1.0f - fLerp);
	CVec4f v4Color1(1.0f, 1.0f, 1.0f, fLerp);

	// Bottom texture
	pVertices->x = 0.0f;
	pVertices->y = 0.0f;
	pVertices->color = v4Color0;
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	pVertices->x = 0.0f;
	pVertices->y = m_fWorldHeight;
	pVertices->color = v4Color0;
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = m_fWorldHeight;
	pVertices->color = v4Color0;
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = 0.0f;
	pVertices->color = v4Color0;
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	// Top texture
	pVertices->x = 0.0f;
	pVertices->y = 0.0f;
	pVertices->color = v4Color1;
	pVertices->tu = 0.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	pVertices->x = 0.0f;
	pVertices->y = m_fWorldHeight;
	pVertices->color = v4Color1;
	pVertices->tu = 0.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = m_fWorldHeight;
	pVertices->color = v4Color1;
	pVertices->tu = 1.0f;
	pVertices->tv = 1.0f;
	++pVertices;

	pVertices->x = m_fWorldWidth;
	pVertices->y = 0.0f;
	pVertices->color = v4Color1;
	pVertices->tu = 1.0f;
	pVertices->tv = 0.0f;
	++pVertices;

	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_FOWQUADS * 4 * sizeof(SGuiVertex), &cVertices, GL_STREAM_DRAW_ARB);
	
	CMaterial &material(vid_fogofwarmapBlur ? g_MaterialGUIBlur : g_MaterialGUI);

	ResHandle hBottom(m_iNextTexturemap ? m_hFogofWarTexture1 : m_hFogofWarTexture0);
	ResHandle hTop(m_iNextTexturemap ? m_hFogofWarTexture0 : m_hFogofWarTexture1);

	material.GetPhase(PHASE_COLOR).GetSampler(0).SetTexture(hBottom);
	material.GetPhase(PHASE_COLOR).SetSrcBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetDstBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetCullMode(CULL_NONE);
	GfxMaterials->SelectMaterial(material, PHASE_COLOR, 0.0f, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBFowQuad);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(24));
	glColorPointer(4, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(8));
	glVertexPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(0));

	glDrawArrays(GL_QUADS, 0, 4);

	material.GetPhase(PHASE_COLOR).GetSampler(0).SetTexture(hTop);
	material.GetPhase(PHASE_COLOR).SetSrcBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetDstBlend(BLEND_ONE);
	material.GetPhase(PHASE_COLOR).SetCullMode(CULL_NONE);
	GfxMaterials->SelectMaterial(material, PHASE_COLOR, 0.0f, 0);

	glDrawArrays(GL_QUADS, 4, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

