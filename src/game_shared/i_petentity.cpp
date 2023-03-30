// (C)2007 S2 Games
// i_petentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_petentity.h"
#include "c_entitysoul.h"

#include "../k2/c_worldentity.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_texture.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_effect.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_FLOAT(g_repairRange);
EXTERN_CVAR_FLOAT(p_speedSprint);
EXTERN_CVAR_FLOAT(p_staminaSprintCost);
EXTERN_CVAR_FLOAT(p_staminaRegenExhausted);
EXTERN_CVAR_FLOAT(p_staminaRegenResting);
EXTERN_CVAR_FLOAT(p_staminaRegenWalking);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* IPetEntity::s_pvFields;

CVAR_FLOAT  (pet_groundFriction,            5.0f);
CVAR_FLOAT  (pet_groundAccelerate,          10.0f);
CVAR_FLOAT  (pet_airAccelerate,             1.0f);
CVAR_FLOAT  (pet_slopeFriction,             1000.0f);
CVAR_FLOAT  (pet_turnSpeed,                 360.0f);
CVAR_FLOAT  (pet_maxAggro,                  10.0f);
CVAR_FLOAT  (pet_sightAggro,                10.0f);
CVAR_FLOAT  (pet_sightAdjacentAggro,        0.0f);
CVAR_FLOAT  (pet_damageAggro,               0.2f);
CVAR_FLOAT  (pet_aggroDecayAttacking,       0.3f);
CVAR_FLOAT  (pet_aggroDecayVisible,         1.0f);
CVAR_FLOAT  (pet_aggroDecayInvisible,       2.0f);
CVAR_FLOAT  (pet_aggroDecayDead,            10.0f);
CVAR_FLOAT  (pet_yawStrafeAngle,            75.0f);

CVAR_BOOL   (pet_debugVision,               false);
//=============================================================================


/*====================
  IPetEntity::CEntityConfig::CEntityConfig
  ====================*/
IPetEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
ICombatEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(FollowDistance, 100.0f),
INIT_ENTITY_CVAR(OrderConfirmedSoundPath, _T("")),
INIT_ENTITY_CVAR(PainEffectPath, _T("")),
INIT_ENTITY_CVAR(AggroRadius, 0.0f),
INIT_ENTITY_CVAR(MultiAggroProc, 0.0f),
INIT_ENTITY_CVAR(MultiAggroRadius, 0.0f),
INIT_ENTITY_CVAR(IsVehicle, false),
INIT_ENTITY_CVAR(YawStrafe, true),
INIT_ENTITY_CVAR(SoulLinkDamage, 0.0f),
INIT_ENTITY_CVAR(SoulLinkHealing, 0.0f),
INIT_ENTITY_CVAR(CanBlock, false),
INIT_ENTITY_CVAR(CanStrongAttack, false),
INIT_ENTITY_CVAR(CanQuickAttack, true),
INIT_ENTITY_CVAR(ReactionTime, -1),
INIT_ENTITY_CVAR(SoulChance, 0.0f),
INIT_ENTITY_CVAR(FollowOwner, true)
{
}


/*====================
  IPetEntity::~IPetEntity
  ====================*/
IPetEntity::~IPetEntity()
{
    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }

    if (m_uiOwnerUID != INVALID_INDEX)
    {
        IGameEntity *pOwner(Game.GetEntityFromUniqueID(m_uiOwnerUID));
        if (pOwner && pOwner->IsPlayer() && pOwner->GetAsPlayerEnt()->GetPetIndex() == m_uiIndex)
            pOwner->GetAsPlayerEnt()->SetPetIndex(INVALID_INDEX);

        m_uiOwnerUID = INVALID_INDEX;
    }
}


/*====================
  IPetEntity::IPetEntity
  ====================*/
IPetEntity::IPetEntity(CEntityConfig *pConfig) :
ICombatEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiOwnerUID(INVALID_INDEX),

m_ePetState(PETSTATE_WAITING),
m_ePetJob(PETJOB_IDLE),
m_ePetMode(PETMODE_PASSIVE),
m_hPath(INVALID_POOL_HANDLE),

m_iCurrentMovement(PET_MOVE_IDLE),
m_iMoveFlags(0),
m_v3FaceAngles(V3_ZERO),

m_uiNextAction(-1),

m_uiNextPainTime(0),

m_fBaseYaw(0.0f),
m_fLastYaw(0.0f),
m_fCurrentYaw(0.0f),
m_uiTurnStartTime(0),
m_iTurnAction(PET_MOVE_IDLE),
m_fYawLerpTime(0.0f),
m_fTiltPitch(0.0f),
m_fTiltRoll(0.0f),
m_fTiltHeight(0.0f)
{
}


/*====================
  IPetEntity::Baseline
  ====================*/
void    IPetEntity::Baseline()
{
    ICombatEntity::Baseline();

    m_yDefaultAnim = 0;
    m_iCurrentMovement = 0;
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::GetSnapshot
  ====================*/
void    IPetEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
    // Base entity info
    ICombatEntity::GetSnapshot(snapshot);

    snapshot.AddField(m_yDefaultAnim);
    snapshot.AddField(m_iCurrentMovement);
    snapshot.AddField(byte(m_ePetState));
}


/*====================
  IPetEntity::ReadSnapshot
  ====================*/
bool    IPetEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        // Base entity info
        if (!ICombatEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextField(m_yDefaultAnim);
        snapshot.ReadNextField(m_iCurrentMovement);
        byte yState(m_ePetState);
        snapshot.ReadNextField(yState);
        m_ePetState = EPetState(yState);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IPetEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IPetEntity::GetTypeVector
  ====================*/
const vector<SDataField>&   IPetEntity::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(ICombatEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_yDefaultAnim"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_iCurrentMovement"), FIELD_PUBLIC, TYPE_INT));
        s_pvFields->push_back(SDataField(_T("m_ePetState"), FIELD_PUBLIC, TYPE_CHAR));
    }

    return *s_pvFields;
}


/*====================
  IPetEntity::Copy
  ====================*/
void    IPetEntity::Copy(const IGameEntity &B)
{
    ICombatEntity::Copy(B);

    const IPetEntity *pB(B.GetAsPet());

    if (!pB)    
        return;

    const IPetEntity &C(*pB);

    m_iCurrentMovement =    C.m_iCurrentMovement;
    m_ePetState =           C.m_ePetState;
}


/*====================
  IPetEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IPetEntity::AllocateSkeleton()
{
    return m_pSkeleton = K2_NEW(global,   CSkeleton);
}


/*====================
  IPetEntity::ApplyWorldEntity
  ====================*/
void    IPetEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    m_sName = ent.GetName();
    m_uiWorldIndex = ent.GetIndex();
    m_v3Position = ent.GetPosition();
    m_v3Angles = ent.GetAngles();
    m_iTeam = ent.GetTeam();

    RegisterEntityScripts(ent);
}


/*====================
  IPetEntity::Spawn
  ====================*/
void    IPetEntity::Spawn()
{
    IVisualEntity::Spawn();

    SetStatus(ENTITY_STATUS_ACTIVE);

    SetModelHandle(Game.RegisterModel(GetModelPath()));
    m_bbBounds.SetCylinder(m_pEntityConfig->GetBoundsRadius(), m_pEntityConfig->GetBoundsHeight());
    m_fScale = m_pEntityConfig->GetScale();

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

    m_ySelectedItem = 0;

    m_fHealth = GetMaxHealth();
    m_fMana = GetMaxMana();
    m_fStamina = GetMaxStamina();
    m_ePetState = PETSTATE_WAITING;
    m_ePetMode = PETMODE_AGGRESSIVE;
    m_ePetJob = PETJOB_IDLE;
    m_uiNextActionTime = 0;
    m_yActivate = NO_SELECTION;

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    StartAnimation(_T("idle"), -1);
    m_yDefaultAnim = m_ayAnim[0];

    // Default Job
    CommandGuard(m_v3Position);

    if (!IsPositionValid(m_v3Position))
    {
        STraceInfo trace;
        Game.TraceBox(trace, m_v3Position + CVec3f(0.0f, 0.0f, 1000.0f), m_v3Position + CVec3f(0.0f, 0.0f, -1000.0f), m_bbBounds, TRACE_PLAYER_MOVEMENT);
        m_v3Position = trace.v3EndPos;
    }

    Link(); 
}

