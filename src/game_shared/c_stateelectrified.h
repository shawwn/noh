// (C)2007 S2 Games
// c_stateelectrified.h
//
//=============================================================================
#ifndef __C_STATEELECTRIFIED_H__
#define __C_STATEELECTRIFIED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateElectrified
//=============================================================================
class CStateElectrified : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Electrified)

public:
	~CStateElectrified()	{}
	CStateElectrified();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEELECTRIFIED_H__
