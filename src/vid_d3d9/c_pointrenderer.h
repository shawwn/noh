// (C)2006 S2 Games
// c_pointrenderer.h
//
//=============================================================================
#ifndef __C_POINTRENDERER_H__
#define __C_POINTRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_debugrenderer.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CPointRenderer
//=============================================================================
class CPointRenderer : public IDebugRenderer
{
private:
    int     m_iNumPoints;

public:
    static CPool<CPointRenderer>        s_Pool;
    
    void*   operator new(size_t z, const char *szContext = NULL, const char *szType = NULL, const char *szFile = NULL, short nLine = 0); // Uses CPool of preallocated instances
    void    operator delete(void *p, const char *szContext, const char *szType, const char *szFile, short nLine) { }
    
    ~CPointRenderer();
    CPointRenderer(int iNumPoints);

    void    Render(EMaterialPhase ePhase);
};
//=============================================================================
#endif //__C_POINTRENDERER_H__
