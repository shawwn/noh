// (C)2007 S2 Games
// c_stateburrowed.h
//
//=============================================================================
#ifndef __C_STATEBURROWED_H__
#define __C_STATEBURROWED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBurrowed
//=============================================================================
class CStateBurrowed : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Burrowed)

public:
	~CStateBurrowed()	{}
	CStateBurrowed();
};
//=============================================================================

#endif //__C_STATEBURROWED_H__
