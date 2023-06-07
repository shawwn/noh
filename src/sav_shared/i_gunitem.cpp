// (C)2006 S2 Games
// i_gunitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gunitem.h"

#include "../k2/c_camera.h"
#include "../k2/c_vid.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_host.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_eventscript.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
//=============================================================================

/*====================
  IGunItem::CEntityConfig::CEntityConfig
  ====================*/
IGunItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IInventoryItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(MinDamage, 0.0f),
INIT_ENTITY_CVAR(MaxDamage, 0.0f),
INIT_ENTITY_CVAR(PierceUnit, 1.0f),
INIT_ENTITY_CVAR(PierceHellbourne, 1.0f),
INIT_ENTITY_CVAR(PierceSiege, 1.0f),
INIT_ENTITY_CVAR(PierceBuilding, 1.0f),
INIT_ENTITY_CVAR(DamageRadius, 0.0f),
INIT_ENTITY_CVAR(SpinupTime, 0),
INIT_ENTITY_CVAR(ChargeTime, 0),
INIT_ENTITY_CVAR(MinChargeTime, 0),
INIT_ENTITY_CVAR(AttackTime, 0),
INIT_ENTITY_CVAR(FirstPersonModelPath, _T("")),
INIT_ENTITY_CVAR(FirstPersonModelOffset, V_ZERO),
INIT_ENTITY_CVAR(FirstPersonModelAngles, V_ZERO),
INIT_ENTITY_CVAR(FirstPersonModelFov, 60.f),
INIT_ENTITY_CVAR(ProjectileName, _T("")),
INIT_ENTITY_CVAR(ImpactEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactBuildingEffectPath, _T("")),
INIT_ENTITY_CVAR(ImpactTerrainEffectPath, _T("")),
INIT_ENTITY_CVAR(ThirdPersonFireAnimName, _T("")),
INIT_ENTITY_CVAR(ThirdPersonFireEffectPath, _T("")),
INIT_ENTITY_CVAR(TraceEffectPath, _T("")),
INIT_ENTITY_CVAR(NumShots, 1),
INIT_ENTITY_CVAR(SpreadX, 0.0f),
INIT_ENTITY_CVAR(SpreadY, 0.0f),
INIT_ENTITY_CVAR(Range, 5000.0f),
INIT_ENTITY_CVAR(AttackOffset, CVec3f(0.0f, 0.0f, 30.0f)),
INIT_ENTITY_CVAR(TargetState, _T("")),
INIT_ENTITY_CVAR(TargetStateDuration, 0),
INIT_ENTITY_CVAR(CanZoom, false),
INIT_ENTITY_CVAR(ZoomFov, 90.0f),
INIT_ENTITY_CVAR(ZoomTime, 750),
INIT_ENTITY_CVAR(ViewDriftX, 0.0f),
INIT_ENTITY_CVAR(ViewDriftY, 0.0f)
{
}


/*====================
  IGunItem::IGunItem
  ====================*/
IGunItem::IGunItem(CEntityConfig *pConfig) :
IInventoryItem(pConfig),
m_pEntityConfig(pConfig),

m_uiChargeStartTime(INVALID_TIME),
m_hFirstPersonModel(INVALID_RESOURCE),

m_bIsZooming(false),
m_uiZoomStartTime(INVALID_TIME),
m_v2Drift(V2_ZERO)
{
}


/*====================
  IGunItem::Spinup
  ====================*/
bool    IGunItem::Spinup(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    if (iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
        CoolDown();
    else if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
    {
        pOwner->SetAction(PLAYER_ACTION_GUN_SPINUP, Game.GetGameTime() + GetSpinupTime());

        if (pOwner->IsPlayer())
            pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("spinup"), 1.0f, GetSpinupTime());
    }

    return true;
}


/*====================
  IGunItem::Charge
  ====================*/
