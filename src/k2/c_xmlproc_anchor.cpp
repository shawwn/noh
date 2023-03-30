// (C)2007 S2 Games
// c_xmlproc_anchor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_frame.h"
#include "c_xmlproc_widgetstate.h"
#include "c_xmlproc_floater.h"
#include "c_interface.h"
#include "c_widgettemplate.h"
//=============================================================================

/*====================
  xmlproc_anchor
  ====================*/
void	xmlproc_anchor(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style)
{
	// Alignment
	EAlignment eAlignment(ALIGN_LEFT);
	EAlignment eVAlignment(ALIGN_TOP);

	if (LowerString(style.GetProperty(_T("align"))) == _T("center"))
		eAlignment = ALIGN_CENTER;
	else if (LowerString(style.GetProperty(_T("align"))) == _T("right"))
		eAlignment = ALIGN_RIGHT;

	if (LowerString(style.GetProperty(_T("valign"))) == _T("center"))
		eVAlignment = ALIGN_CENTER;
	else if (LowerString(style.GetProperty(_T("valign"))) == _T("bottom"))
		eVAlignment = ALIGN_BOTTOM;

	// Size
	float fWidth(0.0f);
	float fHeight(0.0f);

	fWidth = ROUND(IWidget::GetSizeFromString(style.GetProperty(_T("width"), _T("100%")), pParent->GetWidth(), pParent->GetHeight()));
	fHeight = ROUND(IWidget::GetSizeFromString(style.GetProperty(_T("height"), _T("100%")), pParent->GetHeight(), pParent->GetWidth()));

	// Position
	float fX(0.0f);
	float fY(0.0f);

	float fBaseX(0.0f);
	float fBaseY(0.0f);

	if (eAlignment == ALIGN_CENTER)
		fBaseX = (pParent->GetWidth() / 2.0f) - (fWidth / 2.0f);
	else if (eAlignment == ALIGN_RIGHT)
		fBaseX = pParent->GetWidth() - fWidth;

	if (eVAlignment == ALIGN_CENTER)
		fBaseY = (pParent->GetHeight() / 2.0f) - (fHeight / 2.0f);
	else if (eVAlignment == ALIGN_BOTTOM)
		fBaseY = pParent->GetHeight() - fHeight;

	fX = ROUND(fBaseX + IWidget::GetPositionFromString(style.GetProperty(_T("x")), pParent->GetWidth(), pParent->GetHeight()));
	fY = ROUND(fBaseY + IWidget::GetPositionFromString(style.GetProperty(_T("y")), pParent->GetHeight(), pParent->GetWidth()));

	pInterface->SetAnchor(CVec2f(fX, fY));
}

// <anchor>
DECLARE_XML_PROCESSOR(anchor)
BEGIN_XML_REGISTRATION(anchor)
	REGISTER_XML_PROCESSOR(interface)
	REGISTER_XML_PROCESSOR(template)
	REGISTER_XML_PROCESSOR(panel)
	REGISTER_XML_PROCESSOR(webpanel)
	REGISTER_XML_PROCESSOR(frame)
	REGISTER_XML_PROCESSOR(widgetstate)
	REGISTER_XML_PROCESSOR(floater)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(anchor, IWidget)
	try
	{
		CInterface *pInterface(pObject->GetInterface());
		if (pInterface == NULL)
			EX_ERROR(_T("Interface pointer not set for <anchor> tag"));

		CWidgetTemplate *pTemplate(pInterface->GetCurrentTemplate());
		if (pTemplate)
		{
			pTemplate->AddChild(_T("anchor"), node);
			pTemplate->EndChild();
			return true;
		}

		xmlproc_anchor(pInterface, pObject, CWidgetStyle(pInterface, node));
		return true;
	}
	catch (CException& ex)
	{
		ex.Process(_T(""), NO_THROW);
		return false;
	}
END_XML_PROCESSOR(NULL)
