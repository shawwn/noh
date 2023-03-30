// (C)2005 S2 Games
// c_renderlist.h
//
//=============================================================================
#ifndef __C_RENDERLIST_H__
#define __C_RENDERLIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_billboardrenderer.h"
#include "c_effecttrianglerenderer.h"
#include "c_meshrenderer.h"
#include "c_scenepolyrenderer.h"
#include "c_treebranchrenderer.h"
#include "c_treefrondrenderer.h"
#include "c_treeleafrenderer.h"
#include "c_treebillboardrenderer.h"
#include "c_terrainrenderer.h"
#include "c_foliagerenderer.h"
#include "c_skyrenderer.h"
#include "c_boxrenderer.h"
#include "c_pointrenderer.h"
#include "c_linerenderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class   IRenderer;
enum    EMaterialPhase;
template <class T> class CPool;

typedef vector<IRenderer *> RenderList;
//=============================================================================

//=============================================================================
// CRenderList
//=============================================================================
class CRenderList
{
private:
    RenderList      m_vpRenderList;

public:
    ~CRenderList();
    CRenderList();

    void    Clear();
    void    Add(IRenderer *pItem);

    void    Setup(EMaterialPhase ePhase);
    void    Sort();
    void    Render(EMaterialPhase ePhase);
};

extern CRenderList  g_RenderList;
//=============================================================================
#endif //__C_RENDERLIST_H__
