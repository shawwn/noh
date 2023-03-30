// (C)2007 S2 Games
// c_statecheetah.h
//
//=============================================================================
#ifndef __C_STATECHEETAH_H__
#define __C_STATECHEETAH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateCheetah
//=============================================================================
class CStateCheetah : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Cheetah)
	
public:
	~CStateCheetah()	{}
	CStateCheetah();
};
//=============================================================================

#endif //__C_STATECHEETAH_H__
