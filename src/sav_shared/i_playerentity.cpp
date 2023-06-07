// (C)2006 S2 Games
// i_playerentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_playerentity.h"
#include "c_meleeattackevent.h"
#include "c_teaminfo.h"
#include "c_teamdefinition.h"
#include "c_entitysoul.h"
#include "c_playercommander.h"
#include "i_petentity.h"
#include "c_entityclientinfo.h"
#include "i_skilltoggle.h"
#include "c_spellpolymorph.h"

#include "../k2/c_clientsnapshot.h"
#include "../k2/c_world.h"
#include "../k2/c_xmlmanager.h"
#include "../k2/c_camera.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_host.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_worldentity.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_effect.h"
#include "../k2/c_texture.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_model.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_BOOL(g_meleeNewBlock);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*     IPlayerEntity::s_pvFields;

CVAR_FLOATF(    p_speed,                    1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedBackwards,           0.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedStrafe,              1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedSprint,              2.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedSprintStrafe,        1.75f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedSprintBackwards,     1.25f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedBlocking,            0.33f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedMeleeAttack,         1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_speedMeleeAttackTime,     2000,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedExhausted,           1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_speedLimit,               700.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaSprintCost,        15.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaJumpCost,          40.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaRegenResting,      3.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaRegenIdle,         8.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaRegenWalking,      0.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaRegenExhausted,    0.25f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_restTime,                 5000,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_jump,                     200.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_jumpLimiter,              1.25f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_BOOLF(     p_jumpStaminaRequired,      false,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_BOOLF(     p_jumpAppliesSpeedModifiers,false,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_jumpSlopeScale,           0.25f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_gravity,                  500.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_stopSpeed,                0.333f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_groundFriction,           5.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_slopeFriction,            500.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_corpseFriction,           1000.0f,    CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_groundAccelerate,         10.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_airAccelerate,            1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_waterAccelerate,          5.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_stepHeight,               32.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_maxWalkSlope,             0.40f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_groundEpsilon,            0.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_landVelocity,             300.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_BOOLF(     p_stepDown,                 true,       CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_BOOLF(     p_debugMovement,            false,      CONEL_DEV);
CVAR_BOOLF(     p_debugSlide,               false,      CONEL_DEV);
CVAR_BOOLF(     p_debugStep,                false,      CONEL_DEV);
CVAR_BOOLF(     p_debugViewHeight,          false,      CONEL_DEV);
CVAR_FLOAT(     p_yawStrafeAngle,           75.0f);

CVAR_FLOATF(    p_meleeMinPitch,            -70.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_meleeMaxPitch,            0.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_siegeMinPitch,            90.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_siegeMaxPitch,            -10.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_siegeYaw,                 7.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_vehicleMeleeMinPitch,     -70.0f,     CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_vehicleMeleeMaxPitch,     0.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_vehicleMeleeYaw,          0.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    p_bobSpeed,                 15.0f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmount,                1.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmountUp,              2.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmountRight,           1.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmountRoll,            0.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmountPitch,           0.0f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAmountStrafe,          0.5f,       CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobPosHalfLife,           0.05f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_bobAngleHalfLife,         0.05f,      CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    p_dashSpeed,                2.0f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_dashMaxTime,              2000,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_dashCooldownTime,         30000,          CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_BOOLF(     p_dashRequireStamina,       false,          CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_dashActivateStaminaCost,  0,              CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_dashStaminaCost,          10,             CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    g_expRewardExponent,        0.5f,           CVAR_GAMECONFIG);

CVAR_UINTF(     g_corpseTime,               SecToMs(20u),   CVAR_GAMECONFIG);
CVAR_UINTF(     g_deathTime,                SecToMs(30u),   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     g_spawnInvulnerableTime,    SecToMs(3u),    CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     g_minDeathTime,             800,            CVAR_GAMECONFIG);

CVAR_FLOATF(    g_taxRate,                  0.1f,           CVAR_GAMECONFIG);

CVAR_FLOATF(    g_officerAuraRadius,        500.0f,         CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     g_officerAuraFadeTime,      SecToMs(3u),    CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    g_hoverEntityRange,         5000.0f,        CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_INTF(      g_hoverEntityDisplayTime,   2000,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_INTF(      g_hoverEntityAcquireTime,   500,            CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    g_altInfoRange,             7500.0f,        CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF(     g_respawnIncreaseInterval,  30,             CVAR_GAMECONFIG);
CVAR_UINTF(     g_respawnOfficerBonus,      5000,           CVAR_GAMECONFIG);

CVAR_UINTF(     g_loadoutTime,              SecToMs(10u),   CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOAT(     cam_yaw,                    0.0f);
CVAR_BOOL(      cam_detach,                 false);

CVAR_FLOATF(    g_minimapOfficerIconSize,   8.0f,           CVAR_GAMECONFIG);

ResHandle IPlayerEntity::s_hOfficerStarIcon(INVALID_RESOURCE);
//=============================================================================

/*====================
  IPlayerEntity::CEntityConfig::CEntityConfig
  ====================*/
IPlayerEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
ICombatEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(CanEnterLoadout, true),
INIT_ENTITY_CVAR(SoulCost, 0),
INIT_ENTITY_CVAR(CamHeight, 50.0f),
INIT_ENTITY_CVAR(CamDistance, 150.0f),
INIT_ENTITY_CVAR(CamPitch, 0.0f),
INIT_ENTITY_CVAR(YawStrafe, true),
INIT_ENTITY_CVAR(IsVehicle, false),
INIT_ENTITY_CVAR(VehicleTurnSpeed, 90.0f),
INIT_ENTITY_CVAR(CanPurchase, true),
INIT_ENTITY_CVAR(SpawnEffectPath, _T("")),
INIT_ENTITY_CVAR(PainEffectPath, _T("")),
INIT_ENTITY_CVAR(ApplyAttributes, true),
INIT_ENTITY_CVAR(CanDash, true),
INIT_ENTITY_CVAR(ShowResurrectable, false),
INIT_ENTITY_CVAR(TransparentAngle, 0.0f)
{
}


/*====================
  IPlayerEntity::~IPlayerEntity
  ====================*/
IPlayerEntity::~IPlayerEntity()
{
    if (m_uiPetIndex != INVALID_INDEX && Game.IsServer())
    {
        IGameEntity *pPet(Game.GetEntity(m_uiPetIndex));
        if (pPet && pPet->IsPet())
            pPet->GetAsPet()->Kill();

        m_uiPetIndex = INVALID_INDEX;
    }

    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }

    SAFE_DELETE(m_pFirstPersonSkeleton);
}


/*====================
  IPlayerEntity::IPlayerEntity
  ====================*/
IPlayerEntity::IPlayerEntity(CEntityConfig *pConfig) :
ICombatEntity(pConfig),
m_pEntityConfig(pConfig),

m_iClientNum(-1),

m_yFirstPersonAnim(0),
m_yFirstPersonAnimSequence(0),
m_hFirstPersonModel(INVALID_RESOURCE),
m_v3FirstPersonModelOffset(V3_ZERO),
m_pFirstPersonSkeleton(NULL),

m_uiDeathTime(0),

m_uiPreviewBuildingIndex(INVALID_INDEX),
m_v2Cursor(V2_ZERO),

m_iCurrentMovement(PLAYER_MOVE_IDLE),
m_iMoveFlags(0),
m_uiWheelTime(0),
m_uiDashStartTime(0),
m_uiLastDashTime(INVALID_TIME),

m_uiPetIndex(INVALID_INDEX),

m_yCurrentOrder(CMDR_ORDER_CLEAR),
m_v3CurrentOrderPos(V3_ZERO),
m_uiCurrentOrderEntIndex(INVALID_INDEX),
m_yOrderUniqueID(-1),
m_yLastAcknowledgedOfficerOrder(-1),

m_fBaseYaw(0.0f),
m_fLastYaw(0.0f),
m_fCurrentYaw(0.0f),
m_uiTurnStartTime(0),
m_iTurnAction(PLAYER_MOVE_IDLE),
m_fYawLerpTime(0.0f),
m_fTiltPitch(0.0f),
m_fTiltRoll(0.0f),
m_fTiltHeight(0.0f),

m_fBobTime(0.0f),
m_fBobCycle(0.0f),
m_fBobHalfCycle(0.0f),
m_v3BobPos(0.0f, 0.0f, 0.0f),
m_v3BobAngles(0.0f, 0.0f, 0.0f),

m_uiSpawnLocation(INVALID_INDEX),
m_uiNextPainTime(0),
m_iDashStateSlot(-1),
m_yKillStreak(0)
{
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_ayTurnSequence[i] = 0;
}


/*====================
  IPlayerEntity::GetAltInfoName
  ====================*/
const tstring&  IPlayerEntity::GetAltInfoName() const
{
    if (IsDisguised())
    {
        CEntityClientInfo *pClient(Game.GetClientInfo(GetDisguiseClient()));
        if (pClient != NULL)
            return pClient->GetName();
    }

    return GetClientName();
}


/*====================
  IPlayerEntity::GetClientName
  ====================*/
const tstring&  IPlayerEntity::GetClientName() const
{
    const static tstring s_UnknownClientName(_T("<unknown>"));

    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient == NULL)
        return s_UnknownClientName;
    
    return pClient->GetName();
}


/*====================
  IPlayerEntity::GetGameTip
  ====================*/
bool    IPlayerEntity::GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage)
{
    if (pPlayer == NULL)
        return false;

    if (GetTeam() != pPlayer->GetTeam())
        return false;

    if (GetIsVehicle() &&
        GetHealth() < GetMaxHealth() &&
        pPlayer->GetCanBuild() &&
        GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        sMessage = _T("This vehicle is damaged.  Hold down the 'R' key to repair it.");
        return true;
    }

    return IVisualEntity::GetGameTip(pPlayer, sMessage);
}


/*====================
  IPlayerEntity::GetAttributeBoost
  ====================*/
float   IPlayerEntity::GetAttributeBoost(int iAttribute) const
{
    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient == NULL)
        return 0.0f;

    return pClient->GetAttributeBoost(iAttribute);
}


/*====================
  IPlayerEntity::GetAttributeBoostDescription
  ====================*/
tstring IPlayerEntity::GetAttributeBoostDescription(int iAttribute) const
{
/*  switch (iAttribute)
    {
    case ATTRIBUTE_MAX_HEALTH: return XtoA(GetAttributeBoost(iAttribute) * GetBaseMaxHealth(), 0, 0, 0);
    case ATTRIBUTE_MAX_MANA: return XtoA(GetAttributeBoost(iAttribute) * GetBaseMaxMana(), 0, 0, 0);
    case ATTRIBUTE_MAX_STAMINA: return XtoA(GetAttributeBoost(iAttribute) * GetBaseMaxStamina(), 0, 0, 0);
    case ATTRIBUTE_HEALTH_REGEN: return XtoA(GetAttributeBoost(iAttribute) * GetBaseHealthRegen(), 0, 0, 1);
    case ATTRIBUTE_MANA_REGEN: return XtoA(GetAttributeBoost(iAttribute) * GetBaseManaRegen(), 0, 0, 1);
    case ATTRIBUTE_STAMINA_REGEN: return XtoA(GetAttributeBoost(iAttribute) * p_staminaRegenIdle, 0, 0, 1);
    case ATTRIBUTE_ARMOR: return XtoA(GetAttributeBoost(iAttribute) * GetBaseArmor(), 0, 0, 2);
    case ATTRIBUTE_MELEE_DAMAGE: return _T("-");
    case ATTRIBUTE_ATTACK_SPEED: return _T("-");
    case ATTRIBUTE_RANGED_DAMAGE: return _T("-");
    }*/

    //return _T("???");

    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient == NULL)
        return _T("0.0");

    return XtoA(pClient->GetAttributeBoostIncrease(iAttribute) * 100.0f, 0, 0, 1);
}


/*====================
  IPlayerEntity::GetStaminaRegen
  ====================*/
float   IPlayerEntity::GetStaminaRegen(int iMoveState) const
{
    float fBaseRate(GetBaseStaminaRegen());

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        if (iMoveState & (PLAYER_MOVE_SPRINT | PLAYER_MOVE_DASH))
            return 0.0f;
        
        if ((iMoveState & PLAYER_MOVE_NO_FLAGS) == PLAYER_MOVE_IDLE)
        {
            if (IsExhausted())
                fBaseRate *= p_staminaRegenExhausted;
            else if (iMoveState & PLAYER_MOVE_RESTING)
                fBaseRate *= p_staminaRegenResting;
        }
        else
        {
            fBaseRate *= p_staminaRegenWalking;
        }
    }

    FloatMod modRegen;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modRegen += m_apState[i]->GetStaminaRegenMod();
    }

    return modRegen.Modify(fBaseRate * (1.0f + GetAttributeBoost(ATTRIBUTE_AGILITY)));
}


/*====================
  IPlayerEntity::Baseline
  ====================*/
void    IPlayerEntity::Baseline()
{
    ICombatEntity::Baseline();

    m_fFov = 90.0f;

    m_iCurrentAction = PLAYER_ACTION_IDLE;
    m_uiCurrentActionEndTime = INVALID_TIME;
    m_uiLastActionTime = 0;
    m_uiLastMeleeAttackTime = 0;
    m_uiLastMeleeAttackLength = 0;

    m_uiTargetIndex = INVALID_INDEX;

    m_yDefaultAnim = 0;
    m_v3Velocity = V3_ZERO;

    // Player specific info
    m_yFirstPersonAnim = 0;
    m_yFirstPersonAnimSequence = 0;
    m_fFirstPersonAnimSpeed = 1.0f;

    m_iClientNum = -1;

    m_iMoveFlags = 0;
    
    m_iCurrentMovement = 0;

    m_uiDeathTime = 0;
    m_uiWheelTime = 0;

    m_uiPreviewBuildingIndex = INVALID_INDEX;

    m_yOrderUniqueID = -1;
    m_yCurrentOrder = CMDR_ORDER_CLEAR;
    m_uiCurrentOrderEntIndex = INVALID_INDEX;
    m_v3CurrentOrderPos = V3_ZERO;
    
    m_yOfficerOrder = OFFICERCMD_INVALID;
    m_yOfficerOrderSequence = 0;
    m_uiOfficerOrderEntIndex = INVALID_INDEX;
    m_v3OfficerOrderPos = V3_ZERO;

    m_uiPetIndex = INVALID_INDEX;
    m_uiLastDashTime = INVALID_TIME;
    m_uiDashStartTime = 0;

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    m_yAttackSequence = 0;
}


/*====================
  IPlayerEntity::GetSnapshot
  ====================*/
void    IPlayerEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
    // Base entity info
    ICombatEntity::GetSnapshot(snapshot);

    snapshot.AddField(m_yDefaultAnim);
    snapshot.AddField(m_v3Velocity);

    snapshot.AddField(m_iCurrentAction);
    snapshot.AddField(m_uiCurrentActionEndTime);
    snapshot.AddField(m_uiLastActionTime);
    snapshot.AddField(m_uiLastMeleeAttackTime);
    snapshot.AddField(m_uiLastMeleeAttackLength);
    snapshot.AddField(m_yAttackSequence);
    snapshot.AddField(m_fFov);
    snapshot.AddField(m_uiTargetIndex);
    snapshot.AddField(m_uiLastDashTime);
    snapshot.AddField(m_uiDashStartTime);

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        snapshot.AddField(m_auiAnimLockTime[i]);

    // Player specific info
    snapshot.AddField(m_yFirstPersonAnim);
    snapshot.AddField(m_yFirstPersonAnimSequence);
    snapshot.AddField(m_fFirstPersonAnimSpeed);

    snapshot.AddField(m_iClientNum);

    snapshot.AddField(m_iMoveFlags);
    
    snapshot.AddField(m_iCurrentMovement);

    snapshot.AddField(m_uiDeathTime);
    snapshot.AddField(m_uiWheelTime);

    snapshot.AddGameIndex(m_uiPreviewBuildingIndex);

    snapshot.AddField(m_yOrderUniqueID);
    snapshot.AddField(m_yCurrentOrder);
    snapshot.AddGameIndex(m_uiCurrentOrderEntIndex);
    snapshot.AddField(m_v3CurrentOrderPos);

    snapshot.AddField(m_yOfficerOrder);
    snapshot.AddField(m_yOfficerOrderSequence);
    snapshot.AddGameIndex(m_uiOfficerOrderEntIndex);
    snapshot.AddField(m_v3OfficerOrderPos);

    snapshot.AddGameIndex(m_uiPetIndex);
}


/*====================
  IPlayerEntity::ReadSnapshot
  ====================*/
bool    IPlayerEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        // Base entity info
        if (!ICombatEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextField(m_yDefaultAnim);
        snapshot.ReadNextField(m_v3Velocity);

        snapshot.ReadNextField(m_iCurrentAction);
        snapshot.ReadNextField(m_uiCurrentActionEndTime);
        snapshot.ReadNextField(m_uiLastActionTime);
        snapshot.ReadNextField(m_uiLastMeleeAttackTime);
        snapshot.ReadNextField(m_uiLastMeleeAttackLength);
        snapshot.ReadNextField(m_yAttackSequence);
        snapshot.ReadNextField(m_fFov);
        snapshot.ReadNextField(m_uiTargetIndex);
        snapshot.ReadNextField(m_uiLastDashTime);
        snapshot.ReadNextField(m_uiDashStartTime);

        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
            snapshot.ReadNextField(m_auiAnimLockTime[i]);

        // Player specific info
        snapshot.ReadNextField(m_yFirstPersonAnim);
        snapshot.ReadNextField(m_yFirstPersonAnimSequence);
        snapshot.ReadNextField(m_fFirstPersonAnimSpeed);

        snapshot.ReadNextField(m_iClientNum);
        
        snapshot.ReadNextField(m_iMoveFlags);

        snapshot.ReadNextField(m_iCurrentMovement);

        snapshot.ReadNextField(m_uiDeathTime);
        snapshot.ReadNextField(m_uiWheelTime);

        snapshot.ReadNextGameIndex(m_uiPreviewBuildingIndex);

        snapshot.ReadNextField(m_yOrderUniqueID);
        snapshot.ReadNextField(m_yCurrentOrder);
        snapshot.ReadNextGameIndex(m_uiCurrentOrderEntIndex);
        snapshot.ReadNextField(m_v3CurrentOrderPos);

        snapshot.ReadNextField(m_yOfficerOrder);
        snapshot.ReadNextField(m_yOfficerOrderSequence);
        snapshot.ReadNextGameIndex(m_uiOfficerOrderEntIndex);
        snapshot.ReadNextField(m_v3OfficerOrderPos);

        snapshot.ReadNextGameIndex(m_uiPetIndex);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPlayerEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IPlayerEntity::GetTypeVector
  ====================*/
const vector<SDataField>&   IPlayerEntity::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(ICombatEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_yDefaultAnim"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_v3Velocity"), FIELD_PRIVATE, TYPE_V3F));

        s_pvFields->push_back(SDataField(_T("m_iCurrentAction"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiCurrentActionEndTime"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiLastActionTime"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiLastMeleeAttackTime"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiLastMeleeAttackLength"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_yAttackSequence"), FIELD_PRIVATE, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_fFov"), FIELD_PRIVATE, TYPE_FLOAT));
        s_pvFields->push_back(SDataField(_T("m_uiTargetIndex"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiLastDashTime"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiDashStartTime"), FIELD_PRIVATE, TYPE_INT));
        
        for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
            s_pvFields->push_back(SDataField(_T("m_auiAnimLockTime[") + XtoA(i) + _T("]"), FIELD_PRIVATE, TYPE_INT));

        s_pvFields->push_back(SDataField(_T("m_yFirstPersonAnim"), FIELD_PRIVATE, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_yFirstPersonAnimSequence"), FIELD_PRIVATE, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_fFirstPersonAnimSpeed"), FIELD_PRIVATE, TYPE_FLOAT));

        s_pvFields->push_back(SDataField(_T("m_iClientNum"), FIELD_PUBLIC, TYPE_INT));

        s_pvFields->push_back(SDataField(_T("m_iMoveFlags"), FIELD_PRIVATE, TYPE_INT));

        s_pvFields->push_back(SDataField(_T("m_iCurrentMovement"), FIELD_PUBLIC, TYPE_INT));

        s_pvFields->push_back(SDataField(_T("m_uiDeathTime"), FIELD_PRIVATE, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_uiWheelTime"), FIELD_PUBLIC, TYPE_INT));

        s_pvFields->push_back(SDataField(_T("m_uiPreviewBuildingIndex"), FIELD_PRIVATE, TYPE_GAMEINDEX));

        s_pvFields->push_back(SDataField(_T("m_yOrderUniqueID"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_yCurrentOrder"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_uiCurrentOrderEntIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
        s_pvFields->push_back(SDataField(_T("m_v3CurrentOrderPos"), FIELD_PUBLIC, TYPE_V3F));

        s_pvFields->push_back(SDataField(_T("m_yOfficerOrder"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_yOfficerOrderSequence"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_uiOfficerOrderEntIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
        s_pvFields->push_back(SDataField(_T("m_v3OfficerOrderPos"), FIELD_PUBLIC, TYPE_V3F));
        
        s_pvFields->push_back(SDataField(_T("m_uiPetIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
    }

    return *s_pvFields;
}


/*====================
  IPlayerEntity::ReadClientSnapshot
  ====================*/
void    IPlayerEntity::ReadClientSnapshot(const CClientSnapshot &snapshot)
{
    try
    {
        if (Game.GetGamePhase() == GAME_PHASE_ENDED)
            return;

        IInventoryItem *pPrevItem(GetCurrentItem());

        // Store the current target (for snapcasting)
        m_uiSpellTargetIndex = snapshot.GetSelectedEntity();
        m_v2Cursor = snapshot.GetCursorPosition();

        Move(snapshot);

        // Inventory switching
        int iRequestedSlot(snapshot.GetSelectedItem());
        if (m_ySelectedItem != iRequestedSlot &&
            GetItem(iRequestedSlot) != NULL &&
            !GetItem(iRequestedSlot)->IsDisabled() &&
            !HasNetFlags(ENT_NET_FLAG_CHANNELING) &&
            IsIdle())
        {
            IInventoryItem *pItem(GetItem(iRequestedSlot));

            //Console.Dev << _T("Selected inventory slot #") << iRequestedSlot << newl;
            m_ySelectedItem = iRequestedSlot;

            if (pPrevItem != NULL)
                pPrevItem->Unselected();
            if (pItem != NULL)
                pItem->Selected();
        }

        // Building rotation
        IBuildingEntity *pBuilding(Game.GetBuildingEntity(GetPreviewBuildingIndex()));
        if (pBuilding != NULL)
            pBuilding->SetAngles(snapshot.GetAngles());

        SetBaseFov(snapshot.GetFov());
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPlayerEntity::ReadClientSnapshot() - "));
    }
}


/*====================
  IPlayerEntity::IsOfficer
  ====================*/
bool    IPlayerEntity::IsOfficer() const
{
    CEntityClientInfo *pClientInfo(Game.GetClientInfo(GetClientID()));
    if (pClientInfo == NULL)
        return false;
    return pClientInfo->HasFlags(CLIENT_INFO_IS_OFFICER);
}


/*====================
  IPlayerEntity::Spawn
  ====================*/
void    IPlayerEntity::Spawn()
{
    ICombatEntity::Spawn();

    m_uiGroundEntityIndex = INVALID_INDEX;
    m_bOnGround = false;

    SetStatus(ENTITY_STATUS_DORMANT);

    SetModelHandle(Game.RegisterModel(GetModelPath()));
    m_bbBounds.SetCylinder(m_pEntityConfig->GetBoundsRadius(), m_pEntityConfig->GetBoundsHeight());
    m_fScale = m_pEntityConfig->GetScale();

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    StartAnimation(_T("idle"), -1);
    m_yDefaultAnim = m_ayAnim[0];

    StartFirstPersonAnimation(_T("idle"));

    m_v3Velocity = V3_ZERO;
    
    bool bKilled(HasNetFlags(ENT_NET_FLAG_KILLED));
    ClearNetFlags();
    if (bKilled)
        SetNetFlags(ENT_NET_FLAG_KILLED);

    ClearLocalFlags();

    m_uiWheelTime = 0;

    m_yOfficerOrder = OFFICERCMD_INVALID;
    m_yOfficerOrderSequence = 0;
    m_uiOfficerOrderEntIndex = INVALID_INDEX;
    m_v3OfficerOrderPos = CVec3f(0.0f, 0.0f, 0.0f);

    m_uiPetIndex = INVALID_INDEX;
    
    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    if (Game.IsClient() && s_hOfficerStarIcon == INVALID_RESOURCE)
        s_hOfficerStarIcon = g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/officer_star.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_fTiltPitch = 0.0f;
    m_fTiltRoll = 0.0f;
    m_fTiltHeight = 0.0f;

    if (Game.IsClient())
        return;

    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
    if (pTeamInfo == NULL)
    {
        Console.Err << _T("IPlayerEntity::Spawn() - Invalid team: ") << int(m_iTeam) << newl;
        return;
    }

    // Fill inventory
    GiveItem(0, EntityRegistry.LookupID(m_pEntityConfig->GetInventory0()));
    GiveItem(1, EntityRegistry.LookupID(m_pEntityConfig->GetInventory1()));
    GiveItem(2, EntityRegistry.LookupID(m_pEntityConfig->GetInventory2()));
    GiveItem(3, EntityRegistry.LookupID(m_pEntityConfig->GetInventory3()));
    GiveItem(4, EntityRegistry.LookupID(m_pEntityConfig->GetInventory4()));
    GiveItem(5, EntityRegistry.LookupID(m_pEntityConfig->GetInventory5()));
    GiveItem(6, EntityRegistry.LookupID(m_pEntityConfig->GetInventory6()));
    GiveItem(7, EntityRegistry.LookupID(m_pEntityConfig->GetInventory7()));
    GiveItem(8, EntityRegistry.LookupID(m_pEntityConfig->GetInventory8()));
    GiveItem(9, EntityRegistry.LookupID(m_pEntityConfig->GetInventory9()));

    if (IsOfficer())
    {
        GiveItem(INVENTORY_OFFICERCMD_ATTACK, OfficerCommand_Attack2);
        GiveItem(INVENTORY_OFFICERCMD_FOLLOW, OfficerCommand_Follow2);
        GiveItem(INVENTORY_OFFICERCMD_MOVE, OfficerCommand_Move2);
        //GiveItem(INVENTORY_OFFICERCMD_DEFEND, OfficerCommand_Defend);
        if (pTeamInfo != NULL)
        {
            ushort unSkill(EntityRegistry.LookupID(pTeamInfo->GetDefinition()->GetOfficerSkill()));
            if (unSkill != INVALID_ENT_TYPE)
                GiveItem(9, unSkill);
        }
    }
}


/*====================
  IPlayerEntity::GetDashRemaining
  ====================*/
float   IPlayerEntity::GetDashRemaining() const
{
    return 1.0f - CLAMP((Game.GetGameTime() - m_uiDashStartTime) / p_dashMaxTime.GetFloat(), 0.0f, 1.0f);
}


/*====================
  IPlayerEntity::GetDashCooldownPercent
  ====================*/
float   IPlayerEntity::GetDashCooldownPercent() const
{
    if (GetMoveFlags() & PLAYER_MOVE_DASH)
        return 1.0f;
    if (m_uiLastDashTime == INVALID_TIME)
        return 0.0f;

    return 1.0f - CLAMP((Game.GetGameTime() - m_uiLastDashTime) / p_dashCooldownTime.GetFloat(), 0.0f, 1.0f);
}


/*====================
  IPlayerEntity::DrawViewBox
  ====================*/
void    IPlayerEntity::DrawViewBox(CUITrigger &minimap, CCamera &camera)
{
    CBufferFixed<48> buffer;

    CVec3f v3Start(camera.GetOrigin());
    CVec3f v3Left(M_PointOnLine(v3Start, camera.ConstructRay2(-1.0f, 0.0f), 5000.0f));
    CVec3f v3Right(M_PointOnLine(v3Start, camera.ConstructRay2(1.0f, 0.0f), 5000.0f));

    CPlane plPlanes[4] =
    {
        CPlane(1.0f, 0.0f, 0.0, Game.GetWorldWidth()),
        CPlane(0.0f, 1.0f, 0.0, Game.GetWorldHeight()),
        CPlane(-1.0f, 0.0f, 0.0, 0.0f),
        CPlane(0.0f, -1.0f, 0.0, 0.0f)
    };

    {
        CVec3f  p1(v3Start);
        CVec3f  p2(v3Left);
        bool    bValid(true);

        for (int i(0); i < 4 && bValid; ++i)
        {
            if (!M_ClipLine(plPlanes[i], p1, p2))
                bValid = false;
        }

        if (bValid)
        {
            buffer.Clear();
            buffer
                << p1.x / Game.GetWorldWidth() << 1.0f - p1.y / Game.GetWorldHeight()
                << p2.x / Game.GetWorldWidth() << 1.0f - p2.y / Game.GetWorldHeight()
                << 1.0f << 1.0f << 1.0f << 1.0f
                << 1.0f << 1.0f << 1.0f << 1.0f;
            minimap.Execute(_T("line"), buffer);
        }
    }

    {
        CVec3f  p1(v3Start);
        CVec3f  p2(v3Right);
        bool    bValid(true);

        for (int i(0); i < 4 && bValid; ++i)
        {
            if (!M_ClipLine(plPlanes[i], p1, p2))
                bValid = false;
        }

        if (bValid)
        {
            buffer.Clear();
            buffer
                << p1.x / Game.GetWorldWidth() << 1.0f - p1.y / Game.GetWorldHeight()
                << p2.x / Game.GetWorldWidth() << 1.0f - p2.y / Game.GetWorldHeight()
                << 1.0f << 1.0f << 1.0f << 1.0f
                << 1.0f << 1.0f << 1.0f << 1.0f;
            minimap.Execute(_T("line"), buffer);
        }
    }

    {
        CVec3f  p1(v3Right);
        CVec3f  p2(v3Left);
        bool    bValid(true);

        for (int i(0); i < 4 && bValid; ++i)
        {
            if (!M_ClipLine(plPlanes[i], p1, p2))
                bValid = false;
        }

        if (bValid)
        {
            buffer.Clear();
            buffer
                << p1.x / Game.GetWorldWidth() << 1.0f - p1.y / Game.GetWorldHeight()
                << p2.x / Game.GetWorldWidth() << 1.0f - p2.y / Game.GetWorldHeight()
                << 1.0f << 1.0f << 1.0f << 1.0f
                << 1.0f << 1.0f << 1.0f << 1.0f;
            minimap.Execute(_T("line"), buffer);
        }
    }
}


/*====================
  IPlayerEntity::Spawn2
  ====================*/
void    IPlayerEntity::Spawn2()
{
    CEntityClientInfo *pClient(Game.GetClientInfo(GetClientID()));

    if (pClient != NULL && pClient->GetLoadoutTime() > Game.GetGameTime() && !HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
        return;
    if (pClient != NULL && pClient->HasNetFlags(ENT_NET_FLAG_QUEUED))
        return;

    m_uiWheelTime = 0;

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    StartAnimation(_T("idle"), -1);
    m_yDefaultAnim = m_ayAnim[0];

    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
    if (pTeamInfo == NULL)
    {
        Console.Err << _T("IPlayerEntity::Spawn() - Invalid team: ") << int(m_iTeam) << newl;
        return;
    }

    if ((pTeamInfo->GetNumSpawnBuildings(this) > 1 || Game.GetGamePhase() == GAME_PHASE_WARMUP) && !HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
    {
        SetStatus(ENTITY_STATUS_SPAWNING);
        return;
    }

    if (m_uiSpawnLocation == INVALID_INDEX || HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
        m_uiSpawnLocation = pTeamInfo->GetBaseBuildingIndex();
    Spawn3();
}


/*====================
  IPlayerEntity::Spawn3
  ====================*/
void    IPlayerEntity::Spawn3()
{
    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));

    if (pTeamInfo == NULL)
    {
        Console.Err << _T("IPlayerEntity::Spawn3() - Invalid team: ") << int(m_iTeam) << newl;
        return;
    }

    // Replenish ammo and check for availability of a weapon
    for (int i(0); i < INVENTORY_START_BACKPACK; ++i)
    {
        if (m_apInventory[i] == NULL)
            continue;

        if (m_apInventory[i]->GetPrerequisite().empty() || pTeamInfo->HasBuilding(m_apInventory[i]->GetPrerequisite()) || Game.GetGamePhase() == GAME_PHASE_WARMUP)
        {
            m_apInventory[i]->Enable();
            m_apInventory[i]->SetAmmo(m_apInventory[i]->GetAdjustedAmmoCount());
        }
    }

    // Track player's health
    float fHealthPercent(GetHealthPercent());

    // Clear old states
    ClearStates();

    // Apply passive states from inventory
    for (int i = 0; i < MAX_INVENTORY; ++i)
    {
        IInventoryItem *pItem = GetItem(i);
        
        if (pItem != NULL)
            pItem->ActivatePassive();
    }

    m_fHealth = GetMaxHealth() * fHealthPercent;
    m_fMana = GetMaxMana();
    m_fStamina = GetMaxStamina();

    // Determine spawn point
    m_v3Position = GetSpawnLocation(m_uiSpawnLocation);
    m_v3Velocity.Clear();
    SetStatus(ENTITY_STATUS_ACTIVE);
    Link();

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    StartAnimation(_T("idle"), -1);
    m_yDefaultAnim = m_ayAnim[0];

    m_uiSpawnLocation = INVALID_INDEX;

    // Spawn effect
    if (!m_pEntityConfig->GetSpawnEffectPath().empty())
    {
        CGameEvent evSpawn;
        evSpawn.SetSourceEntity(GetIndex());
        evSpawn.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetSpawnEffectPath()));
        Game.AddEvent(evSpawn);
    }

    if (HasNetFlags(ENT_NET_FLAG_KILLED))
    {
        ApplyState(State_Spawned, Game.GetGameTime(), g_spawnInvulnerableTime);
        RemoveNetFlags(ENT_NET_FLAG_KILLED);
    }

    SelectItem(GetDefaultInventorySlot());

    Game.RegisterTriggerParam(_T("name"), GetClientName());
    Game.RegisterTriggerParam(_T("clientid"), XtoA(GetClientID()));
    Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
    Game.TriggerGlobalScript(_T("spawn"));
}


/*====================
  IPlayerEntity::SetSpawnLocation
  ====================*/
bool    IPlayerEntity::SetSpawnLocation(uint uiIndex)
{
    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
    if (pTeamInfo == NULL)
    {
        Console.Err << _T("IPlayerEntity::Spawn() - Invalid team: ") << int(m_iTeam) << newl;
        return false;
    }

    IVisualEntity *pTarget(Game.GetVisualEntity(uiIndex));
    if (pTarget != NULL && pTarget->CanSpawnFrom(this))
    {
        m_uiSpawnLocation = uiIndex;
        return true;
    }

    return false;
}


/*====================
  IPlayerEntity::GetSpawnLocation
  ====================*/
CVec3f  IPlayerEntity::GetSpawnLocation(uint uiIndex)
{
    CEntityTeamInfo *pTeamInfo(Game.GetTeam(m_iTeam));
    if (pTeamInfo == NULL)
    {
        Console.Err << _T("IPlayerEntity::Spawn() - Invalid team: ") << int(m_iTeam) << newl;
        return V3_ZERO;
    }

    IGadgetEntity *pSpawnGadget(Game.GetGadgetEntity(m_uiSpawnLocation));
    if (pSpawnGadget != NULL)
        pSpawnGadget->IncrementCounter(0);

    IVisualEntity *pTarget(Game.GetVisualEntity(uiIndex));
    
    // If no index was specified, try the main base
    if (pTarget == NULL || !pTarget->CanSpawnFrom(this))
        pTarget = Game.GetVisualEntity(pTeamInfo->GetBaseBuildingIndex());
    if (pTarget == NULL || !pTarget->CanSpawnFrom(this))
    {
        if (m_iTeam != 0 || !IsObserver())
        {
            Console.Warn << _T("Team has no base building, choosing random spawn") << newl;
            return V3_ZERO;
        }
        else
        {
            CVec3f v3Pos;

            v3Pos[X] = Game.GetWorldWidth() / 2;
            v3Pos[Y] = Game.GetWorldHeight() / 2;
            v3Pos[Z] = Game.GetTerrainHeight(v3Pos[X], v3Pos[Y]) + 500.0f;

            return v3Pos;
        }
    }

    CVec3f v3Center(pTarget->GetPosition());
    pTarget->PlayerSpawnedFrom(this);

    float fX(pTarget->GetBounds().GetDim(X) / 2.0f);
    float fY(pTarget->GetBounds().GetDim(Y) / 2.0f);
    float fDist(1.0f * sqrt((fX * fX + fY * fY)));

    int iSpawnTries(0);
    float fBaseAngle(pTarget->GetAngles()[YAW]);
    float fAngle(0.0f);
    float fStep(2.0f);
    while (iSpawnTries < 100)
    {
        CVec3f v3Start(v3Center + CVec3f(0.0f, fDist, 0.0f));
        CVec3f v3PosA(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle + fAngle));
        CVec3f v3PosB(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle - fAngle));

        STraceInfo trace;
        Game.TraceBox(trace, v3PosA + V_UP * 1000.0f, v3PosA + V_UP * -1000.0f, m_bbBounds, TRACE_PLAYER_MOVEMENT);
        if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
            return trace.v3EndPos;

        Game.TraceBox(trace, v3PosB + V_UP * 1000.0f, v3PosB + V_UP * -1000.0f, m_bbBounds, TRACE_PLAYER_MOVEMENT);
        if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
            return trace.v3EndPos;

        ++iSpawnTries;
        fAngle += fStep;
    }

    if (iSpawnTries == 100)
    {
        // Retry using the model's bounds instead
        CModel *pModel(g_ResourceManager.GetModel(pTarget->GetModelHandle()));

        if (pModel == NULL)
        {
            Console.Err << _T("Failed to find a spawn location") << newl;
            return V3_ZERO;
        }

        fX = (pModel->GetBounds().GetDim(X) / 2.0f);
        fY = (pModel->GetBounds().GetDim(Y) / 2.0f);
        fDist = (1.0f * sqrt((fX * fX + fY * fY)));

        iSpawnTries = 0;
        fBaseAngle = pTarget->GetAngles()[YAW];
        fAngle = 0.0f;
        fStep = 2.0f;

        while (iSpawnTries < 100)
        {
            CVec3f v3Start(v3Center + CVec3f(0.0f, fDist, 0.0f));
            CVec3f v3PosA(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle + fAngle));
            CVec3f v3PosB(M_RotatePointAroundLine(v3Start, v3Center, v3Center + V_UP, fBaseAngle - fAngle));

            STraceInfo trace;
            Game.TraceBox(trace, v3PosA + V_UP * 1000.0f, v3PosA + V_UP * -1000.0f, m_bbBounds, TRACE_PLAYER_MOVEMENT);
            if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
                return trace.v3EndPos;

            Game.TraceBox(trace, v3PosB + V_UP * 1000.0f, v3PosB + V_UP * -1000.0f, m_bbBounds, TRACE_PLAYER_MOVEMENT);
            if (!trace.bHit || (trace.plPlane.v3Normal.z > 1.0f - p_maxWalkSlope && (trace.uiSurfFlags & SURF_TERRAIN)))
                return trace.v3EndPos;

            ++iSpawnTries;
            fAngle += fStep;
        }
    }

    Console.Err << _T("Failed to find a spawn location") << newl;
    return V3_ZERO;
}


/*====================
  IPlayerEntity::ServerFrame
  ====================*/
bool    IPlayerEntity::ServerFrame()
{
    if (!ICombatEntity::ServerFrame())
        return false;

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        RegenerateMana(MsToSec(Game.GetFrameLength()));
        RegenerateHealth(MsToSec(Game.GetFrameLength()));
    }

    // Check for an expiring corpse
    if (GetStatus() == ENTITY_STATUS_CORPSE && Game.GetGameTime() >= m_uiCorpseTime)
        return false;

    // Apply officer aura
    if (IsOfficer() && GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        ApplyState(State_Officer, INVALID_TIME, INVALID_TIME, GetIndex());

        static uivector vEntities;
        
        vEntities.clear();
        Game.GetEntitiesInRadius(vEntities, CSphere(m_v3Position, g_officerAuraRadius), SURF_MODEL | SURF_HULL);
        for (uivector_it it(vEntities.begin()); it != vEntities.end(); ++it)
        {
            IVisualEntity *pTarget(Game.GetEntityFromWorldIndex(*it));
            if (pTarget == NULL)
                continue;
            IPlayerEntity *pTargetPlayer(pTarget->GetAsPlayerEnt());
            if (pTargetPlayer == NULL)
                continue;
            if (pTargetPlayer->GetIndex() == GetIndex())
                continue;
            if (pTargetPlayer->GetTeam() != GetTeam())
                continue;
            if (pTargetPlayer->GetSquad() != GetSquad())
                continue;
            if (pTargetPlayer->GetStatus() != ENTITY_STATUS_ACTIVE)
                continue;
            
            pTarget->ApplyState(State_OfficerAura, Game.GetGameTime(), g_officerAuraFadeTime, GetIndex());
        }
    }

    // Dead
    if (GetStatus() == ENTITY_STATUS_DEAD)
    {
        // Check for limbo transition
        if (Game.GetGameTime() >= m_uiDeathTime || HasNetFlags(ENT_NET_FLAG_NO_RESURRECT))
        {
            if (HasNetFlags(ENT_NET_FLAG_NO_CORPSE))
                return false;

            Unlink();
            SetStatus(ENTITY_STATUS_CORPSE);
            Link();

            m_uiCorpseTime = Game.GetGameTime() + g_corpseTime;
        }
    }

    // Update movement orders
    if (m_yCurrentOrder == CMDR_ORDER_MOVE)
        if (m_uiCurrentOrderEntIndex == INVALID_INDEX && DistanceSq(m_v3Position, m_v3CurrentOrderPos) < SQR(100.0f))
            IncOrderSequence();

    if (m_yOfficerOrder == OFFICERCMD_MOVE)
    {
        if (m_uiOfficerOrderEntIndex == INVALID_INDEX && DistanceSq(m_v3Position, m_v3OfficerOrderPos) < SQR(100.0f))
            SetOfficerOrder(OFFICERCMD_INVALID);
    }

    // Building placement
    IBuildingEntity *pPreviewBuilding(Game.GetBuildingEntity(GetPreviewBuildingIndex()));
    if (pPreviewBuilding != NULL)
        pPreviewBuilding->SetPosition(GetTargetPosition(FAR_AWAY));

    return true;
}


/*====================
  IPlayerEntity::EnterLoadout
  ====================*/
void    IPlayerEntity::EnterLoadout()
{
    CancelBuild();

    if (m_uiPetIndex != INVALID_INDEX)
    {
        IGameEntity *pPet(Game.GetEntity(m_uiPetIndex));
        if (pPet && pPet->IsPet())
            pPet->GetAsPet()->Kill();

        m_uiPetIndex = INVALID_INDEX;
    }

    CEntityClientInfo *pClient(Game.GetClientInfo(GetClientID()));

    SetStatus(ENTITY_STATUS_DORMANT);

    if (pClient != NULL && Game.GetGamePhase() != GAME_PHASE_WARMUP)
        pClient->SetLoadoutTime(g_loadoutTime);

    Game.RegisterTriggerParam(_T("name"), GetClientName());
    Game.RegisterTriggerParam(_T("clientid"), XtoA(GetClientID()));
    Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
    Game.TriggerGlobalScript(_T("loadout"));

    // Relink (unlink basically)
    Unlink();
    Link();
}


/*====================
  IPlayerEntity::GiveGold
  ====================*/
void    IPlayerEntity::GiveGold(ushort unGold, const CVec3f &v3Pos, bool bUseTax, bool bUseIncomeMod)
{
    unGold = GiveGold(unGold, bUseTax, bUseIncomeMod);

    // Send reward event
    CBufferFixed<17> buffer;
    buffer << GAME_CMD_REWARD << byte(0) << v3Pos << ushort(unGold);
    Game.SendGameData(GetClientID(), buffer, true);
}

ushort  IPlayerEntity::GiveGold(ushort unGold, bool bUseTax, bool bUseIncomeMod)
{
    CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    IEntityState *pState;
    UShortMod modGold;

    if (bUseTax && pTeam != NULL)
    {
        ushort unTeamGold(INT_FLOOR(unGold * g_taxRate));
        pTeam->GiveGold(unTeamGold);
        unGold -= unTeamGold;
    }

    if (bUseIncomeMod)
    {
        for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
        {
            pState = GetState(i);

            if (pState == NULL)
                continue;

            modGold += pState->GetIncomeMod();
        }

        unGold = modGold.Modify(unGold);
    }

    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient != NULL)
        pClient->GiveGold(unGold);

    return unGold;
}


/*====================
  IPlayerEntity::GetExperienceValue
  ====================*/
void    IPlayerEntity::GiveExperience(float fExperience, const CVec3f &v3Pos)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient != NULL)
        pClient->GiveExperience(fExperience, v3Pos);
}

void    IPlayerEntity::GiveExperience(float fExperience)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    if (pClient != NULL)
        pClient->GiveExperience(fExperience);
}


