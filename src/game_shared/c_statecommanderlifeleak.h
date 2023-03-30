// (C)2007 S2 Games
// c_statecommanderlifeleak.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERLIFELEAK_H__
#define __C_STATECOMMANDERLIFELEAK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderLifeLeak
//=============================================================================
class CStateCommanderLifeLeak : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, CommanderLifeLeak);

public:
	~CStateCommanderLifeLeak()	{}
	CStateCommanderLifeLeak();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATECOMMANDERLIFELEAK_H__
