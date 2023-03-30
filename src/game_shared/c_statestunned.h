// (C)2006 S2 Games
// c_statestunned.h
//
//=============================================================================
#ifndef __C_STATESTUNNED_H__
#define __C_STATESTUNNED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateStunned
//=============================================================================
class CStateStunned : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Stunned)

public:
	~CStateStunned()	{}
	CStateStunned();

	void	Activated();
};
//=============================================================================

#endif //__C_STATESTUNNED_H__