/*====================
  IPlayerEntity::GetExperienceValue
  ====================*/
float   IPlayerEntity::GetExperienceValue() const
{
    CEntityClientInfo *pClient(Game.GetClientInfo(m_iClientNum));
    int iLevel(1);
    if (pClient != NULL)
        iLevel = pClient->GetLevel();

    return pow(float(iLevel), g_expRewardExponent) * IVisualEntity::GetExperienceValue();
}


/*====================
  IPlayerEntity::GetDeathPercent
  ====================*/
float   IPlayerEntity::GetDeathPercent() const
{
    //return 1.0f - (GetRemainingDeathTime() / float(g_deathTime));
    return 1.0f - CLAMP(GetRemainingDeathTime() / float(g_economyInterval), 0.0f, 1.0f);
}


/*====================
  IPlayerEntity::GetRemainingDeathTime
  ====================*/
uint    IPlayerEntity::GetRemainingDeathTime() const
{
    if (m_uiDeathTime > Game.GetGameTime())
        return m_uiDeathTime - Game.GetGameTime();
    else
        return 0;
}


/*====================
  IPlayerEntity::KillReward
  ====================*/
void    IPlayerEntity::KillReward(IGameEntity *pKiller)
{
    if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
        return;

    ICombatEntity::KillReward(pKiller);

    if (pKiller == NULL)
        return;
    if (pKiller->GetIndex() == GetIndex())
        return;

    IPlayerEntity *pKillerPlayer(pKiller->GetAsPlayerEnt());
    if (pKillerPlayer == NULL)
    {
        if (pKiller->IsPet())
        {
            IGameEntity *pPetOwner(Game.GetEntityFromUniqueID(pKiller->GetAsPet()->GetOwnerUID()));
            if (pPetOwner != NULL)
                pKillerPlayer = pPetOwner->GetAsPlayerEnt();
        }
        if (pKiller->IsGadget())
        {
            IGameEntity *pGadgetOwner(Game.GetEntity(pKiller->GetAsGadget()->GetOwnerIndex()));
            if (pGadgetOwner != NULL)
                pKillerPlayer = pGadgetOwner->GetAsPlayerEnt();
        }

        if (pKillerPlayer == NULL)
            return;
    }

    // Spawn a soul
    CEntitySoul *pSoul(static_cast<CEntitySoul*>(Game.AllocateEntity(_T("Entity_Soul"))));
    if (pSoul == NULL)
    {
        Console.Warn << _T("Failed to create soul entity") << newl;
    }
    else
    {
        pSoul->SetPosition(m_v3Position);
        pSoul->SetTarget(pKillerPlayer->GetIndex());
        pSoul->Spawn();

        if (g_ResourceManager.GetAnim(GetModelHandle(), _T("die_soul")) != -1)
            StartAnimation(_T("die_soul"), -1);
    }

    Game.SendMessage(GetClientName() + _T(" has been killed by ") + pKillerPlayer->GetClientName(), -1);
}


