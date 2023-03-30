// (C)2007 S2 Games
// c_statepoisoned.h
//
//=============================================================================
#ifndef __C_STATEPOISONED_H__
#define __C_STATEPOISONED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStatePoisoned
//=============================================================================
class CStatePoisoned : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Poisoned)

public:
	~CStatePoisoned()	{}
	CStatePoisoned();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEPOISONED_H__
