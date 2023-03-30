// (C)2008 S2 Games
// c_bstop.h
//
//=============================================================================
#ifndef __C_BSTOP_H__
#define __C_BSTOP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBStop
//=============================================================================
class CBStop : public IBehavior
{
private:

public:
	CBStop() :
	IBehavior(EBT_STOP)
	{}

	virtual void		CopyFrom(const IBehavior* pBehavior);
	virtual IBehavior*	Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

	virtual bool	Validate();
	virtual void	Update() {}
	virtual void	BeginBehavior();
	virtual void	ThinkFrame();
	virtual void	MovementFrame();
	virtual void	ActionFrame();
	virtual void	CleanupFrame();
};
//=============================================================================

#endif // __C_BSTOP_H__
