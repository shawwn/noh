// (C)2006 S2 Games
// c_xmlproc_style.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_interface.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
//=============================================================================

// <style>
DECLARE_XML_PROCESSOR(style)
BEGIN_XML_REGISTRATION(style)
    REGISTER_XML_PROCESSOR(interface)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(style, IWidget)
    PROFILE("CXMLProcessorStyle::Process");
    try
    {
        CInterface *pInterface(pObject->GetInterface());
        if (pInterface == nullptr)
            EX_ERROR(_T("Interface pointer not set for <style> tag"));

        if (!node.HasProperty(_T("name")))
            EX_ERROR(_T("<style> tag has no name property"));

        tstring sName(node.GetProperty(_T("name")));
        CWidgetStyle* pStyle(K2_NEW(ctx_Widgets,  CWidgetStyle)(pInterface, node));
        pStyle->RemoveProperty(_T("name"));
        pInterface->RegisterStyle(sName, pStyle);
        return true;
    }
    catch (CException& ex)
    {
        ex.Process(_T("CXMLProc_Style::Process() - "), NO_THROW);
        return false;
    }
END_XML_PROCESSOR_NO_CHILDREN
