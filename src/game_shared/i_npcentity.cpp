// (C)2007 S2 Games
// i_npcentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_npcentity.h"
#include "c_entitysoul.h"
#include "c_entitychest.h"
#include "c_entitynpccontroller.h"

#include "../k2/c_worldentity.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_npcdefinition.h"
#include "../k2/c_texture.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>* INpcEntity::s_pvFields;

CVAR_FLOAT  (npc_groundFriction,            5.0f);
CVAR_FLOAT  (npc_groundAccelerate,          10.0f);
CVAR_FLOAT  (npc_airAccelerate,             1.0f);
CVAR_FLOAT  (npc_slopeFriction,             1000.0f);
CVAR_FLOAT  (npc_turnSpeed,                 360.0f);
CVAR_FLOAT  (npc_maxAggro,                  10.0f);
CVAR_FLOAT  (npc_sightAggro,                10.0f);
CVAR_FLOAT  (npc_sightAdjacentAggro,        0.0f);
CVAR_FLOAT  (npc_damageAggro,               0.2f);
CVAR_FLOAT  (npc_aggroDecayAttacking,       0.3f);
CVAR_FLOAT  (npc_aggroDecayVisible,         1.0f);
CVAR_FLOAT  (npc_aggroDecayInvisible,       2.0f);
CVAR_FLOAT  (npc_aggroDecayDead,            10.0f);

EXTERN_CVAR(float, g_repairRange);
//=============================================================================


/*====================
  INpcEntity::~INpcEntity
  ====================*/
INpcEntity::~INpcEntity()
{
    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }
}


/*====================
  INpcEntity::INpcEntity
  ====================*/
INpcEntity::INpcEntity() :
ICombatEntity(NULL),

m_hDefinition(INVALID_RESOURCE),
m_iCurrentMovement(NPC_MOVE_IDLE),
m_iMoveFlags(0),
m_v3FaceAngles(V3_ZERO),

m_uiNextActionTime(-1),
m_hPath(INVALID_POOL_HANDLE),

m_uiNextSightTime(INVALID_TIME),
m_uiControllerUID(INVALID_INDEX)
{
    m_sNpcType = _T("Unnamed Critter");
    m_sNpcDescription = _T("DESCRIPTION");
    m_bbBounds.SetCylinder(20.0f, 64.0f);
    m_fMaxHealth = 100.0f;
    m_fMaxMana = 100.0f;
    m_fMaxStamina = 100.0f;
    m_fHealthRegen = 0.0f;
    m_fManaRegen = 0.0f;
    m_fStaminaRegen = 0.0f;
    m_fArmor = 0.0f;
    m_fSpeed = 300.0f;
    m_fExperienceReward = 0.0f;
    m_iGoldReward = 0;
    m_fAggroRadius = 600.0f;
    m_fMultiAggroProc = 0.0f;
    m_fMultiAggroRadius = 0.0f;
    m_sInitialJob = _T("guardpos");
    m_bSoul = false;
    m_hHitByMeleeEffectPath = INVALID_RESOURCE;
    m_hHitByMeleeEffectPath = INVALID_RESOURCE;
    m_fItemDrop = 0.0f;
    m_fCommanderScale = 1.0f;
    m_fEffectScale = 1.0f;
    m_fSelectionRadius = 32.0f;
    m_sCommanderPortraitPath = _T("");
    m_iMinimapIconSize = 0;
    m_sMinimapIconPath = _T("");
    m_sIconPath = _T("");
    m_fPushMultiplier = 1.0f;
    m_fFollowDistance = 100.0f;
    m_eNpcMode = NPCMODE_AGGRESSIVE;
}


/*====================
  INpcEntity::Baseline
  ====================*/
void    INpcEntity::Baseline()
{
    ICombatEntity::Baseline();

    m_yDefaultAnim = 0;
    m_hDefinition = INVALID_RESOURCE;
}


/*====================
  INpcEntity::GetSnapshot
  ====================*/
void    INpcEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
    // Base entity info
    ICombatEntity::GetSnapshot(snapshot);

    snapshot.AddField(m_yDefaultAnim);
    snapshot.AddResHandle(m_hDefinition);
}


/*====================
  INpcEntity::ReadSnapshot
  ====================*/
bool    INpcEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        // Base entity info
        if (!ICombatEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextField(m_yDefaultAnim);
        snapshot.ReadNextResHandle(m_hDefinition);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("INpcEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  INpcEntity::GetTypeVector
  ====================*/
const vector<SDataField>&   INpcEntity::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(ICombatEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_yDefaultAnim"), FIELD_PUBLIC, TYPE_CHAR));
        s_pvFields->push_back(SDataField(_T("m_hDefinition"), FIELD_PUBLIC, TYPE_RESHANDLE));
    }

    return *s_pvFields;
}


/*====================
  INpcEntity::Copy
  ====================*/
void    INpcEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const INpcEntity *pB(B.GetAsNpc());

    if (!pB)    
        return;

    const INpcEntity &C(*pB);

    m_hDefinition               = C.m_hDefinition;

    m_sNpcType                  = C.m_sNpcType;
    m_sNpcDescription           = C.m_sNpcDescription;
    m_fMaxHealth                = C.m_fMaxHealth;
    m_fMaxMana                  = C.m_fMaxMana;
    m_fMaxStamina               = C.m_fMaxStamina;
    m_fHealthRegen              = C.m_fHealthRegen;
    m_fManaRegen                = C.m_fManaRegen;
    m_fStaminaRegen             = C.m_fStaminaRegen;
    m_fArmor                    = C.m_fArmor;
    m_iLevel                    = C.m_iLevel;
    m_fCommanderScale           = C.m_fCommanderScale;
    m_fEffectScale              = C.m_fEffectScale;
    m_fSelectionRadius          = C.m_fSelectionRadius;
    m_sCommanderPortraitPath    = C.m_sCommanderPortraitPath;
    m_iMinimapIconSize          = C.m_iMinimapIconSize;
    m_sMinimapIconPath          = C.m_sMinimapIconPath;
    m_sIconPath                 = C.m_sIconPath;
    m_fPushMultiplier           = C.m_fPushMultiplier;
}


/*====================
  INpcEntity::AllocateSkeleton
  ====================*/
CSkeleton*  INpcEntity::AllocateSkeleton()
{
    return m_pSkeleton = K2_NEW(global,   CSkeleton);
}


