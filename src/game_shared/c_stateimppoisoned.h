// (C)2006 S2 Games
// c_stateimppoisoned.h
//
//=============================================================================
#ifndef __C_STATEIMPPOISONED_H__
#define __C_STATEIMPPOISONED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateImpPoisoned
//=============================================================================
class CStateImpPoisoned : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, ImpPoisoned);

public:
	~CStateImpPoisoned()	{}
	CStateImpPoisoned();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEIMPPOISONED_H__
