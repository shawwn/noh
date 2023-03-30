// (C)2007 S2 Games
// c_xmlproc_instance.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_frame.h"
#include "c_xmlproc_button.h"
#include "c_xmlproc_listitem.h"
#include "c_xmlproc_listbox.h"
#include "c_xmlproc_combobox.h"
#include "c_xmlproc_menu.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_widgetstate.h"
#include "c_xmlproc_floater.h"
#include "c_interface.h"
#include "c_widgettemplate.h"
#include "c_xmlnode.h"
//=============================================================================

// <instance>
DECLARE_XML_PROCESSOR(instance)
BEGIN_XML_REGISTRATION(instance)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(listitem)
    REGISTER_XML_PROCESSOR(frame)
    REGISTER_XML_PROCESSOR(button)
    REGISTER_XML_PROCESSOR(widgetstate)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
    REGISTER_XML_PROCESSOR(floater)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(instance, IWidget)
    PROFILE("CXMLProc_Instance::Process");

    try
    {
        CInterface *pInterface(pObject->GetInterface());
        if (pInterface == NULL)
            EX_ERROR(_T("Interface pointer not set for <instance> tag"));

        CWidgetTemplate *pTemplate(pInterface->GetCurrentTemplate());

        if (pTemplate == NULL)
        {
            if (!node.HasProperty(_T("name")))
            {
                // Doing this on purpose now sometimes
                //EX_ERROR(_T("<instance> tag has no name property"));
                return true;
            }

            const tstring &sName(node.GetProperty(_T("name")));
            CWidgetTemplate* pTemplate(pInterface->GetTemplate(sName));
            if (pTemplate == NULL)
                EX_ERROR(_T("No template exists for: ") + sName);

            CWidgetStyle style(pInterface, node);
            pTemplate->Instantiate(pObject, style);
        }
        else
        {
            pTemplate->AddChild(_T("instance"), node);
            pTemplate->EndChild();
        }

        return true;
    }
    catch (CException& ex)
    {
        ex.Process(_T("CXMLProc_Instance::Process() - "), NO_THROW);
        return false;
    }
END_XML_PROCESSOR_NO_CHILDREN
