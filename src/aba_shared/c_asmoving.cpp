// (C)2007 S2 Games
// c_asMoving.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_asMoving.h"

#include "i_unitentity.h"
#include "c_brain.h"
//=============================================================================

/*====================
  CASMoving::CASMoving
  ====================*/
CASMoving::CASMoving(CBrain &cParent) :
IActionState(cParent),
m_v2Movement(V2_ZERO),
m_fYawDelta(0.0f),
m_bBlocked(false),
m_uiBlockTime(INVALID_TIME),
m_uiMoveTime(INVALID_TIME)
{
}

/*====================
  CASMoving::CopyFrom
  ====================*/
void	CASMoving::CopyFrom(const CASMoving *pActionState)
{
	m_v2Movement = pActionState->m_v2Movement;
	m_fYawDelta = pActionState->m_fYawDelta;
	m_bBlocked = pActionState->m_bBlocked;
	m_plImpactPlane = pActionState->m_plImpactPlane;
	m_uiBlockTime = pActionState->m_uiBlockTime;
	m_uiMoveTime = pActionState->m_uiMoveTime;

	CopyActionStateFrom(pActionState);
}

/*====================
  CASMoving::BeginState
  ====================*/
bool	CASMoving::BeginState()
{
	// May fail on certain debuffs

	// Animation may vary in multistate skills (strafe, etc)
	assert(m_cBrain.GetUnit());

	IUnitEntity *pUnit(m_cBrain.GetUnit());

	if (!pUnit->GetIsMobile())
		return false;

	SetFlag(ASR_ACTIVE);

	m_bBlocked = false;
	m_plImpactPlane = CPlane(0.0f, 0.0f, 0.0f, 0.0f);
	m_uiBlockTime = INVALID_TIME;

	pUnit->Interrupt(UNIT_ACTION_MOVE);

	return true;
}


/*====================
  CASMoving::ShouldTryUnblock
  ====================*/
bool	CASMoving::ShouldTryUnblock()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	if (pUnit == NULL)
		return false;

	if (pUnit->IsImmobilized(true, true))
		return false;
	if (pUnit->IsStunned())
		return false;

	return m_bBlocked && (m_uiBlockTime == INVALID_TIME || m_uiBlockTime < Game.GetGameTime());
}


/*====================
  CASMoving::ContinueStateMovement
  ====================*/
bool	CASMoving::ContinueStateMovement()
{
	// May fail on certain debuffs, requiring the state to continue??
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	if (pUnit == NULL)
		return false;

	if (pUnit->IsImmobilized(false, true))
		return true;
	if (pUnit->IsStunned())
		return true;

	if (ShouldTryUnblock())
		m_bBlocked = false;

	if (m_bBlocked)
		return true;

	if (m_v2Movement != V2_ZERO)
		m_uiMoveTime = Game.GetGameTime();

	// traceline in movedirection
	if (m_v2Movement == V2_ZERO && m_fYawDelta == 0.0f)
		return false;

	CVec3f v3Angles(pUnit->GetAngles());

	if (pUnit->GetCanRotate())
		v3Angles[YAW] += m_fYawDelta;

	pUnit->SetAngles(v3Angles);

	if (!pUnit->IsImmobilized(true, false))
	{
		CVec2f v2Forward(M_GetForwardVec2FromYaw(v3Angles[YAW]));

		if (acos(CLAMP(DotProduct(v2Forward, Normalize(m_v2Movement)), -1.0f, 1.0f)) <= DEG2RAD(g_unitMoveAngle) || !pUnit->GetCanRotate())
			m_bBlocked = pUnit->JustWalkNike(m_v2Movement, m_plImpactPlane, m_bDirectPathing);
	}
	else
	{
		m_bBlocked = false;
	}

	m_v2Movement = V2_ZERO;
	m_fYawDelta = 0.0f;

	if (m_bBlocked)
	{
		m_uiMoveTime = INVALID_TIME;
		m_uiBlockTime = Game.GetGameTime() + pUnit->GetRepathTime() + M_Randnum(0, 1) * Game.GetFrameLength(); 
		//m_uiBlockTime = Game.GetGameTime() + pUnit->GetRepathTime() + M_Randnum((uint)0, pUnit->GetRepathTimeExtra());
	}

	// Continue the state for this frame, movement was performed
	return true;
}


/*====================
  CASMoving::ContinueStateCleanup
  ====================*/
bool	CASMoving::ContinueStateCleanup()
{
	IUnitEntity *pUnit(m_cBrain.GetUnit());
	if (pUnit == NULL)
		return false;

	if (m_uiMoveTime == Game.GetGameTime())
	{
		pUnit->SetAnim(
			0,
			pUnit->HasUnitFlags(UNIT_FLAG_SPRINTING) ? pUnit->GetSprintAnim() : pUnit->GetWalkAnim(),
			pUnit->GetMoveSpeed() / pUnit->GetInitialMoveSpeed(),
			0
		);
	}
	else if (pUnit->IsPlayingAnim(0, pUnit->GetWalkAnim()) || pUnit->IsPlayingAnim(0, pUnit->GetSprintAnim()))
	{
		pUnit->SetAnim(0, pUnit->GetIdleAnim(), 1.0f, 0);
	}

	return true;
}


/*====================
  CASMoving::EndState
  ====================*/
bool	CASMoving::EndState(uint uiPriority)
{
	// Movement may always be force-terminated
	IUnitEntity *pUnit(m_cBrain.GetUnit());

	assert(pUnit);
	if (!pUnit)
		return false;

	if (pUnit->IsPlayingAnim(0, pUnit->GetWalkAnim()) || pUnit->IsPlayingAnim(0, pUnit->GetSprintAnim()))
		pUnit->SetAnim(0, pUnit->GetIdleAnim(), 1.0f, 0);

	ClearFlag(ASR_ACTIVE);

	m_bBlocked = false;
	m_uiBlockTime = INVALID_TIME;

	return true;
}
