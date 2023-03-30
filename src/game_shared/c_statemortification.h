// (C)2006 S2 Games
// c_statemortification.h
//
//=============================================================================
#ifndef __C_STATEMORTIFICATION_H__
#define __C_STATEMORTIFICATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateMortification
//=============================================================================
class CStateMortification : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, SpeedReduction)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Mortification);

public:
	~CStateMortification()	{}
	CStateMortification();
};
//=============================================================================

#endif //__C_STATEMORTIFICATION_H__
