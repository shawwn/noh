// (C)2007 S2 Games
// c_skillbeastbuild.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillbeastbuild.h"
#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Skill, BeastBuild)
//=============================================================================

/*====================
  CSkillBeastBuild::ActivatePrimary
  ====================*/
bool	CSkillBeastBuild::ActivatePrimary(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
		pOwner->SetNetFlags(ENT_NET_FLAG_BUILD_MODE);
	return true;
}
