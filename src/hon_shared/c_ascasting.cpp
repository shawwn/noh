// (C)2008 S2 Games
// c_ascasting.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_asCasting.h"

#include "c_brain.h"
#include "i_unitentity.h"
#include "i_entitytool.h"
//=============================================================================

/*====================
  CASCasting::CASCasting
  ====================*/
CASCasting::CASCasting(CBrain &cParent) :
IActionState(cParent),
m_uiBeginToolUID(INVALID_INDEX),
m_uiBeginTargetIndex(INVALID_INDEX),
m_v3BeginTargetPosition(V2_ZERO),
m_bBeginSecondary(false),
m_iBeginIssuedClientNumber(-1),

m_uiToolUID(INVALID_INDEX),
m_uiTargetIndex(INVALID_INDEX),
m_v3TargetPosition(V2_ZERO),
m_bSecondary(false),
m_iIssuedClientNumber(-1),

m_uiInitTime(0),
m_uiTargetOrderDisjointSequence(uint(-1))
{
}


/*====================
  CASCasting::CopyFrom
  ====================*/
void	CASCasting::CopyFrom(const CASCasting *pActionState)
{
	m_uiBeginToolUID = pActionState->m_uiBeginToolUID;
	m_uiBeginTargetIndex = pActionState->m_uiBeginTargetIndex;
	m_v3BeginTargetPosition = pActionState->m_v3BeginTargetPosition;
	m_v3BeginTargetDelta = pActionState->m_v3BeginTargetDelta;
	m_bBeginSecondary = pActionState->m_bBeginSecondary;
	m_iBeginIssuedClientNumber = pActionState->m_iBeginIssuedClientNumber;

	m_uiToolUID = pActionState->m_uiToolUID;
	m_uiTargetIndex = pActionState->m_uiTargetIndex;
	m_v3TargetPosition = pActionState->m_v3TargetPosition;
	m_v3TargetDelta = pActionState->m_v3TargetDelta;
	m_bSecondary = pActionState->m_bSecondary;
	m_iIssuedClientNumber = pActionState->m_iIssuedClientNumber;

	m_uiInitTime = pActionState->m_uiInitTime;
	m_uiTargetOrderDisjointSequence = pActionState->m_uiTargetOrderDisjointSequence;

	CopyActionStateFrom(pActionState);
}


/*====================
  CASCasting::BeginState
  ====================*/
bool	CASCasting::BeginState()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	if (pUnit == NULL)
		return false;

	IEntityTool *pTool(GetBeginTool());

	if (pTool->IsDisabled())
		return false;
	if (pUnit->IsStunned() && !pTool->GetNoStun())
		return false;

	if (m_bBeginSecondary)
	{
		if (pTool->GetActionType() != TOOL_ACTION_ATTACK && !pTool->GetAllowAutoCast())
			return false;
	}
	else
	{
		if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
		{
			if (!pUnit->IsAttackReady())
				return false;
			if (!pUnit->GetCanAttack())
				return false;
		}
	}

	Reset();
	SetFlag(ASR_ACTIVE);
	
	if (!m_bBeginSecondary)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiBeginTargetIndex));
		pTool->ExecuteActionScript(ACTION_SCRIPT_BEGIN, pTarget, m_v3BeginTargetPosition);

		if (!pTool->IsTargetValid(pTarget, m_v3BeginTargetPosition))
		{
			ClearFlag(ASR_ACTIVE);
			return false;
		}

		if (!pTool->GetNonInterrupting())
			pUnit->Interrupt(UNIT_ACTION_CAST);

		if (pTarget != NULL)
			m_uiTargetOrderDisjointSequence = pTarget->GetOrderDisjointSequence();
	}

	return true;
}


/*====================
  CASCasting::ContinueStateMovement
  ====================*/
