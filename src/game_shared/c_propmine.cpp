// (C)2006 S2 Games
// c_propmine.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_propmine.h"

#include "../k2/c_worldentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Prop, Mine)

vector<SDataField>*	CPropMine::s_pvFields;

CVAR_UINTF(	g_raidBonus, 500,	CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CPropMine::GetTypeVector
  ====================*/
const vector<SDataField>&	CPropMine::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IPropFoundation::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
		
		s_pvFields->push_back(SDataField(_T("m_uiTotalGold"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_uiHarvestedGold"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_uiHarvestRate"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_uiRaidBonus"), FIELD_PUBLIC, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  CPropMine::Baseline
  ====================*/
void	CPropMine::Baseline()
{
	IPropFoundation::Baseline();
	m_uiTotalGold = 0;
	m_uiHarvestedGold = 0;
	m_uiHarvestRate = 0;
	m_uiRaidBonus = 0;
}


/*====================
  CPropMine::GetSnapshot
  ====================*/
void	CPropMine::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IPropFoundation::GetSnapshot(snapshot);
	snapshot.AddField(m_uiTotalGold);
	snapshot.AddField(m_uiHarvestedGold);
	snapshot.AddField(m_uiHarvestRate);
	snapshot.AddField(m_uiRaidBonus);
}


/*====================
  CPropMine::ReadSnapshot
  ====================*/
bool	CPropMine::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		if (!IPropFoundation::ReadSnapshot(snapshot))
			return false;
		snapshot.ReadNextField(m_uiTotalGold);
		snapshot.ReadNextField(m_uiHarvestedGold);
		snapshot.ReadNextField(m_uiHarvestRate);
		snapshot.ReadNextField(m_uiRaidBonus);
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CPropMine::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CPropMine::ApplyWorldEntity
  ====================*/
void	CPropMine::ApplyWorldEntity(const CWorldEntity &ent)
{
	IPropEntity::ApplyWorldEntity(ent);

	m_uiTotalGold = ent.GetPropertyInt(_T("gold"));
	m_uiHarvestRate = ent.GetPropertyInt(_T("harvestrate"));
	m_uiRaidBonus = g_raidBonus;
}


/*====================
  CPropMine::Spawn
  ====================*/
void	CPropMine::Spawn()
{
	SetStatus(ENTITY_STATUS_ACTIVE);

	IPropEntity::Spawn();

	if (m_iTeam == -1)
	{
		for (int i(0); i < Game.GetNumTeams(); ++i)
			AssignToTeam(i);
	}
	else
	{
		AssignToTeam(m_iTeam);
	}
}


/*====================
  CPropMine::AddToScene
  ====================*/
bool	CPropMine::AddToScene(const CVec4f &v4Color, int iFlags)
{
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return false;

	if (!IPropEntity::AddToScene(v4Color, iFlags))
		return false;
	return IPropFoundation::AddToScene(v4Color, iFlags);
}


/*====================
  CPropMine::Copy
  ====================*/
void	CPropMine::Copy(const IGameEntity &B)
{
	IPropFoundation::Copy(B);

	const IPropEntity *pProp(B.GetAsProp());
	if (pProp == NULL)
		return;
	const IPropFoundation *pFoundation(pProp->GetAsFoundation());
	if (pFoundation == NULL)
		return;
	
	if (!pFoundation->IsMine())
		return;
	const CPropMine &C(*static_cast<const CPropMine*>(pFoundation));

	m_uiTotalGold =		C.m_uiTotalGold;
	m_uiHarvestedGold =	C.m_uiHarvestedGold;
	m_uiHarvestRate =	C.m_uiHarvestRate;
	m_uiRaidBonus =		C.m_uiRaidBonus;
}


/*====================
  CPropMine::Link
  ====================*/
void	CPropMine::Link()
{
	if (GetStatus() == ENTITY_STATUS_ACTIVE)
		IPropFoundation::Link();
}
