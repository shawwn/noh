// (C)2005 S2 Games
// c_xmlproc_materiallist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_materiallist.h"
#include "c_world.h"
//=============================================================================

// <materiallist>
DECLARE_XML_PROCESSOR(materiallist);
BEGIN_XML_REGISTRATION(materiallist)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(materiallist, CMaterialList)
END_XML_PROCESSOR(pObject)

// <material>
DECLARE_XML_PROCESSOR(material)
BEGIN_XML_REGISTRATION(material)
	REGISTER_XML_PROCESSOR(materiallist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(material, CMaterialList)
	uint uiID(node.GetPropertyInt(_CWS("id"), -1));
	const tstring &sName(node.GetProperty(_CWS("name")));

	if (uiID != uint(-1))
		pObject->AddMaterial(uiID, sName);
END_XML_PROCESSOR(NULL)
