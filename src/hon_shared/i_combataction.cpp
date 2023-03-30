// (C)2008 S2 Games
// i_combataction.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_combataction.h"
#include "i_unitentity.h"
#include "i_heroentity.h"
#include "i_entitytool.h"
#include "i_areaaffector.h"
#include "i_behavior.h"
#include "c_scriptthread.h"
//=============================================================================

/*====================
  ICombatAction::GetEntityFromActionTarget
  ====================*/
IGameEntity*    ICombatAction::GetEntityFromActionTarget(EActionTarget eTarget) const
{
    switch (eTarget)
    {
    case ACTION_TARGET_INVALID:                     return NULL;
    case ACTION_TARGET_SOURCE_ENTITY:               return m_pEnv->pInitiator;
    case ACTION_TARGET_SOURCE_POSITION:             return NULL;
    case ACTION_TARGET_SOURCE_TARGET_OFFSET:        return NULL;
    case ACTION_TARGET_SOURCE_ATTACK_OFFSET:        return NULL;
    case ACTION_TARGET_SOURCE_OWNER_ENTITY:         return m_pEnv->pInitiator ? m_pEnv->pInitiator->GetOwner() : NULL;
    case ACTION_TARGET_SOURCE_OWNER_POSITION:       return NULL;
    case ACTION_TARGET_TARGET_ENTITY:               return m_pEnv->pTarget;
    case ACTION_TARGET_TARGET_POSITION:             return NULL;
    case ACTION_TARGET_TARGET_TARGET_OFFSET:        return NULL;
    case ACTION_TARGET_TARGET_ATTACK_OFFSET:        return NULL;
    case ACTION_TARGET_TARGET_OWNER_ENTITY:         return m_pEnv->pTarget ? m_pEnv->pTarget->GetOwner() : NULL;
    case ACTION_TARGET_TARGET_OWNER_POSITION:       return NULL;
    case ACTION_TARGET_INFLICTOR_ENTITY:            return m_pEnv->pInflictor;
    case ACTION_TARGET_INFLICTOR_POSITION:          return NULL;
    case ACTION_TARGET_INFLICTOR_TARGET_OFFSET:     return NULL;
    case ACTION_TARGET_INFLICTOR_OWNER_ENTITY:      return m_pEnv->pInflictor ? m_pEnv->pInflictor->GetOwner() : NULL;
    case ACTION_TARGET_INFLICTOR_OWNER_POSITION:    return NULL;
    case ACTION_TARGET_PROXY_ENTITY:                return m_pEnv->pProxy;
    case ACTION_TARGET_PROXY_POSITION:              return NULL;
    case ACTION_TARGET_STACK_ENTITY:                return Game.GetEntityFromUniqueID(PeekEntity());
    case ACTION_TARGET_STACK_POSITION:              return NULL;
    case ACTION_TARGET_THIS_ENTITY:                 return m_pEnv->pThis;
    case ACTION_TARGET_THIS_POSITION:               return NULL;
    case ACTION_TARGET_THIS_TARGET_OFFSET:          return NULL;
    case ACTION_TARGET_THIS_ATTACK_OFFSET:          return NULL;
    case ACTION_TARGET_THIS_OWNER_ENTITY:           return m_pEnv->pThis ? m_pEnv->pThis->GetOwner() : NULL;
    case ACTION_TARGET_THIS_OWNER_POSITION:         return NULL;
    case ACTION_TARGET_THIS_INFLICTOR_ENTITY:       return m_pEnv->pThis && m_pEnv->pThis->IsState() ? m_pEnv->pThis->GetAsState()->GetInflictor() : NULL;
    case ACTION_TARGET_THIS_INFLICTOR_POSITION:     return NULL;
    case ACTION_TARGET_THIS_SPAWNER_ENTITY:         return m_pEnv->pThis && m_pEnv->pThis->IsState() ? m_pEnv->pThis->GetAsState()->GetSpawner() : NULL;
    case ACTION_TARGET_THIS_SPAWNER_POSITION:       return NULL;
    case ACTION_TARGET_THIS_TARGET_ENTITY:          return m_pEnv->pThis && m_pEnv->pThis->IsUnit() ? Game.GetEntity(m_pEnv->pThis->GetAsUnit()->GetTargetIndex()) : NULL;
    case ACTION_TARGET_THIS_TARGET_POSITION:        return NULL;
    case ACTION_TARGET_THIS_OWNER_TARGET_ENTITY:    return m_pEnv->pThis && m_pEnv->pThis->GetOwner() && m_pEnv->pThis->GetOwner()->IsUnit() ? Game.GetEntity(m_pEnv->pThis->GetOwner()->GetAsUnit()->GetTargetIndex()) : NULL;
    case ACTION_TARGET_THIS_OWNER_TARGET_POSITION:  return NULL;
    case ACTION_TARGET_THIS_PROXY_ENTITY:           return m_pEnv->pThis ? m_pEnv->pThis->GetProxy(0) : NULL;
    case ACTION_TARGET_THIS_PROXY_POSITION:         return NULL;
    case ACTION_TARGET_THIS_PROXY_ENTITY1:          return m_pEnv->pThis ? m_pEnv->pThis->GetProxy(1) : NULL;
    case ACTION_TARGET_THIS_PROXY_POSITION1:        return NULL;
    case ACTION_TARGET_THIS_PROXY_ENTITY2:          return m_pEnv->pThis ? m_pEnv->pThis->GetProxy(2) : NULL;
    case ACTION_TARGET_THIS_PROXY_POSITION2:        return NULL;
    case ACTION_TARGET_THIS_PROXY_ENTITY3:          return m_pEnv->pThis ? m_pEnv->pThis->GetProxy(3) : NULL;
    case ACTION_TARGET_THIS_PROXY_POSITION3:        return NULL;
    case ACTION_TARGET_DELTA_POSITION:              return NULL;
    case ACTION_TARGET_POS0:                        return NULL;
    case ACTION_TARGET_POS1:                        return NULL;
    case ACTION_TARGET_POS2:                        return NULL;
    case ACTION_TARGET_POS3:                        return NULL;
    case ACTION_TARGET_ENT0:                        return m_pEnv->pEnt0;
    case ACTION_TARGET_ENT0_POSITION:               return NULL;
    case ACTION_TARGET_ENT1:                        return m_pEnv->pEnt1;
    case ACTION_TARGET_ENT1_POSITION:               return NULL;
    case ACTION_TARGET_ENT2:                        return m_pEnv->pEnt2;
    case ACTION_TARGET_ENT2_POSITION:               return NULL;
    case ACTION_TARGET_ENT3:                        return m_pEnv->pEnt3;
    case ACTION_TARGET_ENT3_POSITION:               return NULL;
    }

    return NULL;
}


/*====================
  ICombatAction::GetUnitFromActionTarget
  ====================*/
IUnitEntity*    ICombatAction::GetUnitFromActionTarget(EActionTarget eTarget) const
{
    IGameEntity *pEntity(GetEntityFromActionTarget(eTarget));
    if (pEntity == NULL)
        return NULL;

    return pEntity->GetAsUnit();
}


/*====================
  ICombatAction::GetPositionFromActionTarget
  ====================*/
