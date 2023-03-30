// (C)2006 S2 Games
// c_statepersistantitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statepersistantitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, PersistantItem);

vector<SDataField>*	CStatePersistantItem::s_pvFields;
//=============================================================================


/*====================
  CStatePersistantItem::CEntityConfig::CEntityConfig
  ====================*/
CStatePersistantItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStatePersistantItem::CStatePersistantItem
  ====================*/
CStatePersistantItem::CStatePersistantItem() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_unItemData(PERSISTANT_ITEM_NULL),
m_uiRegenMod(PERSISTANT_REGEN_NULL),
m_uiIncreaseMod(PERSISTANT_INCREASE_NULL),
m_uiReplenishMod(PERSISTANT_REPLENISH_NULL),
m_uiPersistantType(PERSISTANT_TYPE_NULL)
{
}


/*====================
  CStatePersistantItem::GetTypeVector
  ====================*/
const vector<SDataField>&	CStatePersistantItem::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IEntityState::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
		
		s_pvFields->push_back(SDataField(_T("m_unItemData"), FIELD_PUBLIC, TYPE_SHORT));
	}

	return *s_pvFields;
}


/*====================
  CStatePersistantItem::GetSnapshot
  ====================*/
void	CStatePersistantItem::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IEntityState::GetSnapshot(snapshot);

	snapshot.AddField(m_unItemData);
}


/*====================
  CStatePersistantItem::ReadSnapshot
  ====================*/
bool	CStatePersistantItem::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		if (!IEntityState::ReadSnapshot(snapshot))
			return false;

		snapshot.ReadNextField(m_unItemData);

		UpdateItemData();

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CStatePersistantItem::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CStatePersistantItem::Baseline
  ====================*/
void	CStatePersistantItem::Baseline()
{
	IEntityState::Baseline();

	m_unItemData = PERSISTANT_ITEM_NULL;
}


/*====================
  CStatePersistantItem::UpdateItemData
  ====================*/
void	CStatePersistantItem::UpdateItemData()
{
	if (m_unItemData == PERSISTANT_ITEM_NULL)
	{
		m_uiPersistantType = 0;
		m_uiRegenMod = 0;
		m_uiIncreaseMod = 0;
		m_uiReplenishMod = 0;
		return;
	}

	m_uiPersistantType = (m_unItemData / 1000) % 10;
	m_uiRegenMod = (m_unItemData / 100) % 10;
	m_uiIncreaseMod = (m_unItemData / 10) % 10;
	m_uiReplenishMod = (m_unItemData % 10);

	FloatMod modValue;
	modValue.SetMult(GetMultiplier());

	if (m_uiIncreaseMod == PERSISTANT_INCREASE_HEALTH)
	{
		// Track change in player's health
		IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiOwnerIndex));

		if (pPlayer != NULL)
		{
			float fHealthPercent(1.0f);

			if (!pPlayer->HasNetFlags(ENT_NET_FLAG_KILLED))
				fHealthPercent = pPlayer->GetHealthPercent();

			SetMod(m_uiIncreaseMod, modValue);

			pPlayer->SetHealth(pPlayer->GetMaxHealth() * fHealthPercent);
		}
		else
			SetMod(m_uiIncreaseMod, modValue);
	}
	else
		SetMod(m_uiIncreaseMod, modValue);

	// If we've got an income generation mod, we want
	// to add the multiplier, not multiply gold by it
	if (m_uiRegenMod == PERSISTANT_REGEN_INCOME)
	{
		modValue.SetMult(1.0f);
		modValue.SetAdd((GetMultiplier() - 1.0f) * 100);
	}

	SetMod(m_uiRegenMod + MOD_REGEN_NULL, modValue);
}


/*====================
  CStatePersistantItem::SetItemData
  ====================*/
void	CStatePersistantItem::SetItemData(ushort unData)
{
	m_unItemData = unData;
	UpdateItemData();
}