/*====================
  IPlayerEntity::Kill
  ====================*/
void    IPlayerEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    CancelBuild();

    if (GetCurrentItem())
        GetCurrentItem()->Unselected();

    m_fHealth = 0.0f;
    m_fMana = 0.0f;
    m_fStamina = 0.0f;

    Unlink();
    if (GetStatus() != ENTITY_STATUS_DEAD && !HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
    {
        ushort unAttackerType(INVALID_ENT_TYPE);
        int iAttackerClientID(-1);
        if (pAttacker != NULL)
        {
            unAttackerType = pAttacker->GetType();
            if (pAttacker->GetAsPlayerEnt() != NULL)
                iAttackerClientID = pAttacker->GetAsPlayerEnt()->GetClientID();
        }

        Game.MatchStatEvent(GetClientID(), PLAYER_MATCH_DEATHS, 1, iAttackerClientID, unKillingObjectID, INVALID_ENT_TYPE, Game.GetGameTime());
        m_yKillStreak = 0;
    }

    SetStatus(ENTITY_STATUS_DEAD);

    CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    bool bDemoAccount(true);

    if (Game.GetClientInfo(GetClientID()) != NULL)
        bDemoAccount = Game.GetClientInfo(GetClientID())->IsDemoAccount();

    if (pTeam != NULL)
    {
        int iNumIntervals(1);
        if (pTeam->HasNetFlags(ENT_NET_FLAG_UPKEEP_FAILED))
            ++iNumIntervals;
        iNumIntervals += Game.GetCurrentGameLength() / MinToMs(g_respawnIncreaseInterval.GetUnsignedInteger());

        m_uiDeathTime = pTeam->GetNextEconomyInterval() + g_economyInterval * iNumIntervals;
    }
    else
    {
        m_uiDeathTime = Game.GetGameTime() + g_economyInterval * 2;
    }
    if (IsOfficer())
        m_uiDeathTime -= MIN(m_uiDeathTime, g_respawnOfficerBonus.GetUnsignedInteger());

    if (bDemoAccount)
        m_uiDeathTime += 3000;

    ClearStates();
    
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    StartAnimation(_T("die"), -1);
    Link();

    if (!HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
        KillReward(pAttacker);

    if (IsOfficer() && !HasLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT))
    {
        CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam != NULL)
            pTeam->KillSquadObjects(GetSquad());
    }

    if (m_uiPetIndex != INVALID_INDEX)
    {
        IGameEntity *pPet(Game.GetEntity(m_uiPetIndex));
        if (pPet && pPet->IsPet())
            pPet->GetAsPet()->Kill();

        m_uiPetIndex = INVALID_INDEX;
    }

    IGameEntity *pEnt(Game.GetFirstEntity());
    while (pEnt)
    {
        if (pEnt->IsPlayer())
        {
            IPlayerEntity *pPlayer(pEnt->GetAsPlayerEnt());

            // Clear this order for all players
            for (byte ySeq(0); ySeq < pPlayer->GetNumOrders(); ySeq++)
                if (pPlayer->GetOrderEntIndex(ySeq) == m_uiIndex)
                    pPlayer->DeleteOrder(ySeq);
                        
            if (pPlayer->GetOfficerOrderEntIndex() == m_uiIndex)
                pPlayer->SetOfficerOrder(OFFICERCMD_INVALID);
        }

        pEnt = Game.GetNextEntity(pEnt);
    }

    // Kick them out of the sacrificial shrine menu
    RemoveNetFlags(ENT_NET_FLAG_SACRIFICE_MENU);

    tstring sMethod(_T("Unknown"));
    if (unKillingObjectID != INVALID_ENT_TYPE)
    {
        ICvar *pCvar(EntityRegistry.GetGameSetting(unKillingObjectID, _T("Name")));

        if (pCvar != NULL)
            sMethod = pCvar->GetString();
    }

    Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));
    Game.RegisterTriggerParam(_T("attackingindex"), XtoA(pAttacker != NULL ? pAttacker->GetIndex() : INVALID_INDEX));
    Game.RegisterTriggerParam(_T("method"), sMethod);
    Game.TriggerEntityScript(GetIndex(), _T("death"));
}


