// (C)2007 S2 Games
// c_asAttacking.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_asAttacking.h"

#include "c_brain.h"
#include "i_unitentity.h"
#include "i_heroentity.h"
//=============================================================================

/*====================
  CASAttacking::CASAttacking
  ====================*/
CASAttacking::CASAttacking(CBrain &cParent) :
IActionState(cParent),
m_uiBeginTargetIndex(INVALID_INDEX),
m_uiTargetIndex(INVALID_INDEX)
{
	Reset();
}

/*====================
  CASAttacking::CopyFrom
  ====================*/
void	CASAttacking::CopyFrom(const CASAttacking *pActionState)
{
	m_uiBeginTargetIndex = pActionState->m_uiBeginTargetIndex;

	m_uiInitTime = pActionState->m_uiInitTime;
	m_uiTargetIndex = pActionState->m_uiTargetIndex;

	CopyActionStateFrom(pActionState);
}

/*====================
  CASAttacking::BeginState
  ====================*/
bool	CASAttacking::BeginState()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiBeginTargetIndex));

	if (pUnit == NULL || pTarget == NULL)
		return false;
	if (pUnit->IsDisarmed())
		return false;
	if (pUnit->IsStunned())
		return false;
	if (!pUnit->IsAttackReady())
		return false;
	if (!pUnit->GetCanAttack())
		return false;

	if (!Game.IsValidTarget(pUnit->GetAttackTargetScheme(), pUnit->GetAttackEffectType(), pUnit, pTarget, false))
		return false;

	Reset();
	SetFlag(ASR_ACTIVE);

	pUnit->Interrupt(UNIT_ACTION_ATTACK);
	
	return true;
}


/*====================
  CASAttacking::ContinueStateMovement

  Turn towards target and play attack animation
  ====================*/
bool	CASAttacking::ContinueStateMovement()
{
	bool bRet(true);
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	
	if (pUnit == NULL)
		return false;
	if (pUnit->IsDisarmed())
		return false;
	if (pUnit->IsStunned())
		return false;

	if (~GetFlags() & ASR_COMPLETED)
	{
		if (pTarget == NULL)
			return false;
		if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
			return false;
		if (!Game.IsValidTarget(pUnit->GetAttackTargetScheme(), pUnit->GetAttackEffectType(), pUnit, pTarget, false))
			return false;
#if 0 // Whether or not to interrupt on-going attacks if sight is lost
		if (!pUnit->CanSee(pTarget))
			return false;
#endif
	}

	float fDeltaTime(MsToSec(Game.GetFrameLength()));
	CVec3f v3Angles(pUnit->GetAngles());
	float fYaw(v3Angles.z);
	float fAngleToTarget;
	
	if (pTarget != NULL)
	{
		CVec3f v3VecToTarget(pTarget->GetPosition() - pUnit->GetPosition());
		fAngleToTarget = M_GetYawFromForwardVec2(v3VecToTarget.xy());
	}
	else
	{
		fAngleToTarget = fYaw;
	}

	if (m_uiInitTime == 0)
	{
		if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true))
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

		if (uiElapsedTime < pUnit->GetAdjustedAttackActionTime())
		{
			// Waiting for action
	
			if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true))
			{
				v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, fAngleToTarget);
				pUnit->SetAngles(v3Angles);

				pUnit->SetAttentionYaw(fAngleToTarget);
			}
		}
		else 
		{
			// Waiting for action frame to process
			
			if (~GetFlags() & ASR_COMPLETED)
			{
				if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true))
				{
					// Still turn on the frame we impact
					v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, fAngleToTarget);
					pUnit->SetAngles(v3Angles);

					pUnit->SetAttentionYaw(fAngleToTarget);
				}
			}
			else
			{
				if (pUnit->GetCanRotate() && !pUnit->IsImmobilized(false, true))
				{
					// Finish turn toward target
					v3Angles.z = M_ChangeAngle(pUnit->GetTurnRate() * fDeltaTime, fYaw, pUnit->GetAttentionAngles()[YAW]);
					pUnit->SetAngles(v3Angles);
				}
			}
		}

		if (GetFlags() & ASR_COMPLETED)
		{
			if (uiElapsedTime >= pUnit->GetAdjustedAttackDuration())
			{
				SetFlag(ASR_ALLDONE);
				bRet = false;
			}
		}
	}

	return bRet;
}


/*====================
  CASAttacking::ContinueStateAction
  ====================*/
