// (C)2006 S2 Games
// c_billboardrenderer.h
//
//=============================================================================
#ifndef __C_BILLBOARDRENDERER_H__
#define __C_BILLBOARDRENDERER_H__

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
// CBillboardRenderer
//=============================================================================
class CBillboardRenderer : public IEffectRenderer
{
private:

public:
    static CPool<CBillboardRenderer>        s_Pool;
    
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p) { }
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
    
    ~CBillboardRenderer();
    CBillboardRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_BILLBOARDRENDERITEM_H__