/*====================
  IPlayerEntity::CancelBuild
  ====================*/
void    IPlayerEntity::CancelBuild()
{
    Game.DeleteEntity(m_uiPreviewBuildingIndex);
    m_uiPreviewBuildingIndex = INVALID_INDEX;
}


/*====================
  IPlayerEntity::IncreaseKillStreak()
  ====================*/
void    IPlayerEntity::IncreaseKillStreak()
{
    ++m_yKillStreak;
    
    switch (m_yKillStreak)
    {
        case 3:
        case 5:
        case 7:
        case 10:
        {
            CBufferFixed<6> buffer;
            buffer << GAME_CMD_KILLSTREAK << m_yKillStreak << m_iClientNum;
            Game.BroadcastGameData(buffer, true);
        } break;
        default:
        {
        } break;
    }
}


/*====================
  IPlayerEntity::GetTargetPosition
  ====================*/
CVec3f  IPlayerEntity::GetTargetPosition(float fRange, float fMinRange)
{
    if (HasNetFlags(ENT_NET_FLAG_BUILD_MODE))
    {
        // Set up a virtual camera window using relative cursor coords
        float fWidth(1.0f);
        float fHeight(1.0f);
        CVec2f v2Cursor(m_v2Cursor);
        v2Cursor.x *= fWidth;
        v2Cursor.y *= fHeight;

        CCamera camera;
        camera.SetWidth(fWidth);
        camera.SetHeight(fHeight);
        camera.SetFovXCalc(GetFov());
        SetupCamera(camera, GetPosition(), GetAngles());

        CVec3f v3Start(camera.GetOrigin());
        CVec3f v3Dir(camera.ConstructRay(v2Cursor));
        CVec3f v3End(M_PointOnLine(v3Start, v3Dir, FAR_AWAY));

        STraceInfo trace;
        Game.TraceLine(trace, v3Start, v3End, TRACE_TERRAIN);
        return trace.v3EndPos;
    }

    CVec3f v3Origin;
    CVec3f v3Max;
    CVec3f v3Result;
    CVec3f v3Forward;

    if (GetIsVehicle())
    {
        v3Origin = GetCameraPosition(m_v3Position, GetViewAngles() + GetAngles());
        v3Forward = M_GetForwardVecFromAngles(GetCameraAngles(GetViewAngles() + GetAngles()));
    }
    else
    {
        v3Origin = GetCameraPosition(m_v3Position, GetViewAngles());
        v3Forward = M_GetForwardVecFromAngles(GetCameraAngles(GetViewAngles()));
    }

    v3Max = v3Origin + v3Forward * fRange;

    STraceInfo trace;
    Game.TraceLine(trace, v3Origin, v3Max, ~SURF_TERRAIN);
    if (trace.fFraction == 1.0f)
    {
        v3Result = v3Max;
        v3Result[Z] = Game.GetTerrainHeight(v3Max[X], v3Max[Y]);
    }
    else
    {
        if (sqrt(M_GetDistanceSq(trace.v3EndPos, GetPosition())) < fMinRange)
        {
            v3Forward[Z] = 0.0f;
            M_Normalize(v3Forward);

            v3Result = GetPosition() + (v3Forward * fMinRange);
            v3Result[Z] = Game.GetTerrainHeight(v3Result[X], v3Result[Y]);
        }
        else
            v3Result = trace.v3EndPos;
    }

    return v3Result;
}


/*====================
  IPlayerEntity::GetCameraPosition
  ====================*/
CVec3f  IPlayerEntity::GetCameraPosition(const CVec3f &v3PlayerPos, const CVec3f &v3PlayerAngles)
{
    CVec3f v3CameraPos(v3PlayerPos);
    CVec3f v3CameraAngles(v3PlayerAngles);

    // Check for a third person camera
    if (ICvar::GetBool(_T("cam_alwaysThirdPerson")) ||
        GetStatus() == ENTITY_STATUS_DEAD ||
        GetCurrentItem() == NULL ||
        !GetCurrentItem()->IsFirstPerson())
    {
        v3CameraPos[Z] += m_pEntityConfig->GetCamHeight();

        if (cam_detach)
        {
            v3CameraAngles.Set(m_pEntityConfig->GetCamPitch(), 0.0f, cam_yaw);
            CAxis axis(v3CameraAngles);
            v3CameraPos -= axis.Forward() * m_pEntityConfig->GetCamDistance();
            return v3CameraPos;
        }

        v3CameraAngles[YAW] += cam_yaw;
        v3CameraAngles[PITCH] -= m_pEntityConfig->GetCamPitch();

        float fCameraDistance(m_pEntityConfig->GetCamDistance());

        if (fCameraDistance > 0.0f)
        {
            CVec3f v3DesiredPos(M_PointOnLine(v3CameraPos, CAxis(v3CameraAngles).Forward(), -fCameraDistance));

            STraceInfo trace;
            Game.TraceBox(trace, m_v3Position + CVec3f(0.0f, 0.0f, GetViewHeight()), v3DesiredPos, CBBoxf(CVec3f(-10.0f, -10.0f, -10.0f), CVec3f(10.0f, 10.0f, 10.0f)), TRACE_CAMERA);
            return trace.v3EndPos;
        }
        else
        {
            return v3CameraPos;
        }
    }

    v3CameraPos[Z] += GetViewHeight();
    return v3CameraPos;
}


/*====================
  IPlayerEntity::GetCameraAngles
  ====================*/
CVec3f  IPlayerEntity::GetCameraAngles(const CVec3f &v3PlayerAngles, bool bForceThirdPerson)
{
    CVec3f v3Angles(v3PlayerAngles);

    // Check for a third person camera
    if (ICvar::GetBool(_T("cam_alwaysThirdPerson")) ||
        GetStatus() == ENTITY_STATUS_DEAD ||
        GetCurrentItem() == NULL ||
        !GetCurrentItem()->IsFirstPerson() ||
        bForceThirdPerson)
    {
        if (cam_detach)
            return CVec3f(m_pEntityConfig->GetCamPitch(), 0.0f, cam_yaw);

        v3Angles[YAW] += cam_yaw;
    }

    return v3Angles;
}


/*====================
  IPlayerEntity::UpdateViewBob
  ====================*/
void    IPlayerEntity::UpdateViewBob(uint uiFrameTime)
{
    float fPlayerSpeed(m_v3Velocity.xy().Length());
    float fWalkBobSpeed(p_bobSpeed);
    float fWalkBobAmount(p_bobAmount);

    CVec3f v3NewBobPos(0.0f, 0.0f, 0.0f);
    CVec3f v3NewBobAngles(0.0f, 0.0f, 0.0f);

    CAxis aViewAxis(m_v3Angles);
    
    if (m_iCurrentMovement & PLAYER_MOVE_RUN_FLAGS)
    {
        if (m_iCurrentMovement & PLAYER_MOVE_SPRINT)
        {
            fWalkBobSpeed /= p_speedSprint;
            fWalkBobAmount *= 1.5f;
        }

        // Set bobcycle based on player speed
        float fTimeMult(0.0f);
        float fSpeedMod(GetSpeed());

        if (fSpeedMod != 0)
            fTimeMult = fPlayerSpeed / fSpeedMod;

        m_fBobCycle = sin(m_fBobTime * fWalkBobSpeed);
        m_fBobHalfCycle = sin(m_fBobTime * fWalkBobSpeed * 0.5f);
        m_fBobTime += MsToSec(uiFrameTime) * fTimeMult;

        // More up and down movement when walking forward
        if (m_iCurrentMovement & PLAYER_MOVE_FWD)
        {
            v3NewBobPos.z = m_fBobCycle * p_bobAmountUp * fWalkBobAmount;
            v3NewBobPos.x = aViewAxis.Right().x * m_fBobHalfCycle * p_bobAmountRight * fWalkBobAmount;
            v3NewBobPos.y = aViewAxis.Right().y * m_fBobHalfCycle * p_bobAmountRight * fWalkBobAmount;
            v3NewBobAngles[ROLL] = 0.0f;            
        }
        else 
        {
            v3NewBobPos.z = m_fBobCycle * p_bobAmountUp * 0.6f * fWalkBobAmount;
            v3NewBobPos.x = aViewAxis.Right().x * m_fBobHalfCycle * p_bobAmountRight * 0.6f * fWalkBobAmount;
            v3NewBobPos.y = aViewAxis.Right().y * m_fBobHalfCycle * p_bobAmountRight * 0.6f * fWalkBobAmount;
            
            if (m_iCurrentMovement & PLAYER_MOVE_LEFT)
                v3NewBobAngles[ROLL] = -p_bobAmountStrafe;
            else if (m_iCurrentMovement & PLAYER_MOVE_RIGHT)
                v3NewBobAngles[ROLL] = p_bobAmountStrafe;
            else if (m_iCurrentMovement & PLAYER_MOVE_BACK)
                v3NewBobAngles[PITCH] = 0.0f;
        }
    }
    else
    {
        // Stop the bobbing     
        m_fBobCycle = 0.0f;
        m_fBobHalfCycle = 0.0f;
        v3NewBobPos = V3_ZERO;
        v3NewBobAngles = V3_ZERO;
        m_fBobTime = 0.0f;
    }

    float fFrameTime(MsToSec(uiFrameTime));

    m_v3BobPos = DECAY(m_v3BobPos, v3NewBobPos, p_bobPosHalfLife, fFrameTime);

    for (int i(X); i <= Z; ++i)
    {
        while (m_v3BobAngles[i] - v3NewBobAngles[i] > 180.0f)
            m_v3BobAngles[i] -= 360.0f;

        while (m_v3BobAngles[i] - v3NewBobAngles[i] < -180.0f)
            m_v3BobAngles[i] += 360.0f;
    }

    m_v3BobAngles = DECAY(m_v3BobAngles, v3NewBobAngles, p_bobAngleHalfLife, fFrameTime);
}


/*====================
  IPlayerEntity::CanDash
  ====================*/
bool    IPlayerEntity::CanDash()
{
    if (!m_pEntityConfig->GetCanDash())
        return false;

    if (p_dashRequireStamina && GetStamina() < p_dashActivateStaminaCost)
        return false;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        IEntityState *pState(GetState(i));
        if (pState == NULL)
            continue;
        if (!pState->GetAllowDash())
            return false;
    }

    if (m_uiLastDashTime == INVALID_TIME)
        return true;
    
    if (Game.GetGameTime() - m_uiLastDashTime < p_dashCooldownTime)
        return false;

    return true;
}


/*====================
  IPlayerEntity::IsFirstPerson
  ====================*/
bool    IPlayerEntity::IsFirstPerson()
{
    if (ICvar::GetBool(_T("cam_alwaysThirdPerson")) ||
        GetStatus() == ENTITY_STATUS_DEAD ||
        GetCurrentItem() == NULL ||
        !GetCurrentItem()->IsFirstPerson())
        return false;
    else
        return true;
}


/*====================
  IPlayerEntity::SetupCamera
  ====================*/
void    IPlayerEntity::SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles)
{
    PROFILE("IPlayerEntity::SetupCamera");

    if (IsFirstPerson())
    {
        // First person camera
        camera.AddFlags(CAM_FIRST_PERSON);
        camera.SetOrigin(GetCameraPosition(v3InputPosition, v3InputAngles) + m_v3BobPos);
        camera.SetAngles(GetCameraAngles(v3InputAngles) + m_v3BobAngles);
    }
    else
    {
        // Third person camera
        camera.RemoveFlags(CAM_FIRST_PERSON);
        camera.SetOrigin(GetCameraPosition(v3InputPosition, v3InputAngles));
        camera.SetAngles(GetCameraAngles(v3InputAngles, true));
    }

    camera.SetFovXCalc(GetFov());

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        m_apState[i]->ModifyCamera(camera);
    }

    camera.SetLodDistance(0.0f);
}


/*====================
  IPlayerEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IPlayerEntity::AllocateSkeleton()
{
    return K2_NEW(global,   CSkeleton);
}


/*====================
  IPlayerEntity::UpdateSkeleton
  ====================*/
