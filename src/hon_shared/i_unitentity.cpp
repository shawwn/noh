// (C)2008 S2 Games
// i_unitentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_unitentity.h"

#include "c_player.h"
#include "i_buildingentity.h"
#include "c_teaminfo.h"
#include "i_entitystate.h"
#include "i_gadgetentity.h"
#include "i_heroentity.h"
#include "i_projectile.h"
#include "c_combatevent.h"
#include "i_entityitem.h"
#include "c_entitychest.h"
#include "i_behavior.h"
#include "i_shopentity.h"
#include "c_shopdefinition.h"
#include "c_player.h"
#include "c_shopinfo.h"

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
#include "../k2/c_effect.h"
#include "../k2/c_texture.h"
#include "../k2/c_model.h"
#include "../k2/i_emitter.h"
#include "../k2/c_effectthread.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, Icon)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, Portrait)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, Model)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, MapIconProperty)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, AttackStartEffect)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, AttackActionEffect)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, AttackImpactEffect)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, PassiveEffect)
MUTABLE_MULTI_LEVEL_RESOURCE_IMPL(IUnitEntity, SpawnEffect)
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint            IUnitEntity::s_uiBaseType(ENTITY_BASE_TYPE_UNIT);
ResHandle       IUnitEntity::s_hSelectionIndicator(INVALID_RESOURCE);
ResHandle       IUnitEntity::s_hRecipeEffect(INVALID_RESOURCE);
ResHandle       IUnitEntity::s_hMinimapIcon(INVALID_RESOURCE);

CVAR_BOOL       (d_printSlideInfo,                  false);
CVAR_BOOL       (d_drawUnitBounds,                  false);

CVAR_UINT       (g_minimapFlashInterval,            500);
CVAR_FLOATF     (g_elevationAdvantageMin,           50.0f,          CVAR_GAMECONFIG);

CVAR_FLOATF     (g_unitMapIconSize,                 0.010f,         CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF    (g_unitMapIcon,                     "$white",       CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF     (g_denyCreepHealthPercent,          0.5f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (g_denyHeroHealthPercent,           0.1f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF     (g_denyBuildingHealthPercent,       0.1f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF     (g_damageLinkAmount,                0.9f,           CVAR_GAMECONFIG);

CVAR_BOOLF      (g_unitPlayGibAnims,                true,           CVAR_GAMECONFIG);
CVAR_BOOLF      (g_unitPlayDenyAnims,               true,           CVAR_GAMECONFIG);
CVAR_FLOATF     (g_unitStealthFadeAmount,           0.5f,           CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF     (g_afkBaseDistance,                 3000.0f,        CVAR_GAMECONFIG);

CVAR_BOOL       (g_perks,                           false);

CVAR_INTF       (g_elevationStepMin,                -32,            CVAR_GAMECONFIG);
CVAR_INTF       (g_elevationStepMax,                96,             CVAR_GAMECONFIG);

CVAR_UINTF      (unit_blockRepathTime,              100,            CVAR_GAMECONFIG);
CVAR_UINTF      (unit_blockRepathTimeExtra,         50,             CVAR_GAMECONFIG);
CVAR_FLOATF     (unit_slideThreshold,               0.1f,           CVAR_GAMECONFIG);

CVAR_FLOATF     (cg_healthShadowDecay,              0.25f,          CVAR_SAVECONFIG);
CVAR_UINTF      (cg_healthShadowDelay,              1500,           CVAR_SAVECONFIG);
CVAR_FLOATF     (cg_manaShadowDecay,                1.0f,           CVAR_SAVECONFIG);
CVAR_UINTF      (cg_manaShadowDelay,                250,            CVAR_SAVECONFIG);

CVAR_FLOATF     (g_afkWanderRadius,                 1000.0f,        CVAR_SAVECONFIG);

DEFINE_ENTITY_DESC(IUnitEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IGameEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position.xy"), TYPE_DELTAPOS2D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3Position.z"), TYPE_DELTAPOS1D, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_v3UnitAngles[YAW]"), TYPE_ANGLE8, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yStatus"), TYPE_CHAR, 3, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySequence"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unVisibilityFlags"), TYPE_SHORT, 16, 0));
    
    for (int i(0); i < 1; ++i)
    {
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnim[") + XtoA(i) + _T("]"), TYPE_CHAR, 5, -1));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ayAnimSequence[") + XtoA(i) + _T("]"), TYPE_CHAR, 2, 0));
        s_cDesc.pFieldTypes->push_back(SDataField(_T("m_afAnimSpeed[") + XtoA(i) + _T("]"), TYPE_FLOAT, 32, 0));
    }

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTeamID"), TYPE_INT, 3, TEAM_INVALID));

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_unUnitFlags"), TYPE_SHORT, 9, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLevel"), TYPE_INT, 5, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fHealth"), TYPE_CEIL16, 15, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fMana"), TYPE_FLOOR16, 15, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_iOwnerClientNumber"), TYPE_INT, 5, -1));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiOwnerEntityIndex"), TYPE_GAMEINDEX, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fFade"), TYPE_BYTEPERCENT, 0, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiSpawnTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLifetime"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IUnitEntity::~IUnitEntity
  ====================*/
IUnitEntity::~IUnitEntity()
{
    if (IGame::GetCurrentGamePointer() == nullptr)
        return;

    if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
    {
        Game.UnlinkEntity(m_uiWorldIndex);
        Game.DeleteWorldEntity(m_uiWorldIndex);
    }

    UnblockPath();

    // Delete entire inventory
    for (int iSlot(0); iSlot < MAX_INVENTORY; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        Game.DeleteEntity(m_apInventory[iSlot]);
        m_apInventory[iSlot] = nullptr;
    }

    m_pMorphState = nullptr;
}


/*====================
  IUnitEntity::IUnitEntity
  ====================*/
IUnitEntity::IUnitEntity() :
m_unUnitFlags(0),
m_uiLevel(0),
m_fHealth(0.0f),
m_fMana(0.0f),
m_fCurrentMaxHealth(FAR_AWAY),
m_fCurrentMaxMana(FAR_AWAY),
m_fHealthShadowMarker(0.0f),
m_fManaShadowMarker(0.0f),
m_uiHealthShadowTime(INVALID_TIME),
m_uiManaShadowTime(INVALID_TIME),
m_fAccumulator(0.0f),
m_fHealthAccumulator(0.0f),
m_fLethalDamageAccumulator(0.0f),
m_fNonLethalDamageAccumulator(0.0f),
m_uiKiller(INVALID_INDEX),
m_unDeathInflictor(INVALID_ENT_TYPE),

m_iExclusiveAttackModSlot(MAX_INVENTORY),
m_bHadStashAccess(false),

m_v3UnitAngles(V3_ZERO),
m_v3AttentionAngles(V3_ZERO),

m_uiTargetIndex(INVALID_INDEX),

m_iOwnerClientNumber(-1),
m_uiOwnerEntityIndex(INVALID_INDEX),

m_uiCorpseTime(INVALID_TIME),
m_uiCorpseFadeTime(INVALID_TIME),
m_uiDeathTime(INVALID_TIME),
m_fFade(0.0f),

m_fTiltPitch(0.0f),
m_fTiltRoll(0.0f),

m_fBonusDamage(0.0f),
m_fBonusDamageMultiplier(0.0f),

m_v2BlockPosition(FAR_AWAY, FAR_AWAY),

m_uiAttackCooldownTime(INVALID_TIME),
m_uiSpawnTime(INVALID_TIME),
m_uiLifetime(0),
m_fReceiveDamageMultiplier(1.0f),
m_fInflictDamageMultiplier(1.0f),
m_fExperienceBountyMultiplier(1.0f),
m_fGoldBountyMultiplier(1.0f),
m_uiDeathFlags(0),
m_uiMiscFlags(0),
m_fTotalTrackedDamage(0.0f),
m_fMovementDistance(0.0f),
m_uiDisjointSequence(0),
m_uiOrderDisjointSequence(0),
m_uiArmingSequence(0),
m_pMount(nullptr),
m_uiIdleStartTime(INVALID_TIME),
m_fCurrentDamage(0.0f),
m_eCurrentDamageSuperType(SUPERTYPE_INVALID),
m_uiCurrentDamageEffectType(0),
m_hDeathEffect(INVALID_RESOURCE),
m_uiLastHeroAttackTime(INVALID_TIME),
m_pMorphState(nullptr),
m_uiControllerUID(INVALID_INDEX),
m_uiFadeStartTime(INVALID_TIME),
m_uiProxyUID(INVALID_INDEX),
m_unAttackProjectile(INVALID_ENT_TYPE),
m_hAttackActionEffect(INVALID_RESOURCE),
m_hAttackImpactEffect(INVALID_RESOURCE),
m_uiAttackAbilityUID(INVALID_INDEX),
m_uiLastAttackTargetUID(INVALID_INDEX),
m_uiLastAttackTargetTime(INVALID_TIME),
m_uiLinkedFlags(0),
m_uiAttackSequence(0),
m_uiOrderSequence(1),

m_uiGuardChaseTime(0),
m_uiGuardChaseDistance(0),
m_uiGuardReaggroChaseTime(0),
m_uiGuardReaggroChaseDistance(0),
m_uiOverrideAggroRange(0),

m_bUseAltDeathAnims(false),

m_uiAllUnitsExceptCouriersScheme(INVALID_TARGET_SCHEME),
m_bIsTower(false),
m_bIsKongor(false)
{
    for (int i(0); i < MAX_INVENTORY; ++i)
        m_apInventory[i] = nullptr;

    m_uiLinkFlags = SURF_UNIT;

    m_auiLastAggression[0] = INVALID_TIME;
    m_auiLastAggression[1] = INVALID_TIME;
}


/*====================
  IUnitEntity::Baseline
  ====================*/
void    IUnitEntity::Baseline()
{
    IVisualEntity::Baseline();

    // Basic data
    m_v3Position = V3_ZERO;
    m_v3UnitAngles = V3_ZERO;
    m_yStatus = ENTITY_STATUS_ACTIVE;
    m_ySequence = 0;
    m_unVisibilityFlags = 0;

    // Anims
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        m_ayAnim[i] = ENTITY_INVALID_ANIM;
        m_ayAnimSequence[i] = 0;
        m_afAnimSpeed[i] = 1.0f;
    }

    m_uiTeamID = TEAM_PASSIVE;

    m_unUnitFlags = 0;
    m_uiLevel = 0;
    m_fHealth = 0.0f;
    m_fMana = 0.0f;

    m_iOwnerClientNumber = -1;
    m_uiOwnerEntityIndex = INVALID_INDEX;

    m_fFade = 0.0f;

    m_uiSpawnTime = INVALID_TIME;
    m_uiLifetime = 0;
}


/*====================
  IUnitEntity::GetSnapshot
  ====================*/
void    IUnitEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IGameEntity::GetSnapshot(snapshot, uiFlags);

    // Basic data
    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteDeltaPos2D(CVec2f(0.0f, 0.0f));
        snapshot.WriteDeltaPos1D(0.0f);
        snapshot.WriteAngle8(0.0f);
    }
    else
    {
        snapshot.WriteDeltaPos2D(m_v3Position.xy());
        snapshot.WriteDeltaPos1D(m_v3Position.z);
        snapshot.WriteAngle8(m_v3UnitAngles.z);
    }

    snapshot.WriteField(m_yStatus);
    snapshot.WriteField(m_ySequence);
    snapshot.WriteField(m_unVisibilityFlags);

    // Anims
    for (int i(0); i < 1; ++i)
    {
        snapshot.WriteInteger(m_ayAnim[i]);
        snapshot.WriteField(m_ayAnimSequence[i]);
        snapshot.WriteField(m_afAnimSpeed[i]);
    }

    snapshot.WriteInteger(m_uiTeamID);

    snapshot.WriteField(m_unUnitFlags);
    snapshot.WriteField(m_uiLevel);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteCeil16(0.0f);
        snapshot.WriteFloor16(0.0f);
    }
    else
    {
        snapshot.WriteCeil16(m_fHealth);
        snapshot.WriteFloor16(m_fMana);
    }

    snapshot.WriteInteger(m_iOwnerClientNumber);
    snapshot.WriteGameIndex(m_uiOwnerEntityIndex);
    snapshot.WriteBytePercent(m_fFade);
    snapshot.WriteField(m_uiSpawnTime);
    snapshot.WriteField(m_uiLifetime);

}


/*====================
  IUnitEntity::ReadSnapshot
  ====================*/
bool    IUnitEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        // Base entity info
        if (!IGameEntity::ReadSnapshot(snapshot, 1))
            return false;
        
        snapshot.ReadDeltaPos2D(m_v3Position.xy());
        snapshot.ReadDeltaPos1D(m_v3Position.z);
        snapshot.ReadAngle8(m_v3UnitAngles.z);
        snapshot.ReadField(m_yStatus);
        snapshot.ReadField(m_ySequence);
        snapshot.ReadField(m_unVisibilityFlags);

        // Anims
        for (int i(0); i < 1; ++i)
        {
            snapshot.ReadInteger(m_ayAnim[i]);
            snapshot.ReadField(m_ayAnimSequence[i]);
            snapshot.ReadField(m_afAnimSpeed[i]);
        }

        snapshot.ReadInteger(m_uiTeamID);

        snapshot.ReadField(m_unUnitFlags);
        snapshot.ReadField(m_uiLevel);
        snapshot.ReadFloat16(m_fHealth);
        snapshot.ReadFloat16(m_fMana);
        snapshot.ReadInteger(m_iOwnerClientNumber);
        snapshot.ReadGameIndex(m_uiOwnerEntityIndex);
        snapshot.ReadBytePercent(m_fFade);
        snapshot.ReadField(m_uiSpawnTime);
        snapshot.ReadField(m_uiLifetime);

#if 1 // Temporary hack to exponge restricted data
        CPlayer *pLocalPlayer(Game.GetLocalPlayer());
        if (pLocalPlayer != nullptr && pLocalPlayer->GetTeam() != TEAM_SPECTATOR && pLocalPlayer->GetTeam() != GetTeam())
        {
            m_unUnitFlags &= ~UNIT_FLAG_ILLUSION;
            m_uiSpawnTime = INVALID_TIME;
            m_uiLifetime = 0;
        }
#endif

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IUnitEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IUnitEntity::Damage
  ====================*/
void    IUnitEntity::Damage(CDamageEvent &damage)
{
    // Allow inventory items to react to damage
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (!m_apInventory[iSlot]->OwnerDamaged(damage))
            RemoveSlave(iSlot);
    }

    // Apply damage multipliers
    IUnitEntity *pAttacker(Game.GetUnitEntity(damage.GetAttackerIndex()));
    float fInflictDamageMultipler(pAttacker != nullptr && damage.GetSuperType() == SUPERTYPE_ATTACK ? pAttacker->GetInflictDamageMultiplier() : 1.0f);

    float fOldDamage(m_fCurrentDamage);
    ESuperType eOldSuperType(m_eCurrentDamageSuperType);
    uint uiOldEffectType(m_uiCurrentDamageEffectType);

    m_fCurrentDamage = damage.GetAppliedDamage() * fInflictDamageMultipler;
    m_eCurrentDamageSuperType = damage.GetSuperType();
    m_uiCurrentDamageEffectType = damage.GetEffectType();

    // Distribute damage to "soul linked" entities
    if (!m_setSoulLinks.empty() && !damage.HasFlag(DAMAGE_FLAG_NO_LINK))
    {
        float fLinkDamage(m_fCurrentDamage * g_damageLinkAmount);
        m_fCurrentDamage -= fLinkDamage;

        fLinkDamage /= m_setSoulLinks.size();
        CDamageEvent dmgLink(damage);
        dmgLink.SetAmount(fLinkDamage);
        dmgLink.SetFlag(DAMAGE_FLAG_NO_LINK);
        dmgLink.SetEffectType(Game.LookupEffectType(_T("Magic")));

        Damage(dmgLink);
        for (uiset_it it(m_setSoulLinks.begin()); it != m_setSoulLinks.end(); ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(*it)));
            if (pUnit == nullptr)
                continue;

            pUnit->Damage(dmgLink);
        }
    }

    // Track damage
    if (pAttacker != nullptr)
    {
        // Per-frame tracking, for kill credit
        m_vFrameDamageTrackers.push_back(SDamageTrackerFrame(pAttacker->GetUniqueID(), m_fCurrentDamage));
        m_fTotalTrackedDamage += m_fCurrentDamage;

        // Accumulators, for logging
        if (pAttacker->GetOwnerClientNumber() != -1)
        {
            IGameEntity *pInflictor(Game.GetEntity(damage.GetInflictorIndex()));
            ushort unInflictorType(pInflictor == nullptr ? INVALID_ENT_TYPE : pInflictor->GetType());

            LogDamageVector_it it(m_vLogDamageTrackers.begin());
            uint uiOldestTime(INVALID_TIME);
            uint uiOldestIndex(0);
            for (; it != m_vLogDamageTrackers.end(); ++it)
            {
                if (it->uiTimeStamp < uiOldestTime)
                {
                    uiOldestTime = it->uiTimeStamp;
                    uiOldestIndex = it - m_vLogDamageTrackers.begin();
                }

                if (it->iPlayerOwner == pAttacker->GetOwnerClientNumber() &&
                    it->unAttackerType == pAttacker->GetType() &&
                    it->unInflictorType == unInflictorType)
                {
                    it->uiTimeStamp = Game.GetGameTime();
                    it->fDamage += m_fCurrentDamage;
                    break;
                }
            }
            if (it == m_vLogDamageTrackers.end())
            {
                if (m_vLogDamageTrackers.size() < MAX_LOG_DAMAGE_TRACKERS)
                {
                    m_vLogDamageTrackers.push_back(SDamageTrackerLog(Game.GetGameTime(), pAttacker->GetOwnerClientNumber(), pAttacker->GetUniqueID(), pAttacker->GetType(), unInflictorType, m_fCurrentDamage));
                }
                else
                {
                    SDamageTrackerLog &dmgLog(m_vLogDamageTrackers[uiOldestIndex]);
                    Game.LogDamage(this, dmgLog.iPlayerOwner, dmgLog.unAttackerType, dmgLog.unInflictorType, dmgLog.fDamage);
                    m_vLogDamageTrackers[uiOldestIndex].uiTimeStamp = Game.GetGameTime();
                    m_vLogDamageTrackers[uiOldestIndex].iPlayerOwner = pAttacker->GetOwnerClientNumber();
                    m_vLogDamageTrackers[uiOldestIndex].uiAttackerUID = pAttacker->GetUniqueID();
                    m_vLogDamageTrackers[uiOldestIndex].unAttackerType = pAttacker->GetType();
                    m_vLogDamageTrackers[uiOldestIndex].unInflictorType = unInflictorType;
                    m_vLogDamageTrackers[uiOldestIndex].fDamage = m_fCurrentDamage;
                }
            }
        }
    }

    Action(ACTION_SCRIPT_DAMAGED, pAttacker, Game.GetEntity(damage.GetInflictorIndex()));

    if (pAttacker != nullptr)
    {
        float fOldAttackerDamage(pAttacker->GetCurrentDamage());

        pAttacker->SetCurrentDamage(m_fCurrentDamage);
        pAttacker->Action(ACTION_SCRIPT_DAMAGE, this, Game.GetEntity(damage.GetInflictorIndex()));
        m_fCurrentDamage = pAttacker->GetCurrentDamage();
        pAttacker->SetCurrentDamage(fOldAttackerDamage);
    }

    m_fCurrentDamage *= GetReceiveDamageMultiplier();

    // Add adjusted damage to accumulator
    if (damage.HasFlag(DAMAGE_FLAG_NON_LETHAL))
        m_fNonLethalDamageAccumulator += m_fCurrentDamage;
    else
        m_fLethalDamageAccumulator += m_fCurrentDamage;

    if (m_fCurrentDamage > 0.0f)
    {
        CPlayer *pOwner(GetOwnerPlayer());
        if (pOwner != nullptr)
            pOwner->SetLastInteractionTime(Game.GetGameTime());

        m_cBrain.Damaged(pAttacker);

        // Track damage stats for scoreboard
        if (pAttacker != nullptr && pAttacker->GetOwnerClientNumber() != -1 && pAttacker->IsEnemy(this))
        {
            if (IsHero())
            {
                CPlayer *pAttackerPlayer(Game.GetPlayer(pAttacker->GetOwnerClientNumber()));
                if (pAttackerPlayer != nullptr)
                    pAttackerPlayer->AdjustFloatStat(PLAYER_STAT_HERO_DAMAGE, m_fCurrentDamage);
            }
            else if (IsBuilding())
            {
                CPlayer *pAttackerPlayer(Game.GetPlayer(pAttacker->GetOwnerClientNumber()));
                if (pAttackerPlayer != nullptr)
                    pAttackerPlayer->AdjustFloatStat(PLAYER_STAT_BUILDING_DAMAGE, m_fCurrentDamage);
            }
        }
    }

    m_fCurrentDamage = fOldDamage;
    m_eCurrentDamageSuperType = eOldSuperType;
    m_uiCurrentDamageEffectType = uiOldEffectType;
}


/*====================
  IUnitEntity::Touch
  ====================*/
void    IUnitEntity::Touch(IGameEntity *pActivator, int iIssuedClientNumber)
{
    if (pActivator == nullptr)
        return;

    int iClientNumber(iIssuedClientNumber);

    if (pActivator->IsUnit())
    {
        bool bSafe(true);
        if (iIssuedClientNumber == -1)
        {
            bSafe = false;
            if (pActivator->GetAsUnit()->GetOwnerClientNumber() != -1)
            {
                bSafe = true;
                iClientNumber = pActivator->GetAsUnit()->GetOwnerClientNumber();
            }
        }

        if (bSafe)
        {
            CBufferFixed<5> buffer;
            buffer << GAME_CMD_TOUCH << ushort(GetIndex()) << ushort(pActivator->GetIndex());
            Game.SendGameData(iClientNumber, buffer, true); // ->GetAsUnit()->GetOwnerClientNumber()
        }
    }

    Action(ACTION_SCRIPT_TOUCHED, pActivator->GetAsUnit(), pActivator);
}


/*====================
  IUnitEntity::Copy
  ====================*/
void    IUnitEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const IUnitEntity *pB(B.GetAsUnit());

    if (!pB)    
        return;

    const IUnitEntity &C(*pB);

    m_v3UnitAngles =        C.m_v3UnitAngles;
    m_unUnitFlags =         C.m_unUnitFlags;
    m_uiLevel =             C.m_uiLevel;
    m_fHealth =             C.m_fHealth;
    m_fMana =               C.m_fMana;
    m_iOwnerClientNumber =  C.m_iOwnerClientNumber;
    m_uiOwnerEntityIndex =  C.m_uiOwnerEntityIndex;
    m_fFade =               C.m_fFade;
    m_uiSpawnTime =         C.m_uiSpawnTime;
    m_uiLifetime =          C.m_uiLifetime;

    for (int i(0); i < MAX_INVENTORY; ++i)
        m_apInventory[i] = C.m_apInventory[i];

    m_pMorphState = C.m_pMorphState;
    m_fCurrentMaxHealth = C.m_fCurrentMaxHealth;
    m_fCurrentMaxMana = C.m_fCurrentMaxMana;
}


/*====================
  IUnitEntity::SnapshotUpdate
  ====================*/
void    IUnitEntity::SnapshotUpdate()
{
    m_pMorphState = GetMorphState();

    m_fCurrentMaxHealth = GetMaxHealth();
    m_fCurrentMaxMana = GetMaxMana();

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
        SetEffect(EFFECT_CHANNEL_PASSIVE, GetPassiveEffect());
    else
        SetEffect(EFFECT_CHANNEL_PASSIVE, INVALID_RESOURCE);
}


/*====================
  IUnitEntity::ClientPrecache
  ====================*/
void    IUnitEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IVisualEntity::ClientPrecache(pConfig, eScheme, sModifier);

    K2_WITH_GAME_RESOURCE_SCOPE()
        s_hSelectionIndicator = g_ResourceManager.Register(g_unitSelectionIndicatorPath, RES_MATERIAL);

    s_hMinimapIcon = Game.RegisterIcon(g_unitMapIcon);
    s_hRecipeEffect = Game.RegisterEffect(g_effectRecipePath);
}


/*====================
  IUnitEntity::ServerPrecache
  ====================*/
void    IUnitEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
    IVisualEntity::ServerPrecache(pConfig, eScheme, sModifier);

    s_hRecipeEffect = Game.RegisterEffect(g_effectRecipePath);
}


/*====================
  IUnitEntity::FlushStats
  ====================*/
void    IUnitEntity::FlushStats()
{
    for (LogDamageVector_it it(m_vLogDamageTrackers.begin()); it != m_vLogDamageTrackers.end(); ++it)
        Game.LogDamage(this, it->iPlayerOwner, it->unAttackerType, it->unInflictorType, it->fDamage);

    m_vLogDamageTrackers.clear();
}


/*====================
  IUnitEntity::ServerFrameSetup
  ====================*/
bool    IUnitEntity::ServerFrameSetup()
{
    PROFILE("IUnitEntity::ServerFrameSetup");

    if (HasLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME))
        return false;

    RemoveLocalFlags(ENT_LOCAL_RECIPE_EFFECT);

    m_fMovementDistance = 0.0f;

    for (int i(INVENTORY_START_ACTIVE); i <= INVENTORY_END_ACTIVE; ++i)
    {
        ISlaveEntity *pSlave(GetSlave(i));
        if (pSlave == nullptr)
            continue;

        if (i == m_iExclusiveAttackModSlot)
            pSlave->SetAttackModPriority();
        else
            pSlave->RemoveAttackModPriority();
    }
    UpdateModifiers();

    if (Game.GetGameTime() >= m_uiAttackCooldownTime)
        m_uiAttackCooldownTime = INVALID_TIME;

    // Visibility
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
        ClearVisibilityFlags();
    else
        ClearVisibilityFlagsDead();

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        // Health and mana regeneration
        float fFrameTime(MsToSec(Game.GetFrameLength()));
        RegenerateMana(fFrameTime);
        RegenerateHealth(fFrameTime);
    }
    
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        if (m_pDefinition != nullptr)
            m_pDefinition->ApplyAuras(this, GetLevel());
    }

    return IVisualEntity::ServerFrameSetup();
}


/*====================
  IUnitEntity::ServerFrameThink
  ====================*/
bool    IUnitEntity::ServerFrameThink()
{
    PROFILE("IUnitEntity::ServerFrameThink");

    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        // AI Update
        m_cBrain.FrameThink();

        bool bImmobilized(IsImmobilized(true, false) || IsStunned());
        bool bShouldBlock(!m_cBrain.GetMoving() || bImmobilized);

        if (GetStatus() != ENTITY_STATUS_ACTIVE ||
            (m_uiLinkFlags & (SURF_ITEM | SURF_BUILDING)) ||
            GetFlying() ||
            (m_uiLinkFlags & SURF_UNIT && GetUnitwalking()) ||
            (m_uiLinkFlags & SURF_UNIT && GetBoundsRadius() == 0.0f))
            bShouldBlock = false;

        if ((!bShouldBlock || m_v2BlockPosition != m_v3Position.xy()) && m_vPathBlockers.size())
            UnblockPath();
        
        if (bShouldBlock && !m_vPathBlockers.size())
            BlockPath();
    }

    return IVisualEntity::ServerFrameThink();
}


/*====================
  IUnitEntity::ServerFrameMovement
  ====================*/
