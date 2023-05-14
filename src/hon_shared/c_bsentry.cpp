// (C)2008 S2 Games
// c_bsentry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bsentry.h"
#include "c_asMoving.h"
#include "c_asAttacking.h"
#include "c_brain.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBSentry::CopyFrom
  ====================*/
void    CBSentry::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBSentry *pCBBehavior(static_cast<const CBSentry*>(pBehavior));

    m_uiCurrentTargetIndex = pCBBehavior->m_uiCurrentTargetIndex;
    m_uiCurrentTargetOrderDisjointSequence = pCBBehavior->m_uiCurrentTargetOrderDisjointSequence;

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBSentry::Clone
  ====================*/
IBehavior*  CBSentry::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBSentry)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBSentry::Validate
  ====================*/
bool    CBSentry::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBSentry::Update
  ====================*/
void    CBSentry::Update()
{
    // If our current target disjointed, re-target
    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiCurrentTargetIndex));
    if (pTarget != nullptr && m_uiCurrentTargetOrderDisjointSequence != pTarget->GetOrderDisjointSequence())
    {
        m_uiCurrentTargetIndex = INVALID_INDEX;
        pTarget = nullptr;
    }

    static uivector vEntities;
    CVec3f v3Position(m_pSelf->GetPosition());
    float fAttackRange(m_pSelf->GetAttackRange() + m_pSelf->GetBounds().GetDim(X) * DIAG);
    float fAcquireRange(fAttackRange + 64.0f);
    CBBoxf bbRegion(CVec3f(v3Position.xy() - CVec2f(fAcquireRange), -FAR_AWAY),  CVec3f(v3Position.xy() + CVec2f(fAcquireRange), FAR_AWAY));

    // Fetch
    Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_ACTIVE_UNIT);

    // Find most threating enemy
    float fThreat(-FAR_AWAY);
    uint uiClosestWorldIndex(INVALID_INDEX);
    uivector::const_iterator citEnd(vEntities.end());
    for (uivector::const_iterator cit(vEntities.begin()); cit != citEnd; ++cit)
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

        float fCurrent(m_pSelf->GetThreatLevel(pTarget, pTarget->GetIndex() == m_uiCurrentTargetIndex));

        if (fCurrent > fThreat &&
            Distance(pTarget->GetPosition().xy(), v3Position.xy()) - pTarget->GetBounds().GetDim(X) * DIAG <= fAttackRange)
        {
            fThreat = fCurrent;
            uiClosestWorldIndex = *cit;
        }
    }

    if (uiClosestWorldIndex != INVALID_INDEX)
    {
        m_uiCurrentTargetIndex = Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex);
    }

    m_uiLastUpdate = Game.GetGameTime();
}


/*====================
  CBSentry::BeginBehavior
  ====================*/
void    CBSentry::BeginBehavior()
{
    if (m_pSelf == nullptr)
    {
        Console << _T("CBSentry: Behavior started without valid information") << newl;
        return;
    }

    m_pBrain->EndActionStates(1);

    m_uiLastUpdate = INVALID_TIME;

    ClearFlag(BSR_NEW);
}


/*====================
  CBSentry::ThinkFrame
  ====================*/
void    CBSentry::ThinkFrame()
{
    m_pBrain->SetMoving(false);
}


/*====================
  CBSentry::MovementFrame
  ====================*/
void    CBSentry::MovementFrame()
{
}


/*====================
  CBSentry::ActionFrame
  ====================*/
void    CBSentry::ActionFrame()
{
    CASAttacking *pAttackingState(static_cast<CASAttacking*>(m_pBrain->GetActionState(ASID_ATTACKING)));

    if (pAttackingState->GetFlags() & ASR_ACTIVE)
        return;

    // Perform updates
    uint uiGameTime(Game.GetGameTime());
    if (m_uiLastUpdate == INVALID_TIME || m_uiLastUpdate + BEHAVIOR_UPDATE_MS < uiGameTime)
        Update();

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiCurrentTargetIndex));
    if (pTarget == nullptr)
        return;

    // Check for a target that has become invalid
    if (!m_pSelf->ShouldTarget(pTarget))
    {
        m_uiCurrentTargetIndex = INVALID_INDEX;
        return;
    }

    CVec3f v3Position(m_pSelf->GetPosition());
    CVec3f v3TargetPosition(pTarget->GetPosition());
    float fDistSq(DistanceSq(v3Position.xy(), v3TargetPosition.xy()));
    float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange() + pTarget->GetBounds().GetDim(X) * DIAG);

    if (fDistSq > SQR(fRange))
        return;

    // Set attack begin params
    pAttackingState->SetBeginAttackTarget(m_uiCurrentTargetIndex);

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_ATTACKING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_ATTACKING));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Set attack state params
    pAttackingState->SetAttackTarget(m_uiCurrentTargetIndex);
    m_uiCurrentTargetOrderDisjointSequence = pTarget->GetOrderDisjointSequence();
}


/*====================
  CBSentry::CleanupFrame
  ====================*/
void    CBSentry::CleanupFrame()
{
}

