// (C)2007 S2 Games
// c_sateinterrupted.h
//
//=============================================================================
#ifndef __C_STATEINTERRUPTED_H__
#define __C_STATEINTERRUPTED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateInterrupted
//=============================================================================
class CStateInterrupted : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Interrupted);

public:
	~CStateInterrupted();
	CStateInterrupted();
};
//=============================================================================

#endif //__C_STATEINTERRUPTED_H__
