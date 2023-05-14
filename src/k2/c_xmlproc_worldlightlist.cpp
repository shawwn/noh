// (C)2006 S2 Games
// c_xmlproc_worldlightlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_worldlightlist.h"
#include "c_worldlight.h"
//=============================================================================

// <lightlist>
DECLARE_XML_PROCESSOR(lightlist)
BEGIN_XML_REGISTRATION(lightlist)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(lightlist, CWorldLightList)
END_XML_PROCESSOR(pObject)

// <light>
DECLARE_XML_PROCESSOR(light)
BEGIN_XML_REGISTRATION(light)
    REGISTER_XML_PROCESSOR(lightlist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(light, CWorldLightList)
    uint uiIndex(node.GetPropertyInt(_T("index"), INVALID_INDEX));
    if (uiIndex != pObject->AllocateNewLight(uiIndex))
    {
        Console.Warn << _T("Light allocation failed for light #") << uiIndex << newl;
        return false;
    }

    CWorldLight* pLight(pObject->GetLight(uiIndex));
    if (pLight == nullptr)
    {
        Console.Warn << _T("Light allocation failed for light #") << uiIndex << newl;
        return false;
    }

    pLight->SetIndex(uiIndex);
    pLight->SetPosition(node.GetPropertyV3(_T("position")));
    pLight->SetColor(node.GetPropertyV3(_T("color")));
    pLight->SetFalloffStart(node.GetPropertyFloat(_T("falloffstart")));
    pLight->SetFalloffEnd(node.GetPropertyFloat(_T("falloffend")));
END_XML_PROCESSOR(nullptr)
