// (C)2005 S2 Games
// c_textbuffer_scrollbar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_textbuffer_scrollbar.h"
#include "c_textbuffer.h"
//=============================================================================

/*====================
  CTextBufferScrollbar::CTextBufferScrollbar
  ====================*/
CTextBufferScrollbar::CTextBufferScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
CScrollbar(pInterface, pParent, style)
{
}


/*====================
  CTextBufferScrollbar::DoChange
  ====================*/
void	CTextBufferScrollbar::DoChange()
{
	static_cast<CTextBuffer *>(m_pParent)->VerticalScrollbarChange(GetValueFloat());

	CScrollbar::DoChange();
}
