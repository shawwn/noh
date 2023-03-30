// (C)2007 S2 Games
// c_gadgetmole.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetmole.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarf g_expRepairing;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, Mole)

CCvarf CGadgetMole::s_cvarRepairRadius(	_T("Gadget_Mole_RepairRadius"),	300.0f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf CGadgetMole::s_cvarRepairRate(	_T("Gadget_Mole_RepairRate"),	50.0f,	CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CGadgetMole::CGadgetMole
  ====================*/
CGadgetMole::CGadgetMole() :
IGadgetEntity(GetEntityConfig()),
m_fRepairAccumulator(0.0f)
{
}


/*====================
  CGadgetMole::Kill
  ====================*/
void	CGadgetMole::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
	IPlayerEntity *pOwner(Game.GetPlayerEntity(m_uiOwnerIndex));
	if (pOwner != NULL)
		pOwner->GiveExperience(m_fRepairAccumulator * g_expRepairing, GetPosition());

	IGadgetEntity::Kill(pAttacker, unKillingObjectID);
}


/*====================
  CGadgetMole::ServerFrame
  ====================*/
bool	CGadgetMole::ServerFrame()
{
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	
	uivector vResult;
	Game.GetEntitiesInRadius(vResult, CSphere(GetPosition(), s_cvarRepairRadius), 0);
	for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
	{
		IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
		if (pEntity == NULL)
			continue;
		if (pEntity->GetTeam() != GetTeam())
			continue;
		IBuildingEntity *pBuilding(pEntity->GetAsBuilding());
		if (pBuilding == NULL)
			continue;
		if (pBuilding->GetStatus() != ENTITY_STATUS_SPAWNING)
			continue;

		float fPrevHealth(pBuilding->GetHealth());
		float fHPRepaired(MIN(pBuilding->GetHealth() + MsToSec(Game.GetFrameLength()) * s_cvarRepairRate, pBuilding->GetMaxHealth())); 
		pBuilding->SetHealth(fHPRepaired);
		if (pOwner != NULL)
		{
			if (pOwner->IsPlayer())
				Game.MatchStatEvent(pOwner->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_REPAIRED, fHPRepaired - fPrevHealth, -1, GetType(), pBuilding->GetType());

			m_fRepairAccumulator += fHPRepaired - fPrevHealth;
		}
	}

	return IGadgetEntity::ServerFrame();
}
