// (C)2007 S2 Games
// c_entitysoul.h
//
//=============================================================================
#ifndef __C_ENTITYSOUL_H__
#define __C_ENTITYSOUL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CEntitySoul
//=============================================================================
class CEntitySoul : public IVisualEntity
{
private:
	static vector<SDataField>	*s_pvFields;

protected:
	START_ENTITY_CONFIG(IVisualEntity)
		DECLARE_ENTITY_CVAR(uint, SpawnDelay)
		DECLARE_ENTITY_CVAR(float, InitialSpeed)
		DECLARE_ENTITY_CVAR(float, InitialSpeedDistanceFactor)
		DECLARE_ENTITY_CVAR(uint, LerpTime)
		DECLARE_ENTITY_CVAR(tstring, TrailEffectPath)
		DECLARE_ENTITY_CVAR(tstring, EndEffectPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Entity, Soul);

	uint				m_uiSpawnTime;
	uint				m_uiLastUpdateTime;
	float				m_fInitialSpeed;
	uint				m_uiTarget;
	CVec3f				m_v3Origin;
	bool				m_bStarted;

public:
	~CEntitySoul()	{}
	CEntitySoul();

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();
	
	virtual void				Baseline();
	virtual void				GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool				ReadSnapshot(CEntitySnapshot &snapshot);

	virtual void				Spawn();
	virtual bool				ServerFrame();

	static void					ClientPrecache(CEntityConfig *pConfig);
	static void					ServerPrecache(CEntityConfig *pConfig);

	void			SetTarget(uint uiTarget)		{ m_uiTarget = uiTarget; }

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }
};
//=============================================================================

#endif //__C_ENTITYSOUL_H__
