// (C)2009 S2 Games
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
#include "c_renderlist.h"
#include "c_gfxtextures.h"
#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
#include "c_gfx2d.h"

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
CPostBuffer	g_PostBuffer;

CVAR_BOOLF	(vid_postEffects,		true,		CVAR_SAVECONFIG);
CVAR_STRING	(vid_postEffectPath,	"/core/post/bloom.posteffect");
CVAR_BOOLF	(vid_postEffectMipmaps,	true,		CVAR_SAVECONFIG);
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
		m_auiFrameBufferObject[i] = 0;
		m_auiColorTexture[i] = 0;
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
void	CPostBuffer::Initialize(int iWidth, int iHeight)
{
	PROFILE("CPostBuffer::Initialize");

	if (!vid_postEffects)
		return;

	// Fail if we don't support framebuffer objects
	if (!GLEW_EXT_framebuffer_object)
	{
		Release();
		return;
	}

	vid_postEffects.SetModified(false);

	// Vertex buffer
	glGenBuffersARB(1, &VBPostQuad);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBPostQuad);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4 * sizeof(SGuiVertex), NULL, GL_STREAM_DRAW_ARB);

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
		// Initialize frame buffer object
		glGenFramebuffersEXT(1, &m_auiFrameBufferObject[i]);
		glUseProgramObjectARB(0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_auiFrameBufferObject[i]);

		// Texture
		m_auiColorTexture[i] = GfxTextures->RegisterRenderTargetTexture(_T("$postbuffer") + XtoA(i), m_uiWidth, m_uiHeight, GL_RGBA8, false);

		PRINT_GLERROR_BREAK();

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, m_auiColorTexture[i]);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_auiColorTexture[i], 0);

		PRINT_GLERROR_BREAK();

		bool bFailed(!GL_CheckFrameBufferStatus(_T("PostBuffer") + XtoA(i)));

		// Finish initializing frame buffer object
		glUseProgramObjectARB(0);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		if (bFailed)
			break;

		m_ahPostBufferHandle[i] = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CTexture)(_T("$postbuffer") + XtoA(i), TEXTURE_2D, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, TEXFMT_A8R8G8B8), RES_TEXTURE);

		CTexture *pPostBufferTexture(g_ResourceManager.GetTexture(m_ahPostBufferHandle[i]));
		if (pPostBufferTexture != NULL)
			pPostBufferTexture->SetIndex(m_auiColorTexture[i]);
	}

	for (int i(0); i < 2; ++i)
	{
		if (m_auiColorTexture[i] == 0)
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

	PRINT_GLERROR_BREAK();
}


/*====================
  CPostBuffer::Release
  ====================*/
void	CPostBuffer::Release()
{
	GL_SAFE_DELETE(glDeleteBuffersARB, VBPostQuad);

	for (int i(0); i < 2; ++i)
	{
		GL_SAFE_DELETE(glDeleteFramebuffersEXT, m_auiFrameBufferObject[i]);
		GfxTextures->UnregisterTexture(_T("$postbuffer") + XtoA(i));
		m_auiColorTexture[i] = 0;
	}
	
	m_bActive = false;

	PRINT_GLERROR_BREAK();
}


/*====================
  CPostBuffer::Render
  ====================*/
