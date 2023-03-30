// (C)2007 S2 Games
// c_npcability.h
//
//=============================================================================
#ifndef __C_NPCABILITY_H__
#define __C_NPCABILITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_modifier.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
class CStateNpcAbility;
class CNpcAbilityEffect;
class IProjectile;
class IVisualEntity;
class IGameEntity;

typedef vector<CNpcAbilityEffect>	EffectVector;

enum ENpcAbilityType
{
	NPCABILITY_IMPULSE = 0,
	NPCABILITY_TOGGLE,
	NPCABILITY_PASSIVE
};

enum ENpcAttackType
{
	NPCATTACK_PROJECTILE = 0,
	NPCATTACK_TRACE,
	NPCATTACK_TRIGGER,
	NPCATTACK_SNAP,
	NPCATTACK_SELF
};

extern ushort		g_unAbilityEffectID;
//=============================================================================

//=============================================================================
// CNpcProjectile
//=============================================================================
class CNpcProjectile
{
private:
	ResHandle	m_hDeathEffect;
	float		m_fFriction;
	float		m_fGravity;
	uint		m_uiLifeTime;
	ResHandle	m_hModel;
	float		m_fScale;
	float		m_fSpeed;
	ResHandle	m_hTrailEffect;

public:
	~CNpcProjectile()	{}
	CNpcProjectile() :
	m_hDeathEffect(INVALID_RESOURCE),
	m_hModel(INVALID_RESOURCE),
	m_hTrailEffect(INVALID_RESOURCE)
	{}
	
	CNpcProjectile
	(
		ResHandle hDeathEffect,
		float fFriction,
		float fGravity,
		uint uiLifeTime,
		ResHandle hModel,
		float fScale,
		float fSpeed,
		ResHandle hTrailEffect
	) :
	m_hDeathEffect(hDeathEffect),
	m_fFriction(fFriction),
	m_fGravity(fGravity),
	m_uiLifeTime(uiLifeTime),
	m_hModel(hModel),
	m_fScale(fScale),
	m_fSpeed(fSpeed),
	m_hTrailEffect(hTrailEffect)
	{
	}

	IProjectile*	SpawnProjectile(IGameEntity *pOwner, const CVec3f &v3Start, const CVec3f &v3Forward) const;

	ResHandle	GetModel() const			{ return m_hModel; }
	ResHandle	GetDeathEffect() const		{ return m_hDeathEffect; }
	ResHandle	GetTrailEffect() const		{ return m_hTrailEffect; }
};
//=============================================================================

//=============================================================================
// CNpcAbilityEffect
//=============================================================================
class CNpcAbilityEffect
{
private:
	float		m_fProc;
	uint		m_uiDuration;
	ResHandle	m_hEffect;
	ResHandle	m_hIcon;
	tstring		m_sAnimName;
	float		m_fMinDamage;
	float		m_fMaxDamage;
	bool		m_bStun;
	bool		m_bStack;
	bool		m_bMeleeImpact;

	ushort		m_unID;

	FloatMod		m_modSpeed;
	FloatMod		m_modAttackSpeed;
	FloatMod		m_modDamage;
	FloatMod		m_modHealthRegen;
	FloatMod		m_modManaRegen;
	FloatMod		m_modStaminaRegen;
	FloatMod		m_modArmor;
	FloatMod		m_modAmmo;

public:
	~CNpcAbilityEffect()	{}
	
	CNpcAbilityEffect() :
	m_fProc(1.0f),
	m_uiDuration(0),
	m_hEffect(INVALID_RESOURCE),
	m_hIcon(INVALID_RESOURCE),
	m_fMinDamage(0.0f),
	m_fMaxDamage(0.0f),
	m_bStun(false),
	m_bStack(false),
	m_bMeleeImpact(false),
	m_unID(-1)
	{
	}
	
