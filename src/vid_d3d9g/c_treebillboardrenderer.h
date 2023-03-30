// (C)2007 S2 Games
// c_treebillboardrenderer.h
//
//=============================================================================
#ifndef __C_TREEBILLBOARDRENDERER_H__
#define __C_TREEBILLBOARDRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_effectrenderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CTreeBillboardRenderer
//=============================================================================
class CTreeBillboardRenderer : public IRenderer
{
private:
    ResHandle       m_hMaterial;
    dword           m_dwAlphaTest;
    uint            m_uiStartIndex;
    uint            m_uiEndIndex;

public:
    static CPool<CTreeBillboardRenderer>        s_Pool;
    
    void*   operator new(size_t z); // Uses CPool of preallocated instances
    
    ~CTreeBillboardRenderer();
    CTreeBillboardRenderer(ResHandle hMaterial, dword dwAlphaTest, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

    void    Setup(EMaterialPhase ePhase);
    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_BILLBOARDRENDERITEM_H__
