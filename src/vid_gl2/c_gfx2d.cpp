// (C)2008 S2 Games
// c_gfx2d.cpp
//
// 2D Rendering
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfx2d.h"

#include "c_gfxutils.h"
#include "c_gfx3d.h"
#include "c_gfxmaterials.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
SINGLETON_INIT(CGfx2D)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfx2D *Gfx2D(CGfx2D::GetInstance());

CONST_STRING(IMAGE, _T("image"));
//=============================================================================

/*====================
  CGfx2D::~CGfx2D
  ====================*/
CGfx2D::~CGfx2D()
{
	ForceEmpty();
}


/*====================
  CGfx2D::CGfx2D
  ====================*/
CGfx2D::CGfx2D() :
VBGuiQuad(0),
iNumGuiElements(0)
{
}


/*====================
  CGfx2D::Init
  ====================*/
void	CGfx2D::Init()
{
	// Gui Quad buffer
	glGenBuffersARB(1, &VBGuiQuad);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBGuiQuad);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_GUIQUADS * 4 * sizeof(SGuiVertex), NULL, GL_STREAM_DRAW_ARB);
}


/*====================
  CGfx2D::AddRect
  ====================*/
void	CGfx2D::AddRect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags)
{
	if (iNumGuiElements >= MAX_GUIQUADS)
		Draw();
	if (iNumGuiElements >= MAX_GUIQUADS)
		return;
	if (hTexture == INVALID_RESOURCE)
	{
		Console.Warn << _T("CGfx2D::AddRect() - Invalid texture handle") << newl;
		return;
	}

	GuiElements[iNumGuiElements].eType = GUI_RECT;
	GuiElements[iNumGuiElements].x[0] = x;
	GuiElements[iNumGuiElements].y[0] = y;
	GuiElements[iNumGuiElements].x[1] = GuiElements[iNumGuiElements].x[0];
	GuiElements[iNumGuiElements].y[1] = GuiElements[iNumGuiElements].y[0]+h;
	GuiElements[iNumGuiElements].x[2] = GuiElements[iNumGuiElements].x[0]+w;
	GuiElements[iNumGuiElements].y[2] = GuiElements[iNumGuiElements].y[0]+h;
	GuiElements[iNumGuiElements].x[3] = GuiElements[iNumGuiElements].x[0]+w;
	GuiElements[iNumGuiElements].y[3] = GuiElements[iNumGuiElements].y[0];

	GuiElements[iNumGuiElements].s[0] = s1;
	GuiElements[iNumGuiElements].t[0] = t2;
	GuiElements[iNumGuiElements].s[1] = s1;
	GuiElements[iNumGuiElements].t[1] = t1;
	GuiElements[iNumGuiElements].s[2] = s2;
	GuiElements[iNumGuiElements].t[2] = t1;
	GuiElements[iNumGuiElements].s[3] = s2;
	GuiElements[iNumGuiElements].t[3] = t2;

	GuiElements[iNumGuiElements].color[R] = GfxUtils->GetCurrentColor(R);
	GuiElements[iNumGuiElements].color[G] = GfxUtils->GetCurrentColor(G);
	GuiElements[iNumGuiElements].color[B] = GfxUtils->GetCurrentColor(B);
	GuiElements[iNumGuiElements].color[A] = GfxUtils->GetCurrentColor(A);
	GuiElements[iNumGuiElements].hTexture = hTexture;
	GuiElements[iNumGuiElements].iFlags = iFlags;

	++iNumGuiElements;
}


/*====================
  CGfx2D::AddLine
  ====================*/
