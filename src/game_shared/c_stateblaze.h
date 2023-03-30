// (C)2007 S2 Games
// c_stateblaze.h
//
//=============================================================================
#ifndef __C_STATEBLAZE_H__
#define __C_STATEBLAZE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBlaze
//=============================================================================
class CStateBlaze : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Blaze);

public:
	~CStateBlaze()	{}
	CStateBlaze();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEBLAZE_H__
