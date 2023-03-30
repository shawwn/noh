// (C)2008 S2 Games
// c_entityneutralcampcontroller.h
//
//=============================================================================
#ifndef __C_ENTITYNEUTRALCAMPCONTROLLER_H__
#define __C_ENTITYNEUTRALCAMPCONTROLLER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MAX_CAMP_OCCUPANTS(5);

typedef vector<tstring>					NeutralSpawnerList;
typedef NeutralSpawnerList::iterator	NeutralSpawnerList_it;
typedef vector<NeutralSpawnerList>		NeutralSpawnerArray;
typedef NeutralSpawnerArray::iterator	NeutralSpawnerArray_it;

typedef vector<uint>						NeutralSpawnerUIDList;
typedef NeutralSpawnerUIDList::iterator		NeutralSpawnerUIDList_it;
typedef vector<NeutralSpawnerUIDList>		NeutralSpawnerUIDArray;
typedef NeutralSpawnerUIDArray::iterator	NeutralSpawnerUIDArray_it;

typedef list<uint>						NeutralCreepStack;
typedef list<NeutralCreepStack>			NeutralCreepStackList;

EXTERN_CVAR_FLOAT(g_neutralCampMapIconSize);
//=============================================================================

//=============================================================================
// CEntityNeutralCampController
//=============================================================================
class CEntityNeutralCampController : public IVisualEntity
{
	DECLARE_ENTITY_DESC

private:
	static ResHandle	s_hMinimapIcon;

protected:
	DECLARE_ENT_ALLOCATOR2(Entity, NeutralCampController);

	uint					m_uiLevel;
	NeutralSpawnerArray		m_vSpawnerArray;
	NeutralSpawnerUIDArray	m_vSpawnerUIDArray;
	NeutralCreepStackList	m_vCreepStacks;
	uint					m_uiMaxCreepStacks;
	uint					m_uiNoRespawnRadius;
	uint					m_uiRespawnInterval;
	uint					m_uiNumActive;
	uint					m_uiNextSpawnTime;

	uint					m_uiGuardChaseTime;
	uint					m_uiGuardChaseDistance;
	uint					m_uiGuardReaggroChaseTime;
	uint					m_uiGuardReaggroChaseDistance;
	uint					m_uiOverrideAggroRange;

	bool					m_bInitialSpawnAttempted;

public:
	~CEntityNeutralCampController()	{}
	CEntityNeutralCampController();

	static void				ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
	static void				ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
	
	virtual void			Baseline();
	virtual void			GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool			ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
	virtual void			Copy(const IGameEntity &B);

	virtual void			ApplyWorldEntity(const CWorldEntity &ent);

	virtual void			Spawn();
	virtual bool			ServerFrameThink();
	
	virtual void			GameStart();

	GAME_SHARED_API bool	AttemptSpawn();

	void					CreepDied(uint iCreepID);

	uint					NumCreepStacks() const							{ return (uint)m_vCreepStacks.size(); }

	bool					GetActive()										{ return m_uiNumActive > 0; }

	virtual CVec4f			GetMapIconColor(CPlayer *pLocalPlayer) const;
	virtual float			GetMapIconSize(CPlayer *pLocalPlayer) const		{ return g_neutralCampMapIconSize; }
	virtual ResHandle		GetMapIcon(CPlayer *pLocalPlayer) const			{ return s_hMinimapIcon; }
	virtual bool			IsVisibleOnMap(CPlayer *pLocalPlayer) const		{ return pLocalPlayer ? !HasVisibilityFlags(VIS_SIGHTED(pLocalPlayer->GetTeam())) && !HasVisibilityFlags(VIS_REVEALED(pLocalPlayer->GetTeam())) : true; }
};
//=============================================================================

#endif //__C_ENTITYNEUTRALCAMPCONTROLLER_H__
