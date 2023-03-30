// (C)2006 S2 Games
// c_statewillofgod.h
//
//=============================================================================
#ifndef __C_STATEWILLOFGOD_H__
#define __C_STATEWILLOFGOD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateWillOfGod
//=============================================================================
class CStateWillOfGod : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorMult)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, WillOfGod);

public:
	~CStateWillOfGod()	{}
	CStateWillOfGod();
};
//=============================================================================

#endif //__C_STATEWILLOFGOD_H__
