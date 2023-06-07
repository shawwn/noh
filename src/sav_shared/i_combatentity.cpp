// (C)2007 S2 Games
// i_combatentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_combatentity.h"
#include "c_meleeattackevent.h"
#include "c_teaminfo.h"

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
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_BOOL(g_meleeNewBlock);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*     ICombatEntity::s_pvFields;

CVAR_FLOATF(    p_armorFactor,                      0.06f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_armorMeleePierce,                 0.0f,   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_armorNonMeleeFactor,              1.0f,   CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    p_rearAttackAngle,                  40.0f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_rearAttackDamageMultiplier,       1.0f,   CVAR_GAMECONFIG);

CVAR_FLOATF(    p_blockAngle,                       60.0f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_blockedStunTime,                  1200,   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_blockCanceledStunTime,            500,    CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_UINTF(     p_attackInterruptStunTime,          1200,   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_attackInterruptDamageReduction,   0.25f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_blockDamageReduction,             0.33f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    p_staminaDamageRatio,               0.25f,  CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_staminaExhaustedPoint,            0.1f,   CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    p_attackSpeedExhausted,             1.0f,   CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_FLOATF(    g_repairRange,                      150.0f, CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(    g_expRepairing,                     0.15f,  CVAR_GAMECONFIG);
CVAR_FLOATF(    g_buildingRepairCost,               1.00f,  CVAR_GAMECONFIG);

extern CCvar<float>     p_staminaRegenIdle;
//=============================================================================

/*====================
  ICombatEntity::CEntityConfig::CEntityConfig
  ====================*/
ICombatEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(BoundsRadius, 18.0f),
INIT_ENTITY_CVAR(BoundsHeight, 64.0f),
INIT_ENTITY_CVAR(ViewHeight, 54.0f),
INIT_ENTITY_CVAR(Speed, 150.0f),
INIT_ENTITY_CVAR(MaxMana, 100.0f),
INIT_ENTITY_CVAR(ManaRegenRate, 4.0f),
INIT_ENTITY_CVAR(MaxStamina, 200.0f),
INIT_ENTITY_CVAR(Armor, 2.0f),
INIT_ENTITY_CVAR(Inventory0, _T("")),
INIT_ENTITY_CVAR(Inventory1, _T("")),
INIT_ENTITY_CVAR(Inventory2, _T("")),
INIT_ENTITY_CVAR(Inventory3, _T("")),
INIT_ENTITY_CVAR(Inventory4, _T("")),
INIT_ENTITY_CVAR(Inventory5, _T("")),
INIT_ENTITY_CVAR(Inventory6, _T("")),
INIT_ENTITY_CVAR(Inventory7, _T("")),
INIT_ENTITY_CVAR(Inventory8, _T("")),
INIT_ENTITY_CVAR(Inventory9, _T("")),
INIT_ENTITY_CVAR(DefaultInventorySlot, 0),
INIT_ENTITY_CVAR(IsSiege, false),
INIT_ENTITY_CVAR(IsHellbourne, false),
INIT_ENTITY_CVAR(CanBuild, false),
INIT_ENTITY_CVAR(BuildingRepairRate, 0.25f),
INIT_ENTITY_CVAR(SiegeRepairRate, 0.25f)
{
}


/*====================
  ICombatEntity::~ICombatEntity
  ====================*/
ICombatEntity::~ICombatEntity()
{
}


/*====================
  ICombatEntity::ICombatEntity
  ====================*/
ICombatEntity::ICombatEntity(CEntityConfig *pConfig) :
IVisualEntity(pConfig),
m_pEntityConfig(pConfig),

m_fStamina(0.0f),
m_fMana(0.0f),

m_iCurrentAction(PLAYER_ACTION_IDLE),
m_uiCurrentActionEndTime(0),
m_uiCurrentActionStartTime(0),
m_uiLastMeleeAttackTime(0),
m_uiLastMeleeAttackLength(0),
m_yAttackSequence(0),

m_ySelectedItem(NO_SELECTION),

m_uiSpellTargetIndex(INVALID_INDEX),
m_uiTargetIndex(INVALID_INDEX),

m_fFov(90.0f),
m_fBaseFov(90.0f)
{
}


/*====================
  ICombatEntity::GetSpeed
  ====================*/
float   ICombatEntity::GetSpeed() const
{
    FloatMod modSpeed;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modSpeed += m_apState[i]->GetSpeedMod();
    }

    return modSpeed.Modify(float(m_pEntityConfig->GetSpeed()));
}


/*====================
  ICombatEntity::Baseline
  ====================*/
void    ICombatEntity::Baseline()
{
    IVisualEntity::Baseline();

    m_fMana = 0.0f;
    m_fStamina = 0.0f;

    m_ySelectedItem = NO_SELECTION;
}


/*====================
  ICombatEntity::GetSnapshot
  ====================*/
void    ICombatEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
    // Base entity info
    IVisualEntity::GetSnapshot(snapshot);

    snapshot.AddRound16(m_fMana);
    snapshot.AddRound16(m_fStamina);

    snapshot.AddField(m_ySelectedItem);
}


/*====================
  ICombatEntity::ReadSnapshot
  ====================*/
bool    ICombatEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
    try
    {
        // Base entity info
        if (!IVisualEntity::ReadSnapshot(snapshot))
            return false;

        snapshot.ReadNextRound16(m_fMana);
        snapshot.ReadNextRound16(m_fStamina);

        snapshot.ReadNextField(m_ySelectedItem);

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICombatEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  ICombatEntity::GetTypeVector
  ====================*/
const vector<SDataField>&   ICombatEntity::GetTypeVector()
{
    if (!s_pvFields)
    {
        s_pvFields = K2_NEW(global,   vector<SDataField>)();
        s_pvFields->clear();
        const vector<SDataField> &vBase(IVisualEntity::GetTypeVector());
        s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

        s_pvFields->push_back(SDataField(_T("m_fMana"), FIELD_PUBLIC, TYPE_ROUND16));
        s_pvFields->push_back(SDataField(_T("m_fStamina"), FIELD_PUBLIC, TYPE_ROUND16));

        s_pvFields->push_back(SDataField(_T("m_ySelectedItem"), FIELD_PUBLIC, TYPE_CHAR));
    }

    return *s_pvFields;
}


/*====================
  ICombatEntity::DoAttack
  ====================*/
void    ICombatEntity::DoAttack(CMeleeAttackEvent &attack)
{
    FloatMod modAttackSpeed;

    // Notify each state that an attack is occuring
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        m_apState[i]->DoAttack(attack);
        if (!m_apState[i]->IsValid())
            RemoveState(i);
    }
}


/*====================
  ICombatEntity::DoRangedAttack
  ====================*/
void    ICombatEntity::DoRangedAttack()
{
    // Notify each state that an attack is occuring
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        m_apState[i]->DoRangedAttack();
        if (!m_apState[i]->IsValid())
            RemoveState(i);
    }

    // Notify each item that an attack is occuring
    for (int i(0); i < INVENTORY_END_BACKPACK; i++)
        if (GetItem(i) != NULL)
            GetItem(i)->DoRangedAttack();
}


