// (C)2006 S2 Games
// c_statestealth.h
//
//=============================================================================
#ifndef __C_STATESTEALTH_H__
#define __C_STATESTEALTH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateStealth
//=============================================================================
class CStateStealth : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Stealth);

public:
	~CStateStealth()	{}
	CStateStealth();

	virtual void	DoAttack(CMeleeAttackEvent &attack)	{ Invalidate(); }
	virtual void	DoRangedAttack()					{ Invalidate(); }
	virtual float	OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker) { Invalidate(); return fDamage; }
};
//=============================================================================

#endif //__C_STATEMORALE_H__
