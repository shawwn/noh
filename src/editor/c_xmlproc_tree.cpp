// (C)2009 S2 Games
// c_xmlproc_tree.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "c_treedefinitionresource.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_console.h"
//=============================================================================

namespace XMLTreeDefinitionResource
{
    // <tree>
    DECLARE_XML_PROCESSOR(tree)
    BEGIN_XML_REGISTRATION(tree)
        REGISTER_XML_PROCESSOR(root)
    END_XML_REGISTRATION
    BEGIN_XML_PROCESSOR(tree, CTreeDefinitionResource)
        pObject->SetTreeScaleMin(node.GetPropertyFloat(_CTS("minscale"), 1.0f));
        pObject->SetTreeScaleMax(node.GetPropertyFloat(_CTS("maxscale"), 1.0f));
        pObject->SetTreeModelPath(node.GetProperty(_CTS("model"), _T("undefined")));
        pObject->SetTreeName(node.GetProperty(_CTS("name"), _T("undefined")));
    END_XML_PROCESSOR(pObject)
}