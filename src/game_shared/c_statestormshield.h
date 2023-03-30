// (C)2007 S2 Games
// c_statestormshield.h
//
//=============================================================================
#ifndef __C_STATESTORMSHIELD_H__
#define __C_STATESTORMSHIELD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateStormShield
//=============================================================================
class CStateStormShield : public IEntityState
{
private:
	DECLARE_ENT_ALLOCATOR2(State, StormShield);

public:
	~CStateStormShield()	{}
	CStateStormShield() :
	IEntityState(GetEntityConfig())
	{}

	float	OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker);
};
//=============================================================================

#endif //__C_STATESTORMSHIELD_H__