/*====================
  ICombatEntity::Stun
  ====================*/
void    ICombatEntity::Stun(uint uiEndTime)
{
    StartAnimation(_T("stunned"), -1);
    SetAction(PLAYER_ACTION_STUNNED | PLAYER_ACTION_IMMOBILE, uiEndTime);
}


/*====================
  ICombatEntity::GetArmor
  ====================*/
float   ICombatEntity::GetArmor() const
{
    FloatMod modArmor;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modArmor += m_apState[i]->GetArmorMod();
    }

    return modArmor.Modify(GetBaseArmor());// * (1.0f + GetAttributeBoost(ATTRIBUTE_ARMOR)));
}


/*====================
  ICombatEntity::GetArmorDamageReduction
  ====================*/
float   ICombatEntity::GetArmorDamageReduction(float fArmor) const
{
    return (fArmor * p_armorFactor) / (1.0f + (fArmor * p_armorFactor));
}


/*====================
  ICombatEntity::Damage
  ====================*/
float   ICombatEntity::Damage(float fDamage, int iFlags, IVisualEntity *pAttacker, ushort unDamagingObjectID, bool bFeedback)
{
    if (iFlags & DAMAGE_FLAG_DIRECT)
        return IVisualEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);

    // Determine angle of attack
    float fAngle(0.0f);
    if (pAttacker != NULL && pAttacker->IsVisual())
    {
        CVec2f  v2AttackerDir(pAttacker->GetPosition().xy() - GetPosition().xy());
        v2AttackerDir.Normalize();
        CAxis   axisDefender(GetAngles());
        CVec2f  v2DefenderFwd(axisDefender.Forward2d());
        fAngle = DotProduct(-v2AttackerDir, v2DefenderFwd);
    }

    // Check for a rear attack
    if (fAngle >= DEGCOS(p_rearAttackAngle / 2.0f))
    {
        // Global bonus damage
        fDamage *= p_rearAttackDamageMultiplier;

        // Bonus damage from specific melee attacks
        if (pAttacker != NULL &&
            (iFlags & DAMAGE_FLAG_MELEE) &&
            pAttacker->IsCombat() &&
            pAttacker->GetAsCombatEnt()->GetAttackEvent().IsActive())
            fDamage *= pAttacker->GetAsCombatEnt()->GetAttackEvent().GetRearAttackMultiplier();
    }

    // Check for the results of a block
    if (((iFlags & DAMAGE_FLAG_MELEE) && fAngle <= DEGCOS(180.0f - (p_blockAngle / 2.0f))) &&
            ((!g_meleeNewBlock &&
            (m_iCurrentAction & PLAYER_ACTION_BLOCK) &&
            GetAttackEvent().IsActive() &&
            Game.GetGameTime() >= GetAttackEvent().GetImpactTime())
            ||
            (g_meleeNewBlock &&
            GetAttackEvent().IsActive() &&
            Game.GetGameTime() >= GetAttackEvent().GetImpactTime() &&
            Game.GetGameTime() <= GetAttackEvent().GetImpactEndTime())))
    {
        if (iFlags & DAMAGE_FLAG_BLOCKABLE)
        {
            // Stun the attacker
            if (pAttacker->IsCombat())
            {
                pAttacker->GetAsCombatEnt()->Stun(Game.GetGameTime() + p_blockedStunTime);
                pAttacker->GetAsCombatEnt()->SetEffect(EFFECT_CHANNEL_BLOOD_SPRAY, Game.RegisterEffect(_T("/shared/effects/blocked.effect")));
                pAttacker->GetAsCombatEnt()->IncEffectSequence(EFFECT_CHANNEL_BLOOD_SPRAY);

                // feedback
                if (pAttacker->IsPlayer())
                {
                    CBufferFixed<4> buffer;
                    buffer << GAME_CMD_HITFEEDBACK << byte(HIT_GOT_BLOCKED) << ushort(pAttacker->GetIndex());
                    Game.SendGameData(pAttacker->GetAsPlayerEnt()->GetClientID(), buffer, false);
                    bFeedback = false;
                }
                if (IsPlayer())
                {
                    CBufferFixed<4> buffer;
                    buffer << GAME_CMD_HITFEEDBACK << byte(HIT_BLOCKED) << ushort(pAttacker->GetIndex());
                    Game.SendGameData(GetAsPlayerEnt()->GetClientID(), buffer, false);
                    bFeedback = false;
                }
            }

            if (g_meleeNewBlock)
            {
                // Allow for a push
                GetAttackEvent().Push(GetAngles(), pAttacker->GetAsCombatEnt());
            }
            else
            {
                // Stop blocking once we block an attack
                if (GetItem(0) != NULL && GetItem(0)->GetAsMelee() != NULL)
                    GetItem(0)->GetAsMelee()->FinishedAction(PLAYER_ACTION_BLOCK);
                else
                {
                    StopAnimation(1);
                    SetAction(PLAYER_ACTION_IDLE, INVALID_TIME);

                    CMeleeAttackEvent &pAttack = GetAttackEvent();
                    pAttack.Clear();
                }
            }

            return 0.0f;
        }
        else
        {
            // Damage could not be blocked, player takes some damage and gets stunned
            Stun(Game.GetGameTime() + p_blockCanceledStunTime);
            fDamage *= p_blockDamageReduction;
            // feedback
            if (pAttacker->IsPlayer())
            {
                CBufferFixed<4> buffer;
                buffer << GAME_CMD_HITFEEDBACK << byte(HIT_INTERRUPTED) << ushort(GetIndex());
                Game.SendGameData(pAttacker->GetAsPlayerEnt()->GetClientID(), buffer, false);
                bFeedback = false;
            }
            if (IsPlayer())
            {
                CBufferFixed<4> buffer;
                buffer << GAME_CMD_HITFEEDBACK << byte(HIT_GOT_INTERRUPTED) << ushort(GetIndex());
                Game.SendGameData(GetAsPlayerEnt()->GetClientID(), buffer, false);
                bFeedback = false;
            }
        }
    }

    // Damage can interrupt strong attacks
    if ((m_iCurrentAction & PLAYER_ACTION_STRONG_ATTACK) &&
        (iFlags & DAMAGE_FLAG_INTERRUPT) &&
        p_attackInterruptStunTime)
    {
        Stun(Game.GetGameTime() + p_attackInterruptStunTime);
        fDamage *= p_attackInterruptDamageReduction;
    }

    // Reduce damage based on armor and damage type (melee or ranged)
    float fArmorReduction(1.0f);
    if (GetCurrentItem() == NULL || !GetCurrentItem()->IsMelee())
        fArmorReduction *= p_armorNonMeleeFactor;
    if (iFlags & DAMAGE_FLAG_MELEE)
        fArmorReduction *= (1.0f - p_armorMeleePierce);

    fDamage -= (GetArmorDamageReduction(GetArmor()) * fDamage * fArmorReduction);

    fDamage = IVisualEntity::Damage(fDamage, iFlags, pAttacker, unDamagingObjectID, bFeedback);

    if (fDamage > 0.0f)
    {
        DrainStamina(fDamage * p_staminaDamageRatio);
        SetEffect(EFFECT_CHANNEL_BLOOD_IMPACT, Game.RegisterEffect(_T("/shared/effects/bloodimpact_small.effect")));
        IncEffectSequence(EFFECT_CHANNEL_BLOOD_IMPACT);
    }

    return fDamage;
}


