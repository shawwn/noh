// (C)2008 S2 Games
// c_combatevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_combatevent.h"

#include "c_damageevent.h"
#include "i_unitentity.h"
#include "i_heroentity.h"
#include "i_projectile.h"
#include "i_areaaffector.h"
#include "c_player.h"
//=============================================================================

/*====================
  CCombatEvent::CCombatEvent
  ====================*/
CCombatEvent::CCombatEvent()
{
	Reset();
}


/*====================
  CCombatEvent::PreImpact
  ====================*/
bool	CCombatEvent::PreImpact()
{
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	IUnitEntity *pInitiator(Game.GetUnitEntity(m_uiInitiatorIndex));
	IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiProxyUID));
	IGameEntity *pInflictor(Game.GetEntity(m_uiInflictorIndex));

	// Perform pre-impact actions
	{
		vector<CCombatActionScript> &vScripts(m_aActionScripts[ACTION_SCRIPT_PRE_IMPACT]);
		for (vector<CCombatActionScript>::iterator it(vScripts.begin()); it != vScripts.end(); ++it)
			it->Execute(Game.GetEntity(m_uiInflictorIndex), pInitiator, pTarget, m_v3TargetPosition, pProxy, this, NULL, NULL, m_v3TargetDelta, 0.0f);
	}

	// Allow target to react to the attack
	if (pTarget != NULL && !m_bNoResponse)
		pTarget->Action(ACTION_SCRIPT_ATTACKED_PRE_IMPACT, pInitiator, pInflictor, this);

	if (pInitiator != NULL && !m_bNoResponse)
		pInitiator->Action(ACTION_SCRIPT_ATTACKING_PRE_IMPACT, pTarget, pInflictor, this);

	// Evasion and chance to miss
	bool bEvaded(false);

	if (!m_bTrueStrike && (CHANCE(m_fEvasion) || CHANCE(m_fMissChance)))
	{
		Game.SendPopup(POPUP_MISS, pInitiator);
		bEvaded = true;
	}

	if (m_yRedirectionSequence != m_yOldRedirectionSequence)
	{
		m_yOldRedirectionSequence = m_yRedirectionSequence;
		return false;
	}

	if (m_bNegated || bEvaded)
		return false;

	float fDamage((m_fBaseDamage + m_fAdditionalDamage) * m_fDamageMultiplier);

	m_fTotalAdjustedDamage = fDamage + (fDamage * m_fBonusMultiplier) + m_fBonusDamage;

	// Critical
	if (pTarget == NULL || !pTarget->IsBuilding())
	{
		float fCriticalMultiplier(1.0f);
		for (FloatPairVector_it it(m_vCriticals.begin()); it != m_vCriticals.end(); ++it)
		{
			if (CHANCE(it->first))
				fCriticalMultiplier = MAX(fCriticalMultiplier, it->second);
		}
		m_fTotalAdjustedDamage *= fCriticalMultiplier;
	}
	
	if (m_fTotalAdjustedDamage > fDamage)
		Game.SendPopup(POPUP_CRITICAL, pInitiator, pInitiator, ushort(m_fTotalAdjustedDamage));

	// Deflection
	if (pTarget != NULL && m_eSuperType == SUPERTYPE_ATTACK && m_uiEffectType & Game.LookupEffectType(_T("Attack")) && m_uiDamageType & Game.LookupEffectType(_T("Physical")))
		AddDeflection(pTarget->GetDeflection());

	// Perform pre-damage actions
	{
		vector<CCombatActionScript> &vScripts(m_aActionScripts[ACTION_SCRIPT_PRE_DAMAGE]);
		for (vector<CCombatActionScript>::iterator it(vScripts.begin()); it != vScripts.end(); ++it)
			it->Execute(pInflictor, pInitiator, pTarget, m_v3TargetPosition, pProxy, this, NULL, NULL, m_v3TargetDelta, 0.0f);
	}

	if (pTarget != NULL && !m_bNoResponse)
		pTarget->Action(ACTION_SCRIPT_ATTACKED_PRE_DAMAGE, pInitiator, pInflictor, this);

	if (pInitiator != NULL && !m_bNoResponse)
		pInitiator->Action(ACTION_SCRIPT_ATTACKING_PRE_DAMAGE, pTarget, pInflictor, this);

	return true;
}


