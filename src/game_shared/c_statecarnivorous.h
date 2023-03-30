// (C)2007 S2 Games
// c_statecarnivorous.h
//
//=============================================================================
#ifndef __C_STATECARNIVOROUS_H__
#define __C_STATECARNIVOROUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCarnivorous
//=============================================================================
class CStateCarnivorous : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, LeachAmount)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Carnivorous)

public:
	~CStateCarnivorous()	{}
	CStateCarnivorous();

	void	DoAttack(CMeleeAttackEvent &attack);
};
//=============================================================================

#endif //__C_STATECARNIVOROUS_H__
