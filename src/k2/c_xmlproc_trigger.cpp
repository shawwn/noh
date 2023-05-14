// (C)2007 S2 Games
// c_xmlproc_trigger.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_interface.h"
#include "c_interface.h"
#include "c_uitrigger.h"
//=============================================================================

namespace XMLInterface
{
    // <trigger>
    DECLARE_XML_PROCESSOR(trigger)
    BEGIN_XML_REGISTRATION(trigger)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(trigger, IWidget)
        PROFILE("CXMLProc_Trigger::Process");

        try
        {
            CInterface *pInterface(pObject->GetInterface());
            if (pInterface == nullptr)
                EX_ERROR(_T("Interface pointer not set for <trigger> tag"));

            if (!node.HasProperty(_T("name")))
                EX_ERROR(_T("<trigger> tag has no name property"));

            const tstring &sName(node.GetProperty(_T("name")));
            CUITrigger *pTrigger(K2_NEW(ctx_Widgets,  CUITrigger)(sName));
            if (pTrigger != nullptr)
                pInterface->AddLocalTrigger(pTrigger);

            return true;
        }
        catch (CException& ex)
        {
            ex.Process(_T("CXMLProc_Trigger::Process() - "), NO_THROW);
            return false;
        }
    END_XML_PROCESSOR_NO_CHILDREN
}