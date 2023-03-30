// (C)2007 S2 Games
// c_asMoving.h
//
//=============================================================================
#ifndef __C_ASMOVING_H__
#define __C_ASMOVING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_ActionState.h"
//=============================================================================

//=============================================================================
// CASMoving
//=============================================================================
class CASMoving : public IActionState
{
private:
	CVec2f		m_v2Movement;
	float		m_fYawDelta;
	bool		m_bBlocked;
	CPlane		m_plImpactPlane;
	uint		m_uiBlockTime;
	uint		m_uiMoveTime;
	bool		m_bDirectPathing;
	
public:
	CASMoving(CBrain &cParent);

	void			CopyFrom(const CASMoving *pActionState);

	// Core Operations
	virtual bool	BeginState();
	virtual bool	ContinueStateMovement();
	virtual bool	ContinueStateAction() { return true; }
	virtual bool	ContinueStateCleanup();
	virtual bool	EndState(uint uiPriority);

	// Accessor functions
	void SetMovement(CVec2f &v2Movement, float &fYawDelta, bool bDirectPathing)
	{
		m_v2Movement = v2Movement;
		m_fYawDelta = fYawDelta;
		m_bDirectPathing = bDirectPathing;
	}

	bool			GetBlocked()		{ return m_bBlocked; }
	const CPlane&	GetImpactPlane()	{ return m_plImpactPlane; }
	uint			GetBlockTime()		{ return m_uiBlockTime; }

	bool			IsBlocked()			{ return m_bBlocked && m_uiBlockTime >= Game.GetGameTime(); }
	bool			ShouldTryUnblock();
};
//=============================================================================

#endif //__C_ASMOVING_H__
