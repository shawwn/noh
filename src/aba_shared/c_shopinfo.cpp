// (C)2009 S2 Games
// c_shopinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_shopinfo.h"

#include "c_shopiteminfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint    CShopInfo::s_uiBaseType(ENTITY_BASE_TYPE_SHOP_INFO);

DEFINE_ENT_ALLOCATOR2(Shop, Info)

DEFINE_ENTITY_DESC(CShopInfo, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,    TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  CShopInfo::CShopInfo
  ====================*/
CShopInfo::CShopInfo() :
IGameEntity(NULL)
{
}


/*====================
  CShopInfo::GetSnapshot
  ====================*/
void    CShopInfo::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);
}


/*====================
  CShopInfo::ReadSnapshot
  ====================*/
bool    CShopInfo::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IGameEntity::ReadSnapshot(snapshot, uiVersion);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CShopInfo::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  CShopInfo::Baseline
  ====================*/
void    CShopInfo::Baseline()
{
    IGameEntity::Baseline();
}


/*====================
  CShopInfo::AddItem
  ====================*/
void    CShopInfo::AddItem(CShopItemInfo *pItem)
{
    if (m_mapItemNames.find(pItem->GetItemName()) != m_mapItemNames.end() || m_mapItemTypes.find(pItem->GetItemType()) != m_mapItemTypes.end())
        return;

    m_mapItemNames.insert(ItemNamePair(pItem->GetItemName(), pItem->GetIndex()));
    m_mapItemTypes.insert(ItemTypePair(pItem->GetItemType(), pItem->GetIndex()));
}


/*====================
  CShopInfo::PurchaseItem
  ====================*/
bool    CShopInfo::PurchaseItem(const tstring &sName)
{
    CShopItemInfo *pItem(NULL);
    ItemNameMap_it it(m_mapItemNames.find(sName));

    if (it == m_mapItemNames.end())
        return true;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return true;

    return pItem->PurchaseItem();
}

bool    CShopInfo::PurchaseItem(uint uiTypeID)
{
    CShopItemInfo *pItem(NULL);
    ItemTypeMap_it it(m_mapItemTypes.find(uiTypeID));

    if (it == m_mapItemTypes.end())
        return true;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return true;

    return pItem->PurchaseItem();
}

/*====================
  CShopInfo::ReplenishItem
  ====================*/
void    CShopInfo::ReplenishItem(const tstring &sName, uint uiCharges)
{
    CShopItemInfo *pItem(NULL);
    ItemNameMap_it it(m_mapItemNames.find(sName));

    if (it == m_mapItemNames.end())
        return;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return;

    pItem->ReplenishItem(uiCharges);
}

void    CShopInfo::ReplenishItem(uint uiTypeID, uint uiCharges)
{
    CShopItemInfo *pItem(NULL);
    ItemTypeMap_it it(m_mapItemTypes.find(uiTypeID));

    if (it == m_mapItemTypes.end())
        return;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return;

    pItem->ReplenishItem(uiCharges);
}


/*====================
  CShopInfo::GetStockRemaining
  ====================*/
uint    CShopInfo::GetStockRemaining(const tstring &sName)
{
    CShopItemInfo *pItem(NULL);
    ItemNameMap_it it(m_mapItemNames.find(sName));

    if (it == m_mapItemNames.end())
        return -1;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return -1;

    return pItem->GetStockRemaining();
}

uint    CShopInfo::GetStockRemaining(uint uiTypeID)
{
    CShopItemInfo *pItem(NULL);
    ItemTypeMap_it it(m_mapItemTypes.find(uiTypeID));

    if (it == m_mapItemTypes.end())
        return -1;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return -1;

    return pItem->GetStockRemaining();
}


/*====================
  CShopInfo::GetRestockTimeRemaining
  ====================*/
uint    CShopInfo::GetRestockTimeRemaining(const tstring &sName)
{
    CShopItemInfo *pItem(NULL);
    ItemNameMap_it it(m_mapItemNames.find(sName));

    if (it == m_mapItemNames.end())
        return 0;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return 0;

    if (pItem->GetRestockTime() == INVALID_TIME || Game.GetGameTime() > pItem->GetRestockTime())
        return 0;

    return pItem->GetRestockTime() - Game.GetGameTime();
}

uint    CShopInfo::GetRestockTimeRemaining(ushort unTypeID)
{
    CShopItemInfo *pItem(NULL);
    ItemTypeMap_it it(m_mapItemTypes.find(unTypeID));

    if (it == m_mapItemTypes.end())
        return 0;

    pItem = Game.GetEntityAs<CShopItemInfo>(it->second);

    if (pItem == NULL)
        return 0;

    if (pItem->GetRestockTime() == INVALID_TIME || Game.GetGameTime() < pItem->GetRestockTime())
        return 0;

    return Game.GetGameTime() - pItem->GetRestockTime();
}
