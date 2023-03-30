// (C)2009 S2 Games
// c_shopiteminfo.h
//
//=============================================================================
#ifndef __C_SHOPITEMINFO_H__
#define __C_SHOPITEMINFO_H__

//=============================================================================
// Headers
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CShopItemInfo
//=============================================================================
class CShopItemInfo : public IGameEntity
{
	DECLARE_ENTITY_DESC

private:
	DECLARE_ENT_ALLOCATOR2(Shop, ItemInfo);

	uint			m_uiRemainingStock;
	uint			m_uiNextRestockTime;

	uint			m_uiMaxRestockTime;
	uint			m_uiMaxStock;

	tstring			m_sItemName;
	ushort			m_unItemType;
	byte			m_yTeam;

public:
	~CShopItemInfo()	{}
	CShopItemInfo();

	SUB_ENTITY_ACCESSOR(CShopItemInfo, ShopItemInfo)

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	bool			PurchaseItem();
	void			ReplenishItem(uint uiCharges);

	uint			GetStockRemaining()					{ return m_uiRemainingStock; }
	uint			GetRestockTime()					{ return m_uiNextRestockTime; }

	void			SetItemName(const tstring &sName)	{ m_sItemName = sName; }
	void			SetItemType(ushort unType)			{ m_unItemType = unType; }
	void			SetRestockTime(uint uiTime)			{ m_uiMaxRestockTime = uiTime; }
	void			SetMaxStock(uint uiStock)			{ m_uiMaxStock = uiStock; m_uiRemainingStock = CLAMP(m_uiRemainingStock, uint(0), m_uiMaxStock); }
	void			SetRemainingStock(uint uiStock)		{ m_uiRemainingStock = CLAMP(uiStock, uint(0), m_uiMaxStock); }
	void			SetTeam(byte yTeam)					{ m_yTeam = yTeam; }

	tstring&		GetItemName()						{ return m_sItemName; }
	ushort			GetItemType()						{ return m_unItemType; }
	byte			GetTeam()							{ return m_yTeam; }

	virtual void	MatchStart();
	virtual bool	ServerFrameAction();
};
//=============================================================================

#endif //__C_SHOPITEMINFO_H__
