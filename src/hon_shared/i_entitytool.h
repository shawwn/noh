// (C)2008 S2 Games
// i_entitytool.h
//
//=============================================================================
#ifndef __I_ENTITYTOOL_H__
#define __I_ENTITYTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_slaveentity.h"
#include "i_tooldefinition.h"

#include "i_unitentity.h"
#include "i_projectile.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
// Private flags
const ushort ENTITY_TOOL_FLAG_IN_USE			(BIT(0));
const ushort ENTITY_TOOL_FLAG_INVALID_COST		(BIT(1));
const ushort ENTITY_TOOL_FLAG_LOCKED			(BIT(2));
// Public flags
const ushort ENTITY_TOOL_FLAG_ACTIVE			(BIT(8));
const ushort ENTITY_TOOL_FLAG_ASSEMBLED			(BIT(9));
const ushort ENTITY_TOOL_FLAG_TOGGLE_ACTIVE		(BIT(10));
const ushort ENTITY_TOOL_FLAG_CHANNEL_ACTIVE	(BIT(11));

// Cooldown end times
// 0 == immediately, INVALID_TIME == never
//=============================================================================

//=============================================================================
// IEntityTool
//=============================================================================
class IEntityTool : public ISlaveEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef IToolDefinition TDefinition;
	
protected:
	mutable byte	m_yPublicFlags;
	mutable byte	m_yPrivateFlags;
	ushort			m_unFlags;

	uint		m_uiCooldownTime;
	uint		m_uiCooldownDuration;

	uint		m_uiApparentCooldownTime;
	uint		m_uiApparentCooldownDuration;

	uint		m_uiTargetUID;
	CVec3f		m_v3TargetPos;
	CVec3f		m_v3TargetDelta;
	uivector	m_vChannelEntityUID;
	uivector	m_vToggleEntityUID;
	uiset		m_setPersistentPetUID;
	bool		m_bWasReady;
	bool		m_bNegated;
	uint		m_uiStartChannelTime;
	bool		m_bFinished;

	uint		m_uiActivateTargetUID;
	CVec3f		m_v3ActivateTarget;
	CVec3f		m_v3ActivateDelta;
	bool		m_bActivateSecondary;
	int			m_iActivateIssuedClientNumber;

	uint		m_uiTimer;

	void		AddActionScript(EEntityActionScript eScript, CCombatEvent &combat);
	bool		Impact(IUnitEntity *pTarget, const CVec3f &v3Target, const CVec3f &v3Delta, bool bSecondary, int iIssuedClientNumber, float fManaCost);

	void		ExecuteOwner(EEntityActionScript eScript, EEntityActionScript eOwnerScript,
		IUnitEntity *pTarget, const CVec3f &v3Target, CCombatEvent *pCombatEvent = NULL);