/*====================
  CCombatEvent::Process
  ====================*/
void	CCombatEvent::Process()
{
	if (m_uiTargetIndex != INVALID_INDEX)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
		if (pTarget != NULL)
			m_v3TargetPosition = pTarget->GetPosition();
	}

	if (!PreImpact())
		return;

	Impact();
	PostImpact();

	CheckBounceScripts();

	m_bSuccessful = true;
}


/*====================
  CCombatEvent::Impact
  ====================*/
void	CCombatEvent::Impact()
{
	if (m_fTotalAdjustedDamage == 0.0f)
		return;

	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	IUnitEntity *pInitiator(Game.GetUnitEntity(m_uiInitiatorIndex));

	// Damage
	m_dmg = CDamageEvent();
	m_dmg.SetSuperType(m_eSuperType);
	m_dmg.SetAttackerIndex(m_uiInitiatorIndex);
	m_dmg.SetInflictorIndex(m_uiInflictorIndex);
	m_dmg.SetTargetIndex(m_uiTargetIndex);
	m_dmg.SetAmount(m_fTotalAdjustedDamage);
	m_dmg.SetEffectType(m_uiDamageType);
	m_dmg.SetFlag(m_bNonLethal ? DAMAGE_FLAG_NON_LETHAL : 0);
	m_dmg.SetDeflection(m_fDeflection);
	m_dmg.SetArmorPierce(m_fArmorPierce);
	m_dmg.SetMagicArmorPierce(m_fMagicArmorPierce);
	
	AdjustDamageEvent(m_dmg, pTarget);

	m_dmg.ApplyDamage();

	// Life steal
	if (pTarget != NULL)
	{
		if (pInitiator != NULL && m_fLifeSteal > 0.0f && !pTarget->IsBuilding() && pInitiator->IsEnemy(pTarget))
			pInitiator->Heal(m_dmg.GetAppliedDamage() * m_fLifeSteal);
	}
}


/*====================
  CCombatEvent::PostImpact
  ====================*/
void	CCombatEvent::PostImpact()
{
	IUnitEntity *pInitiator(Game.GetUnitEntity(m_uiInitiatorIndex));
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiProxyUID));
	IGameEntity *pInflictor(Game.GetEntity(m_uiInflictorIndex));

	vector<CCombatActionScript> &vScripts(m_aActionScripts[ACTION_SCRIPT_IMPACT]);
	for (vector<CCombatActionScript>::iterator it(vScripts.begin()); it != vScripts.end(); ++it)
		it->Execute(Game.GetEntity(m_uiInflictorIndex), pInitiator, pTarget, m_v3TargetPosition, pProxy, this, NULL, NULL, m_v3TargetDelta, 0.0f);

	if (pTarget != NULL && !m_bNoResponse)
		pTarget->Action(ACTION_SCRIPT_ATTACKED_POST_IMPACT, pInitiator, pInflictor, this);

	if (pInitiator != NULL && !m_bNoResponse)
		pInitiator->Action(ACTION_SCRIPT_ATTACKING_POST_IMPACT, pTarget, pInflictor, this);
}


/*====================
  CCombatEvent::ProcessInvalid
  ====================*/
void	CCombatEvent::ProcessInvalid()
{
	IUnitEntity *pInitiator(Game.GetUnitEntity(m_uiInitiatorIndex));
	IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
	IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiProxyUID));

	vector<CCombatActionScript> &vScripts(m_aActionScripts[ACTION_SCRIPT_IMPACT_INVALID]);
	for (vector<CCombatActionScript>::iterator it(vScripts.begin()); it != vScripts.end(); ++it)
		it->Execute(Game.GetEntity(m_uiInflictorIndex), pInitiator, pTarget, m_v3TargetPosition, pProxy, this, NULL, NULL, m_v3TargetDelta, 0.0f);
}


