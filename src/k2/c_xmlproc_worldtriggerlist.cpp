// (C)2005 S2 Games
// c_xmlproc_worldtriggerlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_worldtriggerlist.h"
//=============================================================================

// <triggerlist>
DECLARE_XML_PROCESSOR(triggerlist)
BEGIN_XML_REGISTRATION(triggerlist)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(triggerlist, CWorldTriggerList)
END_XML_PROCESSOR(pObject)

// <trigger>
DECLARE_XML_PROCESSOR(trigger)
BEGIN_XML_REGISTRATION(trigger)
	REGISTER_XML_PROCESSOR(triggerlist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(trigger, CWorldTriggerList)
	if (!node.HasProperty(_T("name")))
	{
		Console.Warn << _T("Skipping unnamed trigger") << newl;
		return false;
	}

	pObject->RegisterNewScript(node.GetProperty(_T("name")), node.GetProperty(_T("content")));
END_XML_PROCESSOR(NULL)
