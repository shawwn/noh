// (C)2006 S2 Games
// c_statedoubleswing.h
//
//=============================================================================
#ifndef __C_DOUBLESWING_H__
#define __C_DOUBLESWING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateDoubleSwing
//=============================================================================
class CStateDoubleSwing : public IEntityState
{
private:
	DECLARE_STATE_ALLOCATOR(DoubleSwing);
	DECLARE_ENTITY_STATE_CVARS;
	static CCvarui	s_cvarAttackTime;
	static CCvarf	s_cvarAttackMinDamage;
	static CCvarf	s_cvarAttackMaxDamage;

public:
	~CStateDoubleSwing();
	CStateDoubleSwing();

	void	DoAttack(CMeleeAttackEvent &attack);
};
//=============================================================================

#endif // __C_DOUBLESWING_H__