bool    IUnitEntity::ServerFrameMovement()
{
    CVec2f v2StartPos(GetPosition().xy());

    if (!IVisualEntity::ServerFrameMovementStart())
        return false;

    // If unit is bound, it doesn't get to do a normal move
    if (m_uiBindTargetUID != INVALID_INDEX)
    {
        IGameEntity *pGameEntity(Game.GetEntityFromUniqueID(m_uiBindTargetUID));
        IVisualEntity *pBindTarget(pGameEntity ? pGameEntity->GetAsVisual() : nullptr);
        if (pBindTarget == nullptr || pBindTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
        {
            m_uiBindTargetUID = INVALID_INDEX;
            RemoveVisibilityFlags(UNIT_FLAG_BOUND);
        }

        return true;
    }

    // Apply pushes
    CVec2f v2Slide(V2_ZERO);
    for (uint ui(0); ui < m_vPushRecords.size(); ++ui)
    {
        if (m_vPushRecords[ui].second != INVALID_TIME)
            v2Slide += m_vPushRecords[ui].first * MsToSec(Game.GetFrameLength());
    }
    if (v2Slide != V2_ZERO)
    {
        Unlink();

        CPlane plOutImpactPlane;
        Slide(v2Slide, TRACE_UNIT_PUSH, plOutImpactPlane);

        Link();
    }
    else if (m_uiLinkedFlags != GetLinkFlags())
    {
        Unlink();
        Link();
    }

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    // AI Update
    m_cBrain.FrameMovement();

    m_fMovementDistance = Distance(v2StartPos, GetPosition().xy());

    // Unstick
    if (GetIsMobile() && v2Slide == V2_ZERO)
        ValidatePosition2();

    CPlayer *pOwner(GetOwnerPlayer());
    if (pOwner != nullptr)
    {
        if (DistanceSq(m_v2AnchorPosition, GetPosition().xy()) >= SQR(g_afkWanderRadius.GetValue()))
        {
            pOwner->SetLastInteractionTime(Game.GetGameTime());
            m_v2AnchorPosition = GetPosition().xy();
        }
    }

    /*
    CPlayer *pOwner(GetOwnerPlayer());
    if (pOwner != nullptr)
    {
        CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam != nullptr)
        {
            IBuildingEntity *pBase(Game.GetBuildingEntity(pTeam->GetBaseBuildingIndex()));
            if (pBase != nullptr)
            {
                float fDistanceSq(DistanceSq(GetPosition().xy(), pBase->GetPosition().xy()));
                if (fDistanceSq > SQR(g_afkBaseDistance.GetValue()))
                    pOwner->SetLastInteractionTime(Game.GetGameTime());
            }
        }
    }
    */
    
    return IVisualEntity::ServerFrameMovementEnd();
}


/*====================
  IUnitEntity::ServerFrameAction
  ====================*/
bool    IUnitEntity::ServerFrameAction()
{
    if (!IVisualEntity::ServerFrameAction())
        return false;

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    IUnitDefinition *pDef(GetDefinition<IUnitDefinition>());
    if (pDef != nullptr)
        pDef->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, this, nullptr, this, GetPosition(), GetProxy(0), GetLevel());

    // AI Update
    m_cBrain.FrameAction();

    return true;
}


/*====================
  IUnitEntity::ServerFrameCleanup
  ====================*/
bool    IUnitEntity::ServerFrameCleanup()
{
    // Error check position and angles
    if (!m_v3Position.IsValid())
    {
        Console.Err << _T("IUnitEntity: m_v3Position not finite") << newl;

        m_v3Position.Clear();
        Kill();
    }

    if (!m_v3UnitAngles.IsValid())
    {
        Console.Err << _T("IUnitEntity: m_v3UnitAngles not finite") << newl;
        m_v3UnitAngles.Clear();
    }

    if (!m_v3AttentionAngles.IsValid())
    {
        Console.Err << _T("IUnitEntity: m_v3AttentionAngles not finite") << newl;
        m_v3AttentionAngles.Clear();
    }

    if (!m_v3Angles.IsValid())
    {
        Console.Err << _T("IUnitEntity: m_v3Angles not finite") << newl;
        m_v3Angles.Clear();
    }

    // Turn smoothing
    if (GetTurnSmoothing() > 0.0f)
    {
        for (int i(X); i <= Z; ++i)
        {
            while (m_v3UnitAngles[i] - m_v3Angles[i] > 180.0f)
                m_v3UnitAngles[i] -= 360.0f;

            while (m_v3UnitAngles[i] - m_v3Angles[i] < -180.0f)
                m_v3UnitAngles[i] += 360.0f;
        }

        m_v3UnitAngles = DECAY(m_v3UnitAngles, m_v3Angles, GetTurnSmoothing(), Game.GetFrameLength() * SEC_PER_MS);
    }
    else
    {
        m_v3UnitAngles = m_v3Angles;
    }

#if 0
    // Update stash access
    bool bNewStashAccess(GetStashAccess());
    if (!m_bHadStashAccess && bNewStashAccess)
        CheckRecipes(INVENTORY_START_BACKPACK);
    m_bHadStashAccess = bNewStashAccess;
#endif

    // Check for transition from dead to corpse
    if (GetStatus() == ENTITY_STATUS_DEAD && Game.GetGameTime() >= m_uiDeathTime)
    {
        m_uiCorpseTime = Game.GetGameTime() + GetCorpseTime();

        Unlink();
        SetStatus(ENTITY_STATUS_CORPSE);
        Link();
    }

    // Check for an expiring corpse
    if (GetStatus() == ENTITY_STATUS_CORPSE && Game.GetGameTime() >= m_uiCorpseTime)
    {
        if (m_uiCorpseFadeTime == INVALID_TIME)
        {
            if (!GetNoCorpse() && !(HasUnitFlags(UNIT_FLAG_ILLUSION) && !GetIllusionDeathAnim()))
            {
                m_uiCorpseFadeTime = Game.GetGameTime() + GetCorpseFadeTime();

                if (GetCorpseFadeEffect() != INVALID_RESOURCE)
                {
                    CGameEvent ev;
                    ev.SetSourceEntity(GetIndex());
                    ev.SetEffect(GetCorpseFadeEffect());
                    Game.AddEvent(ev);
                }
            }
            else
            {
                m_uiCorpseFadeTime = Game.GetGameTime();
            }
        }
    }

    // Calculate stealth
    if (GetStealthBits() != 0 &&
        GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        m_fFade = GetMaxStealthFade();

        if (m_fFade == 1.0f)
            SetUnitFlags(UNIT_FLAG_STEALTH);
        else
            RemoveUnitFlags(UNIT_FLAG_STEALTH);
    }
    else
    {
        RemoveUnitFlags(UNIT_FLAG_STEALTH);
        m_fFade = 0.0f;
    }

    // Clear expired pushes
    for (uint ui(0); ui < m_vPushRecords.size(); ++ui)
    {
        if (m_vPushRecords[ui].second <= Game.GetGameTime())
        {
            m_vPushRecords[ui].first.Clear();
            m_vPushRecords[ui].second = INVALID_TIME;
        }
    }

    // AI Update
    m_cBrain.FrameCleanup();

    // Update inventory
    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
    {
        IEntityItem *pItem(GetItem(i));
        if (pItem == nullptr)
            continue;

        if (pItem->GetDestroyOnEmpty() &&
            pItem->GetInitialCharges() > 0 &&
            pItem->GetCharges() == 0 &&
            !pItem->HasFlag(ENTITY_TOOL_FLAG_IN_USE) &&
            pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED) &&
            !pItem->IsChanneling(UNIT_ACTION_CAST))
            RemoveItem(i);
    }

    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        IEntityState *pState(GetState(i));
        if (pState == nullptr)
            continue;

        if (pState->IsExpired() || pState->IsAuraInvalid())
            RemoveState(i);
    }

    if (GetStatus() == ENTITY_STATUS_CORPSE && Game.GetGameTime() >= m_uiCorpseFadeTime)
    {
        SetStatus(ENTITY_STATUS_DORMANT);
        Unlink();

        if (HasUnitFlags(UNIT_FLAG_ILLUSION))
            SetLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME);

        return false;
    }

    // If max health or mana changed, adjust the current value to maintain the same percentage
    float fNewMaxHealth(GetMaxHealth());
    if (m_fCurrentMaxHealth != fNewMaxHealth)
    {
        if (m_fCurrentMaxHealth > 0.0f)
            m_fHealth = (m_fHealth / m_fCurrentMaxHealth) * fNewMaxHealth;
        else
            m_fHealth = 0.0f;

        m_fCurrentMaxHealth = fNewMaxHealth;
    }

    float fNewMaxMana(GetMaxMana());
    if (m_fCurrentMaxMana != fNewMaxMana)
    {
        if (m_fCurrentMaxMana > 0.0f)
            m_fMana = (m_fMana / m_fCurrentMaxMana) * fNewMaxMana;
        else
            m_fMana = 0.0f;

        m_fCurrentMaxMana = fNewMaxMana;
    }

    // Apply accumulated damage and healing
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        m_fHealth = MAX(MIN(0.00001f, m_fHealth), m_fHealth + m_fHealthAccumulator);
        m_fHealth = MAX(MIN(0.00001f, m_fHealth), m_fHealth - m_fNonLethalDamageAccumulator);
        m_fHealth -= m_fLethalDamageAccumulator;

        // If the death flag is set, something else already killed this entity
        if (GetDeath())
        {
            Die(Game.GetUnitEntity(m_uiKiller), m_unDeathInflictor);
        }
        else if (m_fCurrentMaxHealth > 0.0f && GetHealth() <= 0.0f)
        {
            IUnitEntity *pKiller(nullptr);

            // Randomly select rewarded unit weighted by damage dealt this frame
            float fRand(M_Randnum(0.0f, m_fTotalTrackedDamage));
            FrameDamageVector_it it(m_vFrameDamageTrackers.begin());

            while (it != m_vFrameDamageTrackers.end())
            {
                fRand -= it->fDamage;

                if (fRand > 0.0f)
                    ++it;
                else
                    break;
            }

            if (it != m_vFrameDamageTrackers.end())
            {
                IGameEntity *pAttacker(Game.GetEntityFromUniqueID(it->uiAttackerUID));
                if (pAttacker != nullptr && pAttacker->IsUnit())
                    pKiller = pAttacker->GetAsUnit();
            }

            Die(pKiller);
        }
    }
    else
    {
        m_fHealth = 0.0f;
        m_fMana = 0.0f;
    }

    m_fHealth = CLAMP(m_fHealth, 0.0f, m_fCurrentMaxHealth);

    // Reset accumulators
    m_fLethalDamageAccumulator = 0.0f;
    m_fNonLethalDamageAccumulator = 0.0f;
    m_fHealthAccumulator = 0.0f;
    m_vFrameDamageTrackers.clear();
    m_fTotalTrackedDamage = 0.0f;

    // Expire
    if ((GetActualLifetime() > 0 && Game.GetGameTime() >= m_uiSpawnTime + GetActualLifetime()) ||
        (GetMaxDistanceFromOwner() > 0.0f && GetOwner() != nullptr && DistanceSq(GetPosition(), GetOwner()->GetPosition()) > SQR(GetMaxDistanceFromOwner())) ||
        GetExpire())
    {
        Expire();
    }

    // If cooldown speed changed, adjust the current value to maintain the same percentage
    float fNewCooldownSpeed(GetCooldownSpeed());
    float fNewCooldownReduction(MIN(GetReducedCooldowns() - GetIncreasedCooldowns(), 1.0f));
    if (m_fCurrentCooldownSpeed != fNewCooldownSpeed || m_fCurrentCooldownReduction != fNewCooldownReduction)
    {
        float fFactor(m_fCurrentCooldownSpeed / fNewCooldownSpeed * (1.0f - fNewCooldownReduction) / (1.0f - m_fCurrentCooldownReduction));

        for (int i(INVENTORY_START_ACTIVE); i <= INVENTORY_END_ACTIVE; ++i)
        {
            IEntityTool *pTool(GetTool(i));
            if (pTool == nullptr)
                continue;

            uint uiCooldownTime(pTool->GetActualRemainingCooldownTime());
            uint uiDuration(pTool->GetCooldownDuration());
            if (uiCooldownTime == 0 || uiCooldownTime == INVALID_TIME ||
                uiDuration == 0 || uiDuration == INVALID_TIME)
                continue;

            float fPercent(pTool->GetActualRemainingCooldownPercent());

            uint uiNewDuration(INT_CEIL(uiDuration * fFactor));
            uint uiNewStartTime(Game.GetGameTime() - INT_CEIL(uiNewDuration * (1.0f - fPercent)));

            pTool->SetCooldownStartTime(uiNewStartTime);
            pTool->SetCooldownDuration(uiNewDuration);
        }

        for (map<uint, SCooldown>::iterator it(m_mapCooldowns.begin()); it != m_mapCooldowns.end(); ++it)
        {
            uint uiCooldownTime(Game.GetCooldownEndTime(it->second.uiStartTime, it->second.uiDuration));
            uint uiDuration(it->second.uiDuration);
            if (uiCooldownTime == 0 || uiCooldownTime == INVALID_TIME ||
                uiDuration == 0 || uiDuration == INVALID_TIME)
                continue;

            float fPercent;

            if (uiCooldownTime == INVALID_TIME)
                fPercent = 1.0f;
            else if (uiCooldownTime == 0 || Game.GetGameTime() >= uiCooldownTime)
                fPercent = 0.0f;
            else
                fPercent = (uiCooldownTime - Game.GetGameTime()) / float(uiDuration);

            uint uiNewDuration(INT_CEIL(uiDuration * fFactor));
            uint uiNewStartTime(Game.GetGameTime() - INT_CEIL(uiNewDuration * (1.0f - fPercent)));

            it->second.uiStartTime = uiNewStartTime;
            it->second.uiDuration = uiNewDuration;
        }

        for (int i(INVENTORY_START_ACTIVE); i <= INVENTORY_END_ACTIVE; ++i)
        {
            IEntityTool *pTool(GetTool(i));
            if (pTool == nullptr)
                continue;

            pTool->UpdateApparentCooldown();
        }

        m_fCurrentCooldownSpeed = fNewCooldownSpeed;
        m_fCurrentCooldownReduction = fNewCooldownReduction;
    }

    // Remove bad soul links
    for (uiset_it it(m_setSoulLinks.begin()); it != m_setSoulLinks.end(); ++it)
    {
        IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(*it)));
        if (pUnit == nullptr || pUnit->GetStatus() != ENTITY_STATUS_ACTIVE)
        {
            STL_ERASE(m_setSoulLinks, it);
            if (it == m_setSoulLinks.end())
                break;
        }
    }

    if (m_asAnim[0] == GetIdleAnim() || m_asAnim[0] == _T("bored_1"))
    {
        if (m_uiIdleStartTime == INVALID_TIME)
            m_uiIdleStartTime = Game.GetGameTime() + M_Randnum(6000, 10000);
    }
    else
    {
        m_uiIdleStartTime = INVALID_TIME;
    }

    if (m_uiIdleStartTime != INVALID_TIME && m_uiIdleStartTime < Game.GetGameTime() && GetAnimIndex(_T("bored_1")) != -1)
    {
        StartAnimation(_T("bored_1"), 0);
        m_uiIdleStartTime = Game.GetGameTime() + M_Randnum(16000, 20000);
    }

    // Translate anims to current model
    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
    {
        if (!m_asAnim[i].empty())
            m_ayAnim[i] = g_ResourceManager.GetAnim(GetModel(), m_asAnim[i]);
        else
            m_ayAnim[i] = ENTITY_STOP_ANIM;
    }

    m_uiDeathFlags = 0;

    return IVisualEntity::ServerFrameCleanup();
}


typedef pair<int, int> iipair;

static bool ActionScriptPriorityPred(const iipair &elem1, const iipair &elem2)
{
    if (elem1.second == elem2.second)
        return elem1.first < elem2.first;
    else
        return elem1.second > elem2.second;
}

/*====================
  IUnitEntity::Action
  ====================*/
void    IUnitEntity::Action(EEntityActionScript eAction, IUnitEntity *pTarget, IGameEntity *pInflictor, CCombatEvent *pCombatEvent, CDamageEvent *pDamageEvent)
{
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo != nullptr)
        pGameInfo->ExecuteActionScript(eAction, this, pInflictor, pTarget, V_ZERO);

    CVec3f v3TargetPosition(GetPosition());
    if (pTarget != nullptr)
        v3TargetPosition = pTarget->GetPosition();

    // Build Priority map
    vector<iipair> vScriptPriorities;
    vScriptPriorities.clear();
    
    IUnitDefinition *pDef(GetDefinition<IUnitDefinition>());
    if (pDef != nullptr)
        vScriptPriorities.push_back(iipair(-1, pDef->GetActionScriptPriority(eAction)));

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        ISlaveDefinition *pDef(pSlave->GetDefinition<ISlaveDefinition>());
        if (pDef != nullptr)
            vScriptPriorities.push_back(iipair(iSlot, pDef->GetActionScriptPriority(eAction)));
    }

    sort(vScriptPriorities.begin(), vScriptPriorities.end(), ActionScriptPriorityPred);

    for (vector<iipair>::iterator it(vScriptPriorities.begin()), itEnd(vScriptPriorities.end()); it != itEnd; ++it)
    {
        if (it->first == -1)
        {
            IUnitDefinition *pDef(GetDefinition<IUnitDefinition>());
            if (pDef != nullptr)
                pDef->ExecuteActionScript(eAction, this, this, pInflictor, pTarget, v3TargetPosition, GetProxy(0), GetLevel(), pCombatEvent, pDamageEvent);
        }
        else
        {
            ISlaveEntity *pSlave(m_apInventory[it->first]);
            if (pSlave == nullptr)
                continue;
            if (!pSlave->IsActive())
                continue;

            ISlaveDefinition *pDef(pSlave->GetDefinition<ISlaveDefinition>());
            if (pDef != nullptr)
                pDef->ExecuteActionScript(eAction, pSlave, this, pInflictor, pTarget, v3TargetPosition, pSlave->GetProxy(0), pSlave->GetLevel(), pCombatEvent, pDamageEvent);
        }
    }
}

void    IUnitEntity::Action(EEntityActionScript eAction, const CVec3f &v3Target, IGameEntity *pInflictor, CCombatEvent *pCombatEvent, CDamageEvent *pDamageEvent)
{
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo != nullptr)
        pGameInfo->ExecuteActionScript(eAction, this, pInflictor, nullptr, v3Target);

    // Build Priority map
    static vector<iipair> vScriptPriorities;
    vScriptPriorities.clear();
    
    IUnitDefinition *pDef(GetDefinition<IUnitDefinition>());
    if (pDef != nullptr)
        vScriptPriorities.push_back(iipair(-1, pDef->GetActionScriptPriority(eAction)));

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        ISlaveDefinition *pDef(pSlave->GetDefinition<ISlaveDefinition>());
        if (pDef != nullptr)
            vScriptPriorities.push_back(iipair(iSlot, pDef->GetActionScriptPriority(eAction)));
    }

    sort(vScriptPriorities.begin(), vScriptPriorities.end(), ActionScriptPriorityPred);

    for (vector<iipair>::iterator it(vScriptPriorities.begin()), itEnd(vScriptPriorities.end()); it != itEnd; ++it)
    {
        if (it->first == -1)
        {
            IUnitDefinition *pDef(GetDefinition<IUnitDefinition>());
            if (pDef != nullptr)
                pDef->ExecuteActionScript(eAction, this, this, pInflictor, nullptr, v3Target, GetProxy(0), GetLevel(), pCombatEvent, pDamageEvent);
        }
        else
        {
            ISlaveEntity *pSlave(m_apInventory[it->first]);
            if (pSlave == nullptr)
                continue;
            if (!pSlave->IsActive())
                continue;

            ISlaveDefinition *pDef(pSlave->GetDefinition<ISlaveDefinition>());
            if (pDef != nullptr)
                pDef->ExecuteActionScript(eAction, pSlave, this, pInflictor, nullptr, v3Target, pSlave->GetProxy(0), pSlave->GetLevel(), pCombatEvent, pDamageEvent);
        }
    }
}


/*====================
  IUnitEntity::GetCriticals
  ====================*/
void    IUnitEntity::GetCriticals(CCombatEvent &cmbt) const
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;

        if (pSlave->IsActive())
            cmbt.AddCritical(pSlave->GetCriticalChance(), pSlave->GetCriticalMultiplier());
    }
}


/*====================
  IUnitEntity::GetAttackActions
  ====================*/
void    IUnitEntity::GetAttackActions(EEntityActionScript eScriptFrom, EEntityActionScript eScriptTo, CCombatEvent &cmbt) const
{
    if (m_pDefinition != nullptr)
    {
        CCombatActionScript *pScript(m_pDefinition->GetActionScript(eScriptFrom));
        if (pScript != nullptr && (!IsIllusion() || pScript->GetPropagateToIllusions()))
        {
            CCombatActionScript &cScript(cmbt.AddActionScript(eScriptTo, *pScript));
            cScript.SetLevel(GetLevel());
            cScript.SetThisUID(GetUniqueID());
        }
    }

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        ISlaveDefinition *pSlaveDefinition(pSlave->GetDefinition<ISlaveDefinition>());
        if (pSlaveDefinition != nullptr)
        {
            CCombatActionScript *pScript(pSlaveDefinition->GetActionScript(eScriptFrom));
            if (pScript != nullptr && (!IsIllusion() || pScript->GetPropagateToIllusions()))
            {
                CCombatActionScript &cScript(cmbt.AddActionScript(eScriptTo, *pScript));
                cScript.SetLevel(pSlave->GetLevel());
                cScript.SetThisUID(pSlave->GetUniqueID());
            }
        }
    }
}


/*====================
  IUnitEntity::GetDeflection
  ====================*/
float   IUnitEntity::GetDeflection() const
{
    float fDeflection(0.0f);
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(GetSlave(iSlot));
        if (pSlave == nullptr)
            continue;
        if (!CHANCE(pSlave->GetDeflectionChance()))
            continue;

        fDeflection = MAX(fDeflection, pSlave->GetDeflection());
    }

    return fDeflection;
}


/*====================
  IUnitEntity::UpdateModifiers
  ====================*/
void    IUnitEntity::UpdateModifiers()
{
    PROFILE("IUnitEntity::UpdateModifiers");

    ushort unModifierBits(0);

    m_vModifierKeys = m_vPersistentModifierKeys;

    const uivector &vGlobalModifiers(Game.GetGlobalModifiers());
    m_vModifierKeys.insert(m_vModifierKeys.end(), vGlobalModifiers.begin(), vGlobalModifiers.end());

    if (m_uiActiveModifierKey != INVALID_INDEX)
        m_vModifierKeys.push_back(m_uiActiveModifierKey);

    // Find and activate the appropriate exclusive modifiers
    map<uint, SModifierEntry>   mapActiveExclusive; // Key is modifier ID

    GetActiveExclusiveModifiers(this, mapActiveExclusive, 0);

    // Gather modifier keys from slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        uint uiModifierKey(pSlave->GetModifierKey());
        if (uiModifierKey != INVALID_INDEX && uiModifierKey != 0)
            m_vModifierKeys.push_back(uiModifierKey);

        uint uiModifierKey2(pSlave->GetModifierKey2());
        if (uiModifierKey2 != INVALID_INDEX && uiModifierKey2 != 0)
            m_vModifierKeys.push_back(uiModifierKey2);

        pSlave->GetActiveExclusiveModifiers(this, mapActiveExclusive, pSlave->HasAttackModPriority() ? 1000 : 0);
    }

    // Set exclusive modifiers that are active on this entity
    for (map<uint, SModifierEntry>::iterator it(mapActiveExclusive.begin()), itEnd(mapActiveExclusive.end()); it != itEnd; ++it)
    {
        if (it->second.uiGameIndex == m_uiIndex)
            unModifierBits |= GetModifierBit(it->first);
    }

    unModifierBits |= GetModifierBits(m_vModifierKeys);

    // Each tool of this entity receives all of their master's modifiers
    // States receive their modifiers from their inflictor at application time
    for (int iSlot(0); iSlot < MAX_INVENTORY; ++iSlot)
    {
        ISlaveEntity *pSlave(m_apInventory[iSlot]);
        if (pSlave == nullptr)
            continue;

        if (pSlave->IsState())
        {
            IEntityState *pState(pSlave->GetAsState());

            pState->UpdateModifiers();

            // Set exclusive modifiers that are active on this slave
            for (map<uint, SModifierEntry>::iterator it(mapActiveExclusive.begin()), itEnd(mapActiveExclusive.end()); it != itEnd; ++it)
            {
                if (it->second.uiGameIndex == pState->GetIndex())
                    pState->SetModifierBits(pState->GetModifierBits() | pState->GetModifierBit(it->first));
            }
        }
        else if (pSlave->IsTool())
        {
            IEntityTool *pTool(pSlave->GetAsTool());

            pTool->UpdateModifiers(m_vModifierKeys);

            // Set exclusive modifiers that are active on this slave
            for (map<uint, SModifierEntry>::iterator it(mapActiveExclusive.begin()), itEnd(mapActiveExclusive.end()); it != itEnd; ++it)
            {
                if (it->second.uiGameIndex == pTool->GetIndex())
                    pTool->SetModifierBits(pTool->GetModifierBits() | pTool->GetModifierBit(it->first));
            }
        }
    }

    // Grab base definition
    IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
    if (pDefinition == nullptr)
        return; // Oh dear...

    // Search this entity
    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());

    // Activate conditional modifiers
    for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
    {
        if (cit->second->GetExclusive())
            continue;

        const tstring &sCondition(cit->second->GetCondition());
        if (sCondition.empty())
            continue;

        tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

        tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
        for (; itType != itTypeEnd; ++itType)
        {
            if (!itType->empty() && (*itType)[0] == _T('!'))
            {
                if (IsTargetType(itType->substr(1), this))
                    break;
            }
            else
            {
                if (!IsTargetType(*itType, this))
                    break;
            }
        }
        if (itType == itTypeEnd)
            unModifierBits |= cit->first;
    }

    SetModifierBits(unModifierBits);
}


/*====================
  IUnitEntity::GetSlaveModifiers
  ====================*/
void    IUnitEntity::GetSlaveModifiers(uivector &vModifierKeys)
{
    PROFILE("IUnitEntity::GetSlaveModifiers");

    vModifierKeys.clear();

    const uivector &vGlobalModifiers(Game.GetGlobalModifiers());
    vModifierKeys.insert(vModifierKeys.end(), vGlobalModifiers.begin(), vGlobalModifiers.end());

    // Find and activate the appropriate exclusive modifiers
    map<uint, SModifierEntry>   mapActiveExclusive; // Key is modifier ID

    GetActiveExclusiveModifiers(this, mapActiveExclusive, 0);

    // Gather modifier keys from slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        ISlaveEntity *pSlave(GetSlave(iSlot));
        if (pSlave == nullptr)
            continue;
        if (!pSlave->IsActive())
            continue;

        uint uiModifierKey(pSlave->GetModifierKey());
        if (uiModifierKey != INVALID_INDEX && uiModifierKey != 0)
            vModifierKeys.push_back(uiModifierKey);

        uint uiModifierKey2(pSlave->GetModifierKey2());
        if (uiModifierKey2 != INVALID_INDEX && uiModifierKey2 != 0)
            vModifierKeys.push_back(uiModifierKey2);

        pSlave->GetActiveExclusiveModifiers(this, mapActiveExclusive, pSlave->HasAttackModPriority() ? 1000 : 0);
    }
}


/*====================
  IUnitEntity::TiltTrace
  ====================*/
float   IUnitEntity::TiltTrace(const CVec3f &v3Start, const CVec3f &v3End)
{
    STraceInfo trace;

    if (Game.TraceLine(trace, v3Start, v3End, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex))
        return trace.fFraction;
    else
        return 1.0f;
}


/*====================
  IUnitEntity::AttachModel
  ====================*/