/*====================
  ICombatEntity::GetTargetPosition
  ====================*/
CVec3f  ICombatEntity::GetTargetPosition(float fRange, float fMinRange)
{
    CVec3f v3Origin;
    CVec3f v3Max;
    CVec3f v3Result;
    CVec3f v3Forward;

    v3Origin = GetPosition();
    v3Forward = M_GetForwardVecFromAngles(GetAngles());

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
  ICombatEntity::SelectItem
  ====================*/
void    ICombatEntity::SelectItem(int iSlot)
{
    IInventoryItem *pPrevItem(GetCurrentItem());

    m_ySelectedItem = CLAMP(iSlot, 0, int(MAX_INVENTORY));
    if (pPrevItem != NULL)
        pPrevItem->Unselected();
    if (GetCurrentItem() != NULL)
        GetCurrentItem()->Selected();
}


/*====================
  ICombatEntity::UseAmmo
  ====================*/
bool    ICombatEntity::UseAmmo(int iSlot, int iAmount)
{
    IInventoryItem *pItem(GetItem(iSlot));
    if (!pItem)
        return false;
    
    // Deduct ammo from current clip
    if (pItem->GetAmmo() < iAmount)
        return false;

    // Don't predict ammo
    if (Game.IsClient())
        return true;

    pItem->AdjustAmmo(-iAmount);
    return true;
}


/*====================
  ICombatEntity::SetAmmo
  ====================*/
void    ICombatEntity::SetAmmo(int iSlot, int iAmount)
{
    IInventoryItem *pItem(GetItem(iSlot));
    if (pItem)
        pItem->SetAmmo(iAmount);
}


/*====================
  ICombatEntity::GetAmmoCount
  ====================*/
int     ICombatEntity::GetAmmoCount(int iSlot) const
{
    PROFILE("ICombatEntity::GetAmmoCount");
    const IInventoryItem *pItem(GetItem(iSlot));
    if (!pItem ||
        (!pItem->GetAdjustedAmmoCount() && !pItem->IsConsumable() && !pItem->IsPersistant()) ||
        (pItem->IsConsumable() && !pItem->GetAsConsumable()->GetMaxPerStack()))
        return -1;

    return pItem->GetAmmo();
}


/*====================
  ICombatEntity::GetTotalAmmo
  ====================*/
int     ICombatEntity::GetTotalAmmo(int iSlot) const
{
    PROFILE("ICombatEntity::GetTotalAmmo");
    IInventoryItem *pItem(GetItem(iSlot));
    if (!pItem ||
        (!pItem->GetAdjustedAmmoCount() && !pItem->IsConsumable() && !pItem->IsPersistant()) ||
        (pItem->IsConsumable() && !pItem->GetAsConsumable()->GetMaxPerStack()))
        return -1;

    return pItem->GetAmmo();
}


/*====================
  ICombatEntity::RefillAmmo
  ====================*/
bool    ICombatEntity::RefillAmmo(int iSlot)
{
    CEntityTeamInfo *pTeamInfo;

    if (iSlot == -1)
    {
        bool bRefilledAmmo(false);
        for (int i(0); i < MAX_INVENTORY; ++i)
            bRefilledAmmo |= RefillAmmo(i);
        return bRefilledAmmo;
    }

    iSlot = CLAMP(iSlot, 0, MAX_INVENTORY - 1);
    if (m_apInventory[iSlot] == NULL || m_apInventory[iSlot]->IsDisabled())
        return false;

    pTeamInfo = Game.GetTeam(GetTeam());

    // Do not refill ammo if they don't have the building anymore
    if (pTeamInfo != NULL && !m_apInventory[iSlot]->GetPrerequisite().empty() && !pTeamInfo->HasBuilding(m_apInventory[iSlot]->GetPrerequisite()))
        return false;

    ushort unOldAmmo(m_apInventory[iSlot]->GetAmmo());
    m_apInventory[iSlot]->SetAmmo(m_apInventory[iSlot]->GetAdjustedAmmoCount());

    return (unOldAmmo != m_apInventory[iSlot]->GetAmmo());
}


/*====================
  ICombatEntity::GetModifiedDamage
  ====================*/
float   ICombatEntity::GetModifiedDamage(float fDamage)
{
    FloatMod modValue;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(fDamage);
}


/*====================
  ICombatEntity::GetStrongAttackMinDamage
  ====================*/
float   ICombatEntity::GetStrongAttackMinDamage(int iSlot)
{
    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsMelee())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsMelee()->GetStrongAttackMinDamage());
}


