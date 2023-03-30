// (C)2008 S2 Games
// c_xmlproc_posteffect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_posteffect.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

namespace XMLPostEffect
{
	// <posteffect>
	DECLARE_XML_PROCESSOR(posteffect)
	BEGIN_XML_REGISTRATION(posteffect)
		REGISTER_XML_PROCESSOR(root)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(posteffect, CPostEffect)
	END_XML_PROCESSOR(pObject)

	// <filter>
	DECLARE_XML_PROCESSOR(filter)
	BEGIN_XML_REGISTRATION(filter)
		REGISTER_XML_PROCESSOR(posteffect)
	END_XML_REGISTRATION
	BEGIN_XML_PROCESSOR(filter, CPostEffect)
		CPostFilter cFilter
		(
			g_ResourceManager.Register(node.GetProperty(_T("material"), _T("")), RES_MATERIAL)
		);
		pObject->AddFilter(cFilter);
	END_XML_PROCESSOR_NO_CHILDREN
}