CVec3f  ICombatAction::GetPositionFromActionTarget(EActionTarget eTarget) const
{
    switch (eTarget)
    {
    case ACTION_TARGET_INVALID:
        return V_ZERO;

    case ACTION_TARGET_SOURCE_ENTITY:
    case ACTION_TARGET_SOURCE_POSITION:
        if (m_pEnv->pInitiator == NULL || !m_pEnv->pInitiator->GetAsVisual())
            return V_ZERO;
        else
            return m_pEnv->pInitiator->GetAsVisual()->GetPosition();

    case ACTION_TARGET_SOURCE_ATTACK_OFFSET:
        if (m_pEnv->pInitiator == NULL)
            return V_ZERO;
        else if (m_pEnv->pInitiator->IsUnit())
            return m_pEnv->pInitiator->GetAsUnit()->GetPosition() + m_pEnv->pInitiator->GetAsUnit()->GetAttackOffset();
        else if (m_pEnv->pInitiator->IsVisual())
            return m_pEnv->pInitiator->GetAsVisual()->GetPosition();
        else
            return V_ZERO;
            
    case ACTION_TARGET_SOURCE_TARGET_OFFSET:
        if (m_pEnv->pInitiator == NULL)
            return V_ZERO;
        else if (m_pEnv->pInitiator->IsUnit())
            return m_pEnv->pInitiator->GetAsUnit()->GetPosition() + m_pEnv->pInitiator->GetAsUnit()->GetTargetOffset();
        else if (m_pEnv->pInitiator->IsVisual())
            return m_pEnv->pInitiator->GetAsVisual()->GetPosition();
        else
            return V_ZERO;

    case ACTION_TARGET_SOURCE_OWNER_ENTITY:
    case ACTION_TARGET_SOURCE_OWNER_POSITION:
        if (m_pEnv->pInitiator == NULL)
            return V_ZERO;
        else if (m_pEnv->pInitiator->GetOwner() == NULL)
            return V_ZERO;
        else
            return m_pEnv->pInitiator->GetOwner()->GetPosition();

    case ACTION_TARGET_TARGET_ENTITY:
        if (m_pEnv->pTarget == NULL || !m_pEnv->pTarget->IsVisual())
            return V_ZERO;
        return m_pEnv->pTarget->GetAsVisual()->GetPosition();

    case ACTION_TARGET_TARGET_POSITION:
        return m_pEnv->v3Target;

    case ACTION_TARGET_TARGET_ATTACK_OFFSET:
        if (m_pEnv->pTarget == NULL)
            return V_ZERO;
        else if (m_pEnv->pTarget->IsUnit())
            return m_pEnv->pTarget->GetAsUnit()->GetTransformedAttackOffset();
        else if (m_pEnv->pTarget->IsVisual())
            return m_pEnv->pTarget->GetAsVisual()->GetPosition();
        else
            return V_ZERO;

    case ACTION_TARGET_TARGET_TARGET_OFFSET: // ROFL!
        if (m_pEnv->pTarget == NULL)
            return V_ZERO;
        else if (m_pEnv->pTarget->IsUnit())
            return m_pEnv->pTarget->GetAsUnit()->GetTransformedTargetOffset();
        else if (m_pEnv->pTarget->IsVisual())
            return m_pEnv->pTarget->GetAsVisual()->GetPosition();
        else
            return V_ZERO;

    case ACTION_TARGET_TARGET_OWNER_ENTITY:
    case ACTION_TARGET_TARGET_OWNER_POSITION:
        if (m_pEnv->pTarget == NULL)
            return V_ZERO;
        else if (m_pEnv->pTarget->GetOwner() == NULL)
            return V_ZERO;
        else
            return m_pEnv->pTarget->GetOwner()->GetPosition();

    case ACTION_TARGET_INFLICTOR_ENTITY:
    case ACTION_TARGET_INFLICTOR_POSITION:
        if (m_pEnv->pInflictor == NULL || !m_pEnv->pInflictor->IsVisual())
            return V_ZERO;
        return m_pEnv->pInflictor->GetAsVisual()->GetPosition();

    case ACTION_TARGET_INFLICTOR_TARGET_OFFSET:
        if (m_pEnv->pInflictor == NULL || !m_pEnv->pInflictor->IsUnit())
            return V_ZERO;
        return m_pEnv->pInflictor->GetAsUnit()->GetTransformedTargetOffset();

    case ACTION_TARGET_INFLICTOR_OWNER_ENTITY:
    case ACTION_TARGET_INFLICTOR_OWNER_POSITION:
        if (m_pEnv->pInflictor == NULL || !m_pEnv->pInflictor->IsUnit() || m_pEnv->pInflictor->GetAsUnit()->GetOwner() == NULL)
            return V_ZERO;
        return m_pEnv->pInflictor->GetAsUnit()->GetOwner()->GetPosition();

    case ACTION_TARGET_PROXY_ENTITY:
    case ACTION_TARGET_PROXY_POSITION:
        if (m_pEnv->pProxy == NULL || !m_pEnv->pProxy->IsVisual())
            return V_ZERO;
        return m_pEnv->pProxy->GetAsVisual()->GetPosition();

    case ACTION_TARGET_STACK_ENTITY:
    case ACTION_TARGET_STACK_POSITION:
        {
            IGameEntity *pEntity(Game.GetEntityFromUniqueID(PeekEntity()));
            if (pEntity == NULL || !pEntity->IsVisual())
                return V_ZERO;

            return pEntity->GetAsVisual()->GetPosition();
        }

    case ACTION_TARGET_THIS_ENTITY:
    case ACTION_TARGET_THIS_POSITION:
        if (m_pEnv->pThis == NULL || m_pEnv->pThis->GetAsVisual() == NULL)
            return V_ZERO;
        return m_pEnv->pThis->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_TARGET_OFFSET:
        if (m_pEnv->pThis == NULL || m_pEnv->pThis->GetAsUnit() == NULL)
            return V_ZERO;
        return m_pEnv->pThis->GetAsUnit()->GetTransformedTargetOffset();

    case ACTION_TARGET_THIS_ATTACK_OFFSET:
        if (m_pEnv->pThis == NULL || m_pEnv->pThis->GetAsUnit() == NULL)
            return V_ZERO;
        return m_pEnv->pThis->GetAsUnit()->GetTransformedAttackOffset();

    case ACTION_TARGET_THIS_OWNER_ENTITY:
    case ACTION_TARGET_THIS_OWNER_POSITION:
        if (m_pEnv->pThis == NULL || m_pEnv->pThis->GetOwner() == NULL)
            return V_ZERO;
        return m_pEnv->pThis->GetOwner()->GetPosition();

    case ACTION_TARGET_THIS_INFLICTOR_ENTITY:
    case ACTION_TARGET_THIS_INFLICTOR_POSITION:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetAsState() == NULL ||
            m_pEnv->pThis->GetAsState()->GetInflictor() == NULL ||
            m_pEnv->pThis->GetAsState()->GetInflictor()->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetAsState()->GetInflictor()->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_SPAWNER_ENTITY:
    case ACTION_TARGET_THIS_SPAWNER_POSITION:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetAsState() == NULL ||
            m_pEnv->pThis->GetAsState()->GetInflictor() == NULL ||
            m_pEnv->pThis->GetAsState()->GetInflictor()->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetAsState()->GetSpawner()->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_TARGET_ENTITY:
    case ACTION_TARGET_THIS_TARGET_POSITION:
        {
            if (m_pEnv->pThis == NULL || !m_pEnv->pThis->IsUnit())
                return V_ZERO;

            IUnitEntity *pTarget(Game.GetUnitEntity(m_pEnv->pThis->GetAsUnit()->GetTargetIndex()));
            if (pTarget == NULL)
                return V_ZERO;

            return pTarget->GetPosition();
        }

    case ACTION_TARGET_THIS_OWNER_TARGET_ENTITY:
    case ACTION_TARGET_THIS_OWNER_TARGET_POSITION:
        {
            if (m_pEnv->pThis == NULL || !m_pEnv->pThis->GetOwner()->IsUnit())
                return V_ZERO;

            IUnitEntity *pTarget(Game.GetUnitEntity(m_pEnv->pThis->GetOwner()->GetAsUnit()->GetTargetIndex()));
            if (pTarget == NULL)
                return V_ZERO;

            return pTarget->GetPosition();
        }

    case ACTION_TARGET_THIS_PROXY_ENTITY:
    case ACTION_TARGET_THIS_PROXY_POSITION:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetProxy(0) == NULL ||
            m_pEnv->pThis->GetProxy(0)->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetProxy(0)->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_PROXY_ENTITY1:
    case ACTION_TARGET_THIS_PROXY_POSITION1:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetProxy(1) == NULL ||
            m_pEnv->pThis->GetProxy(1)->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetProxy(1)->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_PROXY_ENTITY2:
    case ACTION_TARGET_THIS_PROXY_POSITION2:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetProxy(2) == NULL ||
            m_pEnv->pThis->GetProxy(2)->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetProxy(2)->GetAsVisual()->GetPosition();

    case ACTION_TARGET_THIS_PROXY_ENTITY3:
    case ACTION_TARGET_THIS_PROXY_POSITION3:
        if (m_pEnv->pThis == NULL ||
            m_pEnv->pThis->GetProxy(3) == NULL ||
            m_pEnv->pThis->GetProxy(3)->GetAsVisual() == NULL)
            return V_ZERO;

        return m_pEnv->pThis->GetProxy(3)->GetAsVisual()->GetPosition();

    case ACTION_TARGET_DELTA_POSITION:
        return m_pEnv->v3Target + m_pEnv->v3Delta;

    case ACTION_TARGET_POS0:
        return m_pEnv->v3Pos0;

    case ACTION_TARGET_POS1:
        return m_pEnv->v3Pos1;

    case ACTION_TARGET_POS2:
        return m_pEnv->v3Pos2;

    case ACTION_TARGET_POS3:
        return m_pEnv->v3Pos3;

    case ACTION_TARGET_ENT0:
    case ACTION_TARGET_ENT0_POSITION:
        if (m_pEnv->pEnt0 == NULL || !m_pEnv->pEnt0->GetAsVisual())
            return V_ZERO;
        else
            return m_pEnv->pEnt0->GetAsVisual()->GetPosition();

    case ACTION_TARGET_ENT1:
    case ACTION_TARGET_ENT1_POSITION:
        if (m_pEnv->pEnt1 == NULL || !m_pEnv->pEnt1->GetAsVisual())
            return V_ZERO;
        else
            return m_pEnv->pEnt1->GetAsVisual()->GetPosition();

    case ACTION_TARGET_ENT2:
    case ACTION_TARGET_ENT2_POSITION:
        if (m_pEnv->pEnt2 == NULL || !m_pEnv->pEnt2->GetAsVisual())
            return V_ZERO;
        else
            return m_pEnv->pEnt2->GetAsVisual()->GetPosition();

    case ACTION_TARGET_ENT3:
    case ACTION_TARGET_ENT3_POSITION:
        if (m_pEnv->pEnt3 == NULL || !m_pEnv->pEnt3->GetAsVisual())
            return V_ZERO;
        else
            return m_pEnv->pEnt3->GetAsVisual()->GetPosition();
    }

    return V_ZERO;
}


