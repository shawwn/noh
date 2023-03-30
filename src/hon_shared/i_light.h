// (C)2006 S2 Games
// i_light.h
//
//=============================================================================
#ifndef __I_LIGHT_H__
#define __I_LIGHT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CWorldLight;
//=============================================================================

//=============================================================================
// ILight
//=============================================================================
class ILight : public IVisualEntity
{
	DECLARE_ENTITY_DESC

private:
	CVec3f	m_v3Color;
	float	m_fFalloffStart;
	float	m_fFalloffEnd;

public:
	~ILight()	{}
	ILight();

	SUB_ENTITY_ACCESSOR(ILight, Light)

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	GAME_SHARED_API void		Spawn();

	virtual bool	AddToScene(const CVec4f &v4Color, int iFlags);

	void			Copy(const IGameEntity &B);

	virtual bool	IsVisibleOnMap(CPlayer *pLocalPlayer) const		{ return false; }
};
//=============================================================================

#endif //__I_LIGHT_H__
