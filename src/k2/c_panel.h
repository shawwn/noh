// (C)2005 S2 Games
// c_panel.h
//
//=============================================================================
#ifndef __C_PANEL_H__
#define __C_PANEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_dragwidget.h"
//=============================================================================

//=============================================================================
// CPanel
//=============================================================================
class CPanel : public IDragWidget
{
protected:

public:
	virtual ~CPanel()	{}
	CPanel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style, EWidgetType eWidgetType = WIDGET_PANEL);

	virtual void		MouseUp(EButton button, const CVec2f &v2CursorPos);
	virtual void		MouseDown(EButton button, const CVec2f &v2CursorPos);
};
//=============================================================================

#endif // __C_PANEL_H__
