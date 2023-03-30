// (C)2008 S2 Games
// c_effecttrianglerenderer.cpp
//
// Effect triangle batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_effecttrianglerenderer.h"

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

CPool<CEffectTriangleRenderer> CEffectTriangleRenderer::s_Pool(1, uint(-1));

/*====================
  CEffectTriangleRenderer::operator new
  ====================*/
void*   CEffectTriangleRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
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
    if (!m_bRender)
        return;

    SetShaderVars();

    CMaterial &cMaterial(GfxUtils->GetMaterial(m_hMaterial));

    const SMaterialState &cMaterialState(GfxMaterials->GetMaterialState());
    if (cMaterialState.ePhase == ePhase && cMaterialState.pMaterial == &cMaterial)
    {
        // Do nothing!
    }
    else
    {
        GfxMaterials->SelectMaterial(cMaterial, ePhase, 0.0f, 0);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, Gfx3D->VBEffectTriangle);
    glTexCoordPointer(4, GL_FLOAT, 32, BUFFER_OFFSET(16));
    glColorPointer(4, GL_UNSIGNED_BYTE, 32, BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, 32, BUFFER_OFFSET(0));

    glDrawArrays(GL_TRIANGLES, m_uiStartIndex * 3, (m_uiEndIndex - m_uiStartIndex) * 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    SceneStats.RecordBatch((m_uiEndIndex - m_uiStartIndex) * 3, m_uiEndIndex - m_uiStartIndex, ePhase, SSBATCH_EFFECT);
}
