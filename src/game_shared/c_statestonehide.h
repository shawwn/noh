// (C)2006 S2 Games
// c_statestonehide.h
//
//=============================================================================
#ifndef __C_STATESTONEHIDE_H__
#define __C_STATESTONEHIDE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateStoneHide
//=============================================================================
class CStateStoneHide : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, StoneHide);

public:
	~CStateStoneHide()	{}
	CStateStoneHide();
};
//=============================================================================

#endif //__C_STATESTONEHIDE_H__