/*====================
  INpcEntity::UpdateSkeleton
  ====================*/
void    INpcEntity::UpdateSkeleton(bool bPose)
{
    if (m_pSkeleton == NULL)
        return;

    IVisualEntity::UpdateSkeleton(bPose);
}


/*====================
  INpcEntity::ApplyWorldEntity
  ====================*/
void    INpcEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    m_sName = ent.GetName();
    m_uiWorldIndex = ent.GetIndex();
    m_v3Position = ent.GetPosition();
    m_v3Angles = ent.GetAngles();
    m_iTeam = ent.GetTeam();
    m_hDefinition = g_ResourceManager.Register(ent.GetProperty(_T("definition")), RES_NPC);
    m_sController = ent.GetProperty(_T("controller"));

    RegisterEntityScripts(ent);

    // Register definition as a network resource
    g_NetworkResourceManager.GetNetIndex(m_hDefinition);
}


/*====================
  INpcEntity::Spawn
  ====================*/
void    INpcEntity::Spawn()
{
    SetStatus(ENTITY_STATUS_ACTIVE);

    CNpcDefinition *pDefinition(g_ResourceManager.GetNpcDefiniton(m_hDefinition));
    if (pDefinition)
    {
        m_sNpcType = pDefinition->GetNpcType();
        m_sNpcDescription = pDefinition->GetNpcDescription();
        m_iLevel = pDefinition->GetLevel();
        m_hModel = Game.RegisterModel(pDefinition->GetModelPath());
        m_fScale = pDefinition->GetScale();
        m_bbBounds = pDefinition->GetBounds();
        m_fMaxHealth = pDefinition->GetMaxHealth();
        m_fMaxMana = pDefinition->GetMaxMana();
        m_fMaxStamina = pDefinition->GetMaxStamina();
        m_fHealthRegen = pDefinition->GetHealthRegen();
        m_fManaRegen = pDefinition->GetManaRegen();
        m_fStaminaRegen = pDefinition->GetStaminaRegen();
        m_fArmor = pDefinition->GetArmor();
        m_fSpeed = pDefinition->GetSpeed();
        m_fExperienceReward = pDefinition->GetExperienceReward();
        m_iGoldReward = pDefinition->GetGoldReward();
        m_fAggroRadius = pDefinition->GetAggroRadius();
        m_fMultiAggroProc = pDefinition->GetMultiAggroProc();
        m_fMultiAggroRadius = pDefinition->GetMultiAggroRadius();
        m_sInitialJob = pDefinition->GetInitialJob();
        m_bSoul = pDefinition->GetSoul();
        m_fItemDrop = pDefinition->GetItemDrop();
        m_fCommanderScale = pDefinition->GetCommanderScale();
        m_fEffectScale = pDefinition->GetEffectScale();
        m_fSelectionRadius = pDefinition->GetSelectionRadius();
        m_sCommanderPortraitPath = pDefinition->GetCommanderPortraitPath();
        m_iMinimapIconSize = pDefinition->GetMinimapIconSize();
        m_sMinimapIconPath = pDefinition->GetMinimapIconPath();
        m_sIconPath = pDefinition->GetIconPath();
        m_fPushMultiplier = pDefinition->GetPushMultiplier();
        
        m_hHitByMeleeEffectPath = Game.RegisterEffect(pDefinition->GetHitByMeleeEffectPath());
        m_hHitByRangedEffectPath = Game.RegisterEffect(pDefinition->GetHitByRangedEffectPath());

        for (uint n(0); n < pDefinition->GetNumAbilities(); ++n)
            m_vAbilities.push_back(pDefinition->GetAbility(n));
    }
    else
    {
        m_sNpcType = _T("Unnamed Critter");
        m_sNpcDescription = _T("DESCRIPTION");
        m_bbBounds.SetCylinder(20.0f, 64.0f);
        m_fMaxHealth = 100.0f;
        m_fMaxMana = 100.0f;
        m_fMaxStamina = 100.0f;
        m_fHealthRegen = 0.0f;
        m_fManaRegen = 0.0f;
        m_fStaminaRegen = 0.0f;
        m_fArmor = 0.0f;
        m_fSpeed = 300.0f;
        m_fExperienceReward = 0.0f;
        m_iGoldReward = 0;
        m_fAggroRadius = 600.0f;
        m_fMultiAggroProc = 0.0f;
        m_fMultiAggroRadius = 0.0f;
        m_sInitialJob = _T("guardpos");
        m_bSoul = false;
        m_hHitByMeleeEffectPath = INVALID_RESOURCE;
        m_hHitByMeleeEffectPath = INVALID_RESOURCE;
        m_fItemDrop = 0.0f;
        m_fCommanderScale = 1.0f;
        m_fEffectScale = 1.0f;
        m_fSelectionRadius = 32.0f;
        m_sCommanderPortraitPath = _T("");
        m_iMinimapIconSize = 0;
        m_sMinimapIconPath = _T("");
        m_sIconPath = _T("");
        m_fPushMultiplier = 1.0f;
    }

    if (Game.IsServer())
    {
        for (AbilityIter it(m_vAbilities.begin()); it != m_vAbilities.end(); ++it)
        {
            for (uint ui(0); ui < it->GetNumSourceEffects(); ++ui)
            {
                const CNpcAbilityEffect &cAbilityEffect(it->GetSourceEffect(ui));
                g_NetworkResourceManager.GetNetIndex(cAbilityEffect.GetIcon());
                g_NetworkResourceManager.GetNetIndex(cAbilityEffect.GetEffect());
            }

            for (uint ui(0); ui < it->GetNumTargetEffects(); ++ui)
            {
                const CNpcAbilityEffect &cAbilityEffect(it->GetTargetEffect(ui));
                g_NetworkResourceManager.GetNetIndex(cAbilityEffect.GetIcon());
                g_NetworkResourceManager.GetNetIndex(cAbilityEffect.GetEffect());
            }

            const CNpcProjectile &cProjectile(it->GetProjectileDef());
            g_NetworkResourceManager.GetNetIndex(cProjectile.GetModel());
            g_NetworkResourceManager.GetNetIndex(cProjectile.GetDeathEffect());
            g_NetworkResourceManager.GetNetIndex(cProjectile.GetTrailEffect());
        }
    }

    if (Game.IsClient())
    {
        g_ResourceManager.PrecacheSkin(m_hModel, -1);

        if (!m_sCommanderPortraitPath.empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(m_sCommanderPortraitPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
        
        if (!m_sIconPath.empty())
            g_ResourceManager.Register(K2_NEW(global,   CTexture)(m_sIconPath, TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
    }

    m_itAbility = PickNextAbility();
    m_fHealth = m_fMaxHealth;
    m_fMana = m_fMaxMana;
    m_fStamina = m_fMaxStamina;
    m_eNpcState = NPCSTATE_WAITING;
    m_uiNextActionTime = 0;
    m_bAbilityActivating = false;
    m_v3FaceAngles = m_v3Angles;
    m_eNpcMode = NPCMODE_AGGRESSIVE;

    if (!m_sController.empty())
    {
        IVisualEntity *pEntity(Game.GetEntityFromName(m_sController));
        if (pEntity && pEntity->GetType() == Entity_NpcController)
        {
            static_cast<CEntityNpcController *>(pEntity)->AddNpc(this);
            m_uiControllerUID = pEntity->GetUniqueID();
        }
    }

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    StartAnimation(_T("idle"), -1);
    m_yDefaultAnim = m_ayAnim[0];

    if (Game.IsServer() && !IsPositionValid(m_v3Position))
    {
        STraceInfo trace;
        Game.TraceBox(trace, m_v3Position + CVec3f(0.0f, 0.0f, 1000.0f), m_v3Position + CVec3f(0.0f, 0.0f, -1000.0f), m_bbBounds, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);
        m_v3Position = trace.v3EndPos;
    }

    Link();

    if (m_sInitialJob.empty() || m_sInitialJob == _T("guardpos"))
    {
        m_eNpcJob = NPCJOB_GUARDPOS;
        m_eNpcState = NPCSTATE_WAITING;
        m_v3TargetPos = m_v3Position;
    }
    else
    {
        m_eNpcJob = NPCJOB_GUARDPOS;
        m_eNpcState = NPCSTATE_WAITING;
        m_v3TargetPos = m_v3Position;
    }

    m_uiNextSightTime = Game.GetGameTime() + M_Randnum(0, 1000);

    IVisualEntity::Spawn();
}


/*====================
  INpcEntity::ServerFrame
  ====================*/
bool    INpcEntity::ServerFrame()
{
    PROFILE("INpcEntity::ServerFrame");

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
    RegenerateStamina(fFrameTime, 0);

    // Check for expired actions
    if (m_uiCurrentActionEndTime <= Game.GetGameTime())
    {
        StartAnimation(_T("idle"), -1);
        SetAction(PLAYER_ACTION_IDLE, -1);
    }

    //
    // AI Update
    //

    DecayAggro(fFrameTime);

    if (m_itAbility == m_vAbilities.end())
    {
        m_itAbility = PickNextAbility();
    }

    bool bGoodJob(false);
    
    switch (m_eNpcJob)
    {
    case NPCJOB_FOLLOW:
        bGoodJob = Follow();
        break;
    case NPCJOB_ATTACK:
        bGoodJob = Attack();
        break;
    case NPCJOB_MOVE:
        bGoodJob = Move();
        break;
    case NPCJOB_GUARDPOS:
        bGoodJob = Think(false);
        break;
    case NPCJOB_IDLE:
        bGoodJob = Idle();
        break;
    case NPCJOB_PATROL:
        bGoodJob = Think(true);
        break;
    case NPCJOB_REPAIR:
        bGoodJob = TryRepair();
        break;
    }

    if (!bGoodJob)
    {
        m_eNpcJob = NPCJOB_IDLE;
        m_eNpcState = NPCSTATE_WAITING;
        m_uiTargetUID = INVALID_INDEX;
    }

    CPath *pPath(Game.AccessPath(m_hPath));
    if (!pPath)
        m_hPath = INVALID_POOL_HANDLE;
    
    // Set facing direction while attacking
    if (m_eNpcState == NPCSTATE_ATTACKING && !(m_iCurrentAction & PLAYER_ACTION_STUNNED))
    {
        CVec3f v3Start(m_v3Position + m_bbBounds.GetMid());
        CVec3f v3Forward;
        
        IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
        if (pTargetEnt && pTargetEnt->IsVisual())
        {
            IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

            CVec3f v3Aim(pVisual->GetPosition() + pVisual->GetBounds().GetMid());
            v3Forward = Normalize(v3Aim - v3Start);
            m_v3FaceAngles = M_GetAnglesFromForwardVec(v3Forward);
        }
        else
        {
            v3Forward = M_GetForwardVecFromAngles(m_v3Angles);
            m_v3FaceAngles = m_v3Angles;
        }
    }

    if (m_eNpcState == NPCSTATE_MOVING && pPath)
    {   
        CVec2f v2Move(pPath->CalcNearGoal(m_v3Position.xy(), GetSpeed() * fFrameTime));
        CVec3f v3EndPos(v2Move.x, v2Move.y, Game.GetTerrainHeight(v2Move.x, v2Move.y));

        if (pPath->IsFinished())
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

    if (m_eNpcState == NPCSTATE_ATTACKING && !(m_iCurrentAction & PLAYER_ACTION_STUNNED))
    {
        if (!m_bAbilityActivating && m_uiNextActionTime < Game.GetGameTime() && m_itAbility != m_vAbilities.end() && m_itAbility->IsReady(Game.GetGameTime()))
        {
            //Console << "Activating " << m_itAbility->GetName() << newl;

            m_bAbilityActivating = true;
            m_bAbilityImpacted = false;
            m_itAbility->Activate(Game.GetGameTime());

            if (!m_itAbility->GetAnim().empty())
                StartAnimation(m_itAbility->GetAnim(), 0);

            m_uiNextActionTime = Game.GetGameTime() + m_itAbility->GetActivationTime();
        }
    }   

    if (m_bAbilityActivating && !m_bAbilityImpacted && m_itAbility->GetLastActivationTime() + m_itAbility->GetImpactTime() <= Game.GetGameTime())
    {
        switch (m_itAbility->GetAttackType())
        {
        case NPCATTACK_PROJECTILE:
            AttackProjectile(*m_itAbility);
            break;
        case NPCATTACK_SNAP:
        case NPCATTACK_TRIGGER:
            AttackSnap(*m_itAbility);
            break;
        case NPCATTACK_SELF:
            AttackSelf(*m_itAbility);
            break;
        }

        m_bAbilityImpacted = true;
    }

    if (m_bAbilityActivating && m_itAbility->GetLastActivationTime() + m_itAbility->GetActivationTime() <= Game.GetGameTime())
    {
        m_bAbilityActivating = false;
        
        m_itAbility = PickNextAbility();
    }
    
    return true;
}


/*====================
  INpcEntity::Damage
  ====================*/
float   INpcEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
    if (pAttacker != NULL && m_eNpcState == NPCSTATE_WAITING && ShouldTarget(pAttacker))
        AddAggro(pAttacker->GetIndex(), npc_sightAggro + npc_damageAggro * fDamage, true);
    else if (pAttacker != NULL && ShouldTarget(pAttacker))
        AddAggro(pAttacker->GetIndex(), npc_damageAggro * fDamage, true);

    return ICombatEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);
}


/*====================
  INpcEntity::Hit
  ====================*/
void    INpcEntity::Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy)
{
    ResHandle hHitEffectPath(INVALID_RESOURCE);

    switch (eHitBy)
    {
    case ENTITY_HIT_BY_MELEE:
        hHitEffectPath = m_hHitByMeleeEffectPath;
        break;
    case ENTITY_HIT_BY_RANGED:
        hHitEffectPath = m_hHitByRangedEffectPath;
        break;
    }

    if (hHitEffectPath == INVALID_RESOURCE)
        return;

    CGameEvent evImpact;
    evImpact.SetSourcePosition(v3Pos);
    evImpact.SetEffect(hHitEffectPath);
    Game.AddEvent(evImpact);
}


/*====================
  INpcEntity::KillReward
  ====================*/
void    INpcEntity::KillReward(IGameEntity *pKiller)
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

    if (m_bSoul)
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
            pSoul->SetTarget(pKillerPlayer->GetIndex());
            pSoul->Spawn();

            if (g_ResourceManager.GetAnim(GetModelHandle(), _T("die_soul")) != -1)
                StartAnimation(_T("die_soul"), -1);
        }
    }

    if (m_fItemDrop >= 1.0f || (m_fItemDrop > 0.0f && M_Randnum(0.0f, 1.0f) <= m_fItemDrop))
    {
        // Spawn a treasure chest
        CEntityChest *pChest(static_cast<CEntityChest*>(Game.AllocateEntity(_T("Entity_Chest"))));
        if (pChest == NULL)
        {
            Console.Warn << _T("Failed to create chest entity") << newl;
        }
        else
        {
            pChest->SetPosition(m_v3Position);

            CVec3f v3Dir(Normalize(pKillerPlayer->GetPosition() - m_v3Position));

            CVec3f v3Angles(M_GetAnglesFromForwardVec(v3Dir));

            pChest->SetAngles(CVec3f(0.0f, 0.0f, v3Angles[YAW]));
            pChest->Spawn();

            ushort unItem(Game.GetRandomItem(pKillerPlayer));

            if (unItem == Persistant_Item)
            {
                ushort unItemData(Game.GetRandomPersistantItem());

                pChest->GiveItem(0, Persistant_Item);

                if (pChest->GetItem(0) != NULL)
                {
                    if (pChest->GetItem(0)->GetAsPersistant() != NULL)
                    {
                        pChest->GetItem(0)->GetAsPersistant()->SetItemData(unItemData);
                        pChest->GetItem(0)->GetAsPersistant()->SetItemID(-1);
                    }
                    else
                    {
                        pChest->RemoveItem(0);
                    }
                }
            }
            else
            {
                pChest->GiveItem(0, unItem);
            }

            if (!pChest->GetItem(0))
            {
                Console << _T("Empty chest!") << newl;
            }

            pChest->StartAnimation(_T("open"), 0);
        }
    }
}


