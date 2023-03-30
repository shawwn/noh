// (C)2008 S2 Games
// i_entityitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entityitem.h"
#include "i_unitentity.h"
#include "c_entitychest.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint			IEntityItem::s_uiBaseType(ENTITY_BASE_TYPE_ITEM);

CVAR_FLOATF(	g_itemSellValue,			0.5f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF(	g_itemRecipeSellValue,		0.85f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(		g_itemSellBackGracePeriod,	10000,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_STRINGF(	g_itemRecipeIconPath,		"",		CVAR_GAMECONFIG | CVAR_TRANSMIT);

DEFINE_ENTITY_DESC(IEntityItem, 2)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
	const TypeVector &vBase(IEntityTool::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iPurchaserClientNumber"), TYPE_INT, 5, -1));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiPurchaseTime"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IEntityItem::IEntityItem
  ====================*/
IEntityItem::IEntityItem() :
m_iPurchaserClientNumber(-1),
m_uiPurchaseTime(INVALID_TIME),
m_uiRecipeVariation(0)
{
}


/*====================
  IEntityItem::Baseline
  ====================*/
void	IEntityItem::Baseline()
{
	IEntityTool::Baseline();

	m_iPurchaserClientNumber = -1;
	m_uiPurchaseTime = INVALID_TIME;
}


/*====================
  IEntityItem::GetSnapshot
  ====================*/
void	IEntityItem::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IEntityTool::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteInteger(m_iPurchaserClientNumber);
	snapshot.WriteField(m_uiPurchaseTime);
}


/*====================
  IEntityItem::ReadSnapshot
  ====================*/
bool	IEntityItem::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		IEntityTool::ReadSnapshot(snapshot, 1);

		if (uiVersion > 1)
			snapshot.ReadInteger(m_iPurchaserClientNumber);
		else
			snapshot.ReadField(m_iPurchaserClientNumber);

		snapshot.ReadField(m_uiPurchaseTime);
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IEntityItem::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IEntityItem::UpdatePurchaseTime
  ====================*/
void	IEntityItem::UpdatePurchaseTime(uint uiTime)
{
	if (GetPurchaseTime() == INVALID_TIME || uiTime == INVALID_TIME)
		SetPurchaseTime(INVALID_TIME);
	else
		SetPurchaseTime(MIN(GetPurchaseTime(), uiTime));
}


/*====================
  IEntityItem::WasPurchasedRecently
  ====================*/
bool	IEntityItem::WasPurchasedRecently() const
{
	if (m_uiPurchaseTime == INVALID_TIME)
		return false;

	return (Game.GetGameTime() - m_uiPurchaseTime <= g_itemSellBackGracePeriod);
}


/*====================
  IEntityItem::Spawn
  ====================*/
void	IEntityItem::Spawn()
{
	IEntityTool::Spawn();

	m_yCharges = GetInitialCharges();

	if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
	{
		if (GetComponentsSize() > 0)
		{
			Assemble();
			if (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
				ClearFlag(ENTITY_TOOL_FLAG_ACTIVE);
		}
		else
		{
			SetFlag(ENTITY_TOOL_FLAG_ASSEMBLED);

			IUnitEntity *pOwner(GetOwner());
			if (pOwner != NULL)
				ExecuteActionScript(ACTION_SCRIPT_CREATE, pOwner, pOwner->GetPosition());
		}
	}
}


/*====================
  IEntityItem::ServerFrameAction
  ====================*/
bool	IEntityItem::ServerFrameAction()
{
	// if an item suddenly can't be carried by its owner, then drop it.
	IUnitEntity *pOwner(GetOwner());
	if (pOwner != NULL)
	{
		if (!pOwner->CanCarryItem(this))
		{
			// TODO: Right now, if an item suddenly can't be carried, then it is dropped and its
			// ownership is cleared.  However, I need to somehow expose to the scripters a way
			// to cause the item to be dropped without clearing ownership.
			Drop(pOwner->GetPosition(), true);
		}
	}

	return IEntityTool::ServerFrameAction();
}


/*====================
  IEntityItem::ServerFrameCleanup
  ====================*/
bool	IEntityItem::ServerFrameCleanup()
{
	return IEntityTool::ServerFrameCleanup();
}


/*====================
  IEntityItem::GetValue
  ====================*/
int		IEntityItem::GetValue() const
{
	if (m_uiPurchaseTime != INVALID_TIME && WasPurchasedRecently() &&
		(!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) || GetComponentsSize() == 0))
	{
		// Selling a stacked item
		if (!IsRecipe() && GetInitialCharges() > 0)
			return GetCost() * (GetCharges() / float(GetInitialCharges()));

		// Selling any other item
		return GetCost();
	}

	// Selling a recipe
	if (IsRecipe())
		return GetCost() * g_itemRecipeSellValue;

	uint uiTotalCost(GetTotalCost());

	// Adjust cost for item level
	if (GetLevel() > 0)
		uiTotalCost += GetCost() * (GetLevel() - 1);
	
	// Selling a regular item
	if (GetInitialCharges() == 0)
		return uiTotalCost * g_itemSellValue;

	// Selling an item with charges
	return (uiTotalCost * g_itemSellValue) * (GetCharges() / float(GetInitialCharges()));
}


/*====================
  IEntityItem::Assemble
  ====================*/
int		IEntityItem::Assemble()
{
	int iNewItemSlot(-1);

	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return iNewItemSlot;

	if (IsBorrowed() && !pOwner->CanReceiveOrdersFrom(m_iPurchaserClientNumber))
		return iNewItemSlot;

	bool bInStash(GetSlot() >= INVENTORY_START_STASH && GetSlot() <= INVENTORY_STASH_PROVISIONAL);
	int iStartSlot(bInStash ? INVENTORY_START_STASH : INVENTORY_START_BACKPACK);
	int iEndSlot(bInStash ? INVENTORY_STASH_PROVISIONAL : INVENTORY_BACKPACK_PROVISIONAL);

	// Check for an upgrade
	if (HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) && GetMaxLevel() > 0 && GetLevel() < GetMaxLevel())
	{
		// Search all slots for a recipe of this type
		for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
		{
			IEntityItem *pItem(pOwner->GetItem(iSlot));
			if (pItem == NULL || pItem == this || pItem->GetType() != GetType() || pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
				continue;
			if (pItem->GetPurchaserClientNumber() != -1 && pItem->GetPurchaserClientNumber() != m_iPurchaserClientNumber)
				continue;

			SetLevel(GetLevel() + 1);

			ExecuteActionScript(ACTION_SCRIPT_UPGRADE, pOwner, pOwner->GetPosition());

			pOwner->RemoveItem(pItem->GetSlot());
			iNewItemSlot = GetSlot();
			if (iNewItemSlot == INVENTORY_BACKPACK_PROVISIONAL)
				iNewItemSlot = pItem->GetSlot();

			pOwner->PlayRecipeEffect();
			Game.LogItem(GAME_LOG_ITEM_ASSEMBLE, this);
			SetPurchaseTime(INVALID_TIME);

			if (GetLevel() >= GetMaxLevel())
				break;
		}
	}

	if (GetComponentsSize() == 0 || HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
		return iNewItemSlot;
	
	uint uiRecipe(static_cast<TDefinition*>(m_pDefinition)->Assemble(pOwner, GetSlot()));
	if (uiRecipe == -1)
		return iNewItemSlot;

	// Find a slot in the inventory
	int iTargetSlot(-1);
	for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
	{
		if (iSlot == INVENTORY_BACKPACK_PROVISIONAL || iSlot == INVENTORY_STASH_PROVISIONAL)
			continue;

		IEntityItem *pItem(pOwner->GetItem(iSlot));
		if (pItem == NULL || pItem == this)
		{
			iTargetSlot = iSlot;
			break;
		}
	}
	if (iTargetSlot != -1 && GetSlot() != iTargetSlot)
		pOwner->SwapItem(pOwner->GetOwnerClientNumber(), GetSlot(), iTargetSlot);

	SetRecipeVariation(uiRecipe);
	SetFlag(ENTITY_TOOL_FLAG_ASSEMBLED);
	SetFlag(ENTITY_TOOL_FLAG_ACTIVE);
	SetCharges(GetInitialCharges());

	if (GetMaxLevel() > 0)
		SetLevel(1);

	SetActiveModifierKey(GetDefaultActiveModifierKey());

	pOwner->PlayRecipeEffect();
	Game.LogItem(GAME_LOG_ITEM_ASSEMBLE, this);

	ExecuteActionScript(ACTION_SCRIPT_CREATE, pOwner, pOwner->GetPosition());

	return pOwner->CheckRecipes(GetSlot());
}


/*====================
  IEntityItem::IsDisabled
  ====================*/
bool	IEntityItem::IsDisabled() const
{
	if (GetActionType() != TOOL_ACTION_PASSIVE)
	{
		IUnitEntity *pOwner(GetOwner());
		if (pOwner != NULL && pOwner->IsPerplexed())
			return true;
	}

	return IEntityTool::IsDisabled();
}


/*====================
  IEntityItem::CanUse
  ====================*/
bool	IEntityItem::CanUse() const
{
	// Check for a valid owner
	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return false;

	// If item is not bound to a player, it is free for all
	CPlayer *pPurchaser(Game.GetPlayer(m_iPurchaserClientNumber));
	if (pPurchaser == NULL)
		return true;

	// Items that are flagged can be shared by team mates
	if (pPurchaser->GetTeam() == pOwner->GetTeam() && GetAllowSharing())
		return true;

	// All others are only usable by their original purchaser
	if (pOwner->GetOwnerClientNumber() != m_iPurchaserClientNumber)
		return false;

	return true;
}


/*====================
  IEntityItem::IsBorrowed
  ====================*/
bool	IEntityItem::IsBorrowed() const
{
	if (m_iPurchaserClientNumber == -1)
		return false;

	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return false;

	return pOwner->GetOwnerClientNumber() != m_iPurchaserClientNumber;
}

/*====================
  IEntityItem::IsRecipe
  ====================*/
bool	IEntityItem::IsRecipe() const
{
	if (GetComponentsSize() > 0 && !HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
		return true;

	return false;
}

/*====================
  IEntityItem::CanStack
  ====================*/
bool	IEntityItem::CanStack(ushort unItemID, int iPurchaserClientNumber) const
{
	if (GetType() != unItemID)
		return false;

	if (GetPurchaserClientNumber() != -1 && iPurchaserClientNumber != -1)
	{
		// if the items were bought by different teammates, then don't allow them to be stacked.
		if (GetPurchaserClientNumber() != iPurchaserClientNumber)
			return false;
	}

	if (GetComponentsSize() > 0)
		return false;

	if (!GetRechargeable())
		return false;
	
	if (GetInitialCharges() <= 0)
		return false;

	if (GetMaxCharges() == -1)
		return true;
	
	if (GetCharges() + GetInitialCharges() > uint(GetMaxCharges()))
		return false;

	return true;
}

bool	IEntityItem::CanStack(IEntityItem *pItem) const
{
	if (pItem == NULL)
		return false;

	if (GetType() != pItem->GetType())
		return false;

	if (GetPurchaserClientNumber() != -1 && pItem->GetPurchaserClientNumber() != -1)
	{
		// if the items were bought by different teammates, then don't allow them to be stacked.
		if (GetPurchaserClientNumber() != pItem->GetPurchaserClientNumber())
			return false;
	}

	if (GetComponentsSize() > 0 && (!HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) || !pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED)))
		return false;

	if (!GetRechargeable())
		return false;
	
	if (GetInitialCharges() <= 0)
		return false;

	if (GetMaxCharges() == -1)
		return true;
	
	if (uint(GetCharges()) + pItem->GetCharges() > uint(GetMaxCharges()))
		return false;

	return true;
}


/*====================
  IEntityItem::CanAccess
  ====================*/
bool	IEntityItem::CanAccess(IUnitEntity *pUnit) const
{
	if (pUnit == NULL)
		return false;

	if (!IS_ITEM_SLOT(GetSlot()))
		return false;

	if (IS_BACKPACK_SLOT(GetSlot()))
		return true;

	if (IS_STASH_SLOT(GetSlot()) && pUnit->GetStashAccess())
		return true;

	return false;
}


/*====================
  IEntityItem::CanDrop
  ====================*/
bool	IEntityItem::CanDrop()
{
	if (GetActionType() == TOOL_ACTION_TOGGLE && HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
		return false;

	return true;
}


/*====================
  IEntityItem::Drop
  ====================*/
CEntityChest*	IEntityItem::Drop(const CVec3f &v3Pos, bool bLoseOwnership)
{
	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return false;

	// Spawn a chest
	CEntityChest *pChest(static_cast<CEntityChest*>(Game.AllocateEntity(Entity_Chest)));

	if (pChest == NULL)
	{
		Console.Warn << _T("Failed to create chest entity") << newl;
		return NULL;
	}

	pChest->SetPosition(v3Pos);
	pChest->SetAngles(CVec3f(0.0f, 0.0f, 180.0f));
	pChest->SetTeam(TEAM_PASSIVE);
	pChest->Spawn();
	pChest->ValidatePosition(TRACE_CHEST_SPAWN);

	int iSlot(pChest->TransferItem(pOwner->GetOwnerClientNumber(), this));

	IEntityItem *pChestItem(pChest->GetItem(iSlot));
	if (pChestItem == NULL)
	{
		Game.DeleteEntity(pChest);
		return NULL;
	}

	pChestItem->ClearFlag(ENTITY_TOOL_FLAG_ACTIVE);
	if (bLoseOwnership)
		pChestItem->SetPurchaserClientNumber(-1);

	pChest->StartAnimation(_T("open"), 0);
	pChest->ValidatePosition(TRACE_TREASURE_CHEST);

	return pChest;
}

CEntityChest*	IEntityItem::Drop(ushort unID, const CVec3f &v3Pos)
{
	// Spawn a chest
	CEntityChest *pChest(static_cast<CEntityChest*>(Game.AllocateEntity(Entity_Chest)));
	if (pChest == NULL)
	{
		Console.Warn << _T("Failed to create chest entity") << newl;
		return NULL;
	}

	pChest->SetPosition(v3Pos);
	pChest->SetAngles(CVec3f(0.0f, 0.0f, 180.0f));
	pChest->SetTeam(TEAM_PASSIVE);
	pChest->Spawn();
	pChest->ValidatePosition(TRACE_CHEST_SPAWN);

	pChest->GiveItem(INVENTORY_START_BACKPACK, unID);

	IEntityItem *pItem(pChest->GetItem(INVENTORY_START_BACKPACK));
	if (pItem == NULL)
	{
		Game.DeleteEntity(pChest);
		return NULL;
	}

	pItem->ClearFlag(ENTITY_TOOL_FLAG_ACTIVE);

	pChest->StartAnimation(_T("open"), 0);
	
	return pChest;
}


/*====================
  IEntityItem::Delete

  *Poof*
  ====================*/
void	IEntityItem::Delete()
{
	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return;

	pOwner->RemoveItem(GetSlot());
}


/*====================
  IEntityItem::ClientPrecache
  ====================*/
void	IEntityItem::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	IEntityTool::ClientPrecache(pConfig, eScheme, sModifier);

	Game.RegisterIcon(g_itemRecipeIconPath);
}


/*====================
  IEntityItem::ServerPrecache
  ====================*/
void	IEntityItem::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	IEntityTool::ServerPrecache(pConfig, eScheme, sModifier);
}
