// (C)2006 S2 Games
// c_skillgroupheal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillgroupheal.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ITEM_ALLOCATOR(Skill, GroupHeal);
//=============================================================================

/*====================
  CSkillGroupHeal::~CSkillGroupHeal
  ====================*/
CSkillGroupHeal::~CSkillGroupHeal()
{
}


/*====================
  CSkillGroupHeal::CSkillGroupHeal
  ====================*/
CSkillGroupHeal::CSkillGroupHeal()
{
}


/*====================
  CSkillGroupHeal::ActivatePrimary
  ====================*/
bool    CSkillGroupHeal::ActivatePrimary(int iButtonStatus)
{
    if (!ISkillItem::ActivatePrimary(iButtonStatus))
        return false;

    return true;
}
