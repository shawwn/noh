// (C)2005 S2 Games
// c_draw2d.h
//
//=============================================================================
#ifndef __C_DRAW2D_H__
#define __C_DRAW2D_H__

//=============================================================================
// Headers
//=============================================================================
#include "k2_api.h"
#include "k2_singleton.h"

#include "../public/vid_driver_t.h"
#include "c_vec.h"
#include "i_resourcelibrary.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const int DRAW_STRING_WRAP          (BIT(0));
const int DRAW_STRING_ANCHOR_BOTTOM (BIT(1));
const int DRAW_STRING_CENTER        (BIT(2));
const int DRAW_STRING_VCENTER       (BIT(3));
const int DRAW_STRING_SMILEYS       (BIT(4));
const int DRAW_STRING_NOCOLORCODES  (BIT(5));
const int DRAW_STRING_RIGHT         (BIT(6));
const int DRAW_STRING_BOTTOM        (BIT(7));
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct SShader;
//=============================================================================

//=============================================================================
// CDraw2D
//=============================================================================
class CDraw2D
{
SINGLETON_DEF(CDraw2D)

private:
    CVec4f  m_v4CurrentColor;
    CVec4f  m_v4CurrentFGColor;
    CVec4f  m_v4CurrentBGColor;
    map<tstring, ResHandle> m_mapSmileys;
    ResHandle   m_hGlow;

public:
    ~CDraw2D();

    K2_API void     Clear();

    K2_API void     Quad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4,
                                const CVec2f& t1, const CVec2f& t2, const CVec2f& t3, const CVec2f& t4, ResHandle hTexture, int iFlags = 0);
    K2_API void     Quad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4, int iFlags = 0);
    K2_API void     Quad(const CVec2f& v1, const CVec2f& v2, const CVec2f& v3, const CVec2f& v4, ResHandle hTexture, int iFlags = 0)
                        { Quad(v1, v2, v3, v4, CVec2f(0.0f, 0.0f), CVec2f(0.0f, 1.0f), CVec2f(1.0f, 0.0f), CVec2f(1.0f, 1.0f), hTexture, iFlags = 0); }

    K2_API void     Rect(float x, float y, float w, float h, float s1, float t1, float s2, float t2, ResHandle hTexture, int iFlags = 0);
    K2_API void     Rect(float x, float y, float w, float h, int iFlags = 0);
    inline void         Rect(float x, float y, float w, float h, ResHandle hTexture, int iFlags = 0)
                        { Rect(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f, hTexture, iFlags); }
    inline void         Rect(const CRectf &rec, ResHandle hTexture, int iFlags = 0)
                        { Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight(), 0.0f, 0.0f, 1.0f, 1.0f, hTexture, iFlags); }

    K2_API void     RectOutline(const CRectf &rect, float fThickness);
    K2_API void     RectOutline(const CRectf &rect, float fThickness, CVec4f v4Color);
    inline void         RectOutline(float x, float y, float w, float h, float fThickness)
                        { RectOutline(CRectf(x, y, x + w, y + h), fThickness); }
    inline void         RectOutline(float x, float y, float w, float h, float fThickness, CVec4f v4Color)
                        { RectOutline(CRectf(x, y, x + w, y + h), fThickness, v4Color); }

    K2_API void     FilledRect(const CRectf &rect, float fThickness, CVec4f v4BorderColor, CVec4f v4FillColor);
    K2_API void     Poly(vec2_t v1, vec2_t v2, vec2_t v3, vec2_t v4, float s1, float t1, float s2, float t2, ResHandle hTexture);

    K2_API void     String(float x, float y, const tstring &sStr, ResHandle hFont);
    K2_API void     String(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, int iFlags = 0, size_t zSelectionStart = -1, size_t zSelectionEnd = -1, float fStartingXOffset = 0.0f);
    K2_API void     String(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, uivector vWrappingBreakList, fvector vLineCentering, int iFlags, bool bShadow, bool bOutline, float fXOffset, float fYOffset, CVec4f v4ShadowColor);
    void                String(const CRectf &rect, const tstring &sStr, ResHandle hFont, int iFlags = 0, size_t zSelectionStart = -1, size_t zSelectionEnd = -1, float fStartingXOffset = 0.0f)
                        { String(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), sStr, hFont, iFlags, zSelectionStart, zSelectionEnd, fStartingXOffset); }

    K2_API void     String(float x, float y, float w, float h, const tsvector &vStr, ResHandle hFont,
                                int iFlags = 0, const ColorVector *pColors = nullptr);
    void                String(const CRectf &rect, const tsvector &vStr, ResHandle hFont, int iFlags = 0)
                        { String(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), vStr, hFont, iFlags); }
    K2_API void     String(float x, float y, float w, float h, const tsvector &vStr, const ColorVector &vColors,
                                ResHandle hFont, int iFlags = 0);
    void            String(const CRectf &rect, const tsvector &vStr, const ColorVector &vColors, ResHandle hFont, int iFlags = 0)
                        { String(rect.left, rect.top, rect.GetWidth(), rect.GetHeight(), vStr, vColors, hFont, iFlags); }

    void            String(float x, float y, float w, float h, const tsvector &vStr, float fStartingYOffset, ResHandle hFont, int iFlags);

    void            String(float x, float y, float w, float h, const tsvector &vStr, CVec2f vfStartingOffset, ResHandle hFont, const ColorVector *vColors, int iFlags, CVec2ui v2SelStart = CVec2ui(uint(-1),uint(-1)), CVec2ui v2SelEnd = CVec2ui(uint(-1),uint(-1)));

    void            StringGlow(float x, float y, const tstring &sStr, ResHandle hFont, StrColorMap &vColors);
    void            StringGlow(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, int iFlags, float fStartingXOffset, StrColorMap &vColors);
    void            StringGlow(float x, float y, float w, float h, const tstring &sStr, ResHandle hFont, uivector vWrappingBreakList, fvector vLineCentering, int iFlags, StrColorMap &vColors);

    K2_API void         SetColor(const CVec4f &v4Color);
    void                SetColor(const CVec3f &v3Color)                     { SetColor(CVec4f(v3Color[R], v3Color[G], v3Color[B], 1.0f)); }
    void                SetColor(float fR, float fG, float fB, float fA)    { SetColor(CVec4f(fR, fG, fB, fA)); }
    void                SetColor(float fR, float fG, float fB)              { SetColor(CVec4f(fR, fG, fB, 1.0f)); }
    void                SetAlpha(float fA)                                  { SetColor(m_v4CurrentColor[R], m_v4CurrentColor[R], m_v4CurrentColor[R], fA); }
    CVec4f              GetCurrentColor() const                             { return m_v4CurrentColor; }
    float               GetCurrentAlpha() const                             { return m_v4CurrentColor[A]; }
    void                SetFGColor(const CVec4f &v4Color)                   { m_v4CurrentFGColor = v4Color; }
    void                SetBGColor(const CVec4f &v4Color)                   { m_v4CurrentBGColor = v4Color; }

    K2_API void     Line(const CVec2f& v1, const CVec2f& v2, const CVec4f& v4Color1, const CVec4f& v4Color2, int iFlags = 0);

    K2_API void     SetRegion(int x, int y, int w, int h);
    K2_API float    GetScreenW();
    K2_API float    GetScreenH();

    K2_API void     RegisterSmiley(const tstring &sSmiley, const tstring &sPath);
};

extern K2_API CDraw2D *pDraw2D;
extern K2_API CDraw2D &Draw2D;
//=============================================================================
#endif // __C_DRAW2D_H__
