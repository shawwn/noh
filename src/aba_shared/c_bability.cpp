// (C)2008 S2 Games
// c_bability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bability.h"

#include "c_brain.h"
#include "i_unitentity.h"
#include "c_asMoving.h"
#include "c_asCasting.h"
#include "i_entitytool.h"
//=============================================================================

/*====================
  CBAbility::CopyFrom
  ====================*/
void    CBAbility::CopyFrom( const IBehavior* pBehavior )
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBAbility *pCBBehavior(static_cast<const CBAbility*>(pBehavior));

    m_fDistSq = pCBBehavior->m_fDistSq;     
    m_v2TargetPosition = pCBBehavior->m_v2TargetPosition;
    m_v2ApproachPosition = pCBBehavior->m_v2ApproachPosition;
    m_fRange = pCBBehavior->m_fRange;
    m_bAtGoal = pCBBehavior->m_bAtGoal;
    m_bSight = pCBBehavior->m_bSight;
    m_hRangePath = pCBBehavior->m_hRangePath;

    m_iInventorySlot = pCBBehavior->m_iInventorySlot;
    m_pAbility = pCBBehavior->m_pAbility;

    IBehavior::CopyFrom(pBehavior);
}


/*====================
  CBAbility::Clone
  ====================*/
IBehavior*  CBAbility::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    bool bSecondary( (GetFlags() & BSR_SECONDARY) != 0 );
    IBehavior* pBehavior( K2_NEW(g_heapAI,    CBAbility)( m_iInventorySlot, bSecondary ) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}


/*====================
  CBAbility::IsTargeted
  ====================*/
bool    CBAbility::IsTargeted()
{
    return m_pAbility->GetActionType() == TOOL_ACTION_TARGET_ENTITY ||
        (m_pAbility->GetActionType() == TOOL_ACTION_ATTACK && ~GetFlags() & BSR_SECONDARY) ||
        ((m_pAbility->GetActionType() == TOOL_ACTION_TARGET_DUAL || m_pAbility->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) && m_uiTargetIndex != INVALID_INDEX);
}


/*====================
  CBAbility::Validate
  ====================*/
bool    CBAbility::Validate()
{
#define FAIL { SetFlag(BSR_END); return false; }
    if (m_pBrain == NULL ||
        m_pSelf == NULL ||
        GetFlags() & BSR_END ||
        m_pSelf->IsIllusion())
        FAIL

    if (m_pSelf->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK) &&
        m_iInventorySlot >= INVENTORY_START_BACKPACK &&
        m_iInventorySlot <= INVENTORY_END_BACKPACK)
        FAIL

    m_pAbility = m_pSelf->GetTool(m_iInventorySlot);
    if (m_pAbility == NULL)
        FAIL

    if ((m_pAbility->GetActionType() == TOOL_ACTION_ATTACK || m_pAbility->GetAllowAutoCast()) && GetFlags() & BSR_SECONDARY)
        m_bInheritMovement = true;
    else
        m_bInheritMovement = m_pAbility->GetInheritMovement();

    if (Game.GetUnitEntity(m_uiTargetIndex) == NULL)
    {
        if (IsTargeted())
            FAIL
    }

    if (m_pAbility->IsDisabled() && ~GetFlags() & BSR_CAST)
        FAIL

    return true;
#undef FAIL
}


/*====================
  CBAbility::Update
  ====================*/
void    CBAbility::Update()
{
    if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
        return;

    // Check for path changes
    m_uiLastUpdate = Game.GetGameTime();
    m_v2UpdatedGoal = m_v2ApproachPosition;
    FindPathToUpdatedGoal();
}


/*====================
  CBAbility::BeginBehavior
  ====================*/
