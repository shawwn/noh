// (C)2006 S2 Games
// c_effecttrianglerenderer.cpp
//
// Effect triangle batch renderer
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
#include "c_effecttrianglerenderer.h"

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

CPool<CEffectTriangleRenderer> CEffectTriangleRenderer::s_Pool(1, -1);

/*====================
  CEffectTriangleRenderer::operator new
  ====================*/
void*   CEffectTriangleRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CEffectTriangleRenderer::CEffectTriangleRenderer
  ====================*/
CEffectTriangleRenderer::CEffectTriangleRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
IEffectRenderer(hMaterial, uiStartIndex, uiEndIndex, iEffectLayer, fDepth)
{
}


/*====================
  CEffectTriangleRenderer::~CEffectTriangleRenderer
  ====================*/
CEffectTriangleRenderer::~CEffectTriangleRenderer()
{
}


/*====================
  CEffectTriangleRenderer::Render
  ====================*/
void    CEffectTriangleRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CEffectTriangleRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    D3D_SetStreamSource(0, g_pVBEffectTriangle, 0, sizeof(SEffectVertex));

    CMaterial &cMaterial(D3D_GetMaterial(m_hMaterial));

    const SMaterialState &cMaterialState(D3D_GetMaterialState());
    if (cMaterialState.ePhase == ePhase && g_iCurrentVertexShader == m_iVertexShaderInstance)
    {
        D3D_SelectPixelShader(cMaterial, ePhase, g_pCam->GetTime());
    }
    else
    {
        D3D_SelectMaterial(cMaterial, ePhase, VERTEX_EFFECT, g_pCam->GetTime(), gfx_depthFirst);
    }

    D3D_DrawPrimitive(D3DPT_TRIANGLELIST, m_uiStartIndex * 3, m_uiEndIndex - m_uiStartIndex);

    SceneStats.RecordBatch((m_uiEndIndex - m_uiStartIndex) * 3, m_uiEndIndex - m_uiStartIndex, ePhase, SSBATCH_EFFECT);
}
