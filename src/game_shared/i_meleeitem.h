// (C)2006 S2 Games
// i_meleeitem.h
//
//=============================================================================
#ifndef __I_MELEEITEM_H__
#define __I_MELEEITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CMeleeAttackEvent;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GET_MELEE_ITEM_TIME_VALUE(name) \
uint	Get##name(byte ySequence) \
{ \
	if (m_pEntityConfig == NULL) \
		return 0; \
\
	svector vTimes(TokenizeString(m_pEntityConfig->Get##name(), _T(','))); \
	if (vTimes.size() == 1) \
		return AtoI(vTimes[0]); \
	if (ySequence >= vTimes.size()) \
		return 0; \
	return AtoI(vTimes[ySequence]); \
}

#define GET_MELEE_ITEM_FLOAT_VALUE(name) \
float	Get##name(byte ySequence) \
{ \
	if (m_pEntityConfig == NULL) \
		return 0.0f; \
\
	svector vValues(TokenizeString(m_pEntityConfig->Get##name(), _T(','))); \
	if (vValues.size() == 1) \
		return AtoF(vValues[0]); \
	if (ySequence >= vValues.size()) \
		return 0.0f; \
	return AtoF(vValues[ySequence]); \
}
//=============================================================================

//=============================================================================
// IMeleeItem
//=============================================================================
class IMeleeItem : public IInventoryItem
{
protected:
	// Cvar settings
	START_ENTITY_CONFIG(IInventoryItem)
		DECLARE_ENTITY_CVAR(bool, ApplyAttributes)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackAnimName)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackAnimNameRunForward)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackAnimNameRunBack)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackAnimNameSprint)
		DECLARE_ENTITY_CVAR(int, QuickAttackNumAnims)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackTime)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackImpactTime)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackChainTime)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackImmobileTime)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackMinDamage)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackMaxDamage)
		DECLARE_ENTITY_CVAR(float, QuickAttackHeightMin)
		DECLARE_ENTITY_CVAR(float, QuickAttackHeightMax)
		DECLARE_ENTITY_CVAR(float, QuickAttackHeightStep)
		DECLARE_ENTITY_CVAR(float, QuickAttackAngleMin)
		DECLARE_ENTITY_CVAR(float, QuickAttackAngleMax)
		DECLARE_ENTITY_CVAR(float, QuickAttackAngleStep)
		DECLARE_ENTITY_CVAR(float, QuickAttackRangeMin)
		DECLARE_ENTITY_CVAR(float, QuickAttackRangeMax)
		DECLARE_ENTITY_CVAR(float, QuickAttackRangeStep)
		DECLARE_ENTITY_CVAR(float, QuickAttackPivotHeight)
		DECLARE_ENTITY_CVAR(float, QuickAttackPivotFactor)
		DECLARE_ENTITY_CVAR(tstring, QuickAttackStaminaCost)
		DECLARE_ENTITY_CVAR(CVec3f, QuickAttackPush)
		DECLARE_ENTITY_CVAR(CVec3f, QuickAttackLunge)
		DECLARE_ENTITY_CVAR(bool, QuickAttackBlockable)
		DECLARE_ENTITY_CVAR(bool, QuickAttackStaminaRequired)

		DECLARE_ENTITY_CVAR(tstring, JumpAttackAnimName)
		DECLARE_ENTITY_CVAR(int, JumpAttackNumAnims)
		DECLARE_ENTITY_CVAR(tstring, JumpAttackTime)
		DECLARE_ENTITY_CVAR(tstring, JumpAttackImpactTime)
		DECLARE_ENTITY_CVAR(tstring, JumpAttackChainTime)
		DECLARE_ENTITY_CVAR(uint, JumpAttackImmobileTime)
		DECLARE_ENTITY_CVAR(float, JumpAttackMinDamage)
		DECLARE_ENTITY_CVAR(float, JumpAttackMaxDamage)
		DECLARE_ENTITY_CVAR(float, JumpAttackHeightMin)
		DECLARE_ENTITY_CVAR(float, JumpAttackHeightMax)
		DECLARE_ENTITY_CVAR(float, JumpAttackHeightStep)
		DECLARE_ENTITY_CVAR(float, JumpAttackAngleMin)
		DECLARE_ENTITY_CVAR(float, JumpAttackAngleMax)
		DECLARE_ENTITY_CVAR(float, JumpAttackAngleStep)
		DECLARE_ENTITY_CVAR(float, JumpAttackRangeMin)
		DECLARE_ENTITY_CVAR(float, JumpAttackRangeMax)
		DECLARE_ENTITY_CVAR(float, JumpAttackRangeStep)
		DECLARE_ENTITY_CVAR(float, JumpAttackPivotHeight)
		DECLARE_ENTITY_CVAR(float, JumpAttackPivotFactor)
		DECLARE_ENTITY_CVAR(float, JumpAttackStaminaCost)
		DECLARE_ENTITY_CVAR(CVec3f, JumpAttackPush)
		DECLARE_ENTITY_CVAR(CVec3f, JumpAttackLunge)
		DECLARE_ENTITY_CVAR(bool, JumpAttackBlockable)
		DECLARE_ENTITY_CVAR(bool, JumpAttackEnabled)
		DECLARE_ENTITY_CVAR(bool, JumpAttackStaminaRequired)

		DECLARE_ENTITY_CVAR(tstring, StrongAttackAnimName)
		DECLARE_ENTITY_CVAR(int, StrongAttackNumAnims)
		DECLARE_ENTITY_CVAR(tstring, StrongAttackTime)
		DECLARE_ENTITY_CVAR(tstring, StrongAttackImpactTime)
		DECLARE_ENTITY_CVAR(tstring, StrongAttackChainTime)
		DECLARE_ENTITY_CVAR(uint, StrongAttackImmobileTime)
		DECLARE_ENTITY_CVAR(float, StrongAttackMinDamage)
		DECLARE_ENTITY_CVAR(float, StrongAttackMaxDamage)
		DECLARE_ENTITY_CVAR(float, StrongAttackHeightMin)
		DECLARE_ENTITY_CVAR(float, StrongAttackHeightMax)
		DECLARE_ENTITY_CVAR(float, StrongAttackHeightStep)
		DECLARE_ENTITY_CVAR(float, StrongAttackAngleMin)
		DECLARE_ENTITY_CVAR(float, StrongAttackAngleMax)
		DECLARE_ENTITY_CVAR(float, StrongAttackAngleStep)
		DECLARE_ENTITY_CVAR(float, StrongAttackRangeMin)
		DECLARE_ENTITY_CVAR(float, StrongAttackRangeMax)
		DECLARE_ENTITY_CVAR(float, StrongAttackRangeStep)
		DECLARE_ENTITY_CVAR(float, StrongAttackPivotHeight)
		DECLARE_ENTITY_CVAR(float, StrongAttackPivotFactor)
		DECLARE_ENTITY_CVAR(float, StrongAttackStaminaCost)
		DECLARE_ENTITY_CVAR(CVec3f, StrongAttackPush)
		DECLARE_ENTITY_CVAR(CVec3f, StrongAttackLunge)
		DECLARE_ENTITY_CVAR(bool, StrongAttackStaminaRequired)

		DECLARE_ENTITY_CVAR(tstring, BlockAnimName)
		DECLARE_ENTITY_CVAR(uint, BlockTime)
		DECLARE_ENTITY_CVAR(uint, BlockImmobileTime)
		DECLARE_ENTITY_CVAR(float, BlockStaminaCost)
		DECLARE_ENTITY_CVAR(CVec3f, BlockPush)
		DECLARE_ENTITY_CVAR(CVec3f, BlockLunge)
		DECLARE_ENTITY_CVAR(uint, BlockImpactTime)
		DECLARE_ENTITY_CVAR(uint, BlockDuration)
		DECLARE_ENTITY_CVAR(bool, BlockStaminaRequired)
		DECLARE_ENTITY_CVAR(uint, BlockMaxTime)
		DECLARE_ENTITY_CVAR(uint, BlockRecoverTime)

		DECLARE_ENTITY_CVAR(float, PierceUnit)
		DECLARE_ENTITY_CVAR(float, PierceHellbourne)
		DECLARE_ENTITY_CVAR(float, PierceSiege)
		DECLARE_ENTITY_CVAR(float, PierceBuilding)

		DECLARE_ENTITY_CVAR(float, RearMultiplier)
		DECLARE_ENTITY_CVAR(tstring, ImpactEffectPath)
	END_ENTITY_CONFIG

	CEntityConfig*	m_pEntityConfig;

	bool	TraceArc(const CMeleeAttackEvent &attack, uiset &setIndices, vector<CVec3f> &vImpacts);

	uint	ModifyAttackTime(ICombatEntity *pOwner, uint uiTime) const	{ return INT_ROUND(uiTime / pOwner->GetAttackSpeed(GetApplyAttributes())); }