	void	SetProc(float fProc)								{ m_fProc = fProc; }
	void	SetDuration(uint uiDuration)						{ m_uiDuration = uiDuration; }
	void	SetIcon(ResHandle hIcon)							{ m_hIcon = hIcon; }
	void	SetEffect(ResHandle hEffect)						{ m_hEffect = hEffect; }
	void	SetAnimName(const tstring &sAnimName)				{ m_sAnimName = sAnimName; }
	void	SetDamage(float fMinDamage, float fMaxDamage)		{ m_fMinDamage = fMinDamage; m_fMaxDamage = fMaxDamage; }
	void	SetStun(bool bStun)									{ m_bStun = bStun; }
	void	SetStack(bool bStack)								{ m_bStack = bStack; }
	void	SetMeleeImpact(bool bMeleeImpact)					{ m_bMeleeImpact = bMeleeImpact; }
	void	SetSpeedMod(const FloatMod &modSpeed)				{ m_modSpeed = modSpeed; }
	void	SetAttackSpeedMod(const FloatMod &modAttackSpeed)	{ m_modAttackSpeed = modAttackSpeed; }
	void	SetDamageMod(const FloatMod &modDamage)				{ m_modDamage = modDamage; }
	void	SetHealthRegenMod(const FloatMod &modHealthRegen)	{ m_modHealthRegen = modHealthRegen; }
	void	SetManaRegenMod(const FloatMod &modManaRegen)		{ m_modManaRegen = modManaRegen; }
	void	SetStaminaRegenMod(const FloatMod &modStaminaRegen)	{ m_modStaminaRegen = modStaminaRegen; }
	void	SetArmorMod(const FloatMod &modArmor)				{ m_modArmor = modArmor; }
	void	SetAmmoMod(const FloatMod &modAmmo)					{ m_modAmmo = modAmmo; }
	void	SetID(ushort unID)									{ m_unID = unID; }

	void	ApplyEffect(uint uiTargetIndex, uint uiAttackerIndex) const;
	void	Activated(CStateNpcAbility *pState) const;

	ushort	GetID() const										{ return m_unID; }
	ResHandle	GetEffect() const								{ return m_hEffect; }
	ResHandle	GetIcon() const									{ return m_hIcon; }
};
//=============================================================================

//=============================================================================
// CNpcAbility
//=============================================================================
class CNpcAbility
{
private:
	// Ability properties
	tstring		m_sName;
	ENpcAbilityType	m_eType;
	ENpcAttackType	m_eAttackType;
	uint		m_uiActivationTime;
	uint		m_uiImpactTime;
	uint		m_uiCooldownTime;
	float		m_fRange;
	float		m_fManaCost;
	float		m_fHealthCost;
	float		m_fWeight;

	// Ability area of effect
	float		m_fEffectRadius;
	float		m_fAttackHeightMin;
	float		m_fAttackHeightMax;
	float		m_fAttackWidthMin;
	float		m_fAttackWidthMax;
	float		m_fAttackYawMin;
	float		m_fAttackYawMax;
	float		m_fAttackPitchMin;
	float		m_fAttackPitchMax;

	CNpcProjectile	m_cProjectile;

	tstring		m_sAnim;

	EffectVector	m_vSourceEffects;
	EffectVector	m_vTargetEffects;

	// Dynamic variables
	uint		m_uiLastActivationTime;

public:
	~CNpcAbility()	{}
	CNpcAbility()	{}
	CNpcAbility
	(
		const tstring &sName,
		ENpcAbilityType eType,
		ENpcAttackType eAttackType,
		uint uiActivationTime,
		uint uiImpactTime,
		uint uiCooldownTime,
		float fRange,
		float fManaCost,
		float fHealthCost,
		float fEffectRadius,
		float fAttackHeightMin,
		float fAttackHeightMax,
		float fAttackWidthMin,
		float fAttackWidthMax,
		float fAttackYawMin,
		float fAttackYawMax,
		float fAttackPitchMin,
		float fAttackPitchMax,
		const tstring &sAnim,
		float fWeight
	) :
	m_sName(sName),
	m_eType(eType),
	m_eAttackType(eAttackType),
	m_uiActivationTime(uiActivationTime),
	m_uiImpactTime(MIN(uiImpactTime, uiActivationTime)),
	m_uiCooldownTime(uiCooldownTime),
	m_fRange(fRange),
	m_fManaCost(fManaCost),
	m_fHealthCost(fHealthCost),
	m_fWeight(fWeight),
	m_fEffectRadius(fEffectRadius),
	m_fAttackHeightMin(fAttackHeightMin),
	m_fAttackHeightMax(fAttackHeightMax),
	m_fAttackWidthMin(fAttackWidthMin),
	m_fAttackWidthMax(fAttackWidthMax),
	m_fAttackYawMin(fAttackYawMin),
	m_fAttackYawMax(fAttackYawMax),
	m_fAttackPitchMin(fAttackPitchMin),
	m_fAttackPitchMax(fAttackPitchMax),
	m_sAnim(sAnim),

