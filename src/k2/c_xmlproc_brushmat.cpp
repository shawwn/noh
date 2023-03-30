// (C)2005 S2 Games
// c_xmlproc_brushmat.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_materialbrush.h"
#include "c_texture.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

// <brushmat>
DECLARE_XML_PROCESSOR(brushmat)
BEGIN_XML_REGISTRATION(brushmat)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(brushmat, CMaterialBrush)
	pObject->SetScale(node.GetPropertyInt(_T("scale"), 1));
END_XML_PROCESSOR(pObject)


// <tile>
DECLARE_XML_PROCESSOR(tile)
BEGIN_XML_REGISTRATION(tile)
	REGISTER_XML_PROCESSOR(brushmat)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(tile, CMaterialBrush)
	pObject->SetCurrentTile(node.GetPropertyInt(_T("index")));
END_XML_PROCESSOR(pObject)


// <shader>
DECLARE_XML_PROCESSOR(shader)
BEGIN_XML_REGISTRATION(shader)
	REGISTER_XML_PROCESSOR(tile)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(shader, CMaterialBrush)
	const tstring &sName(node.GetProperty(_T("name")));
	K2_WITH_GAME_RESOURCE_SCOPE()
		pObject->SetTile(g_ResourceManager.Register(K2_NEW(ctx_Resources,  CTexture)(sName, TEXTURE_2D, 0, TEXFMT_A8R8G8B8), RES_TEXTURE));
END_XML_PROCESSOR_NO_CHILDREN