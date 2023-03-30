// (C)2007 S2 Games
// c_statemanaward.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statemanaward.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, ManaWard);

CVAR_FLOATF(	g_expManaRestore,				0.25f,			CVAR_GAMECONFIG);
CVAR_FLOATF(	g_goldManaRestore,				0.25f,			CVAR_GAMECONFIG);
//=============================================================================


/*====================
  CStateManaWard::CEntityConfig::CEntityConfig
  ====================*/
CStateManaWard::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaRegenBoost, 10.0f)
{
}


/*====================
  CStateManaWard::CStateManaWard
  ====================*/
CStateManaWard::CStateManaWard() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_fTotalRecovered(0.0f)
{
	m_modManaRegen.SetAdd(m_pEntityConfig->GetManaRegenBoost());
}


/*====================
  CStateManaWard::StateFrame
  ====================*/
void	CStateManaWard::StateFrame()
{
	IEntityState::StateFrame();

	ICombatEntity *pOwner(Game.GetCombatEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	if (pOwner->GetManaPercent() < 1.0f)
		m_fTotalRecovered += MsToSec(Host.GetFrameLength()) * m_pEntityConfig->GetManaRegenBoost();
}


/*====================
  CStateManaWard::Expired
  ====================*/
void	CStateManaWard::Expired()
{
	IEntityState::Expired();

	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;
	IPlayerEntity *pInflictor(Game.GetPlayerEntity(m_uiInflictorIndex));
	if (pInflictor == NULL)
		return;

	ushort unGoldReward(INT_ROUND(m_fTotalRecovered * g_goldManaRestore));
	pInflictor->GiveExperience(m_fTotalRecovered * g_expManaRestore, pOwner->GetPosition() + pOwner->GetBounds().GetMid());
	pInflictor->GiveGold(unGoldReward, pOwner->GetPosition() + pOwner->GetBounds().GetMid(), true);

	int iClientID(-1);
	if (pOwner->IsPlayer())
		iClientID = pOwner->GetAsPlayerEnt()->GetClientID();
	Game.MatchStatEvent(pInflictor->GetClientID(), PLAYER_MATCH_GOLD_EARNED, unGoldReward, -1, GetType(), pInflictor->GetType());
}
