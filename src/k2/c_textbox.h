// (C)2005 S2 Games
// c_textbox.h
//
//=============================================================================
#ifndef __C_TEXTBOX_H__
#define __C_TEXTBOX_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inputwidget.h"

#include "../k2/c_input.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CFontMap;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const float TEXTBOX_CURSOR_HEIGHT(2.0f);
//=============================================================================

//=============================================================================
// CTextBox
//=============================================================================
class CTextBox : public IInputWidget
{
protected:
    tstring     m_sFont;
    ResHandle   m_hFontMap;
    CFontMap*   m_pFontMap;
    CVec4f      m_v4TextColor;

    tstring     m_sHPadding;
    tstring     m_sVPadding;
    float       m_fHPadding;
    float       m_fVPadding;
    uint        m_uMaxLength;
    bool        m_bWrap;

    tstring     m_sOnEnter;
    tstring     m_sOnEsc;

    size_t      m_zSelStart;

    ResHandle   m_hCursor;
    uint        m_uiCursorPhase;

    bool        m_bMidClick;
    bool        m_bSelectOnFocus;
    bool        m_bNewFocus;

    uint        m_uiDoubleClickTime;
    uint        m_uiLastClickTime;
    size_t      m_zLastClickPos;

public:
    ~CTextBox() {}
    CTextBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    void    Clear();
    void    SetInputLine(const tstring &sInputLine);

    void    SetTextColor(const CVec4f &v4Color)     { m_v4TextColor = v4Color; }

    tstring GetValue() const    { if (m_sPasswordChar.empty()) return m_sInputLine; else return m_sHiddenLine; }

    virtual bool    ProcessInputCursor(const CVec2f &v2CursorPos);
    virtual void    MouseDown(EButton button, const CVec2f &v2CursorPos);
    virtual void    MouseUp(EButton button, const CVec2f &v2CursorPos);

    virtual bool    ButtonDown(EButton button);
    virtual bool    ButtonUp(EButton button);

    virtual void    Rollover();
    virtual void    Rolloff();

    virtual void    Focus();
    virtual void    LoseFocus();

    virtual bool    Char(TCHAR c);

    virtual void    RenderWidget(const CVec2f &vOrigin, float fFade);

    virtual void    RecalculateChildSize();

    void    SetInputPos(size_t zInputPos);

    tstring GetCopyString();
    void    PasteString(const tstring &sString);
};
//=============================================================================

#endif // __C_TEXTBOX_H__
