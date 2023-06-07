// (C)2005 S2 Games
// c_label.h
//
//=============================================================================
#ifndef __C_LABEL_H__
#define __C_LABEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

    //MikeG added  bool m_bOutline; float m_fShadowOffsetY; float m_fShadowOffsetX; CVec4f m_v4OutlineColor; int m_iOutlineOffset;
//=============================================================================
// CLabel
//=============================================================================
class CLabel : public IWidget
{
protected:
    tstring     m_sText;
    tstring     m_sRenderText;
    float       m_fWrapCount;
    float       m_fBiggestWidth;
    CRectf      m_rectLabelConstraints;
    ResHandle   m_hFontMap;
    int         m_iDrawFlags;
    bool        m_bShadow;
    bool        m_bWrap;
    bool        m_bLineRet;
    bool        m_bFitX;
    tstring     m_sFitXPadding;
    tstring     m_sFitXMax;
    bool        m_bFitY;
    tstring     m_sFitYPadding;
    int         m_iPrecision;
    CVec4f      m_v4ShadowColor;
    float       m_fShadowOffset;
    bool        m_bOutline;
    float       m_fShadowOffsetY;
    float       m_fShadowOffsetX;
    float       m_fOutlineOffset;
    CVec4f      m_v4OutlineColor;
    uivector    m_vLineWrap;
    fvector     m_vLineCentering;

public:
    K2_API ~CLabel()    {}
    K2_API CLabel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    tstring         GetValue() const                                    { return m_sText; }

    CRectf&         GetTextConstraints()                                { return m_rectLabelConstraints; }
    const tstring&  GetText() const                                     { return m_sText; }

    K2_API void     SetText(const tstring &sStr);

    ResHandle       GetFont() const                                     { return m_hFontMap; }
    void            SetFont(const tstring &sFont);

    void            RecalculateSize();
    void            RecalculateText();
    void            NullSize();

    void            RenderWidget(const CVec2f &vOrigin, float fFade);
};
//=============================================================================

#endif //__C_LABEL_H__
