// (C)2009 S2 Games
// c_bfollowguard.h
//
// Hybrid of Guard and Follow prioritizing following
//=============================================================================
#ifndef __C_BFOLLOWGUARD_H__
#define __C_BFOLLOWGUARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
#include "c_bfollow.h"
#include "c_battack.h"
#include "c_bguard.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBFollowGuard
//=============================================================================
class CBFollowGuard : public CBFollow
{
private:
	CBAttack	m_Attack;
	bool		m_bAttacking;
	uint		m_uiLastAggroUpdate;

	EGuardState	m_eGuardState;
	uint		m_uiGuardStateEndTime;

	mutable bool	m_bIsTravelingRecurseGuard;

	void	UpdateAggro();
	void	Aggro(IUnitEntity *pAttacker, uint uiChaseTime);

public:
	~CBFollowGuard()	{}
	CBFollowGuard() :
	m_Attack(INVALID_INDEX),
	m_bAttacking(false),
	m_uiLastAggroUpdate(INVALID_TIME),
	m_bIsTravelingRecurseGuard(false)
	{
		SetType(EBT_FOLLOWGUARD);
	}

	virtual void		CopyFrom(const IBehavior* pBehavior);
	virtual IBehavior*	Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

	virtual void	BeginBehavior();
	virtual void	ThinkFrame();
	virtual void	MovementFrame();
	virtual void	ActionFrame();
	virtual void	CleanupFrame();
	virtual void	EndBehavior();

	virtual void	Damaged(IUnitEntity *pAttacker);
	virtual void	Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker);

	virtual bool	IsIdle() const			{ return m_eGuardState == GUARD_HOLDING; }
	virtual bool	IsTraveling() const;
	virtual uint	GetAttackTarget() const	{ return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }
};
//=============================================================================

#endif //__C_BFOLLOWGUARD_H__
