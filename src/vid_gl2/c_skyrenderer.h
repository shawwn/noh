// (C)2007 S2 Games
// c_skyrenderer.h
//
//=============================================================================
#ifndef __C_SKYRENDERER_H__
#define __C_SKYRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_renderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CSkyRenderer
//=============================================================================
class CSkyRenderer : public IRenderer
{
private:

public:
    static CPool<CSkyRenderer>      s_Pool;
    
    void*   operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p) { }
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
    
    ~CSkyRenderer();
    CSkyRenderer();

    void    Setup(EMaterialPhase ePhase);
    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_BILLBOARDRENDERITEM_H__
