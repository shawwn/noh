// (C)2006 S2 Games
// c_statechainmail.h
//
//=============================================================================
#ifndef __C_STATECHAINMAIL_H__
#define __C_STATECHAINMAIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateChainmail
//=============================================================================
class CStateChainmail : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Chainmail);

public:
	~CStateChainmail()	{}
	CStateChainmail();
};
//=============================================================================

#endif //__C_STATECHAINMAIL_H__
