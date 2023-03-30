// (C)2009 S2 Games
// c_shopiteminfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_shopiteminfo.h"

#include "c_shopinfo.h"

#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint	CShopItemInfo::s_uiBaseType(ENTITY_BASE_TYPE_SHOP_INFO);

DEFINE_ENT_ALLOCATOR2(Shop, ItemInfo)

DEFINE_ENTITY_DESC(CShopItemInfo, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,    TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IGameEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiRemainingStock"), TYPE_INT, 4, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiNextRestockTime"), TYPE_INT, 32, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unItemType"), TYPE_SHORT, 16, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yTeam"), TYPE_CHAR, 4, 0));
}
//=============================================================================

/*====================
  CShopItemInfo::CShopItemInfo
  ====================*/
CShopItemInfo::CShopItemInfo() :
IGameEntity(NULL),

m_uiRemainingStock(-1),
m_uiNextRestockTime(INVALID_TIME),

m_uiMaxStock(0),
m_uiMaxRestockTime(INVALID_TIME),

m_unItemType(INVALID_ENT_TYPE),
m_yTeam(0)
{
}


/*====================
  CShopItemInfo::GetSnapshot
  ====================*/
void	CShopItemInfo::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IGameEntity::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteField(m_uiRemainingStock);
	snapshot.WriteField(m_uiNextRestockTime);
	snapshot.WriteField(m_unItemType);
	snapshot.WriteField(m_yTeam);
}


/*====================
  CShopItemInfo::ReadSnapshot
  ====================*/
bool	CShopItemInfo::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		IGameEntity::ReadSnapshot(snapshot, uiVersion);

		snapshot.ReadField(m_uiRemainingStock);
		snapshot.ReadField(m_uiNextRestockTime);
		snapshot.ReadField(m_unItemType);
		snapshot.ReadField(m_yTeam);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CShopItemInfo::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CShopItemInfo::Baseline
  ====================*/
void	CShopItemInfo::Baseline()
{
	IGameEntity::Baseline();

	m_uiRemainingStock = -1;
	m_uiNextRestockTime = INVALID_TIME;

	m_uiMaxStock = 0;
	m_uiMaxRestockTime = INVALID_TIME;

	m_unItemType = INVALID_ENT_TYPE;
}


/*====================
  CShopItemInfo::PurchaseItem
  ====================*/
bool	CShopItemInfo::PurchaseItem()
{
	if (m_uiRemainingStock == 0)
		return false;

	if (m_uiRemainingStock == -1)
		return true;

	if (m_uiRemainingStock == m_uiMaxStock)
		m_uiNextRestockTime = Game.GetGameTime() + m_uiMaxRestockTime;

	m_uiRemainingStock--;

	return true;
}

/*====================
  CShopItemInfo::ReplenishItem
  ====================*/
void	CShopItemInfo::ReplenishItem(uint uiCharges)
{
	if (m_uiRemainingStock == -1)
		return;

	if (m_uiRemainingStock == m_uiMaxStock)
		return;

	m_uiRemainingStock = CLAMP(m_uiRemainingStock + uiCharges, uint(0), m_uiMaxStock);

	if (m_uiRemainingStock == m_uiMaxStock)
		m_uiNextRestockTime = INVALID_TIME;
}

/*====================
  CShopItemInfo::MatchStart
  ====================*/
void	CShopItemInfo::MatchStart()
{
	IGameEntity::MatchStart();

	if (m_uiRemainingStock < m_uiMaxStock)
		m_uiNextRestockTime = Game.GetGameTime() + m_uiMaxRestockTime;
}


/*====================
  CShopItemInfo::ServerFrameAction
  ====================*/
bool	CShopItemInfo::ServerFrameAction()
{
	if (!IGameEntity::ServerFrameAction())
		return false;

	if (m_uiNextRestockTime != INVALID_TIME && m_uiNextRestockTime < Game.GetGameTime())
	{
		m_uiRemainingStock = CLAMP(m_uiRemainingStock + 1, uint(0), m_uiMaxStock);
		
		if (m_uiRemainingStock < m_uiMaxStock)
			m_uiNextRestockTime += m_uiMaxRestockTime;
		else
			m_uiNextRestockTime = INVALID_TIME;
	}

	return true;
}
