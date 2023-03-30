// (C)2008 S2 Games
// i_bitentity.h
//
//=============================================================================
#ifndef __I_BITENTITY_H__
#define __I_BITENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldEntity;
//=============================================================================

//=============================================================================
// IBitEntity
//=============================================================================
class IBitEntity : public IUnitEntity
{
	DECLARE_ENTITY_DESC

protected:
	START_ENTITY_CONFIG(IUnitEntity)
		DECLARE_ENTITY_CVAR(float, BoundsRadius)
		DECLARE_ENTITY_CVAR(float, BoundsHeight)
		DECLARE_ENTITY_CVAR(float, OcclusionRadius)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	uint				m_uiBitIndex;

public:
	virtual ~IBitEntity();
	IBitEntity(CEntityConfig *pConfig);

	SUB_ENTITY_ACCESSOR(IBitEntity, Bit)

	// Settings
	ENTITY_CVAR_ACCESSOR(float, BoundsRadius)
	ENTITY_CVAR_ACCESSOR(float, BoundsHeight)
	ENTITY_CVAR_ACCESSOR(float, OcclusionRadius)

	virtual void	Baseline()										{}
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const	{}
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)			{ return true; }

	uint			GetBitIndex() const				{ return m_uiBitIndex; }
	void			SetBitIndex(uint uiIndex)		{ m_uiBitIndex = uiIndex; }

	virtual void	ApplyWorldEntity(const CWorldEntity &ent);

	virtual void	Spawn();
	virtual void	Die(IUnitEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

	virtual bool	ServerFrameSetup()				{ return true; }
	virtual bool	ServerFrameThink()				{ return true; }
	virtual bool	ServerFrameMovement()			{ return true; }
	virtual bool	ServerFrameAction()				{ return true; }
	virtual bool	ServerFrameCleanup()			{ return true; }

	virtual void	Activate()						{}
	virtual void	Deactivate()					{}

	virtual bool	IsVisibleOnMap(CPlayer *pLocalPlayer) const	{ return false; }

	virtual ResHandle	GetModel() const;
	virtual float		GetBaseScale() const		{ return 1.0f; }

	virtual void			Link();
	virtual void			Unlink();
};
//=============================================================================

#endif //__I_BITENTITY_H__
