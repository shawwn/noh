// (C)2006 S2 Games
// c_boxrenderer.cpp
//
// Box batch renderer
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
#include "c_boxrenderer.h"

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

CPool<CBoxRenderer> CBoxRenderer::s_Pool(1, -1);

/*====================
  CBoxRenderer::operator new
  ====================*/
void*	CBoxRenderer::operator new(size_t z)
{
	return s_Pool.Allocate();
}


/*====================
  CBoxRenderer::CBoxRenderer
  ====================*/
CBoxRenderer::CBoxRenderer(int iNumBoxes) :
m_iNumBoxes(iNumBoxes)
{
}


/*====================
  CBoxRenderer::~CBoxRenderer
  ====================*/
CBoxRenderer::~CBoxRenderer()
{
}


/*====================
  CBoxRenderer::Render
  ====================*/
void	CBoxRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CBoxRenderer::Render");

	if (!m_bRender)
		return;

	SetShaderVars();

	D3D_SetStreamSource(0, g_pVBBox, 0, sizeof(SLineVertex));
	D3D_SetIndices(g_pIBBox);

	D3D_SelectMaterial(g_SimpleMaterial3DColored, ePhase, VERTEX_LINE, g_pCam->GetTime(), gfx_depthFirst);

	D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	D3D_SetRenderState(D3DRS_ZENABLE, FALSE);
	D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	D3D_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	D3D_DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_iNumBoxes * 8, 0, m_iNumBoxes * 12);

	SceneStats.RecordBatch(m_iNumBoxes * 8, m_iNumBoxes * 12, ePhase, SSBATCH_DEBUG);
}