/*====================
  IPetEntity::GetStaminaRegen
  ====================*/
float   IPetEntity::GetStaminaRegen(int iMoveState) const
{
    float fBaseRate(GetBaseStaminaRegen());

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        if (iMoveState & PET_MOVE_SPRINT)
            return 0.0f;
        
        if ((iMoveState & PET_MOVE_NO_FLAGS) == PET_MOVE_IDLE)
        {
            if (IsExhausted())
                fBaseRate *= p_staminaRegenExhausted;
            else if (iMoveState & PET_MOVE_RESTING)
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
  IPetEntity::ServerFrame
  ====================*/
bool    IPetEntity::ServerFrame()
{
    if (!ICombatEntity::ServerFrame())
        return false;

    // Check for an expiring corpse
    if (GetStatus() == ENTITY_STATUS_CORPSE && Game.GetGameTime() >= m_uiCorpseTime)
        return false;

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    if (!Game.IsServer())
        return true;

    float fFrameTime(MsToSec(Game.GetFrameLength()));

    // Mana regeneration
    RegenerateMana(fFrameTime);

    // Health regeneration
    RegenerateHealth(fFrameTime);

    // Stamina regeneration
    RegenerateStamina(fFrameTime, m_iCurrentMovement);

    //
    // AI Update
    //

    DecayAggro(fFrameTime);

    bool bGoodJob(false);
    
    switch (m_ePetJob)
    {
    case PETJOB_FOLLOW:
        bGoodJob = Follow();
        break;
    case PETJOB_ATTACK:
        bGoodJob = Attack();
        break;
    case PETJOB_MOVE:
        bGoodJob = Move();
        break;
    case PETJOB_GUARDPOS:
        bGoodJob = Think(false);
        break;
    case PETJOB_IDLE:
        bGoodJob = Idle();
        break;
    case PETJOB_PATROL:
        bGoodJob = Think(true);
        break;
    case PETJOB_REPAIR:
        bGoodJob = TryRepair();
        break;
    }

    if (!bGoodJob)
    {
        if (GetFollowOwner())
            CommandReturn();
        else
            CommandStop();
    }

    CPath *pPath(Game.AccessPath(m_hPath));
    if (!pPath)
        m_hPath = INVALID_POOL_HANDLE;

    if (m_ePetState == PETSTATE_MOVING && pPath)
    {   
        float fDistance(GetSpeed() * fFrameTime);
        CVec2f v2Move(pPath->CalcNearGoal(m_v3Position.xy(), fDistance));
        CVec3f v3EndPos(v2Move.x, v2Move.y, Game.GetTerrainHeight(v2Move.x, v2Move.y));

        if (pPath->AtGoal(m_v3Position.xy(), fDistance))
        {
            MoveWalk(fFrameTime, v3EndPos, true);
            
            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;
        }
        else
        {
            MoveWalk(fFrameTime, v3EndPos, false);
        }
    }
    else
        MoveWalk(fFrameTime, CVec3f(m_v3Position), false);

    if (m_yActivate != NO_SELECTION && m_apInventory[m_yActivate] != NULL)
        m_apInventory[m_yActivate]->Activate();
    m_yActivate = NO_SELECTION;

    // Actions
    if (m_iCurrentAction & PLAYER_ACTION_ATTACK | PLAYER_ACTION_BLOCK)
        m_attack.TryImpact(Game.GetGameTime());
    if (m_iCurrentAction & PLAYER_ACTION_SKILL)
        m_skillActivate.TryImpact();
    if (m_iCurrentAction & PLAYER_ACTION_SPELL)
        m_spellActivate.TryImpact();

    // Check for expired actions before trying to start new ones
    if (!IsIdle() && m_uiCurrentActionEndTime <= Game.GetGameTime())
    {
        int iFinishedAction(m_iCurrentAction);
        if (GetCurrentItem() != NULL)
            GetCurrentItem()->FinishedAction(iFinishedAction);
    }

    if (m_ePetState == PETSTATE_ATTACKING && !(m_iCurrentAction & PLAYER_ACTION_STUNNED))
    {
        CVec3f v3Start(GetPosition() + V_UP * GetViewHeight());
        CVec3f v3Forward;
        
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (pTargetEnt && pTargetEnt->IsVisual())
        {
            IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

            CVec3f v3Aim;
            if (pVisual->IsBuilding() || pVisual->IsProp())
                v3Aim = pVisual->GetPosition() + CVec3f(0.0f, 0.0f, GetViewHeight());
            else
                v3Aim = pVisual->GetPosition() + pVisual->GetBounds().GetMid();
            
            v3Forward = Normalize(v3Aim - v3Start);
            m_v3FaceAngles = M_GetAnglesFromForwardVec(v3Forward);
        }
        else
        {
            v3Forward = M_GetForwardVecFromAngles(m_v3Angles);
            m_v3FaceAngles = m_v3Angles;
        }

        SetAngles(m_v3FaceAngles);

        if (pTargetEnt && ShouldTarget(pTargetEnt))
        {
            if (GetCurrentItem() != NULL && !GetCurrentItem()->IsDisabled())
            {
                IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

                if (pVisual != NULL)
                {
                    bool bWasIdle(IsIdle());
                    int iPrevSeq(GetAttackSequence());

                    if (ShouldBlock(pVisual))
                        ActivateTertiary(m_ySelectedItem, GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED);
                    else if (ShouldUnblock(pVisual))
                        ActivateTertiary(m_ySelectedItem, GAME_BUTTON_STATUS_RELEASED | GAME_BUTTON_STATUS_UP);
                    else if (ShouldStrongAttack(pVisual))
                        ActivateSecondary(m_ySelectedItem, GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED);
                    else if (ShouldQuickAttack(pVisual))
                        ActivatePrimary(m_ySelectedItem, GAME_BUTTON_STATUS_DOWN | GAME_BUTTON_STATUS_PRESSED);

                    // If we've started our new action, reset the tracker
                    if (bWasIdle && !IsIdle() ||iPrevSeq != GetAttackSequence())
                        m_uiNextAction = -1;
                }
            }
        }
    }   

    Repair(m_ePetState == PETSTATE_REPAIRING && !(m_iCurrentAction & PLAYER_ACTION_STUNNED));

    return true;
}


/*====================
  IPetEntity::Damage
  ====================*/
float   IPetEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
    if (m_ePetMode != PETMODE_PASSIVE)
    {
        if (pAttacker != NULL && m_ePetState == PETSTATE_WAITING && ShouldTarget(pAttacker))
            AddAggro(pAttacker->GetIndex(), pet_sightAggro + pet_damageAggro * fDamage, true);
        else if (pAttacker != NULL && ShouldTarget(pAttacker))
            AddAggro(pAttacker->GetIndex(), pet_damageAggro * fDamage, true);
    }

    float damage = ICombatEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);

    if (m_uiNextPainTime < Host.GetTime() && damage > 0 && !m_pEntityConfig->GetPainEffectPath().empty())
    {
        CGameEvent evPain;
        evPain.SetSourceEntity(GetIndex());
        evPain.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetPainEffectPath()));
        Game.AddEvent(evPain);
        m_uiNextPainTime = Host.GetTime() + 2000;
    }

    IGameEntity *pEnt(Game.GetEntityFromUniqueID(GetOwnerUID()));
    IPlayerEntity *pOwner(NULL);

    if (pEnt != NULL)
        pOwner = pEnt->GetAsPlayerEnt();

    if (GetSoulLinkDamage() > 0.0f && pOwner != NULL)
        pOwner->Damage(damage * GetSoulLinkDamage(), DAMAGE_FLAG_DIRECT, pAttacker, GetType(), bFeedback);

    return damage;
}