void    CBAbility::BeginBehavior()
{
    if (m_pSelf == NULL || m_pAbility == NULL)
    {
        Console << _T("CBAbility: Behavior started without valid information") << newl;
        return;
    }

    // Target is not valid (pre-EndActionStates)
    if (IsTargeted())
    {
        IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
        if (pTarget == NULL ||
            !m_pAbility->IsValidTarget(pTarget) ||
            pTarget->GetStatus() == ENTITY_STATUS_DORMANT ||
            !m_pSelf->CanSee(pTarget))
        {
            // TODO: Error message
            SetFlag(BSR_END);
            return;
        }
    }

    m_pBrain->EndActionStates(1);

    m_fDistSq = FAR_AWAY;
    m_v2ApproachPosition = V2_ZERO;
    m_fRange = 0.0f;
    m_bSight = false;

    m_uiLastUpdate = INVALID_TIME;
    ClearFlag(BSR_NEW);

    if (IsTargeted())
    {
        // Check for a target that started out invalid
        IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
        if (pTarget == NULL ||
            pTarget->GetStatus() == ENTITY_STATUS_DORMANT ||
            !m_pSelf->CanSee(pTarget))
        {
            SetFlag(BSR_END);
            return;
        }

        // If we want this to move to the last known position of the unit,
        // we'll need to track that outside of the behavior code

        m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
        m_bSight = true;
    }
    else if ((m_pAbility->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_VECTOR ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_CURSOR) && 
        !Game.IsInBounds(m_v2UpdatedGoal.x, m_v2UpdatedGoal.y) &&
        !m_pAbility->GetAllowOutOfBoundsCast())
    {
        SetFlag(BSR_END);
        return;
    }
}


/*====================
  CBAbility::ThinkFrame
  ====================*/
