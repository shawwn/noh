// (C)2005 S2 Games
// c_xmlproc_texturelist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_texturelist.h"
//=============================================================================

// <texturelist>
DECLARE_XML_PROCESSOR(texturelist)
BEGIN_XML_REGISTRATION(texturelist)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(texturelist, CTextureList)
END_XML_PROCESSOR(pObject)

// <texture>
DECLARE_XML_PROCESSOR(texture)
BEGIN_XML_REGISTRATION(texture)
	REGISTER_XML_PROCESSOR(texturelist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(texture, CTextureList)
	uint uiID(node.GetPropertyInt(_CWS("id"), -1));
	const tstring &sName(node.GetProperty(_CWS("name")));

	if (uiID != INVALID_INDEX)
		pObject->AddTexture(uiID, sName);
END_XML_PROCESSOR(NULL)
