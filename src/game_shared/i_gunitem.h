// (C)2006 S2 Games
// i_gunitem.h
//
//=============================================================================
#ifndef __I_GUNITEM_H__
#define __I_GUNITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IGunItem
//=============================================================================
class IGunItem : public IInventoryItem
{
protected:
	START_ENTITY_CONFIG(IInventoryItem)
		DECLARE_ENTITY_CVAR(float, MinDamage)
		DECLARE_ENTITY_CVAR(float, MaxDamage)
		DECLARE_ENTITY_CVAR(float, PierceUnit)
		DECLARE_ENTITY_CVAR(float, PierceHellbourne)
		DECLARE_ENTITY_CVAR(float, PierceSiege)
		DECLARE_ENTITY_CVAR(float, PierceBuilding)
		DECLARE_ENTITY_CVAR(float, DamageRadius)
		DECLARE_ENTITY_CVAR(uint, SpinupTime)
		DECLARE_ENTITY_CVAR(uint, ChargeTime)
		DECLARE_ENTITY_CVAR(uint, MinChargeTime)
		DECLARE_ENTITY_CVAR(uint, AttackTime)
		DECLARE_ENTITY_CVAR(tstring, FirstPersonModelPath)
		DECLARE_ENTITY_CVAR(CVec3f, FirstPersonModelOffset)
		DECLARE_ENTITY_CVAR(CVec3f, FirstPersonModelAngles)
		DECLARE_ENTITY_CVAR(float, FirstPersonModelFov)
		DECLARE_ENTITY_CVAR(tstring, ProjectileName)
		DECLARE_ENTITY_CVAR(tstring, ImpactEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ImpactBuildingEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ImpactTerrainEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ThirdPersonFireAnimName)
		DECLARE_ENTITY_CVAR(tstring, ThirdPersonFireEffectPath)
		DECLARE_ENTITY_CVAR(tstring, TraceEffectPath)
		DECLARE_ENTITY_CVAR(uint, NumShots)
		DECLARE_ENTITY_CVAR(float, SpreadX)
		DECLARE_ENTITY_CVAR(float, SpreadY)
		DECLARE_ENTITY_CVAR(float, Range)
		DECLARE_ENTITY_CVAR(CVec3f, AttackOffset)
		DECLARE_ENTITY_CVAR(tstring, TargetState)
		DECLARE_ENTITY_CVAR(uint, TargetStateDuration)
		DECLARE_ENTITY_CVAR(bool, CanZoom)
		DECLARE_ENTITY_CVAR(float, ZoomFov)
		DECLARE_ENTITY_CVAR(uint, ZoomTime)
		DECLARE_ENTITY_CVAR(float, ViewDriftX)
		DECLARE_ENTITY_CVAR(float, ViewDriftY)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;


	uint		m_uiChargeStartTime;
	ResHandle	m_hFirstPersonModel;

	bool		m_bIsZooming;
	uint		m_uiZoomStartTime;
	CVec2f		m_v2Drift;

	virtual IProjectile*	FireProjectile(const CVec3f &v3Origin, const CVec3f &v3Dir, float fCharge);

public:
	~IGunItem()	{}
	IGunItem(CEntityConfig *pConfig);

	bool			IsGun() const			{ return true; }

	virtual bool	IsFirstPerson() const	{ return true; }
	
	virtual void	Selected();

	virtual bool	Spinup(int iButtonStatus);
	virtual bool	Charge(int iButtonStatus);
	virtual bool	Fire(int iButtonStatus);
	virtual bool	CoolDown();

	virtual bool	ActivatePrimary(int iButtonStatus);
	virtual bool	ActivateSecondary(int iButtonStatus);

	virtual void	FinishedAction(int iAction);

	virtual void	Spawn();
	virtual void	LocalClientFrame();

	virtual float	GetFov() const;
	virtual void	ApplyDrift(CVec3f &v3Angles);

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	virtual void	DoRangedAttack();

	TYPE_NAME("Gun")

	// Settings
	ENTITY_CVAR_ACCESSOR(float, MinDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxDamage, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, PierceUnit, 1.0f)
	ENTITY_CVAR_ACCESSOR(float, PierceHellbourne, 1.0f)
	ENTITY_CVAR_ACCESSOR(float, PierceSiege, 1.0f)
	ENTITY_CVAR_ACCESSOR(float, PierceBuilding, 1.0f)
	ENTITY_CVAR_ACCESSOR(float, DamageRadius, 0.0f)
	ENTITY_CVAR_ACCESSOR(uint, SpinupTime, 0)
	ENTITY_CVAR_ACCESSOR(uint, ChargeTime, 0)
	ENTITY_CVAR_ACCESSOR(uint, MinChargeTime, 0)
	ENTITY_CVAR_ACCESSOR(uint, AttackTime, 0)
	ENTITY_CVAR_ACCESSOR(tstring, FirstPersonModelPath, _T(""))
	ENTITY_CVAR_ACCESSOR(CVec3f, FirstPersonModelOffset, V3_ZERO)
	ENTITY_CVAR_ACCESSOR(CVec3f, FirstPersonModelAngles, V3_ZERO)
	ENTITY_CVAR_ACCESSOR(float, FirstPersonModelFov, 60.f)
	ENTITY_CVAR_ACCESSOR(tstring, ProjectileName, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ImpactEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ImpactBuildingEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ImpactTerrainEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ThirdPersonFireAnimName, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, ThirdPersonFireEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, TraceEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(uint, NumShots, 1)
	ENTITY_CVAR_ACCESSOR(float, SpreadX, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, SpreadY, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, Range, 10000.0f)
	ENTITY_CVAR_ACCESSOR(CVec3f, AttackOffset, V3_ZERO)
};
//=============================================================================

#endif //__I_GUNITEM_H__