void    CBAbility::ThinkFrame()
{
    // if out of range we'll need to chase target
    CVec2f v2Position(m_pSelf->GetPosition().xy());
        
    m_fRange = m_pSelf->GetBounds().GetDim(X) * DIAG + m_pAbility->GetRange();

    IUnitEntity *pTarget(NULL);
    switch (m_pAbility->GetActionType())
    {
    case TOOL_ACTION_PASSIVE:
    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TOGGLE:
    case TOOL_ACTION_GLOBAL:
    case TOOL_ACTION_TARGET_SELF:
    case TOOL_ACTION_FACING:
    case TOOL_ACTION_SELF_POSITION:
    case TOOL_ACTION_ATTACK_TOGGLE:
        m_v2ApproachPosition = m_v2TargetPosition = v2Position;
        break;

    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_ENTITY:
    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
    case TOOL_ACTION_TARGET_VECTOR:
    case TOOL_ACTION_TARGET_CURSOR:
        if (m_uiTargetIndex != INVALID_INDEX)
        {
            pTarget = Game.GetUnitEntity(m_uiTargetIndex);
            if (pTarget != NULL && m_pSelf->CanSee(pTarget))
            {
                m_v2TargetPosition = pTarget->GetPosition().xy();
                m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
                m_fRange += pTarget->GetBounds().GetDim(X) * DIAG;
                m_bSight = true;
            }
            else
            {
                // Use our last valid position as the target position
                m_v2TargetPosition = m_v2ApproachPosition;
                m_fRange = 0.0f;
                m_bSight = false;
            }
        }
        else
        {
            m_v2ApproachPosition = m_v2TargetPosition = m_v2UpdatedGoal;
        }
        break;
    
    case TOOL_ACTION_ATTACK:
        if (~GetFlags() & BSR_SECONDARY)
        {
            pTarget = Game.GetUnitEntity(m_uiTargetIndex);

            if (m_pAbility->GetRange() == 0.0f)
                m_fRange = m_pSelf->GetBounds().GetDim(X) * DIAG + m_pSelf->GetAttackRange();

            if (pTarget != NULL && m_pSelf->CanSee(pTarget))
            {
                m_v2TargetPosition = pTarget->GetPosition().xy();
                m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
                m_fRange += pTarget->GetBounds().GetDim(X) * DIAG;
                m_bSight = true;
            }
            else
            {
                // Use our last valid position as the target position
                m_v2TargetPosition = m_v2ApproachPosition;
                m_fRange = 0.0f;
                m_bSight = false;
            }
        }
        else
        {
            m_v2ApproachPosition = m_v2TargetPosition = v2Position;
        }
        break;

    case TOOL_ACTION_INVALID:
        Console.Warn << _T("Invalid tool action type") << newl;
        SetFlag(BSR_END);
        return;
    }

    if (m_pAbility->GetUsePathForRange())
        m_fDistSq = FAR_AWAY;
    else
        m_fDistSq = DistanceSq(v2Position, m_v2TargetPosition);

    m_bAtGoal = false;
    
    if (m_fDistSq <= SQR(m_fRange) || m_pAbility->GetAllowOutOfRangeCast())
    {
        m_pBrain->SetMoving(false);
        return;
    }

    // Don't interrupt an ongoing activation
    if (m_pBrain->GetActionState(ASID_CASTING)->GetFlags() & ASR_ACTIVE)
    {
        m_pBrain->SetMoving(false);
        return;
    }

    // Don't interrupt a previous channel of this ability
    if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
    {
        m_pBrain->SetMoving(false);
        return;
    }

    // Wait for current channel to finish
    if (GetFlags() & BSR_CAST && m_pAbility->IsChanneling(UNIT_ACTION_CAST))
    {
        m_pBrain->SetMoving(false);
        return;
    }
    
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    
    if (m_fDistSq < SQR(m_pAbility->GetMinRange()) && 
        ~GetFlags() & BSR_CAST)
    {
        SetFlag(BSR_END);
        return;
    }

    if ((m_pAbility->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_VECTOR ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
        &&
        !Game.IsInBounds(m_v2TargetPosition.x, m_v2TargetPosition.y) &&
        !m_pAbility->GetAllowOutOfBoundsCast())
    {
        SetFlag(BSR_END);
        return;
    }

    // Not ready yet
    if (pActiveState != pGoalState)
    {
        m_pBrain->SetMoving(false);
        return;
    }

    // Always set moving unless blocked
    m_pBrain->SetMoving(m_uiLastUpdate == INVALID_TIME || !static_cast<CASMoving *>(pGoalState)->IsBlocked());
}


/*====================
  CBAbility::MovementFrame
  ====================*/
void    CBAbility::MovementFrame()
{
    // Don't interrupt an ongoing activation
    if (m_pBrain->GetActionState(ASID_CASTING)->GetFlags() & ASR_ACTIVE)
        return;

    // Don't interrupt a previous channel of this ability
    if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
    {
        float fDeltaTime(MsToSec(Game.GetFrameLength()));
        CVec3f v3Angles(m_pSelf->GetAngles());
        float fGoalYaw(m_pSelf->GetAttentionAngles()[YAW]);
        float fYawDelta(M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW]);

        v3Angles[YAW] += fYawDelta;

        m_pSelf->SetAngles(v3Angles);
        return;
    }

    // Wait for current channel to finish
    if (GetFlags() & BSR_CAST && m_pAbility->IsChanneling(UNIT_ACTION_CAST))
    {
        float fDeltaTime(MsToSec(Game.GetFrameLength()));
        CVec3f v3Angles(m_pSelf->GetAngles());
        float fGoalYaw(m_pSelf->GetAttentionAngles()[YAW]);
        float fYawDelta(M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW]);

        v3Angles[YAW] += fYawDelta;

        m_pSelf->SetAngles(v3Angles);
        return;
    }

    if (m_pAbility->GetUsePathForRange())
    {
        if (m_hRangePath != INVALID_POOL_HANDLE)
        {
            Game.FreePath(m_hRangePath);
            m_hRangePath = INVALID_POOL_HANDLE;
        }

        uint uiNavFlags(NAVIGATION_ALL);
        uiNavFlags &= ~NAVIGATION_UNIT;
        uiNavFlags &= ~NAVIGATION_ANTI;

        vector<PoolHandle> *pBlockers(NULL);
        if (m_uiTargetIndex != INVALID_INDEX)
        {
            IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
            if (pTarget != NULL && m_v2TargetPosition == pTarget->GetBlockPosition())
                pBlockers = &pTarget->GetPathBlockers();
        }

        if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2TargetPosition) < SQR(0.001f))
        {
            m_fDistSq = 0.0f;
        }
        else if (DistanceSq(m_pSelf->GetPosition().xy(), m_v2TargetPosition) <= SQR(m_fRange))
        {
            m_hRangePath = Game.FindPath(m_pSelf->GetPosition().xy(), m_pSelf->GetBounds().GetDim(X) * 0.5f, uiNavFlags, m_v2TargetPosition, 0.0f, pBlockers);

            CPath *pPath(Game.AccessPath(m_hRangePath));
            if (pPath != NULL)
                m_fDistSq = SQR(pPath->GetLength(m_pSelf->GetPosition().xy()));
        }
    }

    if (m_fDistSq <= SQR(m_fRange) || m_pAbility->GetAllowOutOfRangeCast())
        return;

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Perform updates to path when moving and the target has strayed
    float fNewDistSq(DistanceSq(m_v2UpdatedGoal, m_v2ApproachPosition));
    if (((m_uiLastUpdate == INVALID_TIME || fNewDistSq > SQR(PATH_RECALC_DISTANCE)) && !static_cast<CASMoving *>(pGoalState)->GetBlocked()) ||
        static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock())
    {
        Update();
    }

    // Set move state params
    CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
    
    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);

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

    pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);
}