bool	CASCasting::ContinueStateMovement()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	if (pUnit == NULL)
		return false;

	IEntityTool *pTool(GetTool());
	if (pTool == NULL)
		return false;

	pTool->SetFlag(ENTITY_TOOL_FLAG_IN_USE);

	if (!(GetFlags() & (ASR_COMPLETED | ASR_CHANNELING)) &&
		!(pTool->CanOrder() || 
			(pTool->GetActionType() == TOOL_ACTION_ATTACK && m_bSecondary) ||
			pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
			(pTool->GetAllowAutoCast() && m_bSecondary)))
		return false;

	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));

	if (!(pTool->GetAllowAutoCast() && m_bSecondary))
	{
		if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY ||
			(pTool->GetActionType() == TOOL_ACTION_ATTACK && !m_bSecondary) ||
			((pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL || pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) && m_uiTargetIndex != INVALID_INDEX))
		{
			if (pTarget == NULL)
				return false;

			if (~GetFlags() & ASR_COMPLETED)
			{
				if (!pTool->IsValidTarget(pTarget))
					return false;
				if (!pUnit->CanSee(pTarget) && ~GetFlags() & ASR_CHANNELING)
					return false;
			}
		}
	}

	if (~GetFlags() & ASR_COMPLETED)
	{
		if (pTool->IsDisabled())
			return false;

		if (pUnit->IsStunned() && !pTool->GetNoStun())
		{
			SetFlag(ASR_INTERRUPTED);
			return false;
		}

		if (pTarget != NULL && m_uiTargetOrderDisjointSequence != pTarget->GetOrderDisjointSequence())
			return false;
	}

	CVec3f v3VecToTarget(V_ZERO);
	if (pTarget != NULL &&
		pTool->GetActionType() != TOOL_ACTION_TARGET_POSITION &&
		pTool->GetActionType() != TOOL_ACTION_TARGET_VECTOR &&
		pTool->GetActionType() != TOOL_ACTION_TARGET_CURSOR)
		v3VecToTarget = pTarget->GetPosition() - pUnit->GetPosition();
	else
		v3VecToTarget = m_v3TargetPosition - pUnit->GetPosition();

	CVec3f v3Angles(pUnit->GetAngles());

	float fAngleToTarget((M_GetYawFromForwardVec2(v3VecToTarget.xy())));
	float fYaw(v3Angles[YAW]);

	if (pTool->GetActionType() == TOOL_ACTION_GLOBAL ||
		pTool->GetActionType() == TOOL_ACTION_NO_TARGET ||
		pTool->GetActionType() == TOOL_ACTION_TOGGLE ||
		pTool->GetActionType() == TOOL_ACTION_TARGET_SELF ||
		pTool->GetActionType() == TOOL_ACTION_FACING ||
		pTool->GetActionType() == TOOL_ACTION_SELF_POSITION ||
		(pTool->GetActionType() == TOOL_ACTION_ATTACK && m_bSecondary) ||
		pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
		(pTool->GetAllowAutoCast() && m_bSecondary) ||
		m_uiTargetIndex == pUnit->GetIndex())
		fAngleToTarget = fYaw;
	else
		fAngleToTarget = M_GetYawFromForwardVec2(v3VecToTarget.xy());
	
	float fDeltaTime(MsToSec(Game.GetFrameLength()));

	bool bRet(true);
	
	if (m_uiInitTime == 0)
	{
		if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true) && !pTool->GetNoTurnToTarget())
		{
			// Turning toward target
			v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, fAngleToTarget);
			pUnit->SetAngles(v3Angles);

			pUnit->SetAttentionYaw(fAngleToTarget);
		}
	}
	
	if (m_uiInitTime != 0)
	{
		uint uiElapsedTime(Game.GetGameTime() - m_uiInitTime);

		uint uiActionTime(0);
		uint uiCastTime(0);

		if (!m_bSecondary)
		{
			if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
			{
				uiActionTime = pUnit->GetAdjustedAttackActionTime();
				uiCastTime = pUnit->GetAdjustedAttackDuration();
			}
			else
			{
				uiActionTime = pTool->GetAdjustedActionTime();
				uiCastTime = pTool->GetAdjustedCastTime();
			}
		}

		if (uiElapsedTime < uiActionTime)
		{
			if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true) && !pTool->GetNoTurnToTarget())
			{
				// Keep turning until impact time
				v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, fAngleToTarget);
				pUnit->SetAngles(v3Angles);

				pUnit->SetAttentionYaw(fAngleToTarget);
			}
			
			// Commited
			//SetFlag(ASR_COMMITTED);
		}
		else
		{
			if ((~GetFlags() & ASR_COMPLETED || pTool->GetIsChanneling()) && !m_bSecondary)
			{
				if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true) && !pTool->GetNoTurnToTarget())
				{
					// Still turn on the frame we impact or if we're channeling
					v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, fAngleToTarget);
					pUnit->SetAngles(v3Angles);

					pUnit->SetAttentionYaw(fAngleToTarget);
				}
			}
			else
			{
				if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true) && !pTool->GetNoTurnToTarget())
				{
					// Finish turn toward target
					v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, pUnit->GetAttentionAngles()[YAW]);
					pUnit->SetAngles(v3Angles);
				}
			}
		}
		
		if (uiElapsedTime >= uiCastTime && GetFlags() & ASR_COMPLETED)
		{
			SetFlag(ASR_ALLDONE);
			bRet = false;
		}
	}

	return bRet;
}


