// (C)2006 S2 Games
// c_extendedtrianglerenderer.h
//
//=============================================================================
#ifndef __C_EXTENDEDTRIANGLERENDERER_H__
#define __C_EXTENDEDTRIANGLERENDERER_H__

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
// CExtendedTriangleRenderer
//=============================================================================
class CExtendedTriangleRenderer : public IEffectRenderer
{
private:

public:
    static CPool<CExtendedTriangleRenderer>     s_Pool;
    
    void*   operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
    
    ~CExtendedTriangleRenderer();
    CExtendedTriangleRenderer(ResHandle hMaterial, uint uiStartIndex, uint uiEndIndex, int iEffectLayer, float fDepth);

    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_EXTENDEDTRIANGLERENDERER_H__