/*====================
  CBAbility::ActionFrame
  ====================*/
void    CBAbility::ActionFrame()
{
    CASCasting *pActivatingState(static_cast<CASCasting *>(m_pBrain->GetActionState(ASID_CASTING)));

    // Not ready yet or still activating ability
    if (pActivatingState->GetFlags() & ASR_ACTIVE)
        return;

    // Don't interrupt a previous channel of this ability
    if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
        return;

    // Cast and finished activating
    if (GetFlags() & BSR_CAST)
    {
        // Wait for current channel to finish
        if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
            return;

        ClearFlag(BSR_CAST);

        if (~pActivatingState->GetFlags() & ASR_INTERRUPTED)
            SetFlag(BSR_END);

        return;
    }

    // if out of range chase target
    CVec3f v3Position(m_pSelf->GetPosition());
    float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + m_pAbility->GetRange());
    
    CVec3f v3TargetPosition;
    IUnitEntity *pTarget(NULL);
    switch (m_pAbility->GetActionType())
    {
    case TOOL_ACTION_GLOBAL:
    case TOOL_ACTION_PASSIVE:
    case TOOL_ACTION_TOGGLE:
    case TOOL_ACTION_SELF_POSITION:
    case TOOL_ACTION_ATTACK_TOGGLE:
        v3TargetPosition = v3Position;
        break;

    case TOOL_ACTION_NO_TARGET:
    case TOOL_ACTION_TARGET_SELF:
    case TOOL_ACTION_FACING:
        v3TargetPosition = v3Position;
        pTarget = m_pSelf;
        break;

    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_ENTITY:
    case TOOL_ACTION_TARGET_DUAL:
    case TOOL_ACTION_TARGET_DUAL_POSITION:
    case TOOL_ACTION_TARGET_VECTOR:
    case TOOL_ACTION_TARGET_CURSOR:
        if (m_uiTargetIndex != INVALID_INDEX)
        {
            pTarget = Game.GetUnitEntity(m_uiTargetIndex);
            if (pTarget != NULL && m_bSight)
            {
                v3TargetPosition = pTarget->GetPosition();
                fRange += pTarget->GetBounds().GetDim(X) * DIAG;
            }
            else
            {
                v3TargetPosition = Game.GetTerrainPosition(m_v2ApproachPosition);
                fRange = 0.0f;

                if (m_bAtGoal)
                {
                    SetFlag(BSR_END);
                    return;
                }
            }
        }
        else
        {
            v3TargetPosition = Game.GetTerrainPosition(m_v2UpdatedGoal);
        }
        break;

    case TOOL_ACTION_ATTACK:
        if (~GetFlags() & BSR_SECONDARY)
        {
            pTarget = Game.GetUnitEntity(m_uiTargetIndex);
            if (pTarget != NULL && m_bSight)
            {
                v3TargetPosition = pTarget->GetPosition();
                fRange += pTarget->GetBounds().GetDim(X) * DIAG;
            }
            else
            {
                v3TargetPosition = Game.GetTerrainPosition(m_v2ApproachPosition);
                fRange = 0.0f;

                if (m_bAtGoal)
                {
                    SetFlag(BSR_END);
                    return;
                }
            }
        }
        else
        {
            v3TargetPosition = v3Position;
        }
        break;
    }
    
    float fDistSq(FAR_AWAY);
    if (m_pAbility->GetUsePathForRange())
    {
        CPath *pPath(Game.AccessPath(m_hRangePath));
        if (pPath != NULL)
            fDistSq = SQR(pPath->GetLength(v3Position.xy()));
    }
    else
    {
        fDistSq = DistanceSq(v3Position.xy(), v3TargetPosition.xy());
    }

    if (m_fDistSq > SQR(m_fRange) && fDistSq > SQR(fRange))
    {
        if (!m_pAbility->GetAllowOutOfRangeCast())
            return;

        CVec3f v3Dir(v3TargetPosition - v3Position);
        v3Dir.z = 0;
        v3Dir.Normalize();
        v3TargetPosition = v3Position + (v3Dir * m_fRange);
    }
    else if (m_pAbility->GetForceRange() > 0.0f)
    {
        CVec3f v3Dir;
        
        if (m_pSelf != pTarget && v3TargetPosition != v3Position)
        {
            v3Dir = v3TargetPosition - v3Position;
            v3Dir.z = 0;
            v3Dir.Normalize();
        }
        else
        {
            CVec2f v2Forward(M_GetForwardVec2FromYaw(m_pSelf->GetAngles()[YAW]));

            v3Dir.x = v2Forward.x;
            v3Dir.y = v2Forward.y;
            v3Dir.z = 0.0f;
        }
        
        v3TargetPosition = v3Position + (v3Dir * (m_pSelf->GetBounds().GetDim(X) * DIAG + m_pAbility->GetForceRange()));
    }

    if (m_pAbility->GetMaxDelta() > 0.0f && m_v2Delta.LengthSq() > SQR(m_pAbility->GetMaxDelta()))
        m_v2Delta.SetLength(m_pAbility->GetMaxDelta());
    if (m_pAbility->GetForceDelta() > 0.0f)
    {
        if (m_v2Delta.LengthSq() == 0.0f)
        {
            CVec2f v2Dir(v3TargetPosition.xy() - v3Position.xy());
            v2Dir.Normalize();
            m_v2Delta = v2Dir * m_pAbility->GetForceDelta();
        }
        else
        {
            m_v2Delta.SetLength(m_pAbility->GetForceDelta());
        }
    }

    if (fDistSq < SQR(m_pAbility->GetMinRange()))
    {
        SetFlag(BSR_END);
        return;
    }

    if ((m_pAbility->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_VECTOR ||
        m_pAbility->GetActionType() == TOOL_ACTION_TARGET_CURSOR) &&
        !Game.IsInBounds(v3TargetPosition.x, v3TargetPosition.y) &&
        !m_pAbility->GetAllowOutOfBoundsCast())
    {
        SetFlag(BSR_END);
        return;
    }

    // Validate target at start of cast if this is a targeted ability
    if (IsTargeted())
    {
        if (pTarget == NULL ||
            !m_bSight ||
            !m_pAbility->IsValidTarget(pTarget))
        {
            SetFlag(BSR_END);
            return;
        }
    }

    // Set begin params
    pActivatingState->SetBeginToolUID(m_pAbility->GetUniqueID());
    pActivatingState->SetBeginTargetIndex(m_uiTargetIndex);
    pActivatingState->SetBeginTargetPosition(v3TargetPosition);
    pActivatingState->SetBeginTargetDelta(CVec3f(m_v2Delta, 0.0f));
    pActivatingState->SetBeginSecondary((GetFlags() & BSR_SECONDARY) != 0);
    pActivatingState->SetBeginIssuedClientNumber(m_iIssuedClientNumber);
        
    // If in range and we have a valid target, issue activating state
    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_CASTING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_CASTING));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Set state params
    pActivatingState->SetToolUID(m_pAbility->GetUniqueID());
    pActivatingState->SetTargetIndex(m_uiTargetIndex);
    pActivatingState->SetTargetPosition(v3TargetPosition);
    pActivatingState->SetTargetDelta(CVec3f(m_v2Delta, 0.0f));
    pActivatingState->SetSecondary((GetFlags() & BSR_SECONDARY) != 0);
    pActivatingState->SetIssuedClientNumber(m_iIssuedClientNumber);

    SetFlag(BSR_CAST);
}


