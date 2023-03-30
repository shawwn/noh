// (C)2006 S2 Games
// i_entitystate.h
//
//=============================================================================
#ifndef __I_ENTITYSTATE_H__
#define __I_ENTITYSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_slaveentity.h"
#include "c_statedefinition.h"

#include "../k2/c_modifier.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMeleeAttackEvent;
class IVisualEntity;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const ushort INVALID_ENTITY_STATE(USHRT_MAX);
//=============================================================================

//=============================================================================
// IEntityState
//=============================================================================
class IEntityState : public ISlaveEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef CStateDefinition TDefinition;
	
protected:
	uint			m_uiInflictorIndex;

	uint			m_uiCreationTime;
	uint			m_uiLifetime;
	uint			m_uiLastIntervalTriggerTime;

	uint			m_uiAuraSourceUID;
	uint			m_uiAuraTime;
	uint			m_uiAuraTargetScheme;
	uint			m_uiAuraEffectType;
	bool			m_bExpired;
	bool			m_bExpireNextFrame;

	uint			m_uiTimeout;

public:
	virtual ~IEntityState()	{}
	IEntityState();

	SUB_ENTITY_ACCESSOR(IEntityState, State)

	// Network
	GAME_SHARED_API virtual void		Baseline();
	GAME_SHARED_API virtual void		GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	GAME_SHARED_API virtual bool		ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	static void		ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
	static void		ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

	void			SetInflictorIndex(uint uiIndex)		{ m_uiInflictorIndex = uiIndex; }
	uint			GetInflictorIndex()					{ return m_uiInflictorIndex; }

	void			SetStartTime(uint uiTime)			{ m_uiCreationTime = uiTime; }
	uint			GetStartTime() const				{ return m_uiCreationTime; }

	void			SetLifetime(uint uiTime)			{ m_uiLifetime = uiTime; }
	uint			GetLifetime() const					{ return m_uiLifetime; }

	void			SetTimeout(uint uiTime)				{ m_uiTimeout = uiTime; }
	uint			GetTimeout() const					{ return m_uiTimeout; }

	void			Defer(uint uiTime);

	uint			GetExpireTime() const				{ if (m_uiCreationTime != INVALID_TIME && m_uiLifetime != INVALID_TIME) return m_uiCreationTime + m_uiLifetime; else return INVALID_TIME; }
	uint			GetRemainingLifetime() const
	{
		if (m_uiCreationTime != INVALID_TIME && m_uiLifetime != INVALID_TIME)
		{
			if (m_uiCreationTime + m_uiLifetime >= Game.GetGameTime())
				return m_uiCreationTime + m_uiLifetime - Game.GetGameTime();
			else
				return 0;
		}
		else
			return INVALID_TIME;
	}

	float			GetRemainingLifetimePercent() const	{ if (m_uiCreationTime != INVALID_TIME && m_uiLifetime != INVALID_TIME) return GetRemainingLifetime() / float(m_uiLifetime); else return 0.0f; }

	void			SetExpired(bool bExpired)						{ m_bExpired = bExpired; }
	void			SetExpireNextFrame(bool bExpireNextFrame)		{ m_bExpireNextFrame = bExpireNextFrame; }

	void			SetAuraSource(uint uiUID)					{ m_uiAuraSourceUID = uiUID; }
	void			SetAuraTime(uint uiTime)					{ m_uiAuraTime = uiTime; }
	void			SetAuraTargetScheme(uint uiTargetScheme)	{ m_uiAuraTargetScheme = uiTargetScheme; }
	void			SetAuraEffectType(uint uiEffectType)		{ m_uiAuraEffectType = uiEffectType; }
	IUnitEntity*	GetInflictor() const						{ return Game.GetUnitEntity(m_uiInflictorIndex); }

	virtual bool	IsExpired() const					{ return m_bExpired; }
	virtual bool	IsActive() const					{ return !IsExpired() && (m_uiCreationTime == INVALID_TIME || Game.GetServerTime() >= m_uiCreationTime); }
	virtual bool	IsAuraInvalid() const				{ return m_uiAuraSourceUID != INVALID_INDEX && m_uiAuraTime != Game.GetGameTime(); }
	virtual bool	IsAura() const						{ return m_uiAuraSourceUID != INVALID_INDEX; }

	virtual	void	Activated();
	virtual void	Expired();
	virtual void	Spawn();
	virtual bool	ServerFrameSetup();
	virtual bool	ServerFrameThink();
	virtual bool	ServerFrameAction();

	virtual bool	OwnerDamaged(CDamageEvent &damage);
	virtual bool	OwnerAction();

	void			ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target);

	virtual void	UpdateModifiers();

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IsHidden)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DispelOnDamage)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DispelOnAction)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DisplayLevel)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImpactInterval)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MorphPriority)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoRefresh)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, PropagateToIllusions)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DeathPersist)

	PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Armor)
	PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, MoveSpeedSlow)
	PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, MagicArmor)

	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, Strength)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, Agility)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, Intelligence)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MaxHealth)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, HealthRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, HealthProportionRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MaxMana)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, ManaRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, ManaProportionRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MaxStamina)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, StaminaRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, StaminaProportionRegen)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, Armor)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MagicArmor)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MoveSpeed)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, SlowResistance)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, AttackRange)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, LifeSteal)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, HealthRegenReduction)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, ManaRegenReduction)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, StaminaRegenReduction)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tsvector&, UnitType)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, Icon)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, Portrait)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, Model)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, MapIconProperty)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(CVec4f, MapIconColorProperty)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, MapIconSizeProperty)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Skin)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, AttackProjectile)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackType)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackDamageType)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackCooldown)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackDuration)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackActionTime)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AttackTargetScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, AttackNonLethal)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, ThreatScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, ThreatEffectType)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, AggroRange)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, AggroScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, ProximityRange)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(uint, ProximityScheme)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, SightRangeDay)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, SightRangeNight)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, PreGlobalScale)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, EffectScale)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, ModelScale)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, InfoHeight)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, BoundsRadius)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, BoundsHeight)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, SelectionRadius)
	ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, IsSelectable)
	ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, NoCorpse)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, AttackRangeBuffer)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(CVec3f, AttackOffset)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(CVec3f, TargetOffset)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, AttackStartEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, AttackActionEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, AttackImpactEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, DeathEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, PassiveEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(ResHandle, SpawnEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory0)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory1)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory2)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory3)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory4)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory5)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory6)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory7)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, Inventory8)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, SharedInventory0)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, SharedInventory1)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, SharedInventory2)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Invulnerable)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, ClearVision)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Deniable)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, DeniablePercent)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Smackable)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, NoThreat)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, TrueStrike)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Unitwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Treewalking)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Cliffwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Buildingwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, Antiwalking)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, ShopAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, RemoteShopAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, SharedShopAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, SharedRemoteShopAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(const tstring&, RestrictItemAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(bool, StashAccess)
	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(EUnitCommand, DefaultBehavior)

	MULTI_LEVEL_ENTITY_DEFINITION_MUTATION_ACCESSOR(float, Power)
};
//=============================================================================

#endif //__I_ENTITYSTATE_H__
