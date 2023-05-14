// (C)2007 S2 Games
// c_bmove.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bmove.h"

#include "c_asMoving.h"
#include "c_brain.h"
#include "i_unitentity.h"

#include "../k2/c_path.h"
//=============================================================================

/*====================
  CBMove::CopyFrom
  ====================*/
void    CBMove::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBMove *pCBBehavior(static_cast<const CBMove*>(pBehavior));

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBMove::Clone
  ====================*/
IBehavior*  CBMove::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBMove)() );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBMove::Validate
  ====================*/
bool    CBMove::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    if (!m_pSelf->GetIsMobile() || m_v2UpdatedGoal == V2_ZERO)
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBMove::Update
  ====================*/
void    CBMove::Update()
{
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    if (pActiveState != pGoalState)
        return;

    if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
        return;

    // Check for path changes
    if (m_uiLastUpdate == INVALID_TIME ||
        static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock() ||
        m_hPath == INVALID_POOL_HANDLE)
    {
        FindPathToUpdatedGoal();
    }

    m_uiLastUpdate = Game.GetGameTime();
}


/*====================
  CBMove::BeginBehavior
  ====================*/
void    CBMove::BeginBehavior()
{
    if (m_pSelf == nullptr || m_v2UpdatedGoal == V2_ZERO)
    {
        Console << _T("CBMove: Behavior started without valid information") << newl;
        return;
    }

    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    m_pBrain->EndActionStates(1);

    m_uiLastUpdate = INVALID_TIME;

    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    ClearFlag(BSR_NEW);
}


/*====================
  CBMove::ThinkFrame
  ====================*/
void    CBMove::ThinkFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    // Not ready to process a new behavior yet
    if (pActiveState != pGoalState)
        return;

    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);
    bool bAtGoal(false);

    // Turn toward destination if immobilized but can still rotate
    if (m_pSelf->IsImmobilized(true, false) && !m_pSelf->IsImmobilized(false, true))
    {
        float fDeltaTime(MsToSec(Game.GetFrameLength()));

        v2Movement = CVec2f(0.0f, 0.0f);

        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) >= SQR(0.001f))
        {
            fGoalYaw = M_YawToPosition(m_pSelf->GetPosition(), CVec3f(m_v2UpdatedGoal, 0.0f));
            fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];
        }
        else
        {
            fGoalYaw = v3Angles[YAW];
            fYawDelta = 0.0f;
        }

        bAtGoal = false;
    }
    else
    {
        GetMovement(v2Movement, fYawDelta, bAtGoal, fGoalYaw);
    }

    m_pBrain->SetMoving((!bAtGoal || m_uiLastUpdate == INVALID_TIME) &&
        !static_cast<CASMoving *>(pGoalState)->IsBlocked() &&
        m_pSelf->GetPosition().xy() != m_v2UpdatedGoal);
}


/*====================
  CBMove::MovementFrame
  ====================*/
void    CBMove::MovementFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    // Not ready to process a new behavior yet
    if (pActiveState != pGoalState)
        return;

    // Perform updates
    if (((m_uiLastUpdate == INVALID_TIME || m_uiLastUpdate + BEHAVIOR_UPDATE_MS < Game.GetGameTime()) && !static_cast<CASMoving *>(pGoalState)->GetBlocked()) ||
        static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock())
        Update();

    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);
    bool bAtGoal(false);

    // Turn toward destination if immobilized but can still rotate
    if (m_pSelf->IsImmobilized(true, false) && !m_pSelf->IsImmobilized(false, true))
    {
        float fDeltaTime(MsToSec(Game.GetFrameLength()));

        v2Movement = CVec2f(0.0f, 0.0f);

        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal) >= SQR(0.001f))
        {
            fGoalYaw = M_YawToPosition(m_pSelf->GetPosition(), CVec3f(m_v2UpdatedGoal, 0.0f));
            fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];
        }
        else
        {
            fGoalYaw = v3Angles[YAW];
            fYawDelta = 0.0f;
        }

        bAtGoal = false;
    }
    else
    {
        GetMovement(v2Movement, fYawDelta, bAtGoal, fGoalYaw);
    }

    if (bAtGoal)
        SetFlag(BSR_END | BSR_SUCCESS);

    // Process an updated moving state
    CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
    pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);

    m_pSelf->SetAttentionYaw(fGoalYaw);
}


/*====================
  CBMove::CleanupFrame
  ====================*/
void    CBMove::CleanupFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));

    // Not ready to process a new behavior yet
    if (pActiveState != pGoalState)
        return;
}


/*====================
  CBMove::EndBehavior
  ====================*/
void    CBMove::EndBehavior()
{
    IBehavior::EndBehavior();

    m_pBrain->SetMoving(false);
}


/*====================
  CBMove::Moved
  ====================*/
void    CBMove::Moved()
{
    IBehavior::Moved();
    m_uiLastUpdate = INVALID_TIME;
}

