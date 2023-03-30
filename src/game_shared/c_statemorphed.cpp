// (C)2007 S2 Games
// c_statemorphed.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemorphed.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Morphed)

vector<SDataField>*	CStateMorphed::s_pvFields;
//=============================================================================


/*====================
  CStateMorphed::CEntityConfig::CEntityConfig
  ====================*/
CStateMorphed::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateMorphed::CStateMorphed
  ====================*/
CStateMorphed::CStateMorphed() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateMorphed::GetTypeVector
  ====================*/
const vector<SDataField>&	CStateMorphed::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IEntityState::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());
		
		s_pvFields->push_back(SDataField(_T("m_hModel"), FIELD_PUBLIC, TYPE_RESHANDLE));
		s_pvFields->push_back(SDataField(_T("m_iDisguiseTeam"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_iDisguiseClient"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_unDisguiseItem"), FIELD_PUBLIC, TYPE_SHORT));
	}

	return *s_pvFields;
}


/*====================
  CStateMorphed::GetSnapshot
  ====================*/
void	CStateMorphed::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IEntityState::GetSnapshot(snapshot);

	snapshot.AddResHandle(m_hModel);
	snapshot.AddField(m_iDisguiseTeam);
	snapshot.AddField(m_iDisguiseClient);
	snapshot.AddField(m_unDisguiseItem);
}


/*====================
  CStateMorphed::ReadSnapshot
  ====================*/
bool	CStateMorphed::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		if (!IEntityState::ReadSnapshot(snapshot))
			return false;

		snapshot.ReadNextResHandle(m_hModel);
		snapshot.ReadNextField(m_iDisguiseTeam);
		snapshot.ReadNextField(m_iDisguiseClient);
		snapshot.ReadNextField(m_unDisguiseItem);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CStateMorphed::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CStateMorphed::Baseline
  ====================*/
void	CStateMorphed::Baseline()
{
	IEntityState::Baseline();

	m_hModel =			INVALID_RESOURCE;
	m_iDisguiseTeam =	-1;
	m_iDisguiseClient =	-1;
	m_unDisguiseItem =	INVALID_ENT_TYPE;
}