/*====================
  CASCasting::ContinueStateAction
  ====================*/
bool	CASCasting::ContinueStateAction()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	IEntityTool *pTool(GetTool());

	if (pUnit == NULL)
		return false;
	if (pTool == NULL)
		return false;

	pTool->SetFlag(ENTITY_TOOL_FLAG_IN_USE);

	// Abort if we haven't started casting yet and can't order the tool to start
	if (!(GetFlags() & (ASR_COMPLETED | ASR_CHANNELING)) &&
		!(pTool->CanOrder() ||
			(pTool->GetActionType() == TOOL_ACTION_ATTACK && m_bSecondary) || 
			pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
			(pTool->GetAllowAutoCast() && m_bSecondary)))
		return false;
	
	if (!(pTool->GetAllowAutoCast() && m_bSecondary))
	{
		if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY ||
			(pTool->GetActionType() == TOOL_ACTION_ATTACK && !m_bSecondary) ||
			((pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL || pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) && m_uiTargetIndex != INVALID_INDEX))
		{
			if (pTarget == NULL)
				return false;

			if (~GetFlags() & ASR_COMPLETED)
			{
				if (!pTool->IsValidTarget(pTarget))
					return false;
				if (!pUnit->CanSee(pTarget) && ~GetFlags() & ASR_CHANNELING)
					return false;
			}
		}
	}

	// Interrupt actions midcast when silenced, stunned, or disjointed
	if (!(GetFlags() & ASR_COMPLETED) &&
		!((pTool->GetActionType() == TOOL_ACTION_ATTACK && m_bSecondary) || 
		pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
		(pTool->GetAllowAutoCast() && m_bSecondary)))
	{
		if (pTool->IsDisabled())
			return false;

		if (pUnit->IsStunned() && !pTool->GetNoStun())
		{
			SetFlag(ASR_INTERRUPTED);
			return false;
		}

		if (pTarget != NULL && m_uiTargetOrderDisjointSequence != pTarget->GetOrderDisjointSequence())
			return false;
	}

	uint uiActionTime(0);
	uint uiCastTime(0);

	if (!m_bSecondary)
	{
		if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
		{
			uiActionTime = pUnit->GetAdjustedAttackActionTime();
			uiCastTime = pUnit->GetAdjustedAttackDuration();
		}
		else
		{
			uiActionTime = pTool->GetAdjustedActionTime();
			uiCastTime = pTool->GetAdjustedCastTime();
		}
	}

	if (m_uiInitTime == 0)
	{
		CVec3f v3VecToTarget(V_ZERO);
		if (pTarget != NULL &&
			pTool->GetActionType() != TOOL_ACTION_TARGET_POSITION &&
			pTool->GetActionType() != TOOL_ACTION_TARGET_VECTOR &&
			pTool->GetActionType() != TOOL_ACTION_TARGET_CURSOR)
			v3VecToTarget = pTarget->GetPosition() - pUnit->GetPosition();
		else
			v3VecToTarget = m_v3TargetPosition - pUnit->GetPosition();

		CVec3f v3Angles(pUnit->GetAngles());

		float fAngleToTarget((M_GetYawFromForwardVec2(v3VecToTarget.xy())));
		float fYaw(v3Angles[YAW]);

		if (pTool->GetActionType() == TOOL_ACTION_GLOBAL ||
			pTool->GetActionType() == TOOL_ACTION_NO_TARGET ||
			pTool->GetActionType() == TOOL_ACTION_TOGGLE ||
			pTool->GetActionType() == TOOL_ACTION_TARGET_SELF ||
			pTool->GetActionType() == TOOL_ACTION_FACING ||
			pTool->GetActionType() == TOOL_ACTION_SELF_POSITION ||
			(pTool->GetActionType() == TOOL_ACTION_ATTACK && m_bSecondary) ||
			pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE ||
			(pTool->GetAllowAutoCast() && m_bSecondary) ||
			m_uiTargetIndex == pUnit->GetIndex())
			fAngleToTarget = fYaw;
		else
			fAngleToTarget = M_GetYawFromForwardVec2(v3VecToTarget.xy());

		float fActionAngle(g_unitActionOnTurn ? pUnit->GetTurnRate() * MsToSec(uiActionTime) : g_unitActionAngle);

		if (!pUnit->GetCanRotate() || pUnit->IsImmobilized(false, true) || M_DiffAngle(fAngleToTarget, fYaw) <= fActionAngle || pTool->GetNoTurnToTarget())
		{
			m_uiInitTime = Game.GetGameTime();

			if (!pTool->GetNoTurnToTarget())
				pUnit->SetAttentionYaw(fAngleToTarget);

			if (!m_bSecondary)
			{
				if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
				{
					if (!pTool->GetAnim().empty())
						pUnit->StartAnimation(pTool->GetAnim(), pTool->GetAnimChannel(), pUnit->GetAttackSpeed());
					else
						pUnit->StartRandomAnimation(pUnit->GetAttackAnim(), pUnit->GetAttackNumAnims(), 0, pUnit->GetAttackSpeed());
				}
				else
				{
					if (!pTool->GetAnim().empty() && !(pUnit->IsChanneling(UNIT_ACTION_CAST) && pTool->GetNonInterrupting()))
						pUnit->StartAnimation(pTool->GetAnim(), pTool->GetAnimChannel(), pUnit->GetCastSpeed());
				}

				if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
					pUnit->StartAttack(pTarget, true, false);

				pTool->ExecuteActionScript(ACTION_SCRIPT_START, pTarget, m_v3TargetPosition);

				pUnit->Action(ACTION_SCRIPT_ACTIVATE_START, pTarget, pTool);

				if (pTool->IsAbility())
					pUnit->Action(ACTION_SCRIPT_ABILITY_START, pTarget, pTool);

				pTool->PlayCastEffect();
			}
		}
		else
		{
			// Still turning towards target
			return true;
		}
	}

	uint uiElapsedTime(Game.GetGameTime() - m_uiInitTime);
	bool bRet(true);
	
	if (uiElapsedTime < uiActionTime)
	{
		// Commited
		//SetFlag(ASR_COMMITTED);
	}
	else
	{
		if (~GetFlags() & ASR_COMPLETED)
		{
			// Range buffer check
			if (pTarget != NULL)
			{
				if (pTool->GetActionType() == TOOL_ACTION_ATTACK && !m_bSecondary)
				{
					float fRange(pUnit->GetBounds().GetDim(X) * DIAG + pUnit->GetAttackRange() + pUnit->GetAttackRangeBuffer() + pTarget->GetBounds().GetDim(X) * DIAG);

					if (DistanceSq(pUnit->GetPosition().xy(), pTarget->GetPosition().xy()) > SQR(fRange))
					{
						Game.SendPopup(POPUP_TOOFAR, pUnit);
						return false;
					}
				}
				else
				{
					float fRange(pUnit->GetBounds().GetDim(X) * DIAG + pTool->GetRange() + pTool->GetRangeBuffer() + pTarget->GetBounds().GetDim(X) * DIAG);

					if ((pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY ||
						((pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL || pTool->GetActionType() == TOOL_ACTION_TARGET_DUAL_POSITION) && m_uiTargetIndex != INVALID_INDEX)) &&
						DistanceSq(pUnit->GetPosition().xy(), pTarget->GetPosition().xy()) > SQR(fRange))
					{
						Game.SendPopup(POPUP_TOOFAR, pUnit);
						return false;
					}
				}
			}

			if (m_bSecondary)
			{
				if (pTool->GetActionType() == TOOL_ACTION_ATTACK || pTool->GetAllowAutoCast())
					pTool->ToggleAutoCast();
			}
			else
			{
				if (pTool->GetActionType() == TOOL_ACTION_ATTACK)
				{
					pTool->Activate(pTarget, m_v3TargetPosition, m_v3TargetDelta, false, m_iIssuedClientNumber);
				}
				else if (pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
				{
					pTool->ToggleAutoCast();
				}
				else
				{
					if (!pTool->Activate(pTarget, m_v3TargetPosition, m_v3TargetDelta, m_bSecondary, m_iIssuedClientNumber))
						pTool->ExecuteActionScript(ACTION_SCRIPT_CANCEL, pTarget, m_v3TargetPosition);
				}
			}

			SetFlag(ASR_COMPLETED);
		}
	}

	if (uiElapsedTime >= uiCastTime && GetFlags() & ASR_COMPLETED)
	{
		SetFlag(ASR_ALLDONE);
		bRet = false;
	}

	return bRet;
}