/*====================
  ICombatEntity::GetStrongAttackMaxDamage
  ====================*/
float   ICombatEntity::GetStrongAttackMaxDamage(int iSlot)
{
    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsMelee())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsMelee()->GetStrongAttackMaxDamage());
}

/*====================
  ICombatEntity::GetJumpAttackMinDamage
  ====================*/
float   ICombatEntity::GetJumpAttackMinDamage(int iSlot)
{
    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsMelee())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsMelee()->GetJumpAttackMinDamage());
}


/*====================
  ICombatEntity::GetJumpAttackMaxDamage
  ====================*/
float   ICombatEntity::GetJumpAttackMaxDamage(int iSlot)
{
    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsMelee())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsMelee()->GetJumpAttackMaxDamage());
}

/*====================
  ICombatEntity::GetRangedMinDamage
  ====================*/
float   ICombatEntity::GetRangedMinDamage(int iSlot)
{
    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsGun())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsGun()->GetMinDamage());
}


/*====================
  ICombatEntity::GetRangedMaxDamage
  ====================*/
float   ICombatEntity::GetRangedMaxDamage(int iSlot)
{

    FloatMod modValue;

    if (GetItem(iSlot) == NULL || !GetItem(iSlot)->IsGun())
        return 0.0f;

    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetDamageMod();
    }

    return modValue.Modify(GetItem(iSlot)->GetAsGun()->GetMaxDamage());
}