public:
	virtual ~IEntityTool()	{}
	IEntityTool();

	SUB_ENTITY_ACCESSOR(IEntityTool, Tool)

	// Network
	GAME_SHARED_API virtual void		Baseline();
	GAME_SHARED_API virtual void		GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	GAME_SHARED_API virtual bool		ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	virtual bool	ServerFrameSetup();
	virtual bool	ServerFrameAction();

	virtual uint	GetAdjustedActionTime() const;
	virtual uint	GetAdjustedCastTime() const;

	GAME_SHARED_API bool	HasActionScript(EEntityActionScript eScript);
	GAME_SHARED_API void	ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target, CCombatEvent *pCombatEvent = NULL);

	void					SetFlag(ushort unFlag)				{ m_unFlags |= unFlag; }
	bool					HasFlag(ushort unFlag) const		{ return (m_unFlags & unFlag) == unFlag; }
	void					ClearFlag(ushort unFlag)			{ m_unFlags &= ~unFlag; }
	void					ToggleFlag(ushort unFlag)			{ m_unFlags ^= unFlag; }

	ushort					GetAllFlags() const					{ return m_unFlags; }
	void					SetAllFlags(ushort unFlags)			{ m_unFlags = unFlags; }

	virtual bool			IsActive() const					{ return HasFlag(ENTITY_TOOL_FLAG_ACTIVE) && (GetLevel() > 0 || GetMaxLevel() == 0); }
	GAME_SHARED_API bool	IsReady() const;
	
	void					SetCooldownStartTime(uint uiStartTime)	{ m_uiCooldownTime = uiStartTime; }
	uint					GetCooldownStartTime() const			{ return m_uiCooldownTime; }
	
	void					SetCooldownDuration(uint uiDuration)	{ m_uiCooldownDuration = uiDuration; }
	uint					GetCooldownDuration() const				{ return m_uiCooldownDuration; }
	
	GAME_SHARED_API void	StartCooldown(uint uiTime, uint uiDuration);
	GAME_SHARED_API void	ReduceCooldown(uint uiDuration);
	GAME_SHARED_API void	ResetCooldown();
	GAME_SHARED_API uint	GetCooldownEndTime() const			{ return Game.GetCooldownEndTime(m_uiCooldownTime, m_uiCooldownDuration); }
	GAME_SHARED_API void	UpdateApparentCooldown();
	GAME_SHARED_API void	SetApparentCooldown(uint uiStartTime, uint uiDuration);

	GAME_SHARED_API float	GetRemainingCooldownPercent() const;
	GAME_SHARED_API uint	GetRemainingCooldownTime() const;

	GAME_SHARED_API float	GetActualRemainingCooldownPercent() const;
	GAME_SHARED_API uint	GetActualRemainingCooldownTime() const;

	void					AddChannelEntity(uint uiUID)		{ m_vChannelEntityUID.push_back(uiUID); }
	void					AddToggleEntity(uint uiUID)			{ m_vToggleEntityUID.push_back(uiUID); }
	void					AddPersistentPet(uint uiUID)		{ m_setPersistentPetUID.insert(uiUID); }

	uint					GetTimer() const					{ return m_uiTimer; }
	void					SetTimer(uint uiTime)				{ m_uiTimer = uiTime; }

	template<class T>
	bool	CreateProjectile(const T &_target, int iIssuedClientNumber, float fManaCost)
	{
		IUnitEntity *pOwner(GetOwner());
		if (pOwner == NULL)
			return false;

		IProjectile *pProjectile(pOwner->CreateProjectile(GetCastProjectileName(), _target, GetLevel()));
		if (pProjectile == NULL)
			return false;

		CCombatEvent &combat(pProjectile->GetCombatEvent());
		combat.SetSuperType(SUPERTYPE_SPELL);
		combat.SetInitiatorIndex(GetOwnerIndex());
		combat.SetProxyUID(m_uiProxyUID);
		combat.SetEffectType(GetCastEffectType());
		combat.SetManaCost(fManaCost);
		combat.SetIssuedClientNumber(iIssuedClientNumber);
		combat.SetNoResponse(GetNoResponse());

		AddActionScript(ACTION_SCRIPT_PRE_IMPACT, combat);
		AddActionScript(ACTION_SCRIPT_PRE_DAMAGE, combat);
		AddActionScript(ACTION_SCRIPT_DAMAGE_EVENT, combat);
		AddActionScript(ACTION_SCRIPT_IMPACT, combat);
		AddActionScript(ACTION_SCRIPT_IMPACT_INVALID, combat);

		pProjectile->SetTargetScheme(GetTargetScheme());
		pProjectile->SetEffectType(GetCastEffectType());

		pProjectile->ExecuteActionScript(ACTION_SCRIPT_SPAWN, pProjectile->GetTarget(), pProjectile->GetTargetPos());

		return true;
	}

	////
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(CastEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(ActionEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(ImpactEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(BridgeEffect)

	ENTITY_DEFINITION_ACCESSOR(uint, MaxLevel)
	ENTITY_DEFINITION_ACCESSOR(uint, BaseLevel)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CooldownTime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ToggleOffCooldownTime)
	virtual uint	GetCurrentCooldownTime() const
	{
		if (GetActionType() == TOOL_ACTION_TOGGLE && HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
			return GetToggleOffCooldownTime();

		return GetCooldownTime();
	}

	virtual uint	GetTooltipCooldownTime() const
	{
		uint uiCooldownTime(GetCooldownTime());

		IUnitEntity *pOwner(GetOwner());
		if (pOwner != NULL)
		{
			float fCooldownSpeed(pOwner->GetCooldownSpeed());
			float fCooldownReduction(MIN(pOwner->GetReducedCooldowns() - pOwner->GetIncreasedCooldowns(), 1.0f));

			uiCooldownTime = INT_CEIL(uiCooldownTime / fCooldownSpeed * (1.0f - fCooldownReduction));
		}

		return uiCooldownTime;
	}

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CooldownOnDamage)
	
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ManaCost)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ToggleOffManaCost)
	virtual float	GetCurrentManaCost() const
	{
		if (GetActionType() == TOOL_ACTION_TOGGLE && HasFlag(ENTITY_TOOL_FLAG_TOGGLE_ACTIVE))
			return GetToggleOffManaCost();

		return GetManaCost();
	}

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ActiveManaCost)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, TriggeredManaCost)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CastTime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CastActionTime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ChannelTime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, Anim)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(int, AnimChannel)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Range)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ForceRange)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MinRange)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, RangeBuffer)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, TargetRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxDelta)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ForceDelta)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(TargetMaterial)
	ENTITY_DEFINITION_ACCESSOR(EEntityToolAction, ActionType)
	ENTITY_DEFINITION_ACCESSOR(bool, IsChanneling)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AllowOutOfRangeCast)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AllowOutOfBoundsCast)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AllowAutoCast)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, UsePathForRange)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CastEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TargetScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IgnoreInvulnerable)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, Projectile)
	ENTITY_DEFINITION_ACCESSOR(bool, FrontQueue)
	ENTITY_DEFINITION_ACCESSOR(bool, InheritMovement)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, StatusEffectTooltip)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, StatusEffectTooltip2)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, StatusEffectHeader)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, StatusEffectHeader2)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, TooltipFlavorText)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, UseProxy)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ProxyTargetScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ProxyEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ProxySelectionRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(ETargetSelection, ProxySelectionMethod)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, ProxyAllowInvulnerable)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(ProxyTargetMaterial)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SearchRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Disabled)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoStun)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, NoTargetRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(NoTargetMaterial)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, NoCastEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, NoTargetScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoTargetIgnoreInvulnerable)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DeferChannelCost)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DeferChannelImpact)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ChannelRange)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CooldownType)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NonInterrupting)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IgnoreCooldown)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AutoToggleOffWhenDisabled)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AutoToggleOffWithTriggeredManaCost)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoStopAnim)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoResponse)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoVoiceResponse)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NeedVision)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoTurnToTarget)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ActivateScheme)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR_PASSIVE(uint, CarryScheme) // NOTE: Should be an ENTITY_DEFINITION_ACCESSOR, but the defaults are wrong.
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, CloneScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ChargeCost)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, AttackEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, AttackDamageType)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DoubleActivate)

	virtual const tstring& GetCastProjectileName() const { return GetProjectile(); }

	virtual uint		GetLevel() const			{ if (GetMaxLevel() == 0 && m_uiLevel == 0) return 1; return m_uiLevel + GetBaseLevel(); }
	////

	bool				ImpactEntity(uint uiTargetIndex, int iIssuedClientNumber, float fManaCost);
	bool				ImpactPosition(const CVec3f &v3TargetPosition, int iIssuedClientNumber, float fManaCost);
	bool				ImpactVector(const CVec3f &v3TargetPosition, const CVec3f &v3TargetDelta, int iIssuedClientNumber, float fManaCost);

	bool				UpdateEntity(uint uiTargetIndex);
	bool				UpdatePosition(const CVec3f &v3Target);

	bool				FinishEntity(uint uiTargetIndex, bool bCancel);
	bool				FinishPosition(const CVec3f &v3Target, bool bCancel);

	virtual void		PlayCastEffect();
	virtual void		PlayActionEffect();
	virtual bool		IsDisabled() const;
	virtual bool		CanOrder();
	virtual bool		CanActivate();
	virtual bool		IsTargetValid(IUnitEntity *pTarget, const CVec3f &v3Target);
	virtual bool		Activate(IUnitEntity *pTarget, const CVec3f &v3Target, const CVec3f &v3Delta, bool bSecondary, int iIssuedClientNumber);
	virtual bool		ToggleAutoCast();
	virtual bool		Update();
	virtual void		Finish(bool bCancel);
	virtual bool		IsValidTarget(IUnitEntity *pTarget);

	virtual ResHandle	GetEffect();

	virtual void		Spawn();

	virtual bool		OwnerDamaged(CDamageEvent &damage);

	virtual void		UpdateModifiers(const uivector &vModifiers);

	GAME_SHARED_API IUnitEntity*	SelectProxy() const;

	virtual void		Interrupt(EUnitAction eAction);
	virtual bool		IsChanneling(EUnitAction eAction);

	virtual bool		ToggleOff();

	virtual bool		CheckCost();
	virtual bool		CheckTriggeredCost();
};
//=============================================================================

#endif //__I_ENTITYTOOL_H__