/*====================
  IPetEntity::Heal
  ====================*/
float   IPetEntity::Heal(float fHealth, IVisualEntity *pSource)
{
    float fHealed = ICombatEntity::Heal(fHealth, pSource);

    IGameEntity *pEnt(Game.GetEntityFromUniqueID(GetOwnerUID()));
    IPlayerEntity *pOwner(NULL);

    if (pEnt != NULL)
        pOwner = pEnt->GetAsPlayerEnt();

    if (GetSoulLinkHealing() > 0.0f && pOwner != NULL)
        pOwner->Heal(fHealed * GetSoulLinkHealing(), pSource);

    return fHealed;
}


/*====================
  IPetEntity::KillReward
  ====================*/
void    IPetEntity::KillReward(IGameEntity *pKiller)
{
    ICombatEntity::KillReward(pKiller);

    if (pKiller == NULL)
        return;

    if (!pKiller->IsPlayer() && !pKiller->IsPet())
        return;

    if (pKiller->IsPet())
    {
        pKiller = Game.GetEntityFromUniqueID(pKiller->GetAsPet()->GetOwnerUID());

        if (pKiller == NULL || !pKiller->IsPlayer())
            return;
    }

    if (GetSoulChance() > 0 && M_Randnum(0.0f, 1.0f) <= GetSoulChance())
    {
        // Spawn a soul
        CEntitySoul *pSoul(static_cast<CEntitySoul*>(Game.AllocateEntity(_T("Entity_Soul"))));
        if (pSoul == NULL)
        {
            Console.Warn << _T("Failed to create soul entity") << newl;
        }
        else
        {
            pSoul->SetPosition(m_v3Position);
            pSoul->SetTarget(pKiller->GetIndex());
            pSoul->Spawn();

            if (g_ResourceManager.GetAnim(GetModelHandle(), _T("die_soul")) != -1)
                StartAnimation(_T("die_soul"), -1);
        }
    }
}


/*====================
  IPetEntity::Kill
  ====================*/
void    IPetEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_fHealth = 0.0f;
    m_fMana = 0.0f;
    m_fStamina = 0.0f;

    Unlink();
    SetStatus(ENTITY_STATUS_CORPSE);
    Link();

    ClearStates();

    m_uiCorpseTime = Game.GetGameTime() + g_corpseTime;

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    StartAnimation(_T("die"), -1);
    
    KillReward(pAttacker);

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
  IPetEntity::Link
  ====================*/
void    IPetEntity::Link()
{
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
  IPetEntity::Unlink
  ====================*/
void    IPetEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX)
        Game.UnlinkEntity(m_uiWorldIndex);
}


/*====================
  IPetEntity::GetItemRange
  ====================*/
float   IPetEntity::GetItemRange()
{
    IInventoryItem *pCurrentItem(GetCurrentItem());

    if (!pCurrentItem)
        return GetBounds().GetMax(X) * 2.0f;

    if (pCurrentItem->IsMelee())
        return LERP(0.75f, pCurrentItem->GetAsMelee()->GetQuickAttackRangeMin(), pCurrentItem->GetAsMelee()->GetQuickAttackRangeMax());
    else if (pCurrentItem->IsGun())
        return pCurrentItem->GetAsGun()->GetRange();
    else
        return GetBounds().GetMax(X) * 2.0f;
}


/*====================
  IPetEntity::Accelerate
  ====================*/
void    IPetEntity::Accelerate(const CVec3f &v3Intent, float fAcceleration)
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
  IPetEntity::Friction
  ====================*/
void    IPetEntity::Friction(float fFriction)
{
    float fSpeed(s_Move.v3Velocity.Length());

    if (fSpeed > 0.0f)
    {
        float fNewSpeed(MAX(0.0f, fSpeed - fFriction * s_Move.fFrameTime));

        s_Move.v3Velocity *= fNewSpeed / fSpeed;
    }

    if (s_Move.v3Velocity.Length() < 0.001f)
        s_Move.v3Velocity = V3_ZERO;
}


/*====================
  IPetEntity::MoveWalkGround
  ====================*/