/*====================
  INpcEntity::Kill
  ====================*/
void    INpcEntity::Kill(IVisualEntity *pAttacker, ushort unKillingObjectID)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_fHealth = 0.0f;
    m_fMana = 0.0f;
    m_fStamina = 0.0f;

    m_mapAggro.clear();

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
  INpcEntity::Link
  ====================*/
void    INpcEntity::Link()
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
  INpcEntity::Unlink
  ====================*/
void    INpcEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
        Game.UnlinkEntity(m_uiWorldIndex);
}


/*====================
  INpcEntity::GetAbilityRange
  ====================*/
float   INpcEntity::GetAbilityRange() const
{
    if (m_itAbility != m_vAbilities.end())
        return m_itAbility->GetRange();
    else
        return 0.0f;
}


/*====================
  INpcEntity::Accelerate
  ====================*/
void    INpcEntity::Accelerate(const CVec3f &v3Intent, float fAcceleration)
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
  INpcEntity::Friction
  ====================*/
void    INpcEntity::Friction(float fFriction)
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
  INpcEntity::MoveWalkGround
  ====================*/
void    INpcEntity::MoveWalkGround()
{
    // Friction
    float fRelativeFactor(s_Move.v3Velocity.Length() > s_Move.fRunSpeed ? s_Move.v3Velocity.Length() : s_Move.fRunSpeed);
    float fGroundFriction(fRelativeFactor * npc_groundFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

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
    Accelerate(s_Move.v3Intent, npc_groundAccelerate);

    StepSlide(false);
}


/*====================
  INpcEntity::MoveWalkAir
  ====================*/
void    INpcEntity::MoveWalkAir()
{
    if (s_Move.iMoveFlags & MOVE_ON_GROUND)
    {
        // Friction
        float fGroundFriction(npc_slopeFriction * DotProduct(s_Move.plGround.v3Normal, CVec3f(0.0f, 0.0f, 1.0f)));

        // Apply ground friction
        Friction(fGroundFriction);

        // Project movement intent onto the ground plane
        s_Move.v3Intent.SetDirection(Clip(s_Move.v3Intent.Direction(), s_Move.plGround.v3Normal));
    }

    Accelerate(s_Move.v3Intent, npc_airAccelerate);

    Slide(true);
}


/*====================
  INpcEntity::MoveWalk
  ====================*/
bool    INpcEntity::MoveWalk(float fFrameTime, const CVec3f &v3TargetPos, bool bContinue)
{
    PROFILE("INpcEntity::MoveWalk");

    if (GetStatus() == ENTITY_STATUS_DEAD || GetStatus() == ENTITY_STATUS_DORMANT)
        return true;

    int iNewMovement(NPC_MOVE_IDLE);
    bool bFinished(false);

    FloatMod modSpeed;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modSpeed += m_apState[i]->GetSpeedMod();
    }

    // Set working vars
    s_Move.fFrameTime = fFrameTime;
    s_Move.v3OldPosition = m_v3Position;
    s_Move.v3OldVelocity = m_v3Velocity;
    s_Move.v3OldAngles = m_v3Angles;
    s_Move.v3Position = m_v3Position;
    s_Move.v3Velocity = m_v3Velocity;
    s_Move.v3Angles = m_v3Angles;
    s_Move.fRunSpeed = modSpeed.Modify(m_fSpeed);
    s_Move.iMoveFlags = m_iMoveFlags;
    s_Move.bLanded = false;

    s_Move.v3Angles[PITCH] = 0.0f;
    s_Move.v3Angles[ROLL] = 0.0f;

    CheckGround();

    CVec3f v3Intent;
    if (v3TargetPos != m_v3Position && !(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
    {
        // Movement direction
        CVec3f v3Delta(v3TargetPos - m_v3Position);

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

    float fAngleStep(npc_turnSpeed * fFrameTime);

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

    if (s_Move.bGroundControl)
        MoveWalkGround();
    else
        MoveWalkAir();

    if (s_Move.v3Position != s_Move.v3OldPosition)
    {
        Unlink();

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
            if (Game.IsServer())
                Console << _T("NPC move got stuck at ") << ParenStr(m_v3Position) << newl;
        }

        m_v3Angles = s_Move.v3Angles;

        Link();
    }
    else
    {
        m_v3Angles = s_Move.v3Angles;
    }

    if (m_iMoveFlags & MOVE_JUMPING)
    {
        iNewMovement = PLAYER_MOVE_JUMP;
    }
    else
    {
        if (v3Intent.LengthSq() > 0.0f)
            iNewMovement |= PLAYER_MOVE_FWD;
    }

    // Animation
    if ((iNewMovement & ~NPC_MOVE_IGNORE_FLAGS) != (m_iCurrentMovement & ~NPC_MOVE_IGNORE_FLAGS))
    {
        float fNewRelativeSpeed(s_Move.fRunSpeed / m_fSpeed);

        if (!(m_iCurrentAction & PLAYER_ACTION_IMMOBILE))
        {
            tstring sAnimName;
            float fAnimSpeed(1.0f);

            switch ((iNewMovement & NPC_MOVE_NO_FLAGS) &~ NPC_MOVE_IGNORE_FLAGS)
            {
            case NPC_MOVE_IMMOBILE:
                // Don't alter animations
                break;

            case NPC_MOVE_JUMP:
                //sAnimName = _T("jump");
                break;

            case NPC_MOVE_IDLE:
                if (GetCurrentItem() != NULL && GetCurrentItem()->IsGun())
                {           
                    sAnimName = _T("gun_idle");
                    m_yDefaultAnim = GetAnimIndex(_T("gun_idle"));
                }
                else if (iNewMovement & NPC_MOVE_TIRED)
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

            case NPC_MOVE_FWD:
            case NPC_MOVE_FWD_LEFT:
            case NPC_MOVE_FWD_RIGHT:
                {
                    sAnimName = _T("run_fwd");
                    fAnimSpeed = fNewRelativeSpeed;
                }
                break;

            case NPC_MOVE_BACK:
            case NPC_MOVE_BACK_LEFT:
            case NPC_MOVE_BACK_RIGHT:
                sAnimName = _T("run_back");
                fAnimSpeed = fNewRelativeSpeed;
                break;

            case NPC_MOVE_LEFT:
                {
                    sAnimName = _T("run_left");
                    fAnimSpeed = fNewRelativeSpeed;
                }
                break;

            case NPC_MOVE_RIGHT:
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
    }

    // Land Animation
    if (s_Move.bLanded && (iNewMovement & NPC_MOVE_NO_FLAGS) == NPC_MOVE_IDLE)
    {
        StartAnimation(_T("land"), 0);
    }

    return bFinished;
}


/*====================
  INpcEntity::AttackProjectile
  ====================*/
void    INpcEntity::AttackProjectile(const CNpcAbility &cAbility)
{
    CVec3f v3Start(m_v3Position + m_bbBounds.GetMid());
    CVec3f v3Forward;
    
    IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
    if (pTargetEnt && pTargetEnt->IsVisual())
    {
        IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

        CVec3f v3Aim(pVisual->GetPosition() + pVisual->GetBounds().GetMid());
        v3Forward = Normalize(v3Aim - v3Start);
    }
    else
    {
        v3Forward = M_GetForwardVecFromAngles(m_v3Angles);
    }

    cAbility.SpawnProjectile(this, v3Start, v3Forward);
}


/*====================
  INpcEntity::AttackTrace
  ====================*/
void    INpcEntity::AttackTrace(const CNpcAbility &cAbility)
{
    CVec3f v3Start(m_v3Position + m_bbBounds.GetMid());
    CVec3f v3Forward;
    
    IGameEntity *pTargetEnt(Game.GetEntityFromUniqueID(m_uiTargetUID));
    if (pTargetEnt && pTargetEnt->IsVisual())
    {
        IVisualEntity *pVisual(pTargetEnt->GetAsVisualEnt());

        CVec3f v3Aim(pVisual->GetPosition() + pVisual->GetBounds().GetMid());
        v3Forward = Normalize(v3Aim - v3Start);
    }
    else
    {
        v3Forward = M_GetForwardVecFromAngles(m_v3Angles);
    }

    CVec3f v3End(M_PointOnLine(v3Start, v3Forward, cAbility.GetRange()));

    STraceInfo trace;
    Game.TraceLine(trace, v3Start, v3End, SURF_PROJECTILE, m_uiWorldIndex);

    IGameEntity *pHitEnt(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));
    if (trace.uiSurfFlags & SURF_TERRAIN || pHitEnt == NULL)
        cAbility.ImpactPosition(m_uiIndex, trace.v3EndPos);
    else
        cAbility.Impact(m_uiIndex, pHitEnt->GetIndex());
}


/*====================
  INpcEntity::AttackSnap
  ====================*/
void    INpcEntity::AttackSnap(const CNpcAbility &cAbility)
{
    cAbility.Impact(m_uiIndex, Game.GetGameIndexFromUniqueID(m_uiTargetUID));
}


/*====================
  INpcEntity::AttackSelf
  ====================*/
void    INpcEntity::AttackSelf(const CNpcAbility &cAbility)
{
    cAbility.Impact(m_uiIndex, m_uiIndex);
}


/*====================
  INpcEntity::PickNextAbility
  ====================*/
AbilityIter INpcEntity::PickNextAbility()
{
    if (m_vAbilities.size() == 0)
        return m_vAbilities.end();

    vector<AbilityIter> vReady;
    float fSelectionWeightRange(0.0f);

    for (AbilityIter it(m_vAbilities.begin()); it != m_vAbilities.end(); ++it)
    {
        if (it->IsReady(Game.GetGameTime()))
        {
            vReady.push_back(it);
            fSelectionWeightRange += it->GetWeight();
        }
    }

    // if no attacks are ready, choose one that will be ready next
    if (vReady.empty())
    {
        AbilityIter it(m_vAbilities.begin());
        AbilityIter itBest(m_vAbilities.begin());

        for (; it != m_vAbilities.end(); ++it)
        {
            if (it == itBest)
                continue;

            if (it->GetActivationTime() == INVALID_TIME || it->GetActivationTime() + it->GetCooldownTime() < itBest->GetActivationTime() + it->GetCooldownTime())
                itBest = it;
        }

        return itBest;
    }
    
    float   fRand(M_Randnum(0.0f, fSelectionWeightRange));
    vector<AbilityIter>::iterator itAbility(vReady.begin());

    for (;;)
    {
        fRand -= (*itAbility)->GetWeight();

        if (fRand > 0.0f)
        {
            ++itAbility;
            if (itAbility == vReady.end())
                break;
        }
        else
        {
            break;
        }
    }

    if (itAbility == vReady.end())
        return m_vAbilities.begin();

    return *itAbility;
}


/*====================
  INpcEntity::AddAggro
  ====================*/
void    INpcEntity::AddAggro(uint uiIndex, float fAggro, bool bMulti)
{
    if (m_yStatus == ENTITY_STATUS_DEAD || m_yStatus == ENTITY_STATUS_CORPSE)
        return;

    IVisualEntity *pOther(Game.GetVisualEntity(uiIndex));
    if (!pOther)
        return;

    AggroMap::iterator it(m_mapAggro.find(pOther->GetUniqueID()));
    if (it == m_mapAggro.end())
        m_mapAggro[pOther->GetUniqueID()] = fAggro;
    else
        m_mapAggro[pOther->GetUniqueID()] = MIN(it->second + fAggro, float(npc_maxAggro));

    DisturbController();

    if (bMulti)
    {
        uivector vAllies;
        Game.GetEntitiesInRadius(vAllies, CSphere(m_v3Position, m_fMultiAggroRadius), 0);
        for (uivector_it it(vAllies.begin()); it != vAllies.end(); ++it)
        {
            IVisualEntity *pOther(Game.GetEntityFromWorldIndex(*it));

            if (pOther && M_Randnum(0.0f, 1.0f) < m_fMultiAggroProc && pOther->IsNpc() && pOther->GetTeam() == m_iTeam && pOther->GetAsNpc()->GetNpcState() == NPCSTATE_WAITING)
                pOther->GetAsNpc()->AddAggro(uiIndex, fAggro, false);
        }
    }
}


/*====================
  INpcEntity::GetMaxAggro
  ====================*/
uint    INpcEntity::GetMaxAggro()
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
  INpcEntity::GetAggro
  ====================*/
float   INpcEntity::GetAggro(uint uiIndex)
{
    AggroMap::iterator it(m_mapAggro.find(uiIndex));

    if (it != m_mapAggro.end())
        return it->second;

    return 0.0f;
}


/*====================
  INpcEntity::DecayAggro
  ====================*/
void    INpcEntity::DecayAggro(float fFrameTime)
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
            fAmount = npc_aggroDecayDead * fFrameTime;
        else if (pVisual->GetUniqueID() == m_uiTargetUID && m_eNpcState == NPCSTATE_ATTACKING)
            fAmount = npc_aggroDecayAttacking * fFrameTime;
        else
            fAmount = npc_aggroDecayVisible * fFrameTime;

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
  INpcEntity::ShouldTarget
  ====================*/
bool    INpcEntity::ShouldTarget(IGameEntity *pOther)
{
    if (pOther == NULL)
        return false;

    if (!pOther->IsVisual())
        return false;

    IVisualEntity *pVisual(pOther->GetAsVisualEnt());

    if (LooksLikeEnemy(pVisual) &&
        pVisual->GetStatus() == ENTITY_STATUS_ACTIVE && !pVisual->IsStealthed() &&
        (pOther->IsCombat() || pOther->IsGadget() || pOther->IsBuilding() || pOther->IsNpc()) &&
        pVisual->AIShouldTarget())
        return true;
    else
        return false;
}


/*====================
  INpcEntity::AddToScene
  ====================*/
bool    INpcEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    PROFILE("INpcEntity::AddToScene");

    if (Game.IsCommander() && !m_bSighted)
        return false;

    return IVisualEntity::AddToScene(v4Color, iFlags);
}


/*====================
  INpcEntity::Follow
  ====================*/
bool    INpcEntity::Follow()
{
    if (m_eNpcState == NPCSTATE_WAITING)
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

        if (DistanceSq(GetPosition(), pVisual->GetPosition()) < SQR(GetBounds().GetDim(X) + pVisual->GetBounds().GetDim(X) + GetFollowDistance() * 1.5f))
            return true;

        m_v3TargetPos = pVisual->GetPosition();

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_eNpcState = NPCSTATE_MOVING;

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

    m_v3TargetPos = pVisual->GetPosition();

    // Check for repath
    if (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f)
    {
        Game.FreePath(m_hPath);
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
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

    if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR((m_bbBounds.GetDim(X) + pVisual->GetBounds().GetDim(X)) * 0.5f + GetFollowDistance()))
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_eNpcState = NPCSTATE_WAITING;

        return true;
    }

    return true;
}


/*====================
  INpcEntity::Attack
  ====================*/
bool    INpcEntity::Attack()
{
    if (m_uiTargetUID == INVALID_INDEX)
        return false;

    if (m_eNpcState == NPCSTATE_WAITING)
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

        m_v3TargetPos = pVisual->GetPosition();

        if (DistanceSq(GetPosition(), m_v3TargetPos) > SQR(pVisual->GetBounds().GetDim(X) * 0.5f + GetAbilityRange()))
        {
            // Start the move
            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
            m_eNpcState = NPCSTATE_MOVING;

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

    m_v3TargetPos = pVisual->GetPosition();

    // Check for repath
    if (m_eNpcState == NPCSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f))
    {
        Game.FreePath(m_hPath);
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    bool bCanAttack(false);
    if (DistanceSq(GetPosition(), pVisual->GetPosition()) < SQR(pVisual->GetBounds().GetDim(X) * 0.5f + GetAbilityRange()))
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

        m_eNpcState = NPCSTATE_ATTACKING;
    }
    else if (m_hPath == INVALID_POOL_HANDLE && m_eNpcState == NPCSTATE_ATTACKING && m_uiNextActionTime < Game.GetGameTime())
    {
        // Start a new move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_eNpcState = NPCSTATE_MOVING;

        if (m_hPath == INVALID_POOL_HANDLE)
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(1000, 1500); // Failed, so repath sooner
        else
            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);

        m_v3OldTargetPos = m_v3TargetPos;
    }

    return true;
}


