// (C)2006 S2 Games
// c_propscenery.h
//
//=============================================================================
#ifndef __C_PROPSCENERY_H__
#define __C_PROPSCENERY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
#include "../k2/c_sceneentity.h"
//=============================================================================

//=============================================================================
// CPropScenery
//=============================================================================
class CPropScenery : public IPropEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Prop, Scenery);

	CSceneEntity		m_cSceneEntity;

public:
	~CPropScenery()	{}
	CPropScenery() :
	IPropEntity(GetEntityConfig())
	{}

	virtual bool		IsStatic() const				{ return true; }

	virtual void		Spawn();

	virtual bool		ServerFrame()					{ return true; }

	virtual void		DrawOnMap(class CUITrigger &minimap, IPlayerEntity *pLocalPlayer, bool bLargeMap) {}
	virtual bool		IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap)	{ return false; }

	virtual	void		UpdateSighting(const vector<IVisualEntity *> &vVision)		{}

	virtual bool		AddToScene(const CVec4f &v4Color, int iFlags);
	
	virtual void		Copy(const IGameEntity &B);
};
//=============================================================================

#endif //__C_PROPSCENERY_H__
