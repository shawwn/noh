// (C)2008 S2 Games
// c_xmlproc_itemdef.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_itemdefinition.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
//=============================================================================

// <item>
DECLARE_XML_PROCESSOR(item)
BEGIN_XML_REGISTRATION(item)
    REGISTER_XML_PROCESSOR(item)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(item, CItemDefinition)
    PropertyMap mapProperties(node.GetPropertyMap());

    //pObject->SetNpcType(node.GetProperty(_T("type"), _T("Unnamed Item")));

    //for (PropertyMap::iterator it(mapProperties.begin()); it != mapProperties.end(); ++it)
    //  pObject->SetProperty(it->first, it->second);
END_XML_PROCESSOR(pObject)
