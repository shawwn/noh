// (C)2008 S2 Games
// c_bgiveitem.h
//
//=============================================================================
#ifndef __C_BGIVEITEM_H__
#define __C_BGIVEITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBGiveItem
//=============================================================================
class CBGiveItem : public IBehavior
{
private:
	uint	m_uiItemUID;

	// Parameters calculated during setup frame
	float	m_fDistSq;		
	CVec2f	m_v2ApproachPosition;
	float	m_fRange;

public:
	CBGiveItem(uint uiItemUID) :
	IBehavior(EBT_GIVEITEM),
	m_uiItemUID(uiItemUID)
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

#endif // __C_BGIVEITEM_H__
