// (C)2006 S2 Games
// c_statehealthreplenish.h
//
//=============================================================================
#ifndef __C_STATEHEALTHREPLENISH_H__
#define __C_STATEHEALTHREPLENISH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateHealthReplenish
//=============================================================================
class CStateHealthReplenish : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, HealthRegenBoost)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, HealthReplenish);

public:
	~CStateHealthReplenish()	{}
	CStateHealthReplenish();
};
//=============================================================================

#endif //__C_STATEHEALTHREPLENISH_H__
