// (C)2006 S2 Games
// i_propentity.h
//
//=============================================================================
#ifndef __I_PROPENTITY_H__
#define __I_PROPENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CWorldEntity;
class CEntitySnapshot;
class IPropFoundation;
//=============================================================================

//=============================================================================
// IPropEntity
//=============================================================================
class IPropEntity : public IVisualEntity
{
protected:
	static vector<SDataField>	*s_pvFields;

	START_ENTITY_CONFIG(IVisualEntity)
		DECLARE_ENTITY_CVAR(CVec3f, MinimapColor)
		DECLARE_ENTITY_CVAR(tstring, SelectSoundPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	tstring	m_sSurfaceType;

public:
	virtual ~IPropEntity();
	IPropEntity(CEntityConfig *pConfig);

	bool							IsProp() const				{ return true; }
	virtual IPropFoundation*		GetAsFoundation()			{ return NULL; }
	virtual const IPropFoundation*	GetAsFoundation() const		{ return NULL; }

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();

	virtual void			Baseline();
	virtual void			GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool			ReadSnapshot(CEntitySnapshot &snapshot);
	virtual void			Copy(const IGameEntity &B);

	virtual ResHandle		GetModelHandle() const		{ return m_hModel; }
	virtual void			Spawn();

	virtual CSkeleton*		AllocateSkeleton();

	virtual void			ApplyWorldEntity(const CWorldEntity &ent);
	virtual void			Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

	virtual void			Link();
	virtual void			Unlink();

	// Settings
	ENTITY_CVAR_ACCESSOR(CVec3f, MinimapColor, CVec3f(1.0f, 1.0f, 1.0f));
	ENTITY_CVAR_ACCESSOR(tstring, SelectSoundPath, _T(""));

	static void				ClientPrecache(CEntityConfig *pConfig);

	virtual bool	AIShouldTarget()												{ return false; }

	virtual CVec4f	GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap)	{ if (bLargeMap) return WHITE; return CVec4f(GetMinimapColor(), 1.0f); }
	virtual	void	UpdateSighting(const vector<IVisualEntity *> &vVision)			{ SetSightedPos(GetPosition()); IVisualEntity::UpdateSighting(vVision); }

	void					SetSurfaceType(const tstring &sSurfaceType)	{ m_sSurfaceType = sSurfaceType; }
	const tstring&			GetSurfaceType() const						{ return m_sSurfaceType; }

	virtual CVec3f			GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds);
};
//=============================================================================

#endif //__I_PROPENTITY_H__
