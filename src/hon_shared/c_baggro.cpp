// (C)2009 S2 Games
// c_baggro.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_baggro.h"

#include "c_battack.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBAggro::CopyFrom
  ====================*/
void    CBAggro::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBAggro *pCBBehavior(static_cast<const CBAggro*>(pBehavior));

    m_Attack.CopyFrom(&pCBBehavior->m_Attack);
    m_Attack.SetBrain(m_pBrain);
    m_Attack.SetSelf(m_pSelf);
    m_bAttacking = pCBBehavior->m_bAttacking;
    m_uiNextAggroUpdate = pCBBehavior->m_uiNextAggroUpdate;
    m_uiPrimaryTarget = pCBBehavior->m_uiPrimaryTarget;
    m_uiPrimaryTargetTime = pCBBehavior->m_uiPrimaryTargetTime;

    CBHold::CopyFrom(pCBBehavior);
}

/*====================
  CBAggro::Clone
  ====================*/
IBehavior*  CBAggro::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBAggro)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBAggro::UpdateAggro
  ====================*/
void    CBAggro::UpdateAggro()
{
    if (m_uiPrimaryTargetTime != INVALID_TIME && m_uiPrimaryTargetTime < Game.GetGameTime())
    {
        m_uiPrimaryTarget = INVALID_INDEX;
        m_uiPrimaryTargetTime = INVALID_TIME;
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
        }
    }

    if (m_uiPrimaryTarget != INVALID_INDEX)
        return;

    if (m_uiNextAggroUpdate == INVALID_TIME || m_uiNextAggroUpdate > Game.GetGameTime())
        return;

    // No aggro while stealthed
    if (m_pSelf->GetStealthBits() != 0)
        return;

    // No aggro if we're channeling anything that'll break on attack
    if (m_pSelf->IsChanneling(UNIT_ACTION_ATTACK))
        return;

    // No aggro updates while still in an attack
    if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
        return;

    // Update the status of the attackmove behavior
    m_uiNextAggroUpdate = Game.GetGameTime() + BEHAVIOR_UPDATE_MS;

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

        uint uiClosestGameIndex = Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex);

        m_Attack.Init(m_pBrain, m_pSelf, uiClosestGameIndex);
        m_Attack.DisableAggroTrigger();
    }
}


/*====================
  CBAggro::BeginBehavior
  ====================*/
void    CBAggro::BeginBehavior()
{
    if (m_pSelf == nullptr || m_v2UpdatedGoal == V2_ZERO)
    {
        Console << _T("CBAggro: Behavior started without valid information") << newl;
        return;
    }

    m_pBrain->EndActionStates(1);

    m_uiNextAggroUpdate = Game.GetGameTime() + g_unitBehaviorStartAggroDelay;

    UpdateAggro();

    if (!m_bAttacking)
        CBHold::BeginBehavior();

    ClearFlag(BSR_NEW);
}


/*====================
  CBAggro::ThinkFrame
  ====================*/
void    CBAggro::ThinkFrame()
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
            m_uiNextAggroUpdate = Game.GetGameTime() + g_unitBehaviorStartAggroDelay;

            UpdateAggro();

            // Repath towards destination if we didn't find a new target
            if (!m_bAttacking)
                CBHold::BeginBehavior();
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
        CBHold::ThinkFrame();
}


/*====================
  CBAggro::MovementFrame
  ====================*/
void    CBAggro::MovementFrame()
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
        CBHold::MovementFrame();
}


/*====================
  CBAggro::ActionFrame
  ====================*/
void    CBAggro::ActionFrame()
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
            m_uiNextAggroUpdate = Game.GetGameTime() + g_unitBehaviorStartAggroDelay;

            UpdateAggro();

            // Repath towards destination if we didn't find a new target
            if (!m_bAttacking)
                CBHold::BeginBehavior();
        }
    }
}


/*====================
  CBAggro::CleanupFrame
  ====================*/
void    CBAggro::CleanupFrame()
{
}


/*====================
  CBAggro::EndBehavior
  ====================*/
void    CBAggro::EndBehavior()
{
    m_Attack.EndBehavior();
    CBHold::EndBehavior();
}


/*====================
  CBAggro::Aggro
  ====================*/
void    CBAggro::Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock)
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
