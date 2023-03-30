// (C)2006 S2 Games
// c_treebranchrenderer.cpp
//
// SpeedTree branch renderer
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
#include "c_treebranchrenderer.h"
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
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CTreeBranchRenderer> CTreeBranchRenderer::s_Pool(1, -1);

/*====================
  CTreeBranchRenderer::operator new
  ====================*/
void*   CTreeBranchRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeBranchRenderer::CTreeBranchRenderer
  ====================*/
CTreeBranchRenderer::CTreeBranchRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
        const D3DXMATRIXA16 &mWorldViewProj,
        const D3DXMATRIXA16 &mWorld,
        const D3DXMATRIXA16 &mWorldRotate) :
IRenderer(RT_UNKNOWN),
m_cEntity(cEntity),
m_pTreeDef(pTreeDef)
{
    PROFILE("CTreeBranchRenderer::CTreeBranchRenderer");

    m_mWorldViewProj = mWorldViewProj;
    m_mWorld = mWorld;
    m_mWorldRotate = mWorldRotate;

    m_LOD = m_pTreeDef->GetDiscreetBranchLOD();
}


/*====================
  CTreeBranchRenderer::~CTreeBranchRenderer
  ====================*/
CTreeBranchRenderer::~CTreeBranchRenderer()
{
}


/*====================
  CTreeBranchRenderer::Setup
  ====================*/
void    CTreeBranchRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CTreeBranchRenderer::Setup");

    m_bRender = false; // Set to true if we make it to the end of the function

    if (m_LOD.m_iLOD == -1)
        return;

    if (!m_pTreeDef->HasBranchLOD(m_LOD.m_iLOD))
    {
        //Console.Err << _T("CTreeBranchRenderer::Setup() - Invalid LOD: ") << m_LOD.m_iLOD << newl;
        return;
    }

    if (!m_pTreeDef->HasBranchGeometry())
        return;

    CMaterial &material(D3D_GetMaterial(m_pTreeDef->GetBranchMaterial()));

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

    const STreeGeometryBuffers &branches(m_pTreeDef->GetBranchGeometry(m_LOD.m_iLOD));

    // Set sorting variables
    m_bTranslucent = cPhase.GetTranslucent();
    m_iLayer = cPhase.GetLayer();
    m_iEffectLayer = 0;
    m_iVertexType = CTreeModelDef::s_iBranchVertDecl;
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
    m_uiVertexBuffer = reinterpret_cast<size_t>(branches.m_pVBuffer);
    m_uiIndexBuffer = 0;
    m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CTreeBranchRenderer::Render
  ====================*/
void    CTreeBranchRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CTreeBranchRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    if (ePhase == PHASE_COLOR && m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && D3D_GetMaterial(m_pTreeDef->GetBranchMaterial()).HasPhase(PHASE_FADE))
        ePhase = PHASE_FADE;

    const STreeGeometryBuffers &branches(m_pTreeDef->GetBranchGeometry(m_LOD.m_iLOD));
    D3D_SetStreamSource(0, branches.m_pVBuffer, 0, CTreeModelDef::s_uiBranchVertSize);
    D3D_SelectMaterial(D3D_GetMaterial(m_pTreeDef->GetBranchMaterial()), ePhase, CTreeModelDef::s_iBranchVertDecl, 0.0f, gfx_depthFirst);

    // Set custom render states
    D3D_SetRenderState(D3DRS_ALPHAREF, m_LOD.m_dwAlphaTest);

    for (size_t z(0); z < branches.m_vpIBuffers.size(); ++z)
    {
        D3D_SetIndices(branches.m_vpIBuffers[z]);
        D3D_DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, branches.m_iNumVerts, 0, branches.m_viNumIndices[z] - 2);

        SceneStats.RecordBatch(branches.m_iNumVerts, branches.m_viNumIndices[z] - 2, ePhase, SSBATCH_TREEBRANCH);
    }
}
