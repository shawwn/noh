// (C)2008 S2 Games
// i_entitytool.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entitytool.h"

#include "i_unitentity.h"
#include "c_combatevent.h"
#include "i_projectile.h"
#include "i_areaaffector.h"
#include "i_gadgetentity.h"
#include "i_entityitem.h"
#include "c_player.h"
#include "i_bitentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(IEntityTool, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    const TypeVector &vBase(ISlaveEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_ySlot"), TYPE_CHAR, 6, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yPublicFlags"), TYPE_CHAR, 4, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_yPrivateFlags"), TYPE_CHAR, 3, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiApprentCooldownTime"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiApprentCooldownDuration"), TYPE_INT, 32, 0));
    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiTimer"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IEntityTool::IEntityTool
  ====================*/
IEntityTool::IEntityTool() :
m_yPublicFlags(0),
m_yPrivateFlags(0),
m_unFlags(0),
m_bNegated(false),
m_bWasReady(true),

m_uiCooldownTime(INVALID_TIME),
m_uiCooldownDuration(0),
m_uiApparentCooldownTime(INVALID_TIME),
m_uiApparentCooldownDuration(0),
m_uiTargetUID(INVALID_INDEX),
m_v3TargetPos(V3_ZERO),
m_v3TargetDelta(V3_ZERO),
m_uiStartChannelTime(INVALID_TIME),
m_bFinished(false),
m_uiTimer(INVALID_TIME),

m_uiActivateTargetUID(INVALID_INDEX),
m_v3ActivateTarget(V3_ZERO),
m_v3ActivateDelta(V3_ZERO),
m_bActivateSecondary(false),
m_iActivateIssuedClientNumber(-1)
{
}


/*====================
  IEntityTool::Baseline
  ====================*/
void    IEntityTool::Baseline()
{
    ISlaveEntity::Baseline();

    m_ySlot = INVALID_SLOT;
    m_unFlags = 0;
    m_uiApparentCooldownTime = INVALID_TIME;
    m_uiApparentCooldownDuration = 0;
    m_uiTimer = INVALID_TIME;
}


/*====================
  IEntityTool::GetSnapshot
  ====================*/
void    IEntityTool::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    ISlaveEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteField(m_ySlot);

    m_yPublicFlags = byte(m_unFlags >> 8);
    m_yPrivateFlags = byte(m_unFlags & 0xff);
    snapshot.WriteField(m_yPublicFlags);

    if (uiFlags & SNAPSHOT_HIDDEN)
    {
        snapshot.WriteField(byte(0));
        snapshot.WriteField(INVALID_TIME);
        snapshot.WriteField(uint(0));
        snapshot.WriteField(INVALID_TIME);
    }
    else
    {
        snapshot.WriteField(m_yPrivateFlags);
        snapshot.WriteField(m_uiApparentCooldownTime);
        snapshot.WriteField(m_uiApparentCooldownDuration);
        snapshot.WriteField(m_uiTimer);
    }
}


/*====================
  IEntityTool::ReadSnapshot
  ====================*/
bool    IEntityTool::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        ISlaveEntity::ReadSnapshot(snapshot, 1);

        snapshot.ReadField(m_ySlot);

        snapshot.ReadField(m_yPublicFlags);
        snapshot.ReadField(m_yPrivateFlags);
        m_unFlags = ushort(m_yPublicFlags << 8) | m_yPrivateFlags;

        snapshot.ReadField(m_uiApparentCooldownTime);
        snapshot.ReadField(m_uiApparentCooldownDuration);
        snapshot.ReadField(m_uiTimer);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IEntityTool::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IEntityTool::ServerFrameSetup
  ====================*/
bool    IEntityTool::ServerFrameSetup()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    bool bIsReady(IsReady() || !IsActive());

    if (m_bWasReady == false && bIsReady)
    {
        // Send a ready event to all players can that can control this unit
        CBufferFixed<4> buffer;
        buffer << GAME_CMD_INVENTORY_READY_EVENT << ushort(pOwner->GetIndex()) << m_ySlot;

        const PlayerMap &mapPlayers(Game.GetPlayerMap());
        for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
        {
            if (pOwner->CanReceiveOrdersFrom(itPlayer->first))
                Game.SendGameData(itPlayer->first, buffer, false);
        }

        ExecuteActionScript(ACTION_SCRIPT_READY, pOwner, pOwner->GetPosition());
    }

    m_bWasReady = bIsReady;

    if (CheckCost())
        ClearFlag(ENTITY_TOOL_FLAG_INVALID_COST);
    else
        SetFlag(ENTITY_TOOL_FLAG_INVALID_COST);

    // Auras
    if (IsActive() &&
        pOwner->GetStatus() == ENTITY_STATUS_ACTIVE &&
        !pOwner->GetPassiveInventory() &&
        GetSlot() >= INVENTORY_START_ACTIVE &&
        GetSlot() <= INVENTORY_END_ACTIVE)
    {
        if (m_pDefinition != NULL)
            m_pDefinition->ApplyAuras(this, GetLevel());
    }

    if (GetActionType() == TOOL_ACTION_TOGGLE && HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) && GetActiveManaCost() > 0.0f)
    {
        float fManaCost(GetActiveManaCost() * MsToSec(Game.GetFrameLength()));
        if (pOwner->GetMana() < fManaCost)
            ToggleOff();

        pOwner->TakeMana(fManaCost);
    }

    // Update modifier bits and level of toggle entities
    for (uivector_it it(m_vToggleEntityUID.begin()); it != m_vToggleEntityUID.end(); ++it)
    {
        IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->IsState())
        {
            IEntityState *pState(pEntity->GetAsState());
            if (pState == NULL)
                continue;

            pState->SetPersistentModifierKeys(GetModifierKeys());
            pState->UpdateModifiers();
            pState->SetLevel(GetLevel());
        }
        else if (pEntity->IsUnit())
        {
            IUnitEntity *pUnit(pEntity->GetAsUnit());
            if (pUnit == NULL)
                continue;

            pUnit->SetPersistentModifierKeys(GetModifierKeys());
            pUnit->SetLevel(GetLevel());
        }
        else if (pEntity->IsAffector())
        {
            IAffector *pAffector(pEntity->GetAsAffector());
            if (pAffector == NULL)
                continue;

            pAffector->UpdateModifiers(GetModifierKeys());
            pAffector->SetLevel(GetLevel());
        }
        else if (pEntity->IsProjectile())
        {
            IProjectile *pProjectile(pEntity->GetAsProjectile());
            if (pProjectile == NULL)
                continue;

            pProjectile->UpdateModifiers(GetModifierKeys());
            pProjectile->SetLevel(GetLevel());
        }
    }

    return ISlaveEntity::ServerFrameSetup();
}


/*====================
  IEntityTool::ServerFrameAction
  ====================*/
