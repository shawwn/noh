// (C)2006 S2 Games
// c_skilldisembark.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skilldisembark.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Skill, Disembark);
//=============================================================================

/*====================
  CSkillDisembark::ActivatePrimary
  ====================*/
bool    CSkillDisembark::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!ISkillItem::ActivatePrimary(iButtonStatus))
        return false;

    CVec3f v3OldPos(pOwner->GetPosition());

    if (pOwner->IsPlayer())
    {
        IPlayerEntity *pPlayer(pOwner->GetAsPlayerEnt());

        IPlayerEntity *pNewPlayer(Game.ChangeUnit(pPlayer->GetClientID(), Player_Engineer, CHANGE_UNIT_KILL | CHANGE_UNIT_SPAWN | CHANGE_UNIT_INHERIT_POS | CHANGE_UNIT_INHERIT_HP | CHANGE_UNIT_INHERIT_DAMAGE_RECORD));

        if (pNewPlayer != NULL)
            pNewPlayer->SetHealth(MIN(pNewPlayer->GetMaxHealth(), pNewPlayer->GetHealth() * 2));
    }

    return true;
}
