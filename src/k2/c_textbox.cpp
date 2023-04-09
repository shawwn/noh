// (C)2005 S2 Games
// c_TextBox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_textbox.h"
#include "c_interface.h"
#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_uimanager.h"
#include "c_widgetstyle.h"
#include "c_resourcemanager.h"

#include "../k2/c_draw2d.h"
#include "../k2/c_fontmap.h"
//=============================================================================

/*====================
  CTextBox::CTextBox
  ====================*/
CTextBox::CTextBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IInputWidget(pInterface, pParent, WIDGET_TEXTBOX, style),
m_sFont(style.GetProperty(_T("font"))),
m_v4TextColor(GetColorFromString(style.GetProperty(_T("textcolor"), _T("silver")))),
m_fHPadding(0.0f),
m_fVPadding(0.0f),
m_uMaxLength(style.GetPropertyInt(_T("maxlength"), -1)),
m_bWrap(style.GetPropertyBool(_T("wrap"), false)),
m_sOnEnter(style.GetProperty(_T("onenter"))),
m_sOnEsc(style.GetProperty(_T("onesc"))),
m_zSelStart(-1),
m_hCursor(g_ResourceManager.Register(_T("/core/cursors/text.cursor"), RES_K2CURSOR)),
m_uiCursorPhase(0),
m_bMidClick(false),
m_bSelectOnFocus(style.GetPropertyBool(_T("selectonfocus"), false)),
m_bNewFocus(false),
m_uiDoubleClickTime(style.GetPropertyInt(_T("doubleclicktime"), 400)),
m_uiLastClickTime(0),
m_zLastClickPos(-1)
{
    SetFlags(WFLAG_INTERACTIVE);
    SetFlagsRecursive(WFLAG_PROCESS_CURSOR);

    m_hFontMap = g_ResourceManager.LookUpName(m_sFont, RES_FONTMAP);
    m_pFontMap = g_ResourceManager.GetFontMap(m_hFontMap);
    if (m_pFontMap == NULL)
    {
        Console.Warn << _T("CTextBox::CTextBox() - Couldn't retrieve font: ") << m_sFont << newl;
        return;
    }

    m_sHPadding = style.GetProperty(_T("hpadding"), _T("0"));
    m_sVPadding = style.GetProperty(_T("vpadding"), _T("0"));

    m_fHPadding = GetPositionFromString(m_sHPadding, GetWidth(), GetHeight());
    m_fVPadding = GetPositionFromString(m_sVPadding, GetHeight(), GetWidth());

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)

    //float fBottom = m_recArea.top + (m_pFontMap->GetMaxHeight() * m_iNumLines) + (m_fVPadding << 1);
    //m_recArea.Set(m_recArea.left, m_recArea.top, m_recArea.right, fBottom);
}


/*====================
  CTextBox::ButtonUp
  ====================*/
bool    CTextBox::ButtonUp(EButton button)
{
    DO_EVENT_PARAM_RETURN(WEVENT_KEYUP, XtoA(button), true);
    return true;
}


/*====================
  CTextBox::ButtonDown
  ====================*/