/*====================
  CASCasting::EndState
  ====================*/
bool	CASCasting::EndState(uint uiPriority)
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	IEntityTool *pTool(GetTool());

	bool bRet(false);

	if (GetFlags() & ASR_ALLDONE)
	{
		Reset();
		bRet = true;
	}
	else if (GetFlags() & ASR_COMPLETED && uiPriority > 0)
	{
		ClearFlag(ASR_ACTIVE);
		bRet = true;
	}
	else if (~GetFlags() & ASR_COMPLETED && uiPriority < 2)
	{
		bRet = false;
	}
	else if (~GetFlags() & ASR_COMMITTED && uiPriority > 1)
	{
		bool bInterrupted((GetFlags() & ASR_INTERRUPTED) != 0);

		Reset();
		bRet = true;

		if (pTool != NULL && !pTool->GetAnim().empty())
			pUnit->StopAnimation(pTool->GetAnim(), pTool->GetAnimChannel());

		if (bInterrupted)
			SetFlag(ASR_INTERRUPTED);
	}
	else if (GetFlags() & ASR_COMMITTED && uiPriority > 2)
	{
		bool bInterrupted((GetFlags() & ASR_INTERRUPTED) != 0);

		Reset();
		bRet = true;

		if (pTool != NULL && !pTool->GetAnim().empty())
			pUnit->StopAnimation(pTool->GetAnim(), pTool->GetAnimChannel());

		if (bInterrupted)
			SetFlag(ASR_INTERRUPTED);
	}

	if (bRet && pTool != NULL)
		pTool->ClearFlag(ENTITY_TOOL_FLAG_IN_USE);

	return bRet;
}