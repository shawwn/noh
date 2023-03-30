// (C)2006 S2 Games
// c_skillresurrect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillresurrect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ITEM_ALLOCATOR(Skill, Resurrect);
//=============================================================================

/*====================
  CSkillResurrect::~CSkillResurrect
  ====================*/
CSkillResurrect::~CSkillResurrect()
{
}


/*====================
  CSkillResurrect::CSkillResurrect
  ====================*/
CSkillResurrect::CSkillResurrect()
{
}


/*====================
  CSkillResurrect::ActivatePrimary
  ====================*/
bool	CSkillResurrect::ActivatePrimary(int iButtonStatus)
{
	if (!ISkillItem::ActivatePrimary(iButtonStatus))
		return false;

	return true;
}
