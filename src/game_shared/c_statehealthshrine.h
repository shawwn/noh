// (C)2006 S2 Games
// c_statehealthshrine.h
//
//=============================================================================
#ifndef __C_STATEHEALTHSHRINE_H__
#define __C_STATEHEALTHSHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateHealthShrine
//=============================================================================
class CStateHealthShrine : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, HealthRegenBoost)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, HealthShrine);

public:
	~CStateHealthShrine()	{}
	CStateHealthShrine();
};
//=============================================================================

#endif //__C_STATEHEALTHSHRINE_H__
