// (C)2008 S2 Games
// i_unitentity.h
//
//=============================================================================
#ifndef __I_UNITENTITY_H__
#define __I_UNITENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
#include "c_brain.h"
#include "i_slaveentity.h"
#include "i_entitystate.h"
#include "i_unitdefinition.h"
#include "c_entitydefinitionresource.h"

#include "game_shared_cvars.h"
#include "../k2/c_lerps.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CDamageEvent;
class CCombatEvent;

class IEntityTool;
class CShopDefinition;

EXTERN_CVAR_UINT(g_corpseTime);
EXTERN_CVAR_FLOAT(g_unitMapIconSize);
EXTERN_CVAR_STRING(g_unitMapIcon);
EXTERN_CVAR_UINT(unit_blockRepathTime);
EXTERN_CVAR_UINT(unit_blockRepathTimeExtra);
EXTERN_CVAR_FLOAT(unit_slideThreshold);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef map<uint, float>	AggroMap;

const ushort UNIT_FLAG_INVULNERABLE				(BIT(0));
const ushort UNIT_FLAG_BOUND					(BIT(1));
const ushort UNIT_FLAG_IGNORE_ATTACK_COOLDOWN	(BIT(2));
const ushort UNIT_FLAG_STEALTH					(BIT(3));
const ushort UNIT_FLAG_ILLUSION					(BIT(4));
const ushort UNIT_FLAG_TERMINATED				(BIT(5));
const ushort UNIT_FLAG_UNCONTROLLABLE			(BIT(6));
const ushort UNIT_FLAG_REVEALED					(BIT(7));
const ushort UNIT_FLAG_LOCKED_BACKPACK			(BIT(8));
const ushort UNIT_FLAG_NOT_CONTROLLABLE			(BIT(9));
const ushort UNIT_FLAG_SPRINTING				(BIT(10));

const byte DEATH_FLAG_NOCORPSE				(BIT(0));
const byte DEATH_FLAG_NODEATHANIM			(BIT(1));
const byte DEATH_FLAG_FORCE					(BIT(2));
const byte DEATH_FLAG_PROTECTED				(BIT(3));
const byte DEATH_FLAG_EXPIRE				(BIT(4));

const byte MISC_FLAG_ILLUSIONANIM			(BIT(0));

const uint	INVENTORY_MAX_ABILITIES(9);
const uint	INVENTORY_MAX_SHARED_ABILITIES(3);
const uint	INVENTORY_BACKPACK_SIZE(6);
const uint	INVENTORY_STASH_SIZE(6);
const uint	MAX_ACTIVE_ENTITY_STATES(16);
const uint	MAX_ATTACK_MODIFIERS(8);

const uint	MAX_LOG_DAMAGE_TRACKERS(8);

const uint	TRACE_UNIT_MOVEMENT(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM | SURF_TERRAIN | SURF_BLOCKER | SURF_NOBLOCK | SURF_FLYING | SURF_UNITWALKING);
const uint	TRACE_UNIT_SPAWN(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM | SURF_TERRAIN | SURF_BLOCKER | SURF_NOBLOCK | SURF_FLYING | SURF_UNITWALKING);
const uint	TRACE_UNIT_MOVEMENT_FLYING(TRACE_NOCLIP);
const uint	TRACE_UNIT_PUSH(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM | SURF_TERRAIN | SURF_NOBLOCK | SURF_FLYING | SURF_UNITWALKING | SURF_UNIT);
const uint	TRACE_TREASURE_CHEST(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_TERRAIN | SURF_BLOCKER | SURF_NOBLOCK | SURF_FLYING | SURF_UNITWALKING);
const uint	TRACE_PROJECTILE_TOUCH(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_ITEM | SURF_TERRAIN | SURF_BLOCKER | SURF_NOBLOCK);

const int	INVENTORY_START_ACTIVE			(0);

const int	INVENTORY_START_ABILITIES		(INVENTORY_START_ACTIVE);
const int	INVENTORY_END_ABILITIES			(INVENTORY_START_ABILITIES + INVENTORY_MAX_ABILITIES - 1);

const int	INVENTORY_START_STATES			(INVENTORY_END_ABILITIES + 1);
const int	INVENTORY_END_STATES			(INVENTORY_START_STATES + MAX_ACTIVE_ENTITY_STATES - 1);

const int	INVENTORY_START_SHARED_ABILITIES	(INVENTORY_END_STATES + 1);
const int	INVENTORY_END_SHARED_ABILITIES	(INVENTORY_START_SHARED_ABILITIES + INVENTORY_MAX_SHARED_ABILITIES - 1);

const int	INVENTORY_START_BACKPACK		(INVENTORY_END_SHARED_ABILITIES + 1);
const int	INVENTORY_END_BACKPACK			(INVENTORY_START_BACKPACK + INVENTORY_BACKPACK_SIZE - 1);

const int	INVENTORY_END_ACTIVE			(INVENTORY_END_BACKPACK);

const int	INVENTORY_BACKPACK_PROVISIONAL	(INVENTORY_END_ACTIVE + 1);

const int	INVENTORY_START_STASH			(INVENTORY_BACKPACK_PROVISIONAL + 1);
const int	INVENTORY_END_STASH				(INVENTORY_START_STASH + INVENTORY_STASH_SIZE - 1);

const int	INVENTORY_STASH_PROVISIONAL		(INVENTORY_END_STASH + 1);

const int	MAX_INVENTORY					(INVENTORY_STASH_PROVISIONAL + 1);

#define IS_BACKPACK_SLOT(slot)		(((slot) >= INVENTORY_START_BACKPACK) && ((slot) <= INVENTORY_BACKPACK_PROVISIONAL))
#define IS_STASH_SLOT(slot)			(((slot) >= INVENTORY_START_STASH) && ((slot) <= INVENTORY_STASH_PROVISIONAL))
#define IS_ITEM_SLOT(slot)			(IS_BACKPACK_SLOT(slot) || IS_STASH_SLOT(slot))
#define IS_PROVISIONAL_SLOT(slot)	(((slot) == INVENTORY_BACKPACK_PROVISIONAL) || ((slot) == INVENTORY_STASH_PROVISIONAL))
#define IS_SHARED_ABILITY(slot)		(((slot) >= INVENTORY_START_SHARED_ABILITIES) && ((slot) <= INVENTORY_END_SHARED_ABILITIES))

// MULTI_LEVEL_INITIAL_ATTRIBUTE
#define MULTI_LEVEL_INITIAL_ATTRIBUTE(type, name) \
virtual type	GetInitial##name() const \
{ \
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// BASE_ATTRIBUTE
#define BASE_ATTRIBUTE(type, name) \
virtual type	GetBase##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(); \
}

// MULTI_LEVEL_BASE_ATTRIBUTE
#define MULTI_LEVEL_BASE_ATTRIBUTE(type, name) \
virtual type	GetBase##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE
#define PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(type, name) \
virtual type	GetBase##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	uint uiLevelIndex(MAX(1u, GetLevel()) - 1); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(uiLevelIndex) + \
		uiLevelIndex * static_cast<TDefinition *>(m_pDefinition)->Get##name##PerLevel() + \
		GetCharges() * static_cast<TDefinition *>(m_pDefinition)->Get##name##PerCharge(uiLevelIndex); \
}

