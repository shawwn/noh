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

#include "../k2/c_rampresource.h"
#include "../k2/c_resourcemanager.h"
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

	// <rampset>
	DECLARE_XML_PROCESSOR(rampset)
	BEGIN_XML_REGISTRATION(rampset)
		REGISTER_XML_PROCESSOR(cliffdefinition)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(rampset, CCliffDefinitionResource)
		pObject->GetRamp()->SetName(node.GetProperty(_CWS("name"), _T("untitled")));
	END_XML_PROCESSOR(pObject)

	// <type1>
	DECLARE_XML_PROCESSOR(type1)
	BEGIN_XML_REGISTRATION(type1)
		REGISTER_XML_PROCESSOR(rampset)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(type1, CCliffDefinitionResource)
		rRampPiece rType1;
		rType1.iTopRotationVertex = node.GetPropertyInt(_CWS("topRotationVertex"), 0);
		rType1.sTopPath = node.GetProperty(_CWS("topPath"), _T("untitled"));
		rType1.fTopDefRot = node.GetPropertyFloat(_CWS("topDefaultRotation"), 0.0f);
		rType1.iBotRotationVertex = node.GetPropertyInt(_CWS("botRotationVertex"), 0);
		rType1.sBotPath = node.GetProperty(_CWS("botPath"), _T("untitled"));
		rType1.fBotDefRot = node.GetPropertyFloat(_CWS("botDefaultRotation"), 0.0f);
		pObject->GetRamp()->SetRampType1(rType1);
	END_XML_PROCESSOR(pObject)

	// <type2>
	DECLARE_XML_PROCESSOR(type2)
	BEGIN_XML_REGISTRATION(type2)
		REGISTER_XML_PROCESSOR(rampset)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(type2, CCliffDefinitionResource)
		rRampPiece rType2;
		rType2.iTopRotationVertex = node.GetPropertyInt(_CWS("topRotationVertex"), 0);
		rType2.sTopPath = node.GetProperty(_CWS("topPath"), _T("untitled"));
		rType2.fTopDefRot = node.GetPropertyFloat(_CWS("topDefaultRotation"), 0.0f);
		rType2.iBotRotationVertex = node.GetPropertyInt(_CWS("botRotationVertex"), 0);
		rType2.sBotPath = node.GetProperty(_CWS("botPath"), _T("untitled"));
		rType2.fBotDefRot = node.GetPropertyFloat(_CWS("botDefaultRotation"), 0.0f);
		pObject->GetRamp()->SetRampType2(rType2);
	END_XML_PROCESSOR(pObject)
}