bool	CASAttacking::ContinueStateAction()
{
	bool bRet(true);
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	
	if (pUnit == NULL)
		return false;
	if (pUnit->IsDisarmed())
		return false;
	if (pUnit->IsStunned())
		return false;

	if (~GetFlags() & ASR_COMPLETED)
	{
		if (pTarget == NULL)
			return false;
		if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE)
			return false;
		if (!Game.IsValidTarget(pUnit->GetAttackTargetScheme(), pUnit->GetAttackEffectType(), pUnit, pTarget, false))
			return false;
#if 0 // Whether or not to interrupt on-going attacks if sight is lost
		if (!pUnit->CanSee(pTarget))
			return false;
#endif
	}

	CVec3f v3Angles(pUnit->GetAngles());
	float fYaw(v3Angles.z);
	float fAngleToTarget;
	
	if (pTarget != NULL)
	{
		CVec3f v3VecToTarget(pTarget->GetPosition() - pUnit->GetPosition());
		fAngleToTarget = M_GetYawFromForwardVec2(v3VecToTarget.xy());
	}
	else
	{
		fAngleToTarget = fYaw;
	}

	if (m_uiInitTime == 0)
	{
		float fActionAngle(g_unitActionOnTurn ? pUnit->GetTurnRate() * MsToSec(pUnit->GetAdjustedAttackActionTime()) : g_unitActionAngle);

		if (!pUnit->GetCanRotate() || pUnit->IsImmobilized() || M_DiffAngle(fAngleToTarget, fYaw) <= fActionAngle)
		{
			m_uiInitTime = Game.GetGameTime();

			pUnit->SetAttentionYaw(fAngleToTarget);

			if (!pUnit->GetAttackAnim().empty())
				pUnit->StartRandomAnimation(pUnit->GetAttackAnim(), pUnit->GetAttackNumAnims(), 0, pUnit->GetAttackSpeed());

			pUnit->ClearBonusDamage();
			pUnit->StartAttack(pTarget, false, true);
		}
		else
		{
			// Still turning towards target
			return true;
		}
	}

	if (m_uiInitTime != 0)
	{
		uint uiElapsedTime(Game.GetGameTime() - m_uiInitTime);

		// Flag as attacking a hero
		if (pTarget != NULL && pTarget->IsHero())
			pUnit->SetLastHeroAttackTime(Game.GetGameTime());

		// Waiting for action
		if (uiElapsedTime >= pUnit->GetAdjustedAttackActionTime())
		{
			if (~GetFlags() & ASR_COMPLETED)
			{
				// Attack range buffer check
				if (pTarget != NULL)
				{
					float fRange(pUnit->GetBounds().GetDim(X) * DIAG + pUnit->GetAttackRange() + pUnit->GetAttackRangeBuffer() + pTarget->GetBounds().GetDim(X) * DIAG);

					if (DistanceSq(pUnit->GetPosition().xy(), pTarget->GetPosition().xy()) > SQR(fRange))
					{
						Game.SendPopup(POPUP_TOOFAR, pUnit);
						return false;
					}
				}

				if (pUnit->GetAdjustedAttackCooldown() && !pUnit->HasUnitFlags(UNIT_FLAG_IGNORE_ATTACK_COOLDOWN))
					pUnit->SetAttackCooldownTime(m_uiInitTime + int(pUnit->GetAdjustedAttackCooldown()));

				// Perform the swing/projectile launch
				if (pTarget != NULL)
					pUnit->Attack(pTarget, false);

				pUnit->SetLastAttackTarget(pTarget->GetUniqueID(), m_uiInitTime + int(pUnit->GetAdjustedAttackDuration()));

				SetFlag(ASR_COMPLETED);
			}
		}

		if (GetFlags() & ASR_COMPLETED)
		{
			if (uiElapsedTime >= pUnit->GetAdjustedAttackDuration())
			{
				SetFlag(ASR_ALLDONE);
				bRet = false;
			}
		}
	}

	return bRet;
}


/*====================
  CASAttacking::EndState
  ====================*/
bool	CASAttacking::EndState(uint uiPriority)
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());

	if (GetFlags() & ASR_ALLDONE)
	{
		Reset();
		return true;
	}
	else if (GetFlags() & ASR_COMPLETED)
	{
		ClearFlag(ASR_ACTIVE);
		return true;
	}
	else
	{
		if (!pUnit->GetAttackAnim().empty())
		{
			const tstring &sAnimName(pUnit->GetAttackAnim());

			size_t zPos(sAnimName.find(_T('%')));
			if (zPos != tstring::npos)
			{
				const tstring &sFirstPart(sAnimName.substr(0, zPos));
				const tstring &sLastPart(sAnimName.substr(zPos + 1));

				uint uiNumAnims(pUnit->GetAttackNumAnims());
				for (uint ui(0); ui < uiNumAnims; ++ui)
					pUnit->StopAnimation(sFirstPart + XtoA(ui + 1) + sLastPart, 0);
			}
			else
			{
				pUnit->StopAnimation(sAnimName, 0);
			}
		}

		Reset();
		return true;
	}
	
	return false;
}