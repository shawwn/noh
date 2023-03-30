// (C)2006 S2 Games
// c_pointrenderer.cpp
//
// Point batch renderer
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
#include "c_pointrenderer.h"

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

CPool<CPointRenderer> CPointRenderer::s_Pool(1, -1);

/*====================
  CPointRenderer::operator new
  ====================*/
void*   CPointRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CPointRenderer::CPointRenderer
  ====================*/
CPointRenderer::CPointRenderer(int iNumPoints) :
m_iNumPoints(iNumPoints)
{
}


/*====================
  CPointRenderer::~CPointRenderer
  ====================*/
CPointRenderer::~CPointRenderer()
{
}


/*====================
  CPointRenderer::Render
  ====================*/
void    CPointRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CPointRenderer::Render");

    if (!m_bRender)
        return;

    SetShaderVars();

    D3D_SetStreamSource(0, g_pVBPoint, 0, sizeof(SLineVertex));

    D3D_SelectMaterial(g_SimpleMaterial3DColored, ePhase, VERTEX_LINE, g_pCam->GetTime(), gfx_depthFirst);

    D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(4.0f));
    D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    D3D_SetRenderState(D3DRS_ZENABLE, FALSE);
    D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    D3D_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

    D3D_DrawPrimitive(D3DPT_POINTLIST, 0, m_iNumPoints);

    SceneStats.RecordBatch(m_iNumPoints, m_iNumPoints, ePhase, SSBATCH_DEBUG);
}
