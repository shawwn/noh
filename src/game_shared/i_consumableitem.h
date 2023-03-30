// (C)2006 S2 Games
// i_consumableitem.h
//
//=============================================================================
#ifndef __I_CONSUMABLEITEM_H__
#define __I_CONSUMABLEITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern uint g_uiNumConsumableItems;
extern GAME_SHARED_API class CConsumableItemManager* g_pConsumableItemManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef GAME_SHARED_EXPORTS
#define ConsumableItemManager (*CConsumableItemManager::GetInstance())
#else
#define ConsumableItemManager (*g_pConsumableItemManager)
#endif
//=============================================================================

//=============================================================================
// CConsumableItemManager
//=============================================================================
class CConsumableItemManager
{
	SINGLETON_DEF(CConsumableItemManager)

public:
	typedef pair<ICvar*, tstring>		ItemEntry;
	typedef list<ItemEntry>				ItemList;
	typedef ItemList::iterator			ItemList_it;
	typedef ItemList::const_iterator	ItemList_cit;

private:
	ItemList	m_lItems;
	svector		m_vNames;

public:
	~CConsumableItemManager()	{}

	uint					GetNumItems() const				{ return uint(m_vNames.size()); }
	tstring					GetItemName(uint i)				{ if (i >= GetNumItems()) return SNULL; return m_vNames[i]; }
	void					AddItem(const tstring &sName, ICvar *pSortVar);
	GAME_SHARED_API void	SortItems();
};
//=============================================================================

//=============================================================================
// IConsumableItem
//=============================================================================
class IConsumableItem : public IInventoryItem
{
protected:
	START_ENTITY_CONFIG(IInventoryItem)
		DECLARE_ENTITY_CVAR(bool, Passive)
		DECLARE_ENTITY_CVAR(tstring, State)
		DECLARE_ENTITY_CVAR(tstring, UseEffect)
		DECLARE_ENTITY_CVAR(tstring, GadgetName)
		DECLARE_ENTITY_CVAR(tstring, UniqueCategory)
		DECLARE_ENTITY_CVAR(CVec3f, GadgetOffset)
		DECLARE_ENTITY_CVAR(uint, MaxPerStack)
		DECLARE_ENTITY_CVAR(uint, MaxStacks)
		DECLARE_ENTITY_CVAR(uint, Duration)
		DECLARE_ENTITY_CVAR(float, AmmoMult)
		DECLARE_ENTITY_CVAR(float, DropWeight)
		DECLARE_ENTITY_CVAR(int, SortIndex)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;
	int				m_iStateSlot;

public:
	~IConsumableItem();
	IConsumableItem(CEntityConfig *pConfig);

	bool			IsConsumable() const				{ return true; }

	virtual void	Activate()							{ ActivatePrimary(GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED); }
	virtual bool	ActivatePrimary(int iButtonStatus);
	virtual bool	ActivatePassive();

	TYPE_NAME("Consumable Item")

	// Settings
	ENTITY_CVAR_ACCESSOR(bool, Passive, true)
	ENTITY_CVAR_ACCESSOR(tstring, State, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, UseEffect, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, GadgetName, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, UniqueCategory, _T(""))
	ENTITY_CVAR_ACCESSOR(CVec3f, GadgetOffset, V_ZERO)
	ENTITY_CVAR_ACCESSOR(uint, MaxPerStack, 1)
	ENTITY_CVAR_ACCESSOR(uint, MaxStacks, 1)
	ENTITY_CVAR_ACCESSOR(uint, Duration, 0)
	ENTITY_CVAR_ACCESSOR(float, AmmoMult, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, DropWeight, 0.0f)

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__I_CONSUMABLEITEM_H__
