// (C)2006 S2 Games
// i_projectile.h
//
//=============================================================================
#ifndef __I_PROJECTILE_H__
#define __I_PROJECTILE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
struct STraceInfo;
//=============================================================================

//=============================================================================
// IProjectile
//=============================================================================
class IProjectile : public IVisualEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	static uint		s_uiStartTime;
	static uint		s_uiFrameTime;
	static uint		s_uiElapsed;

	ushort			m_unWeaponOriginID;
	uint			m_uiDirectDamageEntityIndex;
	uint			m_uiLastBounceTime;

	IProjectile();

protected:
	START_ENTITY_CONFIG(IVisualEntity)
		DECLARE_ENTITY_CVAR(float, Speed)
		DECLARE_ENTITY_CVAR(uint, LifeTime)
		DECLARE_ENTITY_CVAR(float, Gravity)
		DECLARE_ENTITY_CVAR(float, Bounce)
		DECLARE_ENTITY_CVAR(float, Friction)
		DECLARE_ENTITY_CVAR(tstring, TrailEffectPath)
		DECLARE_ENTITY_CVAR(tstring, DeathEffectPath)
		DECLARE_ENTITY_CVAR(tstring, BounceEffectPath)
		DECLARE_ENTITY_CVAR(bool, Stick)
		DECLARE_ENTITY_CVAR(float, MinStickDistance)
		DECLARE_ENTITY_CVAR(float, MaxStickDistance)
		DECLARE_ENTITY_CVAR(bool, Turn)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	uint			m_uiOriginTime;
	uint			m_uiLastUpdateTime;
	CVec3f			m_v3Origin;
	uint			m_uiOwnerIndex;
	IGameEntity*	m_pImpactEntity;
	float			m_fMinDamage;
	float			m_fMaxDamage;
	float			m_fUnitPierce;
	float			m_fHellbournePierce;
	float			m_fSiegePierce;
	float			m_fBuildingPierce;
	float			m_fDamageRadius;
	ushort			m_unTargetState;
	uint			m_uiTargetStateDuration;
	bool			m_bStuck;
	uint			m_uiStickTime;
	int				m_iDamageFlags;

public:
	virtual ~IProjectile()	{}
	IProjectile(CEntityConfig *pConfig);

	bool						IsProjectile() const			{ return true; }

	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();
	
	virtual void				Baseline();
	virtual void				GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool				ReadSnapshot(CEntitySnapshot &snapshot);

	void						SetOrigin(const CVec3f &v3)		{ m_v3Origin = v3; }
	CVec3f						GetOrigin() const				{ return m_v3Origin; }

	void						SetWeaponOrigin(ushort unID)	{ m_unWeaponOriginID = unID; }

	void						SetOwner(uint uiIndex)			{ m_uiOwnerIndex = uiIndex; }
	uint						GetOwner() const				{ return m_uiOwnerIndex; }

	GAME_SHARED_API void		CalculatePosition(uint uiTime);

	GAME_SHARED_API CSkeleton*	AllocateSkeleton();

	bool						Bounce(const STraceInfo &trace, bool bCheckDamage);
	bool						EvaluateTrajectory();

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }

	virtual void				Spawn();
	virtual void				ApplyCharge(float fValue)		{}
	virtual bool				ServerFrame();
	virtual void				DamageRadius();
	virtual void				Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
	virtual void				Killed()						{};

	virtual void				Copy(const IGameEntity &B);

	static void					ClientPrecache(CEntityConfig *pConfig);
	static void					ServerPrecache(CEntityConfig *pConfig);

	void						SetDamageFlags(int iFlags)		{ m_iDamageFlags = iFlags; }
	void						AddDamageFlags(int iFlags)		{ m_iDamageFlags |= iFlags; }
	virtual int					GetDamageFlags() const			{ return m_iDamageFlags | DAMAGE_FLAG_PROJECTILE; }

	uint						GetOriginTime() const				{ return m_uiOriginTime; }
	void						SetOriginTime(uint uiOriginTime)	{ m_uiOriginTime = uiOriginTime; }
	
	void						SetDamage(float fMinDamage, float fMaxDamage, float fDamageRadius, float fUnitPierce, float fHellbournePierce, float fSiegePierce, float fBuildingPierce)
	{
		m_fMinDamage = fMinDamage;
		m_fMaxDamage = fMaxDamage;
		m_fDamageRadius = fDamageRadius;
		m_fUnitPierce = fUnitPierce;
		m_fHellbournePierce = fHellbournePierce;
		m_fSiegePierce = fSiegePierce;
		m_fBuildingPierce = fBuildingPierce;
	}

	void						SetTargetState(ushort unState)			{ m_unTargetState = unState; }
	void						SetTargetStateDuration(uint uiDuration)	{ m_uiTargetStateDuration = uiDuration; }

	virtual void				Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);

	// Setting
	ENTITY_CVAR_ACCESSOR(float, Speed, 500.0f)
	ENTITY_CVAR_ACCESSOR(uint, LifeTime, 5000)
	ENTITY_CVAR_ACCESSOR(float, Gravity, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, Bounce, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, Friction, 0.0f)
	ENTITY_CVAR_ACCESSOR(tstring, TrailEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, DeathEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(tstring, BounceEffectPath, _T(""))
	ENTITY_CVAR_ACCESSOR(bool, Stick, false)
	ENTITY_CVAR_ACCESSOR(float, MinStickDistance, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, MaxStickDistance, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, Turn, false)
};
//=============================================================================

#endif //__I_PROJECTILE_H__
