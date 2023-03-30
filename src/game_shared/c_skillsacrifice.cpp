// (C)2007 S2 Games
// c_skillsacrifice.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillsacrifice.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Skill, Sacrifice)
//=============================================================================


/*====================
  CSkillSacrifice::Impact
  ====================*/
void    CSkillSacrifice::Impact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        IEntityState *pState(pOwner->GetState(i));
        if (pState != NULL && pState->GetIsDispellable())
            pOwner->RemoveState(i);
    }

    ISkillSelfBuff::Impact();
}