void    IPetEntity::MoveWalkGround()
{
    // Friction
    float fRelativeFactor(s_Move.v3Velocity.Length() > s_Move.fRunSpeed ? s_Move.v3Velocity.Length() : s_Move.fRunSpeed);
    float fGroundFriction(fRelativeFactor * pet_groundFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

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
    Accelerate(s_Move.v3Intent, pet_groundAccelerate);

    StepSlide(false);
}


/*====================
  IPetEntity::MoveWalkAir
  ====================*/
void    IPetEntity::MoveWalkAir()
{
    if (s_Move.iMoveFlags & MOVE_ON_GROUND)
    {
        // Friction
        float fGroundFriction(pet_slopeFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

        // Apply ground friction
        Friction(fGroundFriction);

        // Project movement intent onto the ground plane
        s_Move.v3Intent.SetDirection(Clip(s_Move.v3Intent.Direction(), s_Move.plGround.v3Normal));
    }

    Accelerate(s_Move.v3Intent, pet_airAccelerate);

    Slide(true);
}


/*====================
  IPetEntity::MoveWalk
  ====================*/
bool    IPetEntity::MoveWalk(float fFrameTime, const CVec3f &v3TargetPos, bool bContinue)
{
    PROFILE("IPetEntity::MoveWalk");

    Unlink();
    if (GetStatus() == ENTITY_STATUS_DEAD || GetStatus() == ENTITY_STATUS_DORMANT)
        return true;

    int iNewMovement(PET_MOVE_IDLE);
    bool bFinished(false);

    // Set working vars
    s_Move.fFrameTime = fFrameTime;
    s_Move.v3OldPosition = m_v3Position;
    s_Move.v3OldVelocity = m_v3Velocity;
    s_Move.v3OldAngles = m_v3Angles;
    s_Move.v3Position = m_v3Position;
    s_Move.v3Velocity = m_v3Velocity;
    s_Move.v3Angles = m_v3Angles;
    s_Move.fRunSpeed = GetSpeed();
    s_Move.iMoveFlags = m_iMoveFlags;
    s_Move.bLanded = false;

    s_Move.v3Angles[PITCH] = 0.0f;
    s_Move.v3Angles[ROLL] = 0.0f;

    CheckGround();

    CVec3f v3Delta(v3TargetPos - m_v3Position);
    CVec3f v3TotalDelta(m_v3TargetPos - m_v3Position);

    // Check for sprint
    float fCost(fFrameTime * p_staminaSprintCost);
    float fSprintSpeed(s_Move.fRunSpeed * p_speedSprint);
    if (m_ePetState == PETSTATE_MOVING &&
        v3TotalDelta.Length() >= fSprintSpeed * fFrameTime &&
        !(m_iCurrentAction & PLAYER_ACTION_BLOCK) &&
        m_fStamina > fCost &&
        (!IsExhausted() || m_iCurrentMovement & PET_MOVE_SPRINT) &&
        s_Move.bGroundControl && s_Move.v3Velocity != V3_ZERO)
    {
        iNewMovement |= PET_MOVE_SPRINT;
        s_Move.fRunSpeed = fSprintSpeed;
    }

    CVec3f v3Intent;
    if (v3TargetPos != m_v3Position && !(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
    {
        // Movement direction
        v3Intent = v3Delta;
        v3Intent.z = 0.0f;
        v3Intent.Normalize();

        if (v3Delta.Length() < s_Move.fRunSpeed * fFrameTime)
            bFinished = true;
        else
            bFinished = false;

        if (bContinue)
            v3Intent *= MIN(s_Move.fRunSpeed, v3Delta.Length() / fFrameTime);
        else
            v3Intent *= s_Move.fRunSpeed;

        if (v3Intent.LengthSq() < 20.0f)
            v3Intent.Clear();
        else
        {
            M_GetAnglesFromForwardVec(v3Intent, m_v3FaceAngles);
        }
    }
    else
    {
        v3Intent = CVec3f(0.0f, 0.0f, 0.0f);
    }

    // Interpolate yaw across shortest arc
    if (s_Move.v3Angles[YAW] - m_v3FaceAngles[YAW] > 180.0f)
        m_v3FaceAngles[YAW] += 360.0f;
    if (s_Move.v3Angles[YAW] - m_v3FaceAngles[YAW] < -180.0f)
        s_Move.v3Angles[YAW] += 360.0f;

    float fAngleStep(pet_turnSpeed * fFrameTime);

    if (s_Move.v3Angles[YAW] > m_v3FaceAngles[YAW])
    {
        if (s_Move.v3Angles[YAW] - m_v3FaceAngles[YAW] < fAngleStep)
            s_Move.v3Angles[YAW] = m_v3FaceAngles[YAW];
        else
            s_Move.v3Angles[YAW] -= fAngleStep;
    }
    else if (s_Move.v3Angles[YAW] < m_v3FaceAngles[YAW])
    {
        if (m_v3FaceAngles[YAW] - s_Move.v3Angles[YAW] < fAngleStep)
            s_Move.v3Angles[YAW] = m_v3FaceAngles[YAW];
        else
            s_Move.v3Angles[YAW] += fAngleStep;
    }

    if (s_Move.v3Angles[YAW] >= 180.0f)
        s_Move.v3Angles[YAW] -= 360.0f;

    s_Move.v3Intent = v3Intent;

    if (s_Move.v3Velocity != V_ZERO ||
        s_Move.v3Intent != V_ZERO ||
        !(s_Move.iMoveFlags & MOVE_ON_GROUND))
    {
        if (s_Move.bGroundControl)
            MoveWalkGround();
        else
            MoveWalkAir();

        CheckGround();
    }

    // Apply this movement
    if (IsPositionValid(s_Move.v3Position))
    {
        if (iNewMovement & PET_MOVE_SPRINT && m_v3Position != s_Move.v3Position)
            DrainStamina(fCost);

        m_v3Position = s_Move.v3Position;
        m_v3Velocity = s_Move.v3Velocity;
        m_iMoveFlags = s_Move.iMoveFlags;
    }
    else
    {
        if (Game.IsServer() && false)
            Console << _T("Pet move got stuck") << newl;
    }

    m_v3Angles = s_Move.v3Angles;

    Link();

    if (m_iMoveFlags & MOVE_JUMPING)
    {
        iNewMovement = PET_MOVE_JUMP;
    }
    else
    {
        if (v3Intent.LengthSq() > 0.0f)
            iNewMovement |= PET_MOVE_FWD;
    }

    // Animation
    float fNewRelativeSpeed(s_Move.fRunSpeed / m_pEntityConfig->GetSpeed());

    if (!(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
    {
        tstring sAnimName;
        float fAnimSpeed(1.0f);

        switch ((iNewMovement & PET_MOVE_NO_FLAGS) &~ PET_MOVE_IGNORE_FLAGS)
        {
        case PET_MOVE_IMMOBILE:
            // Don't alter animations
            break;

        case PET_MOVE_JUMP:
            //sAnimName = _T("jump");
            break;

        case PET_MOVE_IDLE:
            if (GetCurrentItem() != NULL && GetCurrentItem()->IsGun() && GetAnimIndex(_T("gun_idle")) != -1)
            {           
                sAnimName = _T("gun_idle");
                m_yDefaultAnim = GetAnimIndex(_T("gun_idle"));
            }
            else if (iNewMovement & PET_MOVE_TIRED)
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

        case PET_MOVE_FWD:
        case PET_MOVE_FWD_LEFT:
        case PET_MOVE_FWD_RIGHT:
            if (iNewMovement & PET_MOVE_SPRINT)
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

        case PET_MOVE_BACK:
        case PET_MOVE_BACK_LEFT:
        case PET_MOVE_BACK_RIGHT:
            sAnimName = _T("run_back");
            fAnimSpeed = fNewRelativeSpeed;
            break;

        case PET_MOVE_LEFT:
            if (m_pEntityConfig->GetYawStrafe())
            {
                if (iNewMovement & PET_MOVE_SPRINT)
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

        case PET_MOVE_RIGHT:
            if (m_pEntityConfig->GetYawStrafe())
            {
                if (iNewMovement & PET_MOVE_SPRINT)
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
    if (s_Move.bLanded && (iNewMovement & PET_MOVE_NO_FLAGS) == PET_MOVE_IDLE)
    {
        StartAnimation(_T("land"), 0);
    }

    return bFinished;
}


/*====================
  IPetEntity::GetCurrentJob
  ====================*/
tstring IPetEntity::GetCurrentJob() const
{
    switch (m_ePetState)
    {
    case PETSTATE_WAITING: return _T("idle");
    case PETSTATE_MOVING: return _T("moving");
    case PETSTATE_ATTACKING: return _T("attacking");
    case PETSTATE_REPAIRING: return _T("repairing");
    }

    return _T("???");
}


/*====================
  IPetEntity::ShouldTarget
  ====================*/
bool    IPetEntity::ShouldTarget(IGameEntity *pOther)
{
    if (!pOther->IsVisual())
        return false;

    IVisualEntity *pVisual(pOther->GetAsVisualEnt());

    if (IsEnemy(pVisual) &&
        (LooksLikeEnemy(pVisual) || pVisual->HasNetFlags(ENT_NET_FLAG_REVEALED)) &&
        (pVisual->GetStatus() == ENTITY_STATUS_ACTIVE || pVisual->GetStatus() == ENTITY_STATUS_SPAWNING)&&
        !pVisual->IsStealthed() &&
        pVisual->AIShouldTarget() && pVisual != Game.GetEntityFromUniqueID(GetOwnerUID()))
        return true;
    else
        return false;
}


/*====================
  IPetEntity::Follow
  ====================*/
bool    IPetEntity::Follow()
{
    if (m_ePetState == PETSTATE_WAITING)
    {
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (!pTargetEnt)
            return false;

        IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
        if (!pVisual)
            return false;

        // Look at target
        CVec3f v3Start(GetPosition() + V_UP * GetViewHeight());
        CVec3f v3Aim(pVisual->GetPosition() + pVisual->GetBounds().GetMid());
        CVec3f v3Forward(Normalize(v3Aim - v3Start));
        m_v3FaceAngles = M_GetAnglesFromForwardVec(v3Forward);
        SetAngles(m_v3FaceAngles);

        CVec3f v3TargetPos(pVisual->GetApproachPosition(m_v3Position, m_bbBounds));
        float fFollowDistance(GetBounds().GetDim(X) * 0.5f + GetFollowDistance());
        if (!pVisual->IsBuilding() && !pVisual->IsProp())
            fFollowDistance += pVisual->GetBounds().GetDim(X) * 0.5f;
        
        if (DistanceSq(GetPosition(), v3TargetPos) < SQR(fFollowDistance))
            return true;

        m_v3TargetPos = v3TargetPos;

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_ePetState = PETSTATE_MOVING;

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
        return true;
    }

    // Update target position
    IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
    if (!pTargetEnt)
        return false;

    IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
    if (!pVisual)
        return false;

    m_v3TargetPos = pVisual->GetApproachPosition(m_v3Position, m_bbBounds);

    // Check for repath
    if (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f)
    {
        Game.FreePath(m_hPath);
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        m_v3OldTargetPos = m_v3TargetPos;
    }

    if (m_hPath == INVALID_POOL_HANDLE)
        return true;

    CPath *pPath(Game.AccessPath(m_hPath));

    if (!pPath)
    {
        m_hPath = INVALID_POOL_HANDLE;
        return false;
    }

    float fFollowDistance(GetBounds().GetDim(X) * 0.5f + GetFollowDistance());
    if (!pVisual->IsBuilding() && !pVisual->IsProp())
        fFollowDistance += pVisual->GetBounds().GetDim(X) * 0.5f;

    if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(fFollowDistance))
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_ePetState = PETSTATE_WAITING;

        return true;
    }

    return true;
}


/*====================
  IPetEntity::Attack
  ====================*/
bool    IPetEntity::Attack()
{
    if (m_uiTargetUID == INVALID_INDEX)
        return false;

    if (m_ePetState == PETSTATE_WAITING)
    {
        //
        // Start attack
        //
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (!pTargetEnt)
            return false;

        IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
        if (!pVisual)
            return false;

        m_v3TargetPos = pVisual->GetApproachPosition(m_v3Position, m_bbBounds);

        float fRange(GetItemRange());
        if (!pVisual->IsBuilding() && !pVisual->IsProp())
            fRange += pVisual->GetBounds().GetDim(X) * 0.5f;

        if (DistanceSq(GetPosition(), m_v3TargetPos) > SQR(fRange))
        {
            // Start the move
            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
            m_ePetState = PETSTATE_MOVING;

            if (m_hPath == INVALID_POOL_HANDLE)
                m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
            else
                m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
            
            m_v3OldTargetPos = m_v3TargetPos;
        }
    }

    //
    // Update target
    //

    if (m_uiNextActionTime < Game.GetGameTime())
    {
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (!pTargetEnt)
            return false;

        IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
        if (!pVisual || (pVisual->GetStatus() != ENTITY_STATUS_ACTIVE && pVisual->GetStatus() != ENTITY_STATUS_SPAWNING))
            return false;
    }

    //
    // Follow/Attack target
    //

    IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
    if (!pTargetEnt)
        return false;

    IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
    if (!pVisual)
        return false;

    m_v3TargetPos = pVisual->GetApproachPosition(m_v3Position, m_bbBounds);

    // Check for repath
    if (m_ePetState == PETSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f))
    {
        Game.FreePath(m_hPath);
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    float fRange(GetItemRange());
    if (!pVisual->IsBuilding() && !pVisual->IsProp())
        fRange += pVisual->GetBounds().GetDim(X) * 0.5f;

    bool bCanAttack(false);
    if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(fRange))
    {
        STraceInfo trace;
        Game.TraceLine(trace, GetPosition() + V_UP * GetViewHeight(), pVisual->GetPosition() + pVisual->GetBounds().GetMid(), SURF_SHIELD | TRACE_PROJECTILE, GetWorldIndex());
        if (trace.uiEntityIndex == pVisual->GetWorldIndex())
            bCanAttack = true;
    }

    if (bCanAttack)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_ePetState = PETSTATE_ATTACKING;
    }
    else if (m_hPath == INVALID_POOL_HANDLE && m_ePetState == PETSTATE_ATTACKING && m_uiNextActionTime < Game.GetGameTime())
    {
        // Start a new move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_ePetState = PETSTATE_MOVING;

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    return true;
}


/*====================
  IPetEntity::TryRepair
  ====================*/
bool    IPetEntity::TryRepair()
{
    // Validate target
    if (m_uiTargetUID == INVALID_INDEX)
        return false;
    IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
    if (!pTargetEnt)
        return false;
    IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());
    if (!pVisual)
        return false;
    if (pVisual->GetStatus() != ENTITY_STATUS_ACTIVE && pVisual->GetStatus() != ENTITY_STATUS_SPAWNING)
        return false;
    if (pVisual->GetHealth() >= pVisual->GetMaxHealth())
        return false;

    float fRange(GetBounds().GetMax(X) + g_repairRange);
    m_v3TargetPos = pVisual->GetApproachPosition(m_v3Position, m_bbBounds);

    // Path to target
    if ((m_ePetState == PETSTATE_WAITING && DistanceSq(GetPosition(), m_v3TargetPos) > SQR(fRange)) ||
        (m_ePetState == PETSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f)))
    {
        Game.FreePath(m_hPath);

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        CPath *pPath(Game.AccessPath(m_hPath));

        if (pPath)
            pPath->AddPoint(m_v3TargetPos.x, m_v3TargetPos.y);

        m_ePetState = PETSTATE_MOVING;

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    if (m_ePetState == PETSTATE_WAITING || (m_ePetState == PETSTATE_MOVING && m_hPath == INVALID_POOL_HANDLE))
    {
        // Face repair target
        M_GetAnglesFromForwardVec(Normalize(m_v3TargetPos - m_v3Position), m_v3FaceAngles);
    }

    // Check if target is in range
    bool bCanRepair(false);
    if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(fRange))
    {
        STraceInfo trace;
        CAxis axis(GetAngles());
        CVec3f v3Forward(axis.Forward());
        CVec3f v3Start(GetPosition());
        CVec3f v3End(v3Start + (v3Forward * fRange * 2.f));
        Game.TraceBox(trace, v3Start, v3End, GetBounds(),SURF_TERRAIN | TRACE_PLAYER_MOVEMENT, GetWorldIndex());
        if (trace.uiEntityIndex == pVisual->GetWorldIndex())
            bCanRepair = true;
    }

    if (bCanRepair)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_ePetState = PETSTATE_REPAIRING;
    }
    else if (m_hPath == INVALID_POOL_HANDLE && m_ePetState == PETSTATE_REPAIRING && m_uiNextActionTime < Game.GetGameTime())
    {
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        CPath *pPath(Game.AccessPath(m_hPath));

        if (pPath)
            pPath->AddPoint(m_v3TargetPos.x, m_v3TargetPos.y);

        m_ePetState = PETSTATE_MOVING;

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    return true;
}


/*====================
  IPetEntity::Move
  ====================*/
bool    IPetEntity::Move()
{
    if (m_ePetState == PETSTATE_WAITING)
    {
        if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(m_bbBounds.GetDim(X)))
            return true;

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_ePetState = PETSTATE_MOVING;

        if (m_ePetJob == PETJOB_REPAIR || m_ePetJob == PETJOB_ATTACK)
        {
            CPath *pPath(Game.AccessPath(m_hPath));

            if (pPath)
                pPath->AddPoint(m_v3TargetPos.x, m_v3TargetPos.y);
        }

        m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        m_v3OldTargetPos = m_v3TargetPos;

        return true;
    }

    // Check for repath
    if (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f)
    {
        Game.FreePath(m_hPath);
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        m_v3OldTargetPos = m_v3TargetPos;
    }

    if (m_hPath == INVALID_POOL_HANDLE)
        return false;

    if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(m_bbBounds.GetDim(X)))
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_ePetState = PETSTATE_WAITING;

        return true;
    }

    return true;
}


/*====================
  IPetEntity::Idle
  ====================*/
bool    IPetEntity::Idle()
{
    return true;
}


/*====================
  IPetEntity::Think

  Combines guard position and patrol, maybe others later
  ====================*/
bool    IPetEntity::Think(bool bMoveAggro)
{
    PROFILE("IPetEntity::Think");

    if ((bMoveAggro || m_ePetState == PETSTATE_WAITING) && m_ePetMode == PETMODE_AGGRESSIVE && GetAggroRadius() > 0.0f && GetMaxAggro() == INVALID_INDEX)
    {
        //
        // Search aggro radius for enemies
        //

        uivector vSight;
        Game.GetEntitiesInRadius(vSight, CSphere(GetPosition(), GetAggroRadius()), 0);
        uint uiTarget(INVALID_INDEX);
        for (uivector_it it(vSight.begin()); it != vSight.end(); ++it)
        {
            uint uiIndex(Game.GetGameIndexFromWorldIndex(*it));

            IVisualEntity *pOther(Game.GetVisualEntity(uiIndex));

            if (ShouldTarget(pOther))
            {
                uiTarget = uiIndex;
                break;
            }
        }

        IVisualEntity *pTargetEnt(Game.GetVisualEntity(uiTarget));
        if (pTargetEnt)
        {
            // Sight aggro
            AddAggro(uiTarget, pet_sightAggro, true);

            // Sight adjacent aggro
            vSight.clear();
            Game.GetEntitiesInRadius(vSight, CSphere(pTargetEnt->GetPosition(), GetAggroRadius() / 2.0f), 0);
            for (uivector_it it(vSight.begin()); it != vSight.end(); ++it)
            {
                uint uiIndex(Game.GetGameIndexFromWorldIndex(*it));

                IVisualEntity *pOther(Game.GetVisualEntity(uiIndex));

                if (ShouldTarget(pOther))
                    AddAggro(uiIndex, pet_sightAdjacentAggro, true);
            }
        }
    }

    //
    // Update target
    //

    if (m_uiNextActionTime > Game.GetGameTime())
        return true;

    uint uiTargetUID(GetMaxAggro());

    if (m_uiTargetUID != uiTargetUID)
    {
        if (uiTargetUID == INVALID_INDEX)
        {
            //
            // Deaggro
            //

            m_uiTargetUID = INVALID_INDEX;

            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;

            m_ePetState = PETSTATE_WAITING;
        }
        else
        {
            //
            // Switch aggro
            //

            m_uiTargetUID = uiTargetUID;
        }
    }
    else
    {
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        IVisualEntity *pVisual(NULL);
        if (pTargetEnt)
            pVisual = pTargetEnt->GetAsVisualEnt();

        if (!pVisual && m_uiTargetUID != INVALID_INDEX)
        {
            // If we lost our target, reset our state
            m_uiTargetUID = INVALID_INDEX;

            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;

            m_ePetState = PETSTATE_WAITING;
        }
    }

    if (m_uiTargetUID != INVALID_INDEX)
    {
        //
        // Follow/Attack target
        //

        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (pTargetEnt && pTargetEnt->IsVisual())
        {
            IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

            if (m_ePetState == PETSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || Distance(pVisual->GetPosition(), m_v3OldTargetPos) > 100.0f))
            {
                Game.FreePath(m_hPath);

                m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, pVisual->GetPosition().xy());
                m_ePetState = PETSTATE_MOVING;

                m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
                m_v3OldTargetPos = pVisual->GetPosition();
            }

            if (DistanceSq(GetPosition(), pVisual->GetPosition()) < SQR(pVisual->GetBounds().GetDim(X) * 0.5f + GetItemRange()))
            {
                Game.FreePath(m_hPath);
                m_hPath = INVALID_POOL_HANDLE;

                if (m_ePetJob == PETJOB_ATTACK || m_ePetJob == PETJOB_GUARDPOS || m_ePetJob == PETJOB_PATROL)
                    m_ePetState = PETSTATE_ATTACKING;
                else if (m_ePetJob == PETJOB_REPAIR)
                    m_ePetState = PETSTATE_REPAIRING;
                else
                    m_ePetState = PETSTATE_WAITING;
            }
            else if (m_hPath == INVALID_POOL_HANDLE && m_ePetState != PETSTATE_MOVING)
            {
                m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, pVisual->GetPosition().xy());
                m_ePetState = PETSTATE_MOVING;

                m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
                m_v3OldTargetPos = pVisual->GetPosition();
            }
        }
    }
    else
    {
        float fDistance(Distance(GetPosition(), m_v3TargetPos));

        if (m_ePetState == PETSTATE_WAITING/* && fDistance > m_fGuardRadius*/ && fDistance > GetBounds().GetDim(X))
        {
            //
            // Head towards target position
            //

            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
            m_ePetState = PETSTATE_MOVING;

            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
            m_v3OldTargetPos = m_v3TargetPos;
        }
        else if (m_ePetState == PETSTATE_MOVING && fDistance <= GetBounds().GetDim(X))
        {
            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;

            m_ePetState = PETSTATE_WAITING;
        }

        if (m_ePetState == PETSTATE_MOVING && m_uiRepathTime <= Game.GetGameTime())
        {
            Game.FreePath(m_hPath);
            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3OldTargetPos.xy());

            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        }
    }

    return true;
}


