// (C)2008 S2 Games
// c_skyrenderer.cpp
//
// Sky renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_skyrenderer.h"

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
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CSkyRenderer> CSkyRenderer::s_Pool(1, uint(-1));

/*====================
  CSkyRenderer::operator new
  ====================*/
void*	CSkyRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
	return s_Pool.Allocate();
}


/*====================
  CSkyRenderer::CSkyRenderer
  ====================*/
CSkyRenderer::CSkyRenderer() :
IRenderer()
{
	CMaterial &material(GfxUtils->GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

	m_iNumActivePointLights = 0;
	m_iNumActiveBones = 0;

	m_bObjectColor = true;
	m_vObjectColor = SceneManager.GetSkyColor();
	m_pCurrentEntity = NULL;

	m_mWorld = g_mIdentity;
	m_mWorldRotate = g_mIdentity;

	m_fDepth = (material.HasPhase(PHASE_COLOR) && material.GetPhase(PHASE_COLOR).GetTranslucent()) ? FAR_AWAY : FAR_AWAY * 2.0f;
	m_bTranslucent = true;
	
	m_bLighting = false;
}


/*====================
  CSkyRenderer::~CSkyRenderer
  ====================*/
CSkyRenderer::~CSkyRenderer()
{
}


/*====================
  CSkyRenderer::Setup
  ====================*/
void	CSkyRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("CSkyRenderer::Setup");

	CMaterial &material(GfxUtils->GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

	if (!material.HasPhase(ePhase))
		return; // Leave if we don't have this phase

	m_pCam = g_pCam;
	
	m_vAmbient = SceneManager.GetEntityAmbientColor();
	m_vSunColor = SceneManager.GetEntitySunColor();

	CVec3f v3Pos(g_pCam->GetOrigin());
	D3DXMatrixTranslation(&m_mWorld, v3Pos.x, v3Pos.y, v3Pos.z);

	m_mWorldViewProj = m_mWorld * g_mViewProj;

	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(m_bFog);

	// Set dynamic sorting variables
	m_iLayer = material.GetPhase(ePhase).GetLayer();
	//m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
	//m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());
	
	m_bRender = true;
}


/*====================
  CSkyRenderer::Render
  ====================*/
void	CSkyRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CSkyRenderer::Render");

	if (!m_bRender)
		return;

	SetShaderVars();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(m_mWorld);

	CMaterial &material(GfxUtils->GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

	GfxMaterials->SelectMaterial(material, PHASE_COLOR, g_pCam->GetTime(), false);

	glBegin(GL_TRIANGLE_FAN);
	glVertex4f(-1.0f, -1.0f, 1.0f, 1.0f);
	glVertex4f( 1.0f, -1.0f, 1.0f, 1.0f);
	glVertex4f( 1.0f,  1.0f, 1.0f, 1.0f);
	glVertex4f(-1.0f,  1.0f, 1.0f, 1.0f);
	glEnd();

	glPopMatrix();
}
