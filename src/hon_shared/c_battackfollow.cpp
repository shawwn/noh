// (C)2009 S2 Games
// c_battackfollow.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_battackfollow.h"

#include "c_battack.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBAttackFollow::CopyFrom
  ====================*/
void    CBAttackFollow::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBAttackFollow *pCBBehavior(static_cast<const CBAttackFollow*>(pBehavior));

    m_Attack.CopyFrom(&pCBBehavior->m_Attack);
    m_Attack.SetBrain(m_pBrain);
    m_Attack.SetSelf(m_pSelf);
    m_bAttacking = pCBBehavior->m_bAttacking;
    m_uiLastAggroUpdate = pCBBehavior->m_uiLastAggroUpdate;
    m_uiPrimaryTarget = pCBBehavior->m_uiPrimaryTarget;
    m_uiPrimaryTargetTime = pCBBehavior->m_uiPrimaryTargetTime;

    CBFollow::CopyFrom(pCBBehavior);
}

/*====================
  CBAttackFollow::Clone
  ====================*/
IBehavior*  CBAttackFollow::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBAttackFollow)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBAttackFollow::UpdateAggro
  ====================*/
void    CBAttackFollow::UpdateAggro()
{
    // No aggro updates while still in an attack
    if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
        return;

    if (m_uiPrimaryTargetTime != INVALID_TIME && m_uiPrimaryTargetTime < Game.GetGameTime())
    {
        m_uiPrimaryTarget = INVALID_INDEX;
        m_uiPrimaryTargetTime = INVALID_TIME;

        if (m_bAttacking)
            m_Attack.EndBehavior();

        m_bAttacking = false;
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
  CBAttackFollow::BeginBehavior
  ====================*/
void    CBAttackFollow::BeginBehavior()
{
    if (m_pSelf == nullptr || m_v2UpdatedGoal == V2_ZERO)
    {
        Console << _T("CBAttackFollow: Behavior started without valid information") << newl;
        return;
    }

    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    m_pBrain->EndActionStates(1);

    m_uiLastAggroUpdate = INVALID_TIME;

    UpdateAggro();

    if (!m_bAttacking)
        CBFollow::BeginBehavior();

    ClearFlag(BSR_NEW);
}


/*====================
  CBAttackFollow::ThinkFrame
  ====================*/
void    CBAttackFollow::ThinkFrame()
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
                CBFollow::BeginBehavior();
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
        CBFollow::ThinkFrame();
}


/*====================
  CBAttackFollow::MovementFrame
  ====================*/
void    CBAttackFollow::MovementFrame()
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
        CBFollow::MovementFrame();
}


/*====================
  CBAttackFollow::ActionFrame
  ====================*/
void    CBAttackFollow::ActionFrame()
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
                CBFollow::BeginBehavior();
        }
    }
}


/*====================
  CBAttackFollow::CleanupFrame
  ====================*/
void    CBAttackFollow::CleanupFrame()
{
}


/*====================
  CBAttackFollow::EndBehavior
  ====================*/
void    CBAttackFollow::EndBehavior()
{
    m_Attack.EndBehavior();
    CBFollow::EndBehavior();
}


/*====================
  CBAttackFollow::Aggro
  ====================*/
void    CBAttackFollow::Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock)
{
    if (pTarget == nullptr || m_uiPrimaryTarget != INVALID_INDEX)
        return;

    m_uiPrimaryTarget = pTarget->GetIndex();
    m_uiPrimaryTargetTime = Game.GetGameTime() + uiDuration;

    if (m_bAttacking)
        m_Attack.EndBehavior();
    
    m_bAttacking = true;

    m_Attack.EndBehavior();

    m_Attack.Init(m_pBrain, m_pSelf, m_uiPrimaryTarget);
    m_Attack.DisableAggroTrigger();
}
