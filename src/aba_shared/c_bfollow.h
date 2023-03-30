// (C)2008 S2 Games
// c_bfollow.h
//
//=============================================================================
#ifndef __C_BFOLLOW_H__
#define __C_BFOLLOW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBFollow
//=============================================================================
class CBFollow : public IBehavior
{
private:
	float	m_fMinFollowDistance;
	float	m_fMaxFollowDistance;

	// Parameters calculated during setup frame
	float	m_fDistSq;
	CVec2f	m_v2ApproachPosition;
	float	m_fFollowRangeMin;
	float	m_fFollowRangeMax;

public:
	CBFollow() :
	IBehavior(EBT_FOLLOW),
	m_fMinFollowDistance(64.0f),
	m_fMaxFollowDistance(128.0f)
	{}

	virtual void		CopyFrom(const IBehavior* pBehavior);
	virtual IBehavior*	Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

	virtual bool	Validate();
	virtual void	Update();
	virtual void	BeginBehavior();
	virtual void	ThinkFrame();
	virtual void	MovementFrame();
	virtual void	ActionFrame();
	virtual void	CleanupFrame();
};
//=============================================================================

#endif //__C_BFOLLOW_H__
