// (C)2005 S2 Games
// i_renderer.cpp
//
// A single item in the Render List
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "i_renderer.h"
#include "d3d9g_main.h"
#include "d3d9g_material.h"
#include "d3d9g_model.h"
#include "d3d9g_scene.h"
#include "d3d9g_terrain.h"
//=============================================================================

/*====================
  IRenderer::IRenderer
  ====================*/
IRenderer::IRenderer(ERenderType eType) :
//m_eType(eType),
m_bRender(false),
m_bTranslucent(false),
m_iLayer(0),
m_iEffectLayer(0),
m_iVertexType(-1),
m_iVertexShaderInstance(-1),
m_iPixelShaderInstance(-1),
m_uiVertexBuffer(-1),
m_uiIndexBuffer(-1),
m_fDepth(0.0f),
m_bDepthFirst(false),
m_iNumActiveBones(0),
m_bLighting(g_bLighting),
m_bShadows(g_bCamShadows),
m_bFog(g_bCamFog),
m_iTexcoords(1),
m_uiLeafClusterDataSize(0),
m_bRefractive(false),
m_bTexkill(false)
{
}


/*====================
  IRenderer::~IRenderer
  ====================*/
IRenderer::~IRenderer()
{
}


/*====================
  IRenderer::SetShaderVars
  ====================*/
void	IRenderer::SetShaderVars()
{
	g_mWorld = m_mWorld;
	g_mWorldViewProj = m_mWorldViewProj;
	g_mWorldRotate = m_mWorldRotate;
	g_v3SunColor = m_vSunColor;
	g_v3Ambient = m_vAmbient;
	g_pCam = m_pCam;
	g_pCurrentEntity = m_pCurrentEntity;
	g_bObjectColor = m_bObjectColor;
	g_vObjectColor = m_vObjectColor;
	if  (m_iNumActivePointLights > 0)
	{
		assert (m_iNumActivePointLights <= MAX_POINT_LIGHTS);

		MemManager.Copy(g_vPointLightPosition, m_vPointLightPosition, sizeof(CVec3f) * m_iNumActivePointLights);
		MemManager.Copy(g_vPointLightColor, m_vPointLightColor, sizeof(CVec3f) * m_iNumActivePointLights);
		MemManager.Copy(g_fPointLightFalloffStart, m_fPointLightFalloffStart, sizeof(float) * m_iNumActivePointLights);
		MemManager.Copy(g_fPointLightFalloffEnd, m_fPointLightFalloffEnd, sizeof(float) * m_iNumActivePointLights);
	}
	g_iNumActivePointLights = m_iNumActivePointLights;
	MemManager.Copy(g_afLeafClusterData, m_afLeafClusterData, sizeof(float) * m_uiLeafClusterDataSize);
	MemManager.Copy(g_vBoneData, m_vBoneData, sizeof(D3DXMATRIXA16) * m_iNumActiveBones);
	g_iNumActiveBones = m_iNumActiveBones;
	g_bLighting = m_bLighting;
	g_bShadows = m_bShadows;
	g_bFog = m_bFog;
	g_iTexcoords = m_iTexcoords;
	g_bTexkill = m_bTexkill;
}
