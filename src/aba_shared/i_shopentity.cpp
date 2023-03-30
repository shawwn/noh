// (C)2008 S2 Games
// i_shopentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_shopentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint				IShopEntity::s_uiBaseType(ENTITY_BASE_TYPE_SHOP);

DEFINE_ENTITY_DESC(IShopEntity, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IGameEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  IShopEntity::IShopEntity
  ====================*/
IShopEntity::IShopEntity() :
IGameEntity(NULL)
{
}
