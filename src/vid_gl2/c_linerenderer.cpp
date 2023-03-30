// (C)2008 S2 Games
// c_linerenderer.cpp
//
// Line batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_linerenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
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

CPool<CLineRenderer> CLineRenderer::s_Pool(1, uint(-1));

/*====================
  CLineRenderer::operator new
  ====================*/
void*	CLineRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
	return s_Pool.Allocate();
}


/*====================
  CLineRenderer::CLineRenderer
  ====================*/
CLineRenderer::CLineRenderer(int iNumLines) :
m_iNumLines(iNumLines)
{
}


/*====================
  CLineRenderer::~CLineRenderer
  ====================*/
CLineRenderer::~CLineRenderer()
{
}


/*====================
  CLineRenderer::Render
  ====================*/
void	CLineRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CLineRenderer::Render");

	if (!m_bRender)
		return;

	SetShaderVars();

	GfxMaterials->SelectMaterial(g_SimpleMaterial3DColored, ePhase, 0.0f, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, Gfx3D->VBLine);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SLineVertex), BUFFER_OFFSET(12));
	glVertexPointer(3, GL_FLOAT, sizeof(SLineVertex), BUFFER_OFFSET(0));

	glLineWidth(1.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glDrawArrays(GL_LINES, 0, m_iNumLines * 2);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	SceneStats.RecordBatch(m_iNumLines * 2, m_iNumLines, ePhase, SSBATCH_DEBUG);
}
