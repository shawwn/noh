// (C)2008 S2 Games
// i_renderer.cpp
//
// Batch render list item base class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "i_renderer.h"

#include "c_gfxmaterials.h"
//=============================================================================

/*====================
  IRenderer::IRenderer
  ====================*/
IRenderer::IRenderer() :
m_bRender(false),
m_bTranslucent(false),
m_iLayer(0),
m_iEffectLayer(0),
m_iShaderProgramInstance(-1),
m_fDepth(0.0f),
m_bDepthFirst(false),
m_iNumActiveBones(0),
m_bLighting(g_bLighting),
m_bShadows(g_bCamShadows),
m_bFog(g_bCamFog),
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
void    IRenderer::SetShaderVars()
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
        MemManager.Copy(g_vPointLightPosition, m_vPointLightPosition, sizeof(CVec3f) * m_iNumActivePointLights);
        MemManager.Copy(g_vPointLightColor, m_vPointLightColor, sizeof(CVec3f) * m_iNumActivePointLights);
        MemManager.Copy(g_fPointLightFalloffStart, m_fPointLightFalloffStart, sizeof(float) * m_iNumActivePointLights);
        MemManager.Copy(g_fPointLightFalloffEnd, m_fPointLightFalloffEnd, sizeof(float) * m_iNumActivePointLights);
    }
    g_iNumActivePointLights = m_iNumActivePointLights;
    MemManager.Copy(g_afLeafClusterData, m_afLeafClusterData, sizeof(float) * m_uiLeafClusterDataSize);
    MemManager.Copy(g_vBoneData, m_vBoneData, sizeof(CVec4f) * m_iNumActiveBones * 3);
    g_iNumActiveBones = m_iNumActiveBones;
    g_bLighting = m_bLighting;
    g_bShadows = m_bShadows;
    g_bFog = m_bFog;
    g_bTexkill = m_bTexkill;
}
