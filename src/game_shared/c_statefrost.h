// (C)2007 S2 Games
// c_statefrost.h
//
//=============================================================================
#ifndef __C_STATEFROST_H__
#define __C_STATEFROST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateFrost
//=============================================================================
class CStateFrost : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, FrostSlow)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Frost);

public:
	~CStateFrost()	{}
	CStateFrost();
};
//=============================================================================

#endif //__C_STATEFROST_H__
