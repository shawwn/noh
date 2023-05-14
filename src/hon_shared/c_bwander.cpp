// (C)2009 S2 Games
// c_bwander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bwander.h"
#include "c_asMoving.h"
//=============================================================================

//=============================================================================
//=============================================================================
CVAR_UINTF(behavior_WanderPeriod,   3000,   CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CBWander::CopyFrom
  ====================*/
void    CBWander::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBWander *pCBBehavior(static_cast<const CBWander*>(pBehavior));
    m_v2Origin = pCBBehavior->m_v2Origin;
    m_v2Offset = pCBBehavior->m_v2Offset;
    m_uiLastWanderTime = pCBBehavior->m_uiLastWanderTime;
    m_fDistSq = pCBBehavior->m_fDistSq;
    m_fMinDistanceSq = pCBBehavior->m_fMinDistanceSq;

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBWander::Clone
  ====================*/
IBehavior*  CBWander::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBWander)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBWander::Validate
  ====================*/
bool    CBWander::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    if (m_uiTargetIndex != INVALID_INDEX && Game.GetUnitEntity(m_uiTargetIndex) == nullptr)
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBWander::Update
  ====================*/
void    CBWander::Update()
{
    // Check for path changes
    m_uiLastUpdate = Game.GetGameTime();
    m_v2UpdatedGoal = m_v2Origin + m_v2Offset;
    FindPathToUpdatedGoal();
}


/*====================
  CBWander::BeginBehavior
  ====================*/
void    CBWander::BeginBehavior()
{
    if (m_pSelf == nullptr)
    {
        Console << _T("CBWander: Behavior started without valid information") << newl;
        return;
    }

    m_pBrain->EndActionStates(1);

    m_uiLastUpdate = 0;
    m_uiLastWanderTime = Game.GetGameTime();

    m_v2Origin = m_v2UpdatedGoal;
    m_v2Offset = M_RandomPointInCircle() * m_pSelf->GetWanderRange();

    ClearFlag(BSR_NEW);
}


/*====================
  CBWander::ThinkFrame
  ====================*/
void    CBWander::ThinkFrame()
{
    IUnitEntity *pTarget(nullptr);

    if (m_uiTargetIndex != INVALID_INDEX)
    {
        pTarget = Game.GetUnitEntity(m_uiTargetIndex);
        if (pTarget == nullptr)
            return;

        m_v2Origin = pTarget->GetPosition().xy();
    }

    // Check for a target that has become invalid
    if (pTarget != nullptr && (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE || !m_pSelf->CanSee(pTarget)))
    {
        SetFlag(BSR_END);
        return;
    }

    if (m_uiLastWanderTime == INVALID_TIME || Game.GetGameTime() - m_uiLastWanderTime > behavior_WanderPeriod)
    {
        m_v2Offset = M_RandomPointInCircle() * m_pSelf->GetWanderRange();
        m_uiLastWanderTime = Game.GetGameTime();
    }

    //IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // If out of range we'll need to chase target
    const CVec2f &v2Position(m_pSelf->GetPosition().xy());
    CVec2f v2TargetPosition(m_v2Origin + m_v2Offset);
    
    m_fDistSq = DistanceSq(v2Position, v2TargetPosition);

    m_fMinDistanceSq = m_pSelf->GetBounds().GetDim(X) * DIAG;
    if (pTarget != nullptr)
        m_fMinDistanceSq += pTarget->GetBounds().GetDim(X) * DIAG;
    m_fMinDistanceSq *= m_fMinDistanceSq;

    if (m_fDistSq > m_fMinDistanceSq)
    {
        m_pBrain->SetMoving(true);
    }
    else 
    {
        m_v2Offset = M_RandomPointInCircle() * m_pSelf->GetWanderRange();
        m_uiLastWanderTime = Game.GetGameTime();

        CVec2f v2TargetPosition(m_v2Origin + m_v2Offset);

        m_fDistSq = DistanceSq(v2Position, v2TargetPosition);

        if (m_fDistSq <= m_fMinDistanceSq)
            m_pBrain->SetMoving(false);
        else
            m_pBrain->SetMoving(true);
    }
}


/*====================
  CBWander::MovementFrame
  ====================*/
void    CBWander::MovementFrame()
{
    IUnitEntity *pTarget(nullptr);

    if (m_uiTargetIndex != INVALID_INDEX)
    {
        pTarget = Game.GetUnitEntity(m_uiTargetIndex);
        if (pTarget == nullptr)
            return;
    }

    // Check for a target that has become invalid
    if (pTarget != nullptr && (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE || !m_pSelf->CanSee(pTarget)))
    {
        SetFlag(BSR_END);
        return;
    }

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // if out of range chase target
    if (m_fDistSq <= m_fMinDistanceSq)
        return;

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    if (m_uiLastUpdate == 0)
        Update();

    // Perform updates to path when moving and the target has strayed
    float fNewDistSq(DistanceSq(m_v2UpdatedGoal, m_v2Origin + m_v2Offset));
    if (fNewDistSq > SQR(64.0f))
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

    if (bAtGoal)
        m_uiLastWanderTime = INVALID_TIME;
}


/*====================
  CBWander::ActionFrame
  ====================*/
void    CBWander::ActionFrame()
{
    IUnitEntity *pTarget(nullptr);

    if (m_uiTargetIndex != INVALID_INDEX)
    {
        pTarget = Game.GetUnitEntity(m_uiTargetIndex);
        if (pTarget == nullptr)
            return;
    }

    // Check for a target that has become invalid
    if (pTarget != nullptr && (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE || !m_pSelf->CanSee(pTarget)))
    {
        SetFlag(BSR_END);
        return;
    }

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    
    if (~pGoalState->GetFlags() & ASR_ACTIVE)
        return;

    // End movement if we're inside min follow range
    const CVec2f &v2Position(m_pSelf->GetPosition().xy());
    const CVec2f &v2TargetPosition(m_v2Origin + m_v2Offset);
    
    float fDistSq(DistanceSq(v2Position, v2TargetPosition));

    float fMinDistanceSq(m_pSelf->GetBounds().GetDim(X) * DIAG);
    if (pTarget != nullptr)
        fMinDistanceSq += pTarget->GetBounds().GetDim(X) * DIAG;
    fMinDistanceSq *= fMinDistanceSq;

    if (fDistSq <= fMinDistanceSq)
    {
        pGoalState->EndState(0);

        // End follow if we have pending behaviors
        if (m_pBrain->GetBehaviorsPending() > 0)
            SetFlag(BSR_END);
    }
}


/*====================
  CBWander::CleanupFrame
  ====================*/
void    CBWander::CleanupFrame()
{
}