bool    IGunItem::Charge(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Check mana
    if (GetManaCost() > pOwner->GetMana())
        return false;

    // Check ammo
    if (GetAdjustedAmmoCount() > 0 && GetAmmo() == 0)
        return false;

    if (iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
    {
        if (m_uiChargeStartTime != INVALID_TIME && Game.GetGameTime() - m_uiChargeStartTime >= GetMinChargeTime())
        {
            Fire(iButtonStatus);
            m_uiChargeStartTime = INVALID_TIME;
            return false;
        }
            
        // Return to idle
        m_uiChargeStartTime = INVALID_TIME;
        if (pOwner->IsPlayer())
            pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("idle"));
        pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
        return false;
    }

    if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
    {
        if (pOwner->IsPlayer())
            pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("charge"));
        
        m_uiChargeStartTime = Game.GetGameTime();
        pOwner->SetAction(PLAYER_ACTION_GUN_CHARGE, Game.GetGameTime() + GetChargeTime());
    }

    return false;
}


/*====================
  IGunItem::FireProjectile
  ====================*/
IProjectile*    IGunItem::FireProjectile(const CVec3f &v3Origin, const CVec3f &v3Dir, float fCharge)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return NULL;

    // Spawn a projectile
    IGameEntity *pNewEnt(Game.AllocateEntity(GetProjectileName()));
    if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
    {
        Console.Warn << _T("Failed to spawn projectile: ") << GetProjectileName() << newl;
        return NULL;
    }
    IProjectile *pProjectile(pNewEnt->GetAsProjectile());

//  float fDamageMult(1.0f + pOwner->GetAttributeBoost(ATTRIBUTE_RANGED_DAMAGE));
    
    ushort unStateID(0);
    if (!m_pEntityConfig->GetTargetState().empty())
        unStateID = EntityRegistry.LookupID(m_pEntityConfig->GetTargetState());
    
    if (unStateID != 0)
    {
        pProjectile->SetTargetState(unStateID);
        pProjectile->SetTargetStateDuration(m_pEntityConfig->GetTargetStateDuration());
    }
    pProjectile->SetOwner(pOwner->GetIndex());
    pProjectile->SetWeaponOrigin(GetType());
    pProjectile->SetTeam(pOwner->GetTeam());
    pProjectile->SetOrigin(v3Origin);

    if (pOwner->IsPlayer())
        pProjectile->SetAngles(pOwner->GetAsPlayerEnt()->GetCameraAngles(pOwner->GetAngles()));
    else
        pProjectile->SetAngles(pOwner->GetAngles());

    pProjectile->SetVelocity(v3Dir * pProjectile->GetSpeed());
    pProjectile->SetOriginTime(Game.GetServerTime() + Game.GetServerFrameLength());
    pProjectile->SetDamageFlags(DAMAGE_FLAG_GUN);
    pProjectile->SetDamage((pOwner->GetRangedMinDamage(m_ySlot) / GetNumShots()), (pOwner->GetRangedMaxDamage(m_ySlot) / GetNumShots()), GetDamageRadius(),
                        GetPierceUnit(), GetPierceHellbourne(), GetPierceSiege(), GetPierceBuilding());
    pProjectile->ApplyCharge(fCharge);
    pProjectile->Spawn();

    return pProjectile;
}


/*====================
  IGunItem::Fire
  ====================*/
