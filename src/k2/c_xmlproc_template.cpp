// (C)2007 S2 Games
// c_xmlproc_template.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_template.h"
#include "c_xmlproc_interface.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
//=============================================================================

// <template>
BEGIN_XML_REGISTRATION(template)
    REGISTER_XML_PROCESSOR(interface)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(template, IWidget)
    CInterface *pInterface(pObject->GetInterface());
    if (pInterface == nullptr)
    {
        Console.Err << _T("Invalid interface for <template>") << newl;
        return false;
    }

    if (!node.HasProperty(_T("name")))
    {
        Console.Err << _T("<template> tag has no name property") << newl;
        return false;
    }

    CWidgetTemplate* pTemplate(K2_NEW(ctx_Widgets,  CWidgetTemplate)(pInterface, node));
    pInterface->RegisterTemplate(pTemplate);

    ProcessChildren(node, pObject);
    pInterface->ClearCurrentTemplate();
END_XML_PROCESSOR_NO_CHILDREN