void	CPostBuffer::Render()
{
	PROFILE("CPostBuffer::Render");

	if (!vid_postEffects)
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

	m_iActiveBuffer = 0;

	g_bInvertedProjection = false;
	
	g_bLighting = false;
	g_iNumActiveBones = 0;
	g_bShadows = false;
	g_bFogofWar = false;
	g_bFog = false;
	g_iNumActivePointLights = 0;
	g_bTexkill = false;

	g_uiImageWidth = m_uiWidth;
	g_uiImageHeight = m_uiHeight;

	ResHandle hPostEffect(g_ResourceManager.Register(vid_postEffectPath, RES_POST_EFFECT));
	if (hPostEffect == INVALID_RESOURCE)
		return;

	CPostEffect *pPostEffect(g_ResourceManager.GetPostEffect(hPostEffect));
	if (pPostEffect == NULL)
		return;

	const vector<CPostFilter> &vFilters(pPostEffect->GetFilters());

	if (vFilters.size() == 0)
		return;

	// Copy back buffer to first post buffer
	glBindTexture(GL_TEXTURE_2D, GetCurrentTexture());
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iViewportX, iViewportY, m_uiWidth, m_uiHeight);

	glBindTexture(GL_TEXTURE_2D, 0);

	float fImageWidth(1.0f);
	float fImageHeight(1.0f);

	// Perform post processing steps
	vector<CPostFilter>::const_iterator itEnd(vFilters.end());
	vector<CPostFilter>::const_iterator itBack(vFilters.end() - 1);

	PRINT_GLERROR_BREAK();

	for (vector<CPostFilter>::const_iterator it(vFilters.begin()); it != itEnd; ++it)
	{
		g_ResourceManager.UpdateReference(m_hPostColorReference, GetCurrentHandle());

		CMaterial &cMaterial(GfxUtils->GetMaterial(it->GetMaterial()));

		if (it != itBack)
		{
			glUseProgramObjectARB(0);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, GetNextFBO());
			glViewport(0, 0, m_uiWidth, m_uiHeight);
		}
		else if (it != vFilters.begin())
		{
			glUseProgramObjectARB(0);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			glViewport(iSceneX, g_iScreenHeight - (iSceneY + iSceneHeight), iSceneWidth, iSceneHeight);
		}

		// Scale rendered quad by fScaleU and fScaleV of the current material
		const IMaterialParameter *pScaleU(cMaterial.GetParameter(_T("fScaleU")));
		if (pScaleU != NULL)
			fImageWidth *= pScaleU->GetFloat(SceneManager.GetShaderTime());

		const IMaterialParameter *pScaleV(cMaterial.GetParameter(_T("fScaleV")));
		if (pScaleV != NULL)
			fImageHeight *= pScaleV->GetFloat(SceneManager.GetShaderTime());

		if (it != itBack)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		}
		else
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		}

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBPostQuad);
		SGuiVertex cVertices[4];
		SGuiVertex* pVertices(cVertices);

		float fPaddedImageWidth(MIN(fImageWidth + 32.0f / g_uiImageWidth, 1.0f));
		float fPaddedImageHeight(MIN(fImageHeight + 32.0f / g_uiImageHeight, 1.0f));
		
		pVertices[0].x = 0.0f;
		pVertices[0].y = 0.0f;
		pVertices[0].tu = 0.0f;
		pVertices[0].tv = 0.0f;
		pVertices[0].color = WHITE;

		pVertices[1].x = 0.0f;
		pVertices[1].y = fPaddedImageHeight;
		pVertices[1].tu = 0.0f;
		pVertices[1].tv = fPaddedImageHeight;
		pVertices[1].color = WHITE;

		pVertices[2].x = fPaddedImageWidth;
		pVertices[2].y = fPaddedImageHeight;
		pVertices[2].tu = fPaddedImageWidth;
		pVertices[2].tv = fPaddedImageHeight;
		pVertices[2].color = WHITE;

		pVertices[3].x = fPaddedImageWidth;
		pVertices[3].y = 0.0f;
		pVertices[3].tu = fPaddedImageWidth;
		pVertices[3].tv = 0.0f;
		pVertices[3].color = WHITE;

		glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4 * sizeof(SGuiVertex), &cVertices, GL_STREAM_DRAW_ARB);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBPostQuad);
		glTexCoordPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(24));
		glColorPointer(4, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(8));
		glVertexPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(0));

		GfxMaterials->SelectMaterial(cMaterial, PHASE_COLOR, SceneManager.GetShaderTime(), false);

		glDrawArrays(GL_QUADS, 0, 4);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

		ToggleBuffer();
	}

	PRINT_GLERROR_BREAK();
}
