// (C)2008 S2 Games
// i_powerupentity.h
//
//=============================================================================
#ifndef __I_POWERUPENTITY_H__
#define __I_POWERUPENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_powerupdefinition.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IPowerupEntity
//=============================================================================
class IPowerupEntity : public IUnitEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef CPowerupDefinition TDefinition;
	
protected:

public:
	virtual ~IPowerupEntity()	{}
	IPowerupEntity();

	SUB_ENTITY_ACCESSOR(IPowerupEntity, Powerup)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TouchTargetScheme)

	bool				IsVisibleOnMap(CPlayer *pLocalPlayer) const	{ return false; }

	virtual void		Baseline();
	virtual void		GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool		ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
	virtual void		Copy(const IGameEntity &B);

	static void			ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
	static void			ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

	virtual void		Spawn();
	virtual void		Touch(IGameEntity *pActivator);
};
//=============================================================================

#endif //__I_POWERUPENTITY_H__
