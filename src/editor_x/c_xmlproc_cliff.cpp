// (C)2009 S2 Games
// c_xmlproc_cliff.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"
#include "../k2/c_cliffsetlist.h"

#include "../k2/c_cliffdefinitionresource.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
#include "../k2/c_console.h"
//=============================================================================

namespace XMLCliffDefinitionResource
{
	// <cliffdefinition>
	DECLARE_XML_PROCESSOR(cliffdefinition)
	BEGIN_XML_REGISTRATION(cliffdefinition)
		REGISTER_XML_PROCESSOR(root)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(cliffdefinition, CCliffDefinitionResource) 
		pObject->SetCliffName(node.GetProperty(_CWS("name"), _T("XMLFAIL")));
	END_XML_PROCESSOR(pObject)

	// <cliffpiece>
	DECLARE_XML_PROCESSOR(cliffpiece)
	BEGIN_XML_REGISTRATION(cliffpiece)
		REGISTER_XML_PROCESSOR(cliffdefinition)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(cliffpiece, CCliffDefinitionResource)
		CCliffPiece cNewCliffPiece;
		cNewCliffPiece.SetPieceType(node.GetProperty(_CWS("type"), _T("XMLFail")));
		pObject->GetCliffPieces()->push_back(cNewCliffPiece);
		END_XML_PROCESSOR(&pObject->GetCliffPieces()->back())

	// <variation>
	DECLARE_XML_PROCESSOR(variation)
	BEGIN_XML_REGISTRATION(variation)
		REGISTER_XML_PROCESSOR(cliffpiece)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(variation, CCliffPiece)
			CVariation cNewVariation;
			cNewVariation.SetDefaultRotation(node.GetPropertyFloat(_CWS("defaultRotation"), 0.0f));
			cNewVariation.SetPiecePath(node.GetProperty(_CWS("path"), _T("XMLfail")));		
			cNewVariation.SetRotationVertex(node.GetPropertyInt(_CWS("rotationVertex"), 0));
			pObject->GetVariations()->push_back(cNewVariation);
	END_XML_PROCESSOR_NO_CHILDREN
}