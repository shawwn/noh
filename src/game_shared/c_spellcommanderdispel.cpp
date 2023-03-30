// (C)2007 S2 Games
// c_spellcommanderdispel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellcommanderdispel.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, CommanderDispel)
//=============================================================================

/*====================
  CSpellCommanderDispel::ImpactEntity
  ====================*/
bool	CSpellCommanderDispel::ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget)
{
	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	if (pTarget == NULL)
		return false;
	if (bCheckTarget && !IsValidTarget(pTarget, true))
		return false;

	for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
	{
		IEntityState *pState(pTarget->GetState(i));
		if (pState != NULL && pState->GetIsDispellable())
			pTarget->RemoveState(i);
	}

	evImpact.SetSourcePosition(pTarget->GetPosition());
	evImpact.SetSourceAngles(pTarget->GetAngles());

	return true;
}
