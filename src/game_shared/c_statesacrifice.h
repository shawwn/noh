// (C)2007 S2 Games
// c_statesacrifice.h
//
//=============================================================================
#ifndef __C_STATESACRIFICE_H__
#define __C_STATESACRIFICE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateSacrifice
//=============================================================================
class CStateSacrifice : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, BlastRadius)
		DECLARE_ENTITY_CVAR(float, BlastDamage)
		DECLARE_ENTITY_CVAR(tstring, ExpiredEffectPath)
		DECLARE_ENTITY_CVAR(float, SpeedMult)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
		DECLARE_ENTITY_CVAR(float, StaminaRegenMult)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Sacrifice)

public:
	~CStateSacrifice()	{}
	CStateSacrifice();

	void	Activated();
	void	Expired();

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void 	ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__C_STATESACRIFICE_H__
