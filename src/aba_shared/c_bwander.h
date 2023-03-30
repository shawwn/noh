// (C)2009 S2 Games
// c_bwander.h
//=============================================================================
#ifndef __C_BWANDER_H__
#define __C_BWANDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBWander
//=============================================================================
class CBWander : public IBehavior
{
protected:
	CVec2f	m_v2Origin;
	CVec2f	m_v2Offset;

	uint	m_uiLastWanderTime;

	float	m_fDistSq;
	float	m_fMinDistanceSq;

public:
	virtual	~CBWander()	{}
	
	CBWander() :
	IBehavior(EBT_WANDER),
	m_v2Origin(V2_ZERO),
	m_v2Offset(V2_ZERO),
	m_uiLastWanderTime(INVALID_TIME),
	m_fDistSq(0.0f),
	m_fMinDistanceSq(0.0f)
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

#endif	//__C_BWANDER_H__