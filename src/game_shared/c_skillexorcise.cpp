// (C)2006 S2 Games
// c_skillexorcise.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillexorcise.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ITEM_ALLOCATOR(Skill, Exorcise);
//=============================================================================

/*====================
  CSkillExorcise::~CSkillExorcise
  ====================*/
CSkillExorcise::~CSkillExorcise()
{
}


/*====================
  CSkillExorcise::CSkillExorcise
  ====================*/
CSkillExorcise::CSkillExorcise()
{
}


/*====================
  CSkillExorcise::ActivatePrimary
  ====================*/
bool	CSkillExorcise::ActivatePrimary(int iButtonStatus)
{
	if (!ISkillItem::ActivatePrimary(iButtonStatus))
		return false;

	return true;
}
