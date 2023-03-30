// (C)2006 S2 Games
// c_staterot.h
//
//=============================================================================
#ifndef __C_STATEROT_H__
#define __C_STATEROT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateRot
//=============================================================================
class CStateRot : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, DamagePerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Rot);

public:
	~CStateRot()	{}
	CStateRot();

	void	StateFrame();
};
//=============================================================================

#endif //__C_STATEROT_H__
