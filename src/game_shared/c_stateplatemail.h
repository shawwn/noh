// (C)2006 S2 Games
// c_stateplatemail.h
//
//=============================================================================
#ifndef __C_STATEPLATEMAIL_H__
#define __C_STATEPLATEMAIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStatePlatemail
//=============================================================================
class CStatePlatemail : public IEntityState
{
private:
	START_ENTITY_CONFIG(IEntityState)
		DECLARE_ENTITY_CVAR(float, ArmorAdd)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(State, Platemail);

public:
	~CStatePlatemail()	{}
	CStatePlatemail();
};
//=============================================================================

#endif //__C_STATEPLATEMAIL_H__
