// (C) 2006 S2 Games
// c_statecombustion.h
//
//=============================================================================
#ifndef __C_STATECOMBUSTION_H__
#define __C_STATECOMBUSTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCombustion
//=============================================================================
class CStateCombustion : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Combustion);

public:
	~CStateCombustion()	{}
	CStateCombustion();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATECOMBUSTION_H__
