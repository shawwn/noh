// (C)2005 S2 Games
// i_xmlproc_widget.h
//
//=============================================================================
#ifndef __I_XMLPROC_WIDGET_H__
#define __I_XMLPROC_WIDGET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_xmlprocessor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// SETUP_XML_PROCESSOR_WIDGET
#define SETUP_XML_PROCESSOR_WIDGET(name, widget_type) \
BEGIN_XML_PROCESSOR(name, IWidget) \
	CInterface *pInterface(pObject->GetInterface()); \
	if (pInterface == NULL) \
	{ \
		Console.Err << _T("Invalid interface for widget: ") << m_sElementName << newl; \
		return false; \
	} \
\
	CWidgetTemplate *pTemplate(pInterface->GetCurrentTemplate()); \
	if (pTemplate != NULL) \
	{ \
		pTemplate->AddChild(m_sElementName, node); \
		ProcessChildren(node, pObject); \
		pTemplate->EndChild(); \
		return true; \
	}

// XML_PROCESSOR_CREATE_WIDGET
#define XML_PROCESSOR_CREATE_WIDGET(widget_type, ...) \
	CWidgetStyle style(pInterface, node); \
	widget_type *pNewWidget(K2_NEW(ctx_Widgets,  widget_type)(pInterface, pObject, style, ##__VA_ARGS__)); \
	if (pNewWidget == NULL) \
	{ \
		Console.Err << _T("Failed creating widget: ") << m_sElementName << newl; \
		return false; \
	}

// BEGIN_XML_PROCESSOR_WIDGET
#define BEGIN_XML_PROCESSOR_WIDGET(name, widget_type, ...) \
SETUP_XML_PROCESSOR_WIDGET(name, widget_type) \
XML_PROCESSOR_CREATE_WIDGET(widget_type, ##__VA_ARGS__)

// END_XML_PROCESSOR_WIDGET
#define END_XML_PROCESSOR_WIDGET \
ProcessChildren(node, pNewWidget); \
pObject->AddChild(pNewWidget); \
pInterface->ClearAnchor(); \
END_XML_PROCESSOR_NO_CHILDREN
//=============================================================================

#endif //__I_XMLPROC_WIDGET_H__
