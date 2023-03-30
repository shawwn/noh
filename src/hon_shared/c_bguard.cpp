// (C)2008 S2 Games
// c_bguard.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bguard.h"

#include "c_battack.h"
#include "c_asMoving.h"
#include "i_unitentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

/*====================
  CBGuard::CopyFrom
  ====================*/
void    CBGuard::CopyFrom(const IBehavior *pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBGuard *pCBBehavior(static_cast<const CBGuard*>(pBehavior));

    m_Attack.CopyFrom(&pCBBehavior->m_Attack);
    m_Attack.SetBrain(m_pBrain);
    m_Attack.SetSelf(m_pSelf);
    m_bAttacking = pCBBehavior->m_bAttacking;
    m_uiLastAggroUpdate = pCBBehavior->m_uiLastAggroUpdate;
    m_eGuardState = pCBBehavior->m_eGuardState;
    m_uiGuardStateEndTime = pCBBehavior->m_uiGuardStateEndTime;

    CBMove::CopyFrom(pCBBehavior);
}


/*====================
  CBGuard::Clone
  ====================*/
IBehavior*  CBGuard::Clone(CBrain *pNewBrain, IUnitEntity *pNewSelf) const
{
    IBehavior *pBehavior(K2_NEW(ctx_Game, CBGuard)());
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}


/*====================
  CBGuard::UpdateAggro
  ====================*/
void    CBGuard::UpdateAggro()
{
    IUnitEntity *pTarget(Game.GetUnitEntity(m_Attack.GetTarget()));

    if (m_eGuardState == GUARD_CHASING &&
        (m_uiGuardStateEndTime == INVALID_TIME ||
            m_uiGuardStateEndTime <= Game.GetGameTime() ||
            pTarget == NULL ||
            !m_pSelf->ShouldTarget(pTarget) ||
            pTarget->HasUnitFlags(UNIT_FLAG_INVULNERABLE) ||
            pTarget->GetInvulnerable()) &&
        ~m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
    {
        float fHoldRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetGuardChaseDistance());

        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) > SQR(fHoldRange))
        {
            //Console << _T("Returning: ") << Distance(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) << newl;

            if (m_bAttacking)
            {
                m_Attack.EndBehavior();
                m_bAttacking = false;
            }

            m_eGuardState = GUARD_RETURNING;
            m_uiGuardStateEndTime = INVALID_TIME;
            CBMove::BeginBehavior();

            if (m_pSelf->IsNeutral() && m_pSelf->GetOwnerIndex() == INVALID_INDEX)
                m_pSelf->Action(ACTION_SCRIPT_LEASH, pTarget, NULL, NULL, NULL);
        }
        else
        {
            //Console << _T("Holding: ") << Distance(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) << newl;

            if (m_bAttacking)
            {
                m_Attack.EndBehavior();
                m_bAttacking = false;
            }

            m_eGuardState = GUARD_HOLDING;
            m_uiGuardStateEndTime = INVALID_TIME;
        }
    }

    if (m_uiLastAggroUpdate != INVALID_TIME && m_uiLastAggroUpdate + BEHAVIOR_UPDATE_MS >= Game.GetGameTime())
        return;

    if (m_eGuardState != GUARD_HOLDING && m_eGuardState != GUARD_CHASING)
        return;

    // No aggro updates while still in an attack
    if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
        return;

    // No aggro updates when disarmed or can't attack
    if (m_pSelf->IsDisarmed() || !m_pSelf->GetCanAttack())
        return;

    // Update the status of the attackmove behavior
    m_uiLastAggroUpdate = Game.GetGameTime();

    uint uiCurrentTargetIndex(m_bAttacking ? m_Attack.GetTarget() : INVALID_INDEX);

    static uivector vEntities;
    CVec3f v3Position(m_pSelf->GetPosition());
    float fAggroRange(m_pSelf->GetAggroRange() + m_pSelf->GetBounds().GetDim(X) * DIAG);
    CBBoxf bbRegion(CVec3f(v3Position.xy() - CVec2f(fAggroRange), -FAR_AWAY),  CVec3f(v3Position.xy() + CVec2f(fAggroRange), FAR_AWAY));

    // Fetch
    Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_ACTIVE_UNIT);

    // Find the most threating enemy
    float fThreat(-FAR_AWAY);
    uint uiClosestWorldIndex(INVALID_INDEX);
    uivector_cit citEnd(vEntities.end());
    for (uivector_cit cit(vEntities.begin()); cit != citEnd; ++cit)
    {
        if (*cit == m_pSelf->GetWorldIndex())
            continue;

        IUnitEntity *pTarget(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*cit)));
        if (pTarget == NULL)
            continue;
        if (!m_pSelf->ShouldTarget(pTarget))
            continue;
        if (!Game.IsValidTarget(m_pSelf->GetAggroScheme(), 0, m_pSelf, pTarget, true))
            continue;

        float fCurrent(m_pSelf->GetThreatLevel(pTarget, pTarget->GetIndex() == uiCurrentTargetIndex));

        if (fCurrent > fThreat &&
            Distance(pTarget->GetPosition().xy(), v3Position.xy()) - pTarget->GetBounds().GetDim(X) * DIAG <= fAggroRange)
        {
            fThreat = fCurrent;
            uiClosestWorldIndex = *cit;
        }
    }

    if (uiClosestWorldIndex != INVALID_INDEX &&
        !(m_bAttacking && m_Attack.GetTarget() == Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex)))
    {
        CBMove::EndBehavior();

        if (m_bAttacking)
            m_Attack.EndBehavior();

        m_bAttacking = true;

        m_Attack.Init(m_pBrain, m_pSelf, Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex));
        m_Attack.DisableAggroTrigger();
    }

    if (m_bAttacking && m_eGuardState == GUARD_HOLDING)
    {
        m_pSelf->CallForHelp(500.0f, Game.GetUnitEntity(m_Attack.GetTarget()));

        m_eGuardState = GUARD_CHASING;
        m_uiGuardStateEndTime = Game.GetGameTime() + m_pSelf->GetGuardChaseTime();
    }
}


