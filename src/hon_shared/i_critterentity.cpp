// (C)2008 S2 Games
// i_critterentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_critterentity.h"

#include "i_behavior.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint				ICritterEntity::s_uiBaseType(ENTITY_BASE_TYPE_CRITTER);

DEFINE_ENTITY_DESC(ICritterEntity, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IUnitEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  ICritterEntity::ICritterEntity
  ====================*/
ICritterEntity::ICritterEntity() :
m_v3SpawnPosition(0.0f, 0.0f, 0.0f)
{
}


/*====================
  ICritterEntity::Baseline
  ====================*/
void	ICritterEntity::Baseline()
{
	IUnitEntity::Baseline();
}


/*====================
  ICritterEntity::GetSnapshot
  ====================*/
void	ICritterEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	// Base entity info
	IUnitEntity::GetSnapshot(snapshot, uiFlags);
}


/*====================
  ICritterEntity::ReadSnapshot
  ====================*/
bool	ICritterEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		// Base entity info
		if (!IUnitEntity::ReadSnapshot(snapshot, 1))
			return false;

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("ICritterEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  ICritterEntity::Copy
  ====================*/
void	ICritterEntity::Copy(const IGameEntity &B)
{
	IUnitEntity::Copy(B);

	const ICritterEntity *pB(B.GetAsCritter());

	if (!pB)	
		return;

	//const ICritterEntity &C(*pB);
}


/*====================
  ICritterEntity::ServerFrameThink
  ====================*/
bool	ICritterEntity::ServerFrameThink()
{
	// Issue default behavior

	if (m_uiOwnerEntityIndex == INVALID_INDEX)
	{
		if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
			m_cBrain.AddCommand(UNITCMD_GUARD, false, m_v3SpawnPosition.xy(), INVALID_INDEX, uint(-1), true);
	}
	else
	{
		if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
			m_cBrain.AddCommand(UNITCMD_GUARD, false, m_v3Position.xy(), INVALID_INDEX, uint(-1), true);
	}

	return IUnitEntity::ServerFrameThink();
}


/*====================
  ICritterEntity::Spawn
  ====================*/
void	ICritterEntity::Spawn()
{
	IUnitEntity::Spawn();

	m_v3SpawnPosition = m_v3Position;
}


/*====================
  ICritterEntity::Die
  ====================*/
void	ICritterEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
	IUnitEntity::Die(pAttacker, unKillingObjectID);
}
