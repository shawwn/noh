// (C)2008 S2 Games
// c_battackmove.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_battackmove.h"

#include "c_battack.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBAttackMove::CopyFrom
  ====================*/
void    CBAttackMove::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBAttackMove *pCBBehavior(static_cast<const CBAttackMove*>(pBehavior));

    m_Attack.CopyFrom(&pCBBehavior->m_Attack);
    m_Attack.SetBrain(m_pBrain);
    m_Attack.SetSelf(m_pSelf);
    m_bAttacking = pCBBehavior->m_bAttacking;
    m_uiLastAggroUpdate = pCBBehavior->m_uiLastAggroUpdate;
    m_uiPrimaryTarget = pCBBehavior->m_uiPrimaryTarget;
    m_uiPrimaryTargetTime = pCBBehavior->m_uiPrimaryTargetTime;
    m_uiDelayTarget = pCBBehavior->m_uiDelayTarget;
    m_uiDelayTime = pCBBehavior->m_uiDelayTime;
    m_uiDelayDuration = pCBBehavior->m_uiDelayDuration;
    m_bDelayReaggroBlock = pCBBehavior->m_bDelayReaggroBlock;
    m_uiLastAttackSequence = pCBBehavior->m_uiLastAttackSequence;

    CBMove::CopyFrom(pCBBehavior);
}

/*====================
  CBAttackMove::Clone
  ====================*/
IBehavior*  CBAttackMove::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBAttackMove)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBAttackMove::UpdateAggro
  ====================*/
void    CBAttackMove::UpdateAggro()
{
    if (m_uiPrimaryTarget == INVALID_INDEX)
    {
        if (m_uiDelayTarget != INVALID_INDEX && m_uiDelayTime <= Game.GetGameTime())
        {
            if (!m_bDelayReaggroBlock || m_pSelf->GetAttackSequence() != m_uiLastAttackSequence)
            {
                m_uiPrimaryTarget = m_uiDelayTarget;
                m_uiPrimaryTargetTime = Game.GetGameTime() + m_uiDelayDuration;

                if (m_bAttacking)
                    m_Attack.EndBehavior();
                
                m_bAttacking = true;

                m_Attack.EndBehavior();

                m_Attack.Init(m_pBrain, m_pSelf, m_uiPrimaryTarget);
                m_Attack.DisableAggroTrigger();

                m_uiLastAttackSequence = m_pSelf->GetAttackSequence();

                m_uiDelayTarget = INVALID_INDEX;
                m_uiDelayTime = INVALID_TIME;
                m_uiDelayDuration = 0;
                return;
            }
            else
            {
                m_uiDelayTarget = INVALID_INDEX;
                m_uiDelayTime = INVALID_TIME;
                m_uiDelayDuration = 0;
            }
        }
    }

    if (!g_unitAttackMoveDAC)
    {
        // No aggro updates while still in an attack
        if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
            return;
    }

    if (m_uiPrimaryTargetTime != INVALID_TIME && m_uiPrimaryTargetTime < Game.GetGameTime())
    {
        m_uiPrimaryTarget = INVALID_INDEX;
        m_uiPrimaryTargetTime = INVALID_TIME;

        if (m_bAttacking)
            m_Attack.EndBehavior();

        m_bAttacking = false;
    }
    else
    {
        // No aggro updates while still in an attack
        if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
            return;
    }

    if (m_uiPrimaryTarget != INVALID_INDEX)
    {
        IUnitEntity *pPrimaryTarget(Game.GetUnitEntity(m_uiPrimaryTarget));
        if (pPrimaryTarget == nullptr ||
            pPrimaryTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
            !m_pSelf->ShouldTarget(pPrimaryTarget))
        {
            m_uiPrimaryTarget = INVALID_INDEX;
            m_uiPrimaryTargetTime = INVALID_TIME;

            if (m_bAttacking)
                m_Attack.EndBehavior();

            m_bAttacking = false;
        }
    }

    if (m_uiPrimaryTarget != INVALID_INDEX)
        return;

    if (m_uiLastAggroUpdate != INVALID_TIME && m_uiLastAggroUpdate + BEHAVIOR_UPDATE_MS >= Game.GetGameTime())
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

    // Find the most threatening enemy
    float fThreat(-FAR_AWAY);
    uint uiClosestWorldIndex(INVALID_INDEX);
    for (uivector_cit cit(vEntities.begin()), citEnd(vEntities.end()); cit != citEnd; ++cit)
    {
        if (*cit == m_pSelf->GetWorldIndex())
            continue;

        IUnitEntity *pTarget(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*cit)));
        if (pTarget == nullptr)
            continue;
        if (pTarget->IsCritter())
            continue;
        if (!m_pSelf->ShouldTarget(pTarget))
            continue;

        float fCurrent(m_pSelf->GetThreatLevel(pTarget, pTarget->GetIndex() == uiCurrentTargetIndex));

        float fAdjustedAggroRange(fAggroRange);
        if (pTarget->GetTeam() == TEAM_NEUTRAL && pTarget->IsIdle())
            fAdjustedAggroRange /= 3.0f;

        if (fCurrent > fThreat &&
            Distance(pTarget->GetPosition().xy(), v3Position.xy()) - pTarget->GetBounds().GetDim(X) * DIAG <= fAdjustedAggroRange)
        {
            fThreat = fCurrent;
            uiClosestWorldIndex = *cit;
        }
    }

    if (uiClosestWorldIndex != INVALID_INDEX &&
        !(m_bAttacking && m_Attack.GetTarget() == Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex)))
    {
        if (m_bAttacking)
            m_Attack.EndBehavior();

        m_bAttacking = true;

        m_Attack.Init(m_pBrain, m_pSelf, Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex));
        m_Attack.DisableAggroTrigger();
    }
}


/*====================
  CBAttackMove::BeginBehavior
  ====================*/
void    CBAttackMove::BeginBehavior()
{
    if (m_pSelf == nullptr || m_v2UpdatedGoal == V2_ZERO)
    {
        Console << _T("CBAttackMove: Behavior started without valid information") << newl;
        return;
    }

    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    m_pBrain->EndActionStates(1);

    m_uiLastAggroUpdate = INVALID_TIME;

    UpdateAggro();

    if (!m_bAttacking)
        CBMove::BeginBehavior();

    ClearFlag(BSR_NEW);
}


/*====================
  CBAttackMove::ThinkFrame
  ====================*/
void    CBAttackMove::ThinkFrame()
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
  CBAttackMove::MovementFrame
  ====================*/
void    CBAttackMove::MovementFrame()
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
        CBMove::MovementFrame();
}


/*====================
  CBAttackMove::ActionFrame
  ====================*/
void    CBAttackMove::ActionFrame()
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
  CBAttackMove::CleanupFrame
  ====================*/
void    CBAttackMove::CleanupFrame()
{
}


/*====================
  CBAttackMove::EndBehavior
  ====================*/
void    CBAttackMove::EndBehavior()
{
    m_Attack.EndBehavior();
    CBMove::EndBehavior();
}


/*====================
  CBAttackMove::Aggro
  ====================*/
void    CBAttackMove::Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock)
{
    if (pTarget == nullptr)
        return;

    if (uiDelay > 0)
    {
        if (m_uiPrimaryTarget != INVALID_INDEX)
            return;

        if (m_uiDelayTarget != INVALID_INDEX && m_uiDelayTime <= Game.GetGameTime() + uiDelay)
            return;

        m_uiDelayTarget = pTarget->GetIndex();
        m_uiDelayTime = Game.GetGameTime() + uiDelay;
        m_uiDelayDuration = uiDuration;
        m_bDelayReaggroBlock = bReaggroBlock;
    }
    else
    {
        if (m_uiPrimaryTarget != INVALID_INDEX)
            return;

        if (bReaggroBlock && m_pSelf->GetAttackSequence() == m_uiLastAttackSequence)
            return;

        m_uiPrimaryTarget = pTarget->GetIndex();
        m_uiPrimaryTargetTime = Game.GetGameTime() + uiDuration;

        if (m_bAttacking)
            m_Attack.EndBehavior();
        
        m_bAttacking = true;

        m_Attack.EndBehavior();

        m_Attack.Init(m_pBrain, m_pSelf, m_uiPrimaryTarget);
        m_Attack.DisableAggroTrigger();

        m_uiLastAttackSequence = m_pSelf->GetAttackSequence();
    }
}