void    IUnitEntity::AttachModel(const tstring &sBoneName, ResHandle hModel)
{
    if (hModel == INVALID_RESOURCE || sBoneName.empty())
        return;

    try
    {
        if (m_pSkeleton == nullptr)
            EX_WARN(_T("Invalid skeleton"));

        uint uiBone(m_pSkeleton->GetBone(sBoneName));
        if (uiBone == INVALID_BONE)
            EX_WARN(_T("Bone ") + QuoteStr(sBoneName) + _T(" does not exist"));

        SBoneState *pBone(m_pSkeleton->GetBoneState(uiBone));
        
        CMatrix4x3<float> tmPlayer(m_aAxis, GetPosition());
        CMatrix4x3<float> tmBone(CAxis_cast(pBone->tm_local.axis), CVec3_cast(pBone->tm_local.pos) * GetBaseScale() * GetScale());
        CMatrix4x3<float> tmWorld(tmPlayer * tmBone);

        CSceneEntity sceneEntity;
        sceneEntity.scale = GetBaseScale() * GetScale();
        sceneEntity.axis = tmWorld.GetAxis();
        sceneEntity.SetPosition(tmWorld.GetPosition());
        sceneEntity.hRes = hModel;
        sceneEntity.flags |= SCENEENT_USE_AXIS;

        // Check for a state setting an alternate skin
        tstring sAlternateSkin;
        for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
        {
            if (m_apInventory[iSlot] == nullptr)
                continue;

            //sAlternateSkin = m_apInventory[i]->GetSkin();
            if (!sAlternateSkin.empty())
            {
                sceneEntity.hSkin = g_ResourceManager.GetSkin(hModel, sAlternateSkin);
                break;
            }
        }

        SceneManager.AddEntity(sceneEntity);
    }
    catch (CException &ex)
    {
        ex.Process(_T("IUnitEntity::AttachModel() - "), NO_THROW);
    }
}


/*====================
  IUnitEntity::GetMapIconColor
  ====================*/
CVec4f  IUnitEntity::GetMapIconColor(CPlayer *pLocalPlayer) const
{
    CVec4f v4Color(GetMapIconColorProperty());

    if (v4Color != V4_ZERO)
        return v4Color;

    uint uiGameTime(Game.GetGameTime());
    if (uiGameTime < m_uiMinimapFlashEndTime && (uiGameTime % g_minimapFlashInterval < (g_minimapFlashInterval / 2)))
        return m_v4MinimapFlashColor;

    if (pLocalPlayer == nullptr || pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
    {
        if (GetTeam() == TEAM_PASSIVE || GetTeam() == TEAM_SPECTATOR)
            return WHITE;
        else if (GetTeam() == 1)
            return GetStatus() == ENTITY_STATUS_ACTIVE ? LIME : GREEN;
        else if (GetTeam() == 2)
            return GetStatus() == ENTITY_STATUS_ACTIVE ? RED : MAROON;
        else
            return ORANGE;
    }

    if (GetTeam() == TEAM_PASSIVE || GetTeam() == TEAM_SPECTATOR)
        return WHITE;
    else if (pLocalPlayer->IsEnemy(this))
        return RED;
    else if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return GREEN;
    else
        return LIME;
}


/*====================
  IUnitEntity::GetMapIconSize
  ====================*/
float   IUnitEntity::GetMapIconSize(CPlayer *pLocalPlayer) const
{
    float fSize(GetMapIconSizeProperty());

    if (fSize != 0.0f)
        return fSize;

    return g_unitMapIconSize;
}


/*====================
  IUnitEntity::GetMapIcon
  ====================*/
ResHandle   IUnitEntity::GetMapIcon(CPlayer *pLocalPlayer) const
{
    ResHandle hIcon(GetMapIconProperty());

    if (hIcon == INVALID_RESOURCE)
        return s_hMinimapIcon;

    return hIcon;
}


/*====================
  IUnitEntity::GetTeamColor
  ====================*/
CVec4f  IUnitEntity::GetTeamColor(CPlayer *pLocalPlayer) const
{
    if (pLocalPlayer == nullptr || pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
    {
        if (GetTeam() == TEAM_PASSIVE || GetTeam() == TEAM_SPECTATOR)
            return GRAY;
        else if (GetTeam() == 1)
            return GREEN;
        else if (GetTeam() == 2)
            return MAROON;
        else
            return ORANGE;
    }

    if (GetTeam() == TEAM_PASSIVE || GetTeam() == TEAM_SPECTATOR)
        return CVec4f(0.75f, 0.75f, 0.75f, 1.0f);
    else if (pLocalPlayer->IsEnemy(this))
        return CVec4f(0.75f, 0.0f, 0.0f, 1.0f);
    else
        return CVec4f(0.0f, 0.75f, 0.0f, 1.0f);
}


/*====================
  IUnitEntity::GetStealthBits
  ====================*/
uint    IUnitEntity::GetStealthBits() const
{
    uint uiStealthBits(GetStealthType());

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        uiStealthBits |= m_apInventory[iSlot]->GetStealthType();
    }

    return uiStealthBits;
}


/*====================
  IUnitEntity::GetStealthFade
  ====================*/
float   IUnitEntity::GetStealthFade() const
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE || GetStealthType() == 0)
        return 0.0f;
    else if (m_uiFadeStartTime == INVALID_TIME)
        return 1.0f;
    else if (Game.GetGameTime() < m_uiFadeStartTime)
        return 0.0f;
    else if (GetFadeTime() == 0)
        return 1.0f;
    else
        return CLAMP((Game.GetGameTime() - m_uiFadeStartTime) / float(GetFadeTime()), 0.0f, 1.0f);
}


/*====================
  IUnitEntity::GetMaxStealthFade
  ====================*/
float   IUnitEntity::GetMaxStealthFade() const
{
    float fMaxStealthFade(0.0f);
    if (GetStealthType())
        fMaxStealthFade = MAX(fMaxStealthFade, GetStealthFade());

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (m_apInventory[iSlot]->GetStealthType())
            fMaxStealthFade = MAX(fMaxStealthFade, m_apInventory[iSlot]->GetStealthFade());
    }

    return fMaxStealthFade;
}


/*====================
  IUnitEntity::IsStealth
  ====================*/
bool    IUnitEntity::IsStealth(bool bCheckRevealed) const
{
    if (bCheckRevealed && HasUnitFlags(UNIT_FLAG_REVEALED))
        return false;
    if (!HasUnitFlags(UNIT_FLAG_STEALTH))
        return false;
    if (GetStealthBits() == 0)
        return false;

    return true;
}


/*====================
  IUnitEntity::GetMinStealthProximity

  TODO: Maybe add a per-stealthtype check for proximity
  ====================*/
float   IUnitEntity::GetMinStealthProximity() const
{
    float fMinStealthProximity(FAR_AWAY);
    if (GetStealthType())
        fMinStealthProximity = MIN(fMinStealthProximity, GetStealthProximity());

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (m_apInventory[iSlot]->GetStealthType())
            fMinStealthProximity = MIN(fMinStealthProximity, m_apInventory[iSlot]->GetStealthProximity());
    }

    return fMinStealthProximity;
}


/*====================
  IUnitEntity::CanSee
  ====================*/
bool    IUnitEntity::CanSee(const IVisualEntity *pTarget) const
{
    CPlayer *pOwner(Game.GetPlayer(GetOwnerClientNumber()));
    if (pOwner != nullptr)
        return pOwner->CanSee(pTarget);

    if (pTarget == nullptr)
        return false;
    const IUnitEntity *pUnit(pTarget->GetAsUnit());
    if (pUnit == nullptr)
        return true;
    
    // Neutrals don't have vision
    if (GetTeam() < 1 || GetTeam() > 2)
    {
        if (pUnit->IsStealth())
            return false;

        return true;
    }

    if (pUnit->IsStealth() && !pUnit->HasVisibilityFlags(VIS_REVEALED(GetTeam())))
        return false;
    if (pUnit->HasVisibilityFlags(VIS_SIGHTED(GetTeam())))
        return true;

    return false;
}


/*====================
  IUnitEntity::GetAdjustedImmunityType
  ====================*/
uint    IUnitEntity::GetAdjustedImmunityType() const
{
    PROFILE("IUnitEntity::GetAdjustedImmunityType");

    uint uiImmunityType(GetImmunityType());

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        uiImmunityType |= m_apInventory[iSlot]->GetImmunityType();
    }

    return uiImmunityType;
}


/*====================
  IUnitEntity::UpdateInventory
  ====================*/
void    IUnitEntity::UpdateInventory()
{
    PROFILE("IUnitEntity::UpdateInventory");

    GiveItem(0, EntityRegistry.LookupID(GetInventory0()));
    GiveItem(1, EntityRegistry.LookupID(GetInventory1()));
    GiveItem(2, EntityRegistry.LookupID(GetInventory2()));
    GiveItem(3, EntityRegistry.LookupID(GetInventory3()));
    GiveItem(4, EntityRegistry.LookupID(GetInventory4()));
    GiveItem(5, EntityRegistry.LookupID(GetInventory5()));
    GiveItem(6, EntityRegistry.LookupID(GetInventory6()));
    GiveItem(7, EntityRegistry.LookupID(GetInventory7()));
    GiveItem(8, EntityRegistry.LookupID(GetInventory8()));

    GiveItem(INVENTORY_START_SHARED_ABILITIES + 0, EntityRegistry.LookupID(GetSharedInventory0()));
    GiveItem(INVENTORY_START_SHARED_ABILITIES + 1, EntityRegistry.LookupID(GetSharedInventory1()));
    GiveItem(INVENTORY_START_SHARED_ABILITIES + 2, EntityRegistry.LookupID(GetSharedInventory2()));

    ValidateExclusiveAttackModSlot();
}


/*====================
  IUnitEntity::ClearStates
  ====================*/
void    IUnitEntity::ClearStates()
{
    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
        RemoveState(i);
}


/*====================
  IUnitEntity::ApplyState
  ====================*/
IEntityState*   IUnitEntity::ApplyState(ushort unID, uint uiLevel, uint uiStartTime, uint uiDuration, uint uiInflictorIndex, uint uiProxyUID, EStateStackType eStack, uint uiSpawnerUID)
{
    PROFILE("IUnitEntity::ApplyState");

    assert(Game.IsServer());

    int iFreeSlot(-1);

    IGameEntity *pInflictor(Game.GetEntity(uiInflictorIndex));

    // If the state already exists, just update it
    IEntityState *pPrevState(nullptr);

    for (int iSlot(INVENTORY_START_STATES); iSlot <= INVENTORY_END_STATES; ++iSlot)
    {
        IEntityState *pState(GetState(iSlot));
        if (pState == nullptr)
        {
            if (iFreeSlot == -1)
                iFreeSlot = iSlot;
            continue;
        }

        if ((eStack == STATE_STACK_NONE && pState->GetType() == unID) ||
            (eStack == STATE_STACK_NOSELF && pState->GetType() == unID && pState->GetInflictorIndex() == uiInflictorIndex))
        {
            pPrevState = pState;
            break;
        }
    }

    if (pPrevState != nullptr && pPrevState->GetNoRefresh())
        return pPrevState;

    if (pPrevState != nullptr)
    {
        if (uiDuration != INVALID_TIME && Game.IsDebuff(pPrevState->GetEffectType()))
            uiDuration *= GetDebuffDurationMultiplier();

        uint uiRemainingLifetime(pPrevState->GetRemainingLifetime());

        if (uiStartTime != INVALID_TIME && uiDuration != INVALID_TIME && (uiRemainingLifetime == INVALID_TIME || uiRemainingLifetime > uiDuration))
            return pPrevState;

        pPrevState->SetStartTime(uiStartTime);
        pPrevState->SetLifetime(uiDuration);
        pPrevState->SetInflictorIndex(uiInflictorIndex);
        pPrevState->SetProxyUID(uiProxyUID);
        pPrevState->SetSpawnerUID(uiSpawnerUID);
        pPrevState->SetLevel(uiLevel);
        pPrevState->SetExpired(false);
        pPrevState->SetExpireNextFrame(false);
        if (pInflictor != nullptr)
            pPrevState->SetPersistentModifierKeys(pInflictor->GetModifierKeys());
        pPrevState->UpdateModifiers();

        UpdateModifiers();

        IEntityDefinition *pDef(pPrevState->GetActiveDefinition<IEntityDefinition>());
        if (pDef != nullptr)
            pDef->ExecuteActionScript(ACTION_SCRIPT_REFRESH, pPrevState, pInflictor ? pInflictor->GetAsUnit() : nullptr, pPrevState, this, GetPosition(), Game.GetEntityFromUniqueID(uiProxyUID), pPrevState->GetLevel());

        UpdateInventory();

        return pPrevState;
    }

    if (iFreeSlot == -1)
    {
        Console.Err << _T("Too many states on entity #") << GetIndex() << newl;
        return nullptr;
    }

    // Apply a new state
    IGameEntity *pNew(Game.AllocateEntity(unID, GetIndex()));
    if (pNew == nullptr)
    {
        Console.Err << _T("Invalid state: ") << unID << newl;
        return nullptr;
    }

    IEntityState *pNewState(pNew->GetAsState());
    if (pNewState == nullptr)
    {
        Console.Err << QuoteStr(pNew->GetTypeName()) << _T(" is not a state") << newl;
        Game.DeleteEntity(pNew);
        return nullptr;
    }

    if (uiDuration != -1 && pNewState->GetEffectType() == Game.LookupEffectType(_T("StatusDebuff")))
        uiDuration *= GetDebuffDurationMultiplier();

    pNewState->SetStartTime(uiStartTime);
    pNewState->SetLifetime(uiDuration);
    pNewState->SetOwnerIndex(GetIndex());
    pNewState->SetInflictorIndex(uiInflictorIndex);
    pNewState->SetProxyUID(uiProxyUID);
    pNewState->SetSpawnerUID(uiSpawnerUID);
    pNewState->SetSlot(iFreeSlot);
    pNewState->SetLevel(uiLevel);
    if (pInflictor != nullptr)
        pNewState->SetPersistentModifierKeys(pInflictor->GetModifierKeys());
    pNewState->UpdateModifiers();
    
    m_apInventory[iFreeSlot] = pNewState;

    UpdateModifiers();

    m_pMorphState = GetMorphState();

    pNewState->Spawn();

    IEntityDefinition *pDef(pNewState->GetActiveDefinition<IEntityDefinition>());
    if (pDef != nullptr)
        pDef->ExecuteActionScript(ACTION_SCRIPT_INFLICT, pNewState, pInflictor ? pInflictor->GetAsUnit() : nullptr, pNewState, this, GetPosition(), Game.GetEntityFromUniqueID(uiProxyUID), pNewState->GetLevel());

    Action(ACTION_SCRIPT_INFLICTED, pInflictor ? pInflictor->GetAsUnit() : nullptr, pNewState);

    if (m_apInventory[iFreeSlot] != pNewState)
        pNewState = nullptr;
    else
        UpdateInventory();

    return pNewState;
}


/*====================
  IUnitEntity::TransferState

  Transfer an slave between one entity and another. This properly cleans
  up the pointer in the old owner entity
  ====================*/
IEntityState*   IUnitEntity::TransferState(IEntityState *pState)
{
    if (pState == nullptr || Game.IsClient())
        return nullptr;

    IUnitEntity *pOldOwner(pState->GetOwner());

    // Find a free slot
    int iSlot(-1);

    for (int iTrySlot(INVENTORY_START_STATES); iTrySlot <= INVENTORY_END_STATES; ++iTrySlot)
    {
        if (m_apInventory[iTrySlot] == nullptr)
        {
            if (iSlot == -1)
                iSlot = iTrySlot;

            continue;
        }

        IEntityState *pInvState(m_apInventory[iTrySlot]->GetAsState());
        if (pInvState == nullptr)
            continue;

        if (pInvState->GetType() == pState->GetType())
        {
            RemoveState(pInvState->GetSlot());
            
            if (iSlot == -1)
                iSlot = iTrySlot;

            continue;
        }
    }

    if (iSlot > INVENTORY_END_STATES)
        return nullptr;

    IGameEntity *pNewEntity(Game.AllocateEntity(pState->GetType(), GetIndex()));
    if (pNewEntity == nullptr || !pNewEntity->IsState())
    {
        Game.DeleteEntity(pNewEntity);
        return nullptr;
    }

    IEntityState *pNewState(pNewEntity->GetAsState());

    pNewState->SetStartTime(pState->GetStartTime());
    pNewState->SetLifetime(pState->GetLifetime());
    pNewState->SetOwnerIndex(GetIndex());
    pNewState->SetInflictorIndex(pState->GetInflictorIndex());
    pNewState->SetProxyUID(pState->GetProxyUID());
    pNewState->SetSlot(iSlot);
    pNewState->SetLevel(pState->GetLevel());
    pNewState->SetPersistentModifierKeys(pState->GetPersistentModifierKeys());
    pNewState->UpdateModifiers();

    m_apInventory[iSlot] = pNewState;

    UpdateModifiers();

    m_pMorphState = GetMorphState();

    pNewState->Spawn();

    if (pOldOwner != nullptr)
    {
        pOldOwner->RemoveState(pState->GetSlot());
        pState = nullptr;
    }
    else
    {
        Game.DeleteEntity(pState);
        pState = nullptr;
    }

    return pNewState;
}


/*====================
  IUnitEntity::RemoveState
  ====================*/
void    IUnitEntity::RemoveState(int iSlot)
{
    if (Game.IsClient())
        return;

    IEntityState *pState(GetState(iSlot));
    if (pState == nullptr)
        return;

    pState->Expired();
    Game.DeleteEntity(pState);
    m_apInventory[iSlot] = nullptr;

    m_pMorphState = GetMorphState();

    UpdateModifiers();
    UpdateInventory();
}


/*====================
  IUnitEntity::RemoveState

  Used in client-side state management
  ====================*/
void    IUnitEntity::RemoveState(IEntityState *pState)
{
    if (Game.IsServer())
        return;

    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        if (m_apInventory[i] != pState)
            continue;

        for (int i2(i); i2 <= INVENTORY_END_STATES - 1; ++i2)
            m_apInventory[i2] = m_apInventory[i2 + 1];

        m_apInventory[INVENTORY_END_STATES] = nullptr;

        m_pMorphState = GetMorphState();
    }
}


/*====================
  IUnitEntity::ExpireState
  ====================*/
void    IUnitEntity::ExpireState(int iSlot)
{
    IEntityState *pState(GetState(iSlot));
    if (pState == nullptr)
        return;

    pState->SetExpired(true);
}


/*====================
  IUnitEntity::ExpireState
  ====================*/
void    IUnitEntity::ExpireState(ushort unID)
{
    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        if (m_apInventory[i] == nullptr ||
            m_apInventory[i]->GetType() != unID ||
            !m_apInventory[i]->IsState())
            continue;

        m_apInventory[i]->GetAsState()->SetExpired(true);
    }
}


/*====================
  IUnitEntity::HasState
  ====================*/
bool    IUnitEntity::HasState(ushort unID)
{
    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        if (m_apInventory[i] == nullptr ||
            m_apInventory[i]->GetType() != unID ||
            !m_apInventory[i]->IsState())
            continue;

        return true;
    }

    return false;
}


/*====================
  IUnitEntity::GetMorphState
  ====================*/
IEntityState*   IUnitEntity::GetMorphState() const
{
    IEntityState *pReturnState(nullptr);
    uint uiHighPriority(0);
    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        if (m_apInventory[i] == nullptr)
            continue;
        IEntityState *pState(m_apInventory[i]->GetAsState());
        if (pState == nullptr)
            continue;
        if (pState->GetMorphPriority() > uiHighPriority)
        {
            uiHighPriority = pState->GetMorphPriority();
            pReturnState = pState;
        }
    }

    return pReturnState;
}


/*====================
  IUnitEntity::AttachGadget
  ====================*/
IGadgetEntity*  IUnitEntity::AttachGadget(ushort unID, uint uiLevel, uint uiStartTime, uint uiDuration, IUnitEntity *pOwner)
{
    PROFILE("IUnitEntity::AttachGadget");

    assert(Game.IsServer());

    if (unID == INVALID_ENT_TYPE || pOwner == nullptr)
        return nullptr;

    // Search bound entities for an instance of this type of gadget
    for (vector<SEntityBind>::iterator it(m_vBinds.begin()); it != m_vBinds.end(); ++it)
    {
        IGadgetEntity *pGadget(Game.GetGadgetEntity(Game.GetGameIndexFromUniqueID(it->uiEntityUID)));
        if (pGadget == nullptr)
            continue;

        if (pGadget->GetType() == unID)
            return pGadget;
    }

    // Allocate a new gadget and bind it
    IGadgetEntity *pGadget(Game.AllocateDynamicEntity<IGadgetEntity>(unID));
    if (pGadget == nullptr)
        return nullptr;

    CPlayer *pPlayer(Game.GetPlayerFromClientNumber(pOwner->GetOwnerClientNumber()));
    if (pPlayer != nullptr)
        pPlayer->AddPet(pGadget, 0, INVALID_INDEX);

    pGadget->SetPosition(GetPosition());
    pGadget->SetLevel(uiLevel);
    pGadget->SetTeam(pOwner->GetTeam());
    pGadget->SetOwnerIndex(pOwner->GetIndex());
    //if (GetInheritModifiers())
    //  pGadget->SetPersistentModifierKeys(pSource->GetModifierKeys());
    pGadget->UpdateModifiers();
    pGadget->Spawn();
    pGadget->ValidatePosition(TRACE_UNIT_SPAWN);

    Bind(pGadget, V3_ZERO, 0);

    return pGadget;
}


/*====================
  IUnitEntity::AddState

  Used in client-side state management
  ====================*/
void    IUnitEntity::AddState(IEntityState *pState)
{
    if (Game.IsServer())
        return;

    for (int iSlot(INVENTORY_START_STATES); iSlot <= INVENTORY_END_STATES; ++iSlot)
    {
        if (m_apInventory[iSlot] == pState)
            return;
    }

    for (int i(INVENTORY_START_STATES); i < INVENTORY_END_STATES; ++i)
    {
        if (m_apInventory[i] != nullptr)
            continue;

        m_apInventory[i] = pState;
        m_pMorphState = GetMorphState();
        return;
    }
}


/*====================
  IUnitEntity::GetStateExpireTime
  ====================*/
uint    IUnitEntity::GetStateExpireTime(int iSlot)
{
    IEntityState *pState(GetState(iSlot));
    if (pState == nullptr)
        return INVALID_TIME;

    return pState->GetExpireTime();
}


/*====================
  IUnitEntity::GetStateExpirePercent
  ====================*/
float   IUnitEntity::GetStateExpirePercent(int iSlot)
{
    IEntityState *pState(GetState(iSlot));
    if (pState == nullptr)
        return 0.0f;

    if (pState->GetStartTime() == INVALID_TIME || pState->GetLifetime() == INVALID_TIME)
        return 0.0f;

    return float(Game.GetGameTime() - pState->GetStartTime()) / float(pState->GetLifetime());
}


/*====================
  IUnitEntity::GetNextExclusiveAttackModSlot
  ====================*/
int     IUnitEntity::GetNextExclusiveAttackModSlot() const
{
    static uint uiAttackModID(EntityRegistry.RegisterModifier(_T("attack")));

    int iStartSlot(m_iExclusiveAttackModSlot);
    if (iStartSlot < INVENTORY_START_ACTIVE)
        iStartSlot = INVENTORY_START_ACTIVE;
    if (iStartSlot > INVENTORY_END_ACTIVE)
        iStartSlot = INVENTORY_END_ACTIVE;

    int iSlot(iStartSlot + 1);

    if (iSlot > INVENTORY_END_ACTIVE)
        iSlot = INVENTORY_START_ACTIVE;

    while (iSlot != iStartSlot)
    {
        ISlaveEntity *pSlave(GetSlave(iSlot));
        if (pSlave == nullptr || !pSlave->IsActive() || pSlave->GetModifierBit(uiAttackModID) == 0)
        {
            ++iSlot;

            if (iSlot > INVENTORY_END_ACTIVE)
                iSlot = INVENTORY_START_ACTIVE;

            continue;
        }

        break;
    }

    return iSlot;
}


/*====================
  IUnitEntity::GetPrevExclusiveAttackModSlot
  ====================*/
int     IUnitEntity::GetPrevExclusiveAttackModSlot() const
{
    static uint uiAttackModID(EntityRegistry.RegisterModifier(_T("attack")));

    int iStartSlot(m_iExclusiveAttackModSlot);
    if (iStartSlot < INVENTORY_START_ACTIVE)
        iStartSlot = INVENTORY_START_ACTIVE;
    if (iStartSlot > INVENTORY_END_ACTIVE)
        iStartSlot = INVENTORY_END_ACTIVE;

    int iSlot(iStartSlot - 1);

    if (iSlot < INVENTORY_START_ACTIVE)
        iSlot = INVENTORY_END_ACTIVE;

    while (iSlot != iStartSlot)
    {
        ISlaveEntity *pSlave(GetSlave(iSlot));
        if (pSlave == nullptr || !pSlave->IsActive() || pSlave->GetModifierBit(uiAttackModID) == 0)
        {
            --iSlot;

            if (iSlot < INVENTORY_START_ACTIVE)
                iSlot = INVENTORY_END_ACTIVE;

            continue;
        }

        break;
    }

    return iSlot;
}


/*====================
  IUnitEntity::ValidateExclusiveAttackModSlot
  ====================*/
void    IUnitEntity::ValidateExclusiveAttackModSlot()
{
    static uint uiAttackModID(EntityRegistry.RegisterModifier(_T("attack")));

    ISlaveEntity *pSlave(GetSlave(GetExclusiveAttackModSlot()));
    if (pSlave == nullptr || !pSlave->IsActive() || pSlave->GetModifierBit(uiAttackModID) == 0)
        SetExclusiveAttackModSlot(GetNextExclusiveAttackModSlot());
}


/*====================
  IUnitEntity::GiveItem
  ====================*/
IEntityTool*    IUnitEntity::GiveItem(int iSlot, ushort unID, bool bEnabled)
{
    if (Game.IsClient())
        return nullptr;

    try
    {
        if (iSlot < 0 || iSlot >= MAX_INVENTORY)
            EX_ERROR(_T("Invalid inventory slot: ") + XtoA(iSlot));

        if (unID == INVALID_ENT_TYPE)
        {
            RemoveItem(iSlot);
            return nullptr;
        }

        IEntityTool *pTool(GetTool(iSlot));
        if (pTool != nullptr && pTool->GetType() == unID)
        {
            IEntityItem *pItem(pTool->GetAsItem());
            if (pItem != nullptr && pItem->CanStack(unID, pItem->GetPurchaserClientNumber()))
            {
                pItem->AddCharges(pItem->GetInitialCharges());
            }

            return pTool;
        }

        RemoveItem(iSlot);

        IEntityTool *pNewTool(static_cast<IEntityTool*>(Game.AllocateEntity(unID, m_uiIndex)));
        if (pNewTool == nullptr)
            EX_ERROR(_T("Item allocation failed for: ") + EntityRegistry.LookupName(unID));

        m_apInventory[iSlot] = pNewTool;
        m_apInventory[iSlot]->SetOwnerIndex(m_uiIndex);
        m_apInventory[iSlot]->SetSlot(iSlot);

        if (bEnabled && m_apInventory[iSlot] != nullptr && m_apInventory[iSlot]->IsItem())
            m_apInventory[iSlot]->GetAsItem()->SetFlag(ENTITY_TOOL_FLAG_ASSEMBLED | ENTITY_TOOL_FLAG_ACTIVE);

        m_apInventory[iSlot]->SetActiveModifierKey(m_apInventory[iSlot]->GetDefaultActiveModifierKey());
        m_apInventory[iSlot]->Spawn();

        if (bEnabled && m_apInventory[iSlot] != nullptr && m_apInventory[iSlot]->IsItem())
        {
            IEntityItem *pItem(m_apInventory[iSlot]->GetAsItem());

            pItem->SetCharges(pItem->GetInitialCharges());

            if (pItem->GetMaxLevel() > 0)
                pItem->SetLevel(1);

            pItem->SetActiveModifierKey(pItem->GetDefaultActiveModifierKey());

            pItem->ExecuteActionScript(ACTION_SCRIPT_CREATE, this, GetPosition());
        }

        pNewTool->UpdateApparentCooldown();

        ValidateExclusiveAttackModSlot();

        if (bEnabled && m_apInventory[iSlot] != nullptr && m_apInventory[iSlot]->IsItem())
            m_apInventory[iSlot]->GetAsItem()->ExecuteActionScript(ACTION_SCRIPT_CREATE, this, GetPosition());

        return pNewTool;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IUnitEntity::GiveItem() - "), NO_THROW);
        return nullptr;
    }
}


