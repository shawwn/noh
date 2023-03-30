// (C)2005 S2 Games
// c_scrollbar_button.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_scrollbar_button.h"
#include "c_scrollbar.h"
#include "c_widgetstyle.h"
//=============================================================================

/*====================
  CScrollbarButton::CScrollbarButton
  ====================*/
CScrollbarButton::CScrollbarButton(CInterface *pInterface, IWidget *pParent, EScrollbarButton eType, const CWidgetStyle& style) :
CButton(pInterface, pParent, style),
m_eType(eType),
m_uiNextRepeat(INVALID_TIME),
m_uiRepeatInterval(style.GetPropertyInt(_T("scrollinterval"), 50))
{
}


/*====================
  CScrollbarButton::Frame
  ====================*/
void    CScrollbarButton::Frame(uint uiFrameLength, bool bProcessFrame)
{
    CButton::Frame(uiFrameLength, bProcessFrame);

    if (m_bPressed && m_uiRepeatInterval > 0)
    {
        while (m_uiNextRepeat < Host.GetTime())
        {
            m_uiNextRepeat += m_uiRepeatInterval;

            if (m_eType == SCROLLBAR_BUTTON_MIN)
                static_cast<CScrollbar *>(m_pParent)->MinButtonCommand();
            else if (m_eType == SCROLLBAR_BUTTON_MAX)
                static_cast<CScrollbar *>(m_pParent)->MaxButtonCommand();

            DO_EVENT(WEVENT_CLICK)
        }
    }
}


/*====================
  CScrollbarButton::MouseDown
  ====================*/
void    CScrollbarButton::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button != BUTTON_MOUSEL)
        return;

    CButton::MouseDown(button, v2CursorPos);

    assert(m_pParent->GetType() == WIDGET_SCROLLBAR);

    if (m_eType == SCROLLBAR_BUTTON_MIN)
        static_cast<CScrollbar *>(m_pParent)->MinButtonCommand();
    else if (m_eType == SCROLLBAR_BUTTON_MAX)
        static_cast<CScrollbar *>(m_pParent)->MaxButtonCommand();

    DO_EVENT(WEVENT_CLICK)

    if (m_uiRepeatInterval > 0)
        m_uiNextRepeat = Host.GetTime() + m_uiRepeatInterval;
}
