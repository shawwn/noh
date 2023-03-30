// (C)2006 S2 Games
// c_meleeattackevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_meleeattackevent.h"
#include "i_meleeitem.h"
//=============================================================================

/*====================
  CMeleeAttackEvent::CMeleeAttackEvent
  ====================*/
CMeleeAttackEvent::CMeleeAttackEvent() :
m_bActive(false),
m_pWeapon(NULL),
m_uiAttackTime(0),
m_rangeDamage(0.0f, 0.0f),
m_bImpacted(false),
m_uiImpactTime(0),
m_uiImpactEndTime(0),
m_uiStartTime(0),
m_fDamage(0.0f),
m_fRearAttackMultiplier(1.0f),
m_iDamageFlags(0),
m_fStaminaCost(0.0f),
m_v3Push(V_ZERO),
m_v3Lunge(V_ZERO),
m_fHealthLeach(0.0f),
m_fPivotHeight(0.0f),
m_fPivotFactor(0.0f)
{
	m_vStates.clear();
	for (int i(0); i < NUM_MELEE_METRICS; ++i)
	{
		m_fMin[i] = 0.0f;
		m_fMax[i] = 0.0f;
		m_fStep[i] = FAR_AWAY;
	}
}


/*====================
  CMeleeAttackEvent::Clear
  ====================*/
void	CMeleeAttackEvent::Clear()
{
	m_bActive = false;

	m_pWeapon = NULL;

	m_sAnimName.clear();
	m_uiAttackTime = 0;
	m_rangeDamage.Set(0.0f);
	m_vStates.clear();
	m_vStateDurations.clear();

	m_bImpacted = false;
	m_uiImpactTime = 0;
	m_uiImpactEndTime = 0;

	m_fDamage = 0.0f;
	ClearDamageFlags();

	for (int n(0); n < NUM_MELEE_METRICS; ++n)
	{
		m_fMin[n] = 0.0f;
		m_fMax[n] = 0.0f;
		m_fStep[n] = 0.0f;
	}

	m_fPivotHeight = 0.0f;
	m_fPivotFactor = 0.0f;

	m_fHealthLeach = 0.0f;

	m_v3Lunge.Clear();
	m_v3Push.Clear();
}


/*====================
  CMeleeAttackEvent::SetMetric
  ====================*/
void	CMeleeAttackEvent::SetMetric(EMeleeMetric eMetric, float fMin, float fMax, float fStep)
{
	if (fMax <= fMin || fStep < 1.0f)
		Console.Warn << _T("Bad attack metric") << newl;

	m_fMin[eMetric] = fMin;
	m_fMax[eMetric] = MAX(fMax, fMin);
	m_fStep[eMetric] = MAX(1.0f, ((fMax - fMin) / MAX(1.0f, floorf((fMax - fMin) / fStep))));

	// Subtract a little bit to make sure that a precison error doesn't cause the
	// last step to be skipped
	if (eMetric != MELEE_ANGLE)
		m_fStep[eMetric] -= 0.001f;
}


/*====================
  CMeleeAttackEvent::Push
  ====================*/
void	CMeleeAttackEvent::Push(const CVec3f &v3Angles, ICombatEntity *pTarget)
{
	if (pTarget == NULL)
		return;

	CAxis axis(v3Angles);
	CVec3f v3AttackPush(GetPush());
	CVec3f v3Push(axis.Forward2d());
	v3AttackPush *= pTarget->GetPushMultiplier();
	v3Push *= v3AttackPush.x;
	v3Push += axis.Right() * v3AttackPush.y;
	v3Push.z = v3AttackPush.z;
	pTarget->ApplyVelocity(v3Push);
}


/*====================
  CMeleeAttackEvent::TryImpact
  ====================*/
bool	CMeleeAttackEvent::TryImpact(uint uiTime)
{
	if (m_pWeapon == NULL)
		return false;

	if (m_bImpacted)
		return false;

	if (uiTime < m_uiImpactTime)
		return false;

	m_pWeapon->Impact();
	m_bImpacted = true;
	return true;
}


/*====================
  CMeleeAttackEvent::GetCenter
  ====================*/
CVec3f	CMeleeAttackEvent::GetCenter(const CVec3f &v3Center, const CAxis &axis, float fHeight) const
{
	CVec3f v3Up(axis.Up() * m_fPivotFactor + V_UP * (1.0f - m_fPivotFactor));
	return v3Center + (v3Up * (fHeight - m_fPivotHeight) + V_UP * m_fPivotHeight);
}


/*====================
  CMeleeAttackEvent::GetDir
  ====================*/
CVec3f	CMeleeAttackEvent::GetDir(const CAxis &axis, float fAngle) const
{
	CVec3f v3ForwardXY(axis.Forward());
	v3ForwardXY[Z] = 0.0f;
	v3ForwardXY.Normalize();
	v3ForwardXY *= cos(DEG2RAD(fAngle));

	CVec3f v3Forward(axis.Forward() * cos(DEG2RAD(fAngle)));
	v3Forward = (v3Forward * m_fPivotFactor) + (v3ForwardXY * (1.0f - m_fPivotFactor));
	
	CVec3f v3RightXY(axis.Right());
	v3RightXY[Z] = 0.0f;
	v3RightXY.Normalize();
	v3RightXY *= sin(DEG2RAD(fAngle));

	CVec3f v3Right(axis.Right() * sin(DEG2RAD(fAngle)));
	v3Right = (v3Right * m_fPivotFactor) + (v3RightXY * (1.0f - m_fPivotFactor));
	
	CVec3f v3Dir(v3Forward + v3Right);
	v3Dir.Normalize();
	return v3Dir;
}
