// (C)2005 S2 Games
// c_listbox_scrollbar.h
//
//=============================================================================
#ifndef __C_LISTBOX_SCROLLBAR_H__
#define __C_LISTBOX_SCROLLBAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_scrollbar.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EListBoxScrollbar
{
	LISTBOX_SCROLLBAR_HORIZONTAL = 0,
	LISTBOX_SCROLLBAR_VERTICAL
};
//=============================================================================

//=============================================================================
// CScrollbar
//=============================================================================
class CListBoxScrollbar : public CScrollbar
{
protected:
	EListBoxScrollbar	m_eType;

	void	DoChange();

public:
	~CListBoxScrollbar();
	CListBoxScrollbar(CInterface *pInterface, IWidget *pParent, EListBoxScrollbar eType, const CWidgetStyle& style);
};
//=============================================================================

#endif //__C_LISTBOX_SCROLLBAR_H__
