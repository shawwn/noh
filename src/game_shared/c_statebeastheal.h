// (C)2007 S2 Games
// c_statebeastheal.h
//
//=============================================================================
#ifndef __C_STATEBEASTHEAL_H__
#define __C_STATEBEASTHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateBeastHeal
//=============================================================================
class CStateBeastHeal : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, HealthPerSecond)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, BeastHeal);

	float	m_fTotalHealed;

public:
	~CStateBeastHeal()	{}
	CStateBeastHeal();

	void	StateFrame();
	void	Expired();
};
//=============================================================================

#endif //__C_STATEBEASTHEAL_H__
