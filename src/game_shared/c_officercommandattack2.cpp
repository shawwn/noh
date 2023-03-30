// (C)2007 S2 Games
// c_officercommandattack2.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_officercommandattack2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(OfficerCommand, Attack2);
//=============================================================================

/*====================
  COfficerCommandAttack2::ImpactEntity
  ====================*/
bool	COfficerCommandAttack2::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	if (pTarget == NULL)
		return false;
	if (bCheckTarget && !IsValidTarget(pTarget, false))
		return false;

	if (pOwner->IsPlayer())
		pOwner->GetAsPlayerEnt()->OfficerCommand(OFFICERCMD_ATTACK, uiTargetIndex, V3_ZERO);
	
	evImpact.SetSourcePosition(pTarget->GetPosition());
	evImpact.SetSourceAngles(pTarget->GetAngles());

	return true;
}
