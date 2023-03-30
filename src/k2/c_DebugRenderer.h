// (C)2006 S2 Games
// c_DebugRenderer.h
//
//=============================================================================
#ifndef __C_DEBUGRENDERER_H__
#define __C_DEBUGRENDERER_H__

//=============================================================================
// Headers
//=============================================================================
class CCamera;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
extern K2_API class CDebugRenderer &g_DebugRenderer;

#ifdef KELLOGS_SHARED_EXPORTS
#define DebugRenderer (*CDebugRenderer::GetInstance())
#else
#define DebugRenderer g_DebugRenderer
#endif

//=============================================================================

struct SLine
{
    CVec3f v3Src;
    CVec3f v3Dst;
    CVec4f v4Color;

    SLine(CVec3f &src, CVec3f &dst, CVec4f &color) : v3Src(src), v3Dst(dst), v4Color(color) { }
};

struct SRect
{
    CVec3f v3TL;
    CVec3f v3TR;
    CVec3f v3BL;
    CVec3f v3BR;
    CVec4f v4Color;

    SRect(CVec3f tl, CVec3f tr, CVec3f bl, CVec3f br, CVec4f color) : v3TL(tl), v3TR(tr), v3BL(bl), v3BR(br), v4Color(color) { }
};

typedef vector<SLine> LineVector;
typedef vector<SRect> RectVector;

//=============================================================================
// CDebugRenderer
//=============================================================================
class K2_API CDebugRenderer
{
    static CDebugRenderer *s_pDebugRenderer;

private:
    RectVector m_vecWorldRects;
    LineVector m_vecWorldLines;

public:
    void AddLine(CVec3f v3Src, CVec3f v3Dst, CVec4f v4Color);
    void AddRect(CVec3f v3TL, CVec3f v3TR, CVec3f v3BL, CVec3f v3BR, CVec4f v4Color);

    void Frame(CCamera *pCamera);
    void ClearLists();

    static CDebugRenderer *GetInstance();
};
//=============================================================================



#endif //__C_DEBUGRENDERER_H__
