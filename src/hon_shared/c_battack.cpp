// (C)2007 S2 Games
// c_battack.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_battack.h"

#include "c_asMoving.h"
#include "c_asAttacking.h"
#include "c_brain.h"
#include "i_unitentity.h"
//=============================================================================

/*====================
  CBAttack::CBAttack
  ====================*/
CBAttack::CBAttack(uint uiArmingSequence, uint uiMaxAttacks) :
IBehavior(EBT_ATTACK),
m_bAtGoal(false),
m_uiArmingSequence(uiArmingSequence),
m_uiAttacks(0),
m_uiMaxAttacks(uiMaxAttacks),
m_bAggroTrigger(false)
{
}


/*====================
  CBAttack::CopyFrom
  ====================*/
void    CBAttack::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBAttack *pCBBehavior(static_cast<const CBAttack*>(pBehavior));

    m_fDistSq = pCBBehavior->m_fDistSq;     
    m_v2ApproachPosition = pCBBehavior->m_v2ApproachPosition;
    m_fRange = pCBBehavior->m_fRange;
    m_bAtGoal = pCBBehavior->m_bAtGoal;
    m_bSight = pCBBehavior->m_bSight;
    m_uiArmingSequence = pCBBehavior->m_uiArmingSequence;
    m_uiAttacks = pCBBehavior->m_uiAttacks;
    m_uiMaxAttacks = pCBBehavior->m_uiMaxAttacks;
    m_bAggroTrigger = pCBBehavior->m_bAggroTrigger;

    IBehavior::CopyFrom(pCBBehavior);
}


/*====================
  CBAttack::Clone
  ====================*/
IBehavior*  CBAttack::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBAttack)(m_uiArmingSequence) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}


/*====================
  CBAttack::Init
  ====================*/
void    CBAttack::Init(CBrain *pBrain, IUnitEntity *pSelf, uint uiTargetIndex, uint uiMaxAttacks)
{
    (*this) = CBAttack(pSelf->GetArmingSequence(), uiMaxAttacks);
    m_pBrain = pBrain;
    m_pSelf = pSelf;
    m_uiTargetIndex = uiTargetIndex;

    // Update disjoint sequence because we're beginning an attack
    const IUnitEntity *pTargetUnit = Game.GetUnitEntity(uiTargetIndex);
    if (pTargetUnit != nullptr)
        m_uiTargetOrderDisjointSequence = pTargetUnit->GetOrderDisjointSequence();
}


/*====================
  CBAttack::Validate
  ====================*/
bool    CBAttack::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    IUnitEntity *pTarget(Game.GetUnitEntity(GetTarget()));

    if (pTarget == nullptr || (m_uiEndTime != INVALID_TIME && m_uiEndTime <= Game.GetGameTime()))
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBAttack::Update
  ====================*/
void    CBAttack::Update()
{
    if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
        return;

    // Check for path changes
    m_uiLastUpdate = Game.GetGameTime();
    m_v2UpdatedGoal = m_v2ApproachPosition;
    m_bAtGoal = false;
    FindPathToUpdatedGoal();
}


/*====================
  CBAttack::BeginBehavior
  ====================*/
void    CBAttack::BeginBehavior()
{
    if (m_pSelf == nullptr)
    {
        Console << _T("CBAttack: Behavior started without valid information") << newl;
        return;
    }

    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    // Don't interrupt an on-going attack on the same target
    CASAttacking *pAttackingState(static_cast<CASAttacking*>(m_pBrain->GetActionState(ASID_ATTACKING)));

    if (~pAttackingState->GetFlags() & ASR_ACTIVE ||
        pAttackingState->GetAttackTarget() != GetTarget())
        m_pBrain->EndActionStates(1);
    
    m_fGoalRange = FAR_AWAY;
    m_fDistSq = FAR_AWAY;
    m_v2ApproachPosition = V2_ZERO;
    m_fRange = 0.0f;
    m_bSight = false;
    m_fGoalRange = 0.0f;

    if (m_hPath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hPath);
        m_hPath = INVALID_POOL_HANDLE;
    }

    ClearFlag(BSR_NEW);

    m_uiLastUpdate = INVALID_TIME;

    // Check for a target that has become invalid
    IUnitEntity *pTarget(Game.GetUnitEntity(GetTarget()));
    if (pTarget == nullptr ||
        (!m_pSelf->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        SetFlag(BSR_END);
        return;
    }

    m_pSelf->SetTargetIndex(pTarget->GetIndex());

    m_bAggroTrigger = false;

    // If we want this movement to move to the last known position of the unit,
    // we'll need to track that outside of the behavior code

    m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
    m_bSight = true;
}


/*====================
  CBAttack::ThinkFrame
  ====================*/