/*====================
  CBAbility::CleanupFrame
  ====================*/
void    CBAbility::CleanupFrame()
{
    CASCasting *pActivatingState(static_cast<CASCasting *>(m_pBrain->GetActionState(ASID_CASTING)));

    // Don't interrupt a previous channel of this ability
    if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
        return;

    // Cast and finished activating
    if (GetFlags() & BSR_CAST && 
        ~pActivatingState->GetFlags() & ASR_ACTIVE)
    {
        // Wait for current channel to finish
        if (m_pAbility->IsChanneling(UNIT_ACTION_CAST))
            return;

        ClearFlag(BSR_CAST);

        if (~pActivatingState->GetFlags() & ASR_INTERRUPTED)
            SetFlag(BSR_END);
    }
}


/*====================
  CBAbility::EndBehavior
  ====================*/
void    CBAbility::EndBehavior()
{
    if (m_hRangePath != INVALID_POOL_HANDLE)
    {
        Game.FreePath(m_hRangePath);
        m_hRangePath = INVALID_POOL_HANDLE;
    }

    IBehavior::EndBehavior();
}

/*====================
  CBAbility::IsChanneling
  ====================*/
bool    CBAbility::IsChanneling() const
{
    if (m_pAbility != NULL && m_pAbility->IsChanneling(UNIT_ACTION_CAST))
        return true;

    return false;
}