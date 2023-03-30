// (C)2007 S2 Games
// c_stateblocked.h
//
//=============================================================================
#ifndef __C_STATEBLOCKED_H__
#define __C_STATEBLOCKED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBlocked
//=============================================================================
class CStateBlocked : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Blocked);

public:
	~CStateBlocked()	{}
	CStateBlocked();
};
//=============================================================================

#endif //__C_STATEBLOCKED_H__
