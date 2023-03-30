// (C)2009 S2 Games
// c_bguardfollow.h
//
// Hybrid of Guard and Follow prioritizing guarding
//=============================================================================
#ifndef __C_BGUARDFOLLOW_H__
#define __C_BGUARDFOLLOW_H__

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
// CBGuardFollow
//=============================================================================
class CBGuardFollow : public CBFollow
{
private:
	CBAttack	m_Attack;
	bool		m_bAttacking;
	uint		m_uiLastAggroUpdate;

	EGuardState	m_eGuardState;
	uint		m_uiGuardStateEndTime;

	void	UpdateAggro();
	void	Aggro(IUnitEntity *pAttacker, uint uiChaseTime);

public:
	~CBGuardFollow()	{}
	CBGuardFollow() :
	m_Attack(INVALID_INDEX),
	m_bAttacking(false),
	m_uiLastAggroUpdate(INVALID_TIME)
	{
		SetType(EBT_GUARDFOLLOW);
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
	virtual bool	IsTraveling() const		{ return m_eGuardState == GUARD_RETURNING; }
	virtual uint	GetAttackTarget() const	{ return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }
};
//=============================================================================

#endif //__C_BGUARDFOLLOW_H__
