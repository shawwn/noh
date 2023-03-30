// (C)2007 S2 Games
// c_statecommandercripple.h
//
//=============================================================================
#ifndef __C_STATECOMMANDERCRIPPLE_H__
#define __C_STATECOMMANDERCRIPPLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCommanderCripple
//=============================================================================
class CStateCommanderCripple : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, SpeedAdd)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, CommanderCripple);

public:
	~CStateCommanderCripple()	{}
	CStateCommanderCripple();
};
//=============================================================================

#endif //__C_STATECOMMANDERCRIPPLE_H__
