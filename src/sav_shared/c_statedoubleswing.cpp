// (C)2006 S2 Games
// c_statedoubleswing.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statedoubleswing.h"
#include "c_meleeattackevent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME  DoubleSwing

DEFINE_STATE_ALLOCATOR(DoubleSwing);

STATE_CVAR(tstring, IconPath,   _T("/human/units/savage/icons/doubleattack.tga"));
STATE_CVAR(tstring, EffectPath, _T(""));
STATE_CVAR(bool,    IsDebuff,       false);
STATE_CVAR(bool,    IsMeleeMove,    true);
STATE_CVAR(tstring, Skin,           _T(""));

STATE_CVAR(uint,    AttackTime,         600);
STATE_CVAR(float,   AttackMinDamage,    140.0f);
STATE_CVAR(float,   AttackMaxDamage,    160.0f);
//=============================================================================

/*====================
  CStateDoubleSwing::~CStateDoubleSwing
  ====================*/
CStateDoubleSwing::~CStateDoubleSwing()
{
}


/*====================
  CStateDoubleSwing::CStateDoubleSwing
  ====================*/
CStateDoubleSwing::CStateDoubleSwing()
{
    ASSIGN_ENTITY_STATE_CVARS;
}


/*====================
  CStateDoubleSwing::DoAttack
  ====================*/
void    CStateDoubleSwing::DoAttack(CMeleeAttackEvent &attack)
{
    attack.SetAnim(_T("double_attack"), s_cvarAttackTime);
    attack.SetDamage(s_cvarAttackMinDamage, s_cvarAttackMaxDamage);
    Invalidate();
}
