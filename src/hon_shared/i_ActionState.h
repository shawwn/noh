// (C)2007 S2 Games
// i_ActionState.h
//
//=============================================================================
#ifndef __I_ACTIONSTATE_H__
#define __I_ACTIONSTATE_H__

//=============================================================================
//=============================================================================
class CBrain;
//=============================================================================

//=============================================================================
//=============================================================================
// Action State Result
const uint ASR_ACTIVE		(BIT(0));	// Brain is processing this state for the unit
const uint ASR_COMMITTED	(BIT(1));	// Require the unit to complete the processing of the current state
const uint ASR_COMPLETED	(BIT(2));	// The intent of the action was completed (attack/cast/moved to goal)
const uint ASR_ALLDONE		(BIT(3));	// No further action will be brought about (animation is complete, intent is complete)
const uint ASR_INTERRUPTED	(BIT(4));	// Try again later

enum eActionStateIDs
{
	ASID_ATTACKING = 0,
	ASID_CASTING,
	ASID_MOVING,
	ASID_COUNT
};
//=============================================================================

//=============================================================================
// IActionState
//=============================================================================
class IActionState
{
private:
	uint	m_uiActionStateFlags;

protected:
	CBrain &m_cBrain;

	// Contain the flags to signal the external world
	void SetFlag(uint uiFlag) { m_uiActionStateFlags |= uiFlag; }
	void ClearFlag(uint uiFlag) { m_uiActionStateFlags &= ~uiFlag; }
	void ClearAllFlags() { m_uiActionStateFlags = 0; }

	void CopyActionStateFrom(const IActionState* pActionState)
	{
		m_uiActionStateFlags = pActionState->m_uiActionStateFlags;
	}

public:
	IActionState(CBrain &cParent) : m_uiActionStateFlags(0), m_cBrain(cParent) { }

	bool	IsActive() const		{ return (m_uiActionStateFlags & ASR_ACTIVE) != 0; }

	// bool frame (param?)
	inline uint GetFlags() const	{ return m_uiActionStateFlags; }

	// A state may choose to fail begin/end
	virtual bool BeginState() = 0;				// (e.g. attack would fail if the angle to target is > 90 degrees)
	virtual bool ContinueStateMovement() = 0;	// (e.g. movement would fail if a debuff prevents the state from continuing)
	virtual bool ContinueStateAction() = 0;		// (e.g. movement would fail if a debuff prevents the state from continuing)
	virtual bool ContinueStateCleanup() = 0;
	virtual bool EndState(uint uiPriority) = 0;	// (e.g. cast would fail if the hero is committed, yet the task is not complete)
};
//=============================================================================

#endif // __I_ACTIONSTATE_H__