void    IPlayerEntity::UpdateSkeleton(bool bPose)
{
    PROFILE("IPlayerEntity::UpdateSkeleton");

    if (m_pSkeleton == NULL)
        return;

    m_pSkeleton->SetModel(GetModelHandle());

    if (GetModelHandle() == INVALID_RESOURCE)
        return;

    float fDiagYaw(0.0f);

    if (!GetIsVehicle())
    {
        if (GetYawStrafe())
        {
            switch (m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS)
            {
            case PLAYER_MOVE_FWD_LEFT:
            case PLAYER_MOVE_BACK_RIGHT:
                fDiagYaw = 45.0f;
                break;
            case PLAYER_MOVE_FWD_RIGHT:
            case PLAYER_MOVE_BACK_LEFT:
                fDiagYaw = -45.0f;
                break;
            case PLAYER_MOVE_LEFT:
                fDiagYaw = p_yawStrafeAngle;
                break;
            case PLAYER_MOVE_RIGHT:
                fDiagYaw = -p_yawStrafeAngle;
                break;
            }
        }
        else
        {
            switch (m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS)
            {
            case PLAYER_MOVE_FWD_LEFT:
            case PLAYER_MOVE_BACK_RIGHT:
                fDiagYaw = 45.0f;
                break;
            case PLAYER_MOVE_FWD_RIGHT:
            case PLAYER_MOVE_BACK_LEFT:
                fDiagYaw = -45.0f;
                break;
            }
        }
    }

    float fPlayerYaw(m_v3Angles[YAW]);
    if (fPlayerYaw > 180.0f)
        fPlayerYaw -= 360.0f;
    if (fPlayerYaw < -180.0f)
        fPlayerYaw += 360.0f;

    // Turn animations
    if (GetIsVehicle())
    {
        m_uiTurnStartTime = 0;
        m_fCurrentYaw = fPlayerYaw;
    }
    else if ((m_iTurnAction != (m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS) ||
        m_ayTurnSequence[0] != m_ayAnimSequence[0] ||
        m_ayTurnSequence[1] != m_ayAnimSequence[1])
        && GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        if (m_uiTurnStartTime)
        {
            float fLerp(M_SmoothStepN((Game.GetGameTime() - m_uiTurnStartTime) / m_fYawLerpTime));
            m_fCurrentYaw = M_LerpAngle(fLerp, m_fLastYaw, m_fBaseYaw);
        }

        // If the player is moving or attacking, immediately start a new turn
        m_uiTurnStartTime = Game.GetGameTime();
        m_fLastYaw = m_fCurrentYaw;
        m_fBaseYaw = fPlayerYaw + fDiagYaw;
        m_fYawLerpTime = 200.0f;
        m_iTurnAction = (m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS);
        m_ayTurnSequence[0] = m_ayAnimSequence[0];
        m_ayTurnSequence[1] = m_ayAnimSequence[1];
    }
    else if ((m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS) == PLAYER_MOVE_IDLE && GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        // When a player is standing still, only turn their torso/head until they
        // pass a threshold, then play an animation

        // Determine the player's variance from the last angle he snapped to
        float fYawDelta(fPlayerYaw - m_fBaseYaw);
        if (fYawDelta > 180.0f)
            fYawDelta -= 360.0f;
        if (fYawDelta < -180.0f)
            fYawDelta += 360.0f;

        // If they've passed the threshold, give them a new base yaw
        if (fabs(fYawDelta) > 90.0f)
        {
            if (m_uiTurnStartTime)
            {
                float fLerp(M_SmoothStepN((Game.GetGameTime() - m_uiTurnStartTime) / m_fYawLerpTime));
                m_fCurrentYaw = M_LerpAngle(fLerp, m_fLastYaw, m_fBaseYaw);
            }

            m_uiTurnStartTime = Game.GetGameTime();
            m_fLastYaw = m_fCurrentYaw;
            m_fBaseYaw = fPlayerYaw;
            m_fYawLerpTime = 500.0f;

            if (fYawDelta > 0.0f)
            {
                if (m_pSkeleton->HasAnim(_T("turn_left")))
                    m_pSkeleton->StartAnim(_T("turn_left"), Game.GetGameTime(), 0);
            }
            else
            {
                if (m_pSkeleton->HasAnim(_T("turn_right")))
                    m_pSkeleton->StartAnim(_T("turn_right"), Game.GetGameTime(), 0);
            }
        }
    }

    if ((m_iCurrentMovement & PLAYER_MOVE_NO_FLAGS) != PLAYER_MOVE_IDLE)
    {
        // Update destination yaw every frame while moving
        m_fBaseYaw = fPlayerYaw + fDiagYaw;

        if (Game.GetGameTime() - m_uiTurnStartTime > m_fYawLerpTime)
        {
            // no lerping after the turn is over
            m_uiTurnStartTime = 0;
            m_fLastYaw = fPlayerYaw + fDiagYaw;
        }
    }

    if (m_uiTurnStartTime)
    {
        float fLerp(M_SmoothStepN((Game.GetGameTime() - m_uiTurnStartTime) / m_fYawLerpTime));
        m_fCurrentYaw = M_LerpAngle(fLerp, m_fLastYaw, m_fBaseYaw);
    }
    else
    {
        m_fCurrentYaw = fPlayerYaw + fDiagYaw;
    }

    // Pose skeleton
    if (bPose)
    {
        if (GetStatus() == ENTITY_STATUS_DEAD)
            m_pSkeleton->Pose(Game.GetGameTime(), 0.0f, 0.0f);
        else if (GetIsVehicle())
            m_pSkeleton->Pose(Game.GetGameTime(), m_uiWheelTime, 0.0f);
        else
            m_pSkeleton->Pose(Game.GetGameTime(), m_v3Angles[PITCH], m_v3Angles[YAW] - m_fCurrentYaw);
    }
    else
    {
        m_pSkeleton->PoseLite(Game.GetGameTime());
    }

    // Process animation events
    if (m_pSkeleton->CheckEvents())
    {
        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModelHandle())));

        const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

        for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
            EventScript.Execute(it->sCmd, it->iTimeNudge);

        m_pSkeleton->ClearEvents();

        FileManager.SetWorkingDirectory(sOldDir);
    }

    // Vehicle Tilting
    if (GetIsVehicle() && GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        CAxis aAxis(CVec3f(0.0f, 0.0f, m_v3Angles[YAW]));

        float fForward(50.0f * DIAG);
        float fRight(50.0f * DIAG);
        float fDown(200.0f);
        float fTiltSpeed(45.0f);

        CVec3f v3FL(m_v3Position + aAxis.Forward() * fForward + aAxis.Right() * -fRight);
        CVec3f v3FR(m_v3Position + aAxis.Forward() * fForward + aAxis.Right() * fRight);
        CVec3f v3BL(m_v3Position + aAxis.Forward() * -fForward + aAxis.Right() * -fRight);
        CVec3f v3BR(m_v3Position + aAxis.Forward() * -fForward + aAxis.Right() * fRight);
        
        CVec3f v3Down(0.0f, 0.0f, -fDown);

        float fTargetPitch, fTargetRoll;

        if (m_iMoveFlags & MOVE_ON_GROUND)
        {
            float fFL(TiltTrace(v3FL, v3FL + v3Down));
            float fFR(TiltTrace(v3FR, v3FR + v3Down));
            float fBL(TiltTrace(v3BL, v3BL + v3Down));
            float fBR(TiltTrace(v3BR, v3BR + v3Down));

            if (fabs(fBL - fFL) > fabs(fBR - fFR))
                fTargetPitch = RAD2DEG(atan(((fBR - fFR) * fDown) / (fForward * 2.0f)));
            else
                fTargetPitch = RAD2DEG(atan(((fBL - fFL) * fDown) / (fForward * 2.0f)));

            if (fabs(fFR - fFL) > fabs(fBR - fBL))
                fTargetRoll = RAD2DEG(atan(((fBR - fBL) * fDown) / (fForward * 2.0f)));
            else
                fTargetRoll = RAD2DEG(atan(((fFR - fFL) * fDown) / (fForward * 2.0f)));
        }
        else
        {
            fTargetPitch = m_fTiltPitch;
            fTargetRoll = m_fTiltRoll;
        }

        if (fTargetPitch > m_fTiltPitch)
            m_fTiltPitch = MIN(fTargetPitch, m_fTiltPitch + fTiltSpeed * MsToSec(Game.GetFrameLength()));
        else
            m_fTiltPitch = MAX(fTargetPitch, m_fTiltPitch - fTiltSpeed * MsToSec(Game.GetFrameLength()));

        if (fTargetRoll > m_fTiltRoll)
            m_fTiltRoll = MIN(fTargetRoll, m_fTiltRoll + fTiltSpeed * MsToSec(Game.GetFrameLength()));
        else
            m_fTiltRoll = MAX(fTargetRoll, m_fTiltRoll - fTiltSpeed * MsToSec(Game.GetFrameLength()));


        // Height offset
        CVec3f v3BoxFL(aAxis.Forward() * 50.0f + aAxis.Right() * -50.0f);
        CVec3f v3BoxFR(aAxis.Forward() * 50.0f + aAxis.Right() * 50.0f);
        CVec3f v3BoxBL(aAxis.Forward() * -50.0f + aAxis.Right() * -50.0f);
        CVec3f v3BoxBR(aAxis.Forward() * -50.0f + aAxis.Right() * 50.0f);

        CVec3f v3Center(0.0f, 0.0f, 0.0f);

        if (m_fTiltPitch > 0.0f)
            v3Center = M_RotatePointAroundLine(v3Center, v3BoxFL, v3BoxFR, m_fTiltPitch);
        else
            v3Center = M_RotatePointAroundLine(v3Center, v3BoxBL, v3BoxBR, m_fTiltPitch);

        if (m_fTiltRoll > 0.0f)
            v3Center = M_RotatePointAroundLine(v3Center, v3BoxBL, v3BoxFL, m_fTiltRoll);
        else
            v3Center = M_RotatePointAroundLine(v3Center, v3BoxBR, v3BoxFR, m_fTiltRoll);

        m_fTiltHeight = v3Center.z;
    }
    else
    {
        m_fTiltPitch = 0.0f;
        m_fTiltRoll = 0.0f;
        m_fTiltHeight = 0.0f;
    }
}


/*====================
  AddDebugLine
  ====================*/
static void AddDebugLine(const CVec3f &v3Start, const CVec3f &v3End)
{
    CSceneEntity sceneEntity;

    sceneEntity.objtype = OBJTYPE_BEAM;
    sceneEntity.SetPosition(v3Start);
    sceneEntity.beamTargetPos = v3End;
    sceneEntity.scale = 2.0f;
    sceneEntity.height = 1.0f;
    sceneEntity.hMaterial = g_ResourceManager.Register(_T("/core/materials/effect_solid.material"), RES_MATERIAL);
    sceneEntity.color = BLUE;

    SceneManager.AddEntity(sceneEntity);
}


/*====================
  IPlayerEntity::AddToScene
  ====================*/
bool    IPlayerEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    PROFILE("IPlayerEntity::AddToScene");

    if (Game.GetLocalClientNum() == m_iClientNum &&
        (Game.GetCamera() == NULL ||
        Game.GetCamera()->HasFlags(CAM_FIRST_PERSON)))
    {
        // Only update skeleton if this is our third-person entity and we're in first-person
        UpdateSkeleton(false);
        return false;
    }

    if (GetStatus() == ENTITY_STATUS_SPAWNING ||
        GetStatus() == ENTITY_STATUS_DORMANT ||
        (GetStatus() == ENTITY_STATUS_DEAD && HasNetFlags(ENT_NET_FLAG_NO_CORPSE)))
        return false;

    if (GetModelHandle() == INVALID_INDEX)
        return false;

    if (Game.IsCommander() && !m_bSighted)
        return false;

    CSceneEntity sceneEntity;
    
    sceneEntity.Clear();

    sceneEntity.SetPosition(m_v3Position + CVec3f(0.0f, 0.0f, m_fTiltHeight));

    CVec3f v3Angles;
    if (m_pSkeleton)
    {
        v3Angles[YAW] = m_fCurrentYaw;
        v3Angles[PITCH] = 0.0f;
        v3Angles[ROLL] = 0.0f;
    }
    else
    {
        v3Angles[YAW] = m_v3Angles[YAW];
        v3Angles[PITCH] = 0.0f;
        v3Angles[ROLL] = 0.0f;
    }

    v3Angles[PITCH] += m_fTiltPitch;
    v3Angles[ROLL] += m_fTiltRoll;

    if (m_v3AxisAngles != v3Angles)
    {
        m_aAxis.Set(v3Angles);
        m_v3AxisAngles = v3Angles;
    }

    sceneEntity.axis = m_aAxis;
    sceneEntity.scale = GetScale() * GetScale2();
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hModel = GetModelHandle();
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4Color;
    sceneEntity.flags = iFlags | SCENEOBJ_SOLID_COLOR | SCENEOBJ_USE_AXIS;

    // Check for a state setting an alternate skin
    tstring sAlternateSkin;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        sAlternateSkin = m_apState[i]->GetSkin();
        if (!sAlternateSkin.empty())
        {
            sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), sAlternateSkin);
            break;
        }
    }

    if (sAlternateSkin.empty() && Game.LooksLikeEnemy(m_uiIndex))
        sceneEntity.hSkin = g_ResourceManager.GetSkin(GetModelHandle(), _T("red"));

    if (m_uiClientRenderFlags & ECRF_SNAPSELECTED)
        sceneEntity.color *= m_v4SelectColor;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        sceneEntity.color[A] *= 0.5f;
    
    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
    {
        UpdateSkeleton(true);

        ushort unDisguiseItem(GetDisguiseItem());
        if (unDisguiseItem == INVALID_ENT_TYPE)
        {
            IInventoryItem *pItem(GetCurrentItem());
            if (pItem != NULL)
            {
                AttachModel(pItem->GetModel1Bone(), pItem->GetModel1Handle());
                AttachModel(pItem->GetModel2Bone(), pItem->GetModel2Handle());
            }
        }
        else
        {
            ICvar *pModel1Bone(g_EntityRegistry.GetGameSetting(unDisguiseItem, _T("Model1Bone")));
            ICvar *pModel2Bone(g_EntityRegistry.GetGameSetting(unDisguiseItem, _T("Model2Bone")));
            ICvar *pModel1Path(g_EntityRegistry.GetGameSetting(unDisguiseItem, _T("Model1Path")));
            ICvar *pModel2Path(g_EntityRegistry.GetGameSetting(unDisguiseItem, _T("Model2Path")));

            if (pModel1Bone != NULL && pModel1Path != NULL)
                AttachModel(pModel1Bone->GetString(), g_ResourceManager.Register(pModel1Path->GetString(), RES_MODEL));
            if (pModel2Bone != NULL && pModel2Path != NULL)
                AttachModel(pModel2Bone->GetString(), g_ResourceManager.Register(pModel2Path->GetString(), RES_MODEL));
        }

        AddSelectionRingToScene();

        if (p_debugViewHeight)
        {
            CAxis aAxis(m_v3Angles);
            CVec3f v3Start(m_v3Position + CVec3f(0.0f, 0.0f, m_pEntityConfig->GetViewHeight()));
            CVec3f v3End(v3Start + aAxis.Forward() * 100.0f);

            AddDebugLine(v3Start, v3End);

            CBBoxf bbBoundsWorld(m_bbBounds);
            bbBoundsWorld.Transform(m_v3Position, CAxis(0.0f, 0.0f, 0.0f), Game.IsCommander() ? GetCommanderScale() : 1.0f);
            vector<CVec3f> v3Points(bbBoundsWorld.GetCorners());

            AddDebugLine(v3Points[0], v3Points[1]);
            AddDebugLine(v3Points[1], v3Points[3]);
            AddDebugLine(v3Points[3], v3Points[2]);
            AddDebugLine(v3Points[2], v3Points[0]);

            AddDebugLine(v3Points[0], v3Points[4]);
            AddDebugLine(v3Points[1], v3Points[5]);
            AddDebugLine(v3Points[2], v3Points[6]);
            AddDebugLine(v3Points[3], v3Points[7]);

            AddDebugLine(v3Points[4], v3Points[5]);
            AddDebugLine(v3Points[5], v3Points[7]);
            AddDebugLine(v3Points[7], v3Points[6]);
            AddDebugLine(v3Points[6], v3Points[4]);
        }
    }
    else
    {
        UpdateSkeleton(false);
    }

    return true;
}


/*====================
  IPlayerEntity::IsVisibleOnMinimap
  ====================*/
bool    IPlayerEntity::IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    if (pLocalPlayer != NULL && pLocalPlayer->GetIndex() == GetIndex())
        return false;

    return ICombatEntity::IsVisibleOnMinimap(pLocalPlayer, bLargeMap);
}


/*====================
  IPlayerEntity::GetMapIcon
  ====================*/
ResHandle   IPlayerEntity::GetMapIcon(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(GetDisguiseClient()));
    if (pClient != NULL && pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
        return s_hOfficerStarIcon;

    return ICombatEntity::GetMapIcon(pLocalPlayer, bLargeMap);
}


/*====================
  IPlayerEntity::GetMapIconColor
  ====================*/
CVec4f  IPlayerEntity::GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    if (pLocalPlayer != NULL && !pLocalPlayer->LooksLikeEnemy(this))
    {
        CEntityTeamInfo *pTeam(Game.GetTeam(pLocalPlayer->GetTeam()));
        if (pTeam != NULL)
            return GetColorFromString(pTeam->GetSquadColor(GetSquad()));
    }

    return ICombatEntity::GetMapIconColor(pLocalPlayer, bLargeMap);
}


/*====================
  IPlayerEntity::GetMapIconSize
  ====================*/
float   IPlayerEntity::GetMapIconSize(IPlayerEntity *pLocalPlayer, bool bLargeMap)
{
    CEntityClientInfo *pClient(Game.GetClientInfo(GetDisguiseClient()));
    if (pClient != NULL && pClient->HasFlags(CLIENT_INFO_IS_OFFICER))
        return g_minimapOfficerIconSize;

    return ICombatEntity::GetMapIconSize(pLocalPlayer, bLargeMap);
}


/*====================
  IPlayerEntity::Link
  ====================*/
void    IPlayerEntity::Link()
{
    if  (m_yStatus == ENTITY_STATUS_DORMANT || m_yStatus == ENTITY_STATUS_SPAWNING || m_yStatus == ENTITY_STATUS_HIDDEN)
        return;

    if (m_uiWorldIndex != INVALID_INDEX)
    {
        CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
        
        if (pWorldEnt != NULL)
        {
            pWorldEnt->SetPosition(GetPosition());
            pWorldEnt->SetScale(GetScale());
            pWorldEnt->SetScale2(GetScale2());
            pWorldEnt->SetAngles(GetAngles());
            pWorldEnt->SetBounds(GetBounds());
            pWorldEnt->SetModelHandle(GetModelHandle());
            pWorldEnt->SetGameIndex(GetIndex());

            uint uiLinkFlags(SURF_PLAYER);
            if (IsIntangible())
                uiLinkFlags |= SURF_INTANGIBLE;

            if (m_yStatus == ENTITY_STATUS_DEAD)
                uiLinkFlags |= SURF_DEAD;
            else if (m_yStatus == ENTITY_STATUS_CORPSE)
                uiLinkFlags |= SURF_CORPSE;

            Game.LinkEntity(m_uiWorldIndex, LINK_BOUNDS, uiLinkFlags);
        }
    }
}


/*====================
  IPlayerEntity::Unlink
  ====================*/
void    IPlayerEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX)
        Game.UnlinkEntity(m_uiWorldIndex);
}


/*====================
  IPlayerEntity::Accelerate
  ====================*/
