// (C)2008 S2 Games
// i_petentity.h
//
//=============================================================================
#ifndef __I_PETENTITY_H__
#define __I_PETENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_petdefinition.h"
//=============================================================================

//=============================================================================
// IPetEntity
//=============================================================================
class IPetEntity : public IUnitEntity
{
public:
	typedef CPetDefinition TDefinition;

private:

public:
	virtual ~IPetEntity()	{}
	IPetEntity()			{}

	SUB_ENTITY_ACCESSOR(IPetEntity, Pet)

	virtual void	Spawn();
	virtual void	UpdateModifiers();
	virtual bool	ServerFrameThink();
	virtual bool	ServerFrameCleanup();
	virtual void	Die(IUnitEntity *pAttacker = NULL, ushort unKillingObject = INVALID_ENT_TYPE);

	virtual void	SetLevel(uint uiLevel);

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IsPersistent)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, Lifetime)
};
//=============================================================================

#endif //__I_PETENTITY_H__
