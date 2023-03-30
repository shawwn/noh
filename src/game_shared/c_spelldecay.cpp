// (C)2007 S2 Games
// c_spelldecay.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spelldecay.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Decay)
//=============================================================================

/*====================
  CSpellDecay::ActiveFrame
  ====================*/
void	CSpellDecay::ActiveFrame()
{
	ISpellToggle::ActiveFrame();

	if (!HasNetFlags(ITEM_NET_FLAG_ACTIVE))
		return;

	ICombatEntity *pOwner(GetOwnerEnt());
	if (pOwner == NULL)
		return;

	CGameEvent evImpact;
	ImpactPosition(pOwner->GetPosition(), evImpact);
}
