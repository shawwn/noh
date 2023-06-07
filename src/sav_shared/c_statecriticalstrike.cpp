// (C)2006 S2 Games
// c_statecriticalstrike.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statecriticalstrike.h"
#include "c_meleeattackevent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME CriticalStrike

DEFINE_STATE_ALLOCATOR(CriticalStrike);

STATE_CVAR(tstring, IconPath,   _T("/human/units/savage/icons/criticalstrike.tga"));
STATE_CVAR(tstring, EffectPath, _T(""));
STATE_CVAR(bool,    IsDebuff,       false);
STATE_CVAR(bool,    IsMeleeMove,    true);
STATE_CVAR(tstring, Skin,           _T(""));

STATE_CVAR(uint,    AttackTime,     600);
STATE_CVAR(float,   AttackDamage,   50.0f);
STATE_CVAR(uint,    BleedDuration,  10000);
//=============================================================================

/*====================
  CStateCriticalStrike::~CStateCriticalStrike
  ====================*/
CStateCriticalStrike::~CStateCriticalStrike()
{
}


/*====================
  CStateCriticalStrike::CStateCriticalStrike
  ====================*/
CStateCriticalStrike::CStateCriticalStrike()
{
    ASSIGN_ENTITY_STATE_CVARS;
}


/*====================
  CStateCriticalStrike::DoAttack
  ====================*/
void    CStateCriticalStrike::DoAttack(CMeleeAttackEvent &attack)
{
    attack.SetAnim(_T("critical_strike"), s_cvarAttackTime);
    attack.AddState(_T("bleed"), s_cvarBleedDuration);
    attack.SetDamage(s_cvarAttackDamage);
    attack.SetDamageFlags(DAMAGE_FLAG_MELEE | DAMAGE_FLAG_PIERCE);
    Invalidate();
}
