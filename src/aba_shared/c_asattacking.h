// (C)2007 S2 Games
// c_asAttacking.h
//
//=============================================================================
#ifndef __C_ASATTACKING_H__
#define __C_ASATTACKING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_ActionState.h"
//=============================================================================

//=============================================================================
// CASAttacking
//=============================================================================
class CASAttacking : public IActionState
{
private:
	uint	m_uiBeginTargetIndex;

	uint	m_uiInitTime;
	uint	m_uiTargetIndex;
	
	inline void Reset()
	{
		ClearAllFlags();
		m_uiInitTime = 0;
		m_uiTargetIndex = INVALID_INDEX;
	}

public:
	CASAttacking(CBrain &cParent);

	void			CopyFrom(const CASAttacking *pActionState);

	// Core Operations
	virtual bool	BeginState();
	virtual bool	ContinueStateMovement();
	virtual bool	ContinueStateAction();
	virtual bool	ContinueStateCleanup() { return true; }
	virtual bool	EndState(uint uiPriority);

	// Begin parameters
	void	SetBeginAttackTarget(uint uiIndex)	{ m_uiBeginTargetIndex = uiIndex; }

	// Actual parameters
	void	SetAttackTarget(uint uiIndex)		{ m_uiTargetIndex = uiIndex; }
	uint	GetAttackTarget() const				{ return m_uiTargetIndex; }
};
//=============================================================================

#endif // __C_ASATTACKING_H__