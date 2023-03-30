// (C)2007 S2 Games
// c_statetargeted.h
//
//=============================================================================
#ifndef __C_STATETARGETED_H__
#define __C_STATETARGETED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateTargeted
//=============================================================================
class CStateTargeted : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorAdjustment)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Targeted)

public:
	~CStateTargeted()	{}
	CStateTargeted();
};
//=============================================================================

#endif //__C_STATETARGETED_H__