bool    IGunItem::Fire(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    // Don't interrupt other actions
    if (pOwner->GetAction() != PLAYER_ACTION_IDLE &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_WARM &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_CHARGE &&
        pOwner->GetAction() != PLAYER_ACTION_GUN_CHARGED)
        return false;

    // Make sure non-automatic items don't activate if the button is held
    if (GetImpulseOnly() &&
        !(iButtonStatus & GAME_BUTTON_STATUS_PRESSED) &&
        GetChargeTime() == 0)
        return false;

    float fManaCost(pOwner->GetGunManaCost(GetManaCost()));

    // Check mana
    if (GetManaCost() > 0.0f &&
        !pOwner->SpendMana(fManaCost))
        return false;

    // Check ammo
    if (GetAdjustedAmmoCount() > 0 &&
        !pOwner->UseAmmo(m_ySlot))
        return CoolDown();

    uint uiGameTime(Game.GetGameTime());
    uint uiAttackTime(GetAttackTime());

    if (pOwner->GetAction() == PLAYER_ACTION_GUN_WARM)
        pOwner->SetAction(PLAYER_ACTION_GUN_FIRE, (uiGameTime / uiAttackTime + 1) * uiAttackTime);
    else
        pOwner->SetAction(PLAYER_ACTION_GUN_FIRE, uiGameTime + uiAttackTime);

    // Fire the weapon
    CAxis axis(pOwner->GetAngles());

    const CVec3f &v3Forward(axis.Forward());
    const CVec3f &v3Right(axis.Right());
    const CVec3f &v3Up(axis.Up());
    const CVec3f &v3Offset(GetAttackOffset());
    CVec3f v3Start(pOwner->GetPosition() + V_UP * pOwner->GetViewHeight());

    // Third Person Animations
    if (!GetThirdPersonFireAnimName().empty())
        pOwner->StartAnimation(GetThirdPersonFireAnimName(), 1);
    
    if (!GetThirdPersonFireEffectPath().empty() && Game.IsServer())
    {
        CGameEvent evFire;
        evFire.SetSourceEntity(pOwner->GetIndex());
        evFire.SetEffect(Game.RegisterEffect(GetThirdPersonFireEffectPath()));
        evFire.SetNoFirstPerson();
        Game.AddEvent(evFire);
    }

    // First person animations
    if (pOwner->IsPlayer())
        pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("fire"));

    if (Game.IsClient())
        return true;

    uint uiChargeTime(m_uiChargeStartTime == INVALID_TIME ? 0 : MIN(Game.GetGameTime() - m_uiChargeStartTime, GetChargeTime()));
    float fCharge((GetChargeTime() > 0) ? uiChargeTime / float(GetChargeTime()) : 1.0f);
    fCharge = MIN(fCharge, 1.0f);
    bool bSuccess(true);

    // Notify all states and items that an attack is occuring
    pOwner->DoRangedAttack();

    for (uint uiShot(0); uiShot < GetNumShots(); ++uiShot)
    {
        CVec3f v3Rand;
        do
        {
            v3Rand.x = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
            v3Rand.y = M_Randnum(-0.5f, 0.5f) + M_Randnum(-0.5f, 0.5f);
            v3Rand.z = v3Rand.x * v3Rand.x + v3Rand.y * v3Rand.y;
        } while (v3Rand.z > 1.0f);

        CVec2f v2Spread(GetSpreadX(), GetSpreadY());

        CVec3f v3Dir(Normalize(v3Forward + v3Right * (v3Rand.x * v2Spread.x) + v3Up * (v3Rand.y * v2Spread.y)));
        CVec3f v3End(v3Start + v3Dir * GetRange());

        if (GetProjectileName().empty())
        {
            // Do a trace
            STraceInfo result;
            Game.TraceLine(result, v3Start, v3End, TRACE_PROJECTILE, pOwner->GetWorldIndex());
            if (result.uiEntityIndex != INVALID_INDEX)
            {
                IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
                if (pEntity != NULL)
                {
                    if (pEntity->Impact(result, pOwner))
                    {
                        float fDamage = M_Randnum(pOwner->GetRangedMinDamage(m_ySlot) / GetNumShots(), pOwner->GetRangedMaxDamage(m_ySlot) / GetNumShots());
                        ICombatEntity *pCombat(pEntity->GetAsCombatEnt());

                        //fDamage *= (1.0f + pOwner->GetAttributeBoost(ATTRIBUTE_RANGED_DAMAGE));

                        if (pEntity->IsBuilding())
                            fDamage *= GetPierceBuilding();
                        else if (pCombat != NULL && pCombat->GetIsSiege())
                            fDamage *= GetPierceSiege();
                        else if (pCombat != NULL && pCombat->GetIsHellbourne())
                            fDamage *= GetPierceHellbourne();
                        else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
                            fDamage *= GetPierceUnit();
                        
                        if (pEntity->Damage(fDamage, DAMAGE_FLAG_GUN, pOwner, GetType()) > 0)
                            pEntity->Hit(result.v3EndPos, result.plPlane.v3Normal); // FIXME: Put something meaningful here

                        if (pOwner->IsEnemy(pEntity))
                        {
                            ushort unStateID(0);
                            if (!m_pEntityConfig->GetTargetState().empty())
                                unStateID = EntityRegistry.LookupID(m_pEntityConfig->GetTargetState());
                            if (unStateID != 0)
                                pEntity->ApplyState(unStateID, Game.GetGameTime(), m_pEntityConfig->GetTargetStateDuration(), pOwner->GetIndex());
                        }
                    }
                }
            }

            if (!GetTraceEffectPath().empty() && Game.IsServer())
            {
                CGameEvent evTrace;
                evTrace.SetSourcePosition(v3Start + v3Right * v3Offset.x + v3Up * v3Offset.y + v3Forward * v3Offset.z);
                evTrace.SetTargetPosition(result.v3EndPos);
                evTrace.SetEffect(Game.RegisterEffect(GetTraceEffectPath()));
                Game.AddEvent(evTrace);
            }

            // Impact effect
            if (result.fFraction < 1.0f && result.uiSurfFlags & SURF_TERRAIN && !GetImpactTerrainEffectPath().empty() && Game.IsServer())
            {
                CGameEvent evDeath;
                evDeath.SetSourcePosition(result.v3EndPos);
                evDeath.SetEffect(Game.RegisterEffect(GetImpactTerrainEffectPath()));
                Game.AddEvent(evDeath);
            }
            else
            {
                IVisualEntity *pEntity(NULL);
                if (result.uiEntityIndex != INVALID_INDEX)
                    pEntity = Game.GetEntityFromWorldIndex(result.uiEntityIndex);
                if (result.fFraction < 1.0f && pEntity && (pEntity->IsBuilding() || pEntity->IsProp()) && !GetImpactBuildingEffectPath().empty() && Game.IsServer())
                {
                    CGameEvent evDeath;
                    evDeath.SetSourcePosition(result.v3EndPos);
                    evDeath.SetEffect(Game.RegisterEffect(GetImpactBuildingEffectPath()));
                    Game.AddEvent(evDeath);
                }
                else if (result.fFraction < 1.0f && !GetImpactEffectPath().empty() && Game.IsServer())
                {
                    CGameEvent evDeath;
                    evDeath.SetSourcePosition(result.v3EndPos);
                    evDeath.SetEffect(Game.RegisterEffect(GetImpactEffectPath()));
                    Game.AddEvent(evDeath);
                }
            }
        }
        else
        {
            bSuccess = bSuccess && (FireProjectile(v3Start + v3Right * v3Offset.x + v3Up * v3Offset.y + v3Forward * v3Offset.z, v3Dir, fCharge) != NULL);
        }
    }

    return bSuccess;
}


