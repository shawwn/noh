// (C)2010 S2 Games
// c_xmlproc_cvar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_interface.h"
#include "c_interface.h"
//=============================================================================

namespace XMLInterface
{
    // <cvar>
    DECLARE_XML_PROCESSOR(cvar)
    BEGIN_XML_REGISTRATION(cvar)
        REGISTER_XML_PROCESSOR(interface)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(cvar, IWidget)
        PROFILE("CXMLProc_Trigger::Process");

        try
        {
            CInterface *pInterface(pObject->GetInterface());
            if (pInterface == nullptr)
                EX_ERROR(_T("Interface pointer not set for <cvar> tag"));

            if (!node.HasProperty(_T("name")))
                EX_ERROR(_T("<cvar> tag has no name property"));

            const tstring &sName(node.GetProperty(_T("name")));
            const tstring &sType(node.GetProperty(_T("type")));
            const tstring &sValue(node.GetProperty(_T("value")));

            ECvarType eType(CT_INVALID);

            if (TStringCompare(sType, _T("int")) == 0)
                eType = CT_INT;
            else if (TStringCompare(sType, _T("uint")) == 0)
                eType = CT_UINT;
            else if (TStringCompare(sType, _T("float")) == 0)
                eType = CT_FLOAT;
            else if (TStringCompare(sType, _T("string")) == 0)
                eType = CT_STRING;
            else if (TStringCompare(sType, _T("vec3")) == 0)
                eType = CT_VEC3;
            else if (TStringCompare(sType, _T("vec4")) == 0)
                eType = CT_VEC4;
            else
                eType = CT_STRING; // Default

            ICvar *pCvar(ICvar::Create(sName, eType, sValue, 0)); 
            if (pCvar == nullptr)
                EX_ERROR(_T("Create failed"));

            return true;
        }
        catch (CException& ex)
        {
            ex.Process(_T("CXMLProc_Cvar::Process() - "), NO_THROW);
            return false;
        }
    END_XML_PROCESSOR_NO_CHILDREN
}