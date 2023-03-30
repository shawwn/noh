// (C)2007 S2 Games
// c_statespawned.h
//
//=============================================================================
#ifndef __C_STATESPAWNED_H__
#define __C_STATESPAWNED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateSpawned
//=============================================================================
class CStateSpawned : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG
	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Spawned);

public:
	~CStateSpawned()	{}
	CStateSpawned();
};
//=============================================================================

#endif //__C_STATESPAWNED_H__
