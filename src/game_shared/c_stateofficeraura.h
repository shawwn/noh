// (C)2007 S2 Games
// c_stateofficeraura.h
//
//=============================================================================
#ifndef __C_STATEOFFICERAURA_H__
#define __C_STATEOFFICERAURA_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateOfficerAura
//=============================================================================
class CStateOfficerAura : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, HealthRegenMult)
		DECLARE_ENTITY_CVAR(float, ManaRegenMult)
		DECLARE_ENTITY_CVAR(float, StaminaRegenMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, OfficerAura);

public:
	~CStateOfficerAura()	{}
	CStateOfficerAura();
};
//=============================================================================

#endif //__C_STATEOFFICERAURA_H__
