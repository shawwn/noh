// (C)2006 S2 Games
// c_stateimmolate.h
//
//=============================================================================
#ifndef __C_STATEIMMOLATE_H__
#define __C_STATEIMMOLATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateImmolate
//=============================================================================
class CStateImmolate : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Immolate);

public:
	~CStateImmolate()	{}
	CStateImmolate();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEIMMOLATE_H__
