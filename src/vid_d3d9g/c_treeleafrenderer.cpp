// (C)2006 S2 Games
// c_treeleafrenderer.cpp
//
// SpeedTree leaf renderer
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
#include "c_treeleafrenderer.h"
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

CPool<CTreeLeafRenderer> CTreeLeafRenderer::s_Pool(1, -1);

/*====================
  CTreeLeafRenderer::operator new
  ====================*/
void*   CTreeLeafRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeLeafRenderer::CTreeLeafRenderer
  ====================*/
CTreeLeafRenderer::CTreeLeafRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
        const D3DXMATRIXA16 &mWorldViewProj,
        const D3DXMATRIXA16 &mWorld,
        const D3DXMATRIXA16 &mWorldRotate) :
IRenderer(RT_UNKNOWN),
m_cEntity(cEntity),
m_pTreeDef(pTreeDef)
{
    PROFILE("CTreeLeafRenderer::CTreeLeafRenderer");

    m_mWorldViewProj = mWorldViewProj;
    m_mWorld = mWorld;
    m_mWorldRotate = mWorldRotate;

    {
        PROFILE("GetLeafBillboardTable");

        // Fill in the cluster data that the vertex shader will use
        uint uiSize(0);
        const float *pTable = m_pTreeDef->GetLeafBillboardTable(uiSize);
        if (uiSize > LEAF_CLUSTER_TABLE_SIZE)
        {
            Console.Warn << _T("Too many entries in this tree's billboard table") << newl;
            uiSize = LEAF_CLUSTER_TABLE_SIZE;
        }
        m_uiLeafClusterDataSize = uiSize;
        MemManager.Copy(m_afLeafClusterData, pTable, uiSize * sizeof(float));
    }

    {
        PROFILE("GetLeafLODData");

        // Update leaf cluster data for current camera position
        m_avLeafLODs[0].m_bActive = m_avLeafLODs[1].m_bActive = false;
        m_pTreeDef->GetLeafLODData(m_avLeafLODs);
    }
}


/*====================
  CTreeLeafRenderer::~CTreeLeafRenderer
  ====================*/
CTreeLeafRenderer::~CTreeLeafRenderer()
{
}


/*====================
  CTreeLeafRenderer::Setup
  ====================*/
void    CTreeLeafRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CTreeLeafRenderer::Setup");

    m_bRender = false; // Set to true if we make it to the end of the function

    if (!m_avLeafLODs[0].m_bActive && m_avLeafLODs[1].m_bActive)
        return;

    if (!m_pTreeDef->HasLeafGeometry())
        return;

    CMaterial &material(D3D_GetMaterial(m_pTreeDef->GetLeafMaterial()));

    if (!material.HasPhase(ePhase))
        return; // Leave if we don't have this phase

    CMaterialPhase &cPhase(material.GetPhase(ePhase));

    m_pCurrentEntity = &m_cEntity;
    m_pCam = g_pCam;

    m_bLighting = gfx_lighting;
    m_bShadows = g_bCamShadows;
    m_bFog = g_bCamFog;
    m_vAmbient = SceneManager.GetEntityAmbientColor();
    m_vSunColor = SceneManager.GetEntitySunColor();
    m_bObjectColor = false;

    m_iNumActiveBones = 0;

    // Pick the four best point lights to light this model
    m_iNumActivePointLights = 0;

    if (ePhase == PHASE_COLOR)
    {
        CBBoxf  bbBoundsWorld(m_pTreeDef->GetBounds());
        bbBoundsWorld.Transform(m_cEntity.GetPosition(), m_cEntity.axis, m_cEntity.scale);

        SceneLightList &LightList(SceneManager.GetLightList());
        for (SceneLightList::iterator itLight(LightList.begin()); itLight != LightList.end() && m_iNumActivePointLights != g_iMaxDynamicLights; ++itLight)
        {
            SSceneLightEntry &cEntry(**itLight);
            const CSceneLight &scLight(cEntry.cLight);

            if (cEntry.bCull)
                continue;

            if (I_SphereBoundsIntersect(CSphere(scLight.GetPosition(), scLight.GetFalloffEnd()), bbBoundsWorld))
            {
                m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                ++m_iNumActivePointLights;
            }
        }   
    }

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
    m_iVertexType = CTreeModelDef::s_iLeafVertDecl;
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());

    if (m_avLeafLODs[0].m_bActive)
        m_uiVertexBuffer = reinterpret_cast<size_t>(m_pTreeDef->GetLeafGeometry(m_avLeafLODs[0].m_iLOD).m_pVBuffer);
    
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
    m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CTreeLeafRenderer::Render
  ====================*/
void    CTreeLeafRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CTreeLeafRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    if (ePhase == PHASE_COLOR && m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && D3D_GetMaterial(m_pTreeDef->GetLeafMaterial()).HasPhase(PHASE_FADE))
        ePhase = PHASE_FADE;

    // Set initial render states
    D3D_SelectMaterial(D3D_GetMaterial(m_pTreeDef->GetLeafMaterial()), ePhase, CTreeModelDef::s_iLeafVertDecl, 0.0f, gfx_depthFirst);

    for (int i(0); i < 2; ++i)
    {
        if (!m_avLeafLODs[i].m_bActive)
            continue;

        const STreeGeometryBuffers &leaves(m_pTreeDef->GetLeafGeometry(m_avLeafLODs[i].m_iLOD));

        D3D_SetRenderState(D3DRS_ALPHAREF, m_avLeafLODs[i].m_dwAlphaTest);
        D3D_SetPixelShaderConstantFloat("fAlphaTest", (m_avLeafLODs[i].m_dwAlphaTest + 1) / 255.0f);
        D3D_SetStreamSource(0, leaves.m_pVBuffer, 0, CTreeModelDef::s_uiLeafVertSize);

        for (size_t z(0); z < leaves.m_vpIBuffers.size(); ++z)
        {
            D3D_SetIndices(leaves.m_vpIBuffers[z]);
            D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, leaves.m_iNumVerts, 0, leaves.m_viNumIndices[z] / 3);

            SceneStats.RecordBatch(leaves.m_iNumVerts, leaves.m_viNumIndices[z] / 3, ePhase, SSBATCH_TREELEAF);
        }
    }
}