/*====================
  INpcEntity::TryRepair
  ====================*/
bool    INpcEntity::TryRepair()
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
    m_v3TargetPos = pVisual->GetPosition();

    // Path to target
    if ((m_eNpcState == NPCSTATE_WAITING && DistanceSq(GetPosition(), m_v3TargetPos) > SQR(fRange)) ||
        (m_eNpcState == NPCSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || DistanceSq(m_v3TargetPos, m_v3OldTargetPos) > 100.0f * 100.0f)))
    {
        Game.FreePath(m_hPath);

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_eNpcState = NPCSTATE_MOVING;

        m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        m_v3OldTargetPos = m_v3TargetPos;
    }

    // Check if target is in range
    bool bCanRepair(false);
    if (DistanceSq(GetPosition(), pVisual->GetPosition()) < SQR(fRange + pVisual->GetBounds().GetMax(X)))
    {
        STraceInfo trace;
        CAxis axis(GetAngles());
        CVec3f v3Forward(axis.Forward());
        CVec3f v3Start(GetPosition() + V_UP * GetViewHeight());
        CVec3f v3End(v3Start + (v3Forward * fRange));
        Game.TraceBox(trace, v3Start, v3End, GetBounds(), TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);
        if (trace.uiEntityIndex == pVisual->GetWorldIndex())
            bCanRepair = true;
    }

    if (bCanRepair)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;

        m_eNpcState = NPCSTATE_REPAIRING;
    }
    else if (m_hPath == INVALID_POOL_HANDLE && m_eNpcState == NPCSTATE_REPAIRING && m_uiNextActionTime < Game.GetGameTime())
    {
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_eNpcState = NPCSTATE_MOVING;

        m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        m_v3OldTargetPos = m_v3TargetPos;
    }

    return true;
}


