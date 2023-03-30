// (C)2006 S2 Games
// c_statebearlothstunned.h
//
//=============================================================================
#ifndef __C_STATEBEARLOTHSTUNNED_H__
#define __C_STATEBEARLOTHSTUNNED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBearlothStunned
//=============================================================================
class CStateBearlothStunned : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, BearlothStunned);

public:
	~CStateBearlothStunned()	{}
	CStateBearlothStunned();

	void	Activated();
};
//=============================================================================

#endif //__C_STATEBEARLOTHSTUNNED_H__
