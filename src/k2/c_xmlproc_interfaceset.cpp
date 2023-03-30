// (C)2010 S2 Games
// c_xmlproc_interfaceset.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"
/*
#include "c_xmlproc_interfaceset.h"
#include "c_xmlprocroot.h"
#include "c_uimanager.h"
#include "c_interfacesetresource.h"
//=============================================================================

// <interfaceset>
BEGIN_XML_REGISTRATION(interfaceset)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(interfaceset, CInterfaceSetResource)
    PROFILE("CXMLProc_InterfaceSet::Process");

    const tstring &sName(node.GetProperty(_T("name")));
    pObject->SetName(sName);
END_XML_PROCESSOR(pObject)


namespace XMLInterfaceSetDynamic
{
    // <interface>
    DECLARE_XML_PROCESSOR(interface)
    BEGIN_XML_REGISTRATION(interface)
        REGISTER_XML_PROCESSOR(interfaceset)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(interface, CInterfaceSetResource)
        const tstring &sName(node.GetProperty(_T("name")));
        const tstring &sFile(node.GetProperty(_T("file")));
        bool bPrecache(node.GetPropertyBool(_T("precache")));

        pObject->AddNewInterface(sName, sFile, bPrecache, false);
    END_XML_PROCESSOR_NO_CHILDREN

    // <overlay>
    DECLARE_XML_PROCESSOR(overlay)
    BEGIN_XML_REGISTRATION(overlay)
        REGISTER_XML_PROCESSOR(interfaceset)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(overlay, CInterfaceSetResource)
        const tstring &sName(node.GetProperty(_T("name")));
        const tstring &sFile(node.GetProperty(_T("file")));

        pObject->AddNewInterface(sName, sFile, true, true);

    END_XML_PROCESSOR_NO_CHILDREN
}
*/