/*====================
  ICombatAction::GetDynamicValue
  ====================*/
float   ICombatAction::GetDynamicValue(EDynamicActionValue eMultiplier) const
{
    PROFILE("ICombatAction::GetDynamicValue");

    switch (eMultiplier)
    {
    case DYNAMIC_VALUE_INVALID:
        return 0.0f;

    case DYNAMIC_VALUE_RESULT:
        return m_pEnv->fResult;

    case DYNAMIC_VALUE_STACK:
        return PeekStack();

    case DYNAMIC_VALUE_VAR0:
        return m_pEnv->fVar0;

    case DYNAMIC_VALUE_VAR1:
        return m_pEnv->fVar1;

    case DYNAMIC_VALUE_VAR2:
        return m_pEnv->fVar2;

    case DYNAMIC_VALUE_VAR3:
        return m_pEnv->fVar3;

    case DYNAMIC_VALUE_PER_SECOND:
        return MsToSec(Game.GetFrameLength());
    
    case DYNAMIC_VALUE_CHARGES:
        if (m_pEnv->pThis == NULL)
            return 0.0f;

        if (m_pEnv->pThis->IsSlave())
            return m_pEnv->pThis->GetAsSlave()->GetCharges();
        else if (m_pEnv->pThis->IsProjectile())
            return m_pEnv->pThis->GetAsProjectile()->GetCharges();

        return 0.0f;

    case DYNAMIC_VALUE_LIFETIME_REMAINING:
        {
            uint uiExpireTime(INVALID_TIME);
            if (m_pEnv->pThis != NULL && m_pEnv->pThis->IsState())
                uiExpireTime = m_pEnv->pThis->GetAsState()->GetExpireTime();

            if (uiExpireTime == INVALID_TIME || uiExpireTime <= Game.GetGameTime())
                return 0.0f;
            else
                return uiExpireTime - Game.GetGameTime();
        }
        break;

    case DYNAMIC_VALUE_HEALTH_LOST:
        {
            ISlaveEntity *pSlave(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsSlave() : NULL);
            if (pSlave == NULL)
                return 0.0f;

            IUnitEntity *pOwner(pSlave->GetOwner());
            if (pOwner != NULL)
                return MAX(pSlave->GetAccumulator() - pOwner->GetHealth(), 0.0f);

            return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_MOVEMENT:
        {
            ISlaveEntity *pSlave(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsSlave() : NULL);
            if (pSlave == NULL)
                return 0.0f;

            IUnitEntity *pOwner(pSlave->GetOwner());
            if (pOwner != NULL)
                return pOwner->GetMovementDistance();

            return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_ACCUMULATOR:
        {
            if (m_pEnv->pThis == NULL)
                return 0.0f;
            else if (m_pEnv->pThis->IsSlave())
                return m_pEnv->pThis->GetAsSlave()->GetAccumulator();
            else if (m_pEnv->pThis->IsUnit())
                return m_pEnv->pThis->GetAsUnit()->GetAccumulator();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_PARAM:
        {
            if (m_pEnv->pThis == NULL)
                return 0.0f;
            else if (m_pEnv->pThis->IsAffector())
                return m_pEnv->pThis->GetAsAffector()->GetParam();
            else if (m_pEnv->pThis->IsProjectile())
                return m_pEnv->pThis->GetAsProjectile()->GetParam();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_LIFETIME:
        {
            if (m_pEnv->pThis == NULL)
                return 0.0f;
            else if (m_pEnv->pThis->IsState())
                return m_pEnv->pThis->GetAsState()->GetRemainingLifetime();
            else if (m_pEnv->pThis->IsUnit())
                return m_pEnv->pThis->GetAsUnit()->GetRemainingLifetime();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_TEAM:
        {
            if (m_pEnv->pThis == NULL)
                return float(int(INVALID_TIME));
            else if (m_pEnv->pThis->IsUnit())
                return float(int(m_pEnv->pThis->GetAsUnit()->GetTeam()));
            else
                return float(int(INVALID_TIME));
        }
        break;

    case DYNAMIC_VALUE_TOTAL_ADJUSTED_DAMAGE:
        return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetTotalAdjustedDamage() : 0.0f;

    case DYNAMIC_VALUE_APPLIED_DAMAGE:
        return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetAppliedDamage() : 0.0f;

    case DYNAMIC_VALUE_TARGET_DAMAGE: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetCurrentDamage() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_HEALTH: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetHealth() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_HEALTH_PERCENT: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetHealthPercent() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_TOTAL_HEALTH: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMaxHealth() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MISSING_HEALTH: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMaxHealth() - GetTargetUnit()->GetHealth() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MISSING_HEALTH_PERCENT: return (GetTargetUnit() != NULL) ? 1.0f - GetTargetUnit()->GetHealthPercent() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MANA: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMana() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MANA_PERCENT: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetManaPercent() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_TOTAL_MANA: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMaxMana() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MISSING_MANA: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMaxMana() - GetTargetUnit()->GetMana() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_MISSING_MANA_PERCENT: return (GetTargetUnit() != NULL) ? 1.0f - GetTargetUnit()->GetManaPercent() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_STRENGTH: return (GetTargetEntity()->GetAsHero() != NULL) ? GetTargetEntity()->GetAsHero()->GetStrength() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_AGILITY: return (GetTargetEntity()->GetAsHero() != NULL) ? GetTargetEntity()->GetAsHero()->GetAgility() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_INTELLIGENCE: return (GetTargetEntity()->GetAsHero() != NULL) ? GetTargetEntity()->GetAsHero()->GetIntelligence() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_ATTACKSPEED: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetAttackSpeed() : 1.0f; break;
    case DYNAMIC_VALUE_TARGET_CASTSPEED: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetCastSpeed() : 1.0f; break;
    case DYNAMIC_VALUE_TARGET_MOVESPEED: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetMoveSpeed() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_ATTACKACTIONTIME: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetAdjustedAttackActionTime() : 1.0f; break;
    case DYNAMIC_VALUE_TARGET_ATTACKDURATION: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetAdjustedAttackDuration() : 1.0f; break;
    case DYNAMIC_VALUE_TARGET_ATTACKCOOLDOWN: return (GetTargetUnit() != NULL) ? GetTargetUnit()->GetAdjustedAttackCooldown() : 1.0f; break;
    
    case DYNAMIC_VALUE_TARGET_ATTACKDAMAGE:
        {
            IUnitEntity *pTarget(GetTargetUnit());
            if (pTarget == NULL)
                return 0.0f;
            
            float fBaseDamage(pTarget->GetBaseDamage());
            float fAdditionalDamage(pTarget->GetBonusDamage());
            float fDamageMultiplier(pTarget->GetTotalDamageMultiplier());

            return (fBaseDamage + fAdditionalDamage) * fDamageMultiplier;
        }
        break;
    
    case DYNAMIC_VALUE_TARGET_ACCUMULATOR:
        {
            IGameEntity *pTarget(GetTargetEntity());

            if (pTarget == NULL)
                return 0.0f;
            else if (pTarget->IsSlave())
                return pTarget->GetAsSlave()->GetAccumulator();
            else if (pTarget->IsUnit())
                return pTarget->GetAsUnit()->GetAccumulator();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_TARGET_LIFETIME:
        {
            IGameEntity *pTarget(GetTargetEntity());

            if (pTarget == NULL)
                return 0.0f;
            else if (pTarget->IsState())
                return pTarget->GetAsState()->GetRemainingLifetime();
            else if (pTarget->IsUnit())
                return pTarget->GetAsUnit()->GetRemainingLifetime();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_TARGET_TEAM:
        {
            IGameEntity *pTarget(GetTargetEntity());

            if (pTarget == NULL)
                return float(int(INVALID_TIME));
            else if (pTarget->IsUnit())
                return float(int(pTarget->GetAsUnit()->GetTeam()));
            else if (pTarget->IsPlayer())
                return float(int(pTarget->GetAsPlayer()->GetTeam()));
            else
                return float(int(INVALID_TIME));
        }
        break;
    
    case DYNAMIC_VALUE_SOURCE_DAMAGE: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetCurrentDamage() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_HEALTH: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetHealth() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_HEALTH_PERCENT: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetHealthPercent() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_TOTAL_HEALTH: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMaxHealth() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MISSING_HEALTH: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMaxHealth() - GetSourceUnit()->GetHealth() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MISSING_HEALTH_PERCENT: return (GetSourceUnit() != NULL) ? 1.0f - GetSourceUnit()->GetHealthPercent() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MANA: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMana() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MANA_PERCENT: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetManaPercent() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_TOTAL_MANA: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMaxMana() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MISSING_MANA: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMaxMana() - GetSourceUnit()->GetMana() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_MISSING_MANA_PERCENT: return (GetSourceUnit() != NULL) ? 1.0f - GetSourceUnit()->GetManaPercent() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_STRENGTH: return (GetSourceEntity()->GetAsHero() != NULL) ? GetSourceEntity()->GetAsHero()->GetStrength() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_AGILITY: return (GetSourceEntity()->GetAsHero() != NULL) ? GetSourceEntity()->GetAsHero()->GetAgility() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_INTELLIGENCE: return (GetSourceEntity()->GetAsHero() != NULL) ? GetSourceEntity()->GetAsHero()->GetIntelligence() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_ATTACKSPEED: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetAttackSpeed() : 1.0f; break;
    case DYNAMIC_VALUE_SOURCE_CASTSPEED: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetCastSpeed() : 1.0f; break;
    case DYNAMIC_VALUE_SOURCE_MOVESPEED: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetMoveSpeed() : 0.0f; break;
    case DYNAMIC_VALUE_SOURCE_ATTACKACTIONTIME: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetAdjustedAttackActionTime() : 1.0f; break;
    case DYNAMIC_VALUE_SOURCE_ATTACKDURATION: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetAdjustedAttackDuration() : 1.0f; break;
    case DYNAMIC_VALUE_SOURCE_ATTACKCOOLDOWN: return (GetSourceUnit() != NULL) ? GetSourceUnit()->GetAdjustedAttackCooldown() : 1.0f; break;
    
    case DYNAMIC_VALUE_SOURCE_ATTACKDAMAGE:
        {
            IUnitEntity *pSource(GetSourceUnit());
            if (pSource == NULL)
                return 0.0f;
            
            float fBaseDamage(pSource->GetBaseDamage());
            float fAdditionalDamage(pSource->GetBonusDamage());
            float fDamageMultiplier(pSource->GetTotalDamageMultiplier());

            return (fBaseDamage + fAdditionalDamage) * fDamageMultiplier;
        }
        break;

    case DYNAMIC_VALUE_SOURCE_ACCUMULATOR:
        {
            IGameEntity *pSource(GetSourceEntity());

            if (pSource == NULL)
                return 0.0f;
            else if (pSource->IsSlave())
                return pSource->GetAsSlave()->GetAccumulator();
            else if (pSource->IsUnit())
                return pSource->GetAsUnit()->GetAccumulator();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_SOURCE_LIFETIME:
        {
            IGameEntity *pSource(GetSourceEntity());

            if (pSource == NULL)
                return 0.0f;
            else if (pSource->IsState())
                return pSource->GetAsState()->GetRemainingLifetime();
            else if (pSource->IsUnit())
                return pSource->GetAsUnit()->GetRemainingLifetime();
            else
                return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_SOURCE_TEAM:
        {
            IGameEntity *pSource(GetSourceEntity());

            if (pSource == NULL)
                return float(int(INVALID_TIME));
            else if (pSource->IsUnit())
                return float(int(pSource->GetAsUnit()->GetTeam()));
            else if (pSource->IsPlayer())
                return float(int(pSource->GetAsPlayer()->GetTeam()));
            else
                return float(int(INVALID_TIME));
        }
        break;

    case DYNAMIC_VALUE_OWNER_COUNTER:
        {
            ISlaveEntity *pSlave(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsSlave() : NULL);
            if (pSlave == NULL)
                return 0.0f;

            IUnitEntity *pOwner(pSlave->GetOwner());
            if (pOwner != NULL)
                return pOwner->GetCounter();

            return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_SOURCE_ENTITY: return (GetSourceEntity() != NULL) ? GetSourceEntity()->GetUniqueID() : 0.0f; break;
    case DYNAMIC_VALUE_TARGET_ENTITY: return (GetTargetEntity() != NULL) ? GetTargetEntity()->GetUniqueID() : 0.0f; break;
    case DYNAMIC_VALUE_INFLICTOR_ENTITY: return (m_pEnv->pInflictor != NULL) ? m_pEnv->pInflictor->GetUniqueID() : 0.0f; break;
    case DYNAMIC_VALUE_PROXY_ENTITY: return (m_pEnv->pProxy != NULL) ? m_pEnv->pProxy->GetUniqueID() : 0.0f; break;
    case DYNAMIC_VALUE_STACK_ENTITY: return (Game.GetEntityFromUniqueID(PeekEntity()) != NULL) ? PeekEntity() : 0.0f; break;
    case DYNAMIC_VALUE_THIS_PROXY_ENTITY: return (m_pEnv->pThis != NULL && m_pEnv->pThis->GetProxy(0) != NULL) ? m_pEnv->pThis->GetProxy(0)->GetUniqueID() : 0.0f; break;

    case DYNAMIC_VALUE_OWNER_ENTITY:
        {
            ISlaveEntity *pSlave(m_pEnv->pThis != NULL ? m_pEnv->pThis->GetAsSlave() : NULL);
            if (pSlave == NULL)
                return 0.0f;

            IUnitEntity *pOwner(pSlave->GetOwner());
            if (pOwner != NULL)
                return pOwner->GetUniqueID();

            return 0.0f;
        }
        break;

    case DYNAMIC_VALUE_TIME: return (Game.GetGamePhase() == GAME_PHASE_ACTIVE) ? Game.GetGameTime() - Game.GetPhaseStartTime() : 0.0f; break;
    case DYNAMIC_VALUE_CASTDURATION: return (m_pEnv->pInflictor != NULL && m_pEnv->pInflictor->IsTool()) ? m_pEnv->pInflictor->GetAsTool()->GetAdjustedCastTime() : 0.0f; break;
    case DYNAMIC_VALUE_LEVEL: return m_pEnv->uiLevel; break;

    case DYNAMIC_COMBAT_TARGET: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetTarget() : INVALID_INDEX;
    case DYNAMIC_COMBAT_EFFECTTYPE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetEffectType() : 0;
    case DYNAMIC_COMBAT_DAMAGETYPE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetDamageType() : 0;
    case DYNAMIC_COMBAT_SUPERTYPE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetSuperType() : 0;
    case DYNAMIC_COMBAT_BASEDAMAGE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetBaseDamage() : 0.0f;
    case DYNAMIC_COMBAT_ADDITIONALDAMAGE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetAdditionalDamage() : 0.0f;
    case DYNAMIC_COMBAT_DAMAGEMULTIPLIER: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetDamageMultiplier() : 1.0f;
    case DYNAMIC_COMBAT_BONUSDAMAGE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetBonusDamage() : 0.0f;
    case DYNAMIC_COMBAT_BONUSMULTIPLIER: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetBonusMultiplier() : 0.0f;
    case DYNAMIC_COMBAT_LIFESTEAL: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetLifeSteal() : 0.0f;
    case DYNAMIC_COMBAT_EVASION: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetEvasion() : 0.0f;
    case DYNAMIC_COMBAT_MISSCHANCE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetMissChance() : 0.0f;
    case DYNAMIC_COMBAT_NONLETHAL: return m_pEnv->pCombatEvent && m_pEnv->pCombatEvent->GetNonLethal() ? 1.0f : 0.0f;
    case DYNAMIC_COMBAT_TRUESTRIKE: return m_pEnv->pCombatEvent && m_pEnv->pCombatEvent->GetTrueStrike() ? 1.0f : 0.0f;
    case DYNAMIC_COMBAT_NEGATED: return m_pEnv->pCombatEvent && m_pEnv->pCombatEvent->GetNegated() ? 1.0f : 0.0f;
    case DYNAMIC_COMBAT_DEFLECTION: return m_pEnv->pCombatEvent && m_pEnv->pCombatEvent->GetDeflection() ? 1.0f : 0.0f;
    case DYNAMIC_COMBAT_MANACOST: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetManaCost() : 0.0f;
    case DYNAMIC_COMBAT_COOLDOWNTIME: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetCooldownTime() : 0.0f;
    case DYNAMIC_COMBAT_ATTACKABILITY: return m_pEnv->pCombatEvent && m_pEnv->pCombatEvent->GetAttackAbility() ? 1.0f : 0.0f;
    case DYNAMIC_COMBAT_ARMORPIERCE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetArmorPierce() : 0.0f;
    case DYNAMIC_COMBAT_MAGICARMORPIERCE: return m_pEnv->pCombatEvent ? m_pEnv->pCombatEvent->GetMagicArmorPierce() : 0.0f;

    case DYNAMIC_DAMAGE_SUPERTYPE: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetSuperType() : 0;
    case DYNAMIC_DAMAGE_EFFECTTYPE: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetEffectType() : 0;
    case DYNAMIC_DAMAGE_ATTEMPTED: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetAttemptedDamage() : 0.0f;
    case DYNAMIC_DAMAGE_APPLIED: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetAppliedDamage() : 0.0f;
    case DYNAMIC_DAMAGE_DEFLECTION: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetDeflection() : 0.0f;
    case DYNAMIC_DAMAGE_TARGET: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetTargetIndex() : INVALID_INDEX;
    case DYNAMIC_DAMAGE_ATTACKER: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetAttackerIndex() : INVALID_INDEX;
    case DYNAMIC_DAMAGE_INFLICTOR: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetInflictorIndex() : INVALID_INDEX;
    case DYNAMIC_DAMAGE_ARMORPIERCE: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetArmorPierce() : 0.0f;
    case DYNAMIC_DAMAGE_MAGICARMORPIERCE: return m_pEnv->pDamageEvent ? m_pEnv->pDamageEvent->GetMagicArmorPierce() : 0.0f;
    }

    return 0.0f;
}


/*====================
  ICombatAction::SetDynamicValue
  ====================*/
void    ICombatAction::SetDynamicValue(EDynamicActionValue eMultiplier, float fValue)
{
    switch (eMultiplier)
    {
    case DYNAMIC_VALUE_TOTAL_ADJUSTED_DAMAGE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetTotalAdjustedDamage(fValue); break;

    case DYNAMIC_COMBAT_BASEDAMAGE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetBaseDamage(fValue); break;
    case DYNAMIC_COMBAT_ADDITIONALDAMAGE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetAdditionalDamage(fValue); break;
    case DYNAMIC_COMBAT_DAMAGEMULTIPLIER: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetDamageMultiplier(fValue); break;
    case DYNAMIC_COMBAT_BONUSDAMAGE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetBonusDamage(fValue); break;
    case DYNAMIC_COMBAT_BONUSMULTIPLIER: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetBonusMultiplier(fValue); break;
    case DYNAMIC_COMBAT_LIFESTEAL: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetLifeSteal(fValue); break;
    case DYNAMIC_COMBAT_EVASION: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetEvasion(fValue); break;
    case DYNAMIC_COMBAT_MISSCHANCE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetMissChance(fValue); break;
    case DYNAMIC_COMBAT_NONLETHAL: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetNonLethal(fValue != 0.0f); break;
    case DYNAMIC_COMBAT_TRUESTRIKE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetTrueStrike(fValue != 0.0f); break;
    case DYNAMIC_COMBAT_NEGATED: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetNegated(fValue != 0.0f); break;
    case DYNAMIC_COMBAT_DEFLECTION: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetDeflection(fValue); break;
    case DYNAMIC_COMBAT_MANACOST: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetManaCost(fValue); break;
    case DYNAMIC_COMBAT_COOLDOWNTIME: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetCooldownTime(INT_ROUND(fValue)); break;
    case DYNAMIC_COMBAT_ATTACKABILITY: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetAttackAbility(fValue != 0.0f); break;
    case DYNAMIC_COMBAT_ARMORPIERCE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetArmorPierce(fValue); break;
    case DYNAMIC_COMBAT_MAGICARMORPIERCE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetMagicArmorPierce(fValue); break;

    case DYNAMIC_DAMAGE_ATTEMPTED: if (m_pEnv->pDamageEvent != NULL) m_pEnv->pDamageEvent->SetAmount(fValue); break;
    case DYNAMIC_DAMAGE_DEFLECTION: if (m_pEnv->pDamageEvent != NULL) m_pEnv->pDamageEvent->SetDeflection(fValue); break;
    case DYNAMIC_DAMAGE_ARMORPIERCE: if (m_pEnv->pDamageEvent != NULL) m_pEnv->pDamageEvent->SetArmorPierce(fValue); break;
    case DYNAMIC_DAMAGE_MAGICARMORPIERCE: if (m_pEnv->pDamageEvent != NULL) m_pEnv->pDamageEvent->SetMagicArmorPierce(fValue); break;
    }
}


/*====================
  ICombatAction::SetDynamicEffectType
  ====================*/
void    ICombatAction::SetDynamicEffectType(EDynamicActionValue eMultiplier, uint uiEffectType)
{
    switch (eMultiplier)
    {
    case DYNAMIC_COMBAT_EFFECTTYPE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetEffectType(uiEffectType); break;
    case DYNAMIC_COMBAT_DAMAGETYPE: if (m_pEnv->pCombatEvent != NULL) m_pEnv->pCombatEvent->SetDamageType(uiEffectType); break;

    case DYNAMIC_DAMAGE_EFFECTTYPE: if (m_pEnv->pDamageEvent != NULL) m_pEnv->pDamageEvent->SetEffectType(uiEffectType); break;
    }
}


/*====================
  ICombatAction::Evaluate
  ====================*/
float   ICombatAction::Evaluate(float fA, float fB, EActionOperator eOp) const
{
    switch (eOp)
    {
    case OPERATOR_ADD:
        return fA + fB;

    case OPERATOR_SUB:
        return fA - fB;

    case OPERATOR_MULT:
        return fA * fB;

    case OPERATOR_DIV:
        return fA / fB;

    case OPERATOR_MIN:
        return MIN(fA, fB);

    case OPERATOR_MAX:
        return MAX(fA, fB);

    case OPERATOR_NONE:
    default:
        return fA;
    };
}


/*====================
  ICombatAction::Compare
  ====================*/
bool    ICombatAction::Compare(float fA, float fB, EActionCmpOperator eOp) const
{
    switch (eOp)
    {
    case OPERATOR_CMP_EQUALS:
        return fA == fB;

    case OPERATOR_CMP_NOT_EQUALS:
        return fA != fB;

    case OPERATOR_CMP_LESS_THAN:
        return fA < fB;

    case OPERATOR_CMP_LESS_THAN_OR_EQUALS:
        return fA <= fB;

    case OPERATOR_CMP_GREATER_THAN:
        return fA > fB;

    case OPERATOR_CMP_GREATER_THAN_OR_EQUALS:
        return fA >= fB;

    case OPERATOR_CMP_NONE:
    default:
        return false;
    };
}


/*====================
  ICombatAction::GetSourceUnit
  ====================*/
IUnitEntity*    ICombatAction::GetSourceUnit() const
{
    IGameEntity *pEntity(GetSourceEntity());
    if (pEntity == NULL)
        return NULL;

    return pEntity->GetAsUnit();
}


/*====================
  ICombatAction::GetTargetUnit
  ====================*/
IUnitEntity*    ICombatAction::GetTargetUnit() const
{
    IGameEntity *pEntity(GetTargetEntity());
    if (pEntity == NULL)
        return NULL;
    
    return pEntity->GetAsUnit();
}


/*====================
  CCombatActionScript::FetchEffectDescription
  ====================*/
void    CCombatActionScript::FetchEffectDescription(const tstring &sName)
{
    tstring sKey(sName + _CWS("_") + GetActionScriptName(m_eAction) + _CWS("_effect"));
    if (m_uiModifierID != INVALID_INDEX)
        sKey += _CWS(":") + EntityRegistry.LookupModifierKey(m_uiModifierID);

    m_uiEffectDescriptionIndex = Game.GetEntityStringIndex(sKey);
}


/*====================
  CCombatActionScript::GetEffectDescription
  ====================*/
const tstring&  CCombatActionScript::GetEffectDescription() const
{
    return Game.GetEntityString(m_uiEffectDescriptionIndex);
}


/*====================
  CCombatActionScript::GetEffectDescriptionIndex
  ====================*/
uint    CCombatActionScript::GetEffectDescriptionIndex() const
{
    return m_uiEffectDescriptionIndex;
}


/*====================
  CCombatActionScript::Execute
  ====================*/
float   CCombatActionScript::Execute(IGameEntity *pInflictor, IGameEntity *pInitiator, IGameEntity *pTarget, const CVec3f &v3Target, IGameEntity *pProxy, CCombatEvent *pCombatEvent, CDamageEvent *pDamageEvent, CScriptThread *pScriptThread, const CVec3f &v3Delta, float fDefault)
{
    SCombatActionEnv cEnv;

    cEnv.pThis = Game.GetEntityFromUniqueID(m_uiThisUID);
    cEnv.uiLevel = m_uiLevel;
    cEnv.pInitiator = pInitiator;
    cEnv.pInflictor = pInflictor;
    cEnv.pTarget = pTarget;
    cEnv.pProxy = pProxy;
    cEnv.v3Target = v3Target;
    cEnv.pCombatEvent = pCombatEvent;
    cEnv.pDamageEvent = pDamageEvent;
    cEnv.v3Delta = v3Delta;
    cEnv.pScriptThread = pScriptThread;

    cEnv.citAct = m_vActions.begin();
    
    cEnv.fResult = fDefault;
    cEnv.fVar0 = 0.0f;
    cEnv.fVar1 = 0.0f;
    cEnv.fVar2 = 0.0f;
    cEnv.fVar3 = 0.0f;
    cEnv.v3Pos0.Clear();
    cEnv.v3Pos1.Clear();
    cEnv.v3Pos2.Clear();
    cEnv.v3Pos3.Clear();
    cEnv.pEnt0 = NULL;
    cEnv.pEnt1 = NULL;
    cEnv.pEnt2 = NULL;
    cEnv.pEnt3 = NULL;

    cEnv.bStall = false;
    cEnv.bTerminate = false;
    cEnv.uiRepeated = 0;
    cEnv.uiTracker = 0;
    cEnv.pNext = NULL;
    
    ExecuteActions(cEnv);

    return cEnv.fResult;
}


/*====================
  CCombatActionScript::ExecuteActions
  ====================*/
void    CCombatActionScript::ExecuteActions(SCombatActionEnv &cEnv)
{
    if (cEnv.pScriptThread != NULL)
    {
        if (cEnv.pScriptThread->GetWaitTime() != 0)
            return;
        if (cEnv.citAct == m_vActions.end())
            return;

        cEnv.bStall = false;

        while (cEnv.citAct != m_vActions.end())
        {
            (*cEnv.citAct)->SetEnv(&cEnv);

            cEnv.fResult = (*cEnv.citAct)->Execute();

            if (cEnv.bTerminate)
                break;

            if (cEnv.bStall)
            {
                ++cEnv.uiRepeated;
                break;
            }

            ++cEnv.citAct;
            cEnv.uiRepeated = 0;
            cEnv.uiTracker = 0;

            if (cEnv.pScriptThread->GetWaitTime() > 0)
                break;
        }
    }
    else
    {
        while (cEnv.citAct != m_vActions.end())
        {
            (*cEnv.citAct)->SetEnv(&cEnv);

            cEnv.fResult = (*cEnv.citAct)->Execute();

            if (cEnv.bTerminate)
                break;

            ++cEnv.citAct;
        }
    }
}


/*====================
  ICombatActionBranch::ExecuteActions
  ====================*/
void    ICombatActionBranch::ExecuteActions()
{
    if (m_pEnv->pScriptThread != NULL)
    {
        if (m_pEnv->pNext == NULL)
        {
            m_pEnv->pNext = K2_NEW(ctx_Game,   SCombatActionEnv)(*m_pEnv);
            m_pEnv->pNext->citAct = m_script.GetActions().begin();
            m_pEnv->pNext->uiRepeated = 0;
            m_pEnv->pNext->uiTracker = 0;
        }

        m_script.ExecuteActions(*m_pEnv->pNext);
        
        if (m_pEnv->pNext->bTerminate)
            m_pEnv->bTerminate = true;              

        if (m_pEnv->pNext->bStall)
            m_pEnv->bStall = true;

        if (m_pEnv->pNext->citAct == m_script.GetActions().end())
        {
            K2_DELETE(m_pEnv->pNext);
            m_pEnv->pNext = NULL;
        }
    }
    else
    {
        SCombatActionEnv cEnv(*m_pEnv);
        cEnv.citAct = m_script.GetActions().begin();
        cEnv.uiRepeated = 0;
        cEnv.uiTracker = 0;

        m_script.ExecuteActions(cEnv);
    }
}


/*====================
  EvaluateConditionalString
  ====================*/
bool    EvaluateConditionalString(const tstring &sTest, IGameEntity *pThis, IGameEntity *pInflictor, IUnitEntity *pSource, IUnitEntity *pTarget, const ICombatAction *pAction)
{
    PROFILE("EvaluateConditionalString");

    tstring::size_type zAnd(sTest.find(_CWS(" and ")));
    tstring::size_type zOr(sTest.find(_CWS(" or ")));

    tstring::size_type zPos(MIN(zAnd, zOr));
    if (zPos != tstring::npos)
    {
        if (zPos == zAnd)
            return EvaluateConditionalString(sTest.substr(0, zAnd), pThis, pInflictor, pSource, pTarget, pAction) &&
                EvaluateConditionalString(sTest.substr(zAnd + 5), pThis, pInflictor, pSource, pTarget, pAction);
        else
            return EvaluateConditionalString(sTest.substr(0, zOr), pThis, pInflictor, pSource, pTarget, pAction) ||
                EvaluateConditionalString(sTest.substr(zOr + 4), pThis, pInflictor, pSource, pTarget, pAction);
    }

    tsvector vTokens(TokenizeString(sTest, SPACE));

    if (vTokens.size() < 1)
        return false;

    if ((TStringCompare(vTokens[0], _T("canactivate")) == 0 && pThis != NULL && pThis->IsTool() && pThis->GetAsTool()->CanActivate()) ||
        (TStringCompare(vTokens[0], _T("isready")) == 0 && pThis != NULL && pThis->IsTool() && pThis->GetAsTool()->IsReady()) ||
        (TStringCompare(vTokens[0], _T("istoggled")) == 0 && pThis != NULL && pThis->IsTool() && pThis->GetAsTool()->HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE)) ||
        (TStringCompare(vTokens[0], _T("owner")) == 0 && pSource != NULL && pTarget != NULL && pSource->GetOwner() == pTarget) ||
        (TStringCompare(vTokens[0], _T("not_owner")) == 0 && pSource != NULL && pTarget != NULL && pSource->GetOwner() != pTarget) ||
        (TStringCompare(vTokens[0], _T("inflictor")) == 0 && pThis != NULL && pTarget != NULL && pThis->IsState() && pThis->GetAsState()->GetInflictor() == pTarget) ||
        (TStringCompare(vTokens[0], _T("not_inflictor")) == 0 && pThis != NULL && pTarget != NULL && pThis->IsState() && pThis->GetAsState()->GetInflictor() == pTarget) ||
        (TStringCompare(vTokens[0], _T("ally")) == 0 && pSource != NULL && pTarget != NULL && pSource->GetTeam() == pTarget->GetTeam()) ||
        (TStringCompare(vTokens[0], _T("enemy")) == 0 && pSource != NULL && pTarget != NULL && pSource->GetTeam() != pTarget->GetTeam()) ||
        (TStringCompare(vTokens[0], _T("return")) == 0 && pThis != NULL && pThis->IsProjectile() && pThis->GetAsProjectile()->GetReturnCount() > 0) ||
        (TStringCompare(vTokens[0], _T("return")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && pInflictor->GetAsProjectile()->GetReturnCount() > 0) ||
        (TStringCompare(vTokens[0], _T("not_return")) == 0 && pThis != NULL && pThis->IsProjectile() && pThis->GetAsProjectile()->GetReturnCount() == 0) ||
        (TStringCompare(vTokens[0], _T("not_return")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && pInflictor->GetAsProjectile()->GetReturnCount() == 0) ||
        (TStringCompare(vTokens[0], _T("redirect")) == 0 && pThis != NULL && pThis->IsProjectile() && pThis->GetAsProjectile()->GetRedirectCount() > 0) ||
        (TStringCompare(vTokens[0], _T("redirect")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && pInflictor->GetAsProjectile()->GetRedirectCount() > 0) ||
        (TStringCompare(vTokens[0], _T("not_redirect")) == 0 && pThis != NULL && pThis->IsProjectile() && pThis->GetAsProjectile()->GetRedirectCount() == 0) ||
        (TStringCompare(vTokens[0], _T("not_redirect")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && pInflictor->GetAsProjectile()->GetRedirectCount() == 0) ||
        (TStringCompare(vTokens[0], _T("bound")) == 0 && pThis != NULL && pThis->IsProjectile() && pThis->GetAsProjectile()->HasBinds()) ||
        (TStringCompare(vTokens[0], _T("bound")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && pInflictor->GetAsProjectile()->HasBinds()) ||
        (TStringCompare(vTokens[0], _T("not_bound")) == 0 && pThis != NULL && pThis->IsProjectile() && !pThis->GetAsProjectile()->HasBinds()) ||
        (TStringCompare(vTokens[0], _T("not_bound")) == 0 && pInflictor != NULL && pInflictor->IsProjectile() && !pInflictor->GetAsProjectile()->HasBinds()) ||
        (TStringCompare(vTokens[0], _T("isattacking")) == 0 && pSource != NULL && pSource->IsAttacking()) ||
        (TStringCompare(vTokens[0], _T("not_isattacking")) == 0 && pSource != NULL && !pSource->IsAttacking()) ||
        (TStringCompare(vTokens[0], _T("hastarget")) == 0 && pSource != NULL && Game.GetUnitEntity(pSource->GetTargetIndex()) != NULL) ||
        (TStringCompare(vTokens[0], _T("not_hastarget")) == 0 && pSource != NULL && Game.GetUnitEntity(pSource->GetTargetIndex()) == NULL) ||
        (TStringCompare(vTokens[0], _T("stealthed")) == 0 && pTarget != NULL && pTarget->IsStealth(false)) ||
        (TStringCompare(vTokens[0], _T("not_stealthed")) == 0 && pTarget != NULL && !pTarget->IsStealth(false)) ||
        (TStringCompare(vTokens[0], _T("illusion")) == 0 && pTarget != NULL && pTarget->IsIllusion()) ||
        (TStringCompare(vTokens[0], _T("not_illusion")) == 0 && pTarget != NULL && !pTarget->IsIllusion()) ||
        (TStringCompare(vTokens[0], _T("hasorder")) == 0 && pTarget != NULL && !pTarget->GetBrain().IsEmpty() && !pTarget->GetBrain().GetCurrentBehavior()->GetDefault()) ||
        (TStringCompare(vTokens[0], _T("not_hasorder")) == 0 && pTarget != NULL && (pTarget->GetBrain().IsEmpty() || pTarget->GetBrain().GetCurrentBehavior()->GetDefault()))
        )
    {
        return true;
    }

    if (vTokens.size() < 2)
        return false;

    if (TStringCompare(vTokens[0], _T("entity_type")) == 0)
    {
        if (pTarget != NULL && pTarget->GetTypeName() == vTokens[1])
            return true;
        else if (pThis != NULL && pThis->GetTypeName() == vTokens[1])
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("not_entity_type")) == 0)
    {
        if (pTarget != NULL && pTarget->GetTypeName() != vTokens[1])
            return true;
        else if (pInflictor != NULL && pInflictor->GetTypeName() != vTokens[1])
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("target_type")) == 0)
    {
        if (pTarget != NULL && pTarget->IsTargetType(vTokens[1], pSource))
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("not_target_type")) == 0)
    {
        if (pTarget == NULL || !pTarget->IsTargetType(vTokens[1], pSource))
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("modifier")) == 0)
    {
        if (pThis != NULL && pThis->HasModifier(vTokens[1]))
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("hasproperty")) == 0)
    {
        ISlaveEntity *pSlave(pInflictor ? pInflictor->GetAsSlave() : NULL);
        if (pSlave == NULL)
            pSlave = pThis ? pThis->GetAsSlave() : NULL;

        if (pSlave != NULL)
        {
            if (TStringCompare(vTokens[1], _T("silenced")) == 0 && pSlave->GetAsSlave()->GetSilenced())
                return true;
            else if (TStringCompare(vTokens[1], _T("disarmed")) == 0 && pSlave->GetAsSlave()->GetDisarmed())
                return true;
            else if (TStringCompare(vTokens[1], _T("perplexed")) == 0 && pSlave->GetAsSlave()->GetPerplexed())
                return true;
            else if (TStringCompare(vTokens[1], _T("immobilized")) == 0 && pSlave->GetAsSlave()->GetImmobilized())
                return true;
            else if (TStringCompare(vTokens[1], _T("restrained")) == 0 && pSlave->GetAsSlave()->GetRestrained())
                return true;
            else if (TStringCompare(vTokens[1], _T("invulnerable")) == 0 && pSlave->GetAsSlave()->GetInvulnerable())
                return true;
            else if (TStringCompare(vTokens[1], _T("revealed")) == 0 && pSlave->GetAsSlave()->GetRevealed())
                return true;
            else if (TStringCompare(vTokens[1], _T("frozen")) == 0 && pSlave->GetAsSlave()->GetFrozen())
                return true;
            else if (TStringCompare(vTokens[1], _T("isolated")) == 0 && pSlave->GetAsSlave()->GetIsolated())
                return true;
            else if (TStringCompare(vTokens[1], _T("freecast")) == 0 && pSlave->GetAsSlave()->GetFreeCast())
                return true;
            else if (TStringCompare(vTokens[1], _T("clearvision")) == 0 && pSlave->GetAsSlave()->GetClearVision())
                return true;
            else if (TStringCompare(vTokens[1], _T("stunned")) == 0 && pSlave->GetAsSlave()->GetStunned())
                return true;
        }

        return false;
    }
    else if (TStringCompare(vTokens[0], _T("not_hasproperty")) == 0)
    {
        ISlaveEntity *pSlave(pInflictor ? pInflictor->GetAsSlave() : NULL);
        if (pSlave == NULL)
            pSlave = pThis ? pThis->GetAsSlave() : NULL;

        if (pSlave != NULL)
        {
            if (TStringCompare(vTokens[1], _T("silenced")) == 0 && pSlave->GetAsSlave()->GetSilenced())
                return false;
            else if (TStringCompare(vTokens[1], _T("disarmed")) == 0 && pSlave->GetAsSlave()->GetDisarmed())
                return false;
            else if (TStringCompare(vTokens[1], _T("perplexed")) == 0 && pSlave->GetAsSlave()->GetPerplexed())
                return false;
            else if (TStringCompare(vTokens[1], _T("immobilized")) == 0 && pSlave->GetAsSlave()->GetImmobilized())
                return false;
            else if (TStringCompare(vTokens[1], _T("restrained")) == 0 && pSlave->GetAsSlave()->GetRestrained())
                return false;
            else if (TStringCompare(vTokens[1], _T("invulnerable")) == 0 && pSlave->GetAsSlave()->GetInvulnerable())
                return false;
            else if (TStringCompare(vTokens[1], _T("revealed")) == 0 && pSlave->GetAsSlave()->GetRevealed())
                return false;
            else if (TStringCompare(vTokens[1], _T("frozen")) == 0 && pSlave->GetAsSlave()->GetFrozen())
                return false;
            else if (TStringCompare(vTokens[1], _T("isolated")) == 0 && pSlave->GetAsSlave()->GetIsolated())
                return false;
            else if (TStringCompare(vTokens[1], _T("freecast")) == 0 && pSlave->GetAsSlave()->GetFreeCast())
                return false;
            else if (TStringCompare(vTokens[1], _T("clearvision")) == 0 && pSlave->GetAsSlave()->GetClearVision())
                return false;
            else if (TStringCompare(vTokens[1], _T("stunned")) == 0 && pSlave->GetAsSlave()->GetStunned())
                return false;
        }

        return true;
    }
    else if (TStringCompare(vTokens[0], _T("hastrait")) == 0)
    {
        if (pTarget != NULL)
        {
            if (TStringCompare(vTokens[1], _T("silenced")) == 0 && pTarget->GetAsUnit()->IsSilenced())
                return true;
            else if (TStringCompare(vTokens[1], _T("disarmed")) == 0 && pTarget->GetAsUnit()->IsDisarmed())
                return true;
            else if (TStringCompare(vTokens[1], _T("perplexed")) == 0 && pTarget->GetAsUnit()->IsPerplexed())
                return true;
            else if (TStringCompare(vTokens[1], _T("immobilized")) == 0 && pTarget->GetAsUnit()->IsImmobilized())
                return true;
            else if (TStringCompare(vTokens[1], _T("restrained")) == 0 && pTarget->GetAsUnit()->IsRestrained())
                return true;
            else if (TStringCompare(vTokens[1], _T("invulnerable")) == 0 && pTarget->GetAsUnit()->GetInvulnerable())
                return true;
            else if (TStringCompare(vTokens[1], _T("revealed")) == 0 && pTarget->GetAsUnit()->IsRevealed())
                return true;
            else if (TStringCompare(vTokens[1], _T("frozen")) == 0 && pTarget->GetAsUnit()->IsFrozen())
                return true;
            else if (TStringCompare(vTokens[1], _T("isolated")) == 0 && pTarget->GetAsUnit()->IsIsolated())
                return true;
            else if (TStringCompare(vTokens[1], _T("freecast")) == 0 && pTarget->GetAsUnit()->IsFreeCast())
                return true;
            else if (TStringCompare(vTokens[1], _T("clearvision")) == 0 && pTarget->GetAsUnit()->GetClearVision())
                return true;
            else if (TStringCompare(vTokens[1], _T("stunned")) == 0 && pTarget->GetAsUnit()->IsStunned())
                return true;
        }

        return false;
    }
    else if (TStringCompare(vTokens[0], _T("not_hastrait")) == 0)
    {
        if (pTarget != NULL)
        {
            if (TStringCompare(vTokens[1], _T("silenced")) == 0 && pTarget->GetAsUnit()->IsSilenced())
                return false;
            else if (TStringCompare(vTokens[1], _T("disarmed")) == 0 && pTarget->GetAsUnit()->IsDisarmed())
                return false;
            else if (TStringCompare(vTokens[1], _T("perplexed")) == 0 && pTarget->GetAsUnit()->IsPerplexed())
                return false;
            else if (TStringCompare(vTokens[1], _T("immobilized")) == 0 && pTarget->GetAsUnit()->IsImmobilized())
                return false;
            else if (TStringCompare(vTokens[1], _T("restrained")) == 0 && pTarget->GetAsUnit()->IsRestrained())
                return false;
            else if (TStringCompare(vTokens[1], _T("invulnerable")) == 0 && pTarget->GetAsUnit()->GetInvulnerable())
                return false;
            else if (TStringCompare(vTokens[1], _T("revealed")) == 0 && pTarget->GetAsUnit()->IsRevealed())
                return false;
            else if (TStringCompare(vTokens[1], _T("frozen")) == 0 && pTarget->GetAsUnit()->IsFrozen())
                return false;
            else if (TStringCompare(vTokens[1], _T("isolated")) == 0 && pTarget->GetAsUnit()->IsIsolated())
                return false;
            else if (TStringCompare(vTokens[1], _T("freecast")) == 0 && pTarget->GetAsUnit()->IsFreeCast())
                return false;
            else if (TStringCompare(vTokens[1], _T("clearvision")) == 0 && pTarget->GetAsUnit()->GetClearVision())
                return false;
            else if (TStringCompare(vTokens[1], _T("stunned")) == 0 && pTarget->GetAsUnit()->IsStunned())
                return false;
        }

        return true;
    }
    else if (TStringCompare(vTokens[0], _T("hasstate")) == 0)
    {
        ushort unStateID(EntityRegistry.LookupID(vTokens[1]));
        if (pTarget != NULL && pTarget->IsUnit() && unStateID != INVALID_ENT_TYPE && pTarget->HasState(unStateID))
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("not_hasstate")) == 0)
    {
        ushort unStateID(EntityRegistry.LookupID(vTokens[1]));
        if (pTarget != NULL && pTarget->IsUnit() && unStateID != INVALID_ENT_TYPE && !pTarget->HasState(unStateID))
            return true;
    }
    else if (TStringCompare(vTokens[0], _T("hasorderseq")) == 0)
    {
        float fA;

        EDynamicActionValue eValue(GetDynamicActionValueFromString(vTokens[1]));
        if (eValue != DYNAMIC_VALUE_INVALID)
            fA = pAction->GetDynamicValue(eValue);
        else
            fA = AtoF(vTokens[0]);

        return pTarget->HasOrder(INT_ROUND(fA));
    }
    else if (TStringCompare(vTokens[0], _T("not_hasorderseq")) == 0)
    {
        float fA;

        EDynamicActionValue eValue(GetDynamicActionValueFromString(vTokens[1]));
        if (eValue != DYNAMIC_VALUE_INVALID)
            fA = pAction->GetDynamicValue(eValue);
        else
            fA = AtoF(vTokens[0]);

        return !pTarget->HasOrder(INT_ROUND(fA));
    }
    
    if (vTokens.size() < 3)
        return false;

    if (vTokens[0] == _T("proximity") && pSource != NULL)
    {
        float fRange(AtoF(vTokens[2]));

        uivector vResult;
        Game.GetEntitiesInRadius(vResult, pSource->GetPosition().xy(), fRange, REGION_UNIT);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == NULL)
                continue;
            if (Game.IsValidTarget(Game.LookupTargetScheme(vTokens[1]), 0, pSource, pUnit, true))
                return true;
        }

        return false;
    }
    else if (vTokens[0] == _T("no_proximity") && pSource != NULL)
    {
        float fRange(AtoF(vTokens[2]));

        uivector vResult;
        Game.GetEntitiesInRadius(vResult, pSource->GetPosition().xy(), fRange, REGION_UNITS_AND_TREES);
        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IUnitEntity *pUnit(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*it)));
            if (pUnit == NULL)
                continue;
            if (Game.IsValidTarget(Game.LookupTargetScheme(vTokens[1]), 0, pSource, pUnit, true))
                return false;
        }

        return true;
    }

    float fA(0.0f);
    if (vTokens[0] == _T("target_angle") && pTarget != NULL && pSource != NULL)
    {
        CAxis axisTarget(pTarget->GetAngles());
        CVec2f v2TargetDir(axisTarget.Forward2d());
        CVec2f v2AttackDir(pTarget->GetPosition() - pSource->GetPosition());
        v2AttackDir.Normalize();
        fA = RAD2DEG(acos(CLAMP(DotProduct(v2TargetDir, v2AttackDir), -1.0f, 1.0f)));
    }
    else if (vTokens[0] == _T("back_impact_angle") && pTarget != NULL && pSource != NULL)
    {
        CAxis axisSource(pSource->GetAngles());
        CVec2f v2SourceDir(axisSource.Forward2d());
        CVec2f v2AttackDir(pSource->GetPosition() - pTarget->GetPosition());
        v2AttackDir.Normalize();
        fA = RAD2DEG(acos(CLAMP(DotProduct(v2SourceDir, v2AttackDir), -1.0f, 1.0f)));
    }
    else if (vTokens[0] == _T("facing_angle") && pTarget != NULL && pSource != NULL)
    {
        CAxis axisSource(pSource->GetAngles());
        CVec2f v2SourceDir(axisSource.Forward2d());
        CAxis axisTarget(pTarget->GetAngles());
        CVec2f v2TargetDir(axisTarget.Forward2d());
        fA = RAD2DEG(acos(CLAMP(DotProduct(v2SourceDir, v2TargetDir), -1.0f, 1.0f)));
    }
    else if (vTokens[0] == _T("distance") && pTarget != NULL && pSource != NULL)
    {
        fA = Distance(pSource->GetPosition().xy(), pTarget->GetPosition().xy());
    }
    else if (vTokens[0] == _T("target_health_percent") && pTarget != NULL)
    {
        fA = pTarget->GetHealthPercent();
    }
    else if (vTokens[0] == _T("target_mana_percent") && pTarget != NULL)
    {
        fA = pTarget->GetManaPercent();
    }
    else if (vTokens[0] == _T("target_health") && pTarget != NULL)
    {
        fA = pTarget->GetHealth();
    }
    else if (vTokens[0] == _T("target_mana") && pTarget != NULL)
    {
        fA = pTarget->GetMana();
    }
        else if (vTokens[0] == _T("target_accumulator") && pTarget != NULL)
    {
        if (pTarget->IsSlave())
            fA = pTarget->GetAsSlave()->GetAccumulator();
        else if (pTarget->IsUnit())
            fA = pTarget->GetAsUnit()->GetAccumulator();
    }
    else if (vTokens[0] == _T("source_health_percent") && pSource != NULL)
    {
        fA = pSource->GetHealthPercent();
    }
    else if (vTokens[0] == _T("source_mana_percent") && pSource != NULL)
    {
        fA = pSource->GetManaPercent();
    }
    else if (vTokens[0] == _T("source_health") && pSource != NULL)
    {
        fA = pSource->GetHealth();
    }
    else if (vTokens[0] == _T("source_mana") && pSource != NULL)
    {
        fA = pSource->GetMana();
    }
    else if (vTokens[0] == _T("source_accumulator") && pSource != NULL)
    {
        if (pSource->IsSlave())
            fA = pSource->GetAsSlave()->GetAccumulator();
        else if (pSource->IsUnit())
            fA = pSource->GetAsUnit()->GetAccumulator();
    }
    else if (vTokens[0] == _T("charges") && pThis != NULL)
    {
        if (pThis->IsSlave())
            fA = pThis->GetAsSlave()->GetCharges();
        else if (pThis->IsProjectile())
            fA = pThis->GetAsProjectile()->GetCharges();
    }
    else if (vTokens[0] == _T("accumulator") && pThis != NULL)
    {
        if (pThis->IsSlave())
            fA = pThis->GetAsSlave()->GetAccumulator();
        else if (pThis->IsUnit())
            fA = pThis->GetAsUnit()->GetAccumulator();
    }
    else if (vTokens[0] == _T("param") && pThis != NULL && pThis->IsAffector())
    {
        fA = pThis->GetAsAffector()->GetParam();
    }
    else if (vTokens[0] == _T("bounce_count") && pThis != NULL && pThis->IsProjectile())
    {
        fA = pThis->GetAsProjectile()->GetBounceCount();
    }
    else if (vTokens[0] == _T("bounce_count") && pInflictor != NULL && pInflictor->IsProjectile())
    {
        fA = pInflictor->GetAsProjectile()->GetBounceCount();
    }
    else
    {
        if (pAction == NULL)
        {
            fA = AtoF(vTokens[0]);
        }
        else
        {
            EDynamicActionValue eValue(GetDynamicActionValueFromString(vTokens[0]));
            if (eValue != DYNAMIC_VALUE_INVALID)
                fA = pAction->GetDynamicValue(eValue);
            else
                fA = AtoF(vTokens[0]);
        }
    }

    float fB(0.0f);

    if (pAction == NULL)
    {
        fB = AtoF(vTokens[2]);
    }
    else
    {
        EDynamicActionValue eValue(GetDynamicActionValueFromString(vTokens[2]));
        if (eValue != DYNAMIC_VALUE_INVALID)
            fB = pAction->GetDynamicValue(eValue);
        else
            fB = AtoF(vTokens[2]);
    }

    if (vTokens[1] == _T("==") || vTokens[1] == _T("eq"))
    {
        if (fA != fB)
            return false;
    }
    else if (vTokens[1] == _T("!=") || vTokens[1] == _T("ne"))
    {
        if (fA == fB)
            return false;
    }
    else if (vTokens[1] == _T("<") || vTokens[1] == _T("lt"))
    {
        if (fA >= fB)
            return false;
    }
    else if (vTokens[1] == _T("<=") || vTokens[1] == _T("le"))
    {
        if (fA > fB)
            return false;
    }
    else if (vTokens[1] == _T(">") || vTokens[1] == _T("gt"))
    {
        if (fA <= fB)
            return false;
    }
    else if (vTokens[1] == _T(">=") || vTokens[1] == _T("ge"))
    {
        if (fA < fB)
            return false;
    }

    return true;
}
