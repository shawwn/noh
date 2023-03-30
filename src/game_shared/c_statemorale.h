// (C)2006 S2 Games
// c_statemorale.h
//
//=============================================================================
#ifndef __C_STATEMORALE_H__
#define __C_STATEMORALE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateMorale
//=============================================================================
class CStateMorale : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, HealthRegenMult)
		DECLARE_ENTITY_CVAR(float, ManaRegenMult)
		DECLARE_ENTITY_CVAR(float, StaminaRegenMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Morale);

public:
	~CStateMorale() {};
	CStateMorale();
};
//=============================================================================

#endif //__C_STATEMORALE_H__
