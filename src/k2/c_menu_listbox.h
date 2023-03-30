// (C)2009 S2 Games
// c_menu_listbox.h
//
//=============================================================================
#ifndef __C_MENU_LISTBOX_H__
#define __C_MENU_LISTBOX_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_listbox.h"
//=============================================================================

//=============================================================================
// CMenuListBox
//=============================================================================
class CMenuListBox : public CListBox
{
protected:
	bool	m_bMidClick;

public:
	~CMenuListBox()	{}
	CMenuListBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);

	void	MouseDown(EButton button, const CVec2f &v2CursorPos);
	void	MouseUp(EButton button, const CVec2f &v2CursorPos);

	void	SetMidClick(bool bMidClick)		{ m_bMidClick = bMidClick; }

	void	DoEvent(EWidgetEvent eEvent, const tstring &sParam = TSNULL);
};
//=============================================================================

#endif // __C_MENU_LISTBOX_H__