/*====================
  INpcEntity::Move
  ====================*/
bool    INpcEntity::Move()
{
    if (m_eNpcState == NPCSTATE_WAITING)
    {
        if (DistanceSq(GetPosition(), m_v3TargetPos) < SQR(m_bbBounds.GetDim(X)))
            return true;

        // Start the move
        m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
        m_eNpcState = NPCSTATE_MOVING;

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

        m_eNpcState = NPCSTATE_WAITING;

        return true;
    }

    return true;
}


/*====================
  INpcEntity::Idle
  ====================*/
bool    INpcEntity::Idle()
{
    return true;
}


/*====================
  INpcEntity::Think

  Combines guard position and patrol, maybe others later
  ====================*/
bool    INpcEntity::Think(bool bMoveAggro)
{
    PROFILE("INpcEntity::Think");

    static uivector s_vSight;

    if ((bMoveAggro || m_eNpcState == NPCSTATE_WAITING) && m_eNpcMode == NPCMODE_AGGRESSIVE && GetAggroRadius() > 0.0f && GetMaxAggro() == INVALID_INDEX && m_uiNextSightTime <= Game.GetGameTime() && Game.GetGamePhase() != GAME_PHASE_WARMUP)
    {
        //
        // Search aggro radius for enemies
        //

        s_vSight.clear();
        Game.GetEntitiesInRadius(s_vSight, CSphere(GetPosition(), GetAggroRadius()), 0);
        uint uiTarget(INVALID_INDEX);
        for (uivector_it it(s_vSight.begin()); it != s_vSight.end(); ++it)
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
            AddAggro(uiTarget, npc_sightAggro, true);

            // Sight adjacent aggro
            s_vSight.clear();
            Game.GetEntitiesInRadius(s_vSight, CSphere(pTargetEnt->GetPosition(), GetAggroRadius() / 2.0f), 0);
            for (uivector_it it(s_vSight.begin()); it != s_vSight.end(); ++it)
            {
                uint uiIndex(Game.GetGameIndexFromWorldIndex(*it));

                IVisualEntity *pOther(Game.GetVisualEntity(uiIndex));

                if (ShouldTarget(pOther))
                    AddAggro(uiIndex, npc_sightAdjacentAggro, true);
            }
        }

        m_uiNextSightTime = Game.GetGameTime() + M_Randnum(800, 1200);
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

            m_eNpcState = NPCSTATE_WAITING;
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

            m_eNpcState = NPCSTATE_WAITING;
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

            if (m_eNpcState == NPCSTATE_MOVING && (m_uiRepathTime < Game.GetGameTime() || DistanceSq(pVisual->GetPosition(), m_v3OldTargetPos) > 100.0f * 100.0f))
            {
                Game.FreePath(m_hPath);

                m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, pVisual->GetPosition().xy());
                m_eNpcState = NPCSTATE_MOVING;

                m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
                m_v3OldTargetPos = pVisual->GetPosition();
            }

            if (DistanceSq(GetPosition(), pVisual->GetPosition()) < SQR(pVisual->GetBounds().GetDim(X) * 0.5f + GetAbilityRange()))
            {
                Game.FreePath(m_hPath);
                m_hPath = INVALID_POOL_HANDLE;

                if (m_eNpcJob == NPCJOB_ATTACK || m_eNpcJob == NPCJOB_GUARDPOS || m_eNpcJob == NPCJOB_PATROL)
                    m_eNpcState = NPCSTATE_ATTACKING;
                else if (m_eNpcJob == NPCJOB_REPAIR)
                    m_eNpcState = NPCSTATE_REPAIRING;
                else
                    m_eNpcState = NPCSTATE_WAITING;
            }
            else if (m_hPath == INVALID_POOL_HANDLE && m_eNpcState != NPCSTATE_MOVING)
            {
                m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, pVisual->GetPosition().xy());
                m_eNpcState = NPCSTATE_MOVING;

                m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
                m_v3OldTargetPos = pVisual->GetPosition();
            }
        }
    }
    else
    {
        float fDistance(Distance(GetPosition(), m_v3TargetPos));

        if (m_eNpcState == NPCSTATE_WAITING/* && fDistance > m_fGuardRadius*/ && fDistance > GetBounds().GetDim(X))
        {
            //
            // Head towards target position
            //

            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3TargetPos.xy());
            m_eNpcState = NPCSTATE_MOVING;

            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
            m_v3OldTargetPos = m_v3TargetPos;
        }
        else if (m_eNpcState == NPCSTATE_MOVING && fDistance <= GetBounds().GetDim(X))
        {
            Game.FreePath(m_hPath);
            m_hPath = INVALID_POOL_HANDLE;

            m_eNpcState = NPCSTATE_WAITING;
        }

        if (m_eNpcState == NPCSTATE_MOVING && m_uiRepathTime <= Game.GetGameTime())
        {
            Game.FreePath(m_hPath);
            m_hPath = Game.FindPath(GetPosition().xy(), GetBounds().GetDim(X), INVALID_INDEX, m_v3OldTargetPos.xy());

            m_uiRepathTime = Game.GetGameTime() + M_Randnum(5000, 6000);
        }
    }

    return true;
}