	m_uiLastActivationTime(INVALID_TIME)
	{
	}

	void	SetProjectile(const CNpcProjectile &cProjectile)
	{
		m_cProjectile = cProjectile;
	}
	
	void	AddSourceEffect(const CNpcAbilityEffect &cEffect)
	{
		m_vSourceEffects.push_back(cEffect);
	}

	void	AddTargetEffect(const CNpcAbilityEffect &cEffect)
	{
		m_vTargetEffects.push_back(cEffect);
	}
	
	const tstring&	GetName() const					{ return m_sName; }
	ENpcAbilityType	GetType() const					{ return m_eType; }
	ENpcAttackType	GetAttackType() const			{ return m_eAttackType; }
	uint		GetActivationTime() const			{ return m_uiActivationTime; }
	uint		GetImpactTime() const				{ return m_uiImpactTime; }
	uint		GetCooldownTime() const				{ return m_uiCooldownTime; }
	float		GetRange() const					{ return m_fRange; }
	float		GetManaCost() const					{ return m_fManaCost; }
	float		GetHealthCost() const				{ return m_fHealthCost; }
	float		GetEffectRadius() const				{ return m_fEffectRadius; }
	float		GetAttackHeightMin() const			{ return m_fAttackHeightMin; }
	float		GetAttackHeightMax() const			{ return m_fAttackHeightMax; }
	float		GetAttackWidthMin() const			{ return m_fAttackWidthMin; }
	float		GetAttackWidthMax() const			{ return m_fAttackWidthMax; }
	float		GetAttackYawMin() const				{ return m_fAttackYawMin; }
	float		GetAttackYawMax() const				{ return m_fAttackYawMax; }
	float		GetAttackPitchMin() const			{ return m_fAttackPitchMin; }
	float		GetAttackPitchMax() const			{ return m_fAttackPitchMax; }
	const CNpcProjectile&	GetProjectileDef() const	{ return m_cProjectile; }
	const tstring&	GetAnim() const					{ return m_sAnim; }
	float		GetWeight() const					{ return m_fWeight; }
	
	uint		GetNumSourceEffects() const				{ return uint(m_vSourceEffects.size()); }
	const CNpcAbilityEffect&	GetSourceEffect(uint uiIndex) const	{ return m_vSourceEffects[uiIndex]; }

	uint		GetNumTargetEffects() const				{ return uint(m_vTargetEffects.size()); }
	const CNpcAbilityEffect&	GetTargetEffect(uint uiIndex) const	{ return m_vTargetEffects[uiIndex]; }

	void		Activate(uint uiTime)				{ m_uiLastActivationTime = uiTime; }
	uint		GetLastActivationTime() const		{ return m_uiLastActivationTime; }
	bool		IsReady(uint uiTime) const			{ return m_uiLastActivationTime == INVALID_TIME || uiTime - m_uiLastActivationTime >= m_uiCooldownTime; }

	IProjectile*	SpawnProjectile(IGameEntity *pOwner, const CVec3f &v3Start, const CVec3f &v3Forward) const;
	void		Impact(uint uiOwnerIndex, uint uiTargetIndex) const;
	void		ImpactPosition(uint uiOwnerIndex, const CVec3f &v3Pos) const;
};
//=============================================================================

#endif //__C_NPCABILITY_H__
