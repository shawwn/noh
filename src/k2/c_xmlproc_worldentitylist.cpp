// (C)2005 S2 Games
// c_xmlproc_worldentitylist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlprocroot.h"
#include "c_worldentitylist.h"
#include "c_worldentity.h"
//=============================================================================

// <entitylist>
DECLARE_XML_PROCESSOR(entitylist)
BEGIN_XML_REGISTRATION(entitylist)
    REGISTER_XML_PROCESSOR(root)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(entitylist, CWorldEntityList)
END_XML_PROCESSOR(pObject)

// <entity>
DECLARE_XML_PROCESSOR(entity)
BEGIN_XML_REGISTRATION(entity)
    REGISTER_XML_PROCESSOR(entitylist)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(entity, CWorldEntityList)
    uint uiIndex(node.GetPropertyInt(_CWS("index"), INVALID_INDEX));
    if (uiIndex != pObject->AllocateNewEntity(uiIndex))
    {
        Console.Warn << _T("Entity allocation failed for entity #") << uiIndex << newl;
        return false;
    }

    CWorldEntity* pEntity(pObject->GetEntity(uiIndex));
    if (pEntity == NULL)
    {
        Console.Warn << _T("Entity allocation failed for entity #") << uiIndex << newl;
        return false;
    }

    const CXMLNode::PropertyMap &mapProperties(node.GetPropertyMap());

    for (CXMLNode::PropertyMap_cit it(mapProperties.begin()); it != mapProperties.end(); ++it)
        pEntity->SetProperty(it->first, it->second);

    pEntity->SetIndex(uiIndex);
    pEntity->SetTeam(node.GetPropertyInt(_CWS("team")));
    pEntity->SetType(node.GetProperty(_CWS("type")));
    pEntity->SetSeed(node.GetPropertyInt(_CWS("seed")));
    pEntity->SetName(node.GetProperty(_CWS("name")));
    pEntity->SetPosition(node.GetPropertyV3(_CWS("position")));
    pEntity->SetAngles(node.GetPropertyV3(_CWS("angles")));
    pEntity->SetScale(node.GetPropertyFloat(_CWS("scale")));
    pEntity->SetModelPath(node.GetProperty(_CWS("model")));
    pEntity->SetSkinName(node.GetProperty(_CWS("skin")));

    if (node.GetPropertyBool(_CWS("notsolid")))
        pEntity->AddFlags(WE_NOT_SOLID);

END_XML_PROCESSOR(NULL)
