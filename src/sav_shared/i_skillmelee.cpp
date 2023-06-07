// (C)2006 S2 Games
// i_skillmelee.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_skillmelee.h"
//=============================================================================

/*====================
  ISkillItem::CEntityConfig::CEntityConfig
  ====================*/
ISkillMelee::CEntityConfig::CEntityConfig(const tstring &sName) :
ISkillItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(MinDamage, 0.0f),
INIT_ENTITY_CVAR(MaxDamage, 0.0f),
INIT_ENTITY_CVAR(ImpactTime, 0),
INIT_ENTITY_CVAR(MinAngle, -30.0f),
INIT_ENTITY_CVAR(MaxAngle, 30.0f),
INIT_ENTITY_CVAR(AngleStep, 15.0f),
INIT_ENTITY_CVAR(MinRange, 10.0f),
INIT_ENTITY_CVAR(MaxRange, 60.0f),
INIT_ENTITY_CVAR(RangeStep, 20.0f),
INIT_ENTITY_CVAR(MinHeight, 20.0f),
INIT_ENTITY_CVAR(MaxHeight, 80.0f),
INIT_ENTITY_CVAR(HeightStep, 20.0f),
INIT_ENTITY_CVAR(RearMultiplier, 1.0f),
INIT_ENTITY_CVAR(TargetState, _T("")),
INIT_ENTITY_CVAR(TargetStateDuration, 0),
INIT_ENTITY_CVAR(Push, V_ZERO),
INIT_ENTITY_CVAR(Lunge, V_ZERO)
{
}


/*====================
  ISkillMelee::ActivatePrimary
  ====================*/
bool    ISkillMelee::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (pOwner->GetItem(0) == NULL)
        return false;

    IMeleeItem *pWeapon(pOwner->GetItem(0)->GetAsMelee());
    if (pWeapon == NULL)
        return false;

    if (!pOwner->IsIdle())
        return false;

    if (!ISkillItem::ActivatePrimary(iButtonStatus))
        return false;

    // Get base stats for the attack
    CMeleeAttackEvent   &attack(pOwner->GetAttackEvent());
    attack.Clear();
    attack.SetWeaponPointer(pWeapon);
    attack.SetAnim(GetAnimName(), GetDuration());
    attack.SetDamage(GetMinDamage(), GetMaxDamage());
    attack.SetDamageFlags(DAMAGE_FLAG_MELEE | DAMAGE_FLAG_INTERRUPT | DAMAGE_FLAG_SKILL);
    attack.SetRearAttackMultiplier(GetRearMultiplier());
    attack.SetImpactTime(Game.GetGameTime() + GetImpactTime());
    attack.SetPivot(0.0f, 0.0f);
    attack.SetMetric(MELEE_HEIGHT, GetMinHeight(), GetMaxHeight(), GetHeightStep());
    attack.SetMetric(MELEE_ANGLE, GetMinAngle(), GetMaxAngle(), GetAngleStep());
    attack.SetMetric(MELEE_RANGE, GetMinRange(), GetMaxRange(), GetRangeStep());
    attack.SetPush(GetPush());
    attack.SetLunge(GetLunge());
    attack.SetStartTime(Game.GetGameTime());
    
    if (!GetTargetState().empty())
        attack.AddState(GetTargetState(), GetTargetStateDuration());
    
    if (pWeapon->StartAttack())
    {
        pOwner->SetAction(PLAYER_ACTION_SKILL_ATTACK | pOwner->GetAction(), Game.GetGameTime() + attack.GetLength());
        return true;
    }

    return false;
}


/*====================
  ISkillMelee::ClientPrecache
  ====================*/
void    ISkillMelee::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));
}


/*====================
  ISkillMelee::ServerPrecache
  ====================*/
void    ISkillMelee::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));
}
