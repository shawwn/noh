// (C)2006 S2 Games
// c_staterainoffire.h
//
//=============================================================================
#ifndef __C_STATERAINOFFIRE_H__
#define __C_STATERAINOFFIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateRainOfFire
//=============================================================================
class CStateRainOfFire : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, RainOfFire);

public:
	~CStateRainOfFire()	{}
	CStateRainOfFire();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATERAINOFFIRE_H__
