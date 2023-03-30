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
#include "c_combatevent.h"
#include "c_projectiledefinition.h"
#include "c_entitydefinitionresource.h"
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
	DECLARE_ENTITY_DESC

public:
	typedef CProjectileDefinition TDefinition;

private:
	static uint		s_uiStartTime;
	static uint		s_uiFrameTime;
	static uint		s_uiElapsed;

protected:
	uint			m_uiLevel;
	uint			m_uiCharges;
	uint			m_uiCreationTime;
	uint			m_uiOriginTime;
	uint			m_uiLastUpdateTime;
	uint			m_uiOwnerIndex;
	bool			m_bReachedTarget;
	bool			m_bForceImpact;
	float			m_fDamageRadius;
	PoolHandle		m_hPath;

	uint			m_uiDirectDamageEntityIndex;
	uint			m_uiLastBounceTime;

	uint			m_uiTargetEntityUID;
	uint			m_uiTargetDisjointSequence;
	CVec3f			m_v3TargetPos;
	bool			m_bIgnoreTargetOffset;

	uint			m_uiTargetScheme;
	uint			m_uiEffectType;
	bool			m_bIgnoreInvulnerable;

	CCombatEvent	m_combat;

	uint			m_uiBounceCount;
	uint			m_uiReturnCount;
	uint			m_uiRedirectCount;
	uint			m_uiTotalTouchCount;
	map<uint, uint>	m_mapTouches;
	map<uint, uint>	m_mapImpacts;
	bool			m_bCliffTouched;

	ResHandle		m_hAttackImpactEffect;

	CVec3f			m_v3StartPosition;
	CVec2f			m_v2TravelPosition;
	CVec2f			m_v2CurveAxis;
	float			m_fCurveVelocity;
	float			m_fCurvePos;

	float			m_fParam;

	uint			m_uiProxyUID;
	
	void	FindPath();
	void	ApplyGravity(float fDeltaTime, float fTimeToGoal);
	CVec2f	ApplyCurvature(float fDeltaTime, float fTimeToGoal);
	void	TryTouch(float fTouchRadius, const CVec2f &v2Delta, float fDeltaTime);
	float	CalcDelta(float fDeltaTime, CVec2f &v2Delta);
	void	CalcPerpendicular(CVec2f &v2Perpendicular);