bool    IEntityTool::ServerFrameAction()
{
    if (!ISlaveEntity::ServerFrameAction())
        return false;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return true;

    if (m_uiTimer != INVALID_TIME && m_uiTimer <= Game.GetGameTime())
    {
        m_uiTimer = INVALID_TIME;
        ExecuteActionScript(ACTION_SCRIPT_TIMER, pOwner, pOwner->GetPosition());
    }

    if (HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
    {
        bool bCancel(false);

        if (GetAutoToggleOffWhenDisabled() && IsDisabled())
            bCancel = true;
        if (GetAutoToggleOffWithTriggeredManaCost() && pOwner->GetMana() < GetTriggeredManaCost())
            bCancel = true;

        if (bCancel)
        {
            ToggleOff();
        }
    }

    // Toggle entities
    if (!HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
    {
        for (uivector_it it(m_vToggleEntityUID.begin()); it != m_vToggleEntityUID.end(); ++it)
        {
            IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
            if (pEntity == NULL)
                continue;
            
            if (pEntity->IsState())
            {
                IEntityState *pState(pEntity->GetAsState());
                if (pState == NULL)
                    continue;
                IUnitEntity *pOwner(pState->GetOwner());
                if (pOwner == NULL)
                    continue;

                pOwner->ExpireState(pState->GetSlot());
            }
            else if (pEntity->IsUnit())
            {
                IUnitEntity *pUnit(pEntity->GetAsUnit());
                if (pUnit == NULL)
                    continue;

                pUnit->Kill();
            }
            else if (pEntity->IsAffector())
            {
                IAffector *pAffector(pEntity->GetAsAffector());
                if (pAffector == NULL)
                    continue;

                pAffector->Expire();
            }
            else if (pEntity->IsProjectile())
            {
                IProjectile *pProjectile(pEntity->GetAsProjectile());
                if (pProjectile == NULL)
                    continue;

                pProjectile->Kill();
            }
        }
        m_vToggleEntityUID.clear();
    }

    if (HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE))
    {
        bool bCancel(false);

        IUnitEntity *pTarget(Game.GetUnitFromUniqueID(m_uiTargetUID));

        if (IsDisabled() || (pOwner->IsStunned() && !GetNoStun()))
            bCancel = true;
        else if (m_uiTargetUID != INVALID_INDEX && !IsValidTarget(pTarget))
            bCancel = true;

        if (GetChannelRange() > 0.0f)
        {
            if (pTarget != NULL && (GetActionType() == TOOL_ACTION_TARGET_ENTITY || ((GetActionType() == TOOL_ACTION_TARGET_DUAL || GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) && m_uiTargetUID != INVALID_INDEX)))
            {
                float fRange(pOwner->GetBounds().GetDim(X) * DIAG + GetChannelRange() + pTarget->GetBounds().GetDim(X) * DIAG);
                
                if (DistanceSq(pOwner->GetPosition().xy(), pTarget->GetPosition().xy()) > SQR(fRange))
                {
                    bCancel = true;
                }
            }
            else if (GetActionType() == TOOL_ACTION_TARGET_POSITION ||
                GetActionType() == TOOL_ACTION_TARGET_VECTOR ||
                GetActionType() == TOOL_ACTION_TARGET_CURSOR ||
                ((GetActionType() == TOOL_ACTION_TARGET_DUAL ||
                GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) &&
                m_uiTargetUID == INVALID_INDEX))
            {
                float fRange(pOwner->GetBounds().GetDim(X) * DIAG + GetChannelRange());
                
                if (DistanceSq(pOwner->GetPosition().xy(), m_v3TargetPos.xy()) > SQR(fRange))
                {
                    bCancel = true;
                }
            }
        }

        if (!Update())
            bCancel = true;

        if (bCancel)
        {
            Finish(true);
            ClearFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE);

            if (!GetAnim().empty())
                pOwner->StopAnimation(GetAnim(), GetAnimChannel());
        }
        else if (Game.GetGameTime() - m_uiStartChannelTime > GetChannelTime())
        {
            Finish(false);
            ClearFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE);

            if (!GetAnim().empty() && !GetNoStopAnim())
                pOwner->StopAnimation(GetAnim(), GetAnimChannel());
        }
    }

    return true;
}


/*====================
  IEntityTool::GetAdjustedActionTime
  ====================*/
uint    IEntityTool::GetAdjustedActionTime() const
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return 0;

    uint uiBase(GetCastActionTime());
    return INT_ROUND(uiBase / GetOwner()->GetCastSpeed());
}


/*====================
  IEntityTool::GetAdjustedCastTime
  ====================*/
uint    IEntityTool::GetAdjustedCastTime() const
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return 0;

    uint uiBase(GetCastTime());
    return INT_ROUND(uiBase / GetOwner()->GetCastSpeed());
}


/*====================
  IEntityTool::ImpactEntity
  ====================*/
bool    IEntityTool::ImpactEntity(uint uiTargetIndex, int iIssuedClientNumber, float fManaCost)
{
    IUnitEntity *pTarget(Game.GetUnitEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;
    if ((GetActionType() == TOOL_ACTION_TARGET_ENTITY || GetActionType() == TOOL_ACTION_TARGET_DUAL || GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) &&
        !IsValidTarget(pTarget))
        return false;

    m_uiTargetUID = pTarget->GetUniqueID();
    m_v3TargetPos = pTarget->GetPosition();

    CCombatEvent combat;
    combat.SetSuperType(SUPERTYPE_SPELL);
    combat.SetInitiatorIndex(GetOwnerIndex());
    combat.SetTarget(uiTargetIndex);
    combat.SetTarget(m_v3TargetPos);
    combat.SetProxyUID(GetProxyUID());
    combat.SetEffectType(GetCastEffectType());
    combat.SetNoResponse(GetNoResponse());
    combat.SetIssuedClientNumber(iIssuedClientNumber);
    combat.SetManaCost(fManaCost);

    AddActionScript(ACTION_SCRIPT_PRE_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, combat);
    AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, combat);

    // Do we want to play the bridge effect when the effect is negated?
    ResHandle hBridgeEffect(GetBridgeEffect());
    if (hBridgeEffect != INVALID_RESOURCE)
    {
        CGameEvent evBridge;
        evBridge.SetSourceEntity(GetOwnerIndex());
        evBridge.SetTargetEntity(uiTargetIndex);
        evBridge.SetEffect(hBridgeEffect);
        Game.AddEvent(evBridge);
    }

    combat.Process();

    m_bNegated = combat.GetNegated();

    if (!m_bNegated)
    {
        ResHandle hImpactEffect(GetImpactEffect());
        if (hImpactEffect != INVALID_RESOURCE)
        {
            CGameEvent evImpact;
            evImpact.SetSourceEntity(uiTargetIndex);
            evImpact.SetEffect(hImpactEffect);
            Game.AddEvent(evImpact);
        }
    }

    return true;
}


/*====================
  IEntityTool::ImpactPosition
  ====================*/
