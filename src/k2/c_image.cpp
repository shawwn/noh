// (C)2005 S2 Games
// c_image.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_image.h"

#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CImage::CImage
  ====================*/
CImage::CImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_IMAGE, style)
{
	// Color
	if (!style.HasProperty(_T("color")))
		SetColor(WHITE);

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)
}


/*====================
  CImage::MouseDown
  ====================*/
void	CImage::MouseDown(EButton button, const CVec2f &v2CursorPos)
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
  CImage::MouseUp
  ====================*/
void	CImage::MouseUp(EButton button, const CVec2f &v2CursorPos)
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
