// (C)2006 S2 Games
// c_statebackstab.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebackstab.h"
#include "c_meleeattackevent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME BackStab

DEFINE_STATE_ALLOCATOR(BackStab);

STATE_CVAR(tstring, IconPath,   _T("/human/units/scout/icons/backstab.tga"));
STATE_CVAR(tstring, EffectPath, _T(""));
STATE_CVAR(bool,    IsDebuff,       false);
STATE_CVAR(bool,    IsMeleeMove,    true);
STATE_CVAR(tstring, Skin,           _T(""));

STATE_CVAR(uint,    AttackTime,     600);
STATE_CVAR(float,   AttackDamage,   50.0f);
//=============================================================================

/*====================
  CStateBackStab::~CStateBackStab
  ====================*/
CStateBackStab::~CStateBackStab()
{
}


/*====================
  CStateBackStab::CStateBackStab
  ====================*/
CStateBackStab::CStateBackStab()
{
    ASSIGN_ENTITY_STATE_CVARS;
}


/*====================
  CStateBackStab::DoAttack
  ====================*/
void    CStateBackStab::DoAttack(CMeleeAttackEvent &attack)
{
    attack.SetAnim(_T("backstab"), s_cvarAttackTime);
    attack.SetDamage(s_cvarAttackDamage);
    attack.SetDamageFlags(DAMAGE_FLAG_MELEE | DAMAGE_FLAG_PIERCE);
    Invalidate();
}