bool    IEntityTool::ImpactPosition(const CVec3f &v3TargetPosition, int iIssuedClientNumber, float fManaCost)
{
    m_bNegated = false;
    m_uiTargetUID = INVALID_INDEX;
    m_v3TargetPos = v3TargetPosition;

    CCombatEvent combat;
    combat.SetSuperType(SUPERTYPE_SPELL);
    combat.SetInitiatorIndex(GetOwnerIndex());
    combat.SetTarget(INVALID_INDEX);
    combat.SetTarget(m_v3TargetPos);
    combat.SetProxyUID(GetProxyUID());
    combat.SetEffectType(GetCastEffectType());
    combat.SetNoResponse(GetNoResponse());
    combat.SetIssuedClientNumber(iIssuedClientNumber);
    combat.SetManaCost(fManaCost);

    AddActionScript(ACTION_SCRIPT_PRE_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, combat);
    AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, combat);

    combat.Process();

    m_bNegated = combat.GetNegated();
    return true;
}


/*====================
  IEntityTool::ImpactVector
  ====================*/
bool    IEntityTool::ImpactVector(const CVec3f &v3TargetPosition, const CVec3f &v3TargetDelta, int iIssuedClientNumber, float fManaCost)
{
    m_bNegated = false;
    m_uiTargetUID = INVALID_INDEX;
    m_v3TargetPos = v3TargetPosition;
    m_v3TargetDelta = v3TargetDelta;

    CCombatEvent combat;
    combat.SetSuperType(SUPERTYPE_SPELL);
    combat.SetInitiatorIndex(GetOwnerIndex());
    combat.SetTarget(INVALID_INDEX);
    combat.SetTarget(m_v3TargetPos);
    combat.SetDelta(m_v3TargetDelta);
    combat.SetProxyUID(GetProxyUID());
    combat.SetEffectType(GetCastEffectType());
    combat.SetNoResponse(GetNoResponse());
    combat.SetIssuedClientNumber(iIssuedClientNumber);
    combat.SetManaCost(fManaCost);

    AddActionScript(ACTION_SCRIPT_PRE_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, combat);
    AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT, combat);
    AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, combat);

    combat.Process();

    m_bNegated = combat.GetNegated();
    return true;
}


/*====================
  IEntityTool::PlayCastEffect
  ====================*/
void    IEntityTool::PlayCastEffect()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    ResHandle hEffect(GetCastEffect());
    if (hEffect == INVALID_RESOURCE)
        return;

    CGameEvent evCast;
    evCast.SetSourceEntity(pOwner->GetIndex());
    evCast.SetEffect(hEffect);
    Game.AddEvent(evCast);
}


/*====================
  IEntityTool::PlayActionEffect
  ====================*/
void    IEntityTool::PlayActionEffect()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    ResHandle hEffect(GetActionEffect());
    if (hEffect == INVALID_RESOURCE)
        return;

    CGameEvent evAction;
    evAction.SetSourceEntity(pOwner->GetIndex());
    evAction.SetEffect(hEffect);
    Game.AddEvent(evAction);
}


/*====================
  IEntityTool::IsDisabled
  ====================*/
bool    IEntityTool::IsDisabled() const
{
    if (GetActionType() == TOOL_ACTION_PASSIVE)
        return false;

    if (GetDisabled())
        return true;

    return false;
}


/*====================
  IEntityTool::CheckCost
  ====================*/
bool    IEntityTool::CheckCost()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    CCombatEvent combat;

    ExecuteActionScript(ACTION_SCRIPT_CHECK_COST, pOwner, pOwner->GetPosition(), &combat);

    return !combat.GetInvalid();
}


/*====================
  IEntityTool::CheckTriggeredCost
  ====================*/
bool    IEntityTool::CheckTriggeredCost()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    CCombatEvent combat;

    ExecuteActionScript(ACTION_SCRIPT_CHECK_TRIGGERED_COST, pOwner, pOwner->GetPosition(), &combat);

    return !combat.GetInvalid();
}


/*====================
  IEntityTool::CanOrder
  ====================*/