public:
	virtual ~IProjectile();
	IProjectile();

	SUB_ENTITY_ACCESSOR(IProjectile, Projectile)
	
	virtual void				Baseline();
	virtual void				GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool				ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	bool						IsActive() const								{ return true; }

	void						SetLevel(uint uiLevel)							{ m_uiLevel = uiLevel; }
	uint						GetLevel() const								{ return m_uiLevel; }

	virtual uint				GetCharges() const								{ return m_uiCharges; }
	virtual void				SetCharges(uint uiCharges)						{ m_uiCharges = uiCharges; }
	virtual void				RemoveCharge()									{ if (m_uiCharges > 0) --m_uiCharges; }
	virtual void				AddCharges(uint uiCharges)						{ if (GetMaxCharges() == 0) m_uiCharges += uiCharges; else m_uiCharges = MIN(m_uiCharges + uiCharges, GetMaxCharges()); }

	void						SetOwner(uint uiIndex)							{ m_uiOwnerIndex = uiIndex; }
	uint						GetOwnerIndex() const							{ return m_uiOwnerIndex; }
	IUnitEntity*				GetOwner() const								{ return Game.GetUnitEntity(GetOwnerIndex()); }

	uint						GetTargetScheme() const							{ return m_uiTargetScheme; }
	void						SetTargetScheme(uint uiTargetScheme)			{ m_uiTargetScheme = uiTargetScheme; }

	uint						GetEffectType() const							{ return m_uiEffectType; }
	void						SetEffectType(uint uiEffectType)				{ m_uiEffectType = uiEffectType; }

	bool						GetIgnoreInvulnerable() const					{ return m_bIgnoreInvulnerable; }
	void						SetIgnoreInvulnerable(bool bIgnoreInvulnerable)	{ m_bIgnoreInvulnerable = bIgnoreInvulnerable; }

	void						SetIgnoreTargetOffset(bool b)					{ m_bIgnoreTargetOffset = b; }

	uint						GetNumImpacts(uint uiUID) const					{ map<uint, uint>::const_iterator it(m_mapImpacts.find(uiUID)); return it == m_mapImpacts.end() ? 0 : it->second; }
	void						ResetImpacts()									{ m_uiBounceCount = 0; m_mapImpacts.clear(); }

	void						ResetTouches()									{ m_uiTotalTouchCount = 0; m_mapTouches.clear(); }
	
	GAME_SHARED_API CSkeleton*	AllocateSkeleton();

	void						UpdateTargetPosition();
	void						EvaluateTrajectory();
	void						TryImpact();

	virtual void				Spawn();
	virtual bool				ServerFrameSetup();
	virtual bool				ServerFrameMovement();
	virtual bool				ServerFrameAction();
	virtual bool				ServerFrameCleanup();
	virtual void				Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

	virtual void				Copy(const IGameEntity &B);

	CCombatEvent&				GetCombatEvent()						{ return m_combat; }

	uint						GetOriginTime() const					{ return m_uiOriginTime; }
	void						SetOriginTime(uint uiOriginTime)		{ m_uiOriginTime = uiOriginTime; }
	
	void						SetTargetEntityUID(uint uiUID)			{ m_uiTargetEntityUID = uiUID; }
	void						SetTargetDisjointSequence(uint uiSeq)	{ m_uiTargetDisjointSequence = uiSeq; }
	void						SetTargetPos(const CVec3f &v3Pos)		{ m_v3TargetPos = v3Pos; }
	void						SetAttackImpactEffect(ResHandle hEffect)	{ m_hAttackImpactEffect = hEffect; }

	virtual void				Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);

	virtual bool				AddToScene(const CVec4f &v4Color, int iFlags);

	uint	GetBounceCount() const	{ return m_uiBounceCount; }
	void	IncrementBounceCount()	{ ++m_uiBounceCount; }

	uint	GetReturnCount() const	{ return m_uiReturnCount; }
	void	IncrementReturnCount()	{ ++m_uiReturnCount; }

	uint	GetRedirectCount() const	{ return m_uiRedirectCount; }
	void	IncrementRedirectCount()	{ ++m_uiRedirectCount; }

	// Setting
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, Lifetime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, InitialCharges)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxCharges)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Speed)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Gravity)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Arc)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, CanTurn)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ModelScale)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(Model)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, EffectScale)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(TrailEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(DeathEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(ImpactEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(InvalidEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, UseExactLifetime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Flying)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Pathing)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Unitwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Treewalking)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Cliffwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Buildingwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, FlyHeight)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, TouchRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, TouchRadiusDirAdjust)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TouchTargetScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TouchEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, TouchIgnoreInvulnerable)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxTouches)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxTouchesPerTarget)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, TouchCliffs)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ImpactDistance)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, ImpactStealth)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Curve)
	
	void	UpdateModifiers(const uivector &vModifiers);

	void	ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target);

	IProjectile*	Clone();
	void			Return();
	void			Revive();
	void			Redirect(IUnitEntity *pSource, IGameEntity *pInflictor, IUnitEntity *pTarget);
	void			Redirect(IUnitEntity *pSource, IGameEntity *pInflictor, const CVec3f &v3Target);

	IUnitEntity*			GetTarget() const;
	const CVec3f&			GetTargetPos() const				{ return m_v3TargetPos; }

	void					SetParam(float fParam)				{ m_fParam = fParam; }
	float					GetParam() const					{ return m_fParam; }

	void					SetProxyUID(uint uiUID)				{ m_uiProxyUID = uiUID; }
	uint					GetProxyUID() const					{ return m_uiProxyUID; }
	IGameEntity*			GetProxy(uint uiIndex) const		{ return Game.GetEntityFromUniqueID(m_uiProxyUID); }

	void					ForceImpact();
};
//=============================================================================

#endif //__I_PROJECTILE_H__