/*====================
  IPetEntity::PlayerCommand
  ====================*/
void    IPetEntity::PlayerCommand(EPetCommand ePetCmd, uint uiIndex, const CVec3f &v3Pos)
{
    switch (ePetCmd)
    {
    case PETCMD_ATTACK:
        CommandAttack(uiIndex);
        break;
    case PETCMD_MOVE:
        CommandMove(v3Pos);
        break;
    case PETCMD_STOP:
        CommandStop();
        break;
    case PETCMD_FOLLOW:
        CommandFollow(uiIndex);
        break;
    case PETCMD_SPECIALABILITY:
        CommandSpecialAbility();
        break;
    case PETCMD_RETURN:
        CommandReturn();
        break;
    case PETCMD_TOGGLEAGGRO:
        CommandToggleAggro();
        break;
    case PETCMD_BANISH:
        CommandBanish();
        break;
    case PETCMD_REPAIR:
        CommandRepair(uiIndex);
        break;
    case PETCMD_PATROL:
        CommandPatrol(v3Pos);
        break;
    }

    IGameEntity *pOwner(Game.GetEntityFromUniqueID(GetOwnerUID()));

    if (pOwner != NULL && pOwner->IsPlayer())
    {
        CBufferFixed<2> buffer;
        buffer << GAME_CMD_PETCMD_ORDERCONFIRMED;
        Game.SendGameData(pOwner->GetAsPlayerEnt()->GetClientID(), buffer, false);
    }
}


