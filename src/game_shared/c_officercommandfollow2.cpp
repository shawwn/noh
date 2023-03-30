// (C)2007 S2 Games
// c_officercommandfollow2.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_officercommandfollow2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(OfficerCommand, Follow2);
//=============================================================================


/*====================
  COfficerCommandFollow::ImpactEntity
  ====================*/
bool	COfficerCommandFollow2::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
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
		pOwner->GetAsPlayerEnt()->OfficerCommand(OFFICERCMD_FOLLOW, uiTargetIndex, V3_ZERO);
	
	evImpact.SetSourcePosition(pTarget->GetPosition());
	evImpact.SetSourceAngles(pTarget->GetAngles());

	return true;
}
