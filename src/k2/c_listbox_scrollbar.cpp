// (C)2005 S2 Games
// c_listbox_scrollbar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_listbox_scrollbar.h"
#include "c_listbox.h"
//=============================================================================

/*====================
  CListBoxScrollbar::~CListBoxScrollbar
  ====================*/
CListBoxScrollbar::~CListBoxScrollbar()
{
}


/*====================
  CListBoxScrollbar::CListBoxScrollbar
  ====================*/
CListBoxScrollbar::CListBoxScrollbar(CInterface *pInterface, IWidget *pParent, EListBoxScrollbar eType, const CWidgetStyle& style) :
CScrollbar(pInterface, pParent, style),
m_eType(eType)
{
}


/*====================
  CListBoxScrollbar::DoChange
  ====================*/
void    CListBoxScrollbar::DoChange()
{
    if (m_eType == LISTBOX_SCROLLBAR_HORIZONTAL)
        static_cast<CListBox *>(m_pParent)->HorizontalScrollbarChange(GetValueFloat());
    else if (m_eType == LISTBOX_SCROLLBAR_VERTICAL)
        static_cast<CListBox *>(m_pParent)->VerticalScrollbarChange(GetValueFloat());

    CScrollbar::DoChange();
}
