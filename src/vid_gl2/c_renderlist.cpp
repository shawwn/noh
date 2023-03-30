// (C)2008 S2 Games
// c_renderlist.cpp
//
// Manages sorting of render batches
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_renderlist.h"
#include "i_renderer.h"
#include "c_scenebuffer.h"

#include "../k2/c_pool.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CRenderList g_RenderList;
//=============================================================================

/*====================
  CRenderList::CRenderList
  ====================*/
CRenderList::CRenderList()
{
}


/*====================
  CRenderList::~CRenderList
  ====================*/
CRenderList::~CRenderList()
{
}


/*====================
  CRenderList::Clear
  ====================*/
void    CRenderList::Clear()
{
    PROFILE("CRenderList::Clear");

    CBillboardRenderer::s_Pool.Reset();
    CBoxRenderer::s_Pool.Reset();
    CEffectTriangleRenderer::s_Pool.Reset();
    //CExtendedTriangleRenderer::s_Pool.Reset();
    CFoliageRenderer::s_Pool.Reset();
    CLineRenderer::s_Pool.Reset();
    CMeshRenderer::s_Pool.Reset();
    CPointRenderer::s_Pool.Reset();
    CScenePolyRenderer::s_Pool.Reset();
    CTerrainRenderer::s_Pool.Reset();
    CTreeBranchRenderer::s_Pool.Reset();
    CTreeFrondRenderer::s_Pool.Reset();
    CTreeLeafRenderer::s_Pool.Reset();
    CTreeBillboardRenderer::s_Pool.Reset();
    CSkyRenderer::s_Pool.Reset();
    
    m_vpRenderList.clear();
}


/*====================
  CRenderList::Add
  ====================*/
void    CRenderList::Add(IRenderer *pItem)
{
    PROFILE("CRenderList::Add");

    m_vpRenderList.push_back(pItem);
}


/*====================
  CRenderList::Setup
  ====================*/
void    CRenderList::Setup(EMaterialPhase ePhase)
{
    PROFILE_EX("CRenderList::Setup", PROFILE_LEAF);

    for (RenderList::iterator it = m_vpRenderList.begin(); it != m_vpRenderList.end(); ++it)
        (*it)->Setup(ePhase);
}


/*====================
  RenderListSort

  Return whether first element should be draw before the second
  ====================*/
static bool RenderListSort(IRenderer *a, IRenderer *b)
{
    if (a->GetRender() > b->GetRender())
        return true;
    else if (a->GetRender() < b->GetRender())
        return false;

    if (a->IsRefractive() < b->IsRefractive())
        return true;
    else if (a->IsRefractive() > b->IsRefractive())
        return false;

    if (a->GetLayer() < b->GetLayer())
        return true;
    else if (a->GetLayer() > b->GetLayer())
        return false;

    if (a->IsTranslucent() < b->IsTranslucent())
        return true;
    else if (a->IsTranslucent() > b->IsTranslucent())
        return false;

    if (a->IsTranslucent())
    {
        if (a->GetDepth() > b->GetDepth())
            return true;
        else if (a->GetDepth() < b->GetDepth())
            return false;
    }

    if (a->GetEffectLayer() < b->GetEffectLayer())
        return true;
    else if (a->GetEffectLayer() > b->GetEffectLayer())
        return false;

    if (a->GetShaderProgramInstance() < b->GetShaderProgramInstance())
        return true;
    else if (a->GetShaderProgramInstance() > b->GetShaderProgramInstance())
        return false;

    if (a->GetVertexBuffer() < b->GetVertexBuffer())
        return true;
    else if (a->GetVertexBuffer() > b->GetVertexBuffer())
        return false;

    if (!a->IsTranslucent())
    {
        if (a->GetDepth() < b->GetDepth())
            return true;
        else if (a->GetDepth() > b->GetDepth())
            return false;
    }

    return a < b;
}


/*====================
  CRenderList::Sort
  ====================*/
void    CRenderList::Sort()
{
    PROFILE("CRenderList::Sort");

    sort(m_vpRenderList.begin(), m_vpRenderList.end(), RenderListSort);
}


/*====================
  CRenderList::Render
  ====================*/
void    CRenderList::Render(EMaterialPhase ePhase)
{
#if 0
    PROFILE_EX("CRenderList::Render", PROFILE_LEAF);
#else
    PROFILE("CRenderList::Render");
#endif

    if (ePhase == PHASE_COLOR)
    {
        bool bSceneBufferUpdated(false);

        RenderList::iterator itEnd(m_vpRenderList.end());
        for (RenderList::iterator it(m_vpRenderList.begin()); it != itEnd; ++it)
        {
            if (!(*it)->GetRender())
                break;

            if (!bSceneBufferUpdated && (*it)->IsRefractive())
            {
                //if (g_bReflectPass)
                //  return;

                if (!g_SceneBuffer.GetActive() || g_pCam->GetFlags() & CAM_NO_SCENE_BUFFER)
                    return;

                g_SceneBuffer.Render();
                bSceneBufferUpdated = true;
            }

            (*it)->Render(ePhase);
        }
    }
    else
    {
        RenderList::iterator itEnd(m_vpRenderList.end());
        for (RenderList::iterator it(m_vpRenderList.begin()); it != itEnd; ++it)
        {
            if (!(*it)->GetRender())
                break;

            (*it)->Render(ePhase);
        }
    }
}
