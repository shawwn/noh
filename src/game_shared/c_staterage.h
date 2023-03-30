// (C)2006 S2 Games
// c_staterage.h
//
//=============================================================================
#ifndef __C_STATERAGE_H__
#define __C_STATERAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateRage
//=============================================================================
class CStateRage : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, AttackSpeedMult)
		DECLARE_ENTITY_CVAR(float, MoveSpeedMult)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Rage);

public:
	~CStateRage()	{}
	CStateRage();
};
//=============================================================================

#endif //__C_STATERAGE_H__
