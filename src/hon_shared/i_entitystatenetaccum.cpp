// (C)2010 S2 Games
// i_entitystatenetaccum.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entitystatenetaccum.h"

#include "../k2/c_xmlprocroot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint	IEntityStateNetAccum::s_uiBaseType(ENTITY_BASE_TYPE_STATE_NET_ACCUM);

DEFINE_ENTITY_DESC(IEntityStateNetAccum, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector, TypeVector)();
	const TypeVector &vBase(IEntityState::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fAccumulator"), TYPE_FLOAT, 0, 0));
}
//=============================================================================

//=============================================================================
// Entity definition
//=============================================================================
START_ENTITY_DEFINITION_XML_PROCESSOR(IEntityStateNetAccum, StateNetAccum)
	CStateDefinition::ReadSettings(pDefinition, node, bMod);
END_ENTITY_DEFINITION_XML_PROCESSOR(StateNetAccum, statenetaccum)
//=============================================================================

/*====================
  IEntityStateNetAccum::IEntityStateNetAccum
  ====================*/
IEntityStateNetAccum::IEntityStateNetAccum()
{
}


/*====================
  IEntityStateNetAccum::Baseline
  ====================*/
void	IEntityStateNetAccum::Baseline()
{
	IEntityState::Baseline();

	m_fAccumulator = 0.0f;
}


/*====================
  IEntityStateNetAccum::GetSnapshot
  ====================*/
void	IEntityStateNetAccum::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IEntityState::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteField(m_fAccumulator);
}


/*====================
  IEntityStateNetAccum::ReadSnapshot
  ====================*/
bool	IEntityStateNetAccum::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		IEntityState::ReadSnapshot(snapshot, 1);

		snapshot.ReadField(m_fAccumulator);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IEntityStateNetAccum::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}
