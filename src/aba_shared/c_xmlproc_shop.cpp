// (C)2008 S2 Games
// c_xmlproc_shop.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_shopentity.h"
#include "c_shopdefinition.h"

#include "i_entitystate.h"
#include "c_statedefinition.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlprocroot.h"
//=============================================================================

DEFINE_DEFINITION_TYPE_INFO(CShopDefinition, ENTITY_BASE_TYPE_SHOP, Shop)

START_ENTITY_DEFINITION_XML_PROCESSOR(IShopEntity, Shop)
	pDefinition->SetDisplayNamePriority(pDefinition->GetPriority());
	pDefinition->SetDescriptionPriority(pDefinition->GetPriority());

	READ_ENTITY_DEFINITION_PROPERTY(Style, style)
	READ_ENTITY_DEFINITION_PROPERTY(Header, header)
	READ_ENTITY_DEFINITION_PROPERTY(Icon, icon)
	READ_ENTITY_DEFINITION_PROPERTY(AllowRemoteAccess, allowremoteaccess)
	READ_ENTITY_DEFINITION_PROPERTY(Order, order)
	READ_ENTITY_DEFINITION_PROPERTY(Slot, slot)
	READ_ENTITY_DEFINITION_PROPERTY(RecommendedItems, recommendeditems)
END_ENTITY_DEFINITION_XML_PROCESSOR(Shop, shop)

// <item>
DECLARE_XML_PROCESSOR(item)
BEGIN_XML_REGISTRATION(item)
	REGISTER_XML_PROCESSOR_EX(XMLShop, shop)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(item, CShopDefinition)
	const tstring &sName(node.GetProperty(_T("name")));
	if (sName.empty())
		return false;
	pObject->AddItem(sName);
END_XML_PROCESSOR_NO_CHILDREN