void    CBAttack::ThinkFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    // Only update think variables on valid targets, otherwise reset
    IUnitEntity *pTarget(Game.GetUnitEntity(GetTarget()));

    if (pTarget != nullptr &&
        pTarget->GetStatus() == ENTITY_STATUS_ACTIVE)
    {
        CVec3f v3Position(m_pSelf->GetPosition());
        CVec3f v3TargetPosition(pTarget->GetPosition());
        
        m_fDistSq = DistanceSq(v3Position.xy(), v3TargetPosition.xy());

        m_bSight = m_pSelf->CanSee(pTarget);

        if (m_bSight || pTarget->GetAlwaysTargetable())
            m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
        
        m_fRange = m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange() + pTarget->GetBounds().GetDim(X) * DIAG;
        
        m_pSelf->SetTargetIndex(pTarget->GetIndex());
    }
    else
    {
        m_fDistSq = FAR_AWAY;
        m_fRange = 0.0f;
        m_bSight = false;
        m_bAtGoal = false;
        m_pSelf->SetTargetIndex(INVALID_INDEX);
    }

    // Cancel attack orders on out of range targets
    if (!m_pSelf->GetIsMobile() && m_fDistSq > SQR(m_fRange))
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        m_pBrain->SetMoving(false);
        SetFlag(BSR_END);
        return;
    }

    m_fGoalRange = MAX(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange() - PATH_RECALC_DISTANCE, 0.0f);

    // Don't interrupt an on-going attack on the same target
    CASAttacking *pAttackingState(static_cast<CASAttacking*>(m_pBrain->GetActionState(ASID_ATTACKING)));
    
    if ((pAttackingState->GetFlags() & ASR_ACTIVE) &&
        pAttackingState->GetAttackTarget() == GetTarget())
    {
        m_pBrain->SetMoving(false);
        return;
    }

    if (m_uiMaxAttacks != uint(-1) && m_uiAttacks >= m_uiMaxAttacks)
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        m_pBrain->SetMoving(false);
        SetFlag(BSR_END);
        return;
    }

    // Check for a target that has become invalid after our current attack had ended
    if (pTarget == nullptr ||
        pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        m_pBrain->SetMoving(false);
        SetFlag(BSR_END);
        return;
    }

    if (m_fDistSq <= SQR(m_fRange) && m_bSight)
    {
        m_pBrain->SetMoving(false);
        return;
    }

    if (!m_bAggroTrigger)
    {
        float fAggroTriggerRange(g_unitAttackAggroTriggerRange + m_pSelf->GetBounds().GetDim(X) * DIAG);

        if (Distance(m_pSelf->GetPosition().xy(), pTarget->GetPosition().xy()) < fAggroTriggerRange + pTarget->GetBounds().GetDim(X) * DIAG)
        {
            if (pTarget->IsHero() && !pTarget->IsIllusion() && pTarget->GetTeam() != m_pSelf->GetTeam())
                m_pSelf->AggroCreeps(g_heroAttackAggroRange, g_heroAttackAggroTime, pTarget->GetTeam(), g_heroAttackAggroDelay, g_heroAttackReaggroBlock);

            m_bAggroTrigger = true;
        }
    }

    // if out of range, chase target
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    
    // Not ready yet
    if (pActiveState != pGoalState)
    {
        m_pBrain->SetMoving(false);
        return;
    }

    // Moving unless blocked
    m_pBrain->SetMoving(m_uiLastUpdate == INVALID_TIME || !static_cast<CASMoving *>(pGoalState)->IsBlocked());
}


/*====================
  CBAttack::MovementFrame
  ====================*/