bool    IEntityTool::CanOrder()
{
    if (!IsReady())
        return false;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    if (pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    if (IsDisabled())
        return false;

    if (pOwner->GetMana() < GetCurrentManaCost() && !pOwner->IsFreeCast())
        return false;

    if (GetCharges() < GetChargeCost())
        return false;

    if (IsItem() && !HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        return false;

#if 0
    if (GetUseProxy() && SelectProxy() == NULL)
        return false;
#endif

    if (GetActionType() == TOOL_ACTION_TOGGLE && GetAutoToggleOffWithTriggeredManaCost() && pOwner->GetMana() < GetTriggeredManaCost())
        return false;

    if (!Game.IsValidTarget(GetActivateScheme(), 0, pOwner, pOwner, true))
        return false;

    if (Game.IsServer())
    {
        if (CheckCost())
            ClearFlag(ENTITY_TOOL_FLAG_INVALID_COST);
        else
            SetFlag(ENTITY_TOOL_FLAG_INVALID_COST);
    }

    if (HasFlag(ENTITY_TOOL_FLAG_INVALID_COST))
        return false;

    return true;
}


/*====================
  IEntityTool::CanActivate
  ====================*/
bool    IEntityTool::CanActivate()
{
    if (!IsReady())
        return false;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    if (pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    if (pOwner->IsStunned() && !GetNoStun())
        return false;

    if (IsDisabled())
        return false;

    if (pOwner->GetMana() < GetCurrentManaCost() && !pOwner->IsFreeCast())
        return false;

    if (IsItem() && !HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
        return false;

#if 0
    if (GetUseProxy() && SelectProxy() == NULL)
        return false;
#endif

    if (GetActionType() == TOOL_ACTION_TOGGLE && GetAutoToggleOffWithTriggeredManaCost() && (pOwner->GetMana() < GetTriggeredManaCost() || !CheckTriggeredCost()))
        return false;

    if (!Game.IsValidTarget(GetActivateScheme(), 0, pOwner, pOwner, true))
        return false;

    if (Game.IsServer())
    {
        if (CheckCost())
            ClearFlag(ENTITY_TOOL_FLAG_INVALID_COST);
        else
            SetFlag(ENTITY_TOOL_FLAG_INVALID_COST);
    }

    if (HasFlag(ENTITY_TOOL_FLAG_INVALID_COST))
        return false;

    return true;
}


/*====================
  IEntityTool::IsTargetValid
  ====================*/
bool    IEntityTool::IsTargetValid(IUnitEntity *pTarget, const CVec3f &v3Target)
{
    if (GetNoTargetRadius() > 0.0f)
    {
        uivector vResult;
        Game.GetEntitiesInRadius(vResult, v3Target.xy(), GetNoTargetRadius(), 0);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == NULL)
                continue;

            if (!Game.IsValidTarget(GetNoTargetScheme(), GetNoCastEffectType(), GetOwner(), pUnit, GetNoTargetIgnoreInvulnerable()))
                continue;

            return false;
        }
    }

    return true;
}


/*====================
  IEntityTool::AddActionScript
  ====================*/
void    IEntityTool::AddActionScript(EEntityActionScript eScript, CCombatEvent &combat)
{
    if (m_pDefinition == NULL)
        return;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    CCombatActionScript *pScript(m_pDefinition->GetActionScript(eScript));
    if (pScript != NULL && (!pOwner->IsIllusion() || pScript->GetPropagateToIllusions()))
    {
        CCombatActionScript &cScript(combat.AddActionScript(eScript, *pScript));
        cScript.SetThisUID(GetUniqueID());
        cScript.SetLevel(GetLevel());
    }
}


/*====================
  IEntityTool::Impact
  ====================*/
bool    IEntityTool::Impact(IUnitEntity *pTarget, const CVec3f &v3Target, const CVec3f &v3Delta, bool bSecondary, int iIssuedClientNumber, float fManaCost)
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    bool bActivated(false);

    switch (GetActionType())
    {
    case TOOL_ACTION_PASSIVE:
        return false;

    case TOOL_ACTION_TOGGLE:
        PlayActionEffect();
        ExecuteActionScript(HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) ? ACTION_SCRIPT_TOGGLE_OFF : ACTION_SCRIPT_TOGGLE_ON, pOwner, pOwner->GetPosition());
        ToggleFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE);
        bActivated = true;
        break;

    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TARGET_SELF:
    case TOOL_ACTION_SELF_POSITION:
        PlayActionEffect();
        bActivated = ImpactEntity(GetOwnerIndex(), iIssuedClientNumber, fManaCost);
        break;
    
    case TOOL_ACTION_FACING:
    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_CURSOR:
        PlayActionEffect();
        if (!GetCastProjectileName().empty())
            bActivated = CreateProjectile(v3Target, iIssuedClientNumber, fManaCost);
        else
            bActivated = ImpactPosition(v3Target, iIssuedClientNumber, fManaCost);
        break;

    case TOOL_ACTION_TARGET_VECTOR:
        PlayActionEffect();
        if (!GetCastProjectileName().empty())
            bActivated = CreateProjectile(v3Target, iIssuedClientNumber, fManaCost);
        else
            bActivated = ImpactVector(v3Target, v3Delta, iIssuedClientNumber, fManaCost);
        break;

    case TOOL_ACTION_TARGET_ENTITY:
        PlayActionEffect();
        if (pTarget != NULL)
        {
            if (!GetCastProjectileName().empty())
                bActivated = CreateProjectile(pTarget->GetIndex(), iIssuedClientNumber, fManaCost);
            else
                bActivated = ImpactEntity(pTarget->GetIndex(), iIssuedClientNumber, fManaCost);
        }
        break;

    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
        PlayActionEffect();
        if (pTarget != NULL)
        {
            if (!GetCastProjectileName().empty())
                bActivated = CreateProjectile(pTarget->GetIndex(), iIssuedClientNumber, fManaCost);
            else
                bActivated = ImpactEntity(pTarget->GetIndex(), iIssuedClientNumber, fManaCost);
        }
        else
        {
            if (!GetCastProjectileName().empty())
                bActivated = CreateProjectile(v3Target, iIssuedClientNumber, fManaCost);
            else
                bActivated = ImpactPosition(v3Target, iIssuedClientNumber, fManaCost);
        }

        break;

    case TOOL_ACTION_ATTACK:
    case TOOL_ACTION_ATTACK_TOGGLE:
        {
            if (!GetCastProjectileName().empty())
                pOwner->SetAttackProjectile(EntityRegistry.LookupID(GetCastProjectileName()));
            
            if (GetActionEffect() != INVALID_RESOURCE)
                pOwner->SetAttackActionEffect(GetActionEffect());
            if (GetImpactEffect() != INVALID_RESOURCE)
                pOwner->SetAttackImpactEffect(GetImpactEffect());

            CCombatEvent &combat(pOwner->GetCombatEvent());
            combat.SetEffectType(GetCastEffectType());
            combat.SetManaCost(fManaCost);

            AddActionScript(ACTION_SCRIPT_PRE_IMPACT, combat);
            AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, combat);
            AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, combat);
            AddActionScript(ACTION_SCRIPT_IMPACT, combat);
            AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, combat);

            bActivated = pOwner->Attack(pTarget, true);

            if (bActivated)
                pOwner->SetAttackCooldownTime(Game.GetGameTime() + MAX(int(pOwner->GetAdjustedAttackCooldown()) - int(pOwner->GetAdjustedAttackActionTime()), 0));
        }
        break;

    case TOOL_ACTION_GLOBAL:
        {
            PlayActionEffect();
            IGameEntity *pEntity(Game.GetFirstEntity());
            while (pEntity != NULL)
            {
                IUnitEntity *pUnit(pEntity->GetAsUnit());
                pEntity = Game.GetNextEntity(pEntity);
                if (pUnit == NULL)
                    continue;

                if (Game.IsValidTarget(GetTargetScheme(), GetCastEffectType(), pOwner, pUnit, GetIgnoreInvulnerable()))
                    ImpactEntity(pUnit->GetIndex(), iIssuedClientNumber, fManaCost);
            }
            bActivated = true;
        }
        break;
    }

    return bActivated;
}


/*====================
  IEntityTool::ExecuteOwner
  ====================*/
void    IEntityTool::ExecuteOwner(EEntityActionScript eScript, EEntityActionScript eOwnerScript,
                                  IUnitEntity *pTarget, const CVec3f &v3Target,
                                  CCombatEvent *pCombatEvent)
{
    ExecuteActionScript(eScript, pTarget, v3Target, pCombatEvent);

    IUnitEntity* pOwner(GetOwner());
    if (pOwner != NULL)
    {
        if (pTarget != NULL)
            pOwner->Action(eOwnerScript, pTarget, this, pCombatEvent);
        else
            pOwner->Action(eOwnerScript, v3Target, this, pCombatEvent);
    }
}


/*====================
  IEntityTool::Activate
  ====================*/