/*====================
  INpcEntity::PlayerCommand
  ====================*/
void    INpcEntity::PlayerCommand(ENpcCommand eNpcCmd, uint uiIndex, const CVec3f &v3Pos)
{
    switch (eNpcCmd)
    {
    case NPCCMD_ATTACK:
        CommandAttack(uiIndex);
        break;
    case NPCCMD_MOVE:
        CommandMove(v3Pos);
        break;
    case NPCCMD_STOP:
        CommandStop();
        break;
    case NPCCMD_FOLLOW:
        CommandFollow(uiIndex);
        break;
    case NPCCMD_SPECIALABILITY:
        CommandSpecialAbility();
        break;
    case NPCCMD_RETURN:
        CommandReturn();
        break;
    case NPCCMD_TOGGLEAGGRO:
        CommandToggleAggro();
        break;
    case NPCCMD_BANISH:
        CommandBanish();
        break;
    case NPCCMD_REPAIR:
        CommandRepair(uiIndex);
        break;
    case NPCCMD_PATROL:
        CommandPatrol(v3Pos);
        break;
    }

#if 0
    IGameEntity *pOwner(Game.GetEntityFromUniqueID(GetOwnerUID()));

    if (pOwner != NULL && pOwner->IsPlayer())
    {
        CBufferFixed<2> buffer;
        buffer << GAME_CMD_NPCCMD_ORDERCONFIRMED;
        Game.SendGameData(pOwner->GetAsPlayerEnt()->GetClientID(), buffer, false);
    }
#endif

    DisturbController();
}