/*====================
  ICombatEntity::Copy
  ====================*/
void    ICombatEntity::Copy(const IGameEntity &B)
{
    IVisualEntity::Copy(B);

    const ICombatEntity *pB(B.GetAsCombatEnt());

    if (!pB)    
        return;

    const ICombatEntity &C(*pB);

    m_ySelectedItem =   C.m_ySelectedItem;

    m_iCurrentAction =          C.m_iCurrentAction;
    m_uiCurrentActionEndTime =  C.m_uiCurrentActionEndTime;
    m_uiCurrentActionStartTime =    C.m_uiCurrentActionStartTime;
    m_uiLastActionTime =        C.m_uiLastActionTime;
    m_uiLastMeleeAttackTime =   C.m_uiLastMeleeAttackTime;
    m_uiLastMeleeAttackLength = C.m_uiLastMeleeAttackLength;
    m_yAttackSequence =         C.m_yAttackSequence;
    
    m_attack =          C.m_attack;

    m_fMana =           C.m_fMana;
    m_fStamina =        C.m_fStamina;

    m_fFov =            C.m_fFov;
}


/*====================
  ICombatEntity::ClientPrecache
  ====================*/
void    ICombatEntity::ClientPrecache(CEntityConfig *pConfig)
{
    g_ResourceManager.Register(_T("/shared/effects/bloodimpact_small.effect"), RES_EFFECT);
    g_ResourceManager.Register(_T("/shared/effects/blocked.effect"), RES_EFFECT);

    IVisualEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetInventory0().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory0()));
    if (!pConfig->GetInventory1().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory1()));
    if (!pConfig->GetInventory2().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory2()));
    if (!pConfig->GetInventory3().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory3()));
    if (!pConfig->GetInventory4().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory4()));
    if (!pConfig->GetInventory5().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory5()));
    if (!pConfig->GetInventory6().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory6()));
    if (!pConfig->GetInventory7().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory7()));
    if (!pConfig->GetInventory8().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory8()));
    if (!pConfig->GetInventory9().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetInventory9()));
}


/*====================
  ICombatEntity::ServerPrecache
  ====================*/
void    ICombatEntity::ServerPrecache(CEntityConfig *pConfig)
{
    g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(_T("/shared/effects/bloodimpact_small.effect"), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
    g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(_T("/shared/effects/blocked.effect"), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

    IVisualEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetInventory0().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory0()));
    if (!pConfig->GetInventory1().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory1()));
    if (!pConfig->GetInventory2().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory2()));
    if (!pConfig->GetInventory3().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory3()));
    if (!pConfig->GetInventory4().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory4()));
    if (!pConfig->GetInventory5().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory5()));
    if (!pConfig->GetInventory6().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory6()));
    if (!pConfig->GetInventory7().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory7()));
    if (!pConfig->GetInventory8().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory8()));
    if (!pConfig->GetInventory9().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetInventory9()));
}