/*====================
  IGunItem::CoolDown
  ====================*/
bool    IGunItem::CoolDown()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

    pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
    SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
    return true;
}


/*====================
  IGunItem::ActivatePrimary
  ====================*/
bool    IGunItem::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return true;

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
        if (GetSpinupTime() > 0)
            return Spinup(iButtonStatus);
        else if (GetChargeTime() > 0)
            return Charge(iButtonStatus);
        else
            return Fire(iButtonStatus);
        break;

    case PLAYER_ACTION_GUN_SPINUP:
        return Spinup(iButtonStatus);

    case PLAYER_ACTION_GUN_WARM:
        if (!(iButtonStatus & GAME_BUTTON_STATUS_DOWN))
            return CoolDown();
        // Fall through to PLAYER_ACTION_GUN_FIRE

    case PLAYER_ACTION_GUN_FIRE:
        Fire(iButtonStatus);
        return true;

    case PLAYER_ACTION_GUN_COOLDOWN:
        return false;

    case PLAYER_ACTION_GUN_CHARGE:
    case PLAYER_ACTION_GUN_CHARGED:
        return Charge(iButtonStatus);
    
    case PLAYER_ACTION_GUN_RELOAD:

    default:
        Console.Warn << _T("IGunItem::ActivatePrimary() - Invalid player action for a gun item: ") << pOwner->GetAction() << newl;
        return false;
    }
}


