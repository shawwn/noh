// (C)2007 S2 Games
// c_stateblind.h
//
//=============================================================================
#ifndef __C_STATEBLIND_H__
#define __C_STATEBLIND_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBlind
//=============================================================================
class CStateBlind : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Blind)

public:
	~CStateBlind()	{}
	CStateBlind();
};
//=============================================================================

#endif //__C_STATEBLIND_H__
