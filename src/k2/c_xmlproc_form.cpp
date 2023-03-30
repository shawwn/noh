// (C)2007 S2 Games
// c_xmlproc_form.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_interface.h"
#include "c_interface.h"
#include "c_uiform.h"
//=============================================================================

namespace XMLInterface
{
    // <form>
    DECLARE_XML_PROCESSOR(form)
    BEGIN_XML_REGISTRATION(form)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(form, IWidget)
        PROFILE("CXMLProc_Form::Process");

        try
        {
            CInterface *pInterface(pObject->GetInterface());
            if (pInterface == NULL)
                EX_ERROR(_T("Interface pointer not set for <form> tag"));

            if (!node.HasProperty(_T("name")))
                EX_ERROR(_T("<form> tag has no name property"));
            const tstring &sName(node.GetProperty(_T("name")));

            tstring sURL;
            const tstring &sHost(node.GetProperty(_T("host")));
            if (sHost == _T("!masterserver"))
                sURL = StringToTString(K2System.GetMasterServerAddress());
            else if (sHost == _T("!hon"))
                sURL = _T("www.heroesofnewerth.com");
            else
                EX_ERROR(_TS("<form> tag tried to set invalid host '") + sHost + _T("'"));

            CUIForm *pForm(K2_NEW(ctx_Widgets,  CUIForm)(Host.GetHTTPManager(), sName));
            pForm->SetTargetHost(sURL);
            pForm->SetTargetURI(node.GetProperty(_T("target")));
            pForm->SetStatusTrigger(node.GetProperty(_T("statustrigger")));
            pForm->SetResultTrigger(node.GetProperty(_T("resulttrigger")));
            if (StringCompare(node.GetProperty(L"method"), L"get") == 0)
                pForm->SetMethod(FORM_METHOD_GET);
            else if (StringCompare(node.GetProperty(L"method"), L"post") == 0)
                pForm->SetMethod(FORM_METHOD_POST);
            pForm->SetUseSSL(node.GetPropertyBool(L"ssl"));

            tstring sParam;
            for (uint ui(0); ui <= 199; ++ui)
            {
                sParam = _T("resultparam") + XtoA(ui);
                if (!node.HasProperty(sParam))
                    break;

                pForm->SetResultParam(ui, node.GetProperty(sParam));
            }

            if (pForm != NULL)
                pInterface->AddForm(pForm);

            return true;
        }
        catch (CException& ex)
        {
            ex.Process(_T("CXMLProc_Trigger::Process() - "), NO_THROW);
            return false;
        }
    END_XML_PROCESSOR_NO_CHILDREN
}