/*====================
  CCombatEvent::Reset
  ====================*/
void	CCombatEvent::Reset()
{
	m_uiInitiatorIndex = INVALID_INDEX;
	m_uiInflictorIndex = INVALID_INDEX;

	m_uiTargetIndex = INVALID_INDEX;
	m_v3TargetPosition = V3_ZERO;
	m_v3TargetDelta = V3_ZERO;

	m_uiProxyUID = INVALID_INDEX;

	m_uiEffectType = 0;

	m_eSuperType = SUPERTYPE_INVALID;

	m_dmg = CDamageEvent();

	m_uiDamageType = 0;
	m_fBaseDamage = 0.0f;
	m_fAdditionalDamage = 0.0f;
	m_fDamageMultiplier = 1.0f;

	m_fBonusMultiplier = 0.0f;
	m_fBonusDamage = 0.0f;
	m_fTotalAdjustedDamage = 0.0f;

	m_fArmorPierce = 0.0f;
	m_fMagicArmorPierce = 0.0f;

	m_fLifeSteal = 0.0f;

	m_vCriticals.clear();

	m_fEvasion = 0.0f;
	m_fMissChance = 0.0f;
	m_fDeflection = 0.0f;
	m_bNonLethal = false;
	m_bTrueStrike = false;
	m_bNoResponse = false;
	m_bAttackAbility = false;
	m_iIssuedClientNumber = -1;

	for (uint ui(0); ui < NUM_ACTION_SCRIPTS; ++ui)
		m_aActionScripts[ui].clear();

	m_yOldRedirectionSequence = 0;
	m_yRedirectionSequence = 0;

	m_bNegated = false;
	m_bSuccessful = false;
	m_bRemoveBounceScripts = false;
	m_bInvalid = false;

	m_fManaCost = 0.0f;
	m_uiCooldownTime = 0;
}


/*====================
  CCombatEvent::CheckBounceScript
  ====================*/
void	CCombatEvent::CheckBounceScript(EEntityActionScript eScript)
{
	vector<CCombatActionScript> &vScripts(m_aActionScripts[eScript]);
	vector<CCombatActionScript>::iterator it(vScripts.begin());
	while (it != vScripts.end())
	{
		if (it->GetActivateOnBounces())
		{
			it++;
			continue;
		}

		it = vScripts.erase(it);
	}
}


/*====================
  CCombatEvent::CheckBounceScripts
  ====================*/
void	CCombatEvent::CheckBounceScripts()
{
	if (!m_bRemoveBounceScripts)
		return;

	for (uint ui(0); ui < NUM_ACTION_SCRIPTS; ++ui)
		CheckBounceScript(EEntityActionScript(ui));
}


/*====================
  CCombatEvent::AdjustDamageEvent

  Perform damage event adjustments
  ====================*/
void	CCombatEvent::AdjustDamageEvent(CDamageEvent &cDmg, IUnitEntity *pTarget)
{
	IUnitEntity *pInitiator(Game.GetUnitEntity(m_uiInitiatorIndex));
	IGameEntity *pProxy(Game.GetEntityFromUniqueID(m_uiProxyUID));
	IGameEntity *pInflictor(Game.GetEntity(m_uiInflictorIndex));

	vector<CCombatActionScript> &vScripts(m_aActionScripts[ACTION_SCRIPT_DAMAGE_EVENT]);
	for (vector<CCombatActionScript>::iterator it(vScripts.begin()); it != vScripts.end(); ++it)
		it->Execute(pInflictor, pInitiator, pTarget, m_v3TargetPosition, pProxy, this, &cDmg, NULL, m_v3TargetDelta, 0.0f);

	if (pTarget != NULL && !m_bNoResponse)
		pTarget->Action(ACTION_SCRIPT_ATTACKED_DAMAGE_EVENT, pInitiator, pInflictor, this, &cDmg);

	if (pInitiator != NULL && !m_bNoResponse)
		pInitiator->Action(ACTION_SCRIPT_ATTACKING_DAMAGE_EVENT, pTarget, pInflictor, this, &cDmg);
}
