// (C)2006 S2 Games
// c_statebackstab.h
//
//=============================================================================
#ifndef __C_STATEBACKSTAB_H__
#define __C_STATEBACKSTAB_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBackStab
//=============================================================================
class CStateBackStab : public IEntityState
{
private:
	DECLARE_STATE_ALLOCATOR(BackStab);
	DECLARE_ENTITY_STATE_CVARS;
	static CCvarui	s_cvarAttackTime;
	static CCvarf	s_cvarAttackDamage;

public:
	~CStateBackStab();
	CStateBackStab();

	void	DoAttack(CMeleeAttackEvent &attack);
};
//=============================================================================

#endif //__C_STATEBACKSTAB_H__