void    IPlayerEntity::Accelerate(const CVec3f &v3Intent, float fAcceleration)
{
    float fCurrentSpeed(s_Move.v3Velocity.Length());

    CVec3f v3IntentDir;
    float fIntentSpeed;

    v3Intent.Decompose(v3IntentDir, fIntentSpeed);

    float fForwardSpeed(DotProduct(s_Move.v3Velocity, v3IntentDir));
    float fAddspeed(fIntentSpeed - fForwardSpeed);
    
    if (fAddspeed <= 0.0f)
        return;

    float fAccelSpeed(fIntentSpeed * s_Move.fFrameTime * fAcceleration);
    
    if (fAccelSpeed > fAddspeed)
        fAccelSpeed = fAddspeed;
    
    s_Move.v3Velocity += v3IntentDir * fAccelSpeed;

    // Don't let us accelerate beyond v3Intent speed
    if (fCurrentSpeed > fIntentSpeed)
        s_Move.v3Velocity.SetLength(fCurrentSpeed);
}


/*====================
  IPlayerEntity::Friction
  ====================*/
void    IPlayerEntity::Friction(float fFriction)
{
    float fSpeed(s_Move.v3Velocity.Length());

    if (fSpeed > 0.0f)
    {
        float fNewSpeed(MAX(0.0f, fSpeed - fFriction * s_Move.fFrameTime));

        s_Move.v3Velocity *= fNewSpeed / fSpeed;
    }

    if (s_Move.v3Velocity.Length() < 0.0001f)
        s_Move.v3Velocity = V3_ZERO;
}


/*====================
  IPlayerEntity::MoveFly
  ====================*/
void    IPlayerEntity::MoveFly(const CClientSnapshot &snapshot)
{
    try
    {
        float fFrameTime(MsToSec(snapshot.GetFrameLength()));

        s_Move.v3Angles = snapshot.GetCameraAngles();
        s_Move.fFrameTime = fFrameTime;
        s_Move.v3OldPosition = m_v3Position;
        s_Move.v3OldVelocity = m_v3Velocity;
        s_Move.v3OldAngles = m_v3Angles;
        s_Move.v3Position = m_v3Position;
        s_Move.v3Velocity = m_v3Velocity;
        s_Move.fRunSpeed = GetSpeed() * p_speed;
        s_Move.iMoveFlags = m_iMoveFlags;
        s_Move.bLanded = false;

        CAxis axis(s_Move.v3Angles);
        s_Move.v3Velocity.Clear();
        if (snapshot.IsButtonDown(GAME_BUTTON_UP))
            s_Move.v3Velocity += V_UP;
        if (snapshot.IsButtonDown(GAME_BUTTON_DOWN))
            s_Move.v3Velocity -= V_UP;
        if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
            s_Move.v3Velocity += axis.Forward();
        if (snapshot.IsButtonDown(GAME_BUTTON_BACK))
            s_Move.v3Velocity -= axis.Forward();
        if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
            s_Move.v3Velocity += axis.Right();
        if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
            s_Move.v3Velocity -= axis.Right();
        s_Move.v3Velocity.Normalize();
        s_Move.v3Velocity *= GetSpeed();

        if (snapshot.IsButtonDown(GAME_BUTTON_SPRINT))
            s_Move.v3Velocity *= (p_speedSprint * 2);
        else
            s_Move.v3Velocity *= p_speed;

        s_Move.v3Position += m_v3Velocity * fFrameTime;
        
        m_v3Velocity = s_Move.v3Velocity;
        m_v3Position = s_Move.v3Position;
        m_v3Angles = s_Move.v3Angles;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPlayerEntity::MoveFly() - "));
    }
}


/*====================
  IPlayerEntity::MoveWalkGround
  ====================*/
