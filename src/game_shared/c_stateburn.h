// (C)2006 S2 Games
// c_stateburn.h
//
//=============================================================================
#ifndef __C_STATEBURN_H__
#define __C_STATEBURN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBurn
//=============================================================================
class CStateBurn : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Burn);

public:
	~CStateBurn()	{}
	CStateBurn();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEBURN_H__
