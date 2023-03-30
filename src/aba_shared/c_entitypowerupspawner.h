// (C)2008 S2 Games
// c_entitypowerupspawner.h
//
//=============================================================================
#ifndef __C_ENTITYPOWERUPSPAWNER_H__
#define __C_ENTITYPOWERUPSPAWNER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEntityPowerupSpawner
//=============================================================================
class CEntityPowerupSpawner : public IVisualEntity
{
protected:
	DECLARE_ENT_ALLOCATOR2(Entity, PowerupSpawner);

public:
	~CEntityPowerupSpawner()	{}
	CEntityPowerupSpawner();

	virtual bool		IsServerEntity() const			{ return true; }

	void	ApplyWorldEntity(const CWorldEntity &ent);

	void	Spawn();
};
//=============================================================================

#endif //__C_ENTITYPOWERUPSPAWNER_H__