/*====================
  IUnitEntity::TransferItem

  Transfer an item between one entity and another. This properly cleans
  up the pointer in the old owner entity
  ====================*/
int     IUnitEntity::TransferItem(int iClientNum, IEntityItem *pItem, int iSlot)
{
    if (pItem == nullptr || Game.IsClient())
        return -1;

    IUnitEntity *pOldOwner(pItem->GetOwner());

    if (pOldOwner != nullptr &&
        pOldOwner->GetOwnerClientNumber() != -1 &&
        pOldOwner->GetOwnerClientNumber() != iClientNum &&
        !pItem->BelongsToClient(iClientNum) &&
        !pItem->GetAllowTransfer())
        return -1;

    if (!CanCarryItem(pItem))
        return -1;

    if (iSlot != -1 && GetTool(iSlot) != nullptr)
        return -1;

    // Find a free slot
    if (iSlot == -1)
    {
        for (int iTrySlot(INVENTORY_START_BACKPACK); iTrySlot <= INVENTORY_BACKPACK_PROVISIONAL; ++iTrySlot)
        {
            if (m_apInventory[iTrySlot] == nullptr)
            {
                if (iSlot == -1)
                    iSlot = iTrySlot;

                continue;
            }

            IEntityItem *pInvItem(m_apInventory[iTrySlot]->GetAsItem());
            if (pInvItem == nullptr)
                continue;

            if (pInvItem->CanStack(pItem))
            {
                pInvItem->UpdatePurchaseTime(pItem->GetPurchaseTime());
                pInvItem->SetCharges(pInvItem->GetCharges() + pItem->GetCharges());
                if (pItem->GetOwner() != nullptr)
                    pItem->GetOwner()->RemoveItem(pItem->GetSlot());
                return iSlot;
            }
        }

        if (iSlot > INVENTORY_BACKPACK_PROVISIONAL)
            return -1;
    }

    IGameEntity *pNewEntity(Game.AllocateEntity(pItem->GetType(), GetIndex()));
    if (pNewEntity == nullptr || !pNewEntity->IsItem())
    {
        Game.DeleteEntity(pNewEntity);
        return -1;
    }

    IEntityItem *pNewItem(pNewEntity->GetAsItem());

    pNewItem->SetAllFlags(pItem->GetAllFlags());
    pNewItem->SetPurchaseTime(pItem->GetPurchaseTime());
    pNewItem->SetActiveModifierKey(pItem->GetActiveModifierKey());
    pNewItem->Spawn();
    pNewItem->SetCharges(pItem->GetCharges());

    uint uiCooldownTime(pItem->GetActualRemainingCooldownTime());
    uint uiDuration(pItem->GetCooldownDuration());
    if (uiCooldownTime != 0 && uiCooldownTime != INVALID_TIME &&
        uiDuration != 0 && uiDuration != INVALID_TIME)
    {
        // Adjust cooldown for global cooldown modifier differences
        float fOldCooldownSpeed(pOldOwner->GetCooldownSpeed());
        float fOldCooldownReduction(MIN(pOldOwner->GetReducedCooldowns() - pOldOwner->GetIncreasedCooldowns(), 1.0f));

        float fNewCooldownSpeed(GetCooldownSpeed());
        float fNewCooldownReduction(MIN(GetReducedCooldowns() - GetIncreasedCooldowns(), 1.0f));

        float fFactor(fOldCooldownSpeed / fNewCooldownSpeed * (1.0f - fNewCooldownReduction) / (1.0f - fOldCooldownReduction));

        float fPercent(pItem->GetActualRemainingCooldownPercent());

        uint uiNewDuration(INT_CEIL(uiDuration * fFactor));
        uint uiNewStartTime(Game.GetGameTime() - INT_CEIL(uiNewDuration * (1.0f - fPercent)));

        pNewItem->SetCooldownStartTime(uiNewStartTime);
        pNewItem->SetCooldownDuration(uiNewDuration);
    }
    else
    {
        pNewItem->SetCooldownStartTime(pItem->GetCooldownStartTime());
        pNewItem->SetCooldownDuration(pItem->GetCooldownDuration());
    }

    pNewItem->SetLevel(pItem->GetLevel());
    pNewItem->SetPurchaserClientNumber(pItem->GetPurchaserClientNumber());
    pNewItem->SetTimer(pItem->GetTimer());

    m_apInventory[iSlot] = pNewItem;
    pNewItem->SetOwnerIndex(GetIndex());
    pNewItem->SetSlot(iSlot);

    int iNewSlot(CheckRecipes(iSlot));
    if (iNewSlot != -1)
    {
        if (iNewSlot == INVENTORY_BACKPACK_PROVISIONAL || (iSlot == INVENTORY_BACKPACK_PROVISIONAL && iNewSlot != iSlot))
            SwapItem(iClientNum, iNewSlot, iSlot);
        else
            iSlot = iNewSlot;
    }

    IEntityItem *pProvisionalItem(GetItem(INVENTORY_BACKPACK_PROVISIONAL));
    if (pProvisionalItem != nullptr)
    {
        RemoveItem(INVENTORY_BACKPACK_PROVISIONAL);
        return -1;
    }

    if (pItem->GetOwner() != nullptr)
        pItem->GetOwner()->RemoveItem(pItem->GetSlot());

    ValidateExclusiveAttackModSlot();

    pNewItem->UpdateApparentCooldown();

    if (pNewItem->GetBindOnPickup() && pNewItem->GetPurchaserClientNumber() == -1)
        pNewItem->SetPurchaserClientNumber(GetOwnerClientNumber());

    pNewItem->ExecuteActionScript(ACTION_SCRIPT_PICKUP, this, GetPosition());

    return iSlot;
}


/*====================
  IUnitEntity::CloneItem
  ====================*/
bool    IUnitEntity::CloneItem(IEntityItem *pItem)
{
    if (Game.IsClient())
        return false;

    int iSlot(pItem->GetSlot());

    if (m_apInventory[iSlot] != nullptr)
        return false;

    IGameEntity *pNewEntity(Game.AllocateEntity(pItem->GetType(), m_uiIndex));
    if (pNewEntity == nullptr || !pNewEntity->IsItem())
    {
        Game.DeleteEntity(pNewEntity);
        return false;
    }

    IEntityItem *pNewItem(pNewEntity->GetAsItem());
    m_apInventory[iSlot] = pNewItem;

    pNewItem->SetOwnerIndex(m_uiIndex);
    pNewItem->SetSlot(iSlot);
    pNewItem->SetAllFlags(pItem->GetAllFlags());
    pNewItem->SetLevel(pItem->GetLevel());
    pNewItem->SetPurchaserClientNumber(pItem->GetPurchaserClientNumber());
    pNewItem->Spawn();
    pNewItem->SetCharges(pItem->GetCharges());
    pNewItem->SetCooldownStartTime(pItem->GetCooldownStartTime());
    pNewItem->SetCooldownDuration(pItem->GetCooldownDuration());
    pNewItem->SetActiveModifierKey(pItem->GetActiveModifierKey());
    pNewItem->UpdateApparentCooldown();

    return true;
}


/*====================
  IUnitEntity::ClearInventory
  ====================*/
void    IUnitEntity::ClearInventory()
{
    for (int i(0); i < MAX_INVENTORY; ++i)
        RemoveItem(i);

    m_pMorphState = nullptr;
}


/*====================
  IUnitEntity::RemoveItem
  ====================*/
void    IUnitEntity::RemoveItem(int iSlot)
{
    if (iSlot < 0 || iSlot >= MAX_INVENTORY)
        return;

    if (m_apInventory[iSlot] == nullptr)
        return;

    Game.DeleteEntity(m_apInventory[iSlot]);
    m_apInventory[iSlot] = nullptr;

    ValidateExclusiveAttackModSlot();

    if (iSlot >= INVENTORY_START_STATES && iSlot <= INVENTORY_END_STATES)
        m_pMorphState = GetMorphState();
}


/*====================
  IUnitEntity::RemoveItemByIndex
  ====================*/
void    IUnitEntity::RemoveItemByIndex(uint uiIndex)
{
    for (int i(0); i < MAX_INVENTORY; i++)
    {
        if (m_apInventory[i] == nullptr)
            continue;

        if (m_apInventory[i]->GetIndex() != uiIndex)
            continue;

        Game.DeleteEntity(m_apInventory[i]);
        m_apInventory[i] = nullptr;

        ValidateExclusiveAttackModSlot();

        if (i >= INVENTORY_START_STATES && i <= INVENTORY_END_STATES)
            m_pMorphState = GetMorphState();

        break;
    }
}


/*====================
  IUnitEntity::CanGiveItem
  ====================*/
bool    IUnitEntity::CanGiveItem(IEntityItem *pItem, IUnitEntity *pTarget)
{
    // Sanity check inputs
    assert(pItem != nullptr && pTarget != nullptr);
    if (pItem == nullptr || pTarget == nullptr)
        return false;

    // Can't give items to ourselves
    if (pTarget == this)
        return false;

    // Can't transfer items from / to illusions
    if (IsIllusion() || pTarget->IsIllusion())
        return false;

    // Can't transfer items from / to units whose backpack is locked
    if (HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK) || pTarget->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
        return false;

    // Chests can always receive items
    if (GetStatus() == ENTITY_STATUS_ACTIVE && pTarget->IsChest())
        return true;

    // Can't transfer items from / to inactive (dead?) units
    if (GetStatus() != ENTITY_STATUS_ACTIVE || pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    // Can't transfer items to a target that can't carry items
    if (!pTarget->GetCanCarryItems())
        return false;

    // If the target can't carry the item, fail
    if (!Game.IsValidTarget(pItem->GetCarryScheme(), 0, pTarget, pTarget, true))
        return false;

    return true;
}


/*====================
  IUnitEntity::CanCarryItem
  ====================*/
bool    IUnitEntity::CanCarryItem(IEntityItem *pItem)
{
    // Sanity check inputs
    assert(pItem != nullptr);
    if (pItem == nullptr)
        return false;

    // Chests can always carry items
    if (IsChest() && !HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
        return true;

    // Can we carry items?
    if (!GetCanCarryItems())
        return false;

    // Allow heroes to carry items when they're dead, otherwise they'll drop all items when dead
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return true;

    // In item drop mode, do not allow couriers to pick up items that have been dropped on death
    if (Game.HasGameOptions(GAME_OPTION_DROP_ITEMS))
    {
        if (m_uiAllUnitsExceptCouriersScheme == INVALID_TARGET_SCHEME)
        {
            m_uiAllUnitsExceptCouriersScheme = Game.LookupTargetScheme(_T("all_units_except_couriers"));
            assert(m_uiAllUnitsExceptCouriersScheme != INVALID_TARGET_SCHEME);
        }

        if (pItem->BelongsToEveryone())
        {
            if (!Game.IsValidTarget(m_uiAllUnitsExceptCouriersScheme, 0, this, this, true))
                return false;
        }
    }

    // If we can't carry the item, fail
    if (!Game.IsValidTarget(pItem->GetCarryScheme(), 0, this, this, true))
        return false;

    return true;
}


/*====================
  IUnitEntity::CanSellItem
  ====================*/
bool    IUnitEntity::CanSellItem(IEntityItem *pItem, int iClientNum)
{
    // Sanity check inputs
    assert(pItem != nullptr);
    if (pItem == nullptr)
        return false;

    // If our backpack is locked, then we can't sell any items
    if (HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
        return false;

    // If we're an illusion, then we can't sell any items
    if (IsIllusion())
        return false;

    // Ensure that the client ordering us to sell an item is a client who owns us
    if (!CanReceiveOrdersFrom(iClientNum))
        return false;

    // In item drop mode, items which have been dropped on death cannot be sold
    if (Game.HasGameOptions(GAME_OPTION_DROP_ITEMS) && pItem->BelongsToEveryone())
        return false;
    
    // Ensure that the item we're about to sell actually belongs to us
    if (!pItem->BelongsToClient(iClientNum))
        return false;

    // Allow recently purchased items to be sold, even if the item is flagged as nosell
    // (e.g. Logger's Hatchet)
    if (pItem->GetNoSell() && !pItem->WasPurchasedRecently())
        return false;

    return true;
}


/*====================
  IUnitEntity::SwapItem
  ====================*/
void    IUnitEntity::SwapItem(int iClientNum, int iSlot1, int iSlot2)
{
    // Validate slots
    if (iSlot1 == iSlot2)
        return;

    if (!IS_ITEM_SLOT(iSlot1) || !IS_ITEM_SLOT(iSlot2))
        return;

    // Check for combining stacks
    IEntityItem *pItem1(GetItem(iSlot1));
    IEntityItem *pItem2(GetItem(iSlot2));

    // A client with shared control can't move items that don't belong to them
    if (GetOwnerClientNumber() != iClientNum)
    {
        if (pItem1 != nullptr && pItem1->GetPurchaserClientNumber() != iClientNum)
            return;
        if (pItem2 != nullptr && pItem2->GetPurchaserClientNumber() != iClientNum)
            return;
    }

    // Don't allow items flagged as nostash to be transferred to the stash
    if ((pItem1 != nullptr && pItem1->GetNoStash() && IS_STASH_SLOT(iSlot2)) ||
        (pItem2 != nullptr && pItem2->GetNoStash() && IS_STASH_SLOT(iSlot1)))
    {
        return;
    }

    if (pItem1 != nullptr && pItem2 != nullptr)
    {
        if (pItem2->CanStack(pItem1))
        {
            pItem2->UpdatePurchaseTime(pItem1->GetPurchaseTime());
            pItem2->SetCharges(pItem2->GetCharges() + pItem1->GetCharges());
            if (m_iExclusiveAttackModSlot == pItem1->GetSlot())
                m_iExclusiveAttackModSlot = pItem2->GetSlot();
            RemoveItem(pItem1->GetSlot());
            return;
        }
        else if (pItem1->CanStack(pItem2))
        {
            pItem1->UpdatePurchaseTime(pItem2->GetPurchaseTime());
            pItem1->SetCharges(pItem1->GetCharges() + pItem2->GetCharges());
            if (m_iExclusiveAttackModSlot == pItem2->GetSlot())
                m_iExclusiveAttackModSlot = pItem1->GetSlot();
            RemoveItem(pItem2->GetSlot());
            return;
        }
    }

    SWAP(m_apInventory[iSlot1], m_apInventory[iSlot2]);

    if (m_apInventory[iSlot1])
        m_apInventory[iSlot1]->SetSlot(iSlot1);
    if (m_apInventory[iSlot2])
        m_apInventory[iSlot2]->SetSlot(iSlot2);

    // Check recipes
    if (IS_BACKPACK_SLOT(iSlot1) != IS_BACKPACK_SLOT(iSlot2))
    {
        if (m_apInventory[iSlot1] != nullptr)
            CheckRecipes(iSlot1);

        if (m_apInventory[iSlot2] != nullptr)
            CheckRecipes(iSlot2);
    }

    // If anything is left in the provisional slot, undo the move
    if (IS_PROVISIONAL_SLOT(iSlot1) && m_apInventory[iSlot1] != nullptr ||
        IS_PROVISIONAL_SLOT(iSlot2) && m_apInventory[iSlot2] != nullptr)
    {
        SWAP(m_apInventory[iSlot1], m_apInventory[iSlot2]);
        if (m_apInventory[iSlot1])
            m_apInventory[iSlot1]->SetSlot(iSlot1);
        if (m_apInventory[iSlot2])
            m_apInventory[iSlot2]->SetSlot(iSlot2);

        return;
    }

    if (IS_BACKPACK_SLOT(iSlot1) != IS_BACKPACK_SLOT(iSlot2))
    {
        if (m_apInventory[iSlot1] != nullptr && IS_BACKPACK_SLOT(iSlot1) && m_apInventory[iSlot1]->IsTool())
            m_apInventory[iSlot1]->GetAsTool()->UpdateApparentCooldown();
        if (m_apInventory[iSlot2] != nullptr && IS_BACKPACK_SLOT(iSlot2) && m_apInventory[iSlot2]->IsTool())
            m_apInventory[iSlot2]->GetAsTool()->UpdateApparentCooldown();
    }

    // Update exclusive attack modifier slot
    if (m_iExclusiveAttackModSlot == iSlot1)
        m_iExclusiveAttackModSlot = iSlot2;
    else if (m_iExclusiveAttackModSlot == iSlot2)
        m_iExclusiveAttackModSlot = iSlot1;
}


/*====================
  IUnitEntity::Disassemble
  ====================*/
void    IUnitEntity::DisassembleItem(int iSlot)
{
    IEntityItem *pItem(GetItem(iSlot));
    if (pItem == nullptr)
        return;

    if (!pItem->GetAllowDisassemble() || pItem->IsBorrowed())
        return;

    int iStartSlot(IS_STASH_SLOT(pItem->GetSlot()) ? INVENTORY_START_STASH : INVENTORY_START_BACKPACK);
    int iEndSlot(IS_STASH_SLOT(pItem->GetSlot()) ? INVENTORY_END_STASH : INVENTORY_END_BACKPACK);

    uivector vFreeSlots;
    for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr || m_apInventory[iSlot] == pItem)
            vFreeSlots.push_back(iSlot);
    }

    CItemDefinition *pDefinition(pItem->GetDefinition<CItemDefinition>());
    if (pDefinition == nullptr)
        return;

    const tsvector &vComponents(pDefinition->GetComponents(pItem->GetRecipeVariation()));
    if (vFreeSlots.size() < vComponents.size())
        return;

    int iPurchaserClientNumber(pItem->GetPurchaserClientNumber());

    RemoveItem(pItem->GetSlot());

    for (tsvector_cit itComponent(vComponents.begin()); itComponent != vComponents.end(); ++itComponent)
    {
        IEntityTool *pTool(GiveItem(vFreeSlots[itComponent - vComponents.begin()], EntityRegistry.LookupID(*itComponent), true));
        if (pTool != nullptr && pTool->IsItem())
        {
            IEntityItem *pItem(pTool->GetAsItem());
            pItem->SetPurchaserClientNumber(iPurchaserClientNumber);
        }
    }
}


/*====================
  IUnitEntity::SetTargetIndex
  ====================*/
void    IUnitEntity::SetTargetIndex(uint uiIndex)
{
    if (uiIndex != m_uiTargetIndex)
        Action(ACTION_SCRIPT_TARGET_ACQUIRED, Game.GetUnitEntity(uiIndex), nullptr);

    m_uiTargetIndex = uiIndex;
}


/*====================
  IUnitEntity::AllocateSkeleton
  ====================*/
CSkeleton*  IUnitEntity::AllocateSkeleton()
{
    return m_pSkeleton = K2_NEW(ctx_Game,   CSkeleton);
}


/*====================
  IUnitEntity::ApplyWorldEntity
  ====================*/
void    IUnitEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
    m_sName = ent.GetName();
    m_uiWorldIndex = ent.GetIndex();
    m_v3Position = ent.GetPosition();
    m_v3AttentionAngles = m_v3UnitAngles = m_v3Angles = ent.GetAngles();
    SetTeam(ent.GetTeam());
    SetLevel(ent.GetPropertyInt(_T("level")));
}


/*====================
  IUnitEntity::Spawn
  ====================*/
void    IUnitEntity::Spawn()
{
    PROFILE("IUnitEntity::Spawn");

    IVisualEntity::Spawn();
    
    SetStatus(ENTITY_STATUS_ACTIVE);

    // determine entity type.
    m_bIsKongor = IsTargetType(_T("Kongor"), this);
    m_bIsTower = IsTargetType(_T("Tower"), this);

    m_bbBounds.SetCylinder(GetBoundsRadius(), GetBoundsHeight());
    m_fScale = 1.0f;

    // Fill inventory
    UpdateInventory();

    m_fHealth = m_fCurrentMaxHealth = GetMaxHealth();
    m_fMana = m_fCurrentMaxMana = GetMaxMana();
    m_fCurrentCooldownSpeed = GetCooldownSpeed();
    m_fCurrentCooldownReduction = MIN(GetReducedCooldowns(), 1.0f);
    m_fHealthAccumulator = 0.0f;
    m_fLethalDamageAccumulator = 0.0f;
    m_fNonLethalDamageAccumulator = 0.0f;
    m_uiDeathFlags = 0;
    m_uiDisjointSequence = 0;
    m_uiOrderDisjointSequence = 0;
    m_uiArmingSequence = 0;
    m_uiLastHeroAttackTime = INVALID_TIME;
    m_uiLastAttackTargetUID = INVALID_INDEX;
    m_uiLastAttackTargetTime = INVALID_TIME;

    if (m_uiWorldIndex == INVALID_INDEX)
        m_uiWorldIndex = Game.AllocateNewWorldEntity();

    StartAnimation(GetIdleAnim(), -1);
    
    if (GetAnimIndex(_T("spawn_1")) != -1)
        SetAnim(0, _T("spawn_1"));
    else
        SetAnim(0, GetIdleAnim(), 1.0f, 0);
        
    m_uiCombatType = Game.LookupCombatType(GetCombatType());

    if (Game.IsServer())
    {
        m_uiSpawnTime = INVALID_TIME;

        m_cBrain.SetUnit(this);
        m_cBrain.Init();

        if (GetSpawnEffect() != INVALID_RESOURCE)
        {
            CGameEvent ev;
            ev.SetSourceEntity(GetIndex());
            ev.SetEffect(GetSpawnEffect());
            Game.AddEvent(ev);
        }

        // Spawn action
        IUnitDefinition *pDefinition(GetDefinition<IUnitDefinition>());
        if (pDefinition != nullptr)
            pDefinition->ExecuteActionScript(ACTION_SCRIPT_SPAWN, this, (GetOwner() ? GetOwner() : this), this, this, GetPosition(), GetProxy(0), GetLevel());

        m_v2AnchorPosition = GetPosition().xy();
    }

    if (GetFlying())
        m_v3Position.z = Game.GetCameraHeight(m_v3Position.x, m_v3Position.y) + GetFlyHeight();
    else
        m_v3Position.z = Game.GetTerrainHeight(m_v3Position.x, m_v3Position.y);

    Link();

    m_fTiltPitch = 0.0f;
    m_fTiltRoll = 0.0f;

    m_v3AttentionAngles = m_v3UnitAngles = m_v3Angles;

    Game.UpdateUnitVisibility(this);
}


/*====================
  IUnitEntity::KillReward
  ====================*/
void    IUnitEntity::KillReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller)
{
    // Determine the killing team
    uint uiKillerTeam(TEAM_PASSIVE);
    if (pPlayerKiller != nullptr)
        uiKillerTeam = pPlayerKiller->GetTeam();
    else if (pKiller != nullptr)
        uiKillerTeam = pKiller->GetTeam();

    // Test for a deny
    bool bDeny(uiKillerTeam == GetTeam());

    // Build list of heroes that should receive experience
    static vector<IHeroEntity*> vHeroes;
    vHeroes.clear();

    if (GetGlobalExperience())
    {
        IGameEntity *pEntity(Game.GetFirstEntity());
        while (pEntity != nullptr)
        {
            IUnitEntity *pUnit(pEntity->GetAsUnit());
            pEntity = Game.GetNextEntity(pEntity);
            if (pUnit == nullptr)
                continue;

            if (pUnit == nullptr || pUnit->GetTeam() == GetTeam() || (!bDeny && pUnit->GetTeam() != uiKillerTeam) || pUnit->IsIllusion())
                continue;

            if (pUnit->IsHero() && (pUnit->GetStatus() == ENTITY_STATUS_ACTIVE || GetDeadExperience()))
            {
                vHeroes.push_back(pUnit->GetAsHero());
            }
            else if (pUnit->GetRelayExperience())
            {
                IUnitEntity *pOwner(pUnit->GetOwner());
                if (pOwner != nullptr && pOwner->IsHero())
                    vHeroes.push_back(pOwner->GetAsHero());
            }
        }
    }
    else
    {
        static uivector vEntities;
        vEntities.clear();
        Game.GetEntitiesInRadius(vEntities, GetPosition().xy(), Game.GetExperienceRange(), REGION_UNIT);

        uivector_it itEnd(vEntities.end());
        for (uivector_it it(vEntities.begin()); it != itEnd; ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == nullptr || pUnit->GetTeam() == GetTeam() || (!bDeny && pUnit->GetTeam() != uiKillerTeam) || pUnit->IsIllusion())
                continue;

            if (pUnit->IsHero() && (pUnit->GetStatus() == ENTITY_STATUS_ACTIVE || GetDeadExperience()))
            {
                vHeroes.push_back(pUnit->GetAsHero());
            }
            else if (pUnit->GetRelayExperience())
            {
                IUnitEntity *pOwner(pUnit->GetOwner());
                if (pOwner != nullptr && pOwner->IsHero())
                    vHeroes.push_back(pOwner->GetAsHero());
            }
        }
    }

    // Give awards to nearby units
    float fTotalExperience(GetExperienceBounty() * m_fExperienceBountyMultiplier);
    float fTotalExpAwarded(0.0f);
    float fExperiencePerPlayer((fTotalExperience / vHeroes.size()) + GetUnsharedExperienceBounty());
    ushort unGoldRadiusBounty(GetGoldBountyRadiusAmount());
    bool bHardcore(Game.HasGameOptions(GAME_OPTION_HARDCORE));
    bool bCasual(Game.HasGameOptions(GAME_OPTION_CASUAL));
    for (vector<IHeroEntity*>::iterator it(vHeroes.begin()); it != vHeroes.end(); ++it)
    {
        IHeroEntity *pHero(*it);

        float fExperience(fExperiencePerPlayer);
        if (bDeny)
        {
            const CAttackType *pAttackType(Game.GetAttackType(pHero->GetAttackType()));
            if (pAttackType != nullptr && !bCasual)
                fExperience *= pAttackType->GetDeniedExpMultiplier();

            // in hardcore mode, a deny results in no experience for the opposing team.
            if (bHardcore)
                fExperience = 0.0f;

            Game.LogExperience(GAME_LOG_EXP_DENIED, pHero, pKiller, fExperiencePerPlayer - fExperience);
        }
        fTotalExpAwarded += fExperience;
        pHero->GiveExperience(fExperience, this);

        if (pKiller != pHero)
        {
            CPlayer *pOwner(pHero->GetOwnerPlayer());
            if (pOwner != nullptr)
            {
                pOwner->GiveGold(unGoldRadiusBounty, pHero);
                if (IsCreep() || IsNeutral() || IsGadget() || IsPet())
                    pOwner->GetGoldReport()->AddCreepGoldEarned(unGoldRadiusBounty);
                else if (IsHero())
                    pOwner->GetGoldReport()->AddPlayerAssistGoldEarned(unGoldRadiusBounty);
                else if (IsBuilding())
                    pOwner->GetGoldReport()->AddBuildingGoldEarned(unGoldRadiusBounty);

                Game.LogGold(GAME_LOG_GOLD_EARNED, pOwner, this, unGoldRadiusBounty);
            }
        }
    }

    // Lookup gold bounty amounts
    ushort unBounty(INT_CEIL(GetGoldBounty() * m_fGoldBountyMultiplier));
        
    // Process a deny
    if (bDeny)
    {
        Game.LogDeny(this, pKiller, nullptr, fTotalExperience - fTotalExpAwarded, unBounty);

        if (pPlayerKiller != nullptr)
        {
            // Send reward event
            Game.SendPopup(POPUP_DENY, this, pKiller);

            if (IsCreep())
                pPlayerKiller->AdjustStat(PLAYER_STAT_DENIES, 1);
        }
    }
    else if (pPlayerKiller != nullptr)
    {
        pPlayerKiller->GiveGold(unBounty, this, pKiller);

        if (IsCreep() || IsNeutral() || IsGadget() || IsPet())
            pPlayerKiller->GetGoldReport()->AddCreepGoldEarned(unBounty);
        else if(IsHero())
            pPlayerKiller->GetGoldReport()->AddPlayerGoldEarned(unBounty);
        else if (IsBuilding())
            pPlayerKiller->GetGoldReport()->AddBuildingGoldEarned(unBounty);

        Game.LogGold(GAME_LOG_GOLD_EARNED, pPlayerKiller, this, unBounty);

        if (IsCreep())
            pPlayerKiller->AdjustStat(PLAYER_STAT_CREEP_KILLS, 1);
        else if (IsNeutral())
            pPlayerKiller->AdjustStat(PLAYER_STAT_NEUTRAL_KILLS, 1);

        if (unBounty > 0 && (IsCreep() || IsNeutral() || IsGadget() || IsPet()))
            Game.SendPopup(POPUP_CREEP_KILL, this, pKiller);
    }
    else if (IsHero() && pKiller != nullptr)
    {
        CTeamInfo *pTeam(Game.GetTeam(pKiller->GetTeam()));
        if (pTeam != nullptr)
        {
            ivector &vPlayers(pTeam->GetClientList());

            int iCount(MAX(int(vPlayers.size()), 1));

            for (ivector_it it(vPlayers.begin()); it != vPlayers.end(); ++it)
            {
                CPlayer *pPlayer(Game.GetPlayerFromClientNumber(*it));
                if (pPlayer == nullptr)
                    continue;

                pPlayer->GiveGold(INT_CEIL(unBounty / iCount), pPlayer->GetHero());
                pPlayer->GetGoldReport()->AddPlayerAssistGoldEarned(INT_CEIL(unBounty / iCount));
                Game.LogGold(GAME_LOG_GOLD_EARNED, pPlayer, this, INT_CEIL(unBounty / iCount));
            }
        }
    }

    if (!IsHero())
        Game.LogKill(this, pKiller);

    // Team gold bounty
    if (bDeny)
    {
        ushort unTeamBounty(0);

        if (IsTower())
        {
            CGameInfo *pGameInfo(Game.GetGameInfo());
            if (pGameInfo != nullptr)
                unTeamBounty = (ushort)(GetGoldBountyTeam() * pGameInfo->GetTowerDenyGoldMultiplier());
            else
                assert(!"Invalid game info");
        }

        if (unTeamBounty > 0)
        {
            CTeamInfo *pTeam(Game.GetTeam(GetTeam() ^ 3));
            if (pTeam != nullptr)
            {
                ivector &vPlayers(pTeam->GetClientList());
                for (ivector_it it(vPlayers.begin()); it != vPlayers.end(); ++it)
                {
                    CPlayer *pPlayer(Game.GetPlayerFromClientNumber(*it));
                    if (pPlayer == nullptr)
                        continue;

                    pPlayer->GiveGold(unTeamBounty, pPlayer->GetHero());
                
                    if (IsCreep() || IsNeutral() || IsGadget() || IsPet())
                        pPlayer->GetGoldReport()->AddCreepGoldEarned(unTeamBounty);
                    else if (IsHero())
                        pPlayer->GetGoldReport()->AddPlayerAssistGoldEarned(unTeamBounty);
                    else if (IsBuilding())
                        pPlayer->GetGoldReport()->AddBuildingGoldEarned(unTeamBounty);

                    Game.LogGold(GAME_LOG_GOLD_EARNED, pPlayer, this, unTeamBounty);
                }
            }
        }
    }
    else
    {
        ushort unTeamBounty(pPlayerKiller ? GetGoldBountyTeam() : GetGoldBountyConsolation());

        if (unTeamBounty > 0 && pKiller != nullptr)
        {
            CTeamInfo *pTeam(Game.GetTeam(pKiller->GetTeam()));
            if (pTeam != nullptr)
            {
                ivector &vPlayers(pTeam->GetClientList());
                for (ivector_it it(vPlayers.begin()); it != vPlayers.end(); ++it)
                {
                    CPlayer *pPlayer(Game.GetPlayerFromClientNumber(*it));
                    if (pPlayer == nullptr || pPlayer == pPlayerKiller)
                        continue;

                    pPlayer->GiveGold(unTeamBounty, pPlayer->GetHero());
                    
                    if (IsCreep() || IsNeutral() || IsGadget() || IsPet())
                        pPlayer->GetGoldReport()->AddCreepGoldEarned(unTeamBounty);
                    else if (IsHero())
                        pPlayer->GetGoldReport()->AddPlayerAssistGoldEarned(unTeamBounty);
                    else if (IsBuilding())
                        pPlayer->GetGoldReport()->AddBuildingGoldEarned(unTeamBounty);

                    Game.LogGold(GAME_LOG_GOLD_EARNED, pPlayer, this, unTeamBounty);
                }
            }
        }
    }
}


