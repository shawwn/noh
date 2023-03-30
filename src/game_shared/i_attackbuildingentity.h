// (C)2007 S2 Games
// i_attackbuildingentity.h
//
//=============================================================================
#ifndef __I_ATTACKBUILDINGENTITY_H__
#define __I_ATTACKBUILDINGENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_buildingentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IAttackBuildingEntity
//=============================================================================
class IAttackBuildingEntity : public IBuildingEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	IAttackBuildingEntity();

protected:
	START_ENTITY_CONFIG(IBuildingEntity)
		DECLARE_ENTITY_CVAR(tstring, Projectile)
		DECLARE_ENTITY_CVAR(uint, AttackTime)
		DECLARE_ENTITY_CVAR(uint, AttackTimeVariance)
		DECLARE_ENTITY_CVAR(float, Range)
		DECLARE_ENTITY_CVAR(float, MinDamage)
		DECLARE_ENTITY_CVAR(float, MaxDamage)
		DECLARE_ENTITY_CVAR(float, PierceUnit)
		DECLARE_ENTITY_CVAR(float, PierceHellbourne)
		DECLARE_ENTITY_CVAR(float, PierceSiege)
		DECLARE_ENTITY_CVAR(float, PierceBuilding)
		DECLARE_ENTITY_CVAR(float, DamageRadius)
		DECLARE_ENTITY_CVAR(float, SpreadX)
		DECLARE_ENTITY_CVAR(float, SpreadY)
		DECLARE_ENTITY_CVAR(CVec3f, AttackOffset)
		DECLARE_ENTITY_CVAR(tstring, TargetState)
		DECLARE_ENTITY_CVAR(uint, TargetStateDuration)
		DECLARE_ENTITY_CVAR(tstring, TraceEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ImpactTerrainEffectPath)
		DECLARE_ENTITY_CVAR(tstring, ImpactEffectPath)
		DECLARE_ENTITY_CVAR(tstring, AttackEffectPath)
		DECLARE_ENTITY_CVAR(float, SiegeTargetPreference)
		DECLARE_ENTITY_CVAR(bool, AlwaysTargetNpcs)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	uint				m_uiTarget;
	uint				m_uiNextAttackTime;
	map<uint, float>	m_mapRecentDamage;
	uint				m_uiNextSightTime;

	bool	ShouldTarget(IGameEntity *pOther);
	void	UpdateTargetPreference();

	void	FireProjectile(const tstring &sName, const CVec3f &v3Start, const CVec3f &v3End, const CVec3f &v3Dir, const CVec3f &v3TargetVelocity);
	void	FireTrace(const CVec3f &v3Start, const CVec3f &v3End);

public:
	virtual ~IAttackBuildingEntity();
	IAttackBuildingEntity(CEntityConfig *pConfig);

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();

	virtual void		Spawn();
	virtual float		Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);

	virtual void		DamageNotification(uint uiIndex, uint uiAttacker, float fDamage);

	virtual bool		ServerFrame();

	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__I_BUILDINGENTITY_H__
