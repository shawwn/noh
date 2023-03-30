// (C)2008 S2 Games
// c_boxrenderer.cpp
//
// Box batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_boxrenderer.h"

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

CPool<CBoxRenderer> CBoxRenderer::s_Pool(1, uint(-1));

/*====================
  CBoxRenderer::operator new
  ====================*/
void*   CBoxRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
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
void    CBoxRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CBoxRenderer::Render");

    if (!m_bRender)
        return;

    SetShaderVars();

    GfxMaterials->SelectMaterial(g_SimpleMaterial3DColored, ePhase, 0.0f, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, Gfx3D->VBBox);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SLineVertex), BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, sizeof(SLineVertex), BUFFER_OFFSET(0));

    glLineWidth(1.0f);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, Gfx3D->IBBox);
    glDrawElements(GL_LINES, m_iNumBoxes * 24, GL_UNSIGNED_SHORT, NULL);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    SceneStats.RecordBatch(m_iNumBoxes * 8, m_iNumBoxes * 12, ePhase, SSBATCH_DEBUG);
}
