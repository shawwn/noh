// (C)2006 S2 Games
// i_effectrenderer.cpp
//
// Effect-style batch renderer base class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9f_main.h"
#include "d3d9f_util.h"
#include "d3d9f_material.h"
#include "d3d9f_scene.h"
#include "d3d9f_state.h"
#include "d3d9f_shader.h"
#include "d3d9f_terrain.h"
#include "d3d9f_texture.h"
#include "i_effectrenderer.h"
#include "c_shaderregistry.h"

#include "../k2/c_world.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

/*====================
  IEffectRenderer::IEffectRenderer
  ====================*/
IEffectRenderer::IEffectRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
IRenderer(RT_UNKNOWN),
m_hMaterial(hMaterial),
m_uiStartIndex(uiStartIndex),
m_uiEndIndex(uiEndIndex)
{
	m_iNumActivePointLights = 0;
	m_iNumActiveBones = 0;

	m_bObjectColor = false;
	m_pCurrentEntity = NULL;

	m_mWorld = g_mIdentity;
	m_mWorldRotate = g_mIdentity;

	// Set static sorting variables
	m_iEffectLayer = iEffectLayer;
	m_fDepth = fDepth;
	m_iVertexType = VERTEX_EFFECT;
	m_uiVertexBuffer = 0;
	m_uiIndexBuffer = 0;
}


/*====================
  IEffectRenderer::~IEffectRenderer
  ====================*/
IEffectRenderer::~IEffectRenderer()
{
}


/*====================
  IEffectRenderer::Setup
  ====================*/
void	IEffectRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("IEffectRenderer::Setup");

	m_bRender = false; // Set to true if we make it to the end of the function

	CMaterial &material(D3D_GetMaterial(m_hMaterial));

	if (!material.HasPhase(ePhase))
		return; // Leave if we don't have this phase

	CMaterialPhase &cPhase(material.GetPhase(ePhase));

	m_pCam = g_pCam;
	m_bLighting = gfx_lighting;
	m_bShadows = g_bCamShadows;
	m_bFog = g_bCamFog;
	m_vAmbient = SceneManager.GetEntityAmbientColor();
	m_vSunColor = SceneManager.GetEntitySunColor();

	m_mWorldViewProj = g_mViewProj;

	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(m_bFog);
	g_ShaderRegistry.SetTexcoords(m_iTexcoords);
	g_ShaderRegistry.SetTexkill(m_bTexkill);

	// Set dynamic sorting variables
	m_bTranslucent = cPhase.GetTranslucent();
	m_iLayer = cPhase.GetLayer();
	m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
	m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
	m_bRefractive = cPhase.GetRefractive();
	
	m_bRender = true;
}
