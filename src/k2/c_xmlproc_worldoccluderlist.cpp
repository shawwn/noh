// (C)2006 S2 Games
// c_xmlproc_worldoccluderlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_worldoccluderlist.h"
#include "c_occluder.h"
//=============================================================================

// <occluderlist>
DECLARE_XML_PROCESSOR(occluderlist)
BEGIN_XML_REGISTRATION(occluderlist)
	REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(occluderlist, CWorldOccluderList)
END_XML_PROCESSOR(pObject)

// <occluder>
DECLARE_XML_PROCESSOR(occluder)
BEGIN_XML_REGISTRATION(occluder)
	REGISTER_XML_PROCESSOR(occluderlist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(occluder, CWorldOccluderList)
	uint uiIndex(node.GetPropertyInt(_T("index"), INVALID_INDEX));
	if (uiIndex != pObject->AllocateNewOccluder(uiIndex))
	{
		Console.Warn << _T("Allocated an occluder with the wrong index") << newl;
		return false;
	}
END_XML_PROCESSOR(pObject->GetOccluder(uiIndex))

// <point>
DECLARE_XML_PROCESSOR(point)
BEGIN_XML_REGISTRATION(point)
	REGISTER_XML_PROCESSOR(occluder)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(point, COccluder)
	pObject->AddPoint(node.GetPropertyV3(_T("position")));
END_XML_PROCESSOR(NULL)
