// (C)2007 S2 Games
// c_statevenus.h
//
//=============================================================================
#ifndef __C_STATEVENUS_H__
#define __C_STATEVENUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateVenus
//=============================================================================
class CStateVenus : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Venus)

public:
	~CStateVenus()	{}
	CStateVenus();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEVENUS_H__
