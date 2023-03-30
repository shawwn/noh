// (C)2007 S2 Games
// c_triggerspawn.h
//
//=============================================================================
#ifndef __C_TRIGGERSPAWN_H__
#define __C_TRIGGERSPAWN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_triggerentity.h"
//=============================================================================

//=============================================================================
// CTriggerSpawn
//=============================================================================
class CTriggerSpawn : public ITriggerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Trigger, Spawn);

	tstring				m_sSpawnType;

	uint				m_uiSpawnDelay;
	uint				m_uiLastSpawn;

	ushort				m_unSpawnType;

	uint				m_uiTotalSpawned;

public:
	~CTriggerSpawn()	{}
	CTriggerSpawn() :
	ITriggerEntity(GetEntityConfig()),
	m_uiSpawnDelay(-1),
	m_uiLastSpawn(-1),
	m_unSpawnType(INVALID_ENT_TYPE),
	m_uiTotalSpawned(0)
	{}

	virtual void			ApplyWorldEntity(const CWorldEntity &ent);
	virtual void			RegisterEntityScripts(const CWorldEntity &ent);

	virtual void			Copy(const IGameEntity &B);

	GAME_SHARED_API bool	ServerFrame();

	const bool				IsSpawnTrigger() const		{ return true; }

	GAME_SHARED_API	virtual void		Trigger(uint uiTriggeringEntIndex, const tstring &sTrigger, bool bPlayEffect = true);
};
//=============================================================================

#endif //__C_TRIGGERSPAWN_H__