/*====================
  ICombatEntity::ServerFrame
  ====================*/
bool    ICombatEntity::ServerFrame()
{
    return IVisualEntity::ServerFrame();
}


/*====================
  ICombatEntity::GetAttributeBoost
  ====================*/
float   ICombatEntity::GetAttributeBoost(int iAttribute) const
{
    return 0.0f;
}


/*====================
  ICombatEntity::GetAttackSpeed
  ====================*/
float   ICombatEntity::GetAttackSpeed(bool bApplyAttributes) const
{
    float fSpeed(1.0f + (bApplyAttributes ? GetAttributeBoost(ATTRIBUTE_STRENGTH) : 0.0f));

    if (IsExhausted())
        fSpeed *= p_attackSpeedExhausted;

    FloatMod modAttackSpeed;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modAttackSpeed += m_apState[i]->GetAttackSpeedMod();
    }

    return modAttackSpeed.Modify(fSpeed);
}


/*====================
  ICombatEntity::GetMaxHealth
  ====================*/
float   ICombatEntity::GetMaxHealth() const
{
    FloatMod modValue;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetHealthMod();
    }

    return modValue.Modify(GetBaseMaxHealth() * (1.0f + GetAttributeBoost(ATTRIBUTE_ENDURANCE)));
}


/*====================
  ICombatEntity::GetMaxMana
  ====================*/
float   ICombatEntity::GetMaxMana() const
{
    FloatMod modValue;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetManaMod();
    }

    return modValue.Modify(GetBaseMaxMana() * (1.0f + GetAttributeBoost(ATTRIBUTE_INTELLIGENCE)));
}


/*====================
  ICombatEntity::GetMaxStamina
  ====================*/
float   ICombatEntity::GetMaxStamina() const
{
    FloatMod modValue;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modValue += m_apState[i]->GetStaminaMod();
    }

    return modValue.Modify(GetBaseMaxStamina() * (1.0f + GetAttributeBoost(ATTRIBUTE_AGILITY)));
}


/*====================
  ICombatEntity::GetHealthRegen
  ====================*/
float   ICombatEntity::GetHealthRegen() const
{
    FloatMod modRegen;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modRegen += m_apState[i]->GetHealthRegenMod();
    }

    return modRegen.Modify(GetBaseHealthRegen() * (1.0f + GetAttributeBoost(ATTRIBUTE_ENDURANCE)));
}


/*====================
  ICombatEntity::RegenerateHealth
  ====================*/
void    ICombatEntity::RegenerateHealth(float fFrameTime)
{
    m_fHealth = CLAMP(m_fHealth + fFrameTime * GetHealthRegen(), 0.0f, GetMaxHealth());

    if (m_fHealth == 0.0f && GetMaxHealth() > 0.0f && !IsInvulnerable())
        Kill(this);
}


/*====================
  ICombatEntity::GetManaRegen
  ====================*/
float   ICombatEntity::GetManaRegen() const
{
    FloatMod modRegen;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modRegen += m_apState[i]->GetManaRegenMod();
    }

    return modRegen.Modify(GetBaseManaRegen() * (1.0f + GetAttributeBoost(ATTRIBUTE_INTELLIGENCE)));
}


/*====================
  ICombatEntity::RegenerateMana
  ====================*/
void    ICombatEntity::RegenerateMana(float fFrameTime)
{
    if (HasNetFlags(ENT_NET_FLAG_CHANNELING))
        return;
    m_fMana = CLAMP(m_fMana + fFrameTime * GetManaRegen(), 0.0f, GetMaxMana());
}


/*====================
  ICombatEntity::GetBaseStaminaRegen
  ====================*/
float   ICombatEntity::GetBaseStaminaRegen() const
{
    return p_staminaRegenIdle;
}


/*====================
  ICombatEntity::GetStaminaRegen
  ====================*/
float   ICombatEntity::GetStaminaRegen(int iMoveState) const
{
    FloatMod modRegen;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modRegen += m_apState[i]->GetStaminaRegenMod();
    }

    return modRegen.Modify(GetBaseStaminaRegen() * (1.0f + GetAttributeBoost(ATTRIBUTE_AGILITY)));
}


/*====================
  ICombatEntity::RegenerateStamina
  ====================*/
void    ICombatEntity::RegenerateStamina(float fFrameTime, int iMoveState)
{
    m_fStamina = CLAMP(m_fStamina + fFrameTime * GetStaminaRegen(iMoveState), 0.0f, GetMaxStamina());
}

/*====================
  ICombatEntity::GetSpellResistance
  ====================*/
