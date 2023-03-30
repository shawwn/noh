// (C)2006 S2 Games
// c_statebeastheal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebeastheal.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarf g_expHealing;
extern CCvarf g_goldHealing;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, BeastHeal);
//=============================================================================


/*====================
  CStateBeastHeal::CEntityConfig::CEntityConfig
  ====================*/
CStateBeastHeal::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthPerSecond, 50.0f)
{
}


/*====================
  CStateBeastHeal::CStateBeastHeal
  ====================*/
CStateBeastHeal::CStateBeastHeal() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),

m_fTotalHealed(0.0f)
{
}


/*====================
  CStateBeastHeal::StateFrame
  ====================*/
void	CStateBeastHeal::StateFrame()
{
	IEntityState::StateFrame();

	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	m_fTotalHealed += pOwner->Heal(m_pEntityConfig->GetHealthPerSecond() * MsToSec(Game.GetFrameLength()), Game.GetVisualEntity(m_uiInflictorIndex));

	return;
}


/*====================
  CStateBeastHeal::Expired
  ====================*/
void	CStateBeastHeal::Expired()
{
	IEntityState::Expired();

	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;
	IPlayerEntity *pInflictor(Game.GetPlayerEntity(m_uiInflictorIndex));
	if (pInflictor == NULL)
		return;

	ushort unGoldReward(INT_ROUND(m_fTotalHealed * g_goldHealing));
	pInflictor->GiveExperience(m_fTotalHealed * g_expHealing, pOwner->GetPosition() + pOwner->GetBounds().GetMid());
	pInflictor->GiveGold(unGoldReward, pOwner->GetPosition() + pOwner->GetBounds().GetMid(), true);

	int iClientID(-1);
	if (pOwner->IsPlayer())
		iClientID = pOwner->GetAsPlayerEnt()->GetClientID();
	
	Game.MatchStatEvent(pInflictor->GetClientID(), PLAYER_MATCH_GOLD_EARNED, unGoldReward, -1, pInflictor->GetType(), GetType());
}
