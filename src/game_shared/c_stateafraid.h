// (C)2007 S2 Games
// c_stateafraid.h
//
//=============================================================================
#ifndef __C_STATEAFRAID_H__
#define __C_STATEAFRAID_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateAfraid
//=============================================================================
class CStateAfraid : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Afraid);

public:
	~CStateAfraid()	{}
	CStateAfraid();
};
//=============================================================================

#endif //__C_STATEAFRAID_H__
