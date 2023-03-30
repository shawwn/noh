// (C)2007 S2 Games
// c_stateshieldtower.h
//
//=============================================================================
#ifndef __C_STATESHIELDTOWER_H__
#define __C_STATESHIELDTOWER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// CStateShieldTower
//=============================================================================
class CStateShieldTower : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorBuff)
		DECLARE_ENTITY_CVAR(float, DamageMultiplier)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, ShieldTower);

public:
	~CStateShieldTower()	{}
	CStateShieldTower();
	virtual float     OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker) { if (GetIsInvulnerable()) return 0; return (fDamage * m_pEntityConfig->GetDamageMultiplier()); }

	
};
//=============================================================================

#endif //__C_STATESHIELDTOWER_H__