void	CGfx2D::AddLine(const CVec2f &v1, const CVec2f &v2, const CVec4f &v4Color1, const CVec4f &v4Color2, int iFlags)
{
	if (iNumGuiElements >= MAX_GUIQUADS)
		Draw();
	if (iNumGuiElements >= MAX_GUIQUADS)
		return;

	GuiElements[iNumGuiElements].eType = GUI_LINE;
	GuiElements[iNumGuiElements].x[0] = v1.x;
	GuiElements[iNumGuiElements].y[0] = v1.y;
	GuiElements[iNumGuiElements].x[1] = v2.x;
	GuiElements[iNumGuiElements].y[1] = v2.y;
	GuiElements[iNumGuiElements].s[0] = v4Color1[0];
	GuiElements[iNumGuiElements].s[1] = v4Color1[1];
	GuiElements[iNumGuiElements].s[2] = v4Color1[2];
	GuiElements[iNumGuiElements].s[3] = v4Color1[3];
	GuiElements[iNumGuiElements].t[0] = v4Color2[0];
	GuiElements[iNumGuiElements].t[1] = v4Color2[1];
	GuiElements[iNumGuiElements].t[2] = v4Color2[2];
	GuiElements[iNumGuiElements].t[3] = v4Color2[3];
	GuiElements[iNumGuiElements].iFlags = iFlags;

	++iNumGuiElements;
}


/*====================
  CGfx2D::AddQuad
  ====================*/
void	CGfx2D::AddQuad(const CVec2f &v1, const CVec2f &v2, const CVec2f &v3, const CVec2f &v4, const CVec2f &t1, const CVec2f &t2, const CVec2f &t3, const CVec2f &t4, ResHandle hTexture, int iFlags)
{
	if (iNumGuiElements >= MAX_GUIQUADS)
		Draw();
	if (iNumGuiElements >= MAX_GUIQUADS)
		return;
	if (hTexture == INVALID_RESOURCE)
	{
		Console.Warn << _T("CGfx2D::AddQuad() - Invalid texture handle") << newl;
		return;
	}

	GuiElements[iNumGuiElements].eType = GUI_QUAD;
	GuiElements[iNumGuiElements].x[0] = v1.x;
	GuiElements[iNumGuiElements].y[0] = v1.y;
	GuiElements[iNumGuiElements].x[1] = v2.x;
	GuiElements[iNumGuiElements].y[1] = v2.y;
	GuiElements[iNumGuiElements].x[2] = v3.x;
	GuiElements[iNumGuiElements].y[2] = v3.y;
	GuiElements[iNumGuiElements].x[3] = v4.x;
	GuiElements[iNumGuiElements].y[3] = v4.y;

	GuiElements[iNumGuiElements].s[0] = t1.x;
	GuiElements[iNumGuiElements].t[0] = t1.y;
	GuiElements[iNumGuiElements].s[1] = t2.x;
	GuiElements[iNumGuiElements].t[1] = t2.y;
	GuiElements[iNumGuiElements].s[2] = t3.x;
	GuiElements[iNumGuiElements].t[2] = t3.y;
	GuiElements[iNumGuiElements].s[3] = t4.x;
	GuiElements[iNumGuiElements].t[3] = t4.y;

	GuiElements[iNumGuiElements].color[R] = GfxUtils->GetCurrentColor(R);
	GuiElements[iNumGuiElements].color[G] = GfxUtils->GetCurrentColor(G);
	GuiElements[iNumGuiElements].color[B] = GfxUtils->GetCurrentColor(B);
	GuiElements[iNumGuiElements].color[A] = GfxUtils->GetCurrentColor(A);
	GuiElements[iNumGuiElements].hTexture = hTexture;
	GuiElements[iNumGuiElements].iFlags = iFlags;

	++iNumGuiElements;
}


/*====================
  CGfx2D::Draw
  ====================*/