/*====================
  IUnitEntity::Kill
  ====================*/
void    IUnitEntity::Kill(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    m_uiKiller = pAttacker ? pAttacker->GetIndex() : INVALID_INDEX;
    m_unDeathInflictor = unKillingObjectID;
    SetDeath(true);
}


/*====================
  IUnitEntity::Die
  ====================*/
void    IUnitEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    Action(ACTION_SCRIPT_DEATH, pAttacker, this);

    if (!GetProtectedDeath())
        Action(ACTION_SCRIPT_KILLED, pAttacker, this);

    if (m_hDeathEffect != INVALID_RESOURCE)
    {
        CGameEvent ev;
        ev.SetSourceEntity(m_uiIndex);
        ev.SetEffect(m_hDeathEffect);
        Game.AddEvent(ev);
    }
    
    if (!HasUnitFlags(UNIT_FLAG_ILLUSION) && !GetProtectedDeath())
    {
        if (pAttacker != nullptr)
        {
            pAttacker->Action(ACTION_SCRIPT_KILL, this, this);

            if (pAttacker->GetOwner() != nullptr)
                pAttacker->GetOwner()->Action(ACTION_SCRIPT_INDIRECT_KILL, this, pAttacker);
        }

        KillReward(pAttacker, pAttacker ? pAttacker->GetOwnerPlayer() : nullptr);

        bool bDropAllItems(IsHero() && Game.HasGameOptions(GAME_OPTION_DROP_ITEMS));

        if (!HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
        {
            for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
            {
                IEntityItem *pItem(GetItem(i));
                if (pItem == nullptr)
                    continue;

                bool bLoseOwnerShip(pItem->GetDropOnDeath() || bDropAllItems);
                if (GetDropItemsOnDeath() || bLoseOwnerShip)
                {
                    pItem->Drop(GetPosition(), bLoseOwnerShip);
                }
            }
        }
    }

    StopLiving();

    if (!GetNoDeathAnim())
    {
        bool bIsDeny(pAttacker != nullptr && pAttacker->GetTeam() == GetTeam());
        if (bIsDeny && g_unitPlayDenyAnims && !GetDeniedAnim().empty())
            StartAnimation(GetDeniedAnim(), 0);
        else if (!bIsDeny && pAttacker != nullptr && pAttacker->GetOwnerPlayer() != nullptr && g_unitPlayGibAnims && !GetGibAnim().empty())
            StartAnimation(GetGibAnim(), 0);
        else
        {
            if (GetUseAltDeathAnims())
                StartRandomAnimation(GetAltDeathAnim(), GetAltDeathNumAnims(), 0);
            else
                StartRandomAnimation(GetDeathAnim(), GetDeathNumAnims(), 0);
        }
    }
}


/*====================
  IUnitEntity::Expire
  ====================*/
void    IUnitEntity::Expire()
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    IUnitEntity *pMounted(nullptr);
    IGadgetEntity *pGadget(GetAsGadget());
    if (pGadget != nullptr)
        pMounted = Game.GetUnitEntity(pGadget->GetMountIndex());
    Action(ACTION_SCRIPT_EXPIRED, pMounted, this);

    StopLiving();

    if (m_hDeathEffect != INVALID_RESOURCE)
    {
        CGameEvent ev;
        ev.SetSourceEntity(m_uiIndex);
        ev.SetEffect(m_hDeathEffect);
        Game.AddEvent(ev);
    }

    if (GetExpireNumAnims() != 0)
        StartRandomAnimation(GetExpireAnim(), GetExpireNumAnims(), 0);
    else
    {
        if (GetUseAltDeathAnims())
            StartRandomAnimation(GetAltDeathAnim(), GetAltDeathNumAnims(), 0);
        else
            StartRandomAnimation(GetDeathAnim(), GetDeathNumAnims(), 0);
    }

    // if we are a chest, and we contain an item which is unkillable, then respawn the item.
    if (IsChest())
    {
        for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
        {
            IEntityItem *pItem(GetItem(i));
            if (pItem == nullptr)
                continue;

            if (pItem->GetUnkillable())
                pItem->Drop(GetPosition(),true);
        }
    }
    
    if (!HasUnitFlags(UNIT_FLAG_ILLUSION) && !HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
    {
        for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
        {
            IEntityItem *pItem(GetItem(i));
            if (pItem == nullptr)
                continue;

            if (!GetDropItemsOnDeath() && !pItem->GetDropOnDeath())
                continue;

            pItem->Drop(GetPosition(),true);
        }
    }
}


/*====================
  IUnitEntity::StopLiving
  ====================*/
void    IUnitEntity::StopLiving()
{
    // Deactivate toggles
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr)
            continue;

        pTool->Interrupt(UNIT_ACTION_DEATH);

        if (pTool->GetActionType() == TOOL_ACTION_TOGGLE && pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
            pTool->ToggleOff();
    }

    // Expire states
    for (int i(INVENTORY_START_STATES); i <= INVENTORY_END_STATES; ++i)
    {
        IEntityState *pState(GetState(i));
        if (pState == nullptr)
            continue;
        if (pState->GetDeathPersist())
            continue;

        RemoveState(i);
    }

    m_fHealth = 0.0f;
    m_fMana = 0.0f;

    if (HasUnitFlags(UNIT_FLAG_ILLUSION) && !GetIllusionDeathAnim())
        m_uiDeathTime = Game.GetGameTime();
    else
        m_uiDeathTime = Game.GetGameTime() + GetDeathTime();

    ReleaseBinds();
    
    Unlink();
    if (m_uiDeathTime == Game.GetGameTime() || GetNoDeathAnim())
    {
        if (HasUnitFlags(UNIT_FLAG_ILLUSION) && !GetIllusionDeathAnim())
            m_uiCorpseTime = Game.GetGameTime();
        else
            m_uiCorpseTime = Game.GetGameTime() + (GetNoCorpse() ? 0 : GetCorpseTime());

        SetStatus(ENTITY_STATUS_CORPSE);
    }
    else
    {
        SetStatus(ENTITY_STATUS_DEAD);
    }
    Link();

    for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
        m_auiAnimLockTime[i] = INVALID_TIME;

    m_cBrain.Killed();

    UnblockPath();

    if (GetBindFlags() & ENT_BIND_UNBIND_ON_DEATH)
        Unbind();

    if (HasUnitFlags(UNIT_FLAG_ILLUSION))
        return;

    // Log damage
    for (LogDamageVector_it it(m_vLogDamageTrackers.begin()); it != m_vLogDamageTrackers.end(); ++it)
        Game.LogDamage(this, it->iPlayerOwner, it->unAttackerType, it->unInflictorType, it->fDamage);
    m_vLogDamageTrackers.clear();

    Game.UnitKilled(m_uiIndex);
}


/*====================
  IUnitEntity::GetLinkFlags
  ====================*/
uint    IUnitEntity::GetLinkFlags()
{
    uint uiLinkFlags(m_uiLinkFlags | SURF_DYNAMIC);

    if (m_yStatus == ENTITY_STATUS_DEAD)
        uiLinkFlags |= SURF_DEAD;
    else if (m_yStatus == ENTITY_STATUS_CORPSE)
        uiLinkFlags |= SURF_CORPSE;

    if (GetFlying())
        uiLinkFlags |= SURF_FLYING;
    if (m_uiLinkFlags & SURF_UNIT && GetUnitwalking())
        uiLinkFlags |= SURF_UNITWALKING;
    if (m_uiLinkFlags & (SURF_UNIT | SURF_BUILDING) && GetBoundsRadius() == 0.0f)
        uiLinkFlags |= SURF_NOBLOCK;
    if (GetAntiBlocking())
        uiLinkFlags |= SURF_NOBLOCK;

    if (!GetIsSelectable())
        uiLinkFlags |= SURF_NOSELECT;

    return uiLinkFlags;
}


/*====================
  IUnitEntity::Link
  ====================*/
void    IUnitEntity::Link()
{
    PROFILE("IUnitEntity::Link");

    if (m_uiWorldIndex == INVALID_INDEX)
        return;

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
    if (pWorldEnt == nullptr)
        return;

    uint uiLinkFlags(GetLinkFlags());

    if (GetBlocking())
    {
        uiLinkFlags &= ~SURF_UNIT;
        uiLinkFlags |= SURF_BLOCKING;
    }

    if (m_yStatus == ENTITY_STATUS_DORMANT)
    {
        m_uiLinkedFlags = uiLinkFlags;
        return;
    }

    pWorldEnt->SetPosition(GetPosition());
    pWorldEnt->SetScale(GetBaseScale() * GetScale());
    pWorldEnt->SetScale2(1.0f); // Change this to GetScale() if we want bounds to resize with scale changes caused by states
    pWorldEnt->SetAngles(GetAngles());
    pWorldEnt->SetBounds(GetBounds());
    pWorldEnt->SetModelHandle(GetModel());
    pWorldEnt->SetGameIndex(GetIndex());

    Game.LinkEntity(m_uiWorldIndex, LINK_BOUNDS, uiLinkFlags);

    m_uiLinkedFlags = uiLinkFlags;
}


/*====================
  IUnitEntity::Unlink
  ====================*/
void    IUnitEntity::Unlink()
{
    if (m_uiWorldIndex != INVALID_INDEX)
        Game.UnlinkEntity(m_uiWorldIndex);

    m_uiLinkedFlags = 0;
}


/*====================
  IUnitEntity::BlockPath
  ====================*/
void    IUnitEntity::BlockPath()
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE ||
        (m_uiLinkFlags & (SURF_ITEM | SURF_BUILDING)) ||
        GetFlying() ||
        (m_uiLinkFlags & SURF_UNIT && GetUnitwalking()) ||
        (m_uiLinkFlags & SURF_UNIT && GetBoundsRadius() == 0.0f))
        return;

    uint uiNavType;

    if (GetAntiBlocking())
        uiNavType = NAVIGATION_ANTI;
    else if (GetBlocking())
        uiNavType = NAVIGATION_BUILDING;
    else
        uiNavType = NAVIGATION_UNIT;

    m_vPathBlockers.push_back(Game.BlockPath(uiNavType, m_bbBounds.GetMin().xy() + m_v3Position.xy() + CVec2f(-g_pathPad, -g_pathPad), m_bbBounds.GetDim(X) + g_pathPad * 2.0f, m_bbBounds.GetDim(Y) + g_pathPad * 2.0f));

    m_v2BlockPosition = m_v3Position.xy();
}


/*====================
  IUnitEntity::UnblockPath
  ====================*/
void    IUnitEntity::UnblockPath()
{
    if (m_uiLinkFlags & (SURF_ITEM | SURF_BUILDING))
        return;

    vector<PoolHandle>::const_iterator citEnd(m_vPathBlockers.end());
    for (vector<PoolHandle>::const_iterator cit(m_vPathBlockers.begin()); cit != citEnd; ++cit)
        Game.ClearPath(*cit);

    m_vPathBlockers.clear();
}


/*====================
  IUnitEntity::ExecuteActionScript
  ====================*/
float   IUnitEntity::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target, float fDefault)
{
    if (m_pDefinition == nullptr)
        return 0.0f;

    return m_pDefinition->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, nullptr, GetLevel(), nullptr, nullptr, V_ZERO, fDefault);
}


/*====================
  IUnitEntity::Slide
  ====================*/
bool    IUnitEntity::Slide(CVec2f v2MovementVector, uint uiTraceFlags, CPlane &plImpactPlane)
{
    const float SLIDE_OVERTRACE(2.0f);
    const int MAX_SLIDE_MOVES(8);

    plImpactPlane = CPlane(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (v2MovementVector == V2_ZERO)
        return false;

    STraceInfo trace;

    if (d_printSlideInfo && Game.IsServer())
        Console << _T("Slide: ");

    bool bFlying(GetFlying());

    if (bFlying)
    {
        CVec3f v3NewPosition(m_v3Position + CVec3f(v2MovementVector, 0.0f));
        v3NewPosition.z = Game.GetCameraHeight(v3NewPosition.x, v3NewPosition.y) + GetFlyHeight();

        m_v3Position = v3NewPosition;
        return false;
    }

    int iMoves(0);
    int iZeroCount(0);
    bool bBlocked(false);
    CPlane plZeroPlanes[3];
    bool bDoubleHit[3] = { false, false, false };

    while (v2MovementVector.LengthSq() >= SQR(CONTACT_EPSILON / SLIDE_OVERTRACE) && iZeroCount < 3 && iMoves < MAX_SLIDE_MOVES)
    {
        CVec3f v3OldPosition(m_v3Position);
        CVec3f v3NewPosition(m_v3Position + CVec3f(v2MovementVector * SLIDE_OVERTRACE, 0.0f));

        v3NewPosition.z = Game.GetTerrainHeight(v3NewPosition.x, v3NewPosition.y);

        Game.TraceBox(trace, v3OldPosition, v3NewPosition, m_bbBounds, uiTraceFlags, m_uiWorldIndex);

        float fFraction(trace.fFraction * SLIDE_OVERTRACE);

        CVec3f v3EndPos;

        if (fFraction > 1.0f)
        {
            v3EndPos = LERP(1.0f / fFraction, v3OldPosition, trace.v3EndPos);
            fFraction = 1.0f;
        }
        else
            v3EndPos = trace.v3EndPos;

        m_v3Position = v3EndPos;
        v2MovementVector *= (1.0f - fFraction);
  
        if (fFraction < 1.0f)
        {
            float fZeroFraction(CONTACT_EPSILON / Length(v3NewPosition - v3OldPosition));

            if (!trace.plPlane.v3Normal.IsValid())
                break;

            if (fFraction > fZeroFraction)
                iZeroCount = 0;

            // Check if we already hit this plane once
            bool bBreak(false);
            bool bContinue(false);

            for (int i(0); iMoves == iZeroCount ? i < iZeroCount : i <= iZeroCount; ++i)
            {
                if (trace.plPlane == plZeroPlanes[i])
                {
                    if (bDoubleHit[i]) // Hit this plane twice, so just give up
                    {
                        if (d_printSlideInfo && Game.IsServer())
                            Console << _T("Triple Hit ");

                        v2MovementVector.Clear();
                        bBreak = true;
                        break;
                    }

                    if (d_printSlideInfo && Game.IsServer())
                        Console << _T("Double Hit ");

                    // Nudge our velocity away from plane and try again
                    bDoubleHit[i] = true;
                    v2MovementVector += trace.plPlane.v3Normal.xy();
                    bContinue = true;
                    break;
                }
            }

            if (bContinue)
                continue;
            else if (bBreak)
                break;

            bool bNoPrev;

            if (iMoves == iZeroCount) // We don't have any previous successful moves
            {
                bNoPrev = true;
                plZeroPlanes[iZeroCount] = trace.plPlane;
            }
            else
            {
                bNoPrev = false;

                if (fFraction <= fZeroFraction)
                    ++iZeroCount;

                plZeroPlanes[iZeroCount] = trace.plPlane;
            }

            switch (iZeroCount)
            {
            case 0: // Single plane intersection, slide along plane
                {
                    if (DotProduct(Normalize(v2MovementVector), plZeroPlanes[0].v3Normal.xy()) > -g_blockSlope)
                        v2MovementVector.Clip(plZeroPlanes[0].v3Normal.xy());
                    else
                    {
                        v2MovementVector.Clear();
                        bBlocked = true;
                    }

                    plImpactPlane = plZeroPlanes[0];
                }
                break;
            case 1: // Double plane intersection, clear velocity
                {
                    if (d_printSlideInfo && Game.IsServer())
                        Console << _T("Double Zero ");
                    
                    v2MovementVector.Clear();
                    bBlocked = true;
                    plImpactPlane = CPlane(0.0f, 0.0f, 0.0f, 0.0f);
                }
                break;
            }

            if (bNoPrev && fFraction <= fZeroFraction)
                ++iZeroCount;
            
            if (!v2MovementVector.IsValid())
                v2MovementVector.Clear();
            
            if (v2MovementVector.LengthSq() < 0.00001f)
                v2MovementVector.Clear();
        }

        if (d_printSlideInfo && Game.IsServer())
            Console << fFraction << _T(" ");

        ++iMoves;
    }

    if (d_printSlideInfo && Game.IsServer())
        Console << newl;

    if (iMoves == MAX_SLIDE_MOVES)
        Console.Warn << _T("IUnitEntity::Slide Moves == MAX_SLIDE_MOVES") << newl;

    return bBlocked;
}


/*====================
  IUnitEntity::TestSlide
  ====================*/
bool    IUnitEntity::TestSlide(CVec2f v2MovementVector, float fFraction, bool bDirectPathing)
{
    if (v2MovementVector == V2_ZERO)
        return true;

    uint uiTraceFlags(TRACE_UNIT_MOVEMENT);
    if (GetUnitwalking())
        uiTraceFlags |= SURF_UNIT;
    if (GetTreewalking())
        uiTraceFlags |= SURF_TREE;
    if (GetCliffwalking())
        uiTraceFlags |= SURF_CLIFF | SURF_PROP;
    if (GetBuildingwalking())
        uiTraceFlags |= SURF_BUILDING | SURF_BLOCKING;
    if (bDirectPathing)
        uiTraceFlags &= ~SURF_BLOCKER;

    STraceInfo trace;

    CVec3f v3OldPosition(m_v3Position);
    CVec3f v3NewPosition(m_v3Position + CVec3f(v2MovementVector, 0.0f));

    v3NewPosition.z = Game.GetTerrainHeight(v3NewPosition.x, v3NewPosition.y);

    Game.TraceBox(trace, v3OldPosition, v3NewPosition, m_bbBounds, uiTraceFlags, m_uiWorldIndex);

    return trace.fFraction > fFraction;
}


/*====================
  IUnitEntity::JustWalkNike
  ====================*/
bool    IUnitEntity::JustWalkNike(const CVec2f &v2MovementVector, CPlane &plOutImpactPlane, bool bDirectPathing)
{
    Unlink();

    uint uiTraceFlags(TRACE_UNIT_MOVEMENT);
    if (GetUnitwalking())
        uiTraceFlags |= SURF_UNIT;
    if (GetTreewalking())
        uiTraceFlags |= SURF_TREE;
    if (GetCliffwalking())
        uiTraceFlags |= SURF_CLIFF | SURF_PROP;
    if (GetBuildingwalking())
        uiTraceFlags |= SURF_BUILDING | SURF_BLOCKING;
    if (bDirectPathing)
        uiTraceFlags &= ~SURF_BLOCKER;

    bool bBlocked(Slide(v2MovementVector, uiTraceFlags, plOutImpactPlane));

    Link();

    return bBlocked;
}


/*====================
  IUnitEntity::StartAttack

  Start phase of the attack
  ====================*/
bool    IUnitEntity::StartAttack(IUnitEntity* pTarget, bool bAbility, bool bAggro)
{
    if (bAggro && pTarget != nullptr && pTarget->IsHero() && !pTarget->IsIllusion() && pTarget->GetTeam() != GetTeam())
        AggroCreeps(g_heroAttackAggroRange, g_heroAttackAggroTime, pTarget->GetTeam(), g_heroAttackAggroDelay, g_heroAttackReaggroBlock);

    if (GetAttackStartEffect() != INVALID_RESOURCE)
    {
        CGameEvent ev;
        ev.SetSourceEntity(m_uiIndex);
        ev.SetEffect(GetAttackStartEffect());
        Game.AddEvent(ev);
    }

    CPlayer *pOwner(GetOwnerPlayer());
    if (pOwner != nullptr)
        pOwner->SetLastInteractionTime(Game.GetGameTime());

    m_cCombatEvent.Reset();

    m_unAttackProjectile = EntityRegistry.LookupID(GetAttackProjectile());
    m_hAttackActionEffect = GetAttackActionEffect();
    m_hAttackImpactEffect = GetAttackImpactEffect();
    m_uiAttackAbilityUID = INVALID_INDEX;

    if (!bAbility)
    {
        // Look for auto-casting attack abilities
        for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
        {
            IEntityTool *pTool(GetTool(iSlot));
            if (pTool == nullptr)
                continue;
            if (!pTool->IsActive())
                continue;

            if ((pTool->GetActionType() != TOOL_ACTION_ATTACK && pTool->GetActionType() != TOOL_ACTION_ATTACK_TOGGLE) ||
                !pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
                continue;

            if (pTool->IsDisabled() ||
                !pTool->IsReady() ||
                !pTool->IsValidTarget(pTarget))
                continue;
            
            if (!Game.IsValidTarget(pTool->GetActivateScheme(), 0, this, this, true))
                continue;

            float fManaCost(IsFreeCast() ? 0.0f : pTool->GetCurrentManaCost());
            if (GetMana() < fManaCost)
                continue;

            if (!pTool->CheckCost())
                continue;

            m_uiAttackAbilityUID = pTool->GetUniqueID();

            if (!pTool->GetAnim().empty())
                StartAnimation(pTool->GetAnim(), pTool->GetAnimChannel(), GetAttackSpeed());

            pTool->ExecuteActionScript(ACTION_SCRIPT_START, pTarget, pTarget != nullptr ? pTarget->GetPosition() : V3_ZERO);

            Action(ACTION_SCRIPT_ACTIVATE_START, pTarget, pTool);

            if (pTool->IsAbility())
                Action(ACTION_SCRIPT_ABILITY_START, pTarget, pTool);

            pTool->PlayCastEffect();
            break;
        }
    }

    Action(ACTION_SCRIPT_ATTACK_START, pTarget, this, &m_cCombatEvent);
    pTarget->Action(ACTION_SCRIPT_ATTACKED_START, this, this, &m_cCombatEvent);

    return true;
}


/*====================
  IUnitEntity::Attack

  Impact phase of the attack
  ====================*/
bool    IUnitEntity::Attack(IUnitEntity* pTarget, bool bAttackAbility)
{
    const CAttackType *pAttackType(Game.GetAttackType(GetAttackType()));
    if (pAttackType == nullptr)
        return false;

    if (!Game.IsValidTarget(GetAttackTargetScheme(), 0, this, pTarget))
        return false;

    SetLastAggression(pTarget->GetTeam(), Game.GetGameTime());
    ++m_uiAttackSequence;

    // Allow inventory items to react to the action
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (!m_apInventory[iSlot]->OwnerAction())
            RemoveSlave(iSlot);
    }

    if (m_uiAttackAbilityUID != INVALID_INDEX)
    {
        IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiAttackAbilityUID));

        m_uiAttackAbilityUID = INVALID_INDEX;

        if (pEntity != nullptr)
        {
            IEntityTool *pTool(pEntity->GetAsTool());
            if (pTool != nullptr)
            {
                if (pTool->Activate(pTarget, pTarget != nullptr ? pTarget->GetPosition() : V3_ZERO, V3_ZERO, false, -1))
                    return true;
            }
        }
    }

    if (m_hAttackActionEffect != INVALID_RESOURCE)
    {
        CGameEvent ev;
        ev.SetSourceEntity(m_uiIndex);
        ev.SetTargetEntity(pTarget->GetIndex());
        ev.SetEffect(m_hAttackActionEffect);
        Game.AddEvent(ev);
    }

    float fMissChance(GetMissChance());

    if (!Game.HasGameOptions(GAME_OPTION_CASUAL))
    {
        if (Game.GetTerrainHeight(pTarget->GetPosition().x, pTarget->GetPosition().y) - GetPosition().z >= g_elevationAdvantageMin)
            fMissChance = 1.0f - ((1.0f - fMissChance) * (1.0f - pAttackType->GetUphillMissChance()));
    }

    if (m_unAttackProjectile != INVALID_ENT_TYPE)
    {   
        IProjectile *pProjectile(CreateProjectile(EntityRegistry.LookupName(m_unAttackProjectile), pTarget->GetIndex(), 1));
        if (pProjectile != nullptr)
        {
            pProjectile->SetTargetScheme(GetAttackTargetScheme());
            pProjectile->SetAttackImpactEffect(m_hAttackImpactEffect);

            m_cCombatEvent.SetSuperType(SUPERTYPE_ATTACK);
            m_cCombatEvent.SetInitiatorIndex(GetIndex());
            m_cCombatEvent.SetInflictorIndex(pProjectile->GetIndex());
            m_cCombatEvent.SetTarget(pTarget->GetIndex());
            m_cCombatEvent.SetProxyUID(GetProxyUID());
            m_cCombatEvent.SetEffectType(GetAttackEffectType());
            m_cCombatEvent.SetDamageType(GetAttackDamageType());
            m_cCombatEvent.SetBaseDamage(GetBaseDamage());
            m_cCombatEvent.SetAdditionalDamage(GetBonusDamage());
            m_cCombatEvent.SetDamageMultiplier(GetTotalDamageMultiplier());
            m_cCombatEvent.AddBonusDamage(m_fBonusDamage);
            m_cCombatEvent.AddBonusDamageMultiplier(m_fBonusDamageMultiplier);
            m_cCombatEvent.SetLifeSteal(GetLifeSteal());
            m_cCombatEvent.SetNonLethal(GetAttackNonLethal());
            GetCriticals(m_cCombatEvent);
            GetAttackActions(ACTION_SCRIPT_ATTACK_PRE_IMPACT, ACTION_SCRIPT_PRE_IMPACT, m_cCombatEvent);
            GetAttackActions(ACTION_SCRIPT_ATTACK_PRE_DAMAGE, ACTION_SCRIPT_PRE_DAMAGE, m_cCombatEvent);
            GetAttackActions(ACTION_SCRIPT_ATTACK_DAMAGE_EVENT, ACTION_SCRIPT_DAMAGE_EVENT, m_cCombatEvent);
            GetAttackActions(ACTION_SCRIPT_ATTACK_IMPACT, ACTION_SCRIPT_IMPACT, m_cCombatEvent);
            GetAttackActions(ACTION_SCRIPT_ATTACK_IMPACT_INVALID, ACTION_SCRIPT_IMPACT_INVALID, m_cCombatEvent);
            m_cCombatEvent.SetMissChance(fMissChance);
            m_cCombatEvent.SetTrueStrike(GetTrueStrike());
            m_cCombatEvent.SetAttackAbility(bAttackAbility);

            CCombatEvent &cmbtRanged(pProjectile->GetCombatEvent());
            cmbtRanged = m_cCombatEvent;

            Action(ACTION_SCRIPT_ATTACK, pTarget, pProjectile, &cmbtRanged);
        }
    }
    else
    {
        // Not a projectile, so deal our damage immediately
        m_cCombatEvent.SetSuperType(SUPERTYPE_ATTACK);
        m_cCombatEvent.SetInitiatorIndex(GetIndex());
        m_cCombatEvent.SetInflictorIndex(GetIndex());
        m_cCombatEvent.SetTarget(pTarget->GetIndex());
        m_cCombatEvent.SetProxyUID(GetProxyUID());
        m_cCombatEvent.SetEffectType(GetAttackEffectType());
        m_cCombatEvent.SetDamageType(GetAttackDamageType());
        m_cCombatEvent.SetBaseDamage(GetBaseDamage());
        m_cCombatEvent.SetAdditionalDamage(GetBonusDamage());
        m_cCombatEvent.SetDamageMultiplier(GetTotalDamageMultiplier());
        m_cCombatEvent.AddBonusDamage(m_fBonusDamage);
        m_cCombatEvent.AddBonusDamageMultiplier(m_fBonusDamageMultiplier);
        m_cCombatEvent.SetLifeSteal(GetLifeSteal());
        m_cCombatEvent.SetNonLethal(GetAttackNonLethal());
        GetCriticals(m_cCombatEvent);
        GetAttackActions(ACTION_SCRIPT_ATTACK_PRE_IMPACT, ACTION_SCRIPT_PRE_IMPACT, m_cCombatEvent);
        GetAttackActions(ACTION_SCRIPT_ATTACK_PRE_DAMAGE, ACTION_SCRIPT_PRE_DAMAGE, m_cCombatEvent);
        GetAttackActions(ACTION_SCRIPT_ATTACK_DAMAGE_EVENT, ACTION_SCRIPT_DAMAGE_EVENT, m_cCombatEvent);
        GetAttackActions(ACTION_SCRIPT_ATTACK_IMPACT, ACTION_SCRIPT_IMPACT, m_cCombatEvent);
        GetAttackActions(ACTION_SCRIPT_ATTACK_IMPACT_INVALID, ACTION_SCRIPT_IMPACT_INVALID, m_cCombatEvent);
        m_cCombatEvent.SetEvasion(pTarget->GetEvasionMelee());
        m_cCombatEvent.SetMissChance(fMissChance);
        m_cCombatEvent.SetTrueStrike(GetTrueStrike());
        m_cCombatEvent.SetAttackAbility(bAttackAbility);

        Action(ACTION_SCRIPT_ATTACK, pTarget, this, &m_cCombatEvent);

        m_cCombatEvent.Process();

        if (m_cCombatEvent.GetSuccessful() &&
            m_hAttackImpactEffect != INVALID_RESOURCE)
        {
            CGameEvent ev;
            ev.SetSourceEntity(pTarget->GetIndex());
            ev.SetEffect(m_hAttackImpactEffect);
            Game.AddEvent(ev);
        }

        m_cCombatEvent.Reset();
    }

    ClearBonusDamage();

    return true;
}