bool    CTextBox::ButtonDown(EButton button)
{
    float fWidth(0.0f);

    switch (button)
    {
    case BUTTON_BACKSPACE:
        if (m_zSelStart != size_t(-1) && m_zSelStart != m_zInputPos)
        {
            size_t zStart(MIN(m_zSelStart, m_zInputPos));
            size_t zEnd(MAX(m_zSelStart, m_zInputPos));

            m_sHiddenLine.erase(zStart, zEnd - zStart);
            m_sInputLine.erase(zStart, zEnd - zStart);

            m_zInputPos = zStart;

            if (!m_bWrap)
            {
                m_zEnd = m_zInputPos;
                
                // A m_zStart position > than an m_zEnd position will throw a C++ exception: std::out_of_range when trying to substr() it
                if (m_zStart > m_zEnd)
                    m_zStart = 0;               

                if (m_zStart > 0)
                {
                    while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance < m_recArea.GetWidth() && m_zStart > 0)
                        --m_zStart;

                    if (m_zStart > 0)
                        ++m_zStart;
                }

                for (size_t z(m_zStart); z < m_sInputLine.length(); ++z)
                {
                    if (fWidth >= GetWidth())
                    {
                        m_zEnd = z + 1;
                        break;
                    }

                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;
                }
            }

            m_zSelStart = -1;

            DO_EVENT_RETURN(WEVENT_CHANGE, true)

            m_uiCursorPhase = Host.GetSystemTime();

            break;
        }

        if (m_zInputPos == 0)
            break;
        --m_zInputPos;

        // fall through to BUTTON_DEL
    case BUTTON_DEL:
        if (m_zSelStart != size_t(-1) && m_zSelStart != m_zInputPos)
        {
            size_t zStart(MIN(m_zSelStart, m_zInputPos));
            size_t zEnd(MAX(m_zSelStart, m_zInputPos));

            m_sHiddenLine.erase(zStart, zEnd - zStart);
            m_sInputLine.erase(zStart, zEnd - zStart);

            m_zInputPos = zStart;

            if (!m_bWrap)
            {
                m_zStart = m_zInputPos;

                if (m_zEnd > m_sInputLine.length())
                    m_zEnd = m_sInputLine.length();

                for (size_t z(m_zStart); z < m_sInputLine.length(); ++z)
                {
                    if (fWidth >= GetWidth())
                    {
                        m_zEnd = z + 1;
                        break;
                    }

                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;
                }

                if (m_zStart > 0)
                {
                    while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance < m_recArea.GetWidth() && m_zStart > 0)
                        --m_zStart;
                }
            }

            m_zSelStart = -1;

            DO_EVENT_RETURN(WEVENT_CHANGE, true)

            m_uiCursorPhase = Host.GetSystemTime();

            break;
        }

        if (m_zInputPos < m_sInputLine.length())
        {
            m_sHiddenLine.erase(m_zInputPos, 1);
            m_sInputLine.erase(m_zInputPos, 1);

            if (m_zEnd > 0)
                --m_zEnd;

            if (m_zInputPos > m_zEnd)
                m_zInputPos = m_zEnd;

            if (m_zStart > 0)
            {
                while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd - m_zStart)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance < m_recArea.GetWidth() && m_zStart > 0)
                    --m_zStart;

                if (m_zStart > 0)
                    ++m_zStart;
            }

            DO_EVENT_RETURN(WEVENT_CHANGE, true)

            m_uiCursorPhase = Host.GetSystemTime();
        }

        break;

    case BUTTON_ESC:
        if (!m_sOnEsc.empty())
            UIScript.Evaluate(this, m_sOnEsc);
        m_zInputPos = 0;
        m_zStart = 0;
        m_zSelStart = -1;
        //m_zEnd = 0;
        m_pInterface->SetActiveWidget(NULL);
        break;

    case BUTTON_HOME:
        if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
            m_zSelStart = m_zInputPos;
        else if (!Input.IsShiftDown())
            m_zSelStart = -1;

        m_zInputPos = 0;
        m_zStart = 0;
        m_zEnd = m_sInputLine.length();

        if (!m_bWrap)
        {
            for (size_t z(0); z < m_sInputLine.length(); ++z)
            {
                fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                if (fWidth >= GetWidth())
                {
                    m_zEnd = z;
                    break;
                }
            }
        }
        break;

    case BUTTON_END:
        if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
            m_zSelStart = m_zInputPos;
        else if (!Input.IsShiftDown())
            m_zSelStart = -1;

        m_zInputPos = m_sInputLine.length();
        m_zStart = 0;
        m_zEnd = m_sInputLine.length();

        if (!m_bWrap)
        {
            for (size_t z(m_sInputLine.length() - 1); z >= 0 && z != size_t(-1); --z)
            {
                fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                if (fWidth >= GetWidth())
                {
                    m_zStart = z + 1;
                    break;
                }
            }
        }
        break;

    case BUTTON_LEFT:
        if (m_zInputPos > 0)
        {
            if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
                m_zSelStart = m_zInputPos;
            else if (!Input.IsShiftDown())
                m_zSelStart = -1;

            --m_zInputPos;

            if (!m_bWrap)
            {
                if (m_zInputPos < m_zStart)
                    m_zStart = m_zInputPos;

                for (size_t z(m_zStart); z < m_sInputLine.length(); ++z)
                {
                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                    if (fWidth >= GetWidth())
                    {
                        m_zEnd = z;
                        break;
                    }
                }
            }

            m_uiCursorPhase = Host.GetSystemTime();
        }
        else
        {
            if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
                m_zSelStart = m_zInputPos;
            else if (!Input.IsShiftDown())
                m_zSelStart = -1;
        }
        break;

    case BUTTON_RIGHT:
        if (m_zInputPos < m_sInputLine.length())
        {
            if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
                m_zSelStart = m_zInputPos;
            else if (!Input.IsShiftDown())
                m_zSelStart = -1;

            ++m_zInputPos;

            if (m_zInputPos > m_zEnd)
                m_zEnd = m_zInputPos;

            if (!m_bWrap)
            {
                for (size_t z(m_zInputPos - 1); z >= m_zStart && z != size_t(-1); z--)
                {
                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                    if (fWidth >= GetWidth())
                    {
                        m_zStart = z + 1;
                        break;
                    }
                }
            }

            m_uiCursorPhase = Host.GetSystemTime();
        }
        else
        {
            if (Input.IsShiftDown() && m_zSelStart == size_t(-1))
                m_zSelStart = m_zInputPos;
            else if (!Input.IsShiftDown())
                m_zSelStart = -1;
        }
        break;

    case BUTTON_ENTER:
        if (!m_sOnEnter.empty())
            UIScript.Evaluate(this, m_sOnEnter);
        break;

    case BUTTON_TAB:
        // Don't eat BUTTON_TAB to allow tab order to change widget focus
        return false;

    default:
        break;
    }

    DO_EVENT_PARAM_RETURN(WEVENT_KEYDOWN, XtoA(button), true);
    return true;
}


