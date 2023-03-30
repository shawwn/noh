// (C)2006 S2 Games
// c_statemanastone.h
//
//=============================================================================
#ifndef __C_STATEMANASTONE_H__
#define __C_STATEMANASTONE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateManaStone
//=============================================================================
class CStateManaStone : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ManaCostMultiplier)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, ManaStone);

public:
	~CStateManaStone()	{}
	CStateManaStone();
};
//=============================================================================

#endif //__C_STATEMANASTONE_H__