/*====================
  CBGuard::BeginBehavior
  ====================*/
void    CBGuard::BeginBehavior()
{
    if (m_pSelf == NULL || m_v2UpdatedGoal == V2_ZERO)
    {
        Console << _T("CBGuard: Behavior started without valid information") << newl;
        return;
    }

    m_pBrain->EndActionStates(1);

    m_eGuardState = GUARD_HOLDING;
    m_uiGuardStateEndTime = Game.GetGameTime();

    m_uiLastAggroUpdate = INVALID_TIME;

    UpdateAggro();

    if (!m_bAttacking)
        CBMove::BeginBehavior();

    ClearFlag(BSR_NEW);
}


/*====================
  CBGuard::ThinkFrame
  ====================*/
void    CBGuard::ThinkFrame()
{
    UpdateAggro();

    if (m_bAttacking)
    {
        if (m_Attack.Validate())
        {
            if (m_Attack.GetFlags() & BSR_NEW)
                m_Attack.BeginBehavior();

            if (~m_Attack.GetFlags() & BSR_NEW)
                m_Attack.ThinkFrame();
        }

        if (m_Attack.GetFlags() & BSR_END)
        {
            m_Attack.EndBehavior();
            m_bAttacking = false;
            m_uiLastAggroUpdate = INVALID_TIME;

            UpdateAggro();

            // Repath towards destination if we didn't find a new target
            if (!m_bAttacking)
                CBMove::BeginBehavior();
            else
            {
                if (m_Attack.Validate())
                {
                    if (m_Attack.GetFlags() & BSR_NEW)
                        m_Attack.BeginBehavior();

                    m_Attack.ThinkFrame();
                }
            }
        }
    }   

    if (!m_bAttacking)
        CBMove::ThinkFrame();
}


/*====================
  CBGuard::MovementFrame
  ====================*/
void    CBGuard::MovementFrame()
{
    if (m_bAttacking)
    {
        if (m_Attack.Validate())
        {
            if (~m_Attack.GetFlags() & BSR_NEW)
                m_Attack.MovementFrame();
        }
    }
    else
    {
        CBMove::MovementFrame();

        if (GetFlags() & BSR_END)
        {
            if (m_eGuardState == GUARD_RETURNING)
            {
                m_eGuardState = GUARD_HOLDING;
                m_uiGuardStateEndTime = Game.GetGameTime();
            }
            
            ClearFlag(BSR_END);
        }
    }
}


/*====================
  CBGuard::ActionFrame
  ====================*/
void    CBGuard::ActionFrame()
{
    UpdateAggro();

    if (m_bAttacking)
    {
        if (m_Attack.Validate())
        {
            if (~m_Attack.GetFlags() & BSR_NEW)
                m_Attack.ActionFrame();
        }

        if (m_Attack.GetFlags() & BSR_END)
        {
            m_Attack.EndBehavior();
            m_bAttacking = false;
            m_uiLastAggroUpdate = INVALID_TIME;

            UpdateAggro();

            // Repath towards destination if we didn't find a new target
            if (!m_bAttacking)
                CBMove::BeginBehavior();
        }
    }
}


/*====================
  CBGuard::CleanupFrame
  ====================*/
void    CBGuard::CleanupFrame()
{
}


/*====================
  CBGuard::EndBehavior
  ====================*/
void    CBGuard::EndBehavior()
{
    m_Attack.EndBehavior();
    IBehavior::EndBehavior();
}


/*====================
  CBGuard::Aggro
  ====================*/
void    CBGuard::Aggro(IUnitEntity *pAttacker, uint uiChaseTime)
{
    if (pAttacker == NULL ||
        !m_pSelf->ShouldTarget(pAttacker))
        return;

    if (m_bAttacking)
        m_Attack.EndBehavior();

    m_bAttacking = true;

    m_Attack.Init(m_pBrain, m_pSelf, pAttacker->GetIndex());
    m_Attack.DisableAggroTrigger();

    m_eGuardState = GUARD_CHASING;
    m_uiGuardStateEndTime = Game.GetGameTime() + uiChaseTime;
}


/*====================
  CBGuard::Damaged
  ====================*/
void    CBGuard::Damaged(IUnitEntity *pAttacker)
{
    if (pAttacker == NULL)
        return;

    m_pSelf->CallForHelp(500.0f, pAttacker);

    if (m_eGuardState == GUARD_HOLDING)
        Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
    else if (m_eGuardState == GUARD_RETURNING && m_pSelf->GetGuardReaggroChaseDistance() > 0.0f && m_pSelf->GetGuardReaggroChaseTime() > 0)
    {
        float fReaggroRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetGuardReaggroChaseDistance());
        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) <= SQR(fReaggroRange))
            Aggro(pAttacker, m_pSelf->GetGuardReaggroChaseTime());
    }
}


/*====================
  CBGuard::Assist
  ====================*/
void    CBGuard::Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker)
{
    if (pAttacker == NULL)
        return;

    if (m_eGuardState == GUARD_HOLDING)
        Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
    else if (m_eGuardState == GUARD_RETURNING && m_pSelf->GetGuardReaggroChaseDistance() > 0.0f && m_pSelf->GetGuardReaggroChaseTime() > 0)
    {
        float fReaggroRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetGuardReaggroChaseDistance());
        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) <= SQR(fReaggroRange))
            Aggro(pAttacker, m_pSelf->GetGuardReaggroChaseTime());
    }
}
