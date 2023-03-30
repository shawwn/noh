// (C)2007 S2 Games
// c_statepetheal.h
//
//=============================================================================
#ifndef __C_STATEPETHEAL_H__
#define __C_STATEPETHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStatePetHeal
//=============================================================================
class CStatePetHeal : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, PetHeal)

public:
	~CStatePetHeal()	{}
	CStatePetHeal();
};
//=============================================================================

#endif //__C_STATEPETHEAL_H__
