// (C)2006 S2 Games
// c_extendedtrianglerenderer.cpp
//
// Extended triangle batch renderer
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
#include "c_extendedtrianglerenderer.h"

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

CPool<CExtendedTriangleRenderer> CExtendedTriangleRenderer::s_Pool(1, -1);

/*====================
  CExtendedTriangleRenderer::operator new
  ====================*/
void*	CExtendedTriangleRenderer::operator new(size_t z)
{
	return s_Pool.Allocate();
}


/*====================
  CExtendedTriangleRenderer::CExtendedTriangleRenderer
  ====================*/
CExtendedTriangleRenderer::CExtendedTriangleRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
IEffectRenderer(hMaterial, uiStartIndex, uiEndIndex, iEffectLayer, fDepth)
{
}


/*====================
  CExtendedTriangleRenderer::~CExtendedTriangleRenderer
  ====================*/
CExtendedTriangleRenderer::~CExtendedTriangleRenderer()
{
}


/*====================
  CExtendedTriangleRenderer::Render
  ====================*/
void	CExtendedTriangleRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CExtendedTriangleRenderer::Render");

	if (!m_bRender)
		return;

	D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	SetShaderVars();

	D3D_SetStreamSource(0, g_pVBExtendedTriangle, 0, sizeof(SExtendedVertex));

	D3D_SelectMaterial(D3D_GetMaterial(m_hMaterial), ePhase, VERTEX_EXTENDED, g_pCam->GetTime(), gfx_depthFirst);
	D3D_DrawPrimitive(D3DPT_TRIANGLELIST, m_uiStartIndex * 3, m_uiEndIndex - m_uiStartIndex);

	SceneStats.RecordBatch((m_uiEndIndex - m_uiStartIndex) * 3, m_uiEndIndex - m_uiStartIndex, ePhase, SSBATCH_EFFECT);
}
