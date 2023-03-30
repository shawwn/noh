// (C)2007 S2 Games
// c_stateenraged.h
//
//=============================================================================
#ifndef __C_STATEENRAGED_H__
#define __C_STATEENRAGED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateEnraged
//=============================================================================
class CStateEnraged : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, AttackSpeedMult)
		DECLARE_ENTITY_CVAR(float, MoveSpeedMult)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, DamageMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Enraged)

public:
	~CStateEnraged()	{}
	CStateEnraged();
};
//=============================================================================

#endif //__C_STATEENRAGED_H__