public:
	~IMeleeItem()	{}
	IMeleeItem(CEntityConfig *pConfig) :
	IInventoryItem(pConfig),
	m_pEntityConfig(pConfig)
	{}

	bool			IsMelee() const				{ return true; }

#define GET_DAMAGE(attack, bound) \
	float	Get##attack##Attack##bound##Damage() const \
	{ \
		if (GetApplyAttributes()) \
			return m_pEntityConfig->Get##attack##Attack##bound##Damage() * (1.0f + GetOwnerEnt()->GetAttributeBoost(ATTRIBUTE_STRENGTH)); \
		return m_pEntityConfig->Get##attack##Attack##bound##Damage(); \
	} \

	GET_DAMAGE(Jump, Min)
	GET_DAMAGE(Jump, Max)
	GET_DAMAGE(Strong, Min)
	GET_DAMAGE(Strong, Max)
#undef GET_DAMAGE

	virtual int		GetPrimaryDamageFlags()		{ return DAMAGE_FLAG_MELEE | DAMAGE_FLAG_INTERRUPT; }
	virtual int		GetSecondaryDamageFlags()	{ return DAMAGE_FLAG_MELEE; }

	virtual bool	ActivatePrimary(int iButtonStatus);		// Quick attack
	virtual bool	ActivateSecondary(int iButtonStatus);	// Strong attack
	virtual bool	ActivateTertiary(int iButtonStatus);	// Block

	bool			StartAttack(bool bJump = false);
	virtual void	Impact();

	virtual void	FinishedAction(int iAction);

	virtual void	Selected();
	virtual void	Unselected();

	static void		ClientPrecache(CEntityConfig *pConfig);
	static void		ServerPrecache(CEntityConfig *pConfig);

	tstring	GetQuickAttackAnimName(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetQuickAttackAnimName() + XtoA(ySequence);
	}

	tstring	GetQuickAttackAnimNameRunForward(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetQuickAttackAnimNameRunForward() + XtoA(ySequence);
	}

	tstring	GetQuickAttackAnimNameRunBack(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetQuickAttackAnimNameRunBack() + XtoA(ySequence);
	}

	tstring	GetQuickAttackAnimNameSprint(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetQuickAttackAnimNameSprint() + XtoA(ySequence);
	}

	TYPE_NAME("Melee Weapon")

	ENTITY_CVAR_ACCESSOR(bool, ApplyAttributes, true)

	ENTITY_CVAR_ACCESSOR(float, QuickAttackPivotHeight, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackPivotFactor, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackHeightMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackHeightMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackHeightStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackAngleMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackAngleMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackAngleStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackRangeMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackRangeMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, QuickAttackRangeStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, QuickAttackBlockable, true)
	GET_MELEE_ITEM_FLOAT_VALUE(QuickAttackMinDamage)
	GET_MELEE_ITEM_FLOAT_VALUE(QuickAttackMaxDamage)
	GET_MELEE_ITEM_FLOAT_VALUE(QuickAttackStaminaCost)
	GET_MELEE_ITEM_TIME_VALUE(QuickAttackTime)
	GET_MELEE_ITEM_TIME_VALUE(QuickAttackImpactTime)
	GET_MELEE_ITEM_TIME_VALUE(QuickAttackChainTime)
	GET_MELEE_ITEM_TIME_VALUE(QuickAttackImmobileTime)
	float	GetQuickAttackMinDamage(byte ySequence) const;
	float	GetQuickAttackMaxDamage(byte ySequence) const;

	tstring	GetJumpAttackAnimName(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetJumpAttackAnimName() + XtoA(ySequence);
	}

	ENTITY_CVAR_ACCESSOR(float, JumpAttackPivotHeight, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackPivotFactor, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackHeightMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackHeightMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackHeightStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackAngleMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackAngleMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackAngleStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackRangeMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackRangeMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, JumpAttackRangeStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(bool, JumpAttackBlockable, true)
	GET_MELEE_ITEM_TIME_VALUE(JumpAttackTime)
	GET_MELEE_ITEM_TIME_VALUE(JumpAttackImpactTime)
	GET_MELEE_ITEM_TIME_VALUE(JumpAttackChainTime)

	tstring	GetStrongAttackAnimName(byte ySequence)
	{
		if (m_pEntityConfig == NULL)
			return _T("");

		return m_pEntityConfig->GetStrongAttackAnimName() + XtoA(ySequence);
	}
	ENTITY_CVAR_ACCESSOR(float, StrongAttackPivotHeight, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackPivotFactor, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackHeightMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackHeightMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackHeightStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackAngleMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackAngleMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackAngleStep, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackRangeMin, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackRangeMax, 0.0f)
	ENTITY_CVAR_ACCESSOR(float, StrongAttackRangeStep, 0.0f)
	GET_MELEE_ITEM_TIME_VALUE(StrongAttackTime)
	GET_MELEE_ITEM_TIME_VALUE(StrongAttackImpactTime)
	GET_MELEE_ITEM_TIME_VALUE(StrongAttackChainTime)

	ENTITY_CVAR_ACCESSOR(uint, BlockTime, 0)
	ENTITY_CVAR_ACCESSOR(uint, BlockImpactTime, 0)
	ENTITY_CVAR_ACCESSOR(uint, BlockDuration, 0)
};
//=============================================================================

#endif //__I_MELEEITEM_H__
