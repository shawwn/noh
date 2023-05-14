// (C)2009 S2 Games
// c_menu_listbox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_menu_listbox.h"
#include "c_menu.h"
#include "c_listitem.h"
//=============================================================================

/*====================
  CMenuListBox::CMenuListBox
  ====================*/
CMenuListBox::CMenuListBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style) :
CListBox(pInterface, pParent, style),
m_bMidClick(false)
{
}


/*====================
  CMenuListBox::MouseDown
  ====================*/
void    CMenuListBox::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    CListBox::MouseDown(button, v2CursorPos);

    if (v2CursorPos.x - m_recArea.left < m_fListItemOffsetX || v2CursorPos.y - m_recArea.top < m_fListItemOffsetY)
        return;

    if ((button == BUTTON_MOUSEL || button == BUTTON_MOUSER) && m_recArea.AltContains(v2CursorPos))
        m_bMidClick = true;
}


/*====================
  CMenuListBox::MouseUp
  ====================*/
void    CMenuListBox::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    CListBox::MouseUp(button, v2CursorPos);

    if ((button == BUTTON_MOUSEL || button == BUTTON_MOUSER) && m_bMidClick && GetSelectedListItem() != nullptr)
        DO_EVENT(WEVENT_SELECT)
}


/*====================
  CMenuListBox::DoEvent
  ====================*/
void    CMenuListBox::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    IWidget::DoEvent(eEvent, sParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;

    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    assert(m_pParent->GetType() == WIDGET_MENU);

    if (eEvent == WEVENT_SELECT)
    {
        if (m_iSelectedListItem == -1)
            static_cast<CMenu *>(m_pParent)->SelectItem(nullptr, true);
        else
            static_cast<CMenu *>(m_pParent)->SelectItem(m_vItems[m_iSelectedListItem], true);
    }
}
