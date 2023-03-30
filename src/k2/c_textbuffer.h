// (C)2005 S2 Games
// c_textbuffer.h
//
//=============================================================================
#ifndef __C_TEXTBUFFER_H__
#define __C_TEXTBUFFER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_draw2d.h"
//=============================================================================

class CTextBufferScrollbar;


    //MikeG added  bool m_bOutline; float m_fShadowOffsetY; float m_fShadowOffsetX; CVec4f m_v4OutlineColor; int m_iOutlineOffset;
//=============================================================================
// CTextBuffer
//=============================================================================
class CTextBuffer : public IWidget
{
protected:
    tsvector                m_vsText;
    uivector                m_viFadeTime;
    ColorVector             m_vFadeColor;
    ColorVector             m_vFadeShadowColor;
    ColorVector             m_vFadeOutlineColor;
    CRectf                  m_rectConstraints;
    ResHandle               m_hFontMap;
    EAlignment              m_eTextAlignment;
    bool                    m_bShadow;
    float                   m_fShadowOffset;
    bool                    m_bWrap;
    bool                    m_bAnchorBottom;
    bool                    m_bExteriorScrollbars;
    bool                    m_bUseSmileys;
    bool                    m_bScrollbarPlaceholder;
    bool                    m_bUseScrollbar;
    CVec4f                  m_v4TextColor;
    CVec4f                  m_v4ShadowColor;
    bool                    m_bOutline;
    float                   m_fShadowOffsetY;
    float                   m_fShadowOffsetX;
    CVec4f                  m_v4OutlineColor;
    int                     m_iOutlineOffset;
    uint                    m_iMaxLines;
    int                     m_iFadeLength;
    float                   m_fScrollbarSize;
    CTextBufferScrollbar*   m_pScrollbar;
    CFontMap*               m_pFontMap;
    tstring                 m_sFile;
    tstring                 m_sDefaultText;

    CVec2ui                 m_vSelStart;
    CVec2ui                 m_vStart;
    CVec2ui                 m_vInputPos;

    bool                    m_bEditable;

    bool                    m_bLinkToChat;
    bool                    m_bIsGameChat;
    tstring                 m_sChannelName;

    uint    WrapAddString(const tstring &sStr, uint uiInsertTime, tstring sStartingColorCode = TSNULL, uint uiInsertPos = -1);
    void    ScrollUp();
    void    ScrollDown();

public:
    K2_API ~CTextBuffer();
    K2_API CTextBuffer(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    tstring GetValue()  const               { return NormalizeLineBreaks(ConcatinateArgs(m_vsText, TLINEBREAK)); }

    CRectf  &GetConstraints()               { return m_rectConstraints; }
    const tstring &GetText()  const         { return m_vsText.back(); }

    void    SetText(const tstring &sStr);
    void    AddText(const tstring &sStr, uint uiInsertTime);
    CVec2ui AddTextAtPos(const tstring &sStr, CVec2ui v2Pos);
    void    ClearText();

    ResHandle   GetFont() const             { return m_hFontMap; }

    void    SetFont(const tstring &sFont);

    void    MouseDown(EButton button, const CVec2f &v2CursorPos);

    bool    ButtonUp(EButton button)        { return false; }
    bool    ButtonDown(EButton button);

    bool    Char(TCHAR c);

    void    RenderWidget(const CVec2f &v2Origin, float fFade);

    void    VerticalScrollbarChange(float fNewValue);
    void    UpdateScrollbars();

    void    Frame(uint uiFrameLength, bool bProcessFrame);

    void    ReloadFile();

    tstring GetCopyString();
    void    PasteString(const tstring &sString);
};
//=============================================================================

#endif //__C_TEXTBUFFER_H__