bool    IEntityTool::Activate(IUnitEntity *pTarget, const CVec3f &v3Target, const CVec3f &v3Delta, bool bSecondary, int iIssuedClientNumber)
{
    if (!IsReady())
        return false;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    float fManaCost(GetCurrentManaCost());
    if (pOwner->GetMana() < (pOwner->IsFreeCast() ? 0.0f : GetCurrentManaCost()))
        return false;

    if (GetCharges() < GetChargeCost())
        return false;

    if (GetUseProxy())
    {
        IUnitEntity *pProxy(SelectProxy());

        if (pProxy == NULL)
            return false;

        m_uiProxyUID = pProxy->GetUniqueID();
    }

    if (GetActionType() == TOOL_ACTION_NO_TARGET ||
        GetActionType() == TOOL_ACTION_TARGET_SELF ||
        GetActionType() == TOOL_ACTION_SELF_POSITION)
        pTarget = GetOwner();

    CVec3f v3Pos(v3Target);
    if (pTarget != NULL && GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION)
    {
        v3Pos = pTarget->GetPosition();
        pTarget = NULL;
    }

    if (!IsTargetValid(pTarget, v3Pos))
        return false;

    // Allow inventory items to react to the action
    for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
    {
        if (pOwner->GetInventorySlot(iSlot) == NULL)
            continue;

        if (!pOwner->GetInventorySlot(iSlot)->OwnerAction())
            pOwner->RemoveSlave(iSlot);
    }

    if (!GetIsChanneling() || !GetDeferChannelCost())
    {
        CCombatEvent combat;
        combat.SetManaCost(fManaCost);
        combat.SetCooldownTime(GetCurrentCooldownTime());

        ExecuteActionScript(ACTION_SCRIPT_PRE_COST, pTarget, pTarget == NULL ? v3Pos : pTarget->GetPosition(), &combat);
        
        if (!GetNoResponse())
            pOwner->Action(ACTION_SCRIPT_ACTIVATE_PRE_COST, pTarget, this, &combat);

        if (pOwner->IsFreeCast())
            combat.SetManaCost(0.0f);

        ExecuteActionScript(ACTION_SCRIPT_ACTIVATE_COST, pOwner, pOwner->GetPosition(), &combat);

        if (combat.GetInvalid())
            return false;

        StartCooldown(Game.GetGameTime(), combat.GetCooldownTime());
        pOwner->SpendMana(combat.GetManaCost());

        if (GetChargeCost())
            RemoveCharge();
    }
    
    if (!GetIsChanneling() || !GetDeferChannelImpact())
    {
        CCombatEvent combat;
        combat.SetManaCost(fManaCost);

        if (!GetNoResponse())
            pOwner->Action(ACTION_SCRIPT_ACTIVATE_PRE_IMPACT, pTarget, this, &combat);
    }

    ExecuteActionScript(ACTION_SCRIPT_ACTION, pTarget, pTarget == NULL ? v3Pos : pTarget->GetPosition());

    m_bFinished = false;
    m_bNegated = false;

    m_uiActivateTargetUID = pTarget != NULL ? pTarget->GetUniqueID() : INVALID_INDEX;
    m_v3ActivateTarget = v3Pos;
    m_v3ActivateDelta = v3Delta;
    m_bActivateSecondary = bSecondary;
    m_iActivateIssuedClientNumber = iIssuedClientNumber;
    
    bool bActivated(true);

    if (!GetIsChanneling() || !GetDeferChannelImpact())
        bActivated = Impact(pTarget, v3Pos, v3Delta, bSecondary, iIssuedClientNumber, fManaCost);
    else
    {
        m_uiTargetUID = m_uiActivateTargetUID;
        m_v3TargetPos = pTarget != NULL ? pTarget->GetPosition() : V3_ZERO;
    }

    if (bActivated)
    {
        if (GetIsChanneling() && !m_bNegated)
            ExecuteOwner(ACTION_SCRIPT_CHANNEL_START, ACTION_SCRIPT_CHANNELING_START, pTarget, pTarget == NULL ? v3Pos : pTarget->GetPosition());

        if (!GetIsChanneling() || !GetDeferChannelImpact())
        {
            if (!GetNoResponse())
                pOwner->Action(ACTION_SCRIPT_ACTIVATE_IMPACT, pTarget, this);
        }

        if (IsAbility())
        {
            if (!GetIsChanneling() || !GetDeferChannelImpact())
            {
                if (!GetNoResponse())
                    pOwner->Action(ACTION_SCRIPT_ABILITY_IMPACT, pTarget, this);

                Game.LogAbility(GAME_LOG_ABILITY_ACTIVATE, GetAsAbility(), pTarget);
            }
        }
        else if (IsItem())
        {
            GetAsItem()->SetPurchaseTime(INVALID_TIME);
            Game.LogItem(GAME_LOG_ITEM_ACTIVATE, GetAsItem(), pTarget);
        }

        if (GetIsChanneling() && !m_bNegated)
        {
            SetFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE);
            m_uiStartChannelTime = Game.GetGameTime();
        }

        if (!m_bNegated)
            ExecuteActionScript(ACTION_SCRIPT_COMPLETE, pTarget, pTarget == NULL ? v3Pos : pTarget->GetPosition());

        if (IsAbility())
        {
            if (!GetIsChanneling() && !GetNoResponse() && GetActionType() != TOOL_ACTION_TOGGLE)
                pOwner->Action(ACTION_SCRIPT_ABILITY_FINISH, pTarget, this);
        }
    }

    return true;
}


/*====================
  IEntityTool::ToggleAutoCast
  ====================*/
bool    IEntityTool::ToggleAutoCast()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    if (GetActionType() == TOOL_ACTION_ATTACK ||
        GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
        GetAllowAutoCast())
    {
        ExecuteActionScript(HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE) ? ACTION_SCRIPT_TOGGLE_OFF : ACTION_SCRIPT_TOGGLE_ON, pOwner, pOwner->GetPosition());
        ToggleFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE);

        return true;
    }

    return false;
}


/*====================
  IEntityTool::UpdateEntity
  ====================*/
