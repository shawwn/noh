// (C)2006 S2 Games
// c_statedefile.h
//
//=============================================================================
#ifndef __C_STATEDEFILE_H__
#define __C_STATEDEFILE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateDefile
//=============================================================================
class CStateDefile : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Defile);

public:
	~CStateDefile()	{}
	CStateDefile();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEDEFILE_H__