/*====================
  INpcEntity::CommandAttack
  ====================*/
void    INpcEntity::CommandAttack(uint uiIndex)
{
    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_eNpcJob = NPCJOB_ATTACK;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_eNpcState = NPCSTATE_WAITING;
}


/*====================
  INpcEntity::CommandMove
  ====================*/
void    INpcEntity::CommandMove(const CVec3f &v3Pos)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_eNpcJob = NPCJOB_MOVE;
    m_v3TargetPos = v3Pos;
    m_eNpcState = NPCSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  INpcEntity::CommandStop
  ====================*/
void    INpcEntity::CommandStop()
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_eNpcJob = NPCJOB_IDLE;
    m_eNpcState = NPCSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  INpcEntity::CommandFollow
  ====================*/
void    INpcEntity::CommandFollow(uint uiIndex)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    m_eNpcJob = NPCJOB_FOLLOW;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_eNpcState = NPCSTATE_WAITING;
}


/*====================
  INpcEntity::CommandPatrol
  ====================*/
void    INpcEntity::CommandPatrol(const CVec3f &v3Pos)
{
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_eNpcJob = NPCJOB_PATROL;
    m_v3TargetPos = v3Pos;
    m_eNpcState = NPCSTATE_WAITING;
    m_uiTargetUID = INVALID_INDEX;
}