bool    IEntityTool::UpdateEntity(uint uiTargetIndex)
{
    IUnitEntity *pTarget(Game.GetUnitEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;

    if (GetIsChanneling())
        ExecuteOwner(ACTION_SCRIPT_CHANNEL_FRAME, ACTION_SCRIPT_CHANNELING_FRAME, pTarget, pTarget->GetPosition());

    return true;
}


/*====================
  IEntityTool::UpdatePosition
  ====================*/
bool    IEntityTool::UpdatePosition(const CVec3f &v3Target)
{
    if (GetIsChanneling())
        ExecuteOwner(ACTION_SCRIPT_CHANNEL_FRAME, ACTION_SCRIPT_CHANNELING_FRAME, NULL, v3Target);

    return true;
}


/*====================
  IEntityTool::Update
  ====================*/
bool    IEntityTool::Update()
{
    switch (GetActionType())
    {
    case TOOL_ACTION_GLOBAL:
    case TOOL_ACTION_ATTACK:
    case TOOL_ACTION_ATTACK_TOGGLE:
    case TOOL_ACTION_PASSIVE:
    case TOOL_ACTION_TOGGLE:
        break;

    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TARGET_ENTITY:
    case TOOL_ACTION_TARGET_SELF:
    case TOOL_ACTION_FACING:
    case TOOL_ACTION_SELF_POSITION:
        UpdateEntity(Game.GetGameIndexFromUniqueID(m_uiTargetUID));
        break;

    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_CURSOR:
    case TOOL_ACTION_TARGET_VECTOR:
        UpdatePosition(m_v3TargetPos);
        break;

    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
        if (m_uiTargetUID != INVALID_INDEX)
            UpdateEntity(Game.GetGameIndexFromUniqueID(m_uiTargetUID));
        else
            UpdatePosition(m_v3TargetPos);
        break;
    }

    return true;
}


/*====================
  IEntityTool::FinishEntity
  ====================*/
bool    IEntityTool::FinishEntity(uint uiTargetIndex, bool bCancel)
{
    IUnitEntity *pTarget(Game.GetUnitEntity(uiTargetIndex));
    if (pTarget == NULL)
        return false;

    if (GetIsChanneling())
    {
        if (bCancel)
            ExecuteOwner(ACTION_SCRIPT_CHANNEL_BROKEN, ACTION_SCRIPT_CHANNELING_BROKEN, pTarget, pTarget->GetPosition());
        else
            ExecuteOwner(ACTION_SCRIPT_CHANNEL_END, ACTION_SCRIPT_CHANNELING_END, pTarget, pTarget->GetPosition());
    }

    return true;
}


/*====================
  IEntityTool::FinishPosition
  ====================*/
bool    IEntityTool::FinishPosition(const CVec3f &v3Target, bool bCancel)
{
    if (GetIsChanneling())
    {
        if (bCancel)
            ExecuteOwner(ACTION_SCRIPT_CHANNEL_BROKEN, ACTION_SCRIPT_CHANNELING_BROKEN, NULL, v3Target);
        else
            ExecuteOwner(ACTION_SCRIPT_CHANNEL_END, ACTION_SCRIPT_CHANNELING_END, NULL, v3Target);
    }

    return true;
}


/*====================
  IEntityTool::Finish
  ====================*/
void    IEntityTool::Finish(bool bCancel)
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    if (m_bFinished)
        return;

    m_bFinished = true;

    if (GetIsChanneling() && !bCancel)
    {
        float fManaCost(0.0f);

        if (GetDeferChannelCost())
        {
            fManaCost = pOwner->IsFreeCast() ? 0.0f : GetCurrentManaCost();
            if (pOwner->GetMana() < fManaCost)
            {
                bCancel = true;
            }
            else
            {
                StartCooldown(Game.GetGameTime(), GetCurrentCooldownTime());
                pOwner->SpendMana(fManaCost);
                RemoveCharge();
            }
        }

        if (GetDeferChannelImpact() && !bCancel)
        {
            IUnitEntity *pTarget(Game.GetUnitFromUniqueID(m_uiActivateTargetUID));

            if (!GetNoResponse())
                pOwner->Action(ACTION_SCRIPT_ACTIVATE_PRE_IMPACT, pTarget, this);

            bool bActivated(false);

            if (IsTargetValid(pTarget, m_v3ActivateTarget))
                bActivated = Impact(pTarget, m_v3ActivateTarget, m_v3ActivateDelta, m_bActivateSecondary, m_iActivateIssuedClientNumber, fManaCost);

            if (bActivated)
            {
                if (!GetNoResponse())
                    pOwner->Action(ACTION_SCRIPT_ACTIVATE_IMPACT, pTarget, this);

                if (IsAbility())
                {
                    if (!GetNoResponse())
                        pOwner->Action(ACTION_SCRIPT_ABILITY_IMPACT, pTarget, this);

                    Game.LogAbility(GAME_LOG_ABILITY_ACTIVATE, GetAsAbility(), pTarget);
                }
            }
            else
                bCancel = true;

            if (m_bNegated)
                bCancel = true;
        }
    }

    switch (GetActionType())
    {
    case TOOL_ACTION_GLOBAL:
    case TOOL_ACTION_ATTACK:
    case TOOL_ACTION_ATTACK_TOGGLE:
    case TOOL_ACTION_PASSIVE:
    case TOOL_ACTION_TOGGLE:
        break;

    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TARGET_ENTITY:
    case TOOL_ACTION_TARGET_SELF:
    case TOOL_ACTION_FACING:
    case TOOL_ACTION_SELF_POSITION:
        FinishEntity(Game.GetGameIndexFromUniqueID(m_uiTargetUID), bCancel);
        break;

    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_CURSOR:
    case TOOL_ACTION_TARGET_VECTOR:
        FinishPosition(m_v3TargetPos, bCancel);
        break;

    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
        if (m_uiTargetUID != INVALID_INDEX)
            FinishEntity(Game.GetGameIndexFromUniqueID(m_uiTargetUID), bCancel);
        else
            FinishPosition(m_v3TargetPos, bCancel);
        break;
    }

    EEntityActionScript eScript;
    IUnitEntity *pTarget(Game.GetUnitFromUniqueID(m_uiTargetUID));
    CVec3f v3Pos;

    if (pTarget != NULL)
        v3Pos = pTarget->GetPosition();
    else
        v3Pos = m_v3TargetPos;

    if (bCancel)
        eScript = ACTION_SCRIPT_CHANNEL_BROKEN;
    else
        eScript = ACTION_SCRIPT_CHANNEL_END;

    for (uivector_it it(m_vChannelEntityUID.begin()); it != m_vChannelEntityUID.end(); ++it)
    {
        IGameEntity *pEntity(Game.GetEntityFromUniqueID(*it));
        if (pEntity == NULL)
            continue;

        if (pEntity->IsState())
        {
            IEntityState *pState(pEntity->GetAsState());
            if (pState == NULL)
                continue;

            pState->ExecuteActionScript(eScript, pTarget, v3Pos);

            IUnitEntity *pStateOwner(pState->GetOwner());
            if (pStateOwner == NULL)
                continue;

            pStateOwner->ExpireState(pState->GetSlot());
        }
        else if (pEntity->IsUnit())
        {
            IUnitEntity *pUnit(pEntity->GetAsUnit());
            if (pUnit == NULL)
                continue;

            pUnit->ExecuteActionScript(eScript, pTarget, v3Pos);
            pUnit->Kill();
        }
        else if (pEntity->IsAffector())
        {
            IAffector *pAffector(pEntity->GetAsAffector());
            if (pAffector == NULL)
                continue;

            pAffector->ExecuteActionScript(eScript, pTarget, v3Pos);
            pAffector->Expire();
        }
        else if (pEntity->IsProjectile())
        {
            IProjectile *pProjectile(pEntity->GetAsProjectile());
            if (pProjectile == NULL)
                continue;

            pProjectile->ExecuteActionScript(eScript, pTarget, v3Pos);
            pProjectile->Kill();
        }
    }
    m_vChannelEntityUID.clear();

    if (IsAbility())
    {
        if (GetIsChanneling() && !GetNoResponse())
            pOwner->Action(ACTION_SCRIPT_ABILITY_FINISH, pTarget, this);
    }
}

/*====================
  IEntityTool::HasActionScript
  ====================*/
bool    IEntityTool::HasActionScript(EEntityActionScript eScript)
{
    if (m_pDefinition == NULL)
        return false;
        
    CCombatActionScript* pScript(m_pDefinition->GetActionScript(eScript));
    if (pScript == NULL)
        return false;

    return true;
}

/*====================
  IEntityTool::ExecuteActionScript
  ====================*/
void    IEntityTool::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target, CCombatEvent *pCombatEvent)
{
    CGameInfo *pGameInfo(Game.GetGameInfo());
    if (pGameInfo != NULL)
        pGameInfo->ExecuteActionScript(eScript, GetOwner(), this, pTarget, v3Target);

    if (m_pDefinition == NULL)
        return;
        
    m_pDefinition->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, GetProxy(0), GetLevel(), pCombatEvent);
}


/*====================
  IEntityTool::IsReady
  ====================*/
