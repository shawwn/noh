// (C)2008 S2 Games
// i_instancer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "i_instancer.h"

#include "d3d9f_main.h"
#include "d3d9f_material.h"
#include "d3d9f_model.h"
#include "d3d9f_scene.h"
#include "d3d9f_terrain.h"
//=============================================================================

/*====================
  IInstancer::~IInstancer
  ====================*/
IInstancer::~IInstancer()
{
}


/*====================
  IInstancer::IInstancer
  ====================*/
IInstancer::IInstancer()
{
}


/*====================
  IInstancer::SetShaderVars
  ====================*/
void	IInstancer::SetShaderVars()
{
	g_mWorld = g_mIdentity;
	g_mWorldViewProj = g_mViewProj;
	g_mWorldRotate = g_mIdentity;
	g_v3SunColor = m_vSunColor;
	g_v3Ambient = m_vAmbient;
	g_pCam = m_pCam;
	g_pCurrentEntity = NULL;
	g_bObjectColor = false;
	if  (m_iNumActivePointLights > 0)
	{
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