/*====================
  IGunItem::ActivateSecondary
  ====================*/
bool    IGunItem::ActivateSecondary(int iButtonStatus)
{
    if (iButtonStatus & GAME_BUTTON_STATUS_DOWN)
    {
        m_bIsZooming = true;
        if (iButtonStatus & GAME_BUTTON_STATUS_PRESSED)
            m_uiZoomStartTime = Game.GetGameTime();
    }

    if (iButtonStatus & GAME_BUTTON_STATUS_UP)
    {
        m_bIsZooming = false;
        if (iButtonStatus & GAME_BUTTON_STATUS_RELEASED)
            m_uiZoomStartTime = Game.GetGameTime();
    }

    return true;
}


/*====================
  IGunItem::FinishedAction
  ====================*/
void    IGunItem::FinishedAction(int iAction)
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

    case PLAYER_ACTION_GUN_CHARGE:
        pOwner->SetAction(PLAYER_ACTION_GUN_CHARGED, 0);
        break;

    case PLAYER_ACTION_GUN_CHARGED:
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

    default:
        pOwner->SetAction(PLAYER_ACTION_IDLE, -1);
        break;
    }
}


/*====================
  IGunItem::Selected
  ====================*/
void    IGunItem::Selected()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    pOwner->SetFov(pOwner->GetBaseFov());
    pOwner->StartAnimation(_T("gun_draw"), 1);
    pOwner->SetDefaultAnimation(_T("gun_idle"));

    if (pOwner->IsPlayer())
    {
        pOwner->GetAsPlayerEnt()->SetFirstPersonModelHandle(Game.RegisterModel(GetFirstPersonModelPath()));
        pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("idle"));
    }
}


/*====================
  IGunItem::LocalClientFrame
  ====================*/
void    IGunItem::LocalClientFrame()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner || !pOwner->IsPlayer())
        return;

    IPlayerEntity *pPlayerOwner(pOwner->GetAsPlayerEnt());
    if (!pPlayerOwner)
        return;

    // These aren't transmitted, so set them manually
    pPlayerOwner->SetFirstPersonModelHandle(m_hFirstPersonModel);
    pPlayerOwner->SetFirstPersonModelOffset(GetFirstPersonModelOffset());
    pPlayerOwner->SetFirstPersonModelAngles(GetFirstPersonModelAngles());
    pPlayerOwner->SetFirstPersonModelFov(GetFirstPersonModelFov());
}


/*====================
  IGunItem::Spawn
  ====================*/
void    IGunItem::Spawn()
{
    if (Game.IsClient())
        m_hFirstPersonModel = Game.RegisterModel(GetFirstPersonModelPath());
}


/*====================
  IGunItem::GetFov
  ====================*/
float   IGunItem::GetFov() const
{
    if (!m_pEntityConfig->GetCanZoom())
        return IInventoryItem::GetFov();

    IPlayerEntity *pOwner(Game.GetPlayerEntity(GetOwner()));
    if (pOwner == NULL)
        return IInventoryItem::GetFov();

    float fZoomFov(m_pEntityConfig->GetZoomFov());

    float fZoomAmount(CLAMP((Game.GetGameTime() - m_uiZoomStartTime) / m_pEntityConfig->GetZoomTime().GetFloat(), 0.0f, 1.0f));

    if (m_bIsZooming)
        return LERP(fZoomAmount, pOwner->GetBaseFov(), fZoomFov);
    else
        return LERP(fZoomAmount, fZoomFov, pOwner->GetBaseFov());
}


/*====================
  IGunItem::ApplyDrift
  ====================*/
void    IGunItem::ApplyDrift(CVec3f &v3Angles)
{
    float fDeltaYaw(sin(Game.GetGameTime() * 0.01f) * Game.GetRand(-m_pEntityConfig->GetViewDriftX(), m_pEntityConfig->GetViewDriftX()));
    if ((fDeltaYaw < 0.0f && m_v2Drift.y > -m_pEntityConfig->GetViewDriftX() / 2.0f) ||
        (fDeltaYaw > 0.0f && m_v2Drift.y < m_pEntityConfig->GetViewDriftX() / 2.0f))
        m_v2Drift.y += fDeltaYaw;

    v3Angles[YAW] += m_v2Drift.y * MsToSec(Game.GetFrameLength());

    float fDeltaPitch(sin(Game.GetGameTime() * 0.001f) * Game.GetRand(-m_pEntityConfig->GetViewDriftY(), m_pEntityConfig->GetViewDriftY()));
    if ((fDeltaPitch < 0.0f && m_v2Drift.x > -m_pEntityConfig->GetViewDriftY() / 2.0f) ||
        (fDeltaPitch > 0.0f && m_v2Drift.x < m_pEntityConfig->GetViewDriftY() / 2.0f))
        m_v2Drift.x += fDeltaPitch;

    v3Angles[PITCH] += m_v2Drift.x * MsToSec(Game.GetFrameLength());
}


/*====================
  IGunItem::DoRangedAttack
  ====================*/
void    IGunItem::DoRangedAttack()
{
    if (GetOwnerEnt() == NULL)
        return;

    IInventoryItem *pItem(GetOwnerEnt()->GetCurrentItem());

    if (pItem == NULL || pItem == this || !pItem->IsGun())
        return;

    IGunItem *pGun(pItem->GetAsGun());

    if (m_uiStartCooldown < Game.GetGameTime() && m_uiCooldownDuration != INVALID_TIME)
    {
        uint uiFinishTime(m_uiStartCooldown + m_uiCooldownDuration);
        if (uiFinishTime > Game.GetGameTime() && uiFinishTime - Game.GetGameTime() > pGun->GetCooldownTime() + pGun->GetAttackTime())
            return;
    }

    SetCooldownTimer(Game.GetGameTime(), pGun->GetAttackTime() + pGun->GetCooldownTime());
}


/*====================
  IGunItem::ClientPrecache
  ====================*/
void    IGunItem::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetFirstPersonModelPath().empty())
    {
        ResHandle hModel(g_ResourceManager.Register(pConfig->GetFirstPersonModelPath(), RES_MODEL));
        
        if (hModel != INVALID_RESOURCE)
            g_ResourceManager.PrecacheSkin(hModel, -1);
    }

    if (!pConfig->GetProjectileName().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetProjectileName()));

    if (!pConfig->GetImpactEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT);

    if (!pConfig->GetImpactBuildingEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetImpactBuildingEffectPath(), RES_EFFECT);

    if (!pConfig->GetImpactTerrainEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetImpactTerrainEffectPath(), RES_EFFECT);

    if (!pConfig->GetThirdPersonFireEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetThirdPersonFireEffectPath(), RES_EFFECT);

    if (!pConfig->GetTraceEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetTraceEffectPath(), RES_EFFECT);

    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));
}


/*====================
  IGunItem::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void    IGunItem::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetFirstPersonModelPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetFirstPersonModelPath(), RES_MODEL, RES_MODEL_SERVER));

    if (!pConfig->GetProjectileName().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetProjectileName()));

    if (!pConfig->GetImpactEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetImpactBuildingEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactBuildingEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetImpactTerrainEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetImpactTerrainEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetThirdPersonFireEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetThirdPersonFireEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetTraceEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTraceEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetTargetState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetTargetState()));
}