bool    IEntityTool::IsReady() const
{
    for (uiset_cit it(m_setPersistentPetUID.begin()); it != m_setPersistentPetUID.end(); ++it)
    {
        IUnitEntity *pPet(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(*it)));
        if (pPet == NULL)
            continue;
        if (pPet->GetStatus() == ENTITY_STATUS_ACTIVE)
            return false;
    }

    if (GetIgnoreCooldown())
        return true;

    uint uiEndTime(Game.GetCooldownEndTime(m_uiApparentCooldownTime, m_uiApparentCooldownDuration));

    if (uiEndTime == INVALID_TIME)
        return false;
    else if (uiEndTime == 0 || Game.GetGameTime() >= uiEndTime)
        return true;
    else
        return false;
}


/*====================
  IEntityTool::IsValidTarget
  ====================*/
bool    IEntityTool::IsValidTarget(IUnitEntity *pTarget)
{
    if (GetOwner() == NULL)
        return false;

    switch (GetActionType())
    {
    default:
    case TOOL_ACTION_PASSIVE:
        return false;

    case TOOL_ACTION_TOGGLE:
    case TOOL_ACTION_FACING:
    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_CURSOR:
    case TOOL_ACTION_TARGET_VECTOR:
    case TOOL_ACTION_GLOBAL:
    case TOOL_ACTION_SELF_POSITION:
        return true;

    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TARGET_SELF:
        if (pTarget != GetOwner())
            return false;
        return true;

    case TOOL_ACTION_TARGET_ENTITY:
    case TOOL_ACTION_ATTACK:
    case TOOL_ACTION_ATTACK_TOGGLE:
    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
        if (!Game.IsValidTarget(GetTargetScheme(), GetCastEffectType(), GetOwner(), pTarget, GetIgnoreInvulnerable()))
            return false;
        return true;
    }
}


/*====================
  IEntityTool::GetEffect

  Passive tool effects only play owner is alive
  ====================*/
ResHandle   IEntityTool::GetEffect()
{
    if (!IsActive())
        return INVALID_RESOURCE;

    IUnitEntity *pOwner(GetOwner());
    if (!pOwner || pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
        return INVALID_RESOURCE;
    
    return GetPassiveEffect();
}


/*====================
  IEntityTool::Spawn
  ====================*/
void    IEntityTool::Spawn()
{
    SetFlag(ENTITY_TOOL_FLAG_ACTIVE);

    ISlaveEntity::Spawn();
}


/*====================
  IEntityTool::OwnerDamaged
  ====================*/
bool    IEntityTool::OwnerDamaged(CDamageEvent &damage)
{
    if (!ISlaveEntity::OwnerDamaged(damage))
        return false;

    uint uiCooldownOnDamage(GetCooldownOnDamage());
    if (uiCooldownOnDamage > 0)
        StartCooldown(Game.GetGameTime(), uiCooldownOnDamage);
    return true;
}


/*====================
  IEntityTool::UpdateModifiers
  ====================*/
void    IEntityTool::UpdateModifiers(const uivector &vModifiers)
{
    m_vModifierKeys = vModifiers;
    m_vModifierKeys.insert(m_vModifierKeys.end(), m_vPersistentModifierKeys.begin(), m_vPersistentModifierKeys.end());

    uint uiModifierBits(GetModifierBits(m_vModifierKeys));
    if (m_uiActiveModifierKey != INVALID_INDEX)
        uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

    // Activate conditional modifiers
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
    {
        SetModifierBits(uiModifierBits);
        return;
    }

    // Grab base definition
    IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
    if (pDefinition == NULL)
        return;

    const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
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
                const tstring &sType(itType->substr(1));

                if (TStringCompare(sType, _T("toggle_active")) == 0)
                {
                    if (HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
                        break;
                }
                else if (TStringCompare(sType, _T("ready")) == 0)
                {
                    if (IsReady())
                        break;
                }
                else if (pOwner->IsTargetType(sType, pOwner))
                    break;
            }
            else
            {
                const tstring &sType(*itType);

                if (TStringCompare(sType, _T("toggle_active")) == 0)
                {
                    if (!HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
                        break;
                }
                else if (TStringCompare(sType, _T("ready")) == 0)
                {
                    if (!IsReady())
                        break;
                }
                else if (!pOwner->IsTargetType(sType, pOwner))
                    break;
            }
        }
        if (itType == itTypeEnd)
            uiModifierBits |= cit->first;
    }

    SetModifierBits(uiModifierBits);
}


/*====================
  IEntityTool::SelectProxy
  ====================*/
IUnitEntity*    IEntityTool::SelectProxy() const
{
    if (!GetUseProxy())
        return NULL;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return NULL;

    uivector vResult;
    Game.GetEntitiesInRadius(vResult, pOwner->GetPosition().xy(), GetProxySelectionRadius(), REGION_UNIT);
    
    vector<IUnitEntity*> vUnits;
    for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
    {
        IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
        if (pUnit == NULL)
            continue;

        if (!Game.IsValidTarget(GetProxyTargetScheme(), GetProxyEffectType(), pOwner, pUnit, GetProxyAllowInvulnerable()))
            continue;

        if (pUnit == GetOwner())
            continue;

        vUnits.push_back(pUnit);
    }

    if (vUnits.empty())
        return NULL;

    switch (GetProxySelectionMethod())
    {
    case TARGET_SELECT_RANDOM_POSITION:
    case TARGET_SELECT_RANDOM_ANGLE_DISTANCE:
    case TARGET_SELECT_ALL:
    case TARGET_SELECT_RANDOM:
        return vUnits[M_Randnum(0u, INT_SIZE(vUnits.size() - 1))];
        break;

    case TARGET_SELECT_CLOSEST:
        {
            IUnitEntity *pProxy(NULL);
            float fClosest(FAR_AWAY);
            for (vector<IUnitEntity*>::iterator itUnit(vUnits.begin()); itUnit != vUnits.end(); ++itUnit)
            {
                float fDistance(DistanceSq(pOwner->GetPosition(), (*itUnit)->GetPosition()));
                if (fDistance < fClosest)
                {
                    fClosest = fDistance;
                    pProxy = *itUnit;
                }
            }

            return pProxy;
        }
        break;

    case TARGET_SELECT_FURTHEST:
        {
            IUnitEntity *pProxy(NULL);
            float fFurthest(0.0f);
            for (vector<IUnitEntity*>::iterator itUnit(vUnits.begin()); itUnit != vUnits.end(); ++itUnit)
            {
                float fDistance(DistanceSq(pOwner->GetPosition(), (*itUnit)->GetPosition()));
                if (fDistance > fFurthest)
                {
                    fFurthest = fDistance;
                    pProxy = *itUnit;
                }
            }

            return pProxy;
        }
        break;
    }

    return NULL;
}


/*====================
  IEntityTool::StartCooldown
  ====================*/