void    IPlayerEntity::MoveWalkGround()
{
    // Friction
    float fRelativeFactor(s_Move.v3Velocity.Length() > s_Move.fRunSpeed ? s_Move.v3Velocity.Length() : s_Move.fRunSpeed);
    float fGroundFriction(fRelativeFactor * p_groundFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

    // Apply ground friction
    Friction(fGroundFriction);

    CVec3f v3IntentDir;
    if (s_Move.v3Intent != V3_ZERO)
        v3IntentDir = Normalize(s_Move.v3Intent);

    const CVec3f &v3Normal(s_Move.plGround.v3Normal);
    float fIntentSpeed(s_Move.v3Intent.Length());

    // Project movement intent onto the ground plane top-down
    // Ax + By + Cz = 0 --> Cz = -Ax - By --> z = (-Ax - By)/C
    s_Move.v3Intent.z = (-v3Normal.x * s_Move.v3Intent.x - v3Normal.y * s_Move.v3Intent.y) / v3Normal.z;
    s_Move.v3Intent.SetLength(fIntentSpeed);

    // Accelerate
    Accelerate(s_Move.v3Intent, p_groundAccelerate);

    StepSlide(false);
}


/*====================
  IPlayerEntity::MoveWalkAir
  ====================*/
void    IPlayerEntity::MoveWalkAir()
{
    if (s_Move.iMoveFlags & MOVE_ON_GROUND)
    {
        // Friction
        float fGroundFriction(p_slopeFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

        // Apply ground friction
        Friction(fGroundFriction);

        // Project movement intent onto the ground plane
        s_Move.v3Intent.SetDirection(Clip(s_Move.v3Intent.Direction(), s_Move.plGround.v3Normal));
    }

    Accelerate(s_Move.v3Intent, p_airAccelerate);

    Slide(true);
}


/*====================
  IPlayerEntity::MoveWalkWater
  ====================*/
void    IPlayerEntity::MoveWalkWater()
{
}


/*====================
  IPlayerEntity::MoveTrajectory
  ====================*/
void    IPlayerEntity::MoveTrajectory(const CClientSnapshot &snapshot)
{
    Unlink();

    float fFrameTime(MIN(MsToSec(snapshot.GetFrameLength()), 0.25f));
    
    CVec3f v3End(GetPosition() + GetVelocity() * fFrameTime);

    STraceInfo trace;
    Game.TraceBox(trace, GetPosition(), v3End, GetBounds(), 0, GetWorldIndex());
    if (trace.fFraction < 1.0f)
    {
        SetVelocity(V_ZERO);
        if (GetCurrentItem() != NULL)
            GetCurrentItem()->FinishedAction(GetAction());
         m_iCurrentAction &= ~PLAYER_ACTION_TRAJECTORY;
    }

    m_v3Velocity[Z] -= p_gravity * fFrameTime;
    SetPosition(trace.v3EndPos);

    Link();
}


/*====================
  IPlayerEntity::MoveCorpse
  ====================*/
void    IPlayerEntity::MoveCorpse(const CClientSnapshot &snapshot)
{
    Unlink();

    float fFrameTime(MIN(MsToSec(snapshot.GetFrameLength()), 0.25f));

    // Set working vars
    s_Move.fFrameTime = fFrameTime;
    s_Move.v3OldPosition = m_v3Position;
    s_Move.v3OldVelocity = m_v3Velocity;
    s_Move.v3OldAngles = m_v3Angles;
    s_Move.v3Position = m_v3Position;
    s_Move.v3Velocity = m_v3Velocity;
    s_Move.v3Angles = m_v3Angles;
    s_Move.fRunSpeed = 0.0f;
    s_Move.iMoveFlags = m_iMoveFlags;
    s_Move.bLanded = false;
    s_Move.v3Intent = V3_ZERO;
 
    CheckGround();

    if (s_Move.iMoveFlags & MOVE_ON_GROUND)
    {
        // Friction
        float fGroundFriction((s_Move.bGroundControl ? p_corpseFriction : p_slopeFriction) * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

        // Apply ground friction
        Friction(fGroundFriction);

        // Project movement intent onto the ground plane
        s_Move.v3Intent.SetDirection(Clip(s_Move.v3Intent.Direction(), s_Move.plGround.v3Normal));
    }

    Accelerate(s_Move.v3Intent, p_airAccelerate);

    Slide(!s_Move.bGroundControl);

    CheckGround();

    // Apply this movement
    if (IsPositionValid(s_Move.v3Position))
    {
        m_v3Position = s_Move.v3Position;
        m_v3Velocity = s_Move.v3Velocity;
        m_iMoveFlags = s_Move.iMoveFlags;
    }
    else
    {
        if (Game.IsServer() && p_debugMovement)
            Console << _T("Player move got stuck") << newl;
    }

    m_v3Angles = s_Move.v3Angles;

    Link();
}


/*====================
  IPlayerEntity::MoveWalk
  ====================*/
void    IPlayerEntity::MoveWalk(const CClientSnapshot &snapshot)
{
    try
    {
        PROFILE("IPlayerEntity::MoveWalk");

        if (GetAction() & PLAYER_ACTION_TRAJECTORY)
        {
            MoveTrajectory(snapshot);
            return;
        }

        m_v3ViewAngles = snapshot.GetCameraAngles();

        // Early release to loadout screen when dead
        if (GetStatus() == ENTITY_STATUS_DEAD)
        {
            if (snapshot.ButtonPressed(GAME_BUTTON_USE))
            {
                if (HasNetFlags(ENT_NET_FLAG_NO_CORPSE) ||
                Game.GetGameTime() - m_uiDeathTime >= g_minDeathTime)
                {
                    SetNetFlags(ENT_NET_FLAG_NO_RESURRECT);
                }
            }
        }

        if (GetStatus() == ENTITY_STATUS_CORPSE || GetStatus() == ENTITY_STATUS_DEAD)
        {
            MoveCorpse(snapshot);
            return;
        }
        
        if (GetStatus() != ENTITY_STATUS_ACTIVE)
            return;

        Unlink();

        int iNewMovement(PLAYER_MOVE_IDLE);
        float fFrameTime(MIN(MsToSec(snapshot.GetFrameLength()), 0.25f));

        // Set working vars
        s_Move.fFrameTime = fFrameTime;
        s_Move.v3OldPosition = m_v3Position;
        s_Move.v3OldVelocity = m_v3Velocity;
        s_Move.v3OldAngles = m_v3Angles;
        s_Move.v3Position = m_v3Position;
        s_Move.v3Velocity = m_v3Velocity;
        s_Move.v3Angles = m_v3Angles;
        s_Move.fRunSpeed = GetSpeed() * p_speed;
        s_Move.iMoveFlags = m_iMoveFlags;
        s_Move.bLanded = false;

        CheckGround();

        // Turning
        if (GetIsVehicle())
        {
            if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
            {
                if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                    s_Move.v3Angles[YAW] -= m_pEntityConfig->GetVehicleTurnSpeed() * s_Move.fFrameTime;
                else if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                    s_Move.v3Angles[YAW] += m_pEntityConfig->GetVehicleTurnSpeed() * s_Move.fFrameTime;
            }
            else if (snapshot.IsButtonDown(GAME_BUTTON_BACK))
            {
                if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                    s_Move.v3Angles[YAW] += m_pEntityConfig->GetVehicleTurnSpeed() * s_Move.fFrameTime;
                else if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                    s_Move.v3Angles[YAW] -= m_pEntityConfig->GetVehicleTurnSpeed() * s_Move.fFrameTime;
            }
        }
        else
        {
            s_Move.v3Angles = snapshot.GetCameraAngles();
        }

        // Get directions in 2d space
        CAxis axis(s_Move.v3Angles);
        CVec3f v3Forward(axis.Forward());
        v3Forward.z = 0.0f;
        v3Forward.Normalize();
        CVec3f v3Right(axis.Right());
        v3Right.z = 0.0f;
        v3Right.Normalize();

        // Button states
        if (!snapshot.IsButtonDown(GAME_BUTTON_UP))
            s_Move.iMoveFlags &= ~MOVE_JUMP_HELD;
        if (!snapshot.IsButtonDown(GAME_BUTTON_ACTIVATE_PRIMARY) &&
            !snapshot.IsButtonDown(GAME_BUTTON_ACTIVATE_SECONDARY) &&
            !snapshot.IsButtonDown(GAME_BUTTON_ACTIVATE_TERTIARY))
            s_Move.iMoveFlags &= ~MOVE_ATTACK_HELD;

        // Check for a dash
        bool bWasDashing((s_Move.iMoveFlags & PLAYER_MOVE_DASH) != 0);

        if (CanDash() && !bWasDashing && snapshot.IsButtonDown(GAME_BUTTON_DASH))
        {
            s_Move.iMoveFlags |= PLAYER_MOVE_DASH;
            m_uiDashStartTime = Game.GetGameTime();
            m_uiLastDashTime = INVALID_TIME;
            m_iDashStateSlot = ApplyState(State_Dash, Game.GetGameTime(), -1);
            DrainStamina(p_dashActivateStaminaCost);
        }

        if (s_Move.iMoveFlags & PLAYER_MOVE_DASH)
        {
            DrainStamina(p_dashStaminaCost * fFrameTime);
            if (p_dashRequireStamina && GetStamina() <= 0.0f)
            {
                m_uiDashStartTime = 0;
                s_Move.iMoveFlags &= ~PLAYER_MOVE_DASH;
                if (m_iDashStateSlot != -1 && GetState(m_iDashStateSlot) != NULL && GetState(m_iDashStateSlot)->GetType() == State_Dash)
                    RemoveState(m_iDashStateSlot);
                m_iDashStateSlot = -1;
            }
        }

        if ((snapshot.GetButtonStatus(GAME_BUTTON_FORWARD) & GAME_BUTTON_STATUS_UP) ||
            Game.GetGameTime() - m_uiDashStartTime >= p_dashMaxTime)
        {
            m_uiDashStartTime = 0;
            s_Move.iMoveFlags &= ~PLAYER_MOVE_DASH;
            if (m_iDashStateSlot != -1 && GetState(m_iDashStateSlot) != NULL && GetState(m_iDashStateSlot)->GetType() == State_Dash)
                RemoveState(m_iDashStateSlot);
            m_iDashStateSlot = -1;
        }

        if (bWasDashing && !(s_Move.iMoveFlags & PLAYER_MOVE_DASH))
            m_uiLastDashTime = Game.GetGameTime();

        // Jumping
        float fJumpLimiter(p_jumpLimiter);
        if (IsExhausted())
            fJumpLimiter *= p_speed;
        else
            fJumpLimiter *= p_speedSprint;

        if (snapshot.IsButtonDown(GAME_BUTTON_UP) &&
            !(s_Move.iMoveFlags & MOVE_JUMP_HELD) &&
            s_Move.bGroundControl &&
            !(m_iCurrentAction & PLAYER_ACTION_IMMOBILE) &&
            Clip(s_Move.v3Velocity, s_Move.plGround.v3Normal).Length() <= s_Move.fRunSpeed * fJumpLimiter &&
            !GetIsVehicle() &&
            (!p_jumpStaminaRequired || GetStamina() >= p_staminaJumpCost))
        {
            s_Move.v3Velocity.z = s_Move.v3Velocity.z * p_jumpSlopeScale + p_jump;
            s_Move.iMoveFlags &= ~MOVE_ON_GROUND;
            s_Move.bGroundControl = false;
            s_Move.iMoveFlags |= MOVE_JUMP_HELD;
            s_Move.iMoveFlags |= MOVE_JUMPING;
            DrainStamina(p_staminaJumpCost);

            StartAnimation(_T("jump"), 0);

            int iJumpIdle(GetAnimIndex(_T("jump_idle")));
            if (iJumpIdle != -1)
                m_yDefaultAnim = iJumpIdle;
        }

        // Movement direction   
        CVec3f v3Intent(V_ZERO);
        if (!(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
        {
            if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
                v3Intent += v3Forward;
            if (snapshot.IsButtonDown(GAME_BUTTON_BACK) && !(s_Move.iMoveFlags & PLAYER_MOVE_DASH))
                v3Intent -= v3Forward;
            
            if (!GetIsVehicle() && !(s_Move.iMoveFlags & PLAYER_MOVE_DASH))
            {
                if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                    v3Intent += v3Right;
                if (snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                    v3Intent -= v3Right;
            }
            v3Intent.Normalize();
        }

        // Set sprinting flag
        float fCost(fFrameTime * p_staminaSprintCost);
        if (snapshot.IsButtonDown(GAME_BUTTON_SPRINT) &&
            v3Intent.Length() > 0.0f &&
            !(m_iCurrentAction & PLAYER_ACTION_BLOCK) &&
            m_fStamina > 0.0f &&
            !(s_Move.iMoveFlags & PLAYER_MOVE_DASH) &&
            !GetIsVehicle())
        {
            iNewMovement |= PLAYER_MOVE_SPRINT;
        }

        // Apply speed modifiers
        if (s_Move.iMoveFlags & PLAYER_MOVE_DASH)
        {
            s_Move.fRunSpeed *= p_dashSpeed;
        }
        else if (s_Move.bGroundControl || p_jumpAppliesSpeedModifiers)
        {
            if (snapshot.IsButtonDown(GAME_BUTTON_BACK))
                s_Move.fRunSpeed *= p_speedBackwards;
            else if (snapshot.IsButtonDown(GAME_BUTTON_LEFT) || snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                s_Move.fRunSpeed *= p_speedStrafe;

            if (IsExhausted())
                s_Move.fRunSpeed *= p_speedExhausted;
            
            if (iNewMovement & PLAYER_MOVE_SPRINT && s_Move.bGroundControl)
            {
                s_Move.fRunSpeed *= p_speedSprint;
                DrainStamina(fCost);
            }
        }

        // Adjust speed for current actions
        if ((m_iCurrentAction & PLAYER_ACTION_BLOCK) && !g_meleeNewBlock)
            s_Move.fRunSpeed *= p_speedBlocking;

        // Apply speed mult of current item and all backpack items
        if (GetCurrentItem() != NULL)
            s_Move.fRunSpeed *= GetCurrentItem()->GetSpeedMult();

        // Apply speed mult of all items in the backpack
        for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK; i++)
        {
            IInventoryItem *pItem(GetItem(i));

            if (pItem != NULL)
            {
                // Consumable items count the number per stack as well
                if (pItem->IsConsumable())
                    s_Move.fRunSpeed *= pow(pItem->GetSpeedMult(), pItem->GetAmmoCount());
                else
                    s_Move.fRunSpeed *= GetItem(i)->GetSpeedMult();
            }
        }
        
        if (Game.GetGameTime() - m_uiLastMeleeAttackTime < p_speedMeleeAttackTime)
            s_Move.fRunSpeed *= LERP((Game.GetGameTime() - m_uiLastMeleeAttackTime) / p_speedMeleeAttackTime.GetFloat(), p_speedMeleeAttack.GetFloat(), 1.0f);

        s_Move.fRunSpeed = CLAMP(s_Move.fRunSpeed, 0.0f, p_speedLimit * p_speed);

        v3Intent *= s_Move.fRunSpeed;
        s_Move.v3Intent = v3Intent;

        if (Game.IsClient())
            m_fCurrentSpeed = s_Move.fRunSpeed;

        if (s_Move.bGroundControl)
            MoveWalkGround();
        else
            MoveWalkAir();

        CheckGround();

        if (Game.IsServer() && p_debugMovement)
        {
            Console << _T("Pos: ") << s_Move.v3Position
                    << _T(" Vel: ") << s_Move.v3Velocity
                    << _T(" Speed: ") << s_Move.v3Velocity.Length()
                    << _T(" Ground: ") << (s_Move.iMoveFlags & MOVE_JUMP_HELD)
                    << _T(" Ground Plane: ") << s_Move.plGround.v3Normal << _T(" ") << s_Move.plGround.fDist << newl;
        }

        // Apply this movement
        if (IsPositionValid(s_Move.v3Position))
        {
            m_v3Position = s_Move.v3Position;
            m_v3Velocity = s_Move.v3Velocity;
            m_iMoveFlags = s_Move.iMoveFlags;
        }
        else
        {
            if (Game.IsServer() && p_debugMovement)
                Console << _T("Player move got stuck") << newl;
        }

        m_v3Angles = s_Move.v3Angles;

        Link();

        // Movement
        if (s_Move.iMoveFlags & PLAYER_MOVE_DASH)
        {
            iNewMovement = PLAYER_MOVE_FWD;
        }
        else if (GetIsVehicle())
        {
            if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD) && !snapshot.IsButtonDown(GAME_BUTTON_BACK))
            {
                iNewMovement = PLAYER_MOVE_FWD;
                m_uiWheelTime += snapshot.GetFrameLength();
            }
            else if (snapshot.IsButtonDown(GAME_BUTTON_BACK) && !snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
            {
                iNewMovement = PLAYER_MOVE_BACK;
                m_uiWheelTime -= snapshot.GetFrameLength();
            }
        }
        else if (!(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
        {
            if (m_iMoveFlags & MOVE_JUMPING)
            {
                iNewMovement = PLAYER_MOVE_JUMP;
            }
            else
            {
                if (snapshot.IsButtonDown(GAME_BUTTON_FORWARD) && !snapshot.IsButtonDown(GAME_BUTTON_BACK))
                    iNewMovement |= PLAYER_MOVE_FWD;
                else if (snapshot.IsButtonDown(GAME_BUTTON_BACK) && !snapshot.IsButtonDown(GAME_BUTTON_FORWARD))
                    iNewMovement |= PLAYER_MOVE_BACK;
                if (snapshot.IsButtonDown(GAME_BUTTON_LEFT) && !snapshot.IsButtonDown(GAME_BUTTON_RIGHT))
                    iNewMovement |= PLAYER_MOVE_LEFT;
                else if (snapshot.IsButtonDown(GAME_BUTTON_RIGHT) && !snapshot.IsButtonDown(GAME_BUTTON_LEFT))
                    iNewMovement |= PLAYER_MOVE_RIGHT;
            }
        }

        if (iNewMovement == PLAYER_MOVE_IDLE && IsExhausted())
            iNewMovement |= PLAYER_MOVE_TIRED;

        if (snapshot.GetTimeStamp() - m_uiLastActionTime > p_restTime)
            iNewMovement |= PLAYER_MOVE_RESTING;

        if (Game.IsServer())
        {
            // Stamina regeneration
            int iTestMovement(iNewMovement);
            if (snapshot.IsButtonDown(GAME_BUTTON_SPRINT) &&
                (v3Intent.Length() > 0.0f || ((m_iCurrentAction & PLAYER_ACTION_BLOCK) && !g_meleeNewBlock)) &&
                !GetIsVehicle() &&
                !(s_Move.iMoveFlags & PLAYER_MOVE_DASH))
            {
                iTestMovement |= PLAYER_MOVE_SPRINT;
            }

            if (s_Move.iMoveFlags & PLAYER_MOVE_DASH)
                iTestMovement |= PLAYER_MOVE_DASH;

            RegenerateStamina(fFrameTime, iTestMovement);
        }

        // Animation
        float fNewRelativeSpeed(s_Move.fRunSpeed / m_pEntityConfig->GetSpeed());

        if (!(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
        {
            tstring sAnimName;
            float fAnimSpeed(1.0f);

            switch ((iNewMovement & PLAYER_MOVE_NO_FLAGS) &~ PLAYER_MOVE_IGNORE_FLAGS)
            {
            case PLAYER_MOVE_IMMOBILE:
                // Don't alter animations
                break;

            case PLAYER_MOVE_JUMP:
                //sAnimName = _T("jump");
                break;

            case PLAYER_MOVE_IDLE:
                if (GetCurrentItem() != NULL && GetCurrentItem()->IsGun())
                {           
                    sAnimName = _T("gun_idle");
                    m_yDefaultAnim = GetAnimIndex(_T("gun_idle"));
                }
                else if (iNewMovement & PLAYER_MOVE_TIRED)
                {
                    sAnimName = _T("exhausted");
                    m_yDefaultAnim = GetAnimIndex(_T("exhausted"));
                }
                else
                {
                    sAnimName = _T("idle");
                    m_yDefaultAnim = GetAnimIndex(_T("idle"));
                }
                break;

            case PLAYER_MOVE_FWD:
            case PLAYER_MOVE_FWD_LEFT:
            case PLAYER_MOVE_FWD_RIGHT:
                if (m_iMoveFlags & PLAYER_MOVE_DASH)
                {
                    sAnimName = _T("sprint_fwd");
                    fAnimSpeed = fNewRelativeSpeed / p_dashSpeed;
                }
                else if (iNewMovement & PLAYER_MOVE_SPRINT)
                {
                    sAnimName = _T("sprint_fwd");
                    fAnimSpeed = fNewRelativeSpeed / p_speedSprint;
                }
                else
                {
                    sAnimName = _T("run_fwd");
                    fAnimSpeed = fNewRelativeSpeed;
                }
                break;

            case PLAYER_MOVE_BACK:
            case PLAYER_MOVE_BACK_LEFT:
            case PLAYER_MOVE_BACK_RIGHT:
                sAnimName = _T("run_back");
                fAnimSpeed = fNewRelativeSpeed;
                break;

            case PLAYER_MOVE_LEFT:
                if (m_pEntityConfig->GetYawStrafe())
                {
                    if (iNewMovement & PLAYER_MOVE_SPRINT)
                    {
                        sAnimName = _T("sprint_fwd");
                        fAnimSpeed = fNewRelativeSpeed / p_speedSprint;
                    }
                    else
                    {
                        sAnimName = _T("run_fwd");
                        fAnimSpeed = fNewRelativeSpeed;
                    }
                }
                else
                {
                    sAnimName = _T("run_left");
                    fAnimSpeed = fNewRelativeSpeed;
                }
                break;

            case PLAYER_MOVE_RIGHT:
                if (m_pEntityConfig->GetYawStrafe())
                {
                    if (iNewMovement & PLAYER_MOVE_SPRINT)
                    {
                        sAnimName = _T("sprint_fwd");
                        fAnimSpeed = fNewRelativeSpeed / p_speedSprint;
                    }
                    else
                    {
                        sAnimName = _T("run_fwd");
                        fAnimSpeed = fNewRelativeSpeed;
                    }
                }
                else
                {
                    sAnimName = _T("run_right");
                    fAnimSpeed = fNewRelativeSpeed;
                }
                break;
            }

            const int iChannel(0);

            if (!sAnimName.empty() && (m_auiAnimLockTime[iChannel] == INVALID_TIME || m_auiAnimLockTime[iChannel] <= Game.GetGameTime()))
            {
                m_ayAnim[iChannel] = g_ResourceManager.GetAnim(GetModelHandle(), sAnimName);
                m_afAnimSpeed[iChannel] = fAnimSpeed;
            }

            m_iCurrentMovement = iNewMovement;
        }

        // Land Animation
        if (s_Move.bLanded && (iNewMovement & PLAYER_MOVE_NO_FLAGS) == PLAYER_MOVE_IDLE)
        {
            StartAnimation(_T("land"), 0);
        }

        // Hold effect
        IInventoryItem *pItem(GetCurrentItem());
        if (pItem != NULL && !pItem->GetHoldEffect().empty())
        {
            ResHandle hEffect(Game.RegisterEffect(pItem->GetHoldEffect()));
            if (GetEffect(EFFECT_CHANNEL_HELD_ITEM) != hEffect)
            {
                SetEffect(EFFECT_CHANNEL_HELD_ITEM, hEffect);
                IncEffectSequence(EFFECT_CHANNEL_HELD_ITEM);
            }
        }
        else if (GetEffect(EFFECT_CHANNEL_HELD_ITEM) != INVALID_RESOURCE)
        {
            SetEffect(EFFECT_CHANNEL_HELD_ITEM, INVALID_RESOURCE);
            IncEffectSequence(EFFECT_CHANNEL_HELD_ITEM);
        }

        // Set Fov
        if (pItem != NULL)
            SetFov(pItem->GetFov());
        else
            SetFov(GetBaseFov());

        // Check if we've fallen outside the world
        if (Game.IsServer() && GetStatus() == ENTITY_STATUS_ACTIVE)
        {
            CBBoxf bbWorld(Game.GetWorldPointer()->GetTerrainBounds());
            bbWorld.Set(bbWorld.GetMin() + CVec3f(-100.0f, -100.0f, -200.0f), bbWorld.GetMax() + CVec3f(100.0f, 100.0f, 10000.0f));

            if (!IntersectBounds(bbWorld, m_bbBounds + m_v3Position))
                Spawn3();
        }

        // Repair
        if (!IsIntangible())
            Repair(snapshot.IsButtonDown(GAME_BUTTON_REPAIR));

        // Actions
        if (m_iCurrentAction & (PLAYER_ACTION_ATTACK | PLAYER_ACTION_BLOCK))
            m_attack.TryImpact(snapshot.GetTimeStamp());
        
        if (m_iCurrentAction & PLAYER_ACTION_SKILL)
        {
            bool bActivated(m_skillActivate.TryImpact());

            if (Game.IsServer() && bActivated)
            {
                CBufferFixed<4> buffer;

                buffer << GAME_CMD_ITEM_SUCCEEDED << GetCurrentItem()->GetSlot() << GetCurrentItem()->GetType();
                Game.SendGameData(GetClientID(), buffer, false);
            }
        }
        if (m_iCurrentAction & PLAYER_ACTION_SPELL)
        {
            bool bActivated(m_spellActivate.TryImpact());
            
            if (Game.IsServer() && bActivated)
            {
                CBufferFixed<4> buffer;

                buffer << (m_spellActivate.GetSucceeded() ? GAME_CMD_ITEM_SUCCEEDED : GAME_CMD_ITEM_FAILED) << GetCurrentItem()->GetSlot() << GetCurrentItem()->GetType();
                Game.SendGameData(GetClientID(), buffer, false);
            }
        }

        // Check for expired actions before trying to start new ones
        if (!IsIdle() && m_uiCurrentActionEndTime <= snapshot.GetTimeStamp())
        {
            int iFinishedAction(m_iCurrentAction);
            if (GetCurrentItem() != NULL)
                GetCurrentItem()->FinishedAction(iFinishedAction);

            if (iFinishedAction & PLAYER_ACTION_SACRIFICING)
            {
                SetNetFlags(ENT_NET_FLAG_SACRIFICED);
                IPlayerEntity *pNewUnit(Game.ChangeUnit(m_iClientNum, m_unNextUnit, CHANGE_UNIT_KILL | CHANGE_UNIT_INHERIT_POS | CHANGE_UNIT_SPAWN | CHANGE_UNIT_INHERIT_DAMAGE_RECORD));
                if (pNewUnit != NULL)
                    pNewUnit->RemoveNetFlags(ENT_NET_FLAG_SACRIFICED);
                else
                    RemoveNetFlags(ENT_NET_FLAG_SACRIFICED);

                SetNextUnit(0);

                SetAction(PLAYER_ACTION_IDLE, -1);
            }
        }

        // Inventory activation
        byte yActivate(snapshot.GetActivate());
        if (yActivate != NO_SELECTION && yActivate < MAX_INVENTORY && m_apInventory[yActivate] != NULL)
            m_apInventory[yActivate]->Activate();

        // Inventory activation
        if (GetCurrentItem() != NULL && !GetCurrentItem()->IsDisabled())
        {
            if (!ActivatePrimary(m_ySelectedItem, snapshot.GetButtonStatus(GAME_BUTTON_ACTIVATE_PRIMARY)))
            {
                if (!ActivateSecondary(m_ySelectedItem, snapshot.GetButtonStatus(GAME_BUTTON_ACTIVATE_SECONDARY)))
                {
                    if (!ActivateTertiary(m_ySelectedItem, snapshot.GetButtonStatus(GAME_BUTTON_ACTIVATE_TERTIARY)))
                        Cancel(m_ySelectedItem, snapshot.GetButtonStatus(GAME_BUTTON_CANCEL));
                }
            }
        }

        // Use
        if (snapshot.ButtonPressed(GAME_BUTTON_USE) && !IsIntangible())
        {
            // Try to "use" any active states
            for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
            {
                if (m_apState[i] == NULL)
                    continue;
                m_apState[i]->Use(this);
            }

            // Check the ground, then in front of the player for an entity they can interact with
            IGameEntity *pGroundEntity(Game.GetEntityFromWorldIndex(m_uiGroundEntityIndex));
            if (pGroundEntity != NULL)
            {
                pGroundEntity->Use(this);
            }
            else
            {
                CAxis axis(m_v3Angles);
                CVec3f v3Forward(axis.Forward());
                CVec3f v3Start(m_v3Position + V_UP * GetViewHeight() + v3Forward * 30.0f);
                CVec3f v3End(v3Start + v3Forward * 100.0f);

                STraceInfo result;
                Game.TraceLine(result, v3Start, v3End, 0, m_uiWorldIndex);
                if (result.uiEntityIndex != INVALID_INDEX)
                {
                    IGameEntity *pEntity(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
                    if (pEntity != NULL)
                        pEntity->Use(this);
                }
            }
        }

        // Pick up items
        STraceInfo trace;
        if (Game.TraceBox(trace, m_v3Position, m_v3Position - CVec3f(0.0, 0.0, p_groundEpsilon), m_bbBounds, ~(SURF_ITEM | SURF_BOUNDS), m_uiWorldIndex))
        {
            if (trace.uiEntityIndex != INVALID_INDEX)
            {
                IGameEntity *pTouch(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));
                if (pTouch)
                    pTouch->Touch(this);
            }
        }

        if (m_iCurrentAction != PLAYER_ACTION_IDLE ||
            (m_iCurrentMovement & ~PLAYER_MOVE_RESTING) != PLAYER_MOVE_IDLE)
            m_uiLastActionTime = snapshot.GetTimeStamp();
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPlayerEntity::MoveWalk() - "));
    }
}


/*====================
  IPlayerEntity::Heal
  ====================*/
float   IPlayerEntity::Heal(float fHealth, IVisualEntity *pSource)
{
    fHealth = MIN(fHealth, GetMaxHealth() - GetHealth());
    SetHealth(CLAMP(GetHealth() + fHealth, 0.0f, GetMaxHealth()));

    if (pSource != NULL && pSource->GetAsPlayerEnt() != NULL && fHealth > 0.0f)
    {
        Game.MatchStatEvent(pSource->GetAsPlayerEnt()->GetClientID(), PLAYER_MATCH_HEALED, fHealth, GetClientID(), (pSource != NULL ? pSource->GetType() : INVALID_ENT_TYPE));
        Game.MatchStatEvent(pSource->GetAsPlayerEnt()->GetClientID(), COMMANDER_MATCH_HEALED, fHealth, GetClientID(), (pSource != NULL ? pSource->GetType() : INVALID_ENT_TYPE));
    }

    return MAX(fHealth, 0.0f);
}


/*====================
  IPlayerEntity::Damage
  ====================*/
float   IPlayerEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
    float damage = ICombatEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);

    if (m_uiNextPainTime < Host.GetTime() && damage > 0 && !m_pEntityConfig->GetPainEffectPath().empty())
    {
        CGameEvent evPain;
        evPain.SetSourceEntity(GetIndex());
        evPain.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetPainEffectPath()));
        Game.AddEvent(evPain);
        m_uiNextPainTime = Host.GetTime() + 2000;
    }

    return damage;
}


/*====================
  IPlayerEntity::Copy
  ====================*/
void    IPlayerEntity::Copy(const IGameEntity &B)
{
    ICombatEntity::Copy(B);

    const IPlayerEntity *pB(B.GetAsPlayerEnt());

    if (!pB)    
        return;

    const IPlayerEntity &C(*pB);

    m_iClientNum =                  C.m_iClientNum;

    m_yFirstPersonAnim =            C.m_yFirstPersonAnim;
    m_yFirstPersonAnimSequence =    C.m_yFirstPersonAnimSequence;
    m_fFirstPersonAnimSpeed =       C.m_fFirstPersonAnimSpeed;

    m_iCurrentMovement =            C.m_iCurrentMovement;
    m_uiDeathTime =                 C.m_uiDeathTime;
    
    m_iMoveFlags =                  C.m_iMoveFlags;
    m_uiWheelTime =                 C.m_uiWheelTime;

    m_uiPreviewBuildingIndex =      C.m_uiPreviewBuildingIndex;
    m_v2Cursor =                    C.m_v2Cursor;

    m_yOrderUniqueID =              C.m_yOrderUniqueID;
    m_yCurrentOrder =               C.m_yCurrentOrder;
    m_uiCurrentOrderEntIndex =      C.m_uiCurrentOrderEntIndex;
    m_v3CurrentOrderPos =           C.m_v3CurrentOrderPos;

    m_yOfficerOrder =               C.m_yOfficerOrder;
    m_yOfficerOrderSequence =       C.m_yOfficerOrderSequence;
    m_uiOfficerOrderEntIndex =      C.m_uiOfficerOrderEntIndex;
    m_v3OfficerOrderPos =           C.m_v3OfficerOrderPos;

    m_uiPetIndex =                  C.m_uiPetIndex;

    m_fCurrentSpeed =               C.m_fCurrentSpeed;

    m_uiDashStartTime =             C.m_uiDashStartTime;
    m_uiLastDashTime =              C.m_uiLastDashTime;

    m_fFov =                        C.m_fFov;
}


/*====================
  IPlayerEntity::ClientPrecache
  ====================*/
void    IPlayerEntity::ClientPrecache(CEntityConfig *pConfig)
{
    EntityRegistry.ClientPrecache(Skill_HumanOfficerPortal);
    EntityRegistry.ClientPrecache(Spell_BeastOfficerPortal);
    EntityRegistry.ClientPrecache(State_Spawned);
    g_ResourceManager.Register(_T("/shared/effects/levelup.effect"), RES_EFFECT);
    g_ResourceManager.Register(K2_NEW(g_heapResources,   CTexture)(_T("/shared/textures/icons/officer_star.tga"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    ICombatEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetSpawnEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetSpawnEffectPath(), RES_EFFECT);

    if (!pConfig->GetPainEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetPainEffectPath(), RES_EFFECT);
}


/*====================
  IPlayerEntity::ServerPrecache
  ====================*/
void    IPlayerEntity::ServerPrecache(CEntityConfig *pConfig)
{
    EntityRegistry.ServerPrecache(Skill_HumanOfficerPortal);
    EntityRegistry.ServerPrecache(Spell_BeastOfficerPortal);
    EntityRegistry.ServerPrecache(State_Spawned);
    g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(_T("/shared/effects/levelup.effect"), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    ICombatEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetSpawnEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetSpawnEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    if (!pConfig->GetPainEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetPainEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IPlayerEntity::PetCommand
  ====================*/
void    IPlayerEntity::PetCommand(EPetCommand ePetCmd, uint uiIndex, const CVec3f &v3Pos)
{
    IGameEntity *pEnt(Game.GetEntity(m_uiPetIndex));
    if (!pEnt || !pEnt->IsPet())
        return;

    pEnt->GetAsPet()->PlayerCommand(ePetCmd, uiIndex, v3Pos);
}


/*====================
  IPlayerEntity::OfficerCommand
  ====================*/
void    IPlayerEntity::OfficerCommand(EOfficerCommand eOfficerCmd, uint uiIndex, const CVec3f &v3Pos)
{
    CEntityTeamInfo *pTeam(Game.GetTeam(m_iTeam));
    if (pTeam == NULL)
        return;

    ivector viClients(pTeam->GetClientList());
    for (ivector::iterator it(viClients.begin()); it != viClients.end(); ++it)
    {
        IPlayerEntity *pOther(Game.GetPlayerEntityFromClientID(*it));
        if (pOther == NULL || pOther->GetSquad() != m_ySquad)
            continue;

        pOther->SetOfficerOrder(eOfficerCmd);
        pOther->SetOfficerOrderEntIndex(uiIndex);
        pOther->SetOfficerOrderPos(v3Pos);
    }
}


/*====================
  IPlayerEntity::StartFirstPersonAnimation
  ====================*/
void    IPlayerEntity::StartFirstPersonAnimation(const tstring &sAnimName, float fSpeed, uint uiLength)
{
    m_yFirstPersonAnim = g_ResourceManager.GetAnim(m_hFirstPersonModel, sAnimName);
    m_yFirstPersonAnimSequence = (m_yFirstPersonAnimSequence + 1) & 0xff;

    float fLength(uiLength != 0 ? float(g_ResourceManager.GetAnimLength(m_hFirstPersonModel, m_yFirstPersonAnim)) / uiLength : 1.0f);
    m_fFirstPersonAnimSpeed = fSpeed * fLength;
}


/*====================
  IPlayerEntity::StopFirstPersonAnimation
  ====================*/
void    IPlayerEntity::StopFirstPersonAnimation()
{
    m_yFirstPersonAnim = ENTITY_STOP_ANIM;
    m_yFirstPersonAnimSequence = (m_yFirstPersonAnimSequence + 1) & 0xff;
    m_fFirstPersonAnimSpeed = 1.0f;
}


/*====================
  IPlayerEntity::StopFirstPersonAnimation
  ====================*/
void    IPlayerEntity::StopFirstPersonAnimation(const tstring &sAnimName)
{
    if (m_yFirstPersonAnim == g_ResourceManager.GetAnim(m_hFirstPersonModel, sAnimName))
    {
        m_yFirstPersonAnim = ENTITY_STOP_ANIM;
        m_yFirstPersonAnimSequence = (m_yFirstPersonAnimSequence + 1) & 0xff;
        m_fFirstPersonAnimSpeed = 1.0f;
    }
}


/*====================
  IPlayerEntity::UpdateFirstPersonSkeleton
  ====================*/
void    IPlayerEntity::UpdateFirstPersonSkeleton()
{
    if (m_pFirstPersonSkeleton == NULL)
        return;

    m_pFirstPersonSkeleton->SetModel(m_hFirstPersonModel);

    if (m_hFirstPersonModel == INVALID_RESOURCE)
        return;

    // Pose skeleton
    m_pFirstPersonSkeleton->Pose(Game.GetGameTime());

    // Process animation events
    if (m_pFirstPersonSkeleton->CheckEvents())
    {
        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(m_hFirstPersonModel)));

        const vector<SAnimEventCmd> &vEventCmds(m_pFirstPersonSkeleton->GetEventCmds());

        for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
            EventScript.Execute(it->sCmd, it->iTimeNudge);

        m_pFirstPersonSkeleton->ClearEvents();

        FileManager.SetWorkingDirectory(sOldDir);
    }
}


/*====================
  IPlayerEntity::AddFirstPersonToScene
  ====================*/
bool    IPlayerEntity::AddFirstPersonToScene(const CCamera &camera)
{
    if (GetStatus() == ENTITY_STATUS_SPAWNING ||
        GetStatus() == ENTITY_STATUS_DORMANT)
        return false;

    if (m_hFirstPersonModel == INVALID_INDEX)
        return false;

    CSceneEntity sceneEntity;
    CAxis axAngles(m_v3FirstPersonModelAngles);

    sceneEntity.Clear();

    sceneEntity.axis = (camera.GetViewAxis() * axAngles);
    sceneEntity.SetPosition(camera.GetOrigin() + sceneEntity.axis * m_v3FirstPersonModelOffset);
    sceneEntity.flags |= SCENEOBJ_USE_AXIS;
    sceneEntity.hModel = m_hFirstPersonModel;
    sceneEntity.skeleton = m_pFirstPersonSkeleton;
    SceneManager.AddEntity(sceneEntity);

    return true;
}


/*====================
  IPlayerEntity::LocalClientFrame
  ====================*/
void    IPlayerEntity::LocalClientFrame()
{
    IInventoryItem *pItem(GetCurrentItem());
    if (pItem)
        pItem->LocalClientFrame();
}


/*====================
  IPlayerEntity::ActivatePrimary
  ====================*/
bool    IPlayerEntity::ActivatePrimary(int iSlot, int iButtonStatus)
{
    bool bReturn(ICombatEntity::ActivatePrimary(iSlot, iButtonStatus));

    if (bReturn)
    {
        Game.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
        Game.RegisterTriggerParam(_T("name"), GetClientName());
        Game.RegisterTriggerParam(_T("clientid"), XtoA(GetClientID()));
        Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));

        if (GetItem(iSlot) != NULL)
            Game.RegisterTriggerParam(_T("itemname"), GetItem(iSlot)->GetName());

        Game.TriggerGlobalScript(_T("activateprimary"));
    }

    for (int i = 0; i < MAX_INVENTORY; ++i)
    {
        if (i == iSlot)
            continue;
        IInventoryItem *pItem = GetItem(i);
        if (pItem == NULL)
            continue;
        if (pItem->GetAsSkill())
        {
            ISkillItem *pSkill(pItem->GetAsSkill());

            if (pSkill->IsToggleSkill())
            {
                ISkillToggle *pToggleSkill(pSkill->GetAsSkillToggle());
                pToggleSkill->ActiveFrame();
            }
            
        }
        //Polymorph is the only toggle spell right now, the rest are setup as skills
        //This checks if the item from the loop is polymorph, if it is it runs its ActiveFrame
        //This is not a very generalized way to handle it.. we may want to make this more generic. For now it fixes the bug.
        //We could also setup polymorph as a skill, but G said not to worry about it for right now.

        if (pItem->GetAsSpell())
        {
            ISpellItem *pSpell(pItem->GetAsSpell());

            if (pSpell->IsSpellToggle())
            {
                CSpellPolymorph *pSpellPolymorph(pSpell->GetAsSpellPolymorph());
                pSpellPolymorph->ActiveFrame();
            }
        }
                

    }


    return bReturn;
}


/*====================
  IPlayerEntity::ActivateSecondary
  ====================*/
bool    IPlayerEntity::ActivateSecondary(int iSlot, int iButtonStatus)
{
    bool bReturn;

    bReturn = ICombatEntity::ActivateSecondary(iSlot, iButtonStatus);

    if (bReturn)
    {
        Game.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
        Game.RegisterTriggerParam(_T("name"), GetClientName());
        Game.RegisterTriggerParam(_T("clientid"), XtoA(GetClientID()));
        Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));

        if (GetItem(iSlot) != NULL)
            Game.RegisterTriggerParam(_T("itemname"), GetItem(iSlot)->GetName());

        Game.TriggerGlobalScript(_T("activatesecondary"));
    }

    return bReturn;
}


/*====================
  IPlayerEntity::ActivateTertiary
  ====================*/
bool    IPlayerEntity::ActivateTertiary(int iSlot, int iButtonStatus)
{
    bool bReturn;

    bReturn = ICombatEntity::ActivateTertiary(iSlot, iButtonStatus);

    if (bReturn)
    {
        Game.RegisterTriggerParam(_T("slot"), XtoA(iSlot + 1));
        Game.RegisterTriggerParam(_T("name"), GetClientName());
        Game.RegisterTriggerParam(_T("clientid"), XtoA(GetClientID()));
        Game.RegisterTriggerParam(_T("index"), XtoA(GetIndex()));

        if (GetItem(iSlot) != NULL)
            Game.RegisterTriggerParam(_T("itemname"), GetItem(iSlot)->GetName());

        Game.TriggerGlobalScript(_T("activatetertiary"));
    }

    return bReturn;
}


/*====================
  IPlayerEntity::UpdateEffectThread
  ====================*/
void    IPlayerEntity::UpdateEffectThread(CEffectThread *pEffectThread)
{
    pEffectThread->SetSourceModel(g_ResourceManager.GetModel(GetModelHandle()));
    pEffectThread->SetSourceSkeleton(m_pSkeleton);

    pEffectThread->SetSourcePos(m_v3Position);

    pEffectThread->SetSourceAxis(CAxis(0.0f, 0.0f, m_fCurrentYaw));

    if (pEffectThread->GetUseEntityEffectScale())
        pEffectThread->SetSourceScale(m_fScale * GetScale2() * GetEffectScale());
    else
        pEffectThread->SetSourceScale(m_fScale * GetScale2());
}


/*====================
  IPlayerEntity::DeleteOrder
  ====================*/
void    IPlayerEntity::DeleteOrder(byte ySequence)
{
    if (ySequence >= GetNumOrders())
        return;

    m_vecOrders.erase(m_vecOrders.begin() + ySequence);
    m_vecOrderEntIndex.erase(m_vecOrderEntIndex.begin() + ySequence);
    m_vecOrderPos.erase(m_vecOrderPos.begin() + ySequence);

    // If the current order was deleted, increment the order
    if (ySequence == 0)
    {
        m_yOrderUniqueID++;

        m_yCurrentOrder = GetOrder(0);
        m_uiCurrentOrderEntIndex = GetOrderEntIndex(0);
        m_v3CurrentOrderPos = GetOrderPos(0);
    }
}


/*====================
  IPlayerEntity::IncOrderSequence
  ====================*/
void    IPlayerEntity::IncOrderSequence()
{
    DeleteOrder(0);
}


/*====================
  IPlayerEntity::AddOrder
  ====================*/
void    IPlayerEntity::AddOrder(byte yOrder, uint uiOrderEntIndex, const CVec3f &v3OrderPos)
{
    if (GetNumOrders() >= 256)
        return;

    m_vecOrders.push_back(yOrder);
    m_vecOrderEntIndex.push_back(uiOrderEntIndex);
    m_vecOrderPos.push_back(v3OrderPos);

    if (GetNumOrders() == 1)
    {
        m_yCurrentOrder = GetOrder(0);
        m_uiCurrentOrderEntIndex = GetOrderEntIndex(0);
        m_v3CurrentOrderPos = GetOrderPos(0);

        m_yOrderUniqueID++;
    }
}


/*====================
  IPlayerEntity::ClearOrders
  ====================*/
void    IPlayerEntity::ClearOrders()
{
    m_vecOrders.clear();
    m_vecOrderEntIndex.clear();
    m_vecOrderPos.clear();

    m_yOrderUniqueID++;

    m_yCurrentOrder = CMDR_ORDER_CLEAR;
    m_uiCurrentOrderEntIndex = INVALID_INDEX;
    m_v3CurrentOrderPos = V3_ZERO;
}


/*====================
  IPlayerEntity::Interpolate
  ====================*/
void    IPlayerEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
    if ((pPrevState->GetStatus() == ENTITY_STATUS_SPAWNING || pPrevState->GetStatus() == ENTITY_STATUS_DORMANT) && 
        (pNextState->GetStatus() != ENTITY_STATUS_SPAWNING && pNextState->GetStatus() != ENTITY_STATUS_DORMANT))
    {
        m_v3Angles = pNextState->GetAngles();
        m_v3Position = pNextState->GetPosition();
        m_fScale = pNextState->GetScale();
    }
    else
    {
        m_v3Angles = M_LerpAngles(fLerp, pPrevState->GetAngles(), pNextState->GetAngles());
        m_v3Position = LERP(fLerp, pPrevState->GetPosition(), pNextState->GetPosition());
        m_fScale = LERP(fLerp, pPrevState->GetScale(), pNextState->GetScale());
    }
}
