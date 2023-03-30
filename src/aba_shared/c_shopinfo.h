// (C)2009 S2 Games
// c_shopinfo.h
//
//=============================================================================
#ifndef __C_SHOPINFO_H__
#define __C_SHOPINFO_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<tstring, uint>		ItemNameMap;
typedef ItemNameMap::iterator	ItemNameMap_it;
typedef	pair<tstring, uint>		ItemNamePair;

typedef map<ushort, uint>		ItemTypeMap;
typedef ItemTypeMap::iterator	ItemTypeMap_it;
typedef	pair<ushort, uint>		ItemTypePair;

class CShopItemInfo;
//=============================================================================

//=============================================================================
// CShopInfo
//=============================================================================
class CShopInfo : public IGameEntity
{
	DECLARE_ENTITY_DESC

private:
	DECLARE_ENT_ALLOCATOR2(Shop, Info);

	ItemNameMap	m_mapItemNames;
	ItemTypeMap	m_mapItemTypes;

public:
	~CShopInfo()	{}
	CShopInfo();

	SUB_ENTITY_ACCESSOR(CShopInfo, ShopInfo)

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	GAME_SHARED_API void			AddItem(CShopItemInfo *pItem);

	GAME_SHARED_API bool			PurchaseItem(const tstring &sName);
	GAME_SHARED_API bool			PurchaseItem(uint uiTypeID);

	GAME_SHARED_API void			ReplenishItem(const tstring &sName, uint uiCharges);
	GAME_SHARED_API void			ReplenishItem(uint uiTypeID, uint uiCharges);

	GAME_SHARED_API uint			GetStockRemaining(const tstring &sName);
	GAME_SHARED_API uint			GetStockRemaining(uint uiTypeID);

	GAME_SHARED_API uint			GetRestockTimeRemaining(const tstring &sName);
	GAME_SHARED_API uint			GetRestockTimeRemaining(ushort unTypeID);
};
//=============================================================================

#endif //__C_SHOPINFO_H__