/*====================
  CTextBox::ProcessInputCursor
  ====================*/
bool    CTextBox::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    if (m_bMidClick)
    {
        float fLastWidth(GetX() + m_fHPadding);

        size_t z(m_zStart);

        for (; z < m_zEnd; ++z)
        {
            float fNewWidth(fLastWidth + m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance);

            if (fNewWidth > v2CursorPos.x)
            {
                m_zInputPos = z;
                break;
            }

            fLastWidth = fNewWidth;
        }

        if (z == m_zEnd)
            m_zInputPos = m_zEnd;
    }

    return false;
}


/*====================
  CTextBox::MouseDown
  ====================*/
void    CTextBox::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    float fLastWidth(GetX() + m_fHPadding);

    switch (button)
    {
    case BUTTON_MOUSEL:
        {
            size_t z(m_zStart);
            size_t zOldInputPos(m_zInputPos);

            for (; z < m_zEnd; ++z)
            {
                float fNewWidth(fLastWidth + m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance);

                if (fNewWidth > v2CursorPos.x)
                {
                    m_zInputPos = z;
                    break;
                }

                fLastWidth = fNewWidth;
            }

            if (z == m_zEnd)
                m_zInputPos = m_zEnd;

            if (Input.IsShiftDown() && (m_zSelStart == size_t(-1) || m_zSelStart == zOldInputPos))
                m_zSelStart = zOldInputPos;
            else if (!Input.IsShiftDown())
                m_zSelStart = m_zInputPos;

            m_uiCursorPhase = Host.GetSystemTime();

            m_bMidClick = true;

            m_pInterface->SetExclusiveWidget(this);

            if (Host.GetSystemTime() - m_uiLastClickTime < m_uiDoubleClickTime &&
                m_zLastClickPos == m_zInputPos)
            {
                m_bMidClick = false;

                m_zSelStart = 0;
                m_zInputPos = m_sInputLine.length();

                m_uiLastClickTime = 0;
                m_zLastClickPos = size_t(-1);
            }
            else
            {
                m_uiLastClickTime = Host.GetSystemTime();
                m_zLastClickPos = m_zInputPos;
            }

            break;
        }

    default:
        break;
    }
}