// MUTABLE_MULTI_LEVEL_ATTRIBUTE
#define MUTABLE_MULTI_LEVEL_ATTRIBUTE(type, name) \
virtual type	Get##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// MUTABLE_MULTI_LEVEL_ATTRIBUTE
#define MUTABLE_MULTI_LEVEL_ATTRIBUTE_OVERRIDE(type, name, overridevar, overrideval) \
virtual type	Get##name() const \
{ \
	if ((overridevar) != (overrideval)) \
		return (overridevar); \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// MUTABLE_MULTI_LEVEL_INITIAL_ATTRIBUTE
#define MUTABLE_MULTI_LEVEL_INITIAL_ATTRIBUTE(type, name) \
virtual type	GetInitial##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return GetDefaultEmptyValue<type>(); \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}

// MUTABLE_MULTI_LEVEL_RESOURCE
#define MUTABLE_MULTI_LEVEL_RESOURCE(name) \
virtual ResHandle	Get##name() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return m_pMorphState->GetMorph##name(); \
\
	if (m_pDefinition == NULL) \
		return INVALID_RESOURCE; \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name(MAX(1u, GetLevel()) - 1); \
}\
virtual const tstring&	Get##name##Path() const \
{ \
	if (m_pMorphState != NULL && m_pMorphState->GetApply##name##Morph()) \
		return g_ResourceManager.GetPath(m_pMorphState->GetMorph##name()); \
\
	if (m_pDefinition == NULL) \
		return TSNULL; \
\
	return static_cast<TDefinition *>(m_pDefinition)->Get##name##Path(MAX(1u, GetLevel()) - 1); \
}

// ADJUSTED_ATTRIBUTE_ADD
#define ADJUSTED_ATTRIBUTE_ADD(type, name, illusion) \
virtual type	Get##name() const \
{ \
	type _Adjustment(GetBase##name()); \
	if (IsIllusion() && !illusion) \
		return _Adjustment; \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
\
		_Adjustment += m_apInventory[iSlot]->Get##name(); \
	} \
\
	return _Adjustment; \
}

// ADJUSTED_ATTRIBUTE_MAX
#define ADJUSTED_ATTRIBUTE_MAX(type, name, illusion) \
virtual type	Get##name() const \
{ \
	type _Adjustment(0); \
	if (IsIllusion() && !illusion) \
		return _Adjustment; \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
\
		_Adjustment = MAX(_Adjustment, m_apInventory[iSlot]->Get##name()); \
	} \
\
	return GetBase##name() + _Adjustment; \
}

// ADJUSTED_ATTRIBUTE_OR
#define ADJUSTED_ATTRIBUTE_OR(type, name, illusion) \
virtual type	Get##name() const \
{ \
	type _Adjustment(0); \
	if (IsIllusion() && !illusion) \
		return _Adjustment; \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
\
		_Adjustment = _Adjustment || m_apInventory[iSlot]->Get##name(); \
	} \
\
	return GetBase##name() || _Adjustment; \
}

// ADJUSTED_ATTRIBUTE_CONCAT
#define ADJUSTED_ATTRIBUTE_CONCAT(type, name, illusion) \
virtual type	Get##name() const \
{ \
	if (IsIllusion() && !illusion) \
		return GetDefaultEmptyValue<type>(); \
\
	type _Value(GetBase##name()); \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
\
		if (_Value.empty()) \
			_Value = m_apInventory[iSlot]->Get##name(); \
		else if (!m_apInventory[iSlot]->Get##name().empty())\
			_Value += _T(" ") + m_apInventory[iSlot]->Get##name(); \
	} \
\
	return _Value; \
}

// ADJUSTED_ATTRIBUTE_INSERTEND
#define ADJUSTED_ATTRIBUTE_INSERTEND(type, name, illusion) \
virtual type	Get##name() const \
{ \
	if (IsIllusion() && !illusion) \
		return GetDefaultEmptyValue<type>(); \
\
	type _Value(GetBase##name()); \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
\
		const type &_SlaveValue(m_apInventory[iSlot]->Get##name()); \
		if (!_SlaveValue.empty()) \
			_Value.insert(_Value.end(), _SlaveValue.begin(), _SlaveValue.end()); \
	} \
\
	return _Value; \
}

// MULTI_LEVEL_ATTRIBUTE
#define MULTI_LEVEL_ATTRIBUTE(type, name, func, illusion) \
MULTI_LEVEL_BASE_ATTRIBUTE(type, name) \
ADJUSTED_ATTRIBUTE_##func(type, name, illusion)

// STATIC_ATTRIBUTE
#define STATIC_ATTRIBUTE(type, name, func, base, illusion) \
virtual type	GetBase##name() const	{ return base; } \
ADJUSTED_ATTRIBUTE_##func(type, name, illusion)

// STATIC_BASE_ATTRIBUTE
#define STATIC_BASE_ATTRIBUTE(type, name, base) \
virtual type	GetBase##name() const	{ return base; }

// BOOL_ATTRIBUTE
#define BOOL_ATTRIBUTE(name, illusion) \
bool	Is##name() const \
{ \
	if (IsIllusion() && !illusion) \
		return false; \
\
	for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot) \
	{ \
		if (m_apInventory[iSlot] == NULL) \
			continue; \
		if (m_apInventory[iSlot]->Get##name()) \
			return true; \
	} \
\
	return false; \
}

// PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE
#define PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(type, name, func, illusion) \
PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(type, name) \
ADJUSTED_ATTRIBUTE_##func(type, name, illusion)

struct SCooldown
{
	uint uiStartTime;
	uint uiDuration;
};

enum EUnitAction
{
	UNIT_ACTION_ATTACK = 0,
	UNIT_ACTION_CAST,
	UNIT_ACTION_MOVE,
	UNIT_ACTION_STOP,
	UNIT_ACTION_DEATH,
	UNIT_ACTION_INTERRUPT
};
//=============================================================================

//=============================================================================
// IUnitEntity
//=============================================================================
class IUnitEntity : public IVisualEntity
{
	DECLARE_ENTITY_DESC

public:
	typedef IUnitDefinition TDefinition;

private:
	static ResHandle	s_hSelectionIndicator;
	static ResHandle	s_hMinimapIcon;
	static ResHandle	s_hRecipeEffect;

protected:
	struct SDamageTrackerLog
	{
		uint	uiTimeStamp;
		int		iPlayerOwner;
		uint	uiAttackerUID;
		ushort	unAttackerType;
		ushort	unInflictorType;
		float	fDamage;

		SDamageTrackerLog(uint _uiTimeStamp, int _iPlayerOwner, uint _uiAttackerUID, ushort _unAttackerType, ushort _unInflictorType, float _fDamage) :
		uiTimeStamp(_uiTimeStamp),
		iPlayerOwner(_iPlayerOwner),
		uiAttackerUID(_uiAttackerUID),
		unAttackerType(_unAttackerType),
		unInflictorType(_unInflictorType),
		fDamage(_fDamage)
		{}
	};

	typedef vector<SDamageTrackerLog>	LogDamageVector;
	typedef LogDamageVector::iterator	LogDamageVector_it;

	struct SDamageTrackerFrame
	{
		uint	uiAttackerUID;
		float	fDamage;

		SDamageTrackerFrame() {}
		SDamageTrackerFrame(uint _uiAttackerUID, float _fDamage) :
		uiAttackerUID(_uiAttackerUID),
		fDamage(_fDamage)
		{}
	};

	typedef vector<SDamageTrackerFrame>	FrameDamageVector;
	typedef FrameDamageVector::iterator	FrameDamageVector_it;

	CBrain			m_cBrain;

	// Stats
	ushort			m_unUnitFlags;
	uint			m_uiLevel;
	float			m_fHealth;
	float			m_fMana;
	float			m_fStamina;
	float			m_fCurrentMaxHealth;
	float			m_fCurrentMaxMana;
	float			m_fCurrentMaxStamina;
	float			m_fCurrentCooldownSpeed;
	float			m_fCurrentCooldownReduction;
	float			m_fAccumulator;

	float				m_fLethalDamageAccumulator;
	float				m_fNonLethalDamageAccumulator;
	float				m_fHealthAccumulator;
	FrameDamageVector	m_vFrameDamageTrackers;
	LogDamageVector		m_vLogDamageTrackers;
	float				m_fTotalTrackedDamage;
	float				m_fMovementDistance;
	
	// Client only display values
	uint			m_uiHealthShadowTime;
	uint			m_uiManaShadowTime;
	float			m_fHealthShadowMarker;
	float			m_fManaShadowMarker;
	////

	uint				m_uiKiller;
	ushort				m_unDeathInflictor;

	int				m_iExclusiveAttackModSlot;
	bool			m_bHadStashAccess;

	ISlaveEntity*	m_apInventory[MAX_INVENTORY];
	IGadgetEntity*	m_pMount;
	IEntityState*	m_pMorphState;

	CVec3f			m_v3UnitAngles;						// Visual unit angle
	CVec3f			m_v3AttentionAngles;
	uint			m_uiTargetIndex;

	int				m_iOwnerClientNumber;
	uint			m_uiOwnerEntityIndex;

	uiset			m_setSoulLinks;

	uint			m_uiTargetUID;
	CVec3f			m_v3TargetPos;
	CVec3f			m_v3OldTargetPos;

	typedef pair<CVec2f, uint>	PushRecord;
	vector<PushRecord>	m_vPushRecords;

	uint			m_uiCorpseTime;
	uint			m_uiCorpseFadeTime;
	float			m_fFade;
	uint			m_uiDeathTime;

	uint			m_uiLinkFlags;
	uint			m_uiLinkedFlags;

	uint			m_uiCombatType;

	vector<PoolHandle>	m_vPathBlockers;
	CVec2f			m_v2BlockPosition;

	uint			m_uiAttackCooldownTime;

	uint			m_uiSpawnTime;
	uint			m_uiLifetime;

	float			m_fReceiveDamageMultiplier;
	float			m_fInflictDamageMultiplier;
	
	float			m_fExperienceBountyMultiplier;
	float			m_fGoldBountyMultiplier;
	uint			m_uiDeathFlags;

	uint			m_uiDisjointSequence;
	uint			m_uiArmingSequence;

	uint			m_uiIdleStartTime;

	float			m_fCurrentDamage;
	ESuperType		m_eCurrentDamageSuperType;
	uint			m_uiCurrentDamageEffectType;
	
	uint			m_uiLastHeroAttackTime;

	uint			m_uiControllerUID;
	uint			m_uiFadeStartTime;

	uint			m_uiProxyUID;

	// Tilting (Client-side only)
	float			m_fTiltPitch;
	float			m_fTiltRoll;

	float			m_fBonusDamage;
	float			m_fBonusDamageMultiplier;

	uint			m_auiLastAggression[2];

	ResHandle		m_hDeathEffect;
	uint			m_uiMiscFlags;

	CCombatEvent	m_cCombatEvent;
	ushort			m_unAttackProjectile;
	ResHandle		m_hAttackActionEffect;
	ResHandle		m_hAttackImpactEffect;
	uint			m_uiAttackAbilityUID;

	uint			m_uiLastAttackTargetUID;
	uint			m_uiLastAttackTargetTime;

	uint			m_uiAttackSequence;
	uint			m_uiOrderSequence;

	uint			m_uiGuardChaseTime;
	uint			m_uiGuardChaseDistance;
	uint			m_uiGuardReaggroChaseTime;
	uint			m_uiGuardReaggroChaseDistance;
	uint			m_uiOverrideAggroRange;

	bool			m_bUseAltDeathAnims;

	uint			m_uiAllUnitsExceptCouriersScheme;

	bool			m_bIsKongor;
	bool			m_bIsTower;

	CVec2f			m_v2AnchorPosition;

	map<uint, SCooldown>	m_mapCooldowns;

	float	TiltTrace(const CVec3f &v3Start, const CVec3f &v3End);

	virtual IProjectile*	SpawnProjectile(const tstring &sName, const CVec3f &v3End, uint uiLevel);

	bool	Slide(CVec2f v2MovementVector, uint uiTraceFlags, CPlane &plImpactPlane);
	void	ValidatePosition2();

	void	AddSelectionRingToScene();

public:
	virtual ~IUnitEntity();
	IUnitEntity();

	// Type
	SUB_ENTITY_ACCESSOR(IUnitEntity, Unit)

	virtual bool			IsActive() const						{ return true; }

	// Network
	virtual void			Baseline();
	virtual void			GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool			ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

	virtual int				GetPrivateClient()						{ return m_iOwnerClientNumber; }

	static void				ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
	static void				ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

	void					RemoveUnitFlags(ushort unFlags)			{ m_unUnitFlags &= ~unFlags; }
	void					SetUnitFlags(ushort unFlags)			{ m_unUnitFlags |= unFlags; }
	bool					HasUnitFlags(ushort unFlags) const		{ return (m_unUnitFlags & unFlags) == unFlags; }
	ushort					GetUnitFlags(ushort unFlags) const		{ return m_unUnitFlags; }

	const CVec3f&			GetUnitAngles() const					{ return m_v3UnitAngles; }
	void					SetUnitAngles(const CVec3f &v3Angles)	{ m_v3UnitAngles = v3Angles; }

	const CVec3f&			GetAttentionAngles() const					{ return m_v3AttentionAngles; }
	void					SetAttentionAngles(const CVec3f &v3Angles)	{ m_v3AttentionAngles = v3Angles; }
	void					SetAttentionYaw(float fYaw)					{ m_v3AttentionAngles[YAW] = fYaw; }

	void					SetTargetIndex(uint uiIndex);
	uint					GetTargetIndex() const					{ return m_uiTargetIndex; }
	IUnitEntity*			GetTargetEntity() const					{ return Game.GetUnitEntity(m_uiTargetIndex); }

	void					SetSpawnTime(uint uiTime)				{ m_uiSpawnTime = uiTime; }
	uint					GetSpawnTime() const					{ return m_uiSpawnTime; }

	bool					IsTower() const							{ return m_bIsTower; }
	bool					IsKongor() const						{ return m_bIsKongor; }

	virtual CSkeleton*		AllocateSkeleton();

	virtual void			ApplyWorldEntity(const CWorldEntity &ent);

	virtual void			Spawn();
	virtual void			Kill(IUnitEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
	virtual void			Die(IUnitEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
	virtual void			Expire();
	virtual void			StopLiving();
	virtual void			KillReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller);
	virtual void			Damage(CDamageEvent &damage);
	virtual void			Touch(IGameEntity *pActivator);

	virtual float			GetBaseScale() const					{ return GetPreGlobalScale() * GetModelScale(); }

	virtual uint			GetLinkFlags();
	virtual void			Link();
	virtual void			Unlink();

	virtual void			BlockPath();
	virtual void			UnblockPath();

	float					ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target, float fDefault = 0.0f);

	GAME_SHARED_API PoolHandle	GetActivePath();
	bool					TestSlide(CVec2f v2MovementVector, float fFraction, bool bDirectPathing);

	void					SetControllerUID(uint uiUID)			{ m_uiControllerUID = uiUID; }
	uint					GetControllerUID() const				{ return m_uiControllerUID; }
	IGameEntity*			GetController() const					{ return Game.GetEntityFromUniqueID(m_uiControllerUID); }

	// Attributes
	virtual uint		GetLevel() const				{ return m_uiLevel; }
	virtual void		SetLevel(uint uiLevel)			{ m_uiLevel = uiLevel; }

	virtual uint		GetInitialCharges() const		{ return 0; }
	virtual ushort		GetCharges() const				{ return 0; }
	virtual void		SetCharges(uint uiCharges)		{}
	virtual void		AddCharges(uint uiCharges)		{}
	virtual void		RemoveCharge()					{}

	void				SetAccumulator(float fValue)	{ m_fAccumulator = fValue; }
	float				GetAccumulator() const			{ return m_fAccumulator; }
	void				AdjustAccumulator(float fDelta)	{ m_fAccumulator += fDelta; }

	ENTITY_DEFINITION_ACCESSOR(const tstring&, IdleAnim)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, WalkAnim)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, SprintAnim)

	MULTI_LEVEL_ATTRIBUTE(tsvector, UnitType, INSERTEND, false)

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, MaxHealth)
	STATIC_ATTRIBUTE(float, MaxHealthMultiplier, ADD, 1.0f, true)
	virtual float	GetMaxHealth() const
	{
		float fBonus(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetMaxHealth();
		}

		return floor((GetBaseMaxHealth() + fBonus) * MAX(0.0f, GetMaxHealthMultiplier()));
	}

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, HealthRegen)
	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, HealthProportionRegen, ADD, false)
	STATIC_ATTRIBUTE(float, HealthRegenMultiplier, ADD, 1.0f, true)
	virtual float	GetHealthRegen() const
	{
		float fBonus(0.0f);
		float fPercent(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetHealthRegen();
			fPercent += m_apInventory[iSlot]->GetHealthRegenPercent();
		}

		if (IsIllusion())
			fBonus = 0.0f;

		float fBase(GetBaseHealthRegen() + GetHealthProportionRegen() * GetMaxHealth());
		float fValue(fBase * MAX(0.0f, GetHealthRegenMultiplier()) + fBonus + (fPercent * GetMaxHealth()));
		return fValue * CLAMP(1.0f - GetHealthRegenReduction(), 0.0f, 1.0f);
	}

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, MaxMana)
	STATIC_ATTRIBUTE(float, MaxManaMultiplier, ADD, 1.0f, true)
	virtual float	GetMaxMana() const
	{
		float fBonus(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetMaxMana();
		}

		return floor((GetBaseMaxMana() + fBonus) * MAX(0.0f, GetMaxManaMultiplier()));
	}

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, ManaRegen)
	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, ManaProportionRegen, ADD, false)
	STATIC_ATTRIBUTE(float, ManaRegenMultiplier, ADD, 1.0f, true)
	virtual float	GetManaRegen() const
	{
		float fBonus(0.0f);
		float fPercent(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetManaRegen();
			fPercent += m_apInventory[iSlot]->GetManaRegenPercent();
		}

		if (IsIllusion())
			fBonus = 0.0f;

		float fBase(GetBaseManaRegen() + GetManaProportionRegen() * GetMaxMana());
		float fValue(fBase * MAX(0.0f, GetManaRegenMultiplier()) + fBonus + (fPercent * GetMaxMana()));
		return fValue * CLAMP(1.0f - GetManaRegenReduction(), 0.0f, 1.0f);
	}

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, MaxStamina)
	STATIC_ATTRIBUTE(float, MaxStaminaMultiplier, ADD, 1.0f, true)
	virtual float	GetMaxStamina() const
	{
		float fBonus(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetMaxStamina();
		}

		return floor((GetBaseMaxStamina() + fBonus) * MAX(0.0f, GetMaxStaminaMultiplier()));
	}

	PROGRESSIVE_MULTI_LEVEL_BASE_ATTRIBUTE(float, StaminaRegen)
	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, StaminaProportionRegen, ADD, false)
	STATIC_ATTRIBUTE(float, StaminaRegenMultiplier, ADD, 1.0f, true)
	virtual float	GetStaminaRegen() const
	{
		float fBonus(0.0f);
		float fPercent(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetStaminaRegen();
			fPercent += m_apInventory[iSlot]->GetStaminaRegenPercent();
		}

		if (IsIllusion())
			fBonus = 0.0f;

		float fBase(GetBaseStaminaRegen() + GetStaminaProportionRegen() * GetMaxStamina());
		float fValue(fBase * MAX(0.0f, GetStaminaRegenMultiplier()) + fBonus + (fPercent * GetMaxStamina()));
		return fValue * CLAMP(1.0f - GetStaminaRegenReduction(), 0.0f, 1.0f);
	}

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ArmorType)
	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, Armor, ADD, false)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MagicArmorType)
	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, MagicArmor, ADD, false)

	MULTI_LEVEL_ATTRIBUTE(float, SlowResistance, ADD, false)
	MULTI_LEVEL_INITIAL_ATTRIBUTE(float, MoveSpeed)
	MULTI_LEVEL_BASE_ATTRIBUTE(float, MoveSpeed)
	STATIC_ATTRIBUTE(float, MoveSpeedMultiplier, ADD, 1.0f, true)
	virtual float	GetMoveSpeed() const
	{
		float fBonus(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus += m_apInventory[iSlot]->GetMoveSpeed();
		}

		float fSlowResistance(CLAMP(GetSlowResistance(), 0.0f, 1.0f));

		float fMultiplier(GetMoveSpeedMultiplier());

		if (HasUnitFlags(UNIT_FLAG_SPRINTING))
		{
			fBonus += GetSprintMoveSpeedBonus();
			fMultiplier += GetSprintMoveSpeedMultiplier();
		}

		float fMoveSpeed((GetBaseMoveSpeed() + fBonus) * MAX(0.0f, fMultiplier));

		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fMoveSpeed *= (1.0f - m_apInventory[iSlot]->GetMoveSpeedSlow() * (1.0f - fSlowResistance));
		}

		return CLAMP<float>(fMoveSpeed, g_unitMoveSpeedMin, g_unitMoveSpeedMax);
	}

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SprintMoveSpeedBonus)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SprintMoveSpeedMultiplier)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SprintStaminaCost)

	MULTI_LEVEL_INITIAL_ATTRIBUTE(float, AttackRange)
	MULTI_LEVEL_BASE_ATTRIBUTE(float, AttackRange)
	STATIC_ATTRIBUTE(float, AttackRangeMultiplier, ADD, 1.0f, true)
	virtual float	GetAttackRange() const
	{
		float fBonus(0.0f);
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fBonus = MAX(fBonus, m_apInventory[iSlot]->GetAttackRange());
		}

		return (GetBaseAttackRange() + fBonus) * MAX(0.0f, GetAttackRangeMultiplier());
	}

	STATIC_BASE_ATTRIBUTE(float, AttackSpeed, 1.0f)
	STATIC_ATTRIBUTE(float, AttackSpeedMultiplier, ADD, 1.0f, true)
	virtual float	GetAttackSpeed() const
	{
		float fBonus(0.0f);

		if (!IsIllusion())
		{
			for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
			{
				if (m_apInventory[iSlot] == NULL)
					continue;

				fBonus += m_apInventory[iSlot]->GetAttackSpeed();
			}
		}

		float fSlowResistance(0.0f);

		float fAttackSpeed((GetBaseAttackSpeed() + fBonus) * MAX(0.0f, GetAttackSpeedMultiplier()));

		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			fAttackSpeed *= (1.0f - m_apInventory[iSlot]->GetAttackSpeedSlow() * (1.0f - fSlowResistance));
		}

		return fAttackSpeed;
	}

	STATIC_ATTRIBUTE(float, CastSpeed, ADD, 1.0f, false)
	STATIC_ATTRIBUTE(float, CooldownSpeed, ADD, 1.0f, false)
	STATIC_ATTRIBUTE(float, ReducedCooldowns, ADD, 0.0f, false)
	STATIC_ATTRIBUTE(float, IncreasedCooldowns, ADD, 0.0f, false)
	STATIC_ATTRIBUTE(float, BaseDamageMultiplier, ADD, 1.0f, true)
	STATIC_ATTRIBUTE(float, TotalDamageMultiplier, ADD, 1.0f, true)
	STATIC_ATTRIBUTE(float, BonusDamage, ADD, 0.0f, false)
	MULTI_LEVEL_ATTRIBUTE(float, LifeSteal, ADD, false)
	STATIC_ATTRIBUTE(float, IncomingDamageMultiplier, ADD, 1.0f, true)
	STATIC_ATTRIBUTE(float, DebuffDurationMultiplier, ADD, 1.0f, true)
	STATIC_ATTRIBUTE(float, HealMultiplier, ADD, 1.0f, false)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, AttackProjectile)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackType)

	MUTABLE_MULTI_LEVEL_INITIAL_ATTRIBUTE(uint, AttackEffectType)
	virtual float	GetAttackEffectType() const;
	
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackDamageType)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackCooldown)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackDuration)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackActionTime)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, ThreatScheme)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, ThreatEffectType)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE_OVERRIDE(float, AggroRange, m_uiOverrideAggroRange, 0)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AggroScheme)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, ProximityRange)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, ProximityScheme)

	STATIC_ATTRIBUTE(float, EvasionRanged, MAX, 0.0f, true)
	STATIC_ATTRIBUTE(float, EvasionMelee, MAX, 0.0f, true)
	STATIC_ATTRIBUTE(float, MissChance, MAX, 0.0f, true)

	MULTI_LEVEL_ATTRIBUTE(float, HealthRegenReduction, ADD, true)
	MULTI_LEVEL_ATTRIBUTE(float, ManaRegenReduction, ADD, true)
	MULTI_LEVEL_ATTRIBUTE(float, StaminaRegenReduction, ADD, true)

	BOOL_ATTRIBUTE(Stunned, true)
	BOOL_ATTRIBUTE(Silenced, true)
	BOOL_ATTRIBUTE(Perplexed, true)
	BOOL_ATTRIBUTE(Disarmed, true)

	bool	IsImmobilized(float bCheckMove = true, float bCheckRotate = false) const
	{
		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] == NULL)
				continue;

			if (bCheckMove && (m_apInventory[iSlot]->GetImmobilized() || m_apInventory[iSlot]->GetImmobilized2()))
				return true;
			if (bCheckRotate && m_apInventory[iSlot]->GetImmobilized())
				return true;
		}

		return false;
	}

	BOOL_ATTRIBUTE(Restrained, true)
	BOOL_ATTRIBUTE(Revealed, true)
	BOOL_ATTRIBUTE(Frozen, true)
	BOOL_ATTRIBUTE(Isolated, true)
	BOOL_ATTRIBUTE(FreeCast, true)
	MULTI_LEVEL_ATTRIBUTE(bool, ClearVision, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Invulnerable, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Deniable, OR, true)
	MULTI_LEVEL_ATTRIBUTE(float, DeniablePercent, MAX, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Smackable, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, NoThreat, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, TrueStrike, OR, true)
	
	ENTITY_DEFINITION_ACCESSOR(bool, AlwaysVisible)
	ENTITY_DEFINITION_ACCESSOR(bool, Hidden)
	ENTITY_DEFINITION_ACCESSOR(bool, AlwaysTargetable)

	STATIC_ATTRIBUTE(float, Counter, ADD, 0.0f, false)

	virtual void	GetCriticals(CCombatEvent &cmbt) const;
	virtual void	GetAttackActions(EEntityActionScript eScriptFrom, EEntityActionScript eScriptTo, CCombatEvent &cmbt) const;
	virtual float	GetDeflection() const;
	
	virtual void			UpdateModifiers();
	virtual void			SetPersistentModifierKeys(const uivector &vModifiers)	{ m_vPersistentModifierKeys = vModifiers; }
	GAME_SHARED_API void	GetSlaveModifiers(uivector &vModifierKeys);

	float					GetHealth() const						{ return m_fHealth; }
	virtual float			GetHealthPercent() const				{ return GetMaxHealth() != 0.0f ? GetHealth() / GetMaxHealth() : 1.0f; }
	float					GetHealthShadow() const					{ return m_fHealthShadowMarker; }
	void					SetHealth(float fHealth)				{ m_fHealth = CLAMP(fHealth, 0.0f, GetMaxHealth()); }
	void					RegenerateHealth(float fFrameTime);
	virtual void			Heal(float fHealth);
	virtual void			ChangeHealth(float fHealth);

	virtual float			GetMana() const							{ return m_fMana; }
	float					GetManaPercent() const					{ return GetMaxMana() != 0.0f ? GetMana() / GetMaxMana() : 1.0f; }
	float					GetManaShadow() const					{ return m_fManaShadowMarker; }
	void					SetMana(float fValue)					{ m_fMana = CLAMP(fValue, 0.0f, GetMaxMana()); }
	void					GiveMana(float fValue);
	bool					SpendMana(float fCost);
	void					TakeMana(float fValue);
	void					RegenerateMana(float fFrameTime);

	virtual float			GetStamina() const						{ return m_fStamina; }
	float					GetStaminaPercent() const				{ return GetMaxStamina() != 0.0f ? GetStamina() / GetMaxStamina() : 1.0f; }
	void					SetStamina(float fValue)				{ m_fStamina = CLAMP(fValue, 0.0f, GetMaxStamina()); }
	void					GiveStamina(float fValue);
	bool					SpendStamina(float fCost);
	void					TakeStamina(float fValue);
	void					RegenerateStamina(float fFrameTime);

	virtual float			GetBaseDamage() const			{ return M_Randnum(GetAdjustedAttackDamageMin(), GetAdjustedAttackDamageMax()); }

	virtual void			GetAnimState(int iChannel, int &iAnim, byte &ySequence, float &fSpeed);
	bool					IsPlayingAnim(int iChannel, const tstring &sAnimName, int iNumAnims = 1);

	void					SetFadeStartTime(uint uiTime)			{ m_uiFadeStartTime = uiTime; }
	uint					GetFadeStartTime() const				{ return m_uiFadeStartTime; }

	virtual uint			GetStealthBits() const;
	virtual float			GetStealthFade() const;
	virtual float			GetMaxStealthFade() const;
	virtual bool			IsStealth(bool bCheckRevealed = true) const;
	virtual float			GetMinStealthProximity() const;
	virtual bool			CanSee(const IVisualEntity *pTarget) const;

	GAME_SHARED_API void			UpdateInventory();

	// States
	GAME_SHARED_API void			ClearStates();
	GAME_SHARED_API IEntityState*	ApplyState(ushort unID, uint uiLevel, uint uiStartTime, uint uiDuration, uint uiInflictorIndex = INVALID_INDEX, uint uiProxyUID = INVALID_INDEX, EStateStackType eStack = STATE_STACK_NONE, uint uiSpawnerUID = INVALID_INDEX);
	GAME_SHARED_API IEntityState*	TransferState(IEntityState *pState);
	GAME_SHARED_API void			RemoveState(int iSlot);
	GAME_SHARED_API void			RemoveState(IEntityState *pState);
	GAME_SHARED_API void			ExpireState(int iSlot);
	GAME_SHARED_API void			ExpireState(ushort unID);
	GAME_SHARED_API bool			HasState(ushort unID);

	IEntityState*	GetMorphState() const;

	GAME_SHARED_API IGadgetEntity*	AttachGadget(ushort unID, uint uiLevel, uint uiStartTime, uint uiDuration, IUnitEntity *pOwner);

	// Client-side state management
	GAME_SHARED_API void	AddState(IEntityState *pState);

	IEntityState*			GetState(int iSlot)
	{
		if (iSlot >= INVENTORY_START_STATES &&
			iSlot <= INVENTORY_END_STATES &&
			m_apInventory[iSlot] != NULL)
			return m_apInventory[iSlot]->GetAsState();
		return NULL;
	}
	GAME_SHARED_API uint	GetStateExpireTime(int iSlot);
	GAME_SHARED_API float	GetStateExpirePercent(int iSlot);

	void					SetState(int iSlot, IEntityState *pState)	{ if (iSlot < INVENTORY_START_STATES || iSlot > INVENTORY_END_STATES) return; m_apInventory[iSlot] = pState; }

	virtual bool			IsTargetType(const tstring &sType, const IUnitEntity *pInitiator) const;
	virtual bool			IsTargetType(const CTargetScheme::STestRecord &test, const IUnitEntity *pInitiator) const;
	virtual bool			IsTargetType(ETargetTrait eTrait, const IUnitEntity *pInitiator) const;
	virtual bool			IsGlobalCondition(EGlobalCondition eGlobal) const;
	virtual bool			IsAttribute(EAttribute eAttribute) const	{ return false; }

	virtual void			AddBonusDamage(float fDamage)				{ m_fBonusDamage += fDamage; }
	virtual void			AddBonusDamageMultiplier(float fMultiplier)	{ m_fBonusDamageMultiplier += fMultiplier; }
	virtual void			ClearBonusDamage()							{ m_fBonusDamage = 0.0f; m_fBonusDamageMultiplier = 0.0f;}

	void					AddSoulLink(uint uiUID)						{ m_setSoulLinks.insert(uiUID); }
	const uiset&			GetSoulLinks() const						{ return m_setSoulLinks; }
	void					RemoveSoulLink(uint uiUID)					{ m_setSoulLinks.erase(uiUID); }

	virtual void			FlushStats();

	virtual bool			ServerFrameSetup();
	virtual bool			ServerFrameThink();
	virtual bool			ServerFrameMovement();
	virtual bool			ServerFrameAction();
	virtual bool			ServerFrameCleanup();

	uint					GetCombatTypeIndex()			{ return m_uiCombatType; }

	void	RemoveSlave(int iSlot)
	{
		ISlaveEntity *pSlave(GetSlave(iSlot));
		if (pSlave == NULL)
			return;
		if (pSlave->IsItem())
			RemoveItem(iSlot);
		else if (pSlave->IsState())
			RemoveState(iSlot);
	}

	GAME_SHARED_API void	Action(EEntityActionScript eAction, IUnitEntity *pTarget, IGameEntity *pInflictor, CCombatEvent *pCombatEvent = NULL, CDamageEvent *pDamageEvent = NULL);
	GAME_SHARED_API void	Action(EEntityActionScript eAction, const CVec3f &v3Target, IGameEntity *pInflictor, CCombatEvent *pCombatEvent = NULL, CDamageEvent *pDamageEvent = NULL);

	// Inventory
	GAME_SHARED_API virtual void	ClearInventory();
	GAME_SHARED_API virtual void	RemoveItem(int iSlot);
	GAME_SHARED_API virtual void	RemoveItemByIndex(uint uiIndex);

	GAME_SHARED_API virtual bool	CanGiveItem(IEntityItem *pItem, IUnitEntity *pTarget);
	GAME_SHARED_API virtual bool	CanCarryItem(IEntityItem *pItem);
	GAME_SHARED_API virtual bool	CanSellItem(IEntityItem *pItem, int iClientNum);

	IEntityTool*			GetTool(int iSlot) const
	{
		if (iSlot >= 0 &&
			iSlot <= MAX_INVENTORY &&
			m_apInventory[iSlot] != NULL)
			return m_apInventory[iSlot]->GetAsTool();

		return NULL;
	}
	IEntityAbility*			GetAbility(int iSlot) const
	{
		if (iSlot >= 0 &&
			iSlot <= MAX_INVENTORY &&
			m_apInventory[iSlot] != NULL)
			return m_apInventory[iSlot]->GetAsAbility();

		return NULL;
	}
	IEntityItem*			GetItem(int iSlot) const
	{
		if (iSlot >= 0 &&
			iSlot <= MAX_INVENTORY &&
			m_apInventory[iSlot] != NULL)
			return m_apInventory[iSlot]->GetAsItem();

		return NULL;
	}
	ISlaveEntity*			GetSlave(int iSlot) const
	{
		if (iSlot >= 0 &&
			iSlot <= MAX_INVENTORY)
			return m_apInventory[iSlot];

		return NULL;
	}
	GAME_SHARED_API	IEntityTool*	GiveItem(int iSlot, ushort unID, bool bEnabled = true);
	GAME_SHARED_API int				TransferItem(int iClientNum, IEntityItem *pItem, int iSlot = -1);
	GAME_SHARED_API bool			CloneItem(IEntityItem *pItem);
	GAME_SHARED_API void			SwapItem(int iClientNum, int iSlot1, int iSlot2);
	GAME_SHARED_API void			DisassembleItem(int iSlot);

	void					SetExclusiveAttackModSlot(int iSlot)	{ m_iExclusiveAttackModSlot = iSlot; }
	int						GetExclusiveAttackModSlot() const		{ return m_iExclusiveAttackModSlot; }
	GAME_SHARED_API int		GetNextExclusiveAttackModSlot() const;
	GAME_SHARED_API int		GetPrevExclusiveAttackModSlot() const;
	GAME_SHARED_API void	ValidateExclusiveAttackModSlot();

	void					SetInventorySlot(int iSlot, ISlaveEntity *pSlave) { if (iSlot >= 0 && iSlot < MAX_INVENTORY) m_apInventory[iSlot] = pSlave; }
	ISlaveEntity*			GetInventorySlot(int iSlot)		{ if (iSlot >= 0 && iSlot < MAX_INVENTORY) return m_apInventory[iSlot]; else return NULL; }

	virtual void			AttachModel(const tstring &sBoneName, ResHandle hModel);

	virtual CVec4f			GetMapIconColor(CPlayer *pLocalPlayer) const;
	virtual	CVec4f			GetTeamColor(CPlayer *pLocalPlayer) const;
	virtual float			GetMapIconSize(CPlayer *pLocalPlayer) const;
	virtual ResHandle		GetMapIcon(CPlayer *pLocalPlayer) const;

	ENTITY_DEFINITION_ACCESSOR(bool, DrawOnMap)
	ENTITY_DEFINITION_ACCESSOR(bool, PartialControlShare)

	virtual bool			IsVisibleOnMap(CPlayer *pLocalPlayer) const		{ return IVisualEntity::IsVisibleOnMap(pLocalPlayer) && GetDrawOnMap(); }

	virtual bool			AddToScene(const CVec4f &v4Color, int iFlags);
	virtual void			UpdateSkeleton(bool bPose);

	virtual void			Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);
	virtual void			UpdateEffectThreadSource(CEffectThread *pEffectThread);
	virtual void			UpdateEffectThreadTarget(CEffectThread *pEffectThread);

	ENTITY_DEFINITION_ACCESSOR(const tstring&, DeniedAnim)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, GibAnim)

	// Combat
	void					SetTargetUID(uint uiTargetUID)			{ m_uiTargetUID = uiTargetUID; }
	uint					GetTargetUID() const					{ return m_uiTargetUID; }
	bool					ShouldTarget(IGameEntity *pOther);
	GAME_SHARED_API	bool	IsEnemy(IUnitEntity *pOther) const;
	virtual float			GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget);
	virtual bool			IsAttacking() const;
	virtual bool			IsIdle() const;
	virtual bool			IsTraveling() const;

	void					SetOwnerClientNumber(int iClientNumber)	{ m_iOwnerClientNumber = iClientNumber; }
	int						GetOwnerClientNumber() const			{ return m_iOwnerClientNumber; }
	CPlayer*				GetOwnerPlayer() const					{ return Game.GetPlayer(m_iOwnerClientNumber); }
	GAME_SHARED_API bool	CanReceiveOrdersFrom(int iClientNumber);
	inline bool				CanActOnOrdersFrom(int iClientNumber)	{ return (GetStatus() == ENTITY_STATUS_ACTIVE) && CanReceiveOrdersFrom(iClientNumber) && !HasUnitFlags(UNIT_FLAG_NOT_CONTROLLABLE); }

	void					SetOwnerIndex(uint uiIndex)				{ m_uiOwnerEntityIndex = uiIndex; }
	IUnitEntity*			GetOwner() const						{ return Game.GetUnitEntity(m_uiOwnerEntityIndex); }
	uint					GetOwnerIndex() const					{ return m_uiOwnerEntityIndex; }
	IUnitEntity*			GetMasterOwner() const					{ IUnitEntity *pResult(const_cast<IUnitEntity*>(this)); while (pResult->GetOwner() != NULL) pResult = pResult->GetOwner(); return pResult; }

	GAME_SHARED_API uint	PlayerCommand(const SUnitCommand &cCmd);

	CVec2f					GetHeading(float fVecLength);
	bool					JustWalkNike(const CVec2f &v2MovementVector, CPlane &plOutImpactPlane, bool bDirectPathing);
	virtual bool			StartAttack(IUnitEntity* pTarget, bool bAbility, bool bAggro);
	virtual bool			Attack(IUnitEntity* pTarget, bool bAttackAbility);
	virtual IProjectile*	CreateProjectile(const tstring &sName, uint uiTargetIndex, uint uiLevel);
	virtual IProjectile*	CreateProjectile(const tstring &sName, const CVec3f &v3TargetPosition, uint uiLevel);

	void					Push(const CVec2f &v2Velocity, uint uiDuration);

	virtual void			Interrupt(EUnitAction eAction);
	virtual bool			IsChanneling(EUnitAction eAction);

	void					ValidatePosition(uint uiIgnoreSurfaces);

	bool					IsAttackReady() const					{ return GetAttackCooldown() == 0 || m_uiAttackCooldownTime == INVALID_TIME || HasUnitFlags(UNIT_FLAG_IGNORE_ATTACK_COOLDOWN); }
	void					SetAttackCooldownTime(uint uiTime)		{ m_uiAttackCooldownTime = uiTime; }

	uint					GetAttackSequence() const				{ return m_uiAttackSequence; }

	// Operators
	GAME_SHARED_API virtual void	Copy(const IGameEntity &B);

	MUTABLE_MULTI_LEVEL_RESOURCE(Icon)
	MUTABLE_MULTI_LEVEL_RESOURCE(Portrait)
	MUTABLE_MULTI_LEVEL_RESOURCE(Model)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Skin)
	MUTABLE_MULTI_LEVEL_RESOURCE(MapIconProperty)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(CVec4f, MapIconColorProperty)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, MapIconSizeProperty)

	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, ModelScale)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, EffectScale)

	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, BoundsRadius)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, BoundsHeight)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(SelectedSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(SelectedFlavorSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(ConfirmMoveSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(ConfirmAttackSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(TauntedSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(TauntKillSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(NoManaSound)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(CooldownSound)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, SelectionRadius)
	MUTABLE_ATTRIBUTE(bool, IsSelectable)
	BASE_ATTRIBUTE(bool, NoCorpse)
	void					SetNoCorpse(bool bNoCorpse)
	{
		if (bNoCorpse)
			m_uiDeathFlags |= DEATH_FLAG_NOCORPSE; 
		else
			m_uiDeathFlags &= ~DEATH_FLAG_NOCORPSE; 
	}
	virtual bool			GetNoCorpse() const
	{ 
		if (m_uiDeathFlags & DEATH_FLAG_NOCORPSE)
			return true;

		return GetBaseNoCorpse();
	}

	BASE_ENTITY_DEFINITION_ACCESSOR(bool, IsControllable)

	virtual bool			GetIsControllable() const
	{
		return GetBaseIsControllable() && !HasUnitFlags(UNIT_FLAG_UNCONTROLLABLE);
	}

	ENTITY_DEFINITION_ACCESSOR(bool, IsUnit)
	ENTITY_DEFINITION_ACCESSOR(bool, NoGlobalSelect)
	ENTITY_DEFINITION_ACCESSOR(bool, NoBlockNeutralSpawn)

	PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackDamageMin)
	PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackDamageMax)
	float	GetAdjustedAttackDamageMin() const	{ return GetAttackDamageMin() * GetBaseDamageMultiplier(); }
	float	GetAdjustedAttackDamageMax() const	{ return GetAttackDamageMax() * GetBaseDamageMultiplier(); }

	ENTITY_DEFINITION_ACCESSOR(float, TurnRate)
	ENTITY_DEFINITION_ACCESSOR(float, TurnSmoothing)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, AttackRangeBuffer)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(CVec3f, AttackOffset)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(CVec3f, TargetOffset)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(uint, AttackTargetScheme)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(bool, AttackNonLethal)
	MUTABLE_MULTI_LEVEL_RESOURCE(AttackStartEffect)
	MUTABLE_MULTI_LEVEL_RESOURCE(AttackActionEffect)
	MUTABLE_MULTI_LEVEL_RESOURCE(AttackImpactEffect)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, PreGlobalScale)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, InfoHeight)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, CombatType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, RevealRange)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, RevealType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, StealthType)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, FadeTime)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, StealthProximity)
	MUTABLE_MULTI_LEVEL_RESOURCE(PassiveEffect)
	MUTABLE_MULTI_LEVEL_RESOURCE(SpawnEffect)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, CanAttack)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IsMobile)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, CanRotate)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Blocking)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AntiBlocking)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImmunityType)
	virtual uint	GetAdjustedImmunityType() const;

	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, SightRangeDay)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(float, SightRangeNight)
	virtual float	GetSightRange() const				{ if (Game.IsNight()) return GetSightRangeNight(); return GetSightRangeDay(); }

	MUTABLE_MULTI_LEVEL_ATTRIBUTE(EUnitCommand, DefaultBehavior)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, WanderRange)

	virtual ushort	GetGoldBounty() const				{ return M_Randnum(GetGoldBountyMin(), GetGoldBountyMax()); }
	virtual ushort	GetGoldBountyRadiusAmount() const	{ return 0; }
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, GoldBountyMin)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, GoldBountyMax)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, GoldBountyTeam)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, GoldBountyConsolation)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ExperienceBounty)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, GlobalExperience)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DeadExperience)
	virtual float	GetUnsharedExperienceBounty() const	{ return 0.0f; }

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, CanCarryItems)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DropItemsOnDeath)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, PassiveInventory)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory0)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory1)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory2)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory3)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory4)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory5)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory6)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory7)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, Inventory8)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, SharedInventory0)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, SharedInventory1)
	MUTABLE_MULTI_LEVEL_ATTRIBUTE(const tstring&, SharedInventory2)

	ENTITY_DEFINITION_ACCESSOR(uint, CorpseTime)
	ENTITY_DEFINITION_ACCESSOR(uint, CorpseFadeTime)
	ENTITY_DEFINITION_RESOURCE_ACCESSOR(CorpseFadeEffect)

	ENTITY_DEFINITION_ACCESSOR(const tstring&, AttackAnim)
	ENTITY_DEFINITION_ACCESSOR(uint, AttackNumAnims)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, DeathAnim)
	ENTITY_DEFINITION_ACCESSOR(uint, DeathNumAnims)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, AltDeathAnim)
	ENTITY_DEFINITION_ACCESSOR(uint, AltDeathNumAnims)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, ExpireAnim)
	ENTITY_DEFINITION_ACCESSOR(uint, ExpireNumAnims)
	ENTITY_DEFINITION_ACCESSOR(uint, DeathTime)

	ENTITY_DEFINITION_ACCESSOR(float, TiltFactor)
	ENTITY_DEFINITION_ACCESSOR(float, TiltSpeed)
	ENTITY_DEFINITION_ACCESSOR(float, CorpseTiltFactor)
	ENTITY_DEFINITION_ACCESSOR(float, CorpseTiltSpeed)

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Flying)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, FlyHeight)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, GroundOffset)

	MULTI_LEVEL_BASE_ATTRIBUTE(bool, Unitwalking)
	virtual bool	GetUnitwalking() const
	{
		if (HasUnitFlags(UNIT_FLAG_BOUND))
			return true;

		for (int iSlot(INVENTORY_START_ACTIVE); iSlot <= INVENTORY_END_ACTIVE; ++iSlot)
		{
			if (m_apInventory[iSlot] != NULL && m_apInventory[iSlot]->GetUnitwalking())
				return true;
		}

		return GetBaseUnitwalking();
	}

	MULTI_LEVEL_ATTRIBUTE(bool, Treewalking, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Cliffwalking, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Buildingwalking, OR, true)
	MULTI_LEVEL_ATTRIBUTE(bool, Antiwalking, OR, true)

	virtual uint			GetAdjustedAttackActionTime() const				{ uint uiBase(GetAttackActionTime()); return INT_ROUND(uiBase / GetAttackSpeed()); }
	virtual uint			GetAdjustedAttackDuration() const				{ uint uiBase(GetAttackDuration()); return INT_ROUND(uiBase / GetAttackSpeed()); }
	virtual uint			GetAdjustedAttackCooldown() const				{ uint uiBase(GetAttackCooldown()); return INT_ROUND(uiBase / GetAttackSpeed()); }

	virtual IUnitEntity*	SpawnIllusion(const CVec3f &v3Position, const CVec3f &v3Angles, uint uiLifetime, 
										float fReceiveDamageMultiplier, float fInflictDamageMultiplier, 
										ResHandle hSpawnEffect, ResHandle hDeathEffect, 
										bool bDeathAnim, bool bInheritActions);

	bool					IsIllusion() const								{ return HasUnitFlags(UNIT_FLAG_ILLUSION); }

	void					SetLifetime(uint uiSpawnTime, uint uiLifetime)	{ m_uiSpawnTime = uiSpawnTime; m_uiLifetime = uiLifetime; }
	virtual uint			GetActualLifetime() const						{ return m_uiLifetime; }

	void					SetReceiveDamageMultiplier(float fMultiplier)	{ m_fReceiveDamageMultiplier = fMultiplier; }
	virtual float			GetReceiveDamageMultiplier() const				{ return m_fReceiveDamageMultiplier; }

	void					SetInflictDamageMultiplier(float fMultiplier)	{ m_fInflictDamageMultiplier = fMultiplier; }
	virtual float			GetInflictDamageMultiplier() const				{ return m_fInflictDamageMultiplier; }

	void					SetNoDeathAnim(bool bNoDeathAnim)				{ if (bNoDeathAnim) m_uiDeathFlags |= DEATH_FLAG_NODEATHANIM; else m_uiDeathFlags &= ~DEATH_FLAG_NODEATHANIM; }
	virtual bool			GetNoDeathAnim() const							{ return (m_uiDeathFlags & DEATH_FLAG_NODEATHANIM) != 0; }

	void					SetDeath(bool bDeath)							{ if (bDeath) m_uiDeathFlags |= DEATH_FLAG_FORCE; else m_uiDeathFlags &= ~DEATH_FLAG_FORCE; }
	virtual bool			GetDeath() const								{ return (m_uiDeathFlags & DEATH_FLAG_FORCE) != 0; }

	void					SetProtectedDeath(bool bProtectedDeath)			{ if (bProtectedDeath) m_uiDeathFlags |= DEATH_FLAG_PROTECTED; else m_uiDeathFlags &= ~DEATH_FLAG_PROTECTED; }
	virtual bool			GetProtectedDeath() const						{ return (m_uiDeathFlags & DEATH_FLAG_PROTECTED) != 0; }

	void					SetExpire(bool bExpire)							{ if (bExpire) m_uiDeathFlags |= DEATH_FLAG_EXPIRE; else m_uiDeathFlags &= ~DEATH_FLAG_EXPIRE; }
	virtual bool			GetExpire() const								{ return (m_uiDeathFlags & DEATH_FLAG_EXPIRE) != 0; }

	void					SetIllusionDeathAnim(bool bIllusionDeathAnim)	{ if (bIllusionDeathAnim) m_uiMiscFlags |= MISC_FLAG_ILLUSIONANIM; else m_uiDeathFlags &= ~MISC_FLAG_ILLUSIONANIM; }
	virtual bool			GetIllusionDeathAnim() const					{ return (m_uiMiscFlags & MISC_FLAG_ILLUSIONANIM) != 0; }

	void					SetExperienceBountyMultipier(float fMultiplier)	{ m_fExperienceBountyMultiplier = fMultiplier; }
	virtual float			GetExperienceBountyMultiplier() const			{ return m_fExperienceBountyMultiplier; }

	void					SetGoldBountyMultiplier(float fMultiplier)		{ m_fGoldBountyMultiplier = fMultiplier; }
	virtual float			GetGoldBountyMultiplier() const					{ return m_fGoldBountyMultiplier; }

	GAME_SHARED_API uint	GetRemainingLifetime() const;
	float					GetRemainingLifetimePercent() const	{ if (GetActualLifetime() == 0) return 1.0f; else return GetRemainingLifetime() / float(GetActualLifetime()); }

	ENTITY_DEFINITION_ACCESSOR(const tstring&, Description)
	ENTITY_DEFINITION_ACCESSOR(const tstring&, DisplayName)

	GAME_SHARED_API int		CheckRecipes(int iTestSlot);

	MULTI_LEVEL_ATTRIBUTE(tstring, ShopAccess, CONCAT, false)
	MULTI_LEVEL_ATTRIBUTE(tstring, RemoteShopAccess, CONCAT, false)
	MULTI_LEVEL_ATTRIBUTE(tstring, SharedShopAccess, CONCAT, false)
	MULTI_LEVEL_ATTRIBUTE(tstring, SharedRemoteShopAccess, CONCAT, false)
	MULTI_LEVEL_ATTRIBUTE(tstring, RestrictItemAccess, CONCAT, false)
	MULTI_LEVEL_ATTRIBUTE(bool, StashAccess, OR, false)

	GAME_SHARED_API bool	CanAccessLocalShop(const tstring &sShopName);
	GAME_SHARED_API bool	CanAccessLocalShop(ushort unShopID)				{ return CanAccessLocalShop(EntityRegistry.LookupName(unShopID)); }
	GAME_SHARED_API bool	CanAccessShop(ushort unShopID);
	GAME_SHARED_API bool	CanAccessShop(const tstring &sShopName)			{ return CanAccessShop(EntityRegistry.LookupID(sShopName)); }
	GAME_SHARED_API bool	CanAccessItem(const tstring &sItem);
	GAME_SHARED_API bool	CanAccessItemLocal(const tstring &sItem);
	GAME_SHARED_API void	GetAccessableShopList(set<CShopDefinition*> &setShops);
	GAME_SHARED_API bool	GetAccessableShop(const tstring &sItem, ushort &unShop, int &iSlot);
	GAME_SHARED_API void	GetLocalShopList(set<CShopDefinition*> &setShops);
	GAME_SHARED_API void	GetRemoteShopList(set<CShopDefinition*> &setShops);

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DieWithOwner)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, RelayExperience)
	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxDistanceFromOwner)

	uint					GetDisjointSequence() const						{ return m_uiDisjointSequence; }
	void					IncDisjointSequence()							{ ++m_uiDisjointSequence; }

	uint					GetArmingSequence() const						{ return m_uiArmingSequence; }
	void					IncArmingSequence()								{ ++m_uiArmingSequence; }

	GAME_SHARED_API void	SetLastAggression(int iTeam, uint uiTime);
	GAME_SHARED_API uint	GetLastAggression(int iTeam) const;

	GAME_SHARED_API void	CallForHelp(float fRange, IUnitEntity *pAttacker);
	virtual void			Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker);

	// Client-side mount management
	void					SetMount(IGadgetEntity *pMount)					{ m_pMount = pMount; }
	IGadgetEntity*			GetMount()										{ return m_pMount; }

	GAME_SHARED_API void	PlayRecipeEffect();

	virtual CVec4f			GetSelectionColor(CPlayer *pLocalPlayer);

	void					SetCurrentDamage(float fDamage)					{ m_fCurrentDamage = fDamage; }
	float					GetCurrentDamage() const						{ return m_fCurrentDamage; }
	void					ScaleCurrentDamage(float fScale)				{ m_fCurrentDamage *= fScale; }
	ESuperType				GetCurrentDamageSuperType() const				{ return m_eCurrentDamageSuperType; }
	uint					GetCurrentDamageEffectType() const				{ return m_uiCurrentDamageEffectType; }

	float					GetMovementDistance() const						{ return m_fMovementDistance; }

	void					SetDeathEffect(ResHandle hDeathEffect)			{ m_hDeathEffect = hDeathEffect; }

	vector<PoolHandle>&		GetPathBlockers()								{ return m_vPathBlockers; }
	const CVec2f&			GetBlockPosition() const						{ return m_v2BlockPosition; }

	virtual void			Moved();

	void					SetLastHeroAttackTime(uint uiGameTime)			{ m_uiLastHeroAttackTime = uiGameTime; }
	uint					GetLastHeroAttackTime() const					{ return m_uiLastHeroAttackTime; }

	void					AggroCreeps(float fRange, uint uiDuration, byte yTeam, uint uiDelay = 0, bool bReaggroBlock = false);
	void					Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay = 0, bool bReaggroBlock = false);

	CBrain&					GetBrain()										{ return m_cBrain; }

	GAME_SHARED_API virtual void	SnapshotUpdate();

	GAME_SHARED_API void	StartCooldown(uint uiCooldownType, uint uiStartTime, uint uiDuration);
	GAME_SHARED_API void	ReduceCooldown(uint uiCooldownType, uint uiDuration);
	GAME_SHARED_API void	ResetCooldown(uint uiCooldownType);
	GAME_SHARED_API void	GetCooldown(uint uiCooldownType, uint &uiStartTime, uint &uiDuration);
	GAME_SHARED_API void	ResetCooldowns();

	void					SetProxyUID(uint uiUID)				{ m_uiProxyUID = uiUID; }
	uint					GetProxyUID() const					{ return m_uiProxyUID; }
	IGameEntity*			GetProxy(uint uiIndex) const		{ return Game.GetEntityFromUniqueID(m_uiProxyUID); }

	void					SetAttackProjectile(ushort unType)			{ m_unAttackProjectile = unType; }
	void					SetAttackActionEffect(ResHandle hEffect)	{ m_hAttackActionEffect = hEffect; }
	void					SetAttackImpactEffect(ResHandle hEffect)	{ m_hAttackImpactEffect = hEffect; }

	CCombatEvent&			GetCombatEvent()					{ return m_cCombatEvent; }

	void					SetLastAttackTarget(uint uiUID, uint uiTime)	{ m_uiLastAttackTargetUID = uiUID; m_uiLastAttackTargetTime = uiTime; }
	uint					GetLastAttackTargetUID() const					{ return m_uiLastAttackTargetUID; }
	uint					GetLastAttackTargetTime() const					{ return m_uiLastAttackTargetTime; }

	uint					GetCurrentAttackStateTarget();
	uint					GetCurrentAttackBehaviorTarget();

	virtual uint			GetRepathTime() const					{ return unit_blockRepathTime; }
	virtual uint			GetRepathTimeExtra() const				{ return unit_blockRepathTimeExtra; }
	virtual float			GetSlideThreshold() const				{ return unit_slideThreshold; }

	bool					HasOrder(uint uiOrderSequence)					{ return m_cBrain.HasOrder(uiOrderSequence); }

	virtual uint			GetGuardChaseTime() const;
	virtual uint			GetGuardChaseDistance() const;
	virtual uint			GetGuardReaggroChaseTime() const;
	virtual uint			GetGuardReaggroChaseDistance() const;

	virtual void			SetGuardChaseTime(uint uiChaseTime)							{ m_uiGuardChaseTime = uiChaseTime; }
	virtual void			SetGuardChaseDistance(uint uiDistance)						{ m_uiGuardChaseDistance = uiDistance; }
	virtual void			SetGuardReaggroChaseTime(uint uiReaggroChaseTime)			{ m_uiGuardReaggroChaseTime = uiReaggroChaseTime; }
	virtual void			SetGuardReaggroChaseDistance(uint uiReaggroDistance)		{ m_uiGuardReaggroChaseDistance = uiReaggroDistance; }
	virtual void			OverrideAggroRange(uint uiAggroRange)						{ m_uiOverrideAggroRange = uiAggroRange; }

	virtual bool			GetUseAltDeathAnims() const									{ return m_bUseAltDeathAnims; }
	virtual void			SetUseAltDeathAnims(bool bUseAltDeathAnims)					{ m_bUseAltDeathAnims = bUseAltDeathAnims; }

	virtual void			OnTakeControl(IUnitEntity *pOwner)		{ }

	CVec3f					GetTransformedAttackOffset() const;
	CVec3f					GetTransformedTargetOffset() const;

	MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, PreferTouch)

	PROGRESSIVE_MULTI_LEVEL_ATTRIBUTE(float, Power, ADD, false)
};
//=============================================================================

#endif //__I_UNITENTITY_H__
