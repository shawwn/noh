// (C)2008 S2 Games
// i_effectrenderer.cpp
//
// Effect-style batch renderer base class
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "i_effectrenderer.h"

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
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

/*====================
  IEffectRenderer::IEffectRenderer
  ====================*/
IEffectRenderer::IEffectRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
m_hMaterial(hMaterial),
m_uiStartIndex(uiStartIndex),
m_uiEndIndex(uiEndIndex)
{
    m_iNumActivePointLights = 0;
    m_iNumActiveBones = 0;

    m_bObjectColor = false;
    m_pCurrentEntity = nullptr;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;

    // Set static sorting variables
    m_iEffectLayer = iEffectLayer;
    m_fDepth = fDepth;
    m_uiVertexBuffer = 0;
    m_uiIndexBuffer = 0;
}


/*====================
  IEffectRenderer::~IEffectRenderer
  ====================*/
IEffectRenderer::~IEffectRenderer()
{
}

CVAR_BOOL(vid_skipEffectRenderer, false);

/*====================
  IEffectRenderer::Setup
  ====================*/
void    IEffectRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipEffectRenderer)
        return;

    PROFILE("IEffectRenderer::Setup");

    CMaterial &material(GfxUtils->GetMaterial(m_hMaterial));

    if (!material.HasPhase(ePhase))
        return; // Leave if we don't have this phase

    m_pCam = g_pCam;
    m_bLighting = gfx_lighting;
    m_vAmbient = SceneManager.GetEntityAmbientColor();
    m_vSunColor = SceneManager.GetEntitySunColor();

    m_mWorldViewProj = g_mViewProj;

    g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
    g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
    g_ShaderRegistry.SetLighting(m_bLighting);
    g_ShaderRegistry.SetShadows(m_bShadows);
    g_ShaderRegistry.SetFogofWar(g_bFogofWar);
    g_ShaderRegistry.SetFog(m_bFog);

    // Set dynamic sorting variables
    m_bTranslucent = material.GetPhase(ePhase).GetTranslucent();
    m_iLayer = material.GetPhase(ePhase).GetLayer();
    m_uiVertexBuffer = m_hMaterial;
    //m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
    //m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());
    m_bRefractive = material.GetPhase(ePhase).GetRefractive();
    
    m_bRender = true;
}