/*====================
  IUnitEntity::SpawnProjectile
  ====================*/
IProjectile*    IUnitEntity::SpawnProjectile(const tstring &sName, const CVec3f &v3End, uint uiLevel)
{
    IProjectile *pProjectile(Game.AllocateDynamicEntity<IProjectile>(sName));
    if (pProjectile == nullptr)
    {
        Console.Warn << _T("Failed to spawn projectile: ") << sName << newl;
        return nullptr;
    }

    CVec3f v3Start(GetTransformedAttackOffset());
    CVec3f v3Dir(Normalize(v3End - v3Start));

    pProjectile->SetOwner(GetIndex());
    pProjectile->SetPosition(v3Start);
    pProjectile->SetLevel(uiLevel);
    pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Dir));
    pProjectile->SetOriginTime(Game.GetGameTime());
    pProjectile->UpdateModifiers(GetModifierKeys());
    
    pProjectile->GetCombatEvent().SetInflictorIndex(pProjectile->GetIndex());
    
    return pProjectile;
}


/*====================
  IUnitEntity::CreateProjectile
  ====================*/
IProjectile*    IUnitEntity::CreateProjectile(const tstring &sName, uint uiTargetIndex, uint uiLevel)
{
    IUnitEntity *pTarget(Game.GetUnitEntity(uiTargetIndex));
    if (pTarget == nullptr)
        return nullptr;

    CAxis axis(pTarget->GetAngles());

    IProjectile *pProjectile(SpawnProjectile(sName, pTarget->GetPosition() + TransformPoint(pTarget->GetTargetOffset(), axis), uiLevel));
    if (pProjectile == nullptr)
        return nullptr;
    
    pProjectile->SetTargetEntityUID(pTarget->GetUniqueID());
    pProjectile->SetTargetDisjointSequence(pTarget->GetDisjointSequence());
    pProjectile->Spawn();
    return pProjectile;
}

IProjectile*    IUnitEntity::CreateProjectile(const tstring &sName, const CVec3f &v3TargetPosition, uint uiLevel)
{
    IProjectile *pProjectile(SpawnProjectile(sName, v3TargetPosition, uiLevel));
    if (pProjectile == nullptr)
        return nullptr;

    pProjectile->SetTargetPos(v3TargetPosition);
    pProjectile->Spawn();
    return pProjectile;
}


/*====================
  IUnitEntity::Push
  ====================*/
void    IUnitEntity::Push(const CVec2f &v2Velocity, uint uiDuration)
{
    for (uint ui(0); ui < m_vPushRecords.size(); ++ui)
    {
        if (m_vPushRecords[ui].second == INVALID_TIME)
        {
            m_vPushRecords[ui].first = v2Velocity;
            m_vPushRecords[ui].second = Game.GetGameTime() + uiDuration;
            return;
        }
    }

    m_vPushRecords.push_back(PushRecord(v2Velocity, Game.GetGameTime() + uiDuration));
}


/*====================
  IUnitEntity::Interrupt
  ====================*/
void    IUnitEntity::Interrupt(EUnitAction eAction)
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr)
            continue;
        if (!pTool->IsActive())
            continue;

        pTool->Interrupt(eAction);
    }
}


/*====================
  IUnitEntity::IsChanneling
  ====================*/
bool    IUnitEntity::IsChanneling(EUnitAction eAction)
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr)
            continue;
        if (!pTool->IsActive())
            continue;

        if (pTool->IsChanneling(eAction))
            return true;
    }

    return false;
}


/*====================
  IUnitEntity::ValidatePosition
  ====================*/
void    IUnitEntity::ValidatePosition(uint uiIgnoreSurfaces)
{
    PROFILE("IUnitEntity::ValidatePosition");

    if (HasLocalFlags(ENT_LOCAL_FIXED_POSITION))
        return;

    CVec3f v3Position(GetPosition());

    if (GetFlying())
        v3Position.z = Game.GetCameraHeight(v3Position.x, v3Position.y) + GetFlyHeight();
    else
        v3Position.z = Game.GetTerrainHeight(v3Position.x, v3Position.y);

    CRectf recGameBounds(Game.GetBounds());

    v3Position.x = CLAMP(v3Position.x, recGameBounds.left, recGameBounds.right);
    v3Position.y = CLAMP(v3Position.y, recGameBounds.top, recGameBounds.bottom);

    Unlink();
    SetPosition(v3Position);

    if (GetUnitwalking())
        uiIgnoreSurfaces |= SURF_UNIT;
    if (GetTreewalking())
        uiIgnoreSurfaces |= SURF_TREE;
    if (GetCliffwalking())
        uiIgnoreSurfaces |= SURF_CLIFF | SURF_PROP;
    if (GetBuildingwalking())
        uiIgnoreSurfaces |= SURF_BUILDING | SURF_BLOCKING;

    STraceInfo trace;
    Game.TraceBox(trace, v3Position, v3Position + CVec3f(0.0f, 0.0, 1.0f), GetBounds(), uiIgnoreSurfaces, m_uiWorldIndex);
    if (trace.uiEntityIndex == INVALID_INDEX)
    {
        Link();
        return;
    }

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(trace.uiEntityIndex));

    if (pWorldEnt == nullptr)
    {
        Link();
        return;
    }

    if (pWorldEnt->GetGameIndex() != INVALID_INDEX)
    {
        IGameEntity *pBlocker(Game.GetEntity(pWorldEnt->GetGameIndex()));
        if (pBlocker != nullptr && pBlocker->GetUniqueID() == m_uiBindTargetUID)
        {
            Link();
            return;
        }
    }

    float fBlockerRadius;
    CVec3f v3BlockerPosition;

    if (pWorldEnt->GetSurfFlags() & SURF_BOUNDS)
    {
        fBlockerRadius = MAX(pWorldEnt->GetWorldBounds().GetDim(X), pWorldEnt->GetWorldBounds().GetDim(Y)) * 0.5f;
        v3BlockerPosition = pWorldEnt->GetPosition();
    }
    else if (pWorldEnt->GetSurfFlags() & SURF_HULL)
    {
        fBlockerRadius = MAX(pWorldEnt->GetSurfaceBounds().GetDim(X), pWorldEnt->GetSurfaceBounds().GetDim(Y)) * 0.5f;
        v3BlockerPosition = pWorldEnt->GetPosition();
    }
    else
    {
        Link();
        return;
    }
    
    float fRadius(32.0f);
    CVec2f v2Position(v3Position.xy());

    CVec2f v2BlockerPosition(v3BlockerPosition.xy());
    CVec3f v3BlockerUp(v2BlockerPosition, FAR_AWAY);
    CVec2f v2Dir(v2BlockerPosition - v2Position);
    if (v2Dir.Normalize() == 0.0f)
    {
        v2Dir = M_RandomDirection().xy();
        v2Dir.Normalize();
    }
    
    CVec2f v2TestPoint(M_PointOnLine(v2BlockerPosition, -v2Dir, fRadius + fBlockerRadius));

    float fStepHeight(g_elevationStepMax - g_elevationStepMin);
    int iCurrentLevel(INT_CEIL((v3Position[Z] - g_elevationStepMin) / fStepHeight));

    CVec3f v3NextBest(v3Position);
    int iNextBestLevel(INT_MAX);

    float fAngle(0.0f);
    float fAngleStep(RAD2DEG(acos(1.0f - (SQR(2.0f * fRadius) / (2.0f * SQR(fRadius + fBlockerRadius))))));
    for (int i(0); i < 5 && fRadius > 0.0f; ++i)
    {
        while (fAngle < 360.0f && fAngleStep > 0.0f)
        {
            CVec3f v3TestPoint(v2TestPoint, 0.0f);
            v3TestPoint = M_RotatePointAroundLine(v3TestPoint, v3BlockerPosition, v3BlockerUp, fAngle);
            v3TestPoint[Z] = Game.GetTerrainHeight(v3TestPoint[X], v3TestPoint[Y]);
            Game.TraceBox(trace, v3TestPoint, v3TestPoint + CVec3f(0.0f, 0.0, 1.0f), GetBounds(), uiIgnoreSurfaces, m_uiWorldIndex);

            if (!trace.bHit)
            {
                int iLevel(INT_CEIL((v3TestPoint[Z] - g_elevationStepMin) / fStepHeight));
                
                if (iLevel == iCurrentLevel)
                {
                    SetPosition(v3TestPoint);
                    Link();
                    return;
                }

                int iLevelDiff(ABS(iLevel - iCurrentLevel));
                if (iLevelDiff < iNextBestLevel)
                {
                    iNextBestLevel = iLevelDiff;
                    v3NextBest = v3TestPoint;
                }
            }
                

            fAngle += fAngleStep;
        }

        v2TestPoint -= v2Dir * (2.0f * fRadius);
        fAngleStep = RAD2DEG(acos(1.0f - (SQR(2.0f * fRadius) / (2.0f * SQR(fRadius + fBlockerRadius + (fRadius * 2.0f * (i + 1)))))));
        fAngle = 0.0f;
    }

    if (iNextBestLevel != INT_MAX)
    {
        SetPosition(v3NextBest);
        Link();
        return;
    }

    Console.Warn << _T("No valid position found!") << newl;
    Link();
}


/*====================
  IUnitEntity::ValidatePosition2

  Called every frame to unstick things
  ====================*/
void    IUnitEntity::ValidatePosition2()
{
    if (GetFlying())
        return;

    if (HasLocalFlags(ENT_LOCAL_FIXED_POSITION))
        return;

    uint uiIgnoreSurfaces(TRACE_UNIT_MOVEMENT);
    if (GetUnitwalking())
        uiIgnoreSurfaces |= SURF_UNIT;
    if (GetTreewalking())
        uiIgnoreSurfaces |= SURF_TREE;
    if (GetCliffwalking())
        uiIgnoreSurfaces |= SURF_CLIFF | SURF_PROP;
    if (GetBuildingwalking())
        uiIgnoreSurfaces |= SURF_BUILDING | SURF_BLOCKING;

    CVec3f v3Position(GetPosition());

    CRectf recGameBounds(Game.GetBounds());

    v3Position.x = CLAMP(v3Position.x, recGameBounds.left, recGameBounds.right);
    v3Position.y = CLAMP(v3Position.y, recGameBounds.top, recGameBounds.bottom);

    if (v3Position != GetPosition())
    {
        Unlink();
        SetPosition(v3Position);
        Link();
    }

    STraceInfo trace;
    Game.TraceBox(trace, v3Position, v3Position + CVec3f(0.0f, 0.0, 1.0f), GetBounds(), uiIgnoreSurfaces, m_uiWorldIndex);
    if (trace.uiEntityIndex == INVALID_INDEX)
        return;

    CWorldEntity *pWorldEnt(Game.GetWorldEntity(trace.uiEntityIndex));

    if (pWorldEnt == nullptr)
        return;

    if (pWorldEnt->GetGameIndex() != INVALID_INDEX)
    {
        IGameEntity *pBlocker(Game.GetEntity(pWorldEnt->GetGameIndex()));
        if (pBlocker != nullptr && pBlocker->GetUniqueID() == m_uiBindTargetUID)
            return;
    }

    float fBlockerRadius;
    CVec3f v3BlockerPosition;

    if (pWorldEnt->GetSurfFlags() & SURF_BOUNDS)
    {
        fBlockerRadius = MAX(pWorldEnt->GetWorldBounds().GetDim(X), pWorldEnt->GetWorldBounds().GetDim(Y)) * 0.5f;
        v3BlockerPosition = pWorldEnt->GetPosition();
    }
    else if (pWorldEnt->GetSurfFlags() & SURF_HULL)
    {
        fBlockerRadius = MAX(pWorldEnt->GetSurfaceBounds().GetDim(X), pWorldEnt->GetSurfaceBounds().GetDim(Y)) * 0.5f;
        v3BlockerPosition = pWorldEnt->GetPosition();
    }
    else
    {
        return;
    }
    
    float fRadius(32.0f);
    CVec2f v2Position(v3Position.xy());

    CVec2f v2BlockerPosition(v3BlockerPosition.xy());
    CVec3f v3BlockerUp(v2BlockerPosition, FAR_AWAY);
    CVec2f v2Dir(v2BlockerPosition - v2Position);
    if (v2Dir.Normalize() == 0.0f)
    {
        v2Dir = M_RandomDirection().xy();
        v2Dir.Normalize();
    }

    CVec2f v2TestPoint(M_PointOnLine(v2BlockerPosition, -v2Dir, fRadius + fBlockerRadius));

    float fStepHeight(g_elevationStepMax - g_elevationStepMin);
    int iCurrentLevel(INT_CEIL((v3Position[Z] - g_elevationStepMin) / fStepHeight));

    CVec3f v3NextBest(v3Position);
    int iNextBestLevel(INT_MAX);

    float fAngle(0.0f);
    float fAngleStep(RAD2DEG(acos(1.0f - (SQR(2.0f * fRadius) / (2.0f * SQR(fRadius + fBlockerRadius))))));

    for (int i(0); i < 5; ++i)
    {
        while (fAngle < 360.0f)
        {
            CVec3f v3TestPoint(v2TestPoint, 0.0f);
            v3TestPoint = M_RotatePointAroundLine(v3TestPoint, v3BlockerPosition, v3BlockerUp, fAngle);
            v3TestPoint[Z] = Game.GetTerrainHeight(v3TestPoint[X], v3TestPoint[Y]);
            Game.TraceBox(trace, v3TestPoint, v3TestPoint + CVec3f(0.0f, 0.0, 1.0f), GetBounds(), uiIgnoreSurfaces, m_uiWorldIndex);

            if (!trace.bHit)
            {
                int iLevel(INT_CEIL((v3TestPoint[Z] - g_elevationStepMin) / fStepHeight));
                
                if (iLevel == iCurrentLevel)
                {
                    Unlink();
                    SetPosition(v3TestPoint);
                    Link();
                    return;
                }

                if (ABS(iLevel - iCurrentLevel) < iNextBestLevel)
                {
                    iNextBestLevel = iLevel;
                    v3NextBest = v3TestPoint;
                }
            }

            fAngle += fAngleStep;
        }

        v2TestPoint -= v2Dir * (2.0f * fRadius);
        fAngleStep = RAD2DEG(acos(1.0f - (SQR(2.0f * fRadius) / (2.0f * SQR(fRadius + fBlockerRadius + (fRadius * 2.0f * (i + 1)))))));
        fAngle = 0.0f;
    }

    if (iNextBestLevel != INT_MAX)
    {
        Unlink();
        SetPosition(v3NextBest);
        Link();
        return;
    }

    Console.Warn << _T("No valid position found!") << newl;
}


/*====================
  IUnitEntity::ShouldTarget
  ====================*/
bool    IUnitEntity::ShouldTarget(IGameEntity *pOther)
{
    IUnitEntity *pTarget(pOther->GetAsUnit());
    if (pTarget == nullptr)
        return false;

    if (pTarget->GetTeam() == TEAM_PASSIVE)
        return false;

    if (!Game.IsValidTarget(GetAttackTargetScheme(), GetAttackEffectType(), this, pTarget, false))
        return false;

    if (pTarget->GetNoThreat())
        return false;

    if (!Game.IsValidTarget(GetThreatScheme(), GetThreatEffectType(), this, pTarget, true))
        return false;

    if (IsEnemy(pTarget) && pTarget->GetStatus() == ENTITY_STATUS_ACTIVE && CanSee(pTarget))
        return true;

    return false;
}


/*====================
  IUnitEntity::IsEnemy
  ====================*/
bool    IUnitEntity::IsEnemy(IUnitEntity *pOther) const
{
    if (pOther == nullptr)
        return false;
    if (pOther == this)
        return false;
    if (pOther->GetTeam() == GetTeam())
        return false;
    if (Game.GetTeam(GetTeam()) != nullptr && Game.GetTeam(GetTeam())->IsAlliedTeam(pOther->GetTeam()))
        return false;

    return true;
}


/*====================
  IUnitEntity::PlayerCommand
  ====================*/
uint    IUnitEntity::PlayerCommand(const SUnitCommand &cCmd)
{
    SUnitCommand cUnitCmd(cCmd);

    cUnitCmd.uiOrderSequence = m_uiOrderSequence;

    ++m_uiOrderSequence;

    SetTargetIndex(cUnitCmd.uiIndex); // I hate you whoever did this...
    m_cBrain.AddCommand(cUnitCmd);

    return cUnitCmd.uiOrderSequence;
}


/*====================
  IUnitEntity::UpdateSkeleton
  ====================*/
void    IUnitEntity::UpdateSkeleton(bool bPose)
{
    float fTiltFactor(GetStatus() == ENTITY_STATUS_ACTIVE ? GetTiltFactor() : GetCorpseTiltFactor());

    // Tilting
    if (fTiltFactor != 0.0f)
    {
        CAxis axis(m_v3Angles);
        float fTiltSpeed(GetStatus() == ENTITY_STATUS_ACTIVE ? GetTiltSpeed() : GetCorpseTiltSpeed());
        
        float fTargetPitch, fTargetRoll;

        if (true)
        {
            CVec3f v3Normal(Game.GetTerrainNormal(m_v3Position.x, m_v3Position.y));

            fTargetPitch = -90.0f + RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Forward2d(), 0.0f))));
            fTargetRoll = 90.0f - RAD2DEG(acos(DotProduct(v3Normal, CVec3f(axis.Right2d(), 0.0f))));
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
    }
    else
    {
        m_fTiltPitch = 0.0f;
        m_fTiltRoll = 0.0f;
    }

    if (m_pSkeleton == nullptr)
        return;

    m_pSkeleton->SetModel(GetModel());

    if (GetModel() == INVALID_RESOURCE)
        return;

    // Pose skeleton
    if (bPose)
        m_pSkeleton->Pose(Game.GetGameTime(), 0.0f, 0.0f);
    else
        m_pSkeleton->PoseLite(Game.GetGameTime());

    // Process animation events
    if (m_pSkeleton->CheckEvents())
    {
        tstring sOldDir(FileManager.GetWorkingDirectory());
        FileManager.SetWorkingDirectory(Filename_GetPath(g_ResourceManager.GetPath(GetModel())));

        const vector<SAnimEventCmd> &vEventCmds(m_pSkeleton->GetEventCmds());

        for (vector<SAnimEventCmd>::const_iterator it(vEventCmds.begin()); it != vEventCmds.end(); ++it)
            EventScript.Execute(it->sCmd, it->iTimeNudge);

        m_pSkeleton->ClearEvents();

        FileManager.SetWorkingDirectory(sOldDir);
    }
}


/*====================
  IUnitEntity::GetAnimSpeed
  ====================*/
void    IUnitEntity::GetAnimState(int iChannel, int &iAnim, byte &ySequence, float &fSpeed)
{
    if (iChannel != 0)
    {
        IVisualEntity::GetAnimState(iChannel, iAnim, ySequence, fSpeed);
        return;
    }

    const tstring *psForceAnim(nullptr);

    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (!m_apInventory[iSlot]->GetForceAnim().empty())
            psForceAnim = &m_apInventory[iSlot]->GetForceAnim();
    }

    if (psForceAnim)
    {
        iAnim = GetAnimIndex(*psForceAnim);
        ySequence = 0;
        fSpeed = 1.0f;
    }
    else
    {
        IVisualEntity::GetAnimState(iChannel, iAnim, ySequence, fSpeed);
    }
}


/*====================
  IUnitEntity::IsPlayingAnim
  ====================*/
bool    IUnitEntity::IsPlayingAnim(int iChannel, const tstring &sAnimName, int iNumAnims)
{
    size_t zPos(sAnimName.find(_T('%')));
    if (zPos == tstring::npos)
        return m_asAnim[iChannel] == sAnimName;

    const tstring &sFirstPart(sAnimName.substr(0, zPos));
    const tstring &sLastPart(sAnimName.substr(zPos + 1));

    for (int i(1); i <= iNumAnims; ++i)
    {
        tstring sRandomAnimName(sFirstPart + XtoA(i) + sLastPart);
        if (m_asAnim[iChannel] == sAnimName)
            return true;
    }

    return false;
}


/*====================
  IUnitEntity::GetSelectionColor
  ====================*/
CVec4f  IUnitEntity::GetSelectionColor(CPlayer *pLocalPlayer)
{
    if (pLocalPlayer == nullptr || pLocalPlayer->GetTeam() == TEAM_SPECTATOR)
    {
        if (GetTeam() == TEAM_PASSIVE || GetTeam() == TEAM_SPECTATOR)
            return WHITE;
        else if (GetTeam() == 1)
            return GetStatus() == ENTITY_STATUS_ACTIVE ? LIME : GREEN;
        else if (GetTeam() == 2)
            return GetStatus() == ENTITY_STATUS_ACTIVE ? RED : MAROON;
        else
            return ORANGE;
    }

    if (GetTeam() == TEAM_PASSIVE)
        return WHITE;
    else if (pLocalPlayer->IsEnemy(this))
        return RED;
    else if (GetOwnerClientNumber() == pLocalPlayer->GetClientNumber())
        return LIME;
    else if (CanReceiveOrdersFrom(pLocalPlayer->GetClientNumber()))
        return CYAN;
    else
        return YELLOW;
}


/*====================
  IUnitEntity::AddSelectionRingToScene
  ====================*/
void    IUnitEntity::AddSelectionRingToScene()
{
    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return;

    float fSize(GetSelectionRadius());
    if (fSize <= 0.0f)
        return;

    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    uint uiGameTime(Game.GetGameTime());

    bool bFlash(m_uiOrderTime != INVALID_TIME && m_uiOrderTime <= uiGameTime && m_uiOrderTime + 1000 > uiGameTime && (uiGameTime - m_uiOrderTime) / 250 % 2 == 0);

    if (!Game.IsEntitySelected(m_uiIndex) && !Game.IsEntityHoverSelected(m_uiIndex) && !bFlash)
        return;

    CSceneEntity sceneEntity;
    sceneEntity.Clear();

    sceneEntity.width = fSize;
    sceneEntity.height = fSize;
    sceneEntity.scale = 1.0f;
    sceneEntity.SetPosition(m_v3Position);
    sceneEntity.hRes = s_hSelectionIndicator;
    sceneEntity.flags = SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

    if (GetFlying())
    {
        sceneEntity.objtype = OBJTYPE_BILLBOARD;
        sceneEntity.hSkin = BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT;
        sceneEntity.width *= 2.0f;
        sceneEntity.height *= 2.0f;
        sceneEntity.angle[PITCH] = -90.0f;
    }
    else
    {
        sceneEntity.objtype = OBJTYPE_GROUNDSPRITE;
    }

    sceneEntity.color = GetSelectionColor(pLocalPlayer);

    if (Game.IsEntityHoverSelected(m_uiIndex) && !Game.IsEntitySelected(m_uiIndex) && !bFlash)
    {
        sceneEntity.color[R] *= 0.5f;
        sceneEntity.color[G] *= 0.5f;
        sceneEntity.color[B] *= 0.5f;
    }

    SceneManager.AddEntity(sceneEntity);
}


