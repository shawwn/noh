// (C)2008 S2 Games
// c_effectquadrenderer.cpp
//
// Effect Quad batch renderer
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
#include "c_effectquadrenderer.h"

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

CPool<CEffectQuadRenderer> CEffectQuadRenderer::s_Pool(1, -1);

/*====================
  CEffectQuadRenderer::operator new
  ====================*/
void*   CEffectQuadRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CEffectQuadRenderer::CEffectQuadRenderer
  ====================*/
CEffectQuadRenderer::CEffectQuadRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
IEffectRenderer(hMaterial, uiStartIndex, uiEndIndex, iEffectLayer, fDepth)
{
}


/*====================
  CEffectQuadRenderer::~CEffectQuadRenderer
  ====================*/
CEffectQuadRenderer::~CEffectQuadRenderer()
{
}


/*====================
  CEffectQuadRenderer::Render
  ====================*/
void    CEffectQuadRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CEffectQuadRenderer::Render");

    if (!m_bRender)
        return;

    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

    SetShaderVars();

    D3D_SetStreamSource(0, g_pVBEffectQuad, 0, sizeof(SEffectVertex));
    D3D_SetIndices(g_pIBEffectQuad);

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

    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_uiStartIndex * 4, (m_uiEndIndex - m_uiStartIndex) << 2, m_uiStartIndex * 6, (m_uiEndIndex - m_uiStartIndex) << 1);

    SceneStats.RecordBatch((m_uiEndIndex - m_uiStartIndex) * 4, (m_uiEndIndex - m_uiStartIndex) * 2, ePhase, SSBATCH_EFFECT);
}