void	CGfx2D::Draw()
{
	PROFILE("CGfx2D::Draw");

	if (iNumGuiElements == 0 || !g_bValidScene || !VBGuiQuad)
		return;

	g_bInvertedProjection = false;
	
	g_bLighting = false;
	g_iNumActiveBones = 0;
	g_bShadows = false;
	g_bFogofWar = false;
	g_bFog = false;
	g_iNumActivePointLights = 0;
	g_bTexkill = false;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight, 0, -MAX_GUIQUADS, 0);
	glViewport(0, 0, g_CurrentVidMode.iWidth, g_CurrentVidMode.iHeight);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBGuiQuad);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, MAX_GUIQUADS * 4 * sizeof(SGuiVertex), NULL, GL_STREAM_DRAW_ARB);
	SGuiVertex* pVertices((SGuiVertex*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY));

	for (int i(0); i < iNumGuiElements; ++i)
	{
		SGuiQuad &quad = GuiElements[i];

		switch (quad.eType)
		{
		case GUI_RECT:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[0];
			pVertices->tv = quad.t[0];
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[1];
			pVertices->tv = quad.t[1];
			++pVertices;

			pVertices->x = quad.x[2];
			pVertices->y = quad.y[2];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[2];
			pVertices->tv = quad.t[2];
			++pVertices;

			pVertices->x = quad.x[3];
			pVertices->y = quad.y[3];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[3];
			pVertices->tv = quad.t[3];
			++pVertices;
			break;
		case GUI_QUAD:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[0];
			pVertices->tv = quad.t[0];
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[1];
			pVertices->tv = quad.t[1];
			++pVertices;

			pVertices->x = quad.x[2];
			pVertices->y = quad.y[2];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[2];
			pVertices->tv = quad.t[2];
			++pVertices;

			pVertices->x = quad.x[3];
			pVertices->y = quad.y[3];
			pVertices->color = quad.color;
			pVertices->tu = quad.s[3];
			pVertices->tv = quad.t[3];
			++pVertices;
			break;
		case GUI_LINE:
			pVertices->x = quad.x[0];
			pVertices->y = quad.y[0];
			pVertices->color = quad.s;
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.t;
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.t;
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;

			pVertices->x = quad.x[1];
			pVertices->y = quad.y[1];
			pVertices->color = quad.t;
			pVertices->tu = 0.0f;
			pVertices->tv = 0.0f;
			++pVertices;
			break;
		}
	}

	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, VBGuiQuad);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(24));
	glColorPointer(4, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(8));
	glVertexPointer(2, GL_FLOAT, sizeof(SGuiVertex), BUFFER_OFFSET(0));

	GfxMaterials->SelectMaterial(g_MaterialGUI, PHASE_COLOR, SceneManager.GetShaderTime(), 0);

	ResHandle hWhiteTexture(g_ResourceManager.GetWhiteTexture());
	//CTexture *pWhiteTexture(g_ResourceManager.GetTexture(hWhiteTexture));
	//int iWhiteTextureFlags(pWhiteTexture != NULL ? pWhiteTexture->GetTextureFlags() : 0);

	CMaterial *pMaterial(NULL);
	int iTextureStage(0);

	for (int i(0); i < iNumGuiElements; ++i)
	{
		SGuiQuad &quad = GuiElements[i];

		if (quad.eType == GUI_LINE)
		{
			int f;
			int iFlags(quad.iFlags);

			for (f = i + 1; f < iNumGuiElements && GuiElements[f].eType == GUI_LINE && GuiElements[f].iFlags == iFlags; ++f);

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : g_MaterialGUI);
		
			CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
			CMaterialSampler &sampler(phase.GetSampler(0));

			sampler.SetTexture(hWhiteTexture);

			phase.SetCullMode(CULL_BACK);

			if (iFlags & GUI_ADDITIVE)
			{
				phase.SetSrcBlend(BLEND_SRC_ALPHA);
				phase.SetDstBlend(BLEND_ONE);
			}
			else if (iFlags & GUI_OVERLAY)
			{
				phase.SetSrcBlend(BLEND_DEST_COLOR);
				phase.SetDstBlend(BLEND_SRC_COLOR);
			}
			else if (iFlags & GUI_FOG)
			{
				phase.SetSrcBlend(BLEND_ZERO);
				phase.SetDstBlend(BLEND_SRC_ALPHA);
			}
			else
			{
				phase.SetSrcBlend(BLEND_SRC_ALPHA);
				phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
			}

			if (iFlags & GUI_TILE_U)
				sampler.AddFlags(SAM_REPEAT_U);
			else
				sampler.ClearFlags(SAM_REPEAT_U);

			if (iFlags & GUI_TILE_V)
				sampler.AddFlags(SAM_REPEAT_V);
			else
				sampler.ClearFlags(SAM_REPEAT_V);

			GfxMaterials->SelectMaterial(g_MaterialGUI, PHASE_COLOR, SceneManager.GetShaderTime(), 0);
			pMaterial = &material;
			iTextureStage = GfxMaterials->GetTextureStage(IMAGE);

			glDrawArrays(GL_LINES, i << 2, (f - i) << 2);

			i += (f - i) - 1;
		}
		else
		{
			ResHandle hTexture(quad.hTexture);
			int iFlags(quad.iFlags);
			int f;
			
			for (f = i + 1; f < iNumGuiElements && GuiElements[f].eType != GUI_LINE && GuiElements[f].hTexture == hTexture && GuiElements[f].iFlags == iFlags; ++f);

			CMaterial &material((iFlags & GUI_GRAYSCALE) ? g_MaterialGUIGrayScale : (iFlags & GUI_BLUR) ? g_MaterialGUIBlur : g_MaterialGUI);

			if (pMaterial != &material)
			{
				CMaterialPhase &phase(material.GetPhase(PHASE_COLOR));
				CMaterialSampler &sampler(phase.GetSampler(0));

				sampler.SetTexture(hTexture);

				phase.SetCullMode(CULL_BACK);

				if (iFlags & GUI_ADDITIVE)
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE);
				}
				else if (iFlags & GUI_OVERLAY)
				{
					phase.SetSrcBlend(BLEND_DEST_COLOR);
					phase.SetDstBlend(BLEND_SRC_COLOR);
				}
				else if (iFlags & GUI_FOG)
				{
					phase.SetSrcBlend(BLEND_ZERO);
					phase.SetDstBlend(BLEND_SRC_ALPHA);
				}
				else
				{
					phase.SetSrcBlend(BLEND_SRC_ALPHA);
					phase.SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
				}

				if (iFlags & GUI_TILE_U)
					sampler.AddFlags(SAM_REPEAT_U);
				else
					sampler.ClearFlags(SAM_REPEAT_U);

				if (iFlags & GUI_TILE_V)
					sampler.AddFlags(SAM_REPEAT_V);
				else
					sampler.ClearFlags(SAM_REPEAT_V);

				GfxMaterials->SelectMaterial(material, PHASE_COLOR, SceneManager.GetShaderTime(), 0);
				pMaterial = &material;
				iTextureStage = GfxMaterials->GetTextureStage(IMAGE);
			}
			else
			{
				GfxMaterials->UpdateShaderTexture(iTextureStage, hTexture);

				CTexture *pTexture(g_ResourceManager.GetTexture(hTexture));
	
				int iTextureFlags(pTexture != NULL ? pTexture->GetTextureFlags() : 0);

				if (iFlags & GUI_ADDITIVE)
					glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				else if (iFlags & GUI_OVERLAY)
					glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
				else if (iFlags & GUI_FOG)
					glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
				else
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				if (iFlags & GUI_TILE_U)
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				else
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

				if (iFlags & GUI_TILE_V)
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				else
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				// Setup texture mipmaps
				if (iTextureFlags & TEX_NO_MIPMAPS)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilter);
					if (GLEW_EXT_texture_filter_anisotropic)
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, g_textureMagFilter);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, g_textureMinFilterMipmap);
					if (GLEW_EXT_texture_filter_anisotropic)
						glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, g_textureMaxAnisotropy);
				}
			}

			glDrawArrays(GL_QUADS, i << 2, (f - i) << 2);

			i += (f - i) - 1;
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	iNumGuiElements = 0;
}


/*====================
  CGfx2D::ForceEmpty
  ====================*/
void	CGfx2D::ForceEmpty()
{
	iNumGuiElements = 0;
}


/*====================
  CGfx2D::Shutdown
  ====================*/
void	CGfx2D::Shutdown()
{
	GL_SAFE_DELETE(glDeleteBuffersARB, VBGuiQuad);

	PRINT_GLERROR_BREAK();
}

