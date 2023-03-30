// (C)2008 S2 Games
// c_triggerspawnpoint.h
//
//=============================================================================
#ifndef __C_TRIGGERSPAWNPOINT_H__
#define __C_TRIGGERSPAWNPOINT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CTriggerSpawnPoint
//=============================================================================
class CTriggerSpawnPoint : public IVisualEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Trigger, SpawnPoint);

public:
	~CTriggerSpawnPoint()	{}
	CTriggerSpawnPoint()	{}

	virtual bool		IsServerEntity() const			{ return true; }

	virtual void		ApplyWorldEntity(const CWorldEntity &ent);
};
//=============================================================================

#endif //__C_TRIGGERREFPOINT_H__
