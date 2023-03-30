// (C)2005 S2 Games
// c_panel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_panel.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
//=============================================================================

/*====================
  CPanel::CPanel
  ====================*/
CPanel::CPanel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style, EWidgetType eWidgetType) :
IDragWidget(pInterface, pParent, eWidgetType, style)
{
	if (!style.HasProperty(_T("texture")) && !style.HasProperty(_T("color")))
		SetFlags(WFLAG_NO_DRAW);

	if (HasFlags(WFLAG_CAN_GRAB))
		SetFlags(WFLAG_INTERACTIVE);

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)
}


/*====================
  CPanel::MouseDown
  ====================*/
void	CPanel::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELDOWN)
		DO_EVENT(WEVENT_CLICK)
		
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERDOWN)
		DO_EVENT(WEVENT_RIGHTCLICK)
	}
}


/*====================
  CPanel::MouseUp
  ====================*/
void	CPanel::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELUP)
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERUP)
	}
}

