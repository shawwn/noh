// (C)2006 S2 Games
// c_skillbuild.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillbuild.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Skill, Build);
//=============================================================================

/*====================
  CSkillBuild::ActivatePrimary
  ====================*/
bool	CSkillBuild::ActivatePrimary(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
		pOwner->SetNetFlags(ENT_NET_FLAG_BUILD_MODE);
	return true;
}


/*====================
  CSkillBuild::Cancel
  ====================*/
bool	CSkillBuild::Cancel(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
		return false;

	pOwner->RemoveNetFlags(ENT_NET_FLAG_BUILD_MODE);
	
	return true;
}
