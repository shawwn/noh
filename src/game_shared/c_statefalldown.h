// (C)2007 S2 Games
// c_statefalldown.h
//
//=============================================================================
#ifndef __C_STATEFALLDOWN_H__
#define __C_STATEFALLDOWN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateFallDown
//=============================================================================
class CStateFallDown : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, FallDown)

public:
	~CStateFallDown()	{}
	CStateFallDown();
};
//=============================================================================

#endif //__C_STATEFALLDOWN_H__
