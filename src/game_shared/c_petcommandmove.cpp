// (C)2007 S2 Games
// c_petcommandmove.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_petcommandmove.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(PetCommand, Move);
//=============================================================================


/*====================
  CPetCommandMove::ImpactPosition
  ====================*/
void    CPetCommandMove::ImpactPosition(const CVec3f &v3Target, CGameEvent &evImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    if (pOwner->IsPlayer())
        pOwner->GetAsPlayerEnt()->PetCommand(PETCMD_MOVE, INVALID_INDEX, v3Target);

    evImpact.SetSourcePosition(v3Target);
    evImpact.SetSourceAngles(CVec3f(0.0f, 0.0f, pOwner->GetAngles()[YAW]));
}
