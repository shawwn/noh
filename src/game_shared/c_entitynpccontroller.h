// (C)2007 S2 Games
// c_entitynpccontroller.h
//
//=============================================================================
#ifndef __C_ENTITYNPCCONTROLLER_H__
#define __C_ENTITYNPCCONTROLLER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class INpcEntity;
//=============================================================================

//=============================================================================
// CEntityNpcController
//=============================================================================
class CEntityNpcController : public IVisualEntity
{
private:
	struct SNpcSpawnState
	{
		ushort		unType;
		CVec3f		v3Pos;
		CVec3f		v3Angles;
		ResHandle	hDefinition;
	};

protected:
	START_ENTITY_CONFIG(IVisualEntity)
		DECLARE_ENTITY_CVAR(CVec3f, MinimapColor)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	DECLARE_ENT_ALLOCATOR2(Entity, NpcController);

	// Settings
	uint						m_uiSpawnPeriod;
	float						m_fSpawnRadius;

	uint						m_uiNextRespawnCheckTime;
	vector<SNpcSpawnState>		m_vNpcSpawnStates;
	uivector					m_vChildren;

	bool						m_bDistrubed;

	void						DeleteChildren();
	void						SpawnChildren();

public:
	~CEntityNpcController()	{}
	CEntityNpcController();

	void	ApplyWorldEntity(const CWorldEntity &ent);

	void	Spawn();
	bool	ServerFrame();

	virtual CVec4f	GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap)	{ return CVec4f(GetMinimapColor(), 1.0f); }
	virtual	void	UpdateSighting(const vector<IVisualEntity *> &vVision)	{ SetSightedPos(GetPosition()); IVisualEntity::UpdateSighting(vVision); }
	virtual bool	IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap)	{ return !m_bSighted; }

	void	AddNpc(INpcEntity *pNpc);

	bool	AIShouldTarget()												{ return false; }

	void	SetDistrubed(bool bDistrubed)		{ m_bDistrubed = bDistrubed; }

	ENTITY_CVAR_ACCESSOR(CVec3f, MinimapColor, CVec3f(1.0f, 1.0f, 1.0f));
};
//=============================================================================

#endif //__C_ENTITYNPCCONTROLLER_H__
