// (C)2006 S2 Games
// c_combobox_listbox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_combobox_listbox.h"
#include "c_combobox.h"
#include "c_listitem.h"
//=============================================================================

/*====================
  CComboBoxListBox::CComboBoxListBox
  ====================*/
CComboBoxListBox::CComboBoxListBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style) :
CListBox(pInterface, pParent, style),
m_bMidClick(false)
{
}


/*====================
  CComboBoxListBox::MouseDown
  ====================*/
void    CComboBoxListBox::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    CListBox::MouseDown(button, v2CursorPos);

    if (button == BUTTON_MOUSEL && m_recArea.AltContains(v2CursorPos))
        m_bMidClick = true;
}


/*====================
  CComboBoxListBox::MouseUp
  ====================*/
void    CComboBoxListBox::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    CListBox::MouseUp(button, v2CursorPos);

    if (button == BUTTON_MOUSEL && m_bMidClick && m_recArea.AltContains(v2CursorPos))
        DO_EVENT(WEVENT_SELECT)
}


/*====================
  CComboBoxListBox::DoEvent
  ====================*/
void    CComboBoxListBox::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    IWidget::DoEvent(eEvent, sParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;

    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    assert(m_pParent->GetType() == WIDGET_COMBOBOX);

    if (eEvent == WEVENT_SELECT)
    {
        if (m_iSelectedListItem == -1)
            static_cast<CComboBox *>(m_pParent)->SetActiveListItem(nullptr, true);
        else
            static_cast<CComboBox *>(m_pParent)->SetActiveListItem(m_vItems[m_iSelectedListItem], true);
    }
}