/*====================
  AddDebugLine
  ====================*/
static void AddDebugLine(const CVec3f &v3Start, const CVec3f &v3End)
{
    CSceneEntity sceneEntity;

    sceneEntity.objtype = OBJTYPE_BEAM;
    sceneEntity.SetPosition(v3Start);
    sceneEntity.angle = v3End;
    sceneEntity.scale = 2.0f;
    sceneEntity.height = 1.0f;
    sceneEntity.color = BLUE;

    K2_WITH_GAME_RESOURCE_SCOPE()
        sceneEntity.hRes = g_ResourceManager.Register(_T("/core/materials/effect_solid.material"), RES_MATERIAL);

    SceneManager.AddEntity(sceneEntity);
}


/*====================
  IUnitEntity::AddToScene
  ====================*/
bool    IUnitEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
    //PROFILE("IUnitEntity::AddToScene");

    if (GetModel() == INVALID_RESOURCE)
        return false;

    if (GetStatus() == ENTITY_STATUS_DORMANT)
        return false;

    CPlayer *pLocalPlayer(Game.GetLocalPlayer());
    if (pLocalPlayer == nullptr)
        return false;

    if (!pLocalPlayer->CanSee(this))
        return false;

    CVec4f v4TintedColor(v4Color);
    if (GetStealthBits() != 0)
    {
        bool bVisible(HasVisibilityFlags(VIS_REVEALED(pLocalPlayer->GetTeam())) || GetTeam() == pLocalPlayer->GetTeam() || IsRevealed());
        float fVisibility(bVisible ? g_unitStealthFadeAmount : 1.0f);
        v4TintedColor[A] *= (1.0f - (m_fFade * fVisibility));
    }

    CVec3f v3Angles(m_v3UnitAngles);
    v3Angles[PITCH] += m_fTiltPitch;
    v3Angles[ROLL] += m_fTiltRoll;

    if (m_v3AxisAngles != v3Angles)
    {
        m_aAxis.Set(v3Angles);
        m_v3AxisAngles = v3Angles;
    }

    static CSceneEntity sceneEntity;
    bool bPositioned(false);

    sceneEntity.Clear();

    if (m_pMount != nullptr)
    {
        m_pMount->UpdateSkeleton(true);

        CSkeleton *pSkeleton(m_pMount->GetSkeleton());
        uint uiBone(pSkeleton->GetBone(_CTS("_bone_hero")));
        if (uiBone != INVALID_BONE)
        {
            SBoneState *pBone(pSkeleton->GetBoneState(uiBone));
            
            CMatrix4x3<float> tmMount(m_pMount->GetAxis(), m_pMount->GetPosition());
            CMatrix4x3<float> tmBone(CAxis_cast(pBone->tm_local.axis), CVec3_cast(pBone->tm_local.pos) * m_pMount->GetBaseScale() * m_pMount->GetScale());
            CMatrix4x3<float> tmWorld(tmMount * tmBone);

            tmWorld.GetAxis().Forward().Normalize();
            tmWorld.GetAxis().Right().Normalize();
            tmWorld.GetAxis().Up().Normalize();

            sceneEntity.SetPosition(tmWorld.GetPosition());
            sceneEntity.axis = tmWorld.GetAxis();

            bPositioned = true;
        }
    }

    if (!bPositioned)
    {
        sceneEntity.SetPosition(m_v3Position);
        sceneEntity.axis = m_aAxis;
    }

    sceneEntity.scale = GetBaseScale() * GetScale();
    sceneEntity.objtype = OBJTYPE_MODEL;
    sceneEntity.hRes = GetModel();
    sceneEntity.hSkin = g_ResourceManager.GetSkin(sceneEntity.hRes, GetSkin());
    sceneEntity.skeleton = m_pSkeleton;
    sceneEntity.color = v4TintedColor;
    sceneEntity.teamcolor = GetTeamColor(pLocalPlayer);
    sceneEntity.flags = iFlags | SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

    CModel *pModel(g_ResourceManager.GetModel(sceneEntity.hRes));
    if (pModel)
    {
        CBBoxf bbBounds(pModel->GetBounds());
        bbBounds.Transform(m_v3Position, m_aAxis, sceneEntity.scale);

        sceneEntity.bounds = bbBounds;
        sceneEntity.flags |= SCENEENT_USE_BOUNDS;
    }

    if (IsHighlighted())
        sceneEntity.color *= m_v4HighlightColor;

    if (m_uiClientRenderFlags & ECRF_HALFTRANSPARENT)
        sceneEntity.color[A] *= 0.5f;

    SSceneEntityEntry &cEntry(SceneManager.AddEntity(sceneEntity));

    if (!cEntry.bCull || !cEntry.bCullShadow)
    {
        AddSelectionRingToScene();
        UpdateSkeleton(true);

        if (d_drawUnitBounds)
        {
            CBBoxf bbBoundsWorld(GetBounds());
            bbBoundsWorld.Offset(m_v3Position);
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
  IUnitEntity::GetHeading
  ====================*/
CVec2f  IUnitEntity::GetHeading(float fVecLength)
{
    CVec3f v3Yaw(0,0,m_v3Angles.z);

    return (CVec2f(M_GetForwardVecFromAngles(v3Yaw) * fVecLength));
}


/*====================
  IUnitEntity::IsTargetType
  ====================*/
bool    IUnitEntity::IsTargetType(const tstring &sType, const IUnitEntity *pInitiator) const
{
    PROFILE("IUnitEntity::IsTargetType");

    // Check global conditions
    EGlobalCondition eGlobal(GetGlobalConditionFromString(sType));
    if (eGlobal != INVALID_GLOBAL_CONDITION)
        return IsGlobalCondition(eGlobal);
        
    // Check attributes
    EAttribute eAttribute(GetAttributeFromString(sType));
    if (eAttribute != ATTRIBUTE_INVALID)
        return IsAttribute(eAttribute);

    if (!pInitiator)
        return false;

    // Check traits
    ETargetTrait eTrait(GetTargetTraitFromString(sType));
    if (eTrait != INVALID_TARGET_TYPE)
        return IsTargetType(eTrait, pInitiator);

    // Check attack type
    uint uiAttackType(Game.LookupAttackType(sType));
    if (uiAttackType != INVALID_ATTACK_TYPE)
        return GetAttackType() == uiAttackType;

    // Check entity type
    ushort unType(EntityRegistry.LookupID(sType));
    if (unType != INVALID_ENT_TYPE)
        return GetType() == unType;

    // Check unit type
    const tsvector &vsUnitType(GetUnitType());
    if (!vsUnitType.empty())
    {
        for (tsvector_cit cit(vsUnitType.begin()), citEnd(vsUnitType.end()); cit != citEnd; ++cit)
        {
            if (CompareNoCase(sType, *cit) == 0)
                return true;
        }
    }

    return false;
}

bool    IUnitEntity::IsTargetType(const CTargetScheme::STestRecord &test, const IUnitEntity *pInitiator) const
{
    PROFILE("IUnitEntity::IsTargetType");

    if (pInitiator == nullptr)
        return false;

    switch (test.m_eTest)
    {
    case CTargetScheme::TARGET_SCHEME_TEST_GLOBAL:
        return IsGlobalCondition(test.m_eGlobal);
    case CTargetScheme::TARGET_SCHEME_TEST_NOT_GLOBAL:
        return !IsGlobalCondition(test.m_eGlobal);
    case CTargetScheme::TARGET_SCHEME_TEST_TRAIT:
        return IsTargetType(test.m_eTrait, pInitiator);
    case CTargetScheme::TARGET_SCHEME_TEST_NOT_TRAIT:
        return !IsTargetType(test.m_eTrait, pInitiator);
    case CTargetScheme::TARGET_SCHEME_TEST_ATTACK:
        return GetAttackType() == test.m_uiValue;
    case CTargetScheme::TARGET_SCHEME_TEST_NOT_ATTACK:
        return GetAttackType() != test.m_uiValue;
    case CTargetScheme::TARGET_SCHEME_TEST_STRING:
        return IsTargetType(test.m_sString, pInitiator);
    case CTargetScheme::TARGET_SCHEME_TEST_NOT_STRING:
        return !IsTargetType(test.m_sString, pInitiator);
    case CTargetScheme::TARGET_SCHEME_TEST_ATTRIBUTE:
        return IsAttribute(test.m_eAttribute);
    case CTargetScheme::TARGET_SCHEME_TEST_NOT_ATTRIBUTE:
        return !IsAttribute(test.m_eAttribute);
    }

    return false;
}

bool    IUnitEntity::IsTargetType(ETargetTrait eTrait, const IUnitEntity *pInitiator) const
{
    PROFILE("IUnitEntity::IsTargetType");

    if (pInitiator == nullptr)
        return false;

    switch (eTrait)
    {
    case TARGET_TRAIT_ALL:
        return true;
    case TARGET_TRAIT_NONE:
        return false;
    case TARGET_TRAIT_SELF:
        return pInitiator == this;
    case TARGET_TRAIT_OTHER:
        return pInitiator != this;
    case TARGET_TRAIT_ALLY:
        return GetTeam() == pInitiator->GetTeam();
    case TARGET_TRAIT_ENEMY:
        return GetTeam() != pInitiator->GetTeam();
    case TARGET_TRAIT_NEUTRAL:
        return GetTeam() == TEAM_NEUTRAL;
    case TARGET_TRAIT_PASSIVE:
        return GetTeam() == TEAM_PASSIVE;
    case TARGET_TRAIT_FRIENDLY:
        return GetTeam() == pInitiator->GetTeam() && pInitiator != this;
    case TARGET_TRAIT_ALIVE:
        return GetStatus() == ENTITY_STATUS_ACTIVE;
    case TARGET_TRAIT_CORPSE:
        return GetStatus() == ENTITY_STATUS_CORPSE;
    case TARGET_TRAIT_DEAD:
        return GetStatus() == ENTITY_STATUS_DEAD;
    case TARGET_TRAIT_UNIT:
        return IsUnit();
    case TARGET_TRAIT_HERO:
        return IsHero();
    case TARGET_TRAIT_CREEP:
        return IsCreep() || IsNeutral();
    case TARGET_TRAIT_BUILDING:
        return IsBuilding();
    case TARGET_TRAIT_PET:
        return IsPet();
    case TARGET_TRAIT_GADGET:
        return IsGadget();
    case TARGET_TRAIT_ILLUSION:
        return IsIllusion();
    case TARGET_TRAIT_POWERUP:
        return IsPowerup();
    case TARGET_TRAIT_CHEST:
        return IsType(Entity_Chest);
    case TARGET_TRAIT_TREE:
        return IsType(Prop_Tree);
    case TARGET_TRAIT_MINE:
        return GetOwnerClientNumber() == pInitiator->GetOwnerClientNumber();
    case TARGET_TRAIT_PLAYER_CONTROLLED:
        return GetOwnerClientNumber() != -1;
    case TARGET_TRAIT_OWNER:
        return pInitiator->GetOwner() == this;
    case TARGET_TRAIT_OWNED:
        return GetOwner() == pInitiator;
    case TARGET_TRAIT_DENIABLE:
        if (GetTeam() != pInitiator->GetTeam())
            return false;
        if (IsCreep() && GetHealthPercent() < g_denyCreepHealthPercent)
            return true;
        if (IsHero())
        {
            if(GetHealthPercent() < g_denyHeroHealthPercent && GetDeniable())
                return true;
            return false;
        }
        if (IsBuilding() && !GetAsBuilding()->GetIsBase() && GetHealthPercent() < g_denyBuildingHealthPercent)
            return true;
        if (GetDeniable())
            return true;
        if (GetHealthPercent() < GetDeniablePercent())
            return true;
        return false;
    case TARGET_TRAIT_SMACKABLE:
        return GetSmackable();
    case TARGET_TRAIT_NOHELP:
        {
            CPlayer *pSourcePlayer(pInitiator->GetOwnerPlayer());
            CPlayer *pTargetPlayer(GetOwnerPlayer());
            if (pSourcePlayer == nullptr || pTargetPlayer == nullptr)
                return false;

            return pTargetPlayer->GetNoHelp(pSourcePlayer);
        }
    case TARGET_TRAIT_VISIBLE:
        return pInitiator->CanSee(this);
    case TARGET_TRAIT_FULL:
        return GetHealthPercent() == 1.0f && GetManaPercent() == 1.0f;
    case TARGET_TRAIT_PERKS:
        {
            CPlayer *pPlayer(GetOwnerPlayer());

            if (pPlayer != nullptr && (pPlayer->HasFlags(PLAYER_FLAG_PREMIUM) || pPlayer->HasFlags(PLAYER_FLAG_STAFF)))
                return true;
            else 
                return g_perks;
        }
    case TARGET_TRAIT_IMMOBILIZED:
        return IsImmobilized();
    case TARGET_TRAIT_RESTRAINED:
        return IsRestrained();
    case TARGET_TRAIT_DISARMED:
        return IsDisarmed();
    case TARGET_TRAIT_SILENCED:
        return IsSilenced();
    case TARGET_TRAIT_PERPLEXED:
        return IsPerplexed();
    case TARGET_TRAIT_STUNNED:
        return IsStunned();
    case TARGET_TRAIT_STEALTH:
        return IsStealth() && !HasVisibilityFlags(VIS_REVEALED(pInitiator->GetTeam()));
    case TARGET_TRAIT_MOVING:
        return m_cBrain.GetMoving();
    case TARGET_TRAIT_IDLE:
        return IsIdle();
    case TARGET_TRAIT_ATTACKING:
        return m_cBrain.GetActionState(ASID_ATTACKING)->IsActive();
    case TARGET_TRAIT_CASTING:
        return m_cBrain.GetActionState(ASID_CASTING)->IsActive();
    case TARGET_TRAIT_MANAPOOL:
        return GetMaxMana() > 0.0f;
    case TARGET_TRAIT_DELETED:
        return HasLocalFlags(ENT_LOCAL_DELETE_NEXT_FRAME);
    }

    return false;
}


/*====================
  IUnitEntity::IsGlobalCondition
  ====================*/
bool    IUnitEntity::IsGlobalCondition(EGlobalCondition eGlobal) const
{
    switch (eGlobal)
    {
    case GLOBAL_CONDITION_DAY:
        return !Game.IsNight();
    case GLOBAL_CONDITION_NIGHT:
        return Game.IsNight();
    }

    return false;
}


/*====================
  IUnitEntity::SpawnIllusion
  ====================*/
IUnitEntity*    IUnitEntity::SpawnIllusion(const CVec3f &v3Position, const CVec3f &v3Angles, uint uiLifetime, 
                                           float fReceiveDamageMultiplier, float fInflictDamageMultiplier, 
                                           ResHandle hSpawnEffect, ResHandle hDeathEffect, 
                                           bool bDeathAnim, bool bInheritActions)
{
    IGameEntity *pNewEntity(Game.AllocateEntity(GetType()));
    if (pNewEntity == nullptr)
    {
        Console.Err << _T("Failed to spawn illusion: ") << GetTypeName() << newl;
        return nullptr;
    }

    IUnitEntity *pIllusion(pNewEntity->GetAsUnit());
    if (pIllusion == nullptr)
    {
        Console.Err << _T("Entity is not a unit: ") << GetTypeName() << newl;
        Game.DeleteEntity(pNewEntity);
        return nullptr;
    }

    pIllusion->SetPosition(v3Position);
    pIllusion->SetAngles(v3Angles);
    pIllusion->SetTeam(GetTeam());
    pIllusion->SetOwnerClientNumber(GetOwnerClientNumber());
    pIllusion->SetUnitFlags(UNIT_FLAG_ILLUSION);
    pIllusion->SetReceiveDamageMultiplier(fReceiveDamageMultiplier);
    pIllusion->SetInflictDamageMultiplier(fInflictDamageMultiplier);
    pIllusion->SetDeathEffect(hDeathEffect);
    pIllusion->SetIllusionDeathAnim(bDeathAnim);
    pIllusion->SetPersistentModifierKeys(GetPersistentModifierKeys());
    pIllusion->Spawn();
    pIllusion->SetLifetime(Game.GetGameTime(), uiLifetime);
    pIllusion->ValidatePosition(TRACE_UNIT_SPAWN);

    pIllusion->SetLevel(GetLevel());

    pIllusion->m_uiDisjointSequence = m_uiDisjointSequence;
    pIllusion->m_uiOrderDisjointSequence = m_uiOrderDisjointSequence;
    pIllusion->m_uiArmingSequence = m_uiArmingSequence;

    // Set ability levels and clone the current active modifier key (not default)
    for (int iSlot(INVENTORY_START_ABILITIES); iSlot <= INVENTORY_END_ABILITIES; ++iSlot)
    {
        ISlaveEntity* pThisAbility(GetInventorySlot(iSlot));
        ISlaveEntity* pIllusionAbility(pIllusion->GetInventorySlot(iSlot));

        if (pIllusionAbility == nullptr || pThisAbility == nullptr)
            continue;
        
        pIllusionAbility->SetLevel(pThisAbility->GetLevel());
        pIllusionAbility->SetActiveModifierKey(pThisAbility->GetActiveModifierKey());
    }

    // Clone backpack
    for (int iSlot(INVENTORY_START_BACKPACK); iSlot <= INVENTORY_END_BACKPACK; ++iSlot)
    {
        if (GetItem(iSlot) == nullptr)
            continue;

        pIllusion->CloneItem(GetItem(iSlot));
    }

    // Clone states
    for (int iSlot(INVENTORY_START_STATES); iSlot <= INVENTORY_END_STATES; ++iSlot)
    {
        if (GetState(iSlot) == nullptr)
            continue;

        IEntityState *pState(GetState(iSlot));

        if (!pState->GetPropagateToIllusions())
            continue;

        IEntityState *pNewState(pIllusion->ApplyState(pState->GetType(), pState->GetLevel(), pState->GetStartTime(), pState->GetLifetime(), pState->GetInflictorIndex(), pState->GetProxyUID()));
        if (pNewState != nullptr)
        {
            pNewState->SetCharges(pState->GetCharges());
        }
    }

    // Clone actions / animations
    if (bInheritActions)
    {
        pIllusion->m_cBrain.CopyFrom(m_cBrain);

        for (int i = 0; i < NUM_ANIM_CHANNELS; ++i)
        {
            pIllusion->m_asAnim[i] = m_asAnim[i];
            pIllusion->m_ayAnim[i] = m_ayAnim[i];
            pIllusion->m_ayAnimSequence[i] = m_ayAnimSequence[i];
            pIllusion->m_afAnimSpeed[i] = m_afAnimSpeed[i];
        }
    }

    pIllusion->m_fCurrentMaxHealth = m_fCurrentMaxHealth;
    pIllusion->m_fCurrentMaxMana = m_fCurrentMaxMana;

    pIllusion->SetHealth(GetHealth());
    pIllusion->SetMana(GetMana());

    if (hSpawnEffect != INVALID_RESOURCE)
    {
        CGameEvent ev;
        ev.SetSourceEntity(pIllusion->GetIndex());
        ev.SetEffect(hSpawnEffect);
        Game.AddEvent(ev);
    }
    
    return pIllusion;
}


/*====================
  IUnitEntity::GetRemainingLifetime
  ====================*/
uint    IUnitEntity::GetRemainingLifetime() const
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return 0;

    if (GetActualLifetime() == 0 || m_uiSpawnTime == INVALID_TIME)
        return INVALID_TIME;

    return GetActualLifetime() - (Game.GetGameTime() - m_uiSpawnTime);
}


/*====================
  IUnitEntity::Interpolate
  ====================*/
void    IUnitEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
    IVisualEntity::Interpolate(fLerp, pPrevState, pNextState);

    if (pPrevState->GetNoInterpolateSequence() != pNextState->GetNoInterpolateSequence())
        return;

    IUnitEntity *pPrev(static_cast<IUnitEntity *>(pPrevState));
    IUnitEntity *pNext(static_cast<IUnitEntity *>(pNextState));

    m_v3Angles = m_v3UnitAngles = M_LerpAngles(fLerp, pPrev->m_v3UnitAngles, pNext->m_v3UnitAngles);

    // Health
    if (pPrev->m_fCurrentMaxHealth > 0.0f && pNext->m_fCurrentMaxHealth > 0.0f)
    {
        float fPrevHealthFrac(pPrev->m_fHealth / pPrev->m_fCurrentMaxHealth);
        float fNextHealthFrac(pNext->m_fHealth / pNext->m_fCurrentMaxHealth);

        if (m_uiHealthShadowTime != INVALID_TIME)
        {
            if (Host.GetTime() >= m_uiHealthShadowTime)
                m_fHealthShadowMarker -= cg_healthShadowDecay * MsToSec(Host.GetFrameLength());

            if (m_fHealthShadowMarker <= fNextHealthFrac)
            {
                m_uiHealthShadowTime = INVALID_TIME;
                m_fHealthShadowMarker = 0.0f;
            }
        }
        else
        {
            if (fPrevHealthFrac > fNextHealthFrac)
            {
                m_uiHealthShadowTime = Host.GetTime() + cg_healthShadowDelay;
                m_fHealthShadowMarker = fPrevHealthFrac;
            }
        }

        m_fHealth = LERP(fLerp, fPrevHealthFrac, fNextHealthFrac) * m_fCurrentMaxHealth;
    }
    else
    {
        m_fHealth = pNext->m_fHealth;
    }

    // Mana
    if (pPrev->m_fCurrentMaxMana > 0.0f && pNext->m_fCurrentMaxMana > 0.0f)
    {
        float fPrevManaFrac(pPrev->m_fMana / pPrev->m_fCurrentMaxMana);
        float fNextManaFrac(pNext->m_fMana / pNext->m_fCurrentMaxMana);

        if (m_uiManaShadowTime != INVALID_TIME)
        {
            if (Host.GetTime() >= m_uiManaShadowTime)
                m_fManaShadowMarker -= cg_manaShadowDecay * MsToSec(Host.GetFrameLength());

            if (m_fManaShadowMarker <= fNextManaFrac)
            {
                m_uiManaShadowTime = INVALID_TIME;
                m_fManaShadowMarker = 0.0f;
            }
        }
        else
        {
            if (fPrevManaFrac > fNextManaFrac)
            {
                m_uiManaShadowTime = Host.GetTime() + cg_manaShadowDelay;
                m_fManaShadowMarker = fPrevManaFrac;
            }
        }

        m_fMana = LERP(fLerp, fPrevManaFrac, fNextManaFrac) * m_fCurrentMaxMana;
    }
    else
    {
        m_fMana = pNext->m_fMana;
    }
        
    m_fFade = LERP(fLerp, pPrev->m_fFade, pNext->m_fFade);

    bool bStealth(pPrev->HasUnitFlags(UNIT_FLAG_STEALTH) || pNext->HasUnitFlags(UNIT_FLAG_STEALTH));
    if (bStealth)
        SetUnitFlags(UNIT_FLAG_STEALTH);
    else
        RemoveUnitFlags(UNIT_FLAG_STEALTH);

    bool bRevealed(pPrev->HasUnitFlags(UNIT_FLAG_REVEALED) && pNext->HasUnitFlags(UNIT_FLAG_REVEALED));
    if (bRevealed)
        SetUnitFlags(UNIT_FLAG_REVEALED);
    else
        RemoveUnitFlags(UNIT_FLAG_REVEALED);
}


/*====================
  IUnitEntity::GetActivePath
  ====================*/
PoolHandle  IUnitEntity::GetActivePath()
{
    IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
    if (pBehavior)
        return pBehavior->GetPath();
    else
        return INVALID_POOL_HANDLE;
}


/*====================
  IUnitEntity::GetThreatLevel
  ====================*/
float   IUnitEntity::GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget)
{
    if (pOther->IsGadget())
        return -10.0f;

    float fDistance(Distance(GetPosition().xy(), pOther->GetPosition().xy()) - GetBounds().GetDim(X) * DIAG - pOther->GetBounds().GetDim(X) * DIAG);

    fDistance = MAX(fDistance, 0.0f);

    float fThreatLevel(-FAR_AWAY);

    if (GetAggroRange() > 0.0f && fDistance <= GetAggroRange())
    {
        float fAggroThreat(Game.GetAggroPriority(GetCombatTypeIndex(), pOther->GetCombatTypeIndex()));

        fAggroThreat -= fDistance / GetAggroRange(); // Range is a tie breaker by 1 point

        fThreatLevel = MAX(fThreatLevel, fAggroThreat);
    }

    if (GetAttackRange() > 0.0f && bCurrentTarget && fDistance <= GetAttackRange())
    {
        float fTargetThreat(Game.GetTargetPriority(GetCombatTypeIndex(), pOther->GetCombatTypeIndex()));

        fThreatLevel = MAX(fThreatLevel, fTargetThreat);
    }
    
    if (GetAttackRange() > 0.0f && fDistance <= GetAttackRange())
    {
        float fAttackThreat(Game.GetAttackPriority(GetCombatTypeIndex(), pOther->GetCombatTypeIndex()));

        fAttackThreat -= fDistance / GetAttackRange(); // Range is a tie breaker by 1 point

        fThreatLevel = MAX(fThreatLevel, fAttackThreat);
    }

    if (GetProximityRange() > 0.0f && fDistance <= GetProximityRange())
    {
        float fProximityThreat(Game.GetProximityPriority(GetCombatTypeIndex(), pOther->GetCombatTypeIndex()));

        fProximityThreat -= fDistance / GetProximityRange(); // Range is a tie breaker by 1 point

        fThreatLevel = MAX(fThreatLevel, fProximityThreat);
    }

    fThreatLevel = ExecuteActionScript(ACTION_SCRIPT_GET_THREAT_LEVEL, pOther, pOther->GetPosition(), fThreatLevel);

    return fThreatLevel;
}


/*====================
  IUnitEntity::IsAttacking
  ====================*/
bool    IUnitEntity::IsAttacking() const
{
    return (m_cBrain.GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE) == ASR_ACTIVE;
}


/*====================
  IUnitEntity::IsIdle
  ====================*/
bool    IUnitEntity::IsIdle() const
{
    IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
    if (pBehavior != nullptr)
        return pBehavior->IsIdle();
    else
        return false;
}


/*====================
  IUnitEntity::IsTraveling
  ====================*/
bool    IUnitEntity::IsTraveling() const
{
    IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
    if (pBehavior != nullptr)
        return pBehavior->IsTraveling();
    else
        return false;
}


/*====================
  IUnitEntity::CanReceiveOrdersFrom
  ====================*/
bool    IUnitEntity::CanReceiveOrdersFrom(int iClientNumber)
{
    if (!GetIsControllable())
        return false;

    if (iClientNumber == -1)
        return false;

    if (GetOwnerClientNumber() == iClientNumber)
        return true;

    CPlayer *pOwner(GetOwnerPlayer());
    if (pOwner == nullptr)
        return false;

    CPlayer *pPlayer(Game.GetPlayer(iClientNumber));
    uint uiTeam(pPlayer ? pPlayer->GetTeam() : TEAM_INVALID);

    if ((pOwner->IsDisconnected() || pOwner->HasFlags(PLAYER_FLAG_LOADING)) && pOwner->GetTeam() == uiTeam)
        return true;

    if (pOwner->HasSharedFullControl(iClientNumber))
        return true;

    if (GetPartialControlShare() && pOwner->HasSharedPartialControl(iClientNumber))
        return true;

    return false;
}


/*====================
  IUnitEntity::CheckRecipes
  ====================*/
int     IUnitEntity::CheckRecipes(int iTestSlot)
{
    static vector<IEntityItem*> s_vComponentItems;

    int iNewItemSlot(iTestSlot);

    // Normal recipes
    bool bInStash(iTestSlot >= INVENTORY_START_STASH && iTestSlot <= INVENTORY_STASH_PROVISIONAL);
    int iStartSlot(bInStash ? INVENTORY_START_STASH : INVENTORY_START_BACKPACK);
    int iEndSlot(bInStash ? INVENTORY_STASH_PROVISIONAL : INVENTORY_BACKPACK_PROVISIONAL);

    for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
    {
        IEntityItem *pItem(GetItem(iSlot));
        if (pItem == nullptr)
            continue;

        int iRecipeItemSlot(pItem->Assemble());
        if (iRecipeItemSlot != -1)
            iNewItemSlot = iRecipeItemSlot;
    }

    IEntityItem *pTestSlot(GetItem(iTestSlot));
    int iSlotOwner(pTestSlot != nullptr ? pTestSlot->GetPurchaserClientNumber() : -1);

    if (iSlotOwner != -1 && !CanReceiveOrdersFrom(iSlotOwner))
        return iNewItemSlot;

    // Auto recipes
    const vector<ushort> &vAutoRecipes(Game.GetAutoRecipeList());
    for (vector<ushort>::const_iterator cit(vAutoRecipes.begin()), citEnd(vAutoRecipes.end()); cit != citEnd; ++cit)
    {
        // Get current auto recipe item definition
        ushort unItemID(*cit);
        CItemDefinition *pItemDefinition(EntityRegistry.GetDefinition<CItemDefinition>(unItemID));
        if (pItemDefinition == nullptr || pItemDefinition->GetComponentsSize() == 0)
            continue;

        uint uiRecipe(pItemDefinition->Assemble(this, iTestSlot));
        if (uiRecipe == -1)
            continue;

        // Find a slot in the inventory
        int iTargetSlot(-1);
        for (int iSlot(iStartSlot); iSlot <= iEndSlot; ++iSlot)
        {
            if (iSlot == INVENTORY_BACKPACK_PROVISIONAL || iSlot == INVENTORY_STASH_PROVISIONAL)
                continue;

            IEntityItem *pItem(GetItem(iSlot));
            if (pItem == nullptr)
            {
                if (iTargetSlot == -1)
                    iTargetSlot = iSlot;
                continue;
            }

            if (pItem->CanStack(unItemID, GetOwnerClientNumber()))
            {
                iTargetSlot = iSlot;
                break;
            }
        }

        PlayRecipeEffect();

        IEntityTool *pTool(GiveItem(iTargetSlot, unItemID, true));
        if (pTool != nullptr && pTool->IsItem())
        {
            pTool->GetAsItem()->SetPurchaserClientNumber(iSlotOwner);
            Game.LogItem(GAME_LOG_ITEM_ASSEMBLE, pTool->GetAsItem());
        }

        pTool->GetAsItem()->SetRecipeVariation(uiRecipe);

        iNewItemSlot = iTargetSlot;
        
        int iRecipeItemSlot(CheckRecipes(iTargetSlot));
        if (iRecipeItemSlot != -1)
            iNewItemSlot = iRecipeItemSlot;
    }

    return iNewItemSlot;
}


/*====================
  IUnitEntity::CanAccessLocalShop
  ====================*/
bool    IUnitEntity::CanAccessLocalShop(const tstring &sShopName)
{
    if (!GetCanCarryItems() || GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    // Check for locally accessible shops
    const tsvector &vsShopAccess(TokenizeString(GetShopAccess(), _T(' ')));
    for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
    {
        if (*it == sShopName)
            return true;
    }

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != nullptr)
    {
        IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            const tsvector &vsShopAccess(TokenizeString(pBase->GetSharedShopAccess(), _T(' ')));
            for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
            {
                if (*it == sShopName)
                    return true;
            }
        }
    }

    return false;
}


/*====================
  IUnitEntity::CanAccessShop
  ====================*/
bool    IUnitEntity::CanAccessShop(ushort unShopID)
{
    if (!GetCanCarryItems())
        return false;

    // Check for locally accessible shops
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        const tsvector &vsShopAccess(TokenizeString(GetShopAccess(), _T(' ')));
        const tstring &sShopName(EntityRegistry.LookupName(unShopID));
        for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
        {
            if (*it == sShopName)
                return true;
        }
    }

    // Check for locally accessible shops (shared)
    if (GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam != nullptr)
        {
            IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
            if (pBase != nullptr)
            {
                const tsvector &vsShopAccess(TokenizeString(pBase->GetSharedShopAccess(), _T(' ')));
                const tstring &sShopName(EntityRegistry.LookupName(unShopID));
                for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
                {
                    if (*it == sShopName)
                        return true;
                }
            }
        }
    }

    // Check remotely accessible shops (old style)
    vector<ushort> vShopList;
    EntityRegistry.GetShopList(vShopList);
    for (vector<ushort>::iterator it(vShopList.begin()); it != vShopList.end(); ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr || !pShop->GetAllowRemoteAccess())
            continue;

        if (pShop->GetTypeID() == unShopID)
            return true;
    }

    // Check for remotely accessible shops
    const tsvector &vsShopAccess(TokenizeString(GetRemoteShopAccess(), _T(' ')));
    const tstring &sShopName(EntityRegistry.LookupName(unShopID));
    for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
    {
        if (*it == sShopName)
            return true;
    }

    // Check for remotely accessible shops (shared)
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != nullptr)
    {
        IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            const tsvector &vsShopAccess(TokenizeString(pBase->GetSharedRemoteShopAccess(), _T(' ')));
            const tstring &sShopName(EntityRegistry.LookupName(unShopID));
            for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
            {
                if (*it == sShopName)
                    return true;
            }
        }
    }

    return false;
}


/*====================
  IUnitEntity::CanAccessItem

  Determines if the this unit can access a certain item
  ====================*/
bool    IUnitEntity::CanAccessItem(const tstring &sItem)
{
    if (!GetCanCarryItems())
        return false;

    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));

    if (pTeam == nullptr)
        return false;

    CShopInfo *pShop(pTeam->GetShopInfo());

    if (pShop == nullptr)
        return false;

    if (pShop->GetStockRemaining(sItem) == 0)
        return false;

    const tstring &sRestrictItemAccess(GetRestrictItemAccess());

    if (!sRestrictItemAccess.empty())
    {
        const tsvector &vsRestrictItemAccess(TokenizeString(sRestrictItemAccess, _T(' ')));
        tsvector_cit it(vsRestrictItemAccess.begin()), itEnd(vsRestrictItemAccess.end());
        while (it != itEnd)
        {
            if (*it == sItem)
                break;

            ++it;
        }

        if (it == itEnd)
            return false;
    }

    CItemDefinition *pItem(EntityRegistry.GetDefinition<CItemDefinition>(sItem));
    if (pItem != nullptr)
    {
        if (pItem->IsRecipe() && pItem->GetAutoAssemble())
            return true;
    }

    set<CShopDefinition*> setShops;
    GetAccessableShopList(setShops);

    for (set<CShopDefinition*>::iterator itShop(setShops.begin()); itShop != setShops.end(); ++itShop)
    {
        const tsvector &vsItems((*itShop)->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
        {
            if (*cit == sItem)
                return true;
        }
    }

    return false;
}


/*====================
  IUnitEntity::CanAccessItemLocal

  Determines if the this unit can access a certain item
  ====================*/
bool    IUnitEntity::CanAccessItemLocal(const tstring &sItem)
{
    if (!GetCanCarryItems())
        return false;

    set<CShopDefinition*> setShops;
    GetLocalShopList(setShops);

    for (set<CShopDefinition*>::iterator itShop(setShops.begin()); itShop != setShops.end(); ++itShop)
    {
        const tsvector &vsItems((*itShop)->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
        {
            if (*cit == sItem)
                return true;
        }
    }

    return false;
}


/*====================
  IUnitEntity::GetAccessableShopList
  ====================*/
void    IUnitEntity::GetAccessableShopList(set<CShopDefinition*> &setShops)
{
    GetLocalShopList(setShops);
    GetRemoteShopList(setShops);
}


/*====================
  IUnitEntity::GetAccessableShop
  ====================*/
bool    IUnitEntity::GetAccessableShop(const tstring &sItem, ushort &unShop, int &iSlot)
{
    unShop = INVALID_ENT_TYPE;
    iSlot = -1;

    if (!GetCanCarryItems())
        return false;

    set<CShopDefinition*> setShops;
    GetAccessableShopList(setShops);

    for (set<CShopDefinition*>::iterator itShop(setShops.begin()); itShop != setShops.end(); ++itShop)
    {
        const tsvector &vsItems((*itShop)->GetItems());
        for (tsvector_cit cit(vsItems.begin()), citEnd(vsItems.end()); cit != citEnd; ++cit)
        {
            if (*cit == sItem)
            {
                unShop = (*itShop)->GetTypeID();
                iSlot = cit - vsItems.begin();
                return true;
            }
        }
    }

    return false;
}


/*====================
  IUnitEntity::GetLocalShopList
  ====================*/
void    IUnitEntity::GetLocalShopList(set<CShopDefinition*> &setShops)
{
    // Check for locally accessible shops
    const tsvector &vsShopAccess(TokenizeString(GetShopAccess(), _T(' ')));
    for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr)
            continue;

        setShops.insert(pShop);
    }

    // Check for locally accessible shops (shared)
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != nullptr)
    {
        IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            const tsvector &vsShopAccess(TokenizeString(pBase->GetSharedShopAccess(), _T(' ')));
            for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
            {
                CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
                if (pShop == nullptr)
                    continue;

                setShops.insert(pShop);
            }
        }
    }
}


/*====================
  IUnitEntity::GetRemoteShopList
  ====================*/
void    IUnitEntity::GetRemoteShopList(set<CShopDefinition*> &setShops)
{
    // Add remotely accessible shops (old style)
    vector<ushort> vShopList;
    EntityRegistry.GetShopList(vShopList);
    for (vector<ushort>::iterator it(vShopList.begin()); it != vShopList.end(); ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr || !pShop->GetAllowRemoteAccess())
            continue;

        setShops.insert(pShop);
    }

    // Check for remotely accessible shops
    const tsvector &vsShopAccess(TokenizeString(GetRemoteShopAccess(), _T(' ')));
    for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
    {
        CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
        if (pShop == nullptr)
            continue;

        setShops.insert(pShop);
    }

    // Check for remotely accessible shops (shared)
    CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
    if (pTeam != nullptr)
    {
        IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
        if (pBase != nullptr)
        {
            const tsvector &vsShopAccess(TokenizeString(pBase->GetSharedRemoteShopAccess(), _T(' ')));
            for (tsvector_cit it(vsShopAccess.begin()), itEnd(vsShopAccess.end()); it != itEnd; ++it)
            {
                CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(*it));
                if (pShop == nullptr)
                    continue;

                setShops.insert(pShop);
            }
        }
    }
}


/*====================
  IUnitEntity::SetLastAggression
  ====================*/
void    IUnitEntity::SetLastAggression(int iTeam, uint uiTime)
{
    if (iTeam == 1)
        m_auiLastAggression[0] = uiTime;
    else if (iTeam == 2)
        m_auiLastAggression[1] = uiTime;
}


/*====================
  IUnitEntity::GetLastAggression
  ====================*/
uint    IUnitEntity::GetLastAggression(int iTeam) const
{
    if (iTeam == 1)
        return m_auiLastAggression[0];
    else if (iTeam == 2)
        return m_auiLastAggression[1];
    else
        return INVALID_TIME;
}


/*====================
  IUnitEntity::CallForHelp
  ====================*/
void    IUnitEntity::CallForHelp(float fRange, IUnitEntity *pAttacker)
{
    static uivector vEntities;
    float fAssistRange(fRange + m_bbBounds.GetDim(X) * DIAG);
    CBBoxf bbRegion(CVec3f(m_v3Position.xy() - CVec2f(fAssistRange), -FAR_AWAY),  CVec3f(m_v3Position.xy() + CVec2f(fAssistRange), FAR_AWAY));

    // Fetch
    Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_ACTIVE_UNIT);

    for (uivector_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        if (*cit == m_uiWorldIndex)
            continue;

        IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*cit)));
        if (pUnit == nullptr)
            continue;

        if (DistanceSq(m_v3Position.xy(), pUnit->GetPosition().xy()) > SQR(fAssistRange + pUnit->GetBounds().GetDim(X) * DIAG))
            continue;

        if (pUnit->GetTeam() == GetTeam())
            pUnit->Assist(this, pAttacker);
    }
}


/*====================
  IUnitEntity::Assist
  ====================*/
void    IUnitEntity::Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker)
{
    m_cBrain.Assist(pAlly, pAttacker);
}


/*====================
  IUnitEntity::UpdateEffectThreadSource
  ====================*/
void    IUnitEntity::UpdateEffectThreadSource(CEffectThread *pEffectThread)
{
    pEffectThread->SetSourceModel(g_ResourceManager.GetModel(GetModel()));
    pEffectThread->SetSourceSkeleton(m_pSkeleton);

    bool bPositioned(false);

    if (m_pMount != nullptr)
    {
        m_pMount->UpdateSkeleton(true);

        CSkeleton *pSkeleton(m_pMount->GetSkeleton());
        uint uiBone(pSkeleton->GetBone(_CTS("_bone_hero")));
        if (uiBone != INVALID_BONE)
        {
            SBoneState *pBone(pSkeleton->GetBoneState(uiBone));
            
            CMatrix4x3<float> tmMount(m_pMount->GetAxis(), m_pMount->GetPosition());
            CMatrix4x3<float> tmBone(CAxis_cast(pBone->tm_local.axis), CVec3_cast(pBone->tm_local.pos) * m_pMount->GetBaseScale() * m_pMount->GetScale());
            CMatrix4x3<float> tmWorld(tmMount * tmBone);

            tmWorld.GetAxis().Forward().Normalize();
            tmWorld.GetAxis().Right().Normalize();
            tmWorld.GetAxis().Up().Normalize();

            pEffectThread->SetSourcePos(tmWorld.GetPosition());
            pEffectThread->SetSourceAxis(tmWorld.GetAxis());

            bPositioned = true;
        }
    }

    if (!bPositioned)
    {
        pEffectThread->SetSourcePos(m_v3Position);
        pEffectThread->SetSourceAxis(m_aAxis);
    }

    pEffectThread->SetSourceScale(GetBaseScale() * GetScale());

    if (pEffectThread->GetUseEntityEffectScale())
        pEffectThread->SetSourceEffectScale(GetEffectScale() / (GetBaseScale() * GetScale()));
    else
        pEffectThread->SetSourceEffectScale(1.0f);

    pEffectThread->SetSourceVisibility(Game.GetLocalPlayer() == nullptr || Game.GetLocalPlayer()->CanSee(this));
}


/*====================
  IUnitEntity::UpdateEffectThreadTarget
  ====================*/
void    IUnitEntity::UpdateEffectThreadTarget(CEffectThread *pEffectThread)
{
    pEffectThread->SetTargetModel(g_ResourceManager.GetModel(GetModel()));
    pEffectThread->SetTargetSkeleton(m_pSkeleton);

    bool bPositioned(false);

    if (m_pMount != nullptr)
    {
        m_pMount->UpdateSkeleton(true);

        CSkeleton *pSkeleton(m_pMount->GetSkeleton());
        uint uiBone(pSkeleton->GetBone(_CTS("_bone_hero")));
        if (uiBone != INVALID_BONE)
        {
            SBoneState *pBone(pSkeleton->GetBoneState(uiBone));
            
            CMatrix4x3<float> tmMount(m_pMount->GetAxis(), m_pMount->GetPosition());
            CMatrix4x3<float> tmBone(CAxis_cast(pBone->tm_local.axis), CVec3_cast(pBone->tm_local.pos) * m_pMount->GetBaseScale() * m_pMount->GetScale());
            CMatrix4x3<float> tmWorld(tmMount * tmBone);

            tmWorld.GetAxis().Forward().Normalize();
            tmWorld.GetAxis().Right().Normalize();
            tmWorld.GetAxis().Up().Normalize();

            pEffectThread->SetTargetPos(tmWorld.GetPosition());
            pEffectThread->SetTargetAxis(tmWorld.GetAxis());

            bPositioned = true;
        }
    }

    if (!bPositioned)
    {
        pEffectThread->SetTargetPos(m_v3Position);
        pEffectThread->SetTargetAxis(m_aAxis);
    }

    pEffectThread->SetTargetScale(GetBaseScale() * GetScale());

    if (pEffectThread->GetUseEntityEffectScale())
        pEffectThread->SetTargetEffectScale(GetEffectScale() / (GetBaseScale() * GetScale()));
    else
        pEffectThread->SetTargetEffectScale(1.0f);

    pEffectThread->SetTargetVisibility(Game.GetLocalPlayer() == nullptr || Game.GetLocalPlayer()->CanSee(this));
}


/*====================
  IUnitEntity::PlayRecipeEffect
  ====================*/
void    IUnitEntity::PlayRecipeEffect()
{
    if (HasLocalFlags(ENT_LOCAL_RECIPE_EFFECT))
        return;

    if (s_hRecipeEffect == INVALID_RESOURCE)
        return;

    SetLocalFlags(ENT_LOCAL_RECIPE_EFFECT);

    CGameEvent ev;
    ev.SetSourceEntity(GetIndex());
    ev.SetEffect(s_hRecipeEffect);
    Game.AddEvent(ev);
}


/*====================
  IUnitEntity::Moved

  Update brain to account for a forced movement
  ====================*/
void    IUnitEntity::Moved()
{
    m_cBrain.Moved();

    Interrupt(UNIT_ACTION_MOVE);
}


/*====================
  IUnitEntity::AggroCreeps

  Aggro enemy creeps in range
  ====================*/
void    IUnitEntity::AggroCreeps(float fRange, uint uiDuration, byte yTeam, uint uiDelay, bool bReaggroBlock)
{
    static uivector vEntities;
    float fAggroRange(fRange + m_bbBounds.GetDim(X) * DIAG);
    CBBoxf bbRegion(CVec3f(m_v3Position.xy() - CVec2f(fAggroRange), -FAR_AWAY),  CVec3f(m_v3Position.xy() + CVec2f(fAggroRange), FAR_AWAY));

    // Fetch
    Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_ACTIVE_UNIT);

    for (uivector_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        if (*cit == m_uiWorldIndex)
            continue;

        IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*cit)));
        if (pUnit == nullptr)
            continue;

        if (DistanceSq(m_v3Position.xy(), pUnit->GetPosition().xy()) > SQR(fAggroRange + pUnit->GetBounds().GetDim(X) * DIAG))
            continue;

        if (pUnit->GetTeam() == yTeam && pUnit->IsCreep() && pUnit->GetCombatType() != _T("Siege"))
            pUnit->Aggro(this, uiDuration, uiDelay, bReaggroBlock);
    }
}


/*====================
  IUnitEntity::Aggro

  Aggro on a specific unit
  ====================*/
void    IUnitEntity::Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock)
{
    m_cBrain.Aggro(pTarget, uiDuration, uiDelay, bReaggroBlock);
}


/*====================
  IUnitEntity::StartCooldown
  ====================*/
void    IUnitEntity::StartCooldown(uint uiCooldownType, uint uiStartTime, uint uiDuration)
{
    uint uiNewEndTime(Game.GetCooldownEndTime(uiStartTime, uiDuration));

    map<uint, SCooldown>::iterator itFind(m_mapCooldowns.find(uiCooldownType));
    if (itFind != m_mapCooldowns.end())
    {
        uint uiEndTime(Game.GetCooldownEndTime(itFind->second.uiStartTime, itFind->second.uiDuration));

        if (uiNewEndTime >= uiEndTime)
        {
            itFind->second.uiStartTime = uiStartTime;
            itFind->second.uiDuration = uiDuration;
        }
    }
    else
    {
        SCooldown cCooldown;
        cCooldown.uiStartTime = uiStartTime;
        cCooldown.uiDuration = uiDuration;

        m_mapCooldowns[uiCooldownType] = cCooldown;
    }

    // Update apparent cooldown of other slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr ||
            pTool->GetCooldownType() != uiCooldownType)
            continue;

        pTool->UpdateApparentCooldown();
    }
}


/*====================
  IUnitEntity::ReduceCooldown
  ====================*/
void    IUnitEntity::ReduceCooldown(uint uiCooldownType, uint uiDuration)
{
    map<uint, SCooldown>::iterator itFind(m_mapCooldowns.find(uiCooldownType));
    if (itFind != m_mapCooldowns.end())
    {
        if (itFind->second.uiDuration > uiDuration)
            itFind->second.uiDuration -= uiDuration;
        else
            itFind->second.uiDuration = 0;
    }

    // Update apparent cooldown of other slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr ||
            pTool->GetCooldownType() != uiCooldownType)
            continue;

        pTool->UpdateApparentCooldown();
    }
}


/*====================
  IUnitEntity::ResetCooldown
  ====================*/
void    IUnitEntity::ResetCooldown(uint uiCooldownType)
{
    map<uint, SCooldown>::iterator itFind(m_mapCooldowns.find(uiCooldownType));
    if (itFind != m_mapCooldowns.end())
    {
        itFind->second.uiStartTime = INVALID_TIME;
        itFind->second.uiDuration = 0;
    }

    // Update apparent cooldown of other slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr ||
            pTool->GetCooldownType() != uiCooldownType)
            continue;

        pTool->UpdateApparentCooldown();
    }
}


/*====================
  IUnitEntity::GetCooldown
  ====================*/
void    IUnitEntity::GetCooldown(uint uiCooldownType, uint &uiStartTime, uint &uiDuration)
{
    map<uint, SCooldown>::iterator itFind(m_mapCooldowns.find(uiCooldownType));
    if (itFind != m_mapCooldowns.end())
    {
        uiStartTime = itFind->second.uiStartTime;
        uiDuration = itFind->second.uiDuration;
    }
    else
    {
        uiStartTime = INVALID_TIME;
        uiDuration = 0;
    }
}


/*====================
  IUnitEntity::ResetCooldowns
  ====================*/
void    IUnitEntity::ResetCooldowns()
{
    m_mapCooldowns.clear();

    // Update apparent cooldown of slaves
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr || pTool->GetCooldownType() == 0)
            continue;

        pTool->SetApparentCooldown(INVALID_TIME, 0);
    }
}


/*====================
  IUnitEntity::GetCurrentAttackStateTarget
  ====================*/
uint    IUnitEntity::GetCurrentAttackStateTarget()
{
    return m_cBrain.GetCurrentAttackStateTarget();
}


/*====================
  IUnitEntity::GetCurrentAttackBehaviorTarget
  ====================*/
uint    IUnitEntity::GetCurrentAttackBehaviorTarget()
{
    return m_cBrain.GetCurrentAttackBehaviorTarget();
}


/*====================
  IUnitEntity::GetAttackEffectType
  ====================*/
float   IUnitEntity::GetAttackEffectType() const
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        IEntityTool *pTool(GetTool(iSlot));
        if (pTool == nullptr)
            continue;
        if (!pTool->IsActive())
            continue;

        if (pTool->GetActionType() != TOOL_ACTION_ATTACK_TOGGLE || !pTool->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
            continue;

        if (pTool->IsDisabled() || !pTool->IsReady())
            continue;

        float fManaCost(IsFreeCast() ? 0.0f : pTool->GetCurrentManaCost());
        if (GetMana() < fManaCost)
            continue;

        if (!pTool->CheckCost())
            continue;

        return pTool->GetAttackEffectType();
    }

    return GetInitialAttackEffectType();
}


/*====================
  IUnitEntity::RegenerateHealth
  ====================*/
void    IUnitEntity::RegenerateHealth(float fFrameTime)
{
    m_fHealth = CLAMP(m_fHealth + fFrameTime * GetHealthRegen(), 0.0f, m_fCurrentMaxHealth);
}


/*====================
  IUnitEntity::Heal
  ====================*/
void    IUnitEntity::Heal(float fHealth)
{
    m_fHealthAccumulator += fHealth * MAX(GetHealMultiplier(), 0.0f);
}


/*====================
  IUnitEntity::ChangeHealth
  ====================*/
void    IUnitEntity::ChangeHealth(float fHealth)
{
    m_fHealthAccumulator += fHealth;
}


/*====================
  IUnitEntity::GiveMana
  ====================*/
void    IUnitEntity::GiveMana(float fValue)
{
    m_fMana = CLAMP(m_fMana + fValue, 0.0f, GetMaxMana());
}


/*====================
  IUnitEntity::SpendMana
  ====================*/
bool    IUnitEntity::SpendMana(float fCost)
{
    if (fCost > m_fMana)
        return false;
    else
        m_fMana -= fCost;
    
    return true;
}


/*====================
  IUnitEntity::TakeMana
  ====================*/
void    IUnitEntity::TakeMana(float fValue)
{
    m_fMana = CLAMP(m_fMana - fValue, 0.0f, GetMaxMana());
}


/*====================
  IUnitEntity::RegenerateMana
  ====================*/
void    IUnitEntity::RegenerateMana(float fFrameTime)
{
    m_fMana = CLAMP(m_fMana + fFrameTime * GetManaRegen(), 0.0f, m_fCurrentMaxMana);
}


/*====================
  IUnitEntity::GetGuardChaseTime
  ====================*/
uint    IUnitEntity::GetGuardChaseTime() const
{
    if (m_uiGuardChaseTime == 0)
        return g_unitGuardChaseTime;
    return m_uiGuardChaseTime;
}


/*====================
  IUnitEntity::GetGuardChaseDistance
  ====================*/
uint    IUnitEntity::GetGuardChaseDistance() const
{
    if (m_uiGuardChaseDistance == 0)
        return g_unitGuardDistance;
    return m_uiGuardChaseDistance;
}


/*====================
  IUnitEntity::GetGuardReaggroChaseTime
  ====================*/
uint    IUnitEntity::GetGuardReaggroChaseTime() const
{
    if (m_uiGuardReaggroChaseTime == 0)
        return g_unitGuardReaggroChaseTime;
    return m_uiGuardReaggroChaseTime;
}


/*====================
  IUnitEntity::GetGuardReaggroChaseDistance
  ====================*/
uint    IUnitEntity::GetGuardReaggroChaseDistance() const
{
    if (m_uiGuardReaggroChaseDistance == 0)
        return g_unitGuardReaggroDistance;
    return m_uiGuardReaggroChaseDistance;
}


/*====================
  IUnitEntity::GetTransformedAttackOffset
  ====================*/
CVec3f  IUnitEntity::GetTransformedAttackOffset() const
{
    CAxis axis(GetAngles());
    return GetPosition() + TransformPoint(GetAttackOffset(), axis);
}


/*====================
  IUnitEntity::GetTransformedTargetOffset
  ====================*/
CVec3f  IUnitEntity::GetTransformedTargetOffset() const
{
    CAxis axis(GetAngles());
    return GetPosition() + TransformPoint(GetTargetOffset(), axis);
}


/*====================
  IUnitEntity::GetShield
  ====================*/
float   IUnitEntity::GetShield() const
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (m_apInventory[iSlot]->GetShield())
            return m_apInventory[iSlot]->GetAccumulator();
    }

    return 0.0f;
}


/*====================
  IUnitEntity::GetMaxShield
  ====================*/
float   IUnitEntity::GetMaxShield() const
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (m_apInventory[iSlot]->GetShield())
            return m_apInventory[iSlot]->GetMaxAccumulator();
    }

    return 0.0f;
}


/*====================
  IUnitEntity::GetShieldPercent
  ====================*/
float   IUnitEntity::GetShieldPercent() const
{
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (m_apInventory[iSlot] == nullptr)
            continue;

        if (m_apInventory[iSlot]->GetShield())
            return m_apInventory[iSlot]->GetAccumulator() / m_apInventory[iSlot]->GetMaxAccumulator();
    }

    return 0.0f;
}
