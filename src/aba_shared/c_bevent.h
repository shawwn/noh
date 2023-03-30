// (C)2009 S2 Games
// c_bevent.h
//
//=============================================================================
#ifndef __C_BEVENT_H__
#define __C_BEVENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBEvent
//=============================================================================
class CBEvent : public IBehavior
{
private:
	// Parameters calculated during setup frame
	float	m_fDistSq;		
	CVec2f	m_v2ApproachPosition;
	float	m_fRange;

public:
	CBEvent() :
	IBehavior(EBT_EVENT)
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
	virtual void	EndBehavior();
};
//=============================================================================

#endif // __C_BEVENT_H__
