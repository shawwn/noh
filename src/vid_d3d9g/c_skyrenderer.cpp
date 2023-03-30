// (C)2007 S2 Games
// c_skyrenderer.cpp
//
// Sky renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_util.h"
#include "d3d9g_material.h"
#include "d3d9g_scene.h"
#include "d3d9g_state.h"
#include "d3d9g_shader.h"
#include "d3d9g_terrain.h"
#include "d3d9g_texture.h"
#include "c_shaderregistry.h"
#include "c_skyrenderer.h"

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
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CSkyRenderer> CSkyRenderer::s_Pool(1, -1);

/*====================
  CSkyRenderer::operator new
  ====================*/
void*   CSkyRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CSkyRenderer::CSkyRenderer
  ====================*/
CSkyRenderer::CSkyRenderer() :
IRenderer(RT_UNKNOWN)
{
    CMaterial &material(D3D_GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

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
    m_iVertexType = VERTEX_SKYBOX;
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
void    CSkyRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CSkyRenderer::Setup");

    m_bRender = false; // Set to true if we make it to the end of the function

    CMaterial &material(D3D_GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

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
    g_ShaderRegistry.SetTexcoords(m_iTexcoords);
    g_ShaderRegistry.SetTexkill(m_bTexkill);

    // Set dynamic sorting variables
    m_iLayer = material.GetPhase(ePhase).GetLayer();
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());
    
    m_bRender = true;
}


/*====================
  CSkyRenderer::Render
  ====================*/
void    CSkyRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CSkyRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    CMaterial &material(D3D_GetMaterial(g_ResourceManager.Register(SceneManager.GetSkyboxMaterial(), RES_MATERIAL)));

    D3DVIEWPORT9 SkyViewport(g_Viewport);

    SkyViewport.MinZ = 1.0f;
    SkyViewport.MaxZ = 1.0f;

    g_pd3dDevice->SetViewport(&SkyViewport);

    D3D_SetStreamSource(0, g_pVBSkybox, 0, sizeof(SSkyboxVertex));
    D3D_SelectMaterial(material, PHASE_COLOR, VERTEX_SKYBOX, g_pCam->GetTime(), gfx_depthFirst);
    D3D_DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);

    g_pd3dDevice->SetViewport(&g_Viewport);
}
