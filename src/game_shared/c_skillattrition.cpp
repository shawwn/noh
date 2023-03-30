// (C)2006 S2 Games
// c_skillattrition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillattrition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ITEM_ALLOCATOR(Skill, Attrition);
//=============================================================================

/*====================
  CSkillAttrition::~CSkillAttrition
  ====================*/
CSkillAttrition::~CSkillAttrition()
{
}


/*====================
  CSkillAttrition::CSkillAttrition
  ====================*/
CSkillAttrition::CSkillAttrition()
{
}


/*====================
  CSkillAttrition::ActivatePrimary
  ====================*/
bool	CSkillAttrition::ActivatePrimary(int iButtonStatus)
{
	if (!ISkillItem::ActivatePrimary(iButtonStatus))
		return false;

	return true;
}
