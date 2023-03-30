// (C)2007 S2 Games
// c_xmlproc_include.cpp
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
#include "c_xmlmanager.h"
#include "c_xmlprocroot.h"
#include "c_interface.h"
//=============================================================================

EXTERN_XML_PROCESSOR(interface)

// <include>
DECLARE_XML_PROCESSOR(include)
BEGIN_XML_REGISTRATION(include)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(frame)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(include, IWidget)
    if (!node.HasProperty(_T("file")))
    {
        Console.Warn << _T("No file specified for <include>") << newl;
        return false;
    }

    const tstring &sFile(node.GetProperty(_T("file")));

    CInterface *pInterface(pObject->GetInterface());
    if (pInterface != NULL)
        pInterface->AddCallback(sFile);

    XMLManager.Process(sFile, _T("package"), pObject);
END_XML_PROCESSOR_NO_CHILDREN

// <package>
DECLARE_XML_PROCESSOR(package)
BEGIN_XML_REGISTRATION(package)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(package, IWidget)
    g_xmlproc_interface.ProcessChildren(node, pObject);
END_XML_PROCESSOR_NO_CHILDREN