float   ICombatEntity::GetSpellResistance() const
{
    FloatMod modResist;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modResist += m_apState[i]->GetSpellResistMod();
    }

    return MAX(modResist.Modify(1.0f - GetAttributeBoost(ATTRIBUTE_INTELLIGENCE)), 0.0f);
}


/*====================
  ICombatEntity::GetSkillResistance
  ====================*/
float   ICombatEntity::GetSkillResistance() const
{
    FloatMod modResist;
    for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
    {
        if (m_apState[i] == NULL)
            continue;

        modResist += m_apState[i]->GetSkillResistMod();
    }

    return MAX(modResist.Modify(1.0f - GetAttributeBoost(ATTRIBUTE_INTELLIGENCE)), 0.0f);
}

/*====================
  ICombatEntity::IsExhausted
  ====================*/
bool    ICombatEntity::IsExhausted() const
{
    return (m_fStamina / GetMaxStamina()) <= p_staminaExhaustedPoint;
}


/*====================
  ICombatEntity::CanAccess
  ====================*/
bool    ICombatEntity::CanAccess(int iSlot)
{
    PROFILE("ICombatEntity::CanAccess");
    IInventoryItem *pCurrent(GetCurrentItem());
    IInventoryItem *pItem(GetItem(iSlot));

    if (!pCurrent || !pItem)
        return true;

    if (pCurrent->IsSkill() && pCurrent->GetSlot() != iSlot)
        return false;

    if (pItem->IsSkill())
    {
        ISkillItem *pSkill(pItem->GetAsSkill());

        if (pCurrent->IsMelee() && !pSkill->GetCanUseWithMelee())
            return false;
        else if (pCurrent->IsGun() && !pSkill->GetCanUseWithRanged())
            return false;
    }

    return true;
}


/*====================
  ICombatEntity::UseItem
  ====================*/
bool    ICombatEntity::UseItem(int iSlot, int iAmount)
{
    IInventoryItem *pItem(GetItem(iSlot));
    if (!pItem)
        return false;

    if (pItem->GetAmmo() < iAmount)
        return false;

    // Only update uses/item serverside, this will allow
    // the client to catch ammo/item changes with the snapshot
    if (Game.IsServer())
    {
        pItem->AdjustAmmo(-iAmount);
        
        if (pItem->GetAmmo() <= 0)
            RemoveItem(iSlot);
    }

    return true;
}


/*====================
  ICombatEntity::GiveItem
  ====================*/
int     ICombatEntity::GiveItem(int iSlot, ushort unID, bool bEnabled)
{
    try
    {
        iSlot = IVisualEntity::GiveItem(iSlot, unID, bEnabled);

        if (iSlot >= 0 && iSlot < MAX_INVENTORY && iSlot == m_ySelectedItem && m_apInventory[iSlot])
            m_apInventory[iSlot]->Selected();

        return iSlot;
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICombatEntity::GiveItem() - "), NO_THROW);
        return -1;
    }
}


/*====================
  ICombatEntity::TiltTrace
  ====================*/
float   ICombatEntity::TiltTrace(const CVec3f &v3Start, const CVec3f &v3End)
{
    STraceInfo trace;

    if (Game.TraceLine(trace, v3Start, v3End, TRACE_PLAYER_MOVEMENT, m_uiWorldIndex))
        return trace.fFraction;
    else
        return 1.0f;
}


/*====================
  ICombatEntity::AttachModel
  ====================*/
