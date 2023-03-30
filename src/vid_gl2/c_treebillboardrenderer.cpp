// (C)2008 S2 Games
// c_billboardrenderer.cpp
//
// Billboard batch renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_treebillboardrenderer.h"

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

CPool<CTreeBillboardRenderer> CTreeBillboardRenderer::s_Pool(1, uint(-1));

/*====================
  CTreeBillboardRenderer::operator new
  ====================*/
void*   CTreeBillboardRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeBillboardRenderer::CTreeBillboardRenderer
  ====================*/
CTreeBillboardRenderer::CTreeBillboardRenderer(ResHandle hMaterial, dword dwAlphaTest, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth) :
m_hMaterial(hMaterial),
m_dwAlphaTest(dwAlphaTest),
m_uiStartIndex(uiStartIndex),
m_uiEndIndex(uiEndIndex)
{
    m_iNumActiveBones = 0;
    m_pCurrentEntity = NULL;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;

    // Set static sorting variables
    m_iEffectLayer = iEffectLayer;
    m_fDepth = fDepth;
    m_uiVertexBuffer = Gfx3D->VBTreeBillboard;
}


/*====================
  CTreeBillboardRenderer::~CTreeBillboardRenderer
  ====================*/
CTreeBillboardRenderer::~CTreeBillboardRenderer()
{
}


/*====================
  CTreeBillboardRenderer::Setup
  ====================*/
void    CTreeBillboardRenderer::Setup(EMaterialPhase ePhase)
{
    CMaterial &material(GfxUtils->GetMaterial(m_hMaterial));

    if (!material.HasPhase(ePhase))
        return; // Leave if we don't have this phase

    m_pCam = g_pCam;

    m_mWorldViewProj = g_mViewProj;

    // Set dynamic sorting variables
    m_bTranslucent = material.GetPhase(ePhase).GetTranslucent();
    m_iLayer = material.GetPhase(ePhase).GetLayer();
    m_bRefractive = material.GetPhase(ePhase).GetRefractive();
    
    m_bRender = true;
}


/*====================
  CTreeBillboardRenderer::Render
  ====================*/
void    CTreeBillboardRenderer::Render(EMaterialPhase ePhase)
{
    if (!m_bRender)
        return;

    SetShaderVars();
    glAlphaFunc(GL_GREATER, m_dwAlphaTest);
    GfxMaterials->SelectMaterial(GfxUtils->GetMaterial(m_hMaterial), ePhase, g_pCam->GetTime(), 0);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, Gfx3D->VBTreeBillboard);
    glTexCoordPointer(2, GL_FLOAT, 24, BUFFER_OFFSET(16));
    glColorPointer(4, GL_UNSIGNED_BYTE, 24, BUFFER_OFFSET(12));
    glVertexPointer(3, GL_FLOAT, 24, BUFFER_OFFSET(0));

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, Gfx3D->IBTreeBillboard);
    glDrawRangeElements(GL_TRIANGLES, m_uiStartIndex * 4, m_uiEndIndex * 4, (m_uiEndIndex - m_uiStartIndex) * 6,
                        GL_UNSIGNED_SHORT, BUFFER_OFFSET(m_uiStartIndex * 6 * sizeof(unsigned short)));
}
