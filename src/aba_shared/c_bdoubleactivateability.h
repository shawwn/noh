// (C)2010 S2 Games
// c_bdoubleactivateability.h
//
//=============================================================================
#ifndef __C_BDOUBLEACTIVATEABILITY_H__
#define __C_BDOUBLEACTIVATEABILITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IEntityTool;
//=============================================================================

//=============================================================================
// CBDoubleActivateAbility
//=============================================================================
class CBDoubleActivateAbility : public IBehavior
{
private:
	int				m_iInventorySlot;
	IEntityTool*	m_pAbility;

	CBDoubleActivateAbility();

public:
	~CBDoubleActivateAbility() {}
	CBDoubleActivateAbility(int iInventorySlot) :
	IBehavior(EBT_DOUBLE_ACTIVATE_ABILITY),
	m_iInventorySlot(iInventorySlot)
	{
	}

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

#endif //__C_BDOUBLEACTIVATEABILITY_H__

