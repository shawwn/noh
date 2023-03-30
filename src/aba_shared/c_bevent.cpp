// (C)2009 S2 Games
// c_bevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bevent.h"

#include "c_asMoving.h"
#include "c_asAttacking.h"
#include "c_brain.h"
#include "i_unitentity.h"
#include "i_orderentity.h"
//=============================================================================

/*====================
  CBEvent::CopyFrom
  ====================*/
void	CBEvent::CopyFrom(const IBehavior* pBehavior)
{
	assert( GetType() == pBehavior->GetType() );
	if (GetType() != pBehavior->GetType())
		return;

	const CBEvent *pCBBehavior(static_cast<const CBEvent*>(pBehavior));

	m_fDistSq = pCBBehavior->m_fDistSq;		
	m_v2ApproachPosition = pCBBehavior->m_v2ApproachPosition;
	m_fRange = pCBBehavior->m_fRange;

	IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBEvent::Clone
  ====================*/
IBehavior*	CBEvent::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
	IBehavior* pBehavior( K2_NEW(g_heapAI,    CBEvent)() );
	pBehavior->SetBrain(pNewBrain);
	pBehavior->SetSelf(pNewSelf);
	pBehavior->CopyFrom(this);
	return pBehavior;
}

/*====================
  CBEvent::Validate
  ====================*/
bool	CBEvent::Validate()
{
	if (m_pBrain == NULL ||
		m_pSelf == NULL ||
		GetFlags() & BSR_END ||
		m_pSelf->IsIllusion() ||
		Game.GetUnitEntity(m_uiTargetIndex) == NULL)
	{
		SetFlag(BSR_END);
		return false;
	}

	return true;
}


/*====================
  CBEvent::Update
  ====================*/
void	CBEvent::Update()
{
	if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
		return;

	// Check for path changes
	m_uiLastUpdate = Game.GetGameTime();
	m_v2UpdatedGoal = m_v2ApproachPosition;
	FindPathToUpdatedGoal();
}


/*====================
  CBEvent::BeginBehavior
  ====================*/
void	CBEvent::BeginBehavior()
{
	if (m_pSelf == NULL)
	{
		Console << _T("CBEvent: Behavior started without valid information") << newl;
		return;
	}

	if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
		return;

	m_pBrain->EndActionStates(1);

	m_fDistSq = FAR_AWAY;
	m_v2ApproachPosition = V2_ZERO;
	m_fRange = 0.0f;

	if (m_unOrderEnt != INVALID_ENT_TYPE)
	{
		IOrderEntity *pOrder(Game.AllocateDynamicEntity<IOrderEntity>(m_unOrderEnt));
		if (pOrder != NULL)
		{
			m_uiOrderEntUID = pOrder->GetUniqueID();

			pOrder->SetLevel(GetLevel());
			pOrder->SetOwnerIndex(m_pSelf->GetIndex());
		}
	}

	IOrderEntity *pOrder(GetOrder());
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	if (pOrder != NULL)
		pOrder->ExecuteActionScript(ACTION_SCRIPT_BEGIN, pTarget, pTarget != NULL ? pTarget->GetPosition() : V3_ZERO);

	m_uiLastUpdate = INVALID_TIME;
	ClearFlag(BSR_NEW);
}


/*====================
  CBEvent::ThinkFrame
  ====================*/
void	CBEvent::ThinkFrame()
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

	// if out of range we'll need to chase target
	CVec3f v3Position(m_pSelf->GetPosition());
	CVec3f v3TargetPosition(pTarget->GetPosition());
	
	m_fDistSq = DistanceSq(v3Position.xy(), v3TargetPosition.xy());
	m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
	m_fRange = m_pSelf->GetBounds().GetDim(X) * DIAG + g_touchRange + pTarget->GetBounds().GetDim(X) * DIAG;

	if (m_fDistSq <= SQR(m_fRange))
	{
		m_pBrain->SetMoving(false);
		return;
	}

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
  CBEvent::MovementFrame
  ====================*/
void	CBEvent::MovementFrame()
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

	if (m_fDistSq <= SQR(m_fRange))
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
		Update();

	// Set move state params
	CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
	
	CVec3f v3Angles(m_pSelf->GetAngles());

	CVec2f v2Movement(V2_ZERO);
	float fYawDelta(0.0f);
	float fGoalYaw(v3Angles[YAW]);
	bool bAtGoal(false);

	GetMovement(v2Movement, fYawDelta, bAtGoal, fGoalYaw);

	m_pSelf->SetAttentionYaw(fGoalYaw);

	if (bAtGoal)
	{
		// if we're still out of range head straight for the target
		CVec2f v2Position(m_pSelf->GetPosition().xy());
		CVec2f v2TargetPosition(pTarget->GetPosition().xy());
		float fDistSq(DistanceSq(v2Position, v2TargetPosition));
		float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + g_transferRange + pTarget->GetBounds().GetDim(X) * DIAG);

		if (fDistSq > SQR(fRange))
		{
			float fDeltaTime(MsToSec(Game.GetFrameLength()));
			CVec2f v2DirectionOfMovement(Normalize(v2TargetPosition - v2Position));

			// Turn the unit to face the direction of movement as we progress
			float fGoalYaw(M_GetYawFromForwardVec2(v2DirectionOfMovement));

			fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];

			v2Movement = v2DirectionOfMovement * m_pSelf->GetMoveSpeed() * fDeltaTime;
		}
	}

	pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);
}


/*====================
  CBEvent::ActionFrame
  ====================*/
void	CBEvent::ActionFrame()
{
	if (GetShared() && m_pSelf->IsChanneling(UNIT_ACTION_MOVE))
		return;

	// No touching items while attacking or activating
	if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
		if (!m_pBrain->GetActionState(ASID_ATTACKING)->EndState(0))
			return;

	if (m_pBrain->GetActionState(ASID_CASTING)->GetFlags() & ASR_ACTIVE)
		if (!m_pBrain->GetActionState(ASID_CASTING)->EndState(0))
			return;

	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	if (pTarget == NULL)
	{
		SetFlag(BSR_END);
		return;
	}

	// Check for a target that has become invalid
	if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
		(!m_pSelf->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
	{
		SetFlag(BSR_END);
		return;
	}

	IOrderEntity *pOrder(GetOrder());
	if (pOrder != NULL)
	{
		pOrder->ExecuteActionScript(ACTION_SCRIPT_FRAME, pTarget, pTarget != NULL ? pTarget->GetPosition() : V3_ZERO);
		if (pOrder->GetComplete())
		{
			SetFlag(BSR_END | BSR_SUCCESS);
			return;
		}
		else if (pOrder->GetCancel())
		{
			SetFlag(BSR_END);
			return;
		}
	}

	// if out of range chase target
	CVec2f v2Position(m_pSelf->GetPosition().xy());
	CVec2f v2TargetPosition(pTarget->GetPosition().xy());
	float fDistSq(DistanceSq(v2Position, v2TargetPosition));
	float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + g_transferRange + pTarget->GetBounds().GetDim(X) * DIAG);

	// check if were in range during either setup frame or action frame
	if (m_fDistSq > SQR(m_fRange) && fDistSq > SQR(fRange))
		return;

	if (pOrder != NULL)
		pOrder->ExecuteActionScript(ACTION_SCRIPT_COMPLETE, pTarget, pTarget != NULL ? pTarget->GetPosition() : V3_ZERO);

	SetFlag(BSR_END | BSR_SUCCESS);
}


/*====================
  CBEvent::CleanupFrame
  ====================*/
void	CBEvent::CleanupFrame()
{
}


/*====================
  CBEvent::EndBehavior
  ====================*/
void	CBEvent::EndBehavior()
{
	IBehavior::EndBehavior();

	if (~GetFlags() & BSR_SUCCESS)
	{
		IOrderEntity *pOrder(GetOrder());
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
		if (pOrder != NULL)
			pOrder->ExecuteActionScript(ACTION_SCRIPT_CANCEL, pTarget, pTarget != NULL ? pTarget->GetPosition() : V3_ZERO);
	}

	if (m_uiOrderEntUID != INVALID_INDEX)
	{
		Game.DeleteEntity(Game.GetGameIndexFromUniqueID(m_uiOrderEntUID));
		m_uiOrderEntUID = INVALID_INDEX;
	}
}