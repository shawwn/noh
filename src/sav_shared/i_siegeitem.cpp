// (C)2006 S2 Games
// i_siegeitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_siegeitem.h"

#include "../k2/c_camera.h"
#include "../k2/c_vid.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_host.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_eventscript.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_clientsnapshot.h"
//=============================================================================

/*====================
  ISiegeItem::CEntityConfig::CEntityConfig
  ====================*/
ISiegeItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(MinDamage, 0.0f),
INIT_ENTITY_CVAR(MaxDamage, 0.0f),
INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f),
INIT_ENTITY_CVAR(DamageRadius, 0.0f),
INIT_ENTITY_CVAR(SpinupTime, 0),
INIT_ENTITY_CVAR(AttackTime, 0),
INIT_ENTITY_CVAR(AttackDelay, 0),
INIT_ENTITY_CVAR(ProjectileName, _T("")),
INIT_ENTITY_CVAR(AreaOfEffect, true),
INIT_ENTITY_CVAR(TargetMaterialPath, _T("")),
INIT_ENTITY_CVAR(TargetRadius, 0.0f),
INIT_ENTITY_CVAR(MaxRange, 5000.0f),
INIT_ENTITY_CVAR(MinRange, 0.0f),
INIT_ENTITY_CVAR(AttackAnimName, _T("")),
INIT_ENTITY_CVAR(AttackOffset, V3_ZERO)
{
}


/*====================
  ISiegeItem::ISiegeItem
  ====================*/
ISiegeItem::ISiegeItem(CEntityConfig *pConfig) :
IInventoryItem(pConfig),
m_pEntityConfig(pConfig),
m_v3DelayedTarget(V3_ZERO)
{
}


/*====================
  ISiegeItem::Spinup
  ====================*/
bool    ISiegeItem::Spinup(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
        CoolDown();
    else if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
        pOwner->SetAction(PLAYER_ACTION_GUN_SPINUP, Game.GetGameTime() + m_pEntityConfig->GetSpinupTime());

    return true;
}

/*====================
  ISiegeItem::Delay
  ====================*/
bool    ISiegeItem::Delay(int iButtonStatus, const CVec3f &v3CameraAngles)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Don't interrupt other actions
    if (pOwner->GetAction() != PLAYER_ACTION_IDLE &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_WARM)
        return false;

    // Make sure non-automatic items don't activate if the button is held
    if (GetImpulseOnly() && !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    // Check mana
    if (GetManaCost() > 0.0f &&
        !pOwner->SpendMana(GetManaCost()))
        return false;

    // Check ammo
    if (GetAdjustedAmmoCount() > 0 &&
        !pOwner->UseAmmo(m_ySlot))
        return CoolDown();

    pOwner->SetAction(PLAYER_ACTION_GUN_DELAY, Game.GetGameTime() + m_pEntityConfig->GetAttackDelay());

    pOwner->StartAnimation(m_pEntityConfig->GetAttackAnimName(), 1);

    m_v3DelayedTarget = GetTargetLocation();

    return true;
}

/*====================
  ISiegeItem::Fire
  ====================*/
bool    ISiegeItem::Fire(int iButtonStatus, const CVec3f &v3CameraAngles)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Don't interrupt other actions
    if (pOwner->GetAction() != PLAYER_ACTION_IDLE &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_WARM &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_DELAY)
        return false;

    if (pOwner->GetAction() != PLAYER_ACTION_GUN_DELAY)
    {
        // Make sure non-automatic items don't activate if the button is held
        if (GetImpulseOnly() && !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
            return false;

        // Check mana
        if (GetManaCost() > 0.0f &&
            !pOwner->SpendMana(GetManaCost()))
            return false;

        // Check ammo
        if (GetAdjustedAmmoCount() > 0 &&
            !pOwner->UseAmmo(m_ySlot))
            return CoolDown();

        pOwner->StartAnimation(m_pEntityConfig->GetAttackAnimName(), 1);
    }

    pOwner->SetAction(PLAYER_ACTION_GUN_FIRE, Game.GetGameTime() + m_pEntityConfig->GetAttackTime());

    // Fire the weapon
    CAxis axis(pOwner->GetAngles());
    const CVec3f &v3Forward(axis.Forward());
    const CVec3f &v3Right(axis.Right());
    const CVec3f &v3Up(axis.Up());
    const CVec3f &v3Offset(m_pEntityConfig->GetAttackOffset());
    CVec3f v3Start(pOwner->GetPosition() + V_UP * pOwner->GetViewHeight());
    CVec3f v3End(v3Start + v3Forward * 5000.0f);
    
    if (Game.IsClient())
        return true;

    if (m_pEntityConfig->GetProjectileName().empty())
    {
        // Do a trace
        STraceInfo result;
        Game.TraceLine(result, v3Start, v3End, 0, pOwner->GetWorldIndex());
        if (result.uiEntityIndex != INVALID_INDEX)
        {
            IGameEntity *pEntity(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
            if (pEntity != NULL)
            {
                float fDamage = M_Randnum(m_pEntityConfig->GetMinDamage(), m_pEntityConfig->GetMaxDamage());
                IPlayerEntity *pPlayer = pEntity->GetAsPlayerEnt();

                if (pEntity->IsBuilding())
                    fDamage *= m_pEntityConfig->GetPierceBuilding();
                else if (pPlayer != NULL && pPlayer->GetIsSiege())
                    fDamage *= m_pEntityConfig->GetPierceSiege();
                else if (pPlayer != NULL && pPlayer->GetIsHellbourne())
                    fDamage *= m_pEntityConfig->GetPierceHellbourne();
                else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
                    fDamage *= m_pEntityConfig->GetPierceUnit();

                pEntity->Damage(fDamage, 0, pOwner, GetType());
                pEntity->Hit(result.v3EndPos, result.plPlane.v3Normal); // FIXME: Put something meaningful here
            }
        }
    }
    else
    {
        // Spawn a projectile
        IGameEntity *pNewEnt(Game.AllocateEntity(m_pEntityConfig->GetProjectileName()));
        if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
        {
            Console.Warn << _T("Failed to spawn projectile: ") << m_pEntityConfig->GetProjectileName() << newl;
            return false;
        }

        IProjectile *pProjectile(pNewEnt->GetAsProjectile());

        CVec3f v3TargetPos;

        if (m_v3DelayedTarget != V3_ZERO)
        {
            v3TargetPos = m_v3DelayedTarget;
            m_v3DelayedTarget = V3_ZERO;
        }
        else
            v3TargetPos = GetTargetLocation();

        pProjectile->SetOwner(pOwner->GetIndex());
        pProjectile->SetWeaponOrigin(GetType());
                
        if (pProjectile->GetGravity() > 0.0f)
        {
            float fGravity(-pProjectile->GetGravity() * p_gravity);

            CVec3f v3Velocity;

            float fTime(4.0f); // 4.0s impact time

            v3Velocity = (v3TargetPos - v3Start) / fTime;
            v3Velocity.z = ((v3TargetPos.z - v3Start.z) - 0.5f * fGravity * fTime * fTime) / fTime;

            pProjectile->SetOrigin(v3Start + v3Right * v3Offset.x + v3Up * v3Offset.y + v3Forward * v3Offset.z);
            pProjectile->SetAngles(pOwner->GetAngles());
            pProjectile->SetVelocity(v3Velocity);
        }
        else
        {
            pProjectile->SetOrigin(v3TargetPos + CVec3f(0.0f, 0.0f, 3000.0f));
            pProjectile->SetAngles(CVec3f(-90.0f, 0.0f, 0.0f));
            pProjectile->SetVelocity(CVec3f(0.0f, 0.0f, -1000.0f));
        }

        pProjectile->SetOriginTime(Game.GetGameTime());
        pProjectile->SetDamage(m_pEntityConfig->GetMinDamage(), m_pEntityConfig->GetMaxDamage(), m_pEntityConfig->GetDamageRadius(),
                                m_pEntityConfig->GetPierceUnit(), m_pEntityConfig->GetPierceHellbourne(), m_pEntityConfig->GetPierceSiege(), m_pEntityConfig->GetPierceBuilding());
        pProjectile->Spawn();
    }

    return true;
}


/*====================
  ISiegeItem::CoolDown
  ====================*/
bool    ISiegeItem::CoolDown()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
    SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
    return true;
}


/*====================
  ISiegeItem::ActivatePrimary
  ====================*/
bool    ISiegeItem::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Check to see if the attack is ready yet
    if (!IsReady())
        return false;

    switch (pOwner->GetAction() & ~PLAYER_ACTION_IMMOBILE)
    {
    case PLAYER_ACTION_SKILL:
        return false;

    case PLAYER_ACTION_STUNNED:
        return false;

    case PLAYER_ACTION_IDLE:
        if (iButtonStatus & GAME_BUTTON_STATUS_UP)
            return false;
        if (m_pEntityConfig->GetSpinupTime() == 0)
        {
            if (m_pEntityConfig->GetAttackDelay() == 0)
                return Fire(iButtonStatus, pOwner->GetViewAngles());
            else
                return Delay(iButtonStatus, pOwner->GetViewAngles());
        }           
        else
            return Spinup(iButtonStatus);
        break;

    case PLAYER_ACTION_GUN_DELAY:
        return true;

    case PLAYER_ACTION_GUN_SPINUP:
        Spinup(iButtonStatus);
        return true;

    case PLAYER_ACTION_GUN_WARM:
        if (!(iButtonStatus & GAME_BUTTON_STATUS_DOWN))
            return CoolDown();
        
        if (m_pEntityConfig->GetAttackDelay() == 0)
            return Fire(iButtonStatus, pOwner->GetViewAngles());
        else
            return Delay(iButtonStatus, pOwner->GetViewAngles());

    case PLAYER_ACTION_GUN_FIRE:
        Fire(iButtonStatus, pOwner->GetViewAngles());
        return true;

    case PLAYER_ACTION_GUN_COOLDOWN:
        return false;

    case PLAYER_ACTION_GUN_CHARGE:
    
    default:
        Console.Warn << _T("ISiegeItem::ActivatePrimary() - Invalid player action for a siege item: ") << pOwner->GetAction() << newl;
        return false;
    }
}


/*====================
  ISiegeItem::FinishedAction
  ====================*/
void    ISiegeItem::FinishedAction(int iAction)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    switch (iAction & ~PLAYER_ACTION_IMMOBILE)
    {
    case PLAYER_ACTION_GUN_WARM:
        // Do nothing, this just relays to ActivatePrimary that the
        // the weapon isn't coming from an idle state
        break;

    case PLAYER_ACTION_GUN_FIRE:
        if (GetImpulseOnly())
            CoolDown();
        else
            pOwner->SetAction(PLAYER_ACTION_GUN_WARM, 0);
        break;

    case PLAYER_ACTION_GUN_SPINUP:
        pOwner->SetAction(PLAYER_ACTION_GUN_WARM, 0);
        break;

    case PLAYER_ACTION_GUN_DELAY:
        Fire(GAME_BUTTON_STATUS_PRESSED | GAME_BUTTON_STATUS_DOWN, V3_ZERO);
        break;

    default:
        pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
        break;
    }
}


/*====================
  ISiegeItem::Selected
  ====================*/
void    ISiegeItem::Selected()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;
}


/*====================
  ISiegeItem::GetTargetLocation
  ====================*/
CVec3f  ISiegeItem::GetTargetLocation() const
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return CVec3f(0.0f, 0.0f, 0.0f);

    if (pOwner->GetAction() == PLAYER_ACTION_GUN_DELAY && m_v3DelayedTarget != V3_ZERO)
        return m_v3DelayedTarget;

    return pOwner->GetTargetPosition(m_pEntityConfig->GetMaxRange(), m_pEntityConfig->GetMinRange());
}


/*====================
  ISiegeItem::IsValidTarget
  ====================*/
bool    ISiegeItem::IsValidTarget(IGameEntity *pEntity)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!pEntity->IsVisual())
        return false;

    IVisualEntity *pVisual(pEntity->GetAsVisualEnt());

    bool bValidStatus(false);
    if (pVisual->GetStatus() == ENTITY_STATUS_ACTIVE)
        bValidStatus = true;

    bool bValidTeam(false);
    if (pVisual->GetTeam() != pOwner->GetTeam())
        bValidTeam = true;

    bool bValidType(false);
    if (((pVisual->IsPlayer() && !pVisual->GetAsPlayerEnt()->GetIsVehicle()) || pVisual->IsNpc()))
        bValidType = true;
    else if (pVisual->IsPlayer() && pVisual->GetAsPlayerEnt()->GetIsVehicle())
        bValidType = true;
    else if (pVisual->IsBuilding())
        bValidType = true;
    else if (pVisual->IsGadget())
        bValidType = true;

    return bValidStatus && bValidTeam && bValidType;
}


/*====================
  ISiegeItem::ClientPrecache
  ====================*/
void    ISiegeItem::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (pConfig)
    {
        if (!pConfig->GetProjectileName().empty())
            EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetProjectileName()));

        if (!pConfig->GetTargetMaterialPath().empty())
            g_ResourceManager.Register(pConfig->GetTargetMaterialPath(), RES_MATERIAL);
    }
}
