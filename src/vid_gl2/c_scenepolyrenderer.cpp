// (C)2008 S2 Games
// c_renderlist.cpp
//
// Billboard batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_scenepolyrenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
#include "c_shaderregistry.h"

#include "../k2/c_world.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CScenePolyRenderer> CScenePolyRenderer::s_Pool(1, uint(-1));

/*====================
  CScenePolyRenderer::operator new
  ====================*/
void*	CScenePolyRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
	return s_Pool.Allocate();
}


/*====================
  CScenePolyRenderer::CScenePolyRenderer
  ====================*/
CScenePolyRenderer::CScenePolyRenderer(ResHandle hMaterial, SSceneFaceVert *pVerts, uint uiNumVerts, int iFlags) :
m_hMaterial(hMaterial),
m_pVerts(pVerts),
m_uiNumVerts(uiNumVerts),
m_iFlags(iFlags)
{
}


/*====================
  CScenePolyRenderer::~CScenePolyRenderer
  ====================*/
CScenePolyRenderer::~CScenePolyRenderer()
{
}


/*====================
  CScenePolyRenderer::Setup
  ====================*/
void	CScenePolyRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("CScenePolyRenderer::Setup");

	CMaterial &material(GfxUtils->GetMaterial(m_hMaterial));

	if (!material.HasPhase(ePhase))
		return; // Leave if we don't have this phase

	m_pCam = g_pCam;
	m_bLighting = gfx_lighting;
	m_vAmbient = SceneManager.GetEntityAmbientColor();
	m_vSunColor = SceneManager.GetEntitySunColor();

	m_iNumActivePointLights = 0;
	m_iNumActiveBones = 0;

	m_bObjectColor = false;
	m_pCurrentEntity = NULL;

	m_mWorld = g_mIdentity;
	m_mWorldRotate = g_mIdentity;
	m_mWorldViewProj = g_mViewProj;

	m_bRender = true;

	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(m_bFog);

	// Set sorting variables
	m_bTranslucent = material.GetPhase(ePhase).GetTranslucent();
	m_iLayer = material.GetPhase(ePhase).GetLayer();
	m_iEffectLayer = 0;
	//m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
	//m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());
	m_uiVertexBuffer = Gfx3D->VBScenePoly;
	m_uiIndexBuffer = 0;
	m_fDepth = 0.0f;
	m_bRefractive = material.GetPhase(ePhase).GetRefractive();
}


/*====================
  CScenePolyRenderer::Render
  ====================*/
void	CScenePolyRenderer::Render(EMaterialPhase ePhase)
{
	if (!m_bRender)
		return;

	SetShaderVars();
	glPointSize(4.0f);
	GfxMaterials->SelectMaterial(GfxUtils->GetMaterial(m_hMaterial), ePhase, g_pCam->GetTime(), 0);

	if (m_iFlags & POLY_NO_DEPTH_TEST)
		glDisable(GL_DEPTH_TEST);
	if (m_iFlags & POLY_DOUBLESIDED)
		glDisable(GL_CULL_FACE);

	if (m_iFlags & POLY_WIREFRAME || m_iFlags & POLY_LINESTRIP || m_iFlags & POLY_LINELIST)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if (m_iFlags & POLY_POINT)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, Gfx3D->VBScenePoly);
	SEffectVertex* pVertices = (SEffectVertex*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
	for (uint v(0); v < m_uiNumVerts; ++v)
	{
		pVertices[v].v = m_pVerts[v].vtx;
		pVertices[v].color = D3DCOLOR_ARGB(m_pVerts[v].col[3], m_pVerts[v].col[2], m_pVerts[v].col[1], m_pVerts[v].col[0]);
		pVertices[v].t.x = m_pVerts[v].tex.x;
		pVertices[v].t.y = m_pVerts[v].tex.y;
		pVertices[v].t.z = 0.0f;
	}
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

	glTexCoordPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(16));
	glColorPointer(4, GL_UNSIGNED_BYTE, 32, BUFFER_OFFSET(12));
	glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));

	if (m_iFlags & POLY_POINTLIST)
		glDrawArrays(GL_POINTS, 0, m_uiNumVerts);
	else if (m_iFlags & POLY_LINELIST)
		glDrawArrays(GL_LINES, 0, m_uiNumVerts);
	else if (m_iFlags & POLY_LINESTRIP)
		glDrawArrays(GL_LINE_STRIP, 0, m_uiNumVerts);
	else if (m_iFlags & POLY_TRILIST)
		glDrawArrays(GL_TRIANGLES, 0, m_uiNumVerts);
	else
		glDrawArrays(GL_TRIANGLE_FAN, 0, m_uiNumVerts);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
