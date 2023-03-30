// (C)2007 S2 Games
// c_stateflames.h
//
//=============================================================================
#ifndef __C_STATEFLAMES_H__
#define __C_STATEFLAMES_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateFlames
//=============================================================================
class CStateFlames : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, Radius)
		DECLARE_ENTITY_CVAR(tstring, State)
		DECLARE_ENTITY_CVAR(uint, StateDuration)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Flames);

public:
	~CStateFlames()	{}
	CStateFlames();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEFLAMES_H__