/*====================
  CTextBox::MouseUp
  ====================*/
void    CTextBox::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    switch (button)
    {
    case BUTTON_MOUSEL:
        {
            m_bMidClick = false;

            m_pInterface->SetExclusiveWidget(NULL);

            if (m_bNewFocus && m_bSelectOnFocus)
            {
                m_bNewFocus = false;

                if (m_zInputPos == m_zSelStart || m_zSelStart == size_t(-1))
                {
                    m_zSelStart = 0;
                    m_zInputPos = m_sInputLine.length();
                }
            }

            break;
        }

    default:
        break;
    }
}


/*====================
  CTextBox::Rollover
  ====================*/
void    CTextBox::Rollover()
{
    Input.SetCursor(CURSOR_UI, m_hCursor);
}


/*====================
  CTextBox::Rolloff
  ====================*/
void    CTextBox::Rolloff()
{
    Input.SetCursor(CURSOR_UI, INVALID_RESOURCE);
}


/*====================
  CTextBox::Focus
  ====================*/
void    CTextBox::Focus()
{
    if (m_bSelectOnFocus)
    {
        m_bNewFocus = true;

        m_zSelStart = 0;
        m_zInputPos = m_sInputLine.length();
    }

    m_uiCursorPhase = Host.GetSystemTime();

    IWidget::Focus();
}


/*====================
  CTextBox::LoseFocus
  ====================*/
void    CTextBox::LoseFocus()
{
    m_zSelStart = -1;

    IWidget::LoseFocus();
}


/*====================
  CTextBox::PasteString
  ====================*/
void    CTextBox::PasteString(const tstring &sString)
{
    for (size_t z(0); z < sString.length(); z++)
        Char(sString[z]);
}


/*====================
  CTextBox::GetCopyString
  ====================*/
tstring CTextBox::GetCopyString()
{
    if (m_zSelStart == size_t(-1) || m_zSelStart == m_zInputPos)
        return _T("");

    size_t zStart(MIN(m_zSelStart, m_zInputPos));
    size_t zEnd(MAX(m_zSelStart, m_zInputPos));

    return m_sHiddenLine.substr(zStart, zEnd - zStart);
}


/*====================
  CTextBox::Char
  ====================*/
bool    CTextBox::Char(TCHAR c)
{
    if (unsigned(c) < _T(' '))
        return true;

    if (IsInteractive())
    {
        if (m_zSelStart != size_t(-1) && m_zSelStart != m_zInputPos)
        {

            if (m_zSelStart == size_t(0) && m_zInputPos == m_sInputLine.length())
            {
                Clear();
            }
            else
            {
                size_t zStart(MIN(m_zSelStart, m_zInputPos));
                size_t zEnd(MAX(m_zSelStart, m_zInputPos));

                m_sHiddenLine.erase(zStart, zEnd - zStart);
                m_sInputLine.erase(zStart, zEnd - zStart);

                m_zInputPos = zStart;

                if (!m_bWrap)
                {
                    m_zEnd = m_zInputPos;

                    if (m_zStart > 0)
                    {
                        while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance < m_recArea.GetWidth() && m_zStart > 0)
                            --m_zStart;

                        if (m_zStart > 0)
                            ++m_zStart;
                    }

                    float fWidth(0.0f);

                    for (size_t z(m_zStart); z < m_sInputLine.length(); ++z)
                    {
                        if (fWidth >= GetWidth())
                        {
                            m_zEnd = z + 1;
                            break;
                        }

                        fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;
                    }
                }
            }

            m_zSelStart = size_t(-1);
        }

        m_zSelStart = size_t(-1);

        if (m_sInputLine.length() < m_uMaxLength)
        {
            if (m_zInputPos < m_sInputLine.length())
                InsertIntoInput(c);
            else
                AppendToInput(c);

            m_uiCursorPhase = Host.GetSystemTime();
        }

        float fWidth(0.0f);

        if (!m_bWrap)
        {
            if (m_zInputPos >= m_zEnd - 1)
            {
                m_zEnd = MIN(m_zInputPos + 1, m_sInputLine.length());

                for (size_t z(m_zEnd - 1); z >= m_zStart && z != size_t(-1); z--)
                {
                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                    if (fWidth >= GetWidth())
                    {
                        m_zStart = z + 1;
                        break;
                    }
                }
            }
            else
            {
                for (size_t z(m_zStart); z < m_sInputLine.length(); ++z)
                {
                    fWidth += m_pFontMap->GetCharMapInfo(m_sHiddenLine[z])->m_fAdvance;

                    if (fWidth >= GetWidth())
                    {
                        m_zEnd = z;
                        break;
                    }
                }
            }
        }
    }
    return true;
}


/*====================
  CTextBox::Clear
  ====================*/
void    CTextBox::Clear()
{
    if (m_sInputLine.size() <= 0)
        return;

    m_zInputPos = 0;
    m_zStart = 0;
    m_zEnd = 0;
    m_zSelStart = -1;
    m_sInputLine.clear();
    m_sHiddenLine.clear();

    DO_EVENT(WEVENT_CHANGE)
}


/*====================
  CTextBox::RenderWidget
  ====================*/
void    CTextBox::RenderWidget(const CVec2f &v2Origin, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    IInputWidget::RenderWidget(v2Origin, fFade);

    float fX(v2Origin.x + m_fHPadding);
    float fY(v2Origin.y + m_fVPadding);

    float fCursorWidth(1.0f);
    float fCursorHeight(ROUND((m_pFontMap ? m_pFontMap->GetMaxHeight() : 0.0f) * 0.95f));
    float fCursorX(m_pFontMap ? m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zInputPos - m_zStart)) : 0.0f);
    float fCursorY(0.0f);

    CRectf recArea(m_recArea);
    recArea.MoveTo(fX, fY);

    size_t zSelStart(-1);
    size_t zSelEnd(-1);

    if (m_zSelStart != size_t(-1) && m_zSelStart != m_zInputPos)
    {
        if (m_zSelStart >= m_zStart)
        {
            size_t zLeft(m_zSelStart - m_zStart);
            size_t zRight(m_zInputPos - m_zStart);

            zSelStart = MIN(zLeft, zRight);
            zSelEnd = MAX(zLeft, zRight);
        }
        else
        {
            zSelStart = 0;
            zSelEnd = m_zInputPos - m_zStart;
        }
    }

    CVec4f v4FontColor(GetFadedColor(m_v4TextColor, fFade));

    Draw2D.SetColor(v4FontColor);

    Draw2D.SetBGColor(CVec4f(v4FontColor.x, v4FontColor.y, v4FontColor.z, v4FontColor.w * 0.5f));
    Draw2D.SetFGColor(v4FontColor);

    Draw2D.String(recArea, m_sInputLine.substr(m_zStart, m_zEnd - m_zStart), m_hFontMap, (m_bWrap ? DRAW_STRING_WRAP : 0), zSelStart, zSelEnd);

    // render insertion point
    if (m_pInterface->GetActiveWidget() == this)
    {
        // Scroll the cursor vertically
        while (fCursorX > recArea.GetWidth())
        {
            fCursorX -= recArea.GetWidth();
            fCursorY += ((m_pFontMap->GetMaxHeight() / 3) * 2);
        }

        if (!(int(((Host.GetSystemTime() - m_uiCursorPhase) / 1000.0f) * 2.0f) & 0x01))
        {
            Draw2D.SetColor(GetFadedColor(m_v4TextColor, fFade));
            Draw2D.Rect(fX + fCursorX, fY + fCursorY + ((m_pFontMap ? m_pFontMap->GetMaxHeight() : 0.0f) - fCursorHeight), fCursorWidth, fCursorHeight);
        }
    }
}


