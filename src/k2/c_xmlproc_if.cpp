// (C)2008 S2 Games
// c_xmlproc_if.cpp
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
#include "c_xmlproc_floater.h"
#include "c_xmlproc_template.h"
#include "c_xmlmanager.h"
#include "c_xmlprocroot.h"
#include "c_uiscript.h"
#include "xtoa.h"

//=============================================================================

EXTERN_XML_PROCESSOR(interface)

// <if>
DECLARE_XML_PROCESSOR(if)
BEGIN_XML_REGISTRATION(if)
    REGISTER_XML_PROCESSOR(root)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(frame)
    REGISTER_XML_PROCESSOR(floater)
    REGISTER_XML_PROCESSOR(template)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(if, IWidget)

    // HACK: ParenStr causes the statement to leave a value on the stack
    // conditional processing of children for interfaces
    if (node.HasProperty(_T("condition"))
        && !AtoB(UIScript.Evaluate(pObject, ParenStr(node.GetProperty(_T("condition"))))))
        return true; // condition not met, skip children

    g_xmlproc_interface.ProcessChildren(node, pObject);
END_XML_PROCESSOR_NO_CHILDREN
