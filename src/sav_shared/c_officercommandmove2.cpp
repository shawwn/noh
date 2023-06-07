// (C)2007 S2 Games
// c_officercommandmove2.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_officercommandmove2.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(OfficerCommand, Move2);
//=============================================================================


/*====================
  COfficerCommandMove2::ImpactPosition
  ====================*/
bool    COfficerCommandMove2::ImpactPosition(const CVec3f &v3Target, CGameEvent &evImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (pOwner->IsPlayer())
        pOwner->GetAsPlayerEnt()->OfficerCommand(OFFICERCMD_MOVE, INVALID_INDEX, v3Target);

    evImpact.SetSourcePosition(v3Target);
    evImpact.SetSourceAngles(CVec3f(0.0f, 0.0f, pOwner->GetAngles()[YAW]));

    return true;
}