void    IEntityTool::StartCooldown(uint uiTime, uint uiDuration)
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner != NULL)
    {
        float fCooldownSpeed(pOwner->GetCooldownSpeed());
        float fCooldownReduction(MIN(pOwner->GetReducedCooldowns() - pOwner->GetIncreasedCooldowns(), 1.0f));

        uiDuration = INT_CEIL(uiDuration / fCooldownSpeed * (1.0f - fCooldownReduction));
    }

    uint uiNewEndTime(Game.GetCooldownEndTime(uiTime, uiDuration));
    if (uiNewEndTime > GetCooldownEndTime())
    {
        m_uiCooldownTime = uiTime;
        m_uiCooldownDuration = uiDuration;
    }

    uint uiCooldownType(GetCooldownType());
    if (uiCooldownType != 0)
    {
        if (pOwner != NULL)
            pOwner->StartCooldown(uiCooldownType, uiTime, uiDuration);
    }

    UpdateApparentCooldown();
}


/*====================
  IEntityTool::ReduceCooldown
  ====================*/
void    IEntityTool::ReduceCooldown(uint uiDuration)
{
    IUnitEntity *pOwner(GetOwner());

#if 0 // Scale cooldown reduction by cooldown speed?
    if (pOwner != NULL)
    {
        float fCooldownSpeed(pOwner->GetCooldownSpeed());
        float fCooldownReduction(MIN(pOwner->GetReducedCooldowns() - pOwner->GetIncreasedCooldowns(), 1.0f));

        uiDuration = INT_CEIL(uiDuration / fCooldownSpeed * (1.0f - fCooldownReduction));
    }
#endif

    if (m_uiCooldownDuration != INVALID_TIME)
    {
        if (m_uiCooldownDuration > uiDuration)
            m_uiCooldownDuration -= uiDuration;
        else
            m_uiCooldownDuration = 0;
    }

    uint uiCooldownType(GetCooldownType());
    if (uiCooldownType != 0)
    {
        if (pOwner != NULL)
            pOwner->ReduceCooldown(uiCooldownType, uiDuration);
    }

    UpdateApparentCooldown();
}


/*====================
  IEntityTool::ResetCooldown
  ====================*/
void    IEntityTool::ResetCooldown()
{
    m_uiCooldownTime = INVALID_TIME;
    m_uiCooldownDuration = 0;

    uint uiCooldownType(GetCooldownType());
    if (uiCooldownType != 0)
    {
        IUnitEntity *pOwner(GetOwner());

        if (pOwner != NULL)
            pOwner->ResetCooldown(uiCooldownType);
    }

    UpdateApparentCooldown();
}


/*====================
  IEntityTool::UpdateApparentCooldown
  ====================*/
void    IEntityTool::UpdateApparentCooldown()
{
    uint uiCooldownType(GetCooldownType());
    if (uiCooldownType != 0)
    {
        IUnitEntity *pOwner(GetOwner());
        if (pOwner != NULL)
        {
            uint uiStartTime;
            uint uiDuration;

            pOwner->GetCooldown(uiCooldownType, uiStartTime, uiDuration);

            SetApparentCooldown(uiStartTime, uiDuration);
        }
    }
    else
    {
        m_uiApparentCooldownTime = m_uiCooldownTime;
        m_uiApparentCooldownDuration = m_uiCooldownDuration;
    }
}


/*====================
  IEntityTool::SetApparentCooldown
  ====================*/
void    IEntityTool::SetApparentCooldown(uint uiStartTime, uint uiDuration)
{
    uint uiEndTime(Game.GetCooldownEndTime(m_uiCooldownTime, m_uiCooldownDuration));
    uint uiNewEndTime(Game.GetCooldownEndTime(uiStartTime, uiDuration));

    if (uiNewEndTime > uiEndTime)
    {
        m_uiApparentCooldownTime = uiStartTime;
        m_uiApparentCooldownDuration = uiDuration;
    }
    else
    {
        m_uiApparentCooldownTime = m_uiCooldownTime;
        m_uiApparentCooldownDuration = m_uiCooldownDuration;
    }
}


/*====================
  IEntityTool::GetRemainingCooldownPercent
  ====================*/
float   IEntityTool::GetRemainingCooldownPercent() const
{
    if (GetIgnoreCooldown())
        return 0.0f;

    uint uiEndTime(Game.GetCooldownEndTime(m_uiApparentCooldownTime, m_uiApparentCooldownDuration));

    if (uiEndTime == INVALID_TIME)
        return 1.0f;
    else if (uiEndTime == 0 || Game.GetGameTime() >= uiEndTime)
        return 0.0f;
    else
        return (uiEndTime - Game.GetGameTime()) / float(m_uiApparentCooldownDuration);
}


/*====================
  IEntityTool::GetRemainingCooldownTime
  ====================*/
uint    IEntityTool::GetRemainingCooldownTime() const
{
    if (GetIgnoreCooldown())
        return 0;

    uint uiEndTime(Game.GetCooldownEndTime(m_uiApparentCooldownTime, m_uiApparentCooldownDuration));

    if (uiEndTime == INVALID_TIME)
        return INVALID_TIME;
    else if (uiEndTime == 0 || Game.GetGameTime() >= uiEndTime)
        return 0;
    else
        return uiEndTime - Game.GetGameTime();
}


/*====================
  IEntityTool::GetActualRemainingCooldownPercent
  ====================*/
float   IEntityTool::GetActualRemainingCooldownPercent() const
{
    uint uiEndTime(Game.GetCooldownEndTime(m_uiCooldownTime, m_uiCooldownDuration));

    if (uiEndTime == INVALID_TIME)
        return 1.0f;
    else if (uiEndTime == 0 || Game.GetGameTime() >= uiEndTime)
        return 0.0f;
    else
        return (uiEndTime - Game.GetGameTime()) / float(m_uiCooldownDuration);
}


/*====================
  IEntityTool::GetActualRemainingCooldownTime
  ====================*/
uint    IEntityTool::GetActualRemainingCooldownTime() const
{
    uint uiEndTime(Game.GetCooldownEndTime(m_uiCooldownTime, m_uiCooldownDuration));

    if (uiEndTime == INVALID_TIME)
        return INVALID_TIME;
    else if (uiEndTime == 0 || Game.GetGameTime() >= uiEndTime)
        return 0;
    else
        return uiEndTime - Game.GetGameTime();
}


/*====================
  IEntityTool::Interrupt
  ====================*/
void    IEntityTool::Interrupt(EUnitAction eAction)
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return;

    if (HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE))
    {
        Finish(true);
        ClearFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE);

        if (!GetAnim().empty())
            pOwner->StopAnimation(GetAnim(), GetAnimChannel());
    }
}


/*====================
  IEntityTool::IsChanneling
  ====================*/
bool    IEntityTool::IsChanneling(EUnitAction eAction)
{
    return HasFlag(ENTITY_TOOL_FLAG_CHANNEL_ACTIVE);
}


/*====================
  IEntityTool::ToggleOff
  ====================*/
bool    IEntityTool::ToggleOff()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == NULL)
        return false;

    if (GetActionType() != TOOL_ACTION_TOGGLE || !HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
        return false;

    StartCooldown(Game.GetGameTime(), GetToggleOffCooldownTime());
    ExecuteActionScript(ACTION_SCRIPT_TOGGLE_OFF, pOwner, pOwner->GetPosition());
    ClearFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE);
    return true;
}