/*====================
  CTextBox::SetInputLine
  ====================*/
void    CTextBox::SetInputLine(const tstring &sInputLine)
{
    m_sHiddenLine = sInputLine;

    if (!m_sPasswordChar.empty())
    {
        m_sInputLine.clear();

        for (uint uLoop = 0; uLoop < sInputLine.length(); uLoop++)
            m_sInputLine += m_sPasswordChar[0];
    }
    else
        m_sInputLine = sInputLine;

    m_zEnd = m_sInputLine.length();
    m_zInputPos = m_zEnd;

    m_zStart = 0;
    m_zSelStart = -1;

    if (!m_bWrap && m_pFontMap != NULL)
    {
        while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd - m_zStart)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance > m_recArea.GetWidth()) {
            if (m_zStart >= m_zEnd)
                break;
            m_zStart++;
        }
    }

    DO_EVENT(WEVENT_CHANGE)
}


/*====================
  CTextBox::SetInputPos
  ====================*/
void    CTextBox::SetInputPos(size_t zInputPos)
{
    if (zInputPos < m_sInputLine.length())
    {
        m_zInputPos = zInputPos;

        if (zInputPos < m_zStart)
            m_zStart = zInputPos;

        if (zInputPos > m_zEnd)
            m_zEnd = m_zInputPos;
    }
    else
    {
        m_zInputPos = m_sInputLine.length();
        m_zEnd = m_zInputPos;
        m_zStart = 0;
    }

    if (!m_bWrap && m_pFontMap != NULL)
    {
        while (m_pFontMap->GetStringWidth(m_sInputLine.substr(m_zStart, m_zEnd - m_zStart)) + m_pFontMap->GetCharMapInfo(_T(' '))->m_fAdvance > m_recArea.GetWidth()) {
            if (m_zStart >= m_zEnd)
                break;
            m_zStart++;
        }
    }

    m_zSelStart = -1;
}


/*====================
  CTextBox::RecalculateChildSize
  ====================*/
void    CTextBox::RecalculateChildSize()
{
    m_fHPadding = GetPositionFromString(m_sHPadding, GetWidth(), GetHeight());
    m_fVPadding = GetPositionFromString(m_sVPadding, GetHeight(), GetWidth());

    IInputWidget::RecalculateChildSize();
}


/*--------------------
  SetInputLine
  --------------------*/
UI_VOID_CMD(SetInputLine, 1)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBOX)
        return;

    tstring sLine(vArgList[0]->Evaluate());
    static_cast<CTextBox*>(pThis)->SetInputLine(sLine);

    // Focus on the end of the set line
    static_cast<CTextBox*>(pThis)->SetInputPos(sLine.length());
}


/*--------------------
  EraseInputLine
  --------------------*/
UI_VOID_CMD(EraseInputLine, 0)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBOX)
        return;

    static_cast<CTextBox*>(pThis)->SetInputLine(TSNULL);
}


/*--------------------
  SetTextColor
  --------------------*/
UI_VOID_CMD(SetTextColor, 1)
{
    if (!pThis || pThis->GetType() != WIDGET_TEXTBOX)
        return;

    static_cast<CTextBox*>(pThis)->SetTextColor(GetColorFromString(vArgList[0]->Evaluate()));
}
