// (C)2007 S2 Games
// c_skillpounce.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillpounce.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Skill, Pounce)
//=============================================================================

/*====================
  CSkillPounce::ActivatePrimary
  ====================*/
CSkillPounce::CEntityConfig::CEntityConfig(const tstring &sName) :
ISkillMelee::CEntityConfig(sName),
INIT_ENTITY_CVAR(LeapRange, 0.0f),
INIT_ENTITY_CVAR(LeapTime, 0),
INIT_ENTITY_CVAR(ImpactAnim, _T(""))
{
}


/*====================
  CSkillPounce::CSkillPounce
  ====================*/
CSkillPounce::CSkillPounce() :
ISkillMelee(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CSkillPounce::FinishedAction
  ====================*/
void    CSkillPounce::FinishedAction(int iAction)
{
    if (iAction | PLAYER_ACTION_TRAJECTORY)
    {
        bool bDoAttack(true);

        ICombatEntity *pOwner(GetOwnerEnt());
        if (!pOwner)
            bDoAttack = false;

        if (pOwner->GetItem(0) == NULL)
            bDoAttack = false;

        IMeleeItem *pWeapon(pOwner->GetItem(0)->GetAsMelee());
        if (pWeapon == NULL)
            bDoAttack = false;

        Game.SelectItem(pOwner->GetDefaultInventorySlot());
        pOwner->SelectItem(pOwner->GetDefaultInventorySlot());

        if (bDoAttack)
        {
            // Get base stats for the attack
            CMeleeAttackEvent   &attack(pOwner->GetAttackEvent());
            attack.Clear();
            attack.SetWeaponPointer(pWeapon);
            attack.SetAnim(m_pEntityConfig->GetImpactAnim(), GetDuration());
            attack.SetDamage(GetMinDamage(), GetMaxDamage());
            attack.SetDamageFlags(DAMAGE_FLAG_MELEE | DAMAGE_FLAG_INTERRUPT);
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
                pOwner->SetAction(PLAYER_ACTION_SKILL_ATTACK | pOwner->GetAction(), Game.GetGameTime() + attack.GetLength());
            pOwner->StopAnimation(0);
        }
        return;
    }

    ISkillMelee::FinishedAction(iAction);
}


/*====================
  CSkillPounce::ActivatePrimary
  ====================*/
bool    CSkillPounce::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (pOwner->GetItem(0) == NULL)
        return false;

    // Check cooldown timer
    if (IsDisabled() || !IsReady())
        return false;
    
    IMeleeItem *pWeapon(pOwner->GetItem(0)->GetAsMelee());
    if (pWeapon == NULL)
        return false;

    if (!pOwner->IsIdle())
        return false;

    // Check mana cost
    if (!pOwner->SpendMana(GetManaCost()))
        return false;

    SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());

    // Animation
    if (!GetAnimName().empty())
        pOwner->StartAnimation(GetAnimName(), 0);

    // Create an event for the player activating this
    CSkillActivateEvent &activate(pOwner->GetSkillActivateEvent());
    activate.Clear();
    activate.SetOwner(pOwner);
    activate.SetSlot(m_ySlot);
    activate.SetActivateTime(Game.GetGameTime() + GetActivationTime());

    // Set the player's new action
    pOwner->SetAction(PLAYER_ACTION_SKILL | PLAYER_ACTION_TRAJECTORY, INVALID_TIME);

    CVec3f v3Pos(pOwner->GetPosition());
    CVec3f v3Dir(M_GetForwardVecFromAngles(pOwner->GetViewAngles()));
    v3Dir[Z] = 0.0f;
    v3Dir.Normalize();
    
    CVec3f v3Target(pOwner->GetPosition() + v3Dir * m_pEntityConfig->GetLeapRange());
    v3Target[Z] = Game.GetTerrainHeight(v3Target.x, v3Target.y);

    float fTime(MsToSec(m_pEntityConfig->GetLeapTime()));
    CVec3f v3Velocity(v3Dir * (m_pEntityConfig->GetLeapRange() / fTime));
    v3Velocity[Z] = ((v3Target.z - v3Pos.z) - 0.5f * -p_gravity * fTime * fTime) / fTime;

    pOwner->SetVelocity(v3Velocity);
    Game.SelectItem(GetSlot());
    pOwner->SelectItem(GetSlot());
    return true;
}
