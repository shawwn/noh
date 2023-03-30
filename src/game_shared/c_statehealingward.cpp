// (C)2007 S2 Games
// c_statehealingward.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statehealingward.h"
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
DEFINE_ENT_ALLOCATOR2(State, HealingWard);
//=============================================================================

/*====================
  CStateHealingWard::CEntityConfig::CEntityConfig
  ====================*/
CStateHealingWard::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(HealthRegenBoost, 10.0f)
{
}


/*====================
  CStateHealingWard::CStateHealingWard
  ====================*/
CStateHealingWard::CStateHealingWard() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_fTotalHealed(0.0f)
{
	m_modHealthRegen.SetAdd(m_pEntityConfig->GetHealthRegenBoost());
}


/*====================
  CStateHealingWard::StateFrame
  ====================*/
void	CStateHealingWard::StateFrame()
{
	IEntityState::StateFrame();

	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	if (pOwner->GetHealthPercent() < 1.0f)
		m_fTotalHealed += MsToSec(Host.GetFrameLength()) * m_pEntityConfig->GetHealthRegenBoost();
}


/*====================
  CStateHealingWard::Expired
  ====================*/
void	CStateHealingWard::Expired()
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
	Game.MatchStatEvent(pInflictor->GetClientID(), PLAYER_MATCH_GOLD_EARNED, unGoldReward, -1, GetType(), pInflictor->GetType());
	Game.MatchStatEvent(pInflictor->GetClientID(), PLAYER_MATCH_HEALED, m_fTotalHealed, -1, GetType(), pInflictor->GetType());
}
