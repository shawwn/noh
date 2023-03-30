// (C)2007 S2 Games
// c_xmlproc_widgetstate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_widgetstate.h"
#include "c_xmlproc_button.h"
#include "c_xmlproc_listbox.h"
#include "c_xmlproc_combobox.h"
#include "c_xmlproc_menu.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_widgetstate.h"
#include "c_xmlproc_floater.h"
#include "c_widgetstyle.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
#include "c_uimanager.h"

#include "../k2/c_xmlnode.h"
#include "../k2/c_filemanager.h"
//=============================================================================

// <widgetstate>
BEGIN_XML_REGISTRATION(widgetstate)
    REGISTER_XML_PROCESSOR(button)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(floater)
END_XML_REGISTRATION
SETUP_XML_PROCESSOR_WIDGET(widgetstate, CWidgetState)
    CWidgetStyle style(pInterface, node);
    CWidgetState *pNewWidget(pObject->AllocateWidgetState(style));
    if (pNewWidget == NULL)
    {
        Console.Err << _T("Failed creating widget: ") << m_sElementName << newl;
        return false;
    }

    ProcessChildren(node, pNewWidget);
    pObject->AddWidgetState(pNewWidget);
    pInterface->ClearAnchor();
END_XML_PROCESSOR_NO_CHILDREN
