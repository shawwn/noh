// (C)2006 S2 Games
// c_effecttrianglerenderer.h
//
//=============================================================================
#ifndef __C_EFFECTTRIANGLERENDERER_H__
#define __C_EFFECTTRIANGLERENDERER_H__

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
// CEffectTriangleRenderer
//=============================================================================
class CEffectTriangleRenderer : public IEffectRenderer
{
private:

public:
    static CPool<CEffectTriangleRenderer>       s_Pool;
    
    void*   operator new(size_t z, const char *szContext = nullptr, const char *szType = nullptr, const char *szFile = nullptr, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p) { }
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
    
    ~CEffectTriangleRenderer();
    CEffectTriangleRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_EFFECTTRIANGLERENDERER_H__