/*====================
  IPetEntity::CommandAttack
  ====================*/
void    IPetEntity::CommandAttack(uint uiIndex)
{
    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_ATTACK;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::CommandMove
  ====================*/
void    IPetEntity::CommandMove(const CVec3f &v3Pos)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_MOVE;
    m_v3TargetPos = v3Pos;
    m_ePetState = PETSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  IPetEntity::CommandStop
  ====================*/
void    IPetEntity::CommandStop()
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_IDLE;
    m_ePetState = PETSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  IPetEntity::CommandFollow
  ====================*/
void    IPetEntity::CommandFollow(uint uiIndex)
{
    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_FOLLOW;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::CommandPatrol
  ====================*/
void    IPetEntity::CommandPatrol(const CVec3f &v3Pos)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_PATROL;
    m_v3TargetPos = v3Pos;
    m_ePetState = PETSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  IPetEntity::CommandSpecialAbility
  ====================*/
void    IPetEntity::CommandSpecialAbility()
{
    m_yActivate = 1;
}


/*====================
  IPetEntity::CommandReturn
  ====================*/
void    IPetEntity::CommandReturn()
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_FOLLOW;
    m_uiTargetUID = m_uiOwnerUID;
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::CommandToggleAggro
  ====================*/
void    IPetEntity::CommandToggleAggro()
{
    if (m_ePetMode == PETMODE_PASSIVE)
        m_ePetMode = PETMODE_AGGRESSIVE;
    else if (m_ePetMode == PETMODE_AGGRESSIVE)
        m_ePetMode = PETMODE_DEFENSIVE;
    else
        m_ePetMode = PETMODE_PASSIVE;
}


/*====================
  IPetEntity::CommandBanish
  ====================*/
void    IPetEntity::CommandBanish()
{
    Kill();
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::CommandRepair
  ====================*/
void    IPetEntity::CommandRepair(uint uiIndex)
{
    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    m_ePetJob = PETJOB_REPAIR;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_ePetState = PETSTATE_WAITING;
}


/*====================
  IPetEntity::CommandGuard
  ====================*/
void    IPetEntity::CommandGuard(const CVec3f &v3Pos)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_ePetJob = PETJOB_GUARDPOS;
    m_v3TargetPos = v3Pos;
    m_ePetState = PETSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  IPetEntity::UpdateSkeleton
  ====================*/
void    IPetEntity::UpdateSkeleton(bool bPose)
{
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
            switch (m_iCurrentMovement & PET_MOVE_NO_FLAGS)
            {
            case PET_MOVE_FWD_LEFT:
            case PET_MOVE_BACK_RIGHT:
                fDiagYaw = 45.0f;
                break;
            case PET_MOVE_FWD_RIGHT:
            case PET_MOVE_BACK_LEFT:
                fDiagYaw = -45.0f;
                break;
            case PET_MOVE_LEFT:
                fDiagYaw = pet_yawStrafeAngle;
                break;
            case PET_MOVE_RIGHT:
                fDiagYaw = -pet_yawStrafeAngle;
                break;
            }
        }
        else
        {
            switch (m_iCurrentMovement & PET_MOVE_NO_FLAGS)
            {
            case PET_MOVE_FWD_LEFT:
            case PET_MOVE_BACK_RIGHT:
                fDiagYaw = 45.0f;
                break;
            case PET_MOVE_FWD_RIGHT:
            case PET_MOVE_BACK_LEFT:
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
    else if ((m_iTurnAction != (m_iCurrentMovement & PET_MOVE_NO_FLAGS) ||
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
        m_iTurnAction = (m_iCurrentMovement & PET_MOVE_NO_FLAGS);
        m_ayTurnSequence[0] = m_ayAnimSequence[0];
        m_ayTurnSequence[1] = m_ayAnimSequence[1];
    }
    else if ((m_iCurrentMovement & PET_MOVE_NO_FLAGS) == PET_MOVE_IDLE && GetStatus() == ENTITY_STATUS_ACTIVE)
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

    if ((m_iCurrentMovement & PET_MOVE_NO_FLAGS) != PET_MOVE_IDLE)
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
            m_pSkeleton->Pose(Game.GetGameTime(), 0, 0.0f);
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
        CAxis aAxis(m_v3Angles);

        float fForward(50.0f * DIAG);
        float fRight(50.0f * DIAG);
        float fDown(200.0f);
        float fTiltSpeed(200.0f);

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
  IPetEntity::AddToScene
  ====================*/
bool    IPetEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    PROFILE("IPetEntity::AddToScene");

    if (GetModelHandle() == INVALID_INDEX)
        return false;

    if (Game.IsCommander() && !m_bSighted)
        return false;

    CVec4f v4TintedColor(v4Color);

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

    if (m_v3AxisAngles != v3Angles)
    {
        m_aAxis.Set(v3Angles);
        m_v3AxisAngles = v3Angles;
    }

    static CSceneEntity sceneEntity;

    sceneEntity.Clear();
    sceneEntity.scale = GetScale() * GetScale2();
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.axis = m_aAxis;
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hModel = GetModelHandle();
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
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

        if (pet_debugVision)
        {
            CVec3f v3Start(GetPosition() + V_UP * GetViewHeight());
            CVec3f v3End(M_PointOnLine(v3Start, CAxis(GetAngles()).Forward(), 9999.0f));
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

        IInventoryItem *pItem(GetCurrentItem());
        if (pItem != NULL)
        {
            AttachModel(pItem->GetModel1Bone(), pItem->GetModel1Handle());
            AttachModel(pItem->GetModel2Bone(), pItem->GetModel2Handle());
        }

        AddSelectionRingToScene();
    }
    else
    {
        UpdateSkeleton(false);
    }

    return true;
}


/*====================
  IPetEntity::AddAggro
  ====================*/
void    IPetEntity::AddAggro(uint uiIndex, float fAggro, bool bMulti)
{
    if (m_yStatus == ENTITY_STATUS_DEAD || m_yStatus == ENTITY_STATUS_CORPSE)
        return;

    IVisualEntity *pOther(Game.GetVisualEntity(uiIndex));
    if (!pOther)
        return;

    AggroMap::iterator it(m_mapAggro.find(pOther->GetUniqueID()));
    if (it == m_mapAggro.end())
        m_mapAggro[pOther->GetUniqueID()] = MIN(fAggro, float(pet_maxAggro));
    else
        m_mapAggro[pOther->GetUniqueID()] = MIN(it->second + fAggro, float(pet_maxAggro));

    if (bMulti && GetMultiAggroRadius() > 0.0f)
    {
        uivector vAllies;
        Game.GetEntitiesInRadius(vAllies, CSphere(m_v3Position, GetMultiAggroRadius()), 0);
        for (uivector_it it(vAllies.begin()); it != vAllies.end(); ++it)
        {
            IVisualEntity *pOther(Game.GetEntityFromWorldIndex(*it));

            if (pOther && pOther->IsPet() && pOther->GetTeam() == m_iTeam && pOther->GetAsPet()->GetPetState() == PETSTATE_WAITING && pOther->GetAsPet()->GetPetMode() != PETMODE_PASSIVE)
                pOther->GetAsPet()->AddAggro(uiIndex, fAggro, false);
        }
    }
}


/*====================
  IPetEntity::GetMaxAggro
  ====================*/
uint    IPetEntity::GetMaxAggro()
{
    float   fMaxAggro(0.0f);
    uint    uiIndex(INVALID_INDEX);

    for (AggroMap::iterator it(m_mapAggro.begin()); it != m_mapAggro.end(); ++it)
    {
        if (it->second > fMaxAggro)
        {
            uiIndex = it->first;
            fMaxAggro = it->second;
        }
    }

    return Game.GetEntityFromUniqueID(uiIndex) ? uiIndex : INVALID_INDEX;
}


/*====================
  IPetEntity::DecayAggro
  ====================*/
void    IPetEntity::DecayAggro(float fFrameTime)
{
    for (AggroMap::iterator it(m_mapAggro.begin()); it != m_mapAggro.end(); )
    {
        float fAmount;

        IGameEntity *pOther(Game.GetEntityFromUniqueID(it->first));
        IVisualEntity *pVisual(pOther ? pOther->GetAsVisualEnt() : NULL);
        if (!pVisual)
        {
            STL_ERASE(m_mapAggro, it);
            continue;
        }
        
        if (pVisual->GetStatus() != ENTITY_STATUS_ACTIVE)
            fAmount = pet_aggroDecayDead * fFrameTime;
        else if (pVisual->GetUniqueID() == m_uiTargetUID && m_ePetState == PETSTATE_ATTACKING)
            fAmount = pet_aggroDecayAttacking * fFrameTime;
        else
            fAmount = pet_aggroDecayVisible * fFrameTime;

        if (it->second <= fAmount)
        {
            STL_ERASE(m_mapAggro, it);
        }
        else
        {
            m_mapAggro[it->first] -= fAmount;
            ++it;
        }
    }
}


/*====================
  IPetEntity::ClientPrecache
  ====================*/
void    IPetEntity::ClientPrecache(CEntityConfig *pConfig)
{
    ICombatEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetPainEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetPainEffectPath(), RES_EFFECT);
    if (!pConfig->GetOrderConfirmedSoundPath().empty())
        g_ResourceManager.Register(pConfig->GetOrderConfirmedSoundPath(), RES_SAMPLE);
}


/*====================
  IPetEntity::ServerPrecache
  ====================*/
void    IPetEntity::ServerPrecache(CEntityConfig *pConfig)
{
    ICombatEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetPainEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetPainEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}


/*====================
  IPetEntity::ShouldBlock
  ====================*/
bool    IPetEntity::ShouldBlock(IVisualEntity *pTarget)
{
    if (GetAction() & PLAYER_ACTION_UNBLOCK && (Game.GetGameTime() - GetActionStartTime() >= (uint)GetReactionTime() || GetReactionTime() == -1))
        return false;

    if (pTarget == NULL)
        return false;

    if (!GetCanBlock() || GetItem(m_ySelectedItem) == NULL || !GetItem(m_ySelectedItem)->IsMelee())
        return false;

    ICombatEntity *pCombat(pTarget->GetAsCombatEnt());

    if (pCombat == NULL)
        return false;

    // If reaction time == -1 and blocking is all they can do, ALWAYS block with a target
    if (GetReactionTime() < 0 && !GetCanStrongAttack() && !GetCanQuickAttack())
        return true;

    // If the target has been stunned, stop blocking after our reaction time
    if (pCombat->GetAction() & PLAYER_ACTION_STUNNED && Game.GetGameTime() - pCombat->GetActionStartTime() >= (uint)GetReactionTime())
        return false;

    CMeleeAttackEvent &pAttack(pCombat->GetAttackEvent());

    if (!(pAttack.GetDamageFlags() & DAMAGE_FLAG_MELEE) || !(pAttack.GetDamageFlags() & DAMAGE_FLAG_BLOCKABLE))
        return false;
    
    if (Game.GetGameTime() - pAttack.GetStartTime() <= (uint)GetReactionTime())
        return false;

    IInventoryItem *pItem(pCombat->GetItem(pCombat->GetSelectedItem()));

    if (pItem == NULL)
        return false;

    IMeleeItem *pMelee(pItem->GetAsMelee());

    if (pMelee == NULL)
        return false;

    // Do not block if they've hit the end of their combo
    if (pMelee->GetQuickAttackChainTime(pCombat->GetAttackSequence()) == 0 && INT_ROUND(pMelee->GetQuickAttackImpactTime(pCombat->GetAttackSequence()) / pCombat->GetAttackSpeed(pMelee->GetApplyAttributes())) + pCombat->GetLastMeleeAttackTime() + GetReactionTime() >= Game.GetGameTime())
        return false;
    
    // Block if they have not hit the end of their time for a quick attack
    if (pCombat->GetLastMeleeAttackTime() + pCombat->GetLastMeleeAttackLength() + GetReactionTime() >= Game.GetGameTime())
        return true;

    return false;
}


/*====================
  IPetEntity::ShouldUnblock
  ====================*/
bool    IPetEntity::ShouldUnblock(IVisualEntity *pTarget)
{
    if (GetAction() & PLAYER_ACTION_UNBLOCK && (Game.GetGameTime() - GetActionStartTime() >= (uint)GetReactionTime() || GetReactionTime() == -1))
        return true;

    if (!(GetAction() & PLAYER_ACTION_BLOCK))
        return false;

    if (pTarget == NULL)
        return false;

    if (!GetCanBlock() || GetItem(m_ySelectedItem) == NULL || !GetItem(m_ySelectedItem)->IsMelee())
        return false;

    ICombatEntity *pCombat(pTarget->GetAsCombatEnt());

    if (pCombat == NULL)
        return false;

    // If the player has been stunned, stop blocking after our reaction time
    if (pCombat->GetAction() & PLAYER_ACTION_STUNNED && Game.GetGameTime() - pCombat->GetActionStartTime() >= (uint)GetReactionTime())
        return true;

    CMeleeAttackEvent &pAttack(pCombat->GetAttackEvent());

    if ((!(pAttack.GetDamageFlags() & DAMAGE_FLAG_MELEE) || !(pAttack.GetDamageFlags() & DAMAGE_FLAG_BLOCKABLE)) && Game.GetGameTime() - pAttack.GetStartTime() >= (uint)GetReactionTime())
        return true;

    IInventoryItem *pItem(pCombat->GetItem(pCombat->GetSelectedItem()));

    if (pItem == NULL)
        return true;

    IMeleeItem *pMelee(pItem->GetAsMelee());

    if (pMelee == NULL)
        return true;

    // Unblock if they've hit the end of their combo
    if (pMelee->GetQuickAttackChainTime(pCombat->GetAttackSequence()) == 0 && Game.GetGameTime() - pAttack.GetStartTime() >= (uint)GetReactionTime())
        return true;

    return false;
}


/*====================
  IPetEntity::ShouldStrongAttack
  ====================*/
bool    IPetEntity::ShouldStrongAttack(IVisualEntity *pTarget)
{
    if (pTarget == NULL)
        return false;

    if (!GetCanStrongAttack() || GetItem(m_ySelectedItem) == NULL || !GetItem(m_ySelectedItem)->IsMelee())
        return false;

    ICombatEntity *pCombat(pTarget->GetAsCombatEnt());

    if (pCombat == NULL)
        return false;

    if (GetReactionTime() < 0 && !GetCanQuickAttack())
        return true;

    // If they aren't doing anything yet (idle), randomly pick quick or strong attack to use
    if (pCombat->IsIdle() && m_uiNextAction != PLAYER_ACTION_QUICK_ATTACK && Game.GetRand(0,2) == 0)
    {
        m_uiNextAction = PLAYER_ACTION_STRONG_ATTACK;
        return true;
    }
    else if (!pCombat->IsIdle())
        m_uiNextAction = -1;

    if (pCombat->GetAction() & PLAYER_ACTION_BLOCK && Game.GetGameTime() - pCombat->GetActionStartTime() >= (uint)GetReactionTime())
        return true;

    return (m_uiNextAction == PLAYER_ACTION_STRONG_ATTACK);
}


/*====================
  IPetEntity::ShouldQuickAttack
  ====================*/
bool    IPetEntity::ShouldQuickAttack(IVisualEntity *pTarget)
{
    if (pTarget == NULL)
        return false;

    if (GetItem(m_ySelectedItem) == NULL)
        return false;
    
    // If this is not a melee weapon, we always want to use it when possible
    if (!GetItem(m_ySelectedItem)->IsMelee())
        return true;

    if (!GetCanQuickAttack())
        return false;

    if (GetReactionTime() < 0)
        return true;

    ICombatEntity *pCombat(pTarget->GetAsCombatEnt());

    if (pCombat != NULL && pCombat->GetAction() & PLAYER_ACTION_BLOCK && Game.GetGameTime() - pCombat->GetActionStartTime() >= (uint)GetReactionTime())
        return false;

    if (m_uiNextAction == -1)
        m_uiNextAction = PLAYER_ACTION_QUICK_ATTACK;

    return (m_uiNextAction == PLAYER_ACTION_QUICK_ATTACK);
}

/*====================
  IPetEntity::GiveExperience
  ====================*/
void    IPetEntity::GiveExperience(float fExperience, const CVec3f &v3Pos)
{
    // Give experience earned to the owner, if he exists
    IGameEntity *pGame(Game.GetEntityFromUniqueID(GetOwnerUID()));
    ICombatEntity *pCombat(NULL);

    if (pGame != NULL)
        pCombat = pGame->GetAsCombatEnt();

    if (pCombat != NULL)
        pCombat->GiveExperience(fExperience, v3Pos);
}

/*====================
  IPetEntity::GiveExperience
  ====================*/
void    IPetEntity::GiveExperience(float fExperience)
{
    // Give experience earned to the owner, if he exists
    IGameEntity *pGame(Game.GetEntityFromUniqueID(GetOwnerUID()));
    ICombatEntity *pCombat(NULL);

    if (pGame != NULL)
        pCombat = pGame->GetAsCombatEnt();

    if (pCombat != NULL)
        pCombat->GiveExperience(fExperience);
}
