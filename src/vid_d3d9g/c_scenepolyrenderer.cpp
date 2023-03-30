// (C)2006 S2 Games
// c_scenepolyrenderer.cpp
//
// Billboard batch renderer
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
#include "c_scenepolyrenderer.h"
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
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CScenePolyRenderer> CScenePolyRenderer::s_Pool(1, -1);

/*====================
  CScenePolyRenderer::operator new
  ====================*/
void*   CScenePolyRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CScenePolyRenderer::CScenePolyRenderer
  ====================*/
CScenePolyRenderer::CScenePolyRenderer(ResHandle hMaterial, SSceneFaceVert *pVerts, uint uiNumVerts, int iFlags) :
IRenderer(RT_UNKNOWN),
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
void    CScenePolyRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CScenePolyRenderer::Setup");

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
    g_ShaderRegistry.SetTexcoords(m_iTexcoords);
    g_ShaderRegistry.SetTexkill(m_bTexkill);

    // Set sorting variables
    m_bTranslucent = cPhase.GetTranslucent();
    m_iLayer = cPhase.GetLayer();
    m_iEffectLayer = 0;
    m_iVertexType = VERTEX_EFFECT;
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
    m_uiVertexBuffer = reinterpret_cast<size_t>(g_pVBScenePoly);
    m_uiIndexBuffer = 0;
    m_fDepth = cPhase.GetDepthSortBias();
    m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CScenePolyRenderer::Render
  ====================*/
void    CScenePolyRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CScenePolyRenderer::Render");

    if (!m_bRender)
        return;

    SetShaderVars();

    D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(4.0f));

    D3D_SelectMaterial(D3D_GetMaterial(m_hMaterial), ePhase, VERTEX_EFFECT, g_pCam->GetTime(), gfx_depthFirst);

    if (m_iFlags & POLY_NO_DEPTH_TEST)
        D3D_SetRenderState(D3DRS_ZENABLE, FALSE);

    if (m_iFlags & POLY_DOUBLESIDED)
        D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    SEffectVertex* pVertices;

    if (FAILED(g_pVBScenePoly->Lock(0, m_uiNumVerts * sizeof(SEffectVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
        return;

    if (m_iFlags & POLY_WIREFRAME || m_iFlags & POLY_LINESTRIP || m_iFlags & POLY_LINELIST)
        D3D_PushRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    else if (m_iFlags & POLY_POINT)
        D3D_PushRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
    else
        D3D_PushRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    for (uint v(0); v < m_uiNumVerts; ++v)
    {
        pVertices[v].v = m_pVerts[v].vtx;
        pVertices[v].color = D3DCOLOR_ARGB(m_pVerts[v].col[3], m_pVerts[v].col[0], m_pVerts[v].col[1], m_pVerts[v].col[2]);
        pVertices[v].t.x = m_pVerts[v].tex.x;
        pVertices[v].t.y = m_pVerts[v].tex.y;
        pVertices[v].t.z = 0.0f;
    }

    g_pVBScenePoly->Unlock();

    D3D_SetStreamSource(0, g_pVBScenePoly, 0, sizeof(SEffectVertex));

    if (m_iFlags & POLY_POINTLIST)
    {
        D3D_DrawPrimitive(D3DPT_POINTLIST, 0, m_uiNumVerts);
        SceneStats.RecordBatch(m_uiNumVerts, m_uiNumVerts, ePhase, SSBATCH_SCENEPOLY);
    }
    else if (m_iFlags & POLY_LINELIST)
    {
        D3D_DrawPrimitive(D3DPT_LINELIST, 0, m_uiNumVerts / 2);
        SceneStats.RecordBatch(m_uiNumVerts, m_uiNumVerts / 2, ePhase, SSBATCH_SCENEPOLY);
    }
    else if (m_iFlags & POLY_LINESTRIP)
    {
        D3D_DrawPrimitive(D3DPT_LINESTRIP, 0, m_uiNumVerts - 1);
        SceneStats.RecordBatch(m_uiNumVerts, m_uiNumVerts - 1, ePhase, SSBATCH_SCENEPOLY);
    }
    else if (m_iFlags & POLY_TRILIST)
    {
        D3D_DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_uiNumVerts / 3);
        SceneStats.RecordBatch(m_uiNumVerts, m_uiNumVerts / 3, ePhase, SSBATCH_SCENEPOLY);
    }
    else
    {
        D3D_DrawPrimitive(D3DPT_TRIANGLEFAN, 0, m_uiNumVerts - 2);
        SceneStats.RecordBatch(m_uiNumVerts, m_uiNumVerts - 2, ePhase, SSBATCH_SCENEPOLY);
    }

    D3D_PopRenderState(D3DRS_FILLMODE);
}
