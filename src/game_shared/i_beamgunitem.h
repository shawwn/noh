// (C)2007 S2 Games
// i_gunitem.h
//
//=============================================================================
#ifndef __I_BEAMGUNITEM_H__
#define __I_BEAMGUNITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IBeamGunItem
//=============================================================================
class IBeamGunItem : public IGunItem
{
protected:
	START_ENTITY_CONFIG(IGunItem)
		DECLARE_ENTITY_CVAR(float, ManaCostPerSecondHit)
		DECLARE_ENTITY_CVAR(float, ManaCostPerSecondMiss)
		DECLARE_ENTITY_CVAR(tstring, ThirdPersonHitEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ThirdPersonMissEffectPath)
		DECLARE_ENTITY_CVAR(tstring, FirstPersonHitEffectPath)
		DECLARE_ENTITY_CVAR(tstring, FirstPersonMissEffectPath)
		DECLARE_ENTITY_CVAR(bool, UseHitEffect)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	uint		m_uiEffectIndex;

	void		StopFire();

public:
	~IBeamGunItem();
	IBeamGunItem(CEntityConfig *pConfig);

	virtual void	Unselected();

	virtual bool	Fire(int iButtonStatus);

	virtual bool	ActivatePrimary(int iButtonStatus);

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	// Settings
	ENTITY_CVAR_ACCESSOR(float, ManaCostPerSecondHit, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, ManaCostPerSecondMiss, 0.0f)
};
//=============================================================================

#endif //__I_BEAMGUNITEM_H__
