// (C)2008 S2 Games
// i_debugrenderer.cpp
//
// Debug-style batch renderer base class (points/lines/boxes)
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "i_debugrenderer.h"

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
  IDebugRenderer::IDebugRenderer
  ====================*/
IDebugRenderer::IDebugRenderer()
{
    m_iNumActivePointLights = 0;
    m_iNumActiveBones = 0;

    m_bObjectColor = false;
    m_pCurrentEntity = nullptr;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;

    // Set static sorting variables
    m_bTranslucent = false;
    m_iLayer = INT_MAX;
    m_iEffectLayer = 0;
    m_uiVertexBuffer = 0;
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
}


/*====================
  IDebugRenderer::~IDebugRenderer
  ====================*/
IDebugRenderer::~IDebugRenderer()
{
}

CVAR_BOOL(vid_skipDebugRenderer, false);

/*====================
  IDebugRenderer::Setup
  ====================*/
void    IDebugRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipDebugRenderer)
        return;

    PROFILE("IDebugRenderer::Setup");

    if (ePhase != PHASE_COLOR)
        return;

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
    //m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(g_SimpleMaterial3DColored.GetPhase(ePhase).GetVertexShader());
    //m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(g_SimpleMaterial3DColored.GetPhase(ePhase).GetPixelShader());

    m_bRender = true;
}
