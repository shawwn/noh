// (C)2006 S2 Games
// c_treefrondrenderer.cpp
//
// SpeedTree frond renderer
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
#include "c_treefrondrenderer.h"
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

CPool<CTreeFrondRenderer> CTreeFrondRenderer::s_Pool(1, -1);

/*====================
  CTreeFrondRenderer::operator new
  ====================*/
void*   CTreeFrondRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeFrondRenderer::CTreeFrondRenderer
  ====================*/
CTreeFrondRenderer::CTreeFrondRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
        const D3DXMATRIXA16 &mWorldViewProj,
        const D3DXMATRIXA16 &mWorld,
        const D3DXMATRIXA16 &mWorldRotate) :
IRenderer(RT_UNKNOWN),
m_cEntity(cEntity),
m_pTreeDef(pTreeDef)
{
    PROFILE("CTreeFrondRenderer::CTreeFrondRenderer");

    m_mWorldViewProj = mWorldViewProj;
    m_mWorld = mWorld;
    m_mWorldRotate = mWorldRotate;

    m_LOD = m_pTreeDef->GetDiscreetFrondLOD();
}


/*====================
  CTreeFrondRenderer::~CTreeFrondRenderer
  ====================*/
CTreeFrondRenderer::~CTreeFrondRenderer()
{
}


/*====================
  CTreeFrondRenderer::Setup
  ====================*/
void    CTreeFrondRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CTreeFrondRenderer::Setup");

    m_bRender = false; // Set to true if we make it to the end of the function

    if (m_LOD.m_iLOD == -1)
        return;

    if (!m_pTreeDef->HasFrondLOD(m_LOD.m_iLOD))
    {
        //Console.Err << _T("CTreeFrondRenderer::Setup() - Invalid LOD: ") << m_LOD.m_iLOD << newl;
        return;
    }

    if (!m_pTreeDef->HasFrondGeometry())
        return;

    CMaterial &material(D3D_GetMaterial(m_pTreeDef->GetFrondMaterial()));

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

    const STreeGeometryBuffers &fronds(m_pTreeDef->GetFrondGeometry(m_LOD.m_iLOD));

    // Set sorting variables
    m_bTranslucent = cPhase.GetTranslucent();
    m_iLayer = cPhase.GetLayer();
    m_iEffectLayer = 0;
    m_iVertexType = CTreeModelDef::s_iFrondVertDecl;
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
    m_uiVertexBuffer = reinterpret_cast<size_t>(fronds.m_pVBuffer);
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
    m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CTreeFrondRenderer::Render
  ====================*/
void    CTreeFrondRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CTreeFrondRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    if (ePhase == PHASE_COLOR && m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && D3D_GetMaterial(m_pTreeDef->GetFrondMaterial()).HasPhase(PHASE_FADE))
        ePhase = PHASE_FADE;

    const STreeGeometryBuffers &fronds(m_pTreeDef->GetFrondGeometry(m_LOD.m_iLOD));

    // Set initial render states
    D3D_SetRenderState(D3DRS_ALPHAREF, m_LOD.m_dwAlphaTest);

    D3D_SetStreamSource(0, fronds.m_pVBuffer, 0, CTreeModelDef::s_uiFrondVertSize);
    D3D_SelectMaterial(D3D_GetMaterial(m_pTreeDef->GetFrondMaterial()), ePhase, CTreeModelDef::s_iFrondVertDecl, 0.0f, gfx_depthFirst);

    for (size_t z(0); z < fronds.m_vpIBuffers.size(); ++z)
    {
        D3D_SetIndices(fronds.m_vpIBuffers[z]);
        D3D_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, fronds.m_iNumVerts, 0, fronds.m_viNumIndices[z] - 2);

        SceneStats.RecordBatch(fronds.m_iNumVerts, fronds.m_viNumIndices[z] - 2, ePhase, SSBATCH_TREEFROND);
    }
}
