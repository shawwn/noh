// (C)2007 S2 Games
// c_billboardrenderer.cpp
//
// Billboard batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
#include "d3d9_util.h"
#include "d3d9_material.h"
#include "d3d9_scene.h"
#include "d3d9_state.h"
#include "d3d9_shader.h"
#include "d3d9_terrain.h"
#include "d3d9_texture.h"
#include "c_shaderregistry.h"
#include "c_treebillboardrenderer.h"

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

CPool<CTreeBillboardRenderer> CTreeBillboardRenderer::s_Pool(1, -1);

/*====================
  CTreeBillboardRenderer::operator new
  ====================*/
void*   CTreeBillboardRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeBillboardRenderer::CTreeBillboardRenderer
  ====================*/
CTreeBillboardRenderer::CTreeBillboardRenderer(ResHandle hMaterial, dword dwAlphaTest, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
IRenderer(RT_UNKNOWN),
m_hMaterial(hMaterial),
m_dwAlphaTest(dwAlphaTest),
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
    m_iVertexType = VERTEX_TREE_BILLBOARD;
    m_uiVertexBuffer = reinterpret_cast<size_t>(g_pVBTreeBillboard);
    m_uiIndexBuffer = reinterpret_cast<size_t>(g_pIBTreeBillboard);
}


/*====================
  CTreeBillboardRenderer::~CTreeBillboardRenderer
  ====================*/
CTreeBillboardRenderer::~CTreeBillboardRenderer()
{
}


/*====================
  CTreeBillboardRenderer::Setup
  ====================*/
void    CTreeBillboardRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CTreeBillboardRenderer::Setup");

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


/*====================
  CTreeBillboardRenderer::Render
  ====================*/
void    CTreeBillboardRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CTreeBillboardRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    D3D_SetRenderState(D3DRS_ALPHAREF, m_dwAlphaTest);
    D3D_SetStreamSource(0, g_pVBTreeBillboard, 0, sizeof(STreeBillboardVertex));
    D3D_SetIndices(g_pIBTreeBillboard);

    D3D_SelectMaterial(D3D_GetMaterial(m_hMaterial), ePhase, VERTEX_TREE_BILLBOARD, g_pCam->GetTime(), gfx_depthFirst);
    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_uiStartIndex * 4, (m_uiEndIndex - m_uiStartIndex) << 2, m_uiStartIndex * 6, (m_uiEndIndex - m_uiStartIndex) << 1);

    SceneStats.RecordBatch((m_uiEndIndex - m_uiStartIndex) << 2, (m_uiEndIndex - m_uiStartIndex) << 1, ePhase, SSBATCH_TREEBILLBOARD);
}