/*====================
  INpcEntity::CommandSpecialAbility
  ====================*/
void    INpcEntity::CommandSpecialAbility()
{
    //m_yActivate = 1;
}


/*====================
  INpcEntity::CommandReturn
  ====================*/
void    INpcEntity::CommandReturn()
{
#if 0
    Game.FreePath(m_hPath);
    m_hPath = INVALID_POOL_HANDLE;

    m_eNpcJob = NPCJOB_FOLLOW;
    m_uiTargetUID = m_uiOwnerUID;
    m_eNpcState = NPCSTATE_WAITING;
#endif
}


/*====================
  INpcEntity::CommandToggleAggro
  ====================*/
void    INpcEntity::CommandToggleAggro()
{
    if (m_eNpcMode == NPCMODE_PASSIVE)
        m_eNpcMode = NPCMODE_AGGRESSIVE;
    else if (m_eNpcMode == NPCMODE_AGGRESSIVE)
        m_eNpcMode = NPCMODE_DEFENSIVE;
    else
        m_eNpcMode = NPCMODE_PASSIVE;
}


/*====================
  INpcEntity::CommandBanish
  ====================*/
void    INpcEntity::CommandBanish()
{
    Kill();
    m_eNpcState = NPCSTATE_WAITING;
}


/*====================
  INpcEntity::CommandRepair
  ====================*/
void    INpcEntity::CommandRepair(uint uiIndex)
{
    IGameEntity *pTargetEnt(Game.GetEntity(uiIndex));
    if (!pTargetEnt)
        return;

    m_eNpcJob = NPCJOB_REPAIR;
    m_uiTargetUID = pTargetEnt->GetUniqueID();
    m_eNpcState = NPCSTATE_WAITING;
}


/*====================
  INpcEntity::DisturbController
  ====================*/
void    INpcEntity::DisturbController()
{
    IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiControllerUID));
    if (!pEntity || pEntity->GetType() != Entity_NpcController)
        return;

    static_cast<CEntityNpcController *>(pEntity)->SetDistrubed(true);
}