void    ICombatEntity::AttachModel(const tstring &sBoneName, ResHandle hModel)
{
    if (hModel == INVALID_RESOURCE || sBoneName.empty())
        return;

    try
    {
        if (m_pSkeleton == NULL)
            EX_WARN(_T("Invalid skeleton"));

        uint uiBone(m_pSkeleton->GetBone(sBoneName));
        if (uiBone == INVALID_BONE)
            EX_WARN(_T("Bone ") + QuoteStr(sBoneName) + _T(" does not exist"));

        SBoneState *pBone(m_pSkeleton->GetBoneState(uiBone));
        
        CMatrix4x3<float> tmPlayer(m_aAxis, GetPosition());
        CMatrix4x3<float> tmBone(CAxis_cast(pBone->tm_local.axis), CVec3_cast(pBone->tm_local.pos) * GetScale() * GetScale2());
        CMatrix4x3<float> tmWorld(tmPlayer * tmBone);

        CSceneEntity sceneEntity;
        sceneEntity.scale = GetScale() * GetScale2();
        sceneEntity.axis = tmWorld.GetAxis();
        sceneEntity.SetPosition(tmWorld.GetPosition());
        sceneEntity.hModel = hModel;
        sceneEntity.flags |= SCENEOBJ_USE_AXIS;

        // Check for a state setting an alternate skin
        tstring sAlternateSkin;
        for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
        {
            if (m_apState[i] == NULL)
                continue;

            sAlternateSkin = m_apState[i]->GetSkin();
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
        ex.Process(_T("IPlayerEntity::AttachModel() - "), NO_THROW);
    }
}


/*====================
  ICombateEntity::IsVisibleOnMinimap
  ====================*/
bool    ICombatEntity::IsVisibleOnMinimap(IPlayerEntity *pPlayer, bool bLargeMap)
{
    return IVisualEntity::IsVisibleOnMinimap(pPlayer, bLargeMap);
}


/*====================
  ICombatEntity::Repair
  ====================*/
void    ICombatEntity::Repair(bool bActivate)
{
    bool bRepair(bActivate);

    if (!GetCanBuild() || !IsIdle())
        bRepair = false;

    // Check for an entity in range
    CAxis axis(m_v3Angles);
    CVec3f v3Forward(axis.Forward());
    CVec3f v3Start(m_v3Position);
    CVec3f v3End(v3Start + v3Forward * (g_repairRange + GetBounds().GetMax(X)) * 2.f);

    STraceInfo trace;
    Game.TraceBox(trace, v3Start, v3End, GetBounds(), SURF_TERRAIN | TRACE_PLAYER_MOVEMENT, m_uiWorldIndex);
    if (trace.uiEntityIndex == INVALID_INDEX)
        bRepair = false;

    // Validate target and determine repair rate
    float fRepairRate(0.0f);

    IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(trace.uiEntityIndex));
    if (pEntity == NULL || pEntity->GetTeam() != m_iTeam || pEntity->GetHealthPercent() >= 1.0f)
        bRepair = false;

    if (bRepair && pEntity->IsBuilding())
    {
        if (pEntity->HasNetFlags(ENT_NET_FLAG_NO_REPAIR))
            bRepair = false;
    }

    CEntityTeamInfo *pTeam(NULL);
    if (pEntity != NULL)
        pTeam = Game.GetTeam(pEntity->GetTeam());

    if (bRepair)
    {
        if (pEntity->IsBuilding() && (pEntity->GetStatus() == ENTITY_STATUS_SPAWNING || (pTeam != NULL && pTeam->GetGold() > 0 && !Game.IsSuddenDeathActive())))
            fRepairRate = GetBuildingRepairRate();
        else if (pEntity->IsPlayer() && pEntity->GetAsPlayerEnt()->GetIsVehicle() && pEntity->GetStatus() == ENTITY_STATUS_ACTIVE)
            fRepairRate = GetSiegeRepairRate();
        else
            bRepair = false;
    }

    // Apply repair
    if (bRepair && Game.IsServer())
    {
        float fPrevHealth(pEntity->GetHealth());
        float fNewHeath(MIN(fPrevHealth + MsToSec(Game.GetFrameLength()) * fRepairRate, pEntity->GetMaxHealth()));
        float fHPRepaired(fNewHeath - fPrevHealth);
        
        IBuildingEntity *pBuilding(pEntity->GetAsBuilding());

        if (pBuilding == NULL ||
            pBuilding->GetStatus() == ENTITY_STATUS_SPAWNING ||
            (pTeam != NULL && pTeam->SpendGold(pBuilding->GetAccumulatedRepairCost())))
        {
            pEntity->Heal(fHPRepaired, this);
            GiveExperience(fHPRepaired * g_expRepairing);

            if (pBuilding != NULL)
            {
                pBuilding->SetHealLimit(pBuilding->GetHealth());
                
                if (pBuilding->GetStatus() == ENTITY_STATUS_ACTIVE && pBuilding->GetCost() > 0)
                {
                    pBuilding->CostPaid();
                    pBuilding->AddRepairCost((fHPRepaired / pBuilding->GetMaxHealth()) * pBuilding->GetCost() * g_buildingRepairCost * pBuilding->GetBaseArmor());
                }
            }
        }
    }

    // Start/stop animation
    if (bRepair && m_ayAnim[1] != g_ResourceManager.GetAnim(GetModelHandle(), _T("repair")))
        StartAnimation(_T("repair"), 1);
    if (!bRepair && m_ayAnim[1] == g_ResourceManager.GetAnim(GetModelHandle(), _T("repair")))
        StopAnimation(1);

}




/*====================
  ICombatEntity::SetAction
  ====================*/
void    ICombatEntity::SetAction(int iAction, uint uiEndTime)
{
    m_iCurrentAction = iAction;
    m_uiCurrentActionEndTime = uiEndTime;
    m_uiCurrentActionStartTime = Game.GetGameTime();
}