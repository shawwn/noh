// (C)2008 S2 Games
// c_bfollow.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bfollow.h"

#include "c_asMoving.h"
#include "c_brain.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBFollow::CopyFrom
  ====================*/
void    CBFollow::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBFollow *pCBBehavior(static_cast<const CBFollow*>(pBehavior));

    m_fMinFollowDistance = pCBBehavior->m_fMinFollowDistance;
    m_fMaxFollowDistance = pCBBehavior->m_fMaxFollowDistance;

    m_fDistSq = pCBBehavior->m_fDistSq;
    m_v2ApproachPosition = pCBBehavior->m_v2ApproachPosition;
    m_fFollowRangeMin = pCBBehavior->m_fFollowRangeMin;
    m_fFollowRangeMax = pCBBehavior->m_fFollowRangeMax;

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBFollow::Clone
  ====================*/
IBehavior*  CBFollow::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(g_heapAI,    CBFollow)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBFollow::Validate
  ====================*/
bool    CBFollow::Validate()
{
    if (m_pBrain == NULL ||
        m_pSelf == NULL ||
        GetFlags() & BSR_END ||
        Game.GetUnitEntity(m_uiTargetIndex) == NULL)
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBFollow::Update
  ====================*/
void    CBFollow::Update()
{
    if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
        return;

    // Check for path changes
    m_uiLastUpdate = Game.GetGameTime();
    m_v2UpdatedGoal = m_v2ApproachPosition;
    FindPathToUpdatedGoal();
}


/*====================
  CBFollow::BeginBehavior
  ====================*/
void    CBFollow::BeginBehavior()
{
    if (m_pSelf == NULL)
    {
        Console << _T("CBFollow: Behavior started without valid information") << newl;
        return;
    }

    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    m_pBrain->EndActionStates(1);

    m_uiLastUpdate = INVALID_TIME;

    ClearFlag(BSR_NEW);
}


/*====================
  CBFollow::ThinkFrame
  ====================*/
void    CBFollow::ThinkFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == NULL)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        (!m_pSelf->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
    {
        SetFlag(BSR_END);
        return;
    }

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // if out of range we'll need to chase target
    CVec3f v3Position(m_pSelf->GetPosition());
    CVec3f v3TargetPosition(pTarget->GetPosition());
    
    m_fDistSq = DistanceSq(v3Position.xy(), v3TargetPosition.xy());
    m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
    
    m_fFollowRangeMin = m_pSelf->GetBounds().GetDim(X) * DIAG + m_fMinFollowDistance + pTarget->GetBounds().GetDim(X) * DIAG;
    m_fFollowRangeMax = m_pSelf->GetBounds().GetDim(X) * DIAG + m_fMaxFollowDistance + pTarget->GetBounds().GetDim(X) * DIAG;

    if (m_fDistSq > SQR(m_fFollowRangeMax) || (pGoalState->GetFlags() & ASR_ACTIVE && m_fDistSq > SQR(m_fFollowRangeMin)))
        m_pBrain->SetMoving(!static_cast<CASMoving *>(pGoalState)->IsBlocked());
    else 
    {
        if (pGoalState->GetFlags() & ASR_ACTIVE)
            pGoalState->EndState(0);

        m_pBrain->SetMoving(false);
    }
}


/*====================
  CBFollow::MovementFrame
  ====================*/
void    CBFollow::MovementFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == NULL)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        (!m_pSelf->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
    {
        SetFlag(BSR_END);
        return;
    }

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // if out of range chase target
    if (m_fDistSq <= SQR(m_fFollowRangeMax) && !(pGoalState->GetFlags() & ASR_ACTIVE && m_fDistSq > SQR(m_fFollowRangeMin)))
        return;

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Perform updates to path when moving and the target has strayed
    float fNewDistSq(DistanceSq(m_v2UpdatedGoal, m_v2ApproachPosition));
    if (((m_uiLastUpdate == INVALID_TIME || fNewDistSq > SQR(PATH_RECALC_DISTANCE)) && !static_cast<CASMoving *>(pGoalState)->GetBlocked()) ||
        static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock())
        Update();

    // Set move state params
    CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
    
    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);
    bool bAtGoal(false);

    GetMovement(v2Movement, fYawDelta, bAtGoal, fGoalYaw);

    pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);

    m_pSelf->SetAttentionYaw(fGoalYaw);
    
    ClearFlag(BSR_SUCCESS);
}


/*====================
  CBFollow::ActionFrame
  ====================*/
void    CBFollow::ActionFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == NULL)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        (!m_pSelf->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
    {
        SetFlag(BSR_END);
        return;
    }

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    
    if (~pGoalState->GetFlags() & ASR_ACTIVE)
        return;

    // End movement if we're inside min follow range
    CVec3f v3Position(m_pSelf->GetPosition());
    CVec3f v3TargetPosition(pTarget->GetPosition());
    
    float fDistSq(DistanceSq(v3Position.xy(), v3TargetPosition.xy()));
    CVec2f v2ApproachPosition(pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy());
    
    float fFollowRangeMin(m_pSelf->GetBounds().GetDim(X) * DIAG + m_fMinFollowDistance + pTarget->GetBounds().GetDim(X) * DIAG);

    if (fDistSq <= SQR(fFollowRangeMin))
    {
        pGoalState->EndState(0);

        SetFlag(BSR_SUCCESS);

        // End follow if we have pending behaviors
        if (m_pBrain->GetBehaviorsPending() > 0)
            SetFlag(BSR_END);
    }
}


/*====================
  CBFollow::CleanupFrame
  ====================*/
void    CBFollow::CleanupFrame()
{
}
