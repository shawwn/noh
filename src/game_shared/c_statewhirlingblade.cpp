// (C)2006 S2 Games
// c_statewhirlingblade.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statewhirlingblade.h"
#include "c_meleeattackevent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME	WhirlingBlade

DEFINE_STATE_ALLOCATOR(WhirlingBlade);

STATE_CVAR(tstring,	IconPath,	_T("/human/units/legionnaire/icons/whirlingblade.tga"));
//=============================================================================

/*====================
  CStateWhirlingBlade::~CStateWhirlingBlade
  ====================*/
CStateWhirlingBlade::~CStateWhirlingBlade()
{
}


/*====================
  CStateWhirlingBlade::CStateWhirlingBlade
  ====================*/
CStateWhirlingBlade::CStateWhirlingBlade()
{
	ASSIGN_ENTITY_STATE_CVARS;
}


/*====================
  CStateWhirlingBlade::DoAttack
  ====================*/
void	CStateWhirlingBlade::DoAttack(CMeleeAttackEvent &attack)
{
	attack.SetAnimName(_T("whirlingblade"));
	attack.SetAnimLength(1500);
	attack.SetAttackLength(1600);
	Invalidate();
}