void    CBAttack::MovementFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    IUnitEntity *pTarget(Game.GetUnitEntity(GetTarget()));
    if (pTarget == nullptr)
        return;

    if (m_fDistSq <= SQR(m_fRange) && m_bSight)
        return;

    // Don't interrupt an on-going attack on the same target
    CASAttacking *pAttackingState(static_cast<CASAttacking*>(m_pBrain->GetActionState(ASID_ATTACKING)));
    
    if ((pAttackingState->GetFlags() & ASR_ACTIVE) &&
        pAttackingState->GetAttackTarget() == GetTarget())
    {
        return;
    }

    if (m_uiMaxAttacks != uint(-1) && m_uiAttacks >= m_uiMaxAttacks)
    {
        return;
    }

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    if (!m_pSelf->IsStunned() && !m_pSelf->IsImmobilized(true, true))
    {
        // Perform updates to path when moving and the target has strayed
        float fNewDistSq(DistanceSq(m_v2UpdatedGoal, m_v2ApproachPosition));
        if (((m_uiLastUpdate == INVALID_TIME || fNewDistSq > SQR(PATH_RECALC_DISTANCE)) && !static_cast<CASMoving *>(pGoalState)->GetBlocked()) ||
            static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock())
            Update();
    }

    // Set move state params
    CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
    
    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);
    
    if (!m_bAtGoal)
    {
        // Turn toward destination if immobilized but can still rotate
        if (m_pSelf->IsImmobilized(true, false) && !m_pSelf->IsImmobilized(false, true))
        {
            float fDeltaTime(MsToSec(Game.GetFrameLength()));

            v2Movement = CVec2f(0.0f, 0.0f);

            if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2ApproachPosition) >= SQR(0.001f))
            {
                fGoalYaw = M_YawToPosition(m_pSelf->GetPosition(), CVec3f(m_v2ApproachPosition, 0.0f));
                fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];
            }
            else
            {
                fGoalYaw = v3Angles[YAW];
                fYawDelta = 0.0f;
            }

            m_bAtGoal = false;
        }
        else
        {
            GetMovement(v2Movement, fYawDelta, m_bAtGoal, fGoalYaw);
        }

        m_pSelf->SetAttentionYaw(fGoalYaw);
    }

    if (m_bAtGoal && (m_bSight || pTarget->GetAlwaysTargetable()))
    {
        // if we're still out of range head straight for the target
        CVec2f v2Position(m_pSelf->GetPosition().xy());
        float fDistSq(DistanceSq(v2Position, m_v2ApproachPosition));
        float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange() + pTarget->GetBounds().GetDim(X) * DIAG);

        if (fDistSq > SQR(fRange))
        {
            float fDeltaTime(MsToSec(Game.GetFrameLength()));
            CVec2f v2DirectionOfMovement(m_v2ApproachPosition - v2Position);
            float fDistance(v2DirectionOfMovement.Normalize());

            // Turn the unit to face the direction of movement as we progress
            float fGoalYaw(M_GetYawFromForwardVec2(v2DirectionOfMovement));

            CVec3f v3Angles(m_pSelf->GetAngles());

            fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];

            fDistance = MIN(fDistance, m_pSelf->GetMoveSpeed() * fDeltaTime);

            v2Movement = v2DirectionOfMovement * fDistance;

            m_pSelf->SetAttentionYaw(M_GetYawFromForwardVec2(v2DirectionOfMovement));
        }
    }

    pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);
}


/*====================
  CBAttack::ActionFrame
  ====================*/
void    CBAttack::ActionFrame()
{
    if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
        return;

    // Don't interrupt an on-going attack on the same target
    CASAttacking *pAttackingState(static_cast<CASAttacking*>(m_pBrain->GetActionState(ASID_ATTACKING)));
    
    if ((pAttackingState->GetFlags() & ASR_ACTIVE) &&
        pAttackingState->GetAttackTarget() == GetTarget())
        return;

    if (m_uiMaxAttacks != uint(-1) && m_uiAttacks >= m_uiMaxAttacks)
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        SetFlag(BSR_END);
        return;
    }

    IUnitEntity *pTarget(Game.GetUnitEntity(GetTarget()));

    bool bAlwaysTargetable(pTarget != nullptr && pTarget->GetAlwaysTargetable());

    // Chase until we reach last sighted position
    if (!m_bSight && !bAlwaysTargetable)
    {
        if (m_bAtGoal)
        {
            m_pSelf->SetTargetIndex(INVALID_INDEX);
            SetFlag(BSR_END);
        }
        return;
    }

    // If in range, attack target
    CVec2f v2Position(m_pSelf->GetPosition().xy());
    CVec2f v2TargetPosition(pTarget->GetPosition().xy());
    float fDistSq(DistanceSq(v2Position, v2TargetPosition));
    float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange() + pTarget->GetBounds().GetDim(X) * DIAG);

    // check if were in range during either setup frame or action frame
    if (m_fDistSq > SQR(m_fRange) && fDistSq > SQR(fRange))
        return;

    // Don't attack if we don't have sight, even if we are always targetable
    if (!m_bSight)
        return;

    IActionState *pGoalState(m_pBrain->GetActionState(ASID_ATTACKING));

    // Attempt to cancel the previous attack if we made it this far (new target, etc)
    if (pGoalState->GetFlags() & ASR_ACTIVE)
        pGoalState->EndState(0);

    // Check for a target that has become invalid
    if (!Game.IsValidTarget(m_pSelf->GetAttackTargetScheme(), 0, m_pSelf, pTarget))
    {
        m_pSelf->SetTargetIndex(INVALID_INDEX);
        SetFlag(BSR_END);
        return;
    }

    if (m_uiArmingSequence != m_pSelf->GetArmingSequence())
        return;

    // Set attack begin params
    pAttackingState->SetBeginAttackTarget(GetTarget());
        
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_ATTACKING, 0));
    
    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Set attack state params
    pAttackingState->SetAttackTarget(GetTarget());
    
    ++m_uiAttacks;
}


/*====================
  CBAttack::CleanupFrame
  ====================*/
void    CBAttack::CleanupFrame()
{
}
