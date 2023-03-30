// (C)2006 S2 Games
// c_statebash.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebash.h"
#include "c_meleeattackevent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME	Bash

DEFINE_STATE_ALLOCATOR(Bash);

STATE_CVAR(tstring,	IconPath,		_T("/human/units/legionnaire/icons/stun.tga"));
STATE_CVAR(tstring,	EffectPath,		_T(""));
STATE_CVAR(bool,	IsDebuff,		false);
STATE_CVAR(bool,	IsMeleeMove,	true);
STATE_CVAR(tstring,	Skin,			_T(""));
//=============================================================================

/*====================
  CStateBash::~CStateBash
  ====================*/
CStateBash::~CStateBash()
{
}


/*====================
  CStateBash::CStateBash
  ====================*/
CStateBash::CStateBash()
{
	ASSIGN_ENTITY_STATE_CVARS;
}


/*====================
  CStateBash::DoAttack
  ====================*/
void	CStateBash::DoAttack(CMeleeAttackEvent &attack)
{
	attack.SetAnim(_T("bash"), 600);
	attack.AddState(_T("stunned"), 3000);
	attack.SetDamage(20.0f, 40.0f);
	Invalidate